/***************************** (C) COPYRIGHT 2019 方诚电力 *****************************
* File Name          : LTE.c
* Author             : 杜颖成、陈威、等
* Version            : 见历史版本信息
* Date               : 2019/02/21
* Description        : LTE通信功能实现，需要调用通信协议与4G模块驱动文件。
************************************  历史版本信息  ************************************
* 2019/03/28    : V4.1.0
* Description   : 南网测温项目初版。基础功能完成，调试中。
*******************************************************************************/
#include "LTE.h"

/*全局变量定义*/
INT8U 			LTE_Buff[LTE_BUFF_LEN] = {0};									//用于组帧
INT8U 			LTE_Tx_Buff[LTE_BUFF_LEN] = {0};								//用于LTE发送
INT8U 			LTE_Rx_Buff[LTE_BUFF_LEN] = {0};								//用于LTE接收
INT32U			Net_Fault_Time=0;												//用于网络连接失败计时，当计时满30分钟时产生一次网络连接错误故障上报，重新连上时将会产生网络连接恢复上报
INT32U 			Host_No_Reply_Time=0;											//用于主站无应答计时，当计时满30分钟时产生一次主站无应答错误故障上报，再次发送讯息得到回复时产生故障恢复上报



/************************************************************************************************************************
* Function Name : void Task_LTE_Main(void *arg)                                                    
* Description   : LTE任务函数，用于LTE传输数据。
*************************************************************************************************************************/
void Task_LTE_Main(void *arg)
{
	INT8U	retry = 0;
	INT8U	SOCKID = 1;   														// 直接指定Socket号为1
    INT8U	NW_CMD = 0;
	INT8U	LTE_Sending_Flag = UNDONE;
	
	TaskActive &= LTE_INACT;													//不再轮巡该任务的任务看门狗
	OSTaskSuspend(OS_PRIO_SELF);												//挂起自身任务
	TaskActive |= LTE_ACT;       												//任务恢复，继续轮巡该任务看门狗
	
	B485_init(38400);															//485初始化波特率38400（当LOCAL任务存在时会强制执行，否则根据B485DIS宏定义判断是否执行）
	
	while(1)
	{
	/*电源控制*/
		if(Reset_Count>2) BAT_CTL_PIN_H();										//当冷复位次数超过2次时，直接强制打开锂电池
		
	/*节能工作模式判断*/
		while(Equipment_state.BAT_Volt<BAT_UNDER && Equipment_state.FALA_Volt<FALA_UNDER)	//BAT<9.2V，FALA<5V
		{
			BspUartWrite(2,SIZE_OF("节能模式中……\r\n"));
			OSTimeDly(3*60*20);													//3min轮询
			Get_Voltage_MCUtemp_Data( 3 );										//获取电池电压数据和单片机温度
		}
		
	/*LTE模块开机*/
		BspUartWrite(2,SIZE_OF("----------------------------------LTE模块开机配置中\r\n"));OSTimeDly(1);
		ME909SInit(ME909SBaudrate);
		for(retry=0;retry<3;retry++)											//最多重试3次
		{
		/*LTE模块操作*/
			if(!ME909S_ON())													//模块开机，并配置串口
			{
				ME909S_PW_OFF();												//关电源
				OSTimeDly(3*20);
				continue;														//失败时重试（跳过下面语句）
			}
//			if(!ME909S_Contact()) continue;										//若尝试联络失败则重新开机		//开机失败的话，这里还可以充当10秒的延时
			if(!ME909S_SMS_CFG()) continue; 									//短信配置			
			if(!ME909S_CONFIG()) continue;     						 			//ME909S进行本机配置
			if(!ME909S_REG()) continue;											//网络注册
			if(!ME909S_Link(SOCKID)) continue;									//连接并打开透传
			
		/*正常时都在此循环，仅退出休眠后重连失败、电源欠压时跳出*/
			while(1)															//用于休眠结束回到南网通信流程
			{
			/*装置在线中……*/
				BspUartWrite(2,SIZE_OF("----------------------------------已进入在线状态\r\n"));OSTimeDly(1);	
				BSP_InitFm(LTE_Num);											//初始化铁电存储

				LTE_Sending_Flag = UNDONE;
				BSP_WriteDataToFm(LTE_Sending_Flag_Addr,&LTE_Sending_Flag,1);   //写入铁电
				Feed_Dog();	
				NW_CMD = NW_Comm_Process();										//根据南网协议编写的通信流程（装置waking）
				if(NW_CMD)														//LTE发送未失败
				{
					Reset_Count=0;												//此标志区分发送成败
					BSP_WriteDataToFm(Reset_Count_Addr,&Reset_Count,1); 		//每次发送完成后将reset计数清空，累计两次复位会强制打开电池						
				}
				LTE_Sending_Flag = DONE;										//LTE发送标志，此标志 不 区分发送成败
				BSP_WriteDataToFm(LTE_Sending_Flag_Addr,&LTE_Sending_Flag,1);   //写入铁电

				BspUartWrite(2,SIZE_OF("\r\n----------------------------------已退出在线状态\r\n"));OSTimeDly(1);
				
			/*节能工作模式判断*/
				if(Equipment_state.BAT_Volt<BAT_UNDER && Equipment_state.FALA_Volt<FALA_UNDER)	//BAT<9.2V，FALA<5V
				{
					BspUartWrite(2,SIZE_OF("---------->>电源欠压，进入节能工作模式<<----------\r\n"));OSTimeDly(1);
					retry = 3;													//用于跳出for循环
					break;														//退出while循环
				}
				
			/*装置休眠中……*/
				Sleep_Or_Reset(NW_CMD);											//执行返回的休眠或重启指令（装置sleeping）有低功耗处理
				
			/*链接重连并打开透传*/
				if(!ME909S_Link(SOCKID)) 	break ;								//退出休眠后需进行链接重连并打开透传，重连失败则重新开机								
			}
		}
		
	/*LTE模块关机*/	
		BspUartWrite(2,SIZE_OF("----------------------------------LTE模块关机\r\n"));OSTimeDly(1);
		ME909S_OFF();			  												//关机
		ME909S_LowPower();														//低功耗
	}
}/*end of Task_LTE_Main()*/





/*******************************************************************************
名称：void Sleep_Or_Reset(u8 CMD)
功能：根据传入的指令选择执行休眠或重启动作
入参：u8 CMD，传入的指令
出参：无
返回：无
*******************************************************************************/
void Sleep_Or_Reset(u8 CMD)
{	
	static INT8U	msg_state;
	static INT8U	msg_cmd;
	INT16U			SMS_Len=0;
	INT8U			Err=0;
	INT8U*			R=NULL;
	
	ME909S_Trans_OFF();															//返回到命令模式,进行复位或短信的等待，需用到AT指令
	ME909S_IPCLOSE(7);															//关闭所有链接
	ME909S_SMS_Delete(1,4);														//休眠前清空所有短信避免受到干扰
	
	/*装置重启*/
	if(CMD==RESET_DEV) 
	{
		BspUartWrite(2,SIZE_OF("----------------------------------装置重新启动中\r\n"));OSTimeDly(1);
		DeviceRstOnCMD();														//整机重启
	}
	
	/*装置进入休眠状态，成功时发Dev_STAB0X消息*/
	BspUartWrite(2,SIZE_OF("----------------------------------已进入休眠状态\r\n"));OSTimeDly(1);
	
	/*外设低功耗*/		
	FM_LowPower(LTE_Num);
	BAT_CTL_PIN_L();            												//关闭强制打开电源电池    若法拉电容没电时，这个拉低也不会造成什么影响，本身电源控制引脚就会处于低电平

	msg_state = SLEEP_SUCCESS;
	OSMboxPost(Dev_STAB0X, &msg_state);											//通知看门狗任务中的计时函数开始休眠计时

	while(1)
	{
	/*等待定时唤醒50ms*/
		msg_cmd = *(INT8U *)OSMboxPend(Dev_CMDB0X,1,&Err);						//延时；查询邮箱（会清空原信息，读过就要POST）
		if((Err==OS_NO_ERR) && (msg_cmd==WAKE_CMD)) break;						//若休眠时长超时（收到唤醒命令）则跳出
		
	/*进入休眠并等待短信唤醒5s*/
		SMS_Len=ME909S_Waitfor_SMS(LTE_Rx_Buff,5);								//等待短信5秒（LTE任务不再做任何事情，仅等待短信唤醒跳出休眠）	
		if(SMS_Len!=0) 															//未收到短信，继续等待
		{
			R=ME909S_SMS_Extract(LTE_Rx_Buff,SMS_Len,(INT8U*)&SMS_Len);	
			if((R)&&(SMS_Judge(R,SMS_Len))) break;								//确认为唤醒短信则跳出
		}
		WDTClear(LTE_PRIO);														//清除任务看门狗
	}
	
/*已被唤醒*/
	msg_state = WAKE_SUCCESS;
	OSMboxPost(Dev_STAB0X, &msg_state);											//通知看门狗任务中的计时函数开始唤醒计时
	BspUartWrite(2,SIZE_OF("----------------------------------已退出休眠状态\r\n"));OSTimeDly(1);
}
