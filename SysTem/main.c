/***************************** (C) COPYRIGHT 2019 方诚电力 *****************************
* File Name          : main.c
* Author             : 研发部
* Version            : 见历史版本信息
* Date               : 2019/02/21
* Description        : 符合南方电网输电线路在线监测通信规约定V3.0标准，能传感采集导线或
金具表面温度，并将结果通过通信网络传至监测代理装置或状态监测主站。具有自动采集温度功能。
具备温度受控采集功能，能响应远程指令，按设定采集方式、自动采集时间、采集时间间隔启动采集。
能循环储存至少30天的温度状态数据。
************************************  历史版本信息  ************************************/
#include "main.h"




/*全局变量*/
unsigned int		OSInterrputSum;
INT8U				TaskActive=RF_ACT|LTE_ACT|Local_ACT;						//bit0\1\2分别指示 RF,LTE,Local任务是否处于激活状态，挂起/删除任务时对应位置0，上电初始化3个bit都是1
INT32U				WDT[RWNUM] = {0};
INT8U				StopModeLock=0;												//用于判断是否要进入STOP模式
OS_STK				RF_STK[RF_STK_SIZE] = {0};
OS_STK				Local_STK[Local_STK_SIZE] = {0};
OS_STK				LTE_STK[LTE_STK_SIZE] = {0};
OS_STK				WDT_STK[WDT_STK_SIZE] = {0};

/*静态全局变量*/
static INT32U		OldTime=0;													//记录每次状态开始的时间（上一次切换状态的时间点）

/*UCOSII邮箱*/
OS_EVENT			*Dev_CMDB0X = (OS_EVENT *)0;								//设备命令邮箱，用于设备命令消息传递
OS_EVENT			*Dev_STAB0X = (OS_EVENT *)0;								//设备状态邮箱，用于设备状态消息传递
OS_EVENT			*Data_CMDB0X = (OS_EVENT *)0;								//采集指令邮箱，用于采集指令消息传递
OS_EVENT			*Fault_CMDB0X = (OS_EVENT *)0;								//故障信息邮箱，用于故障信息消息传递

/*DEBUG模式下，assert_failed()用于检测传递给函数的参数是否是有效的参数；调试结束后请使用Release模式重新编译，以提高运行效率*/
void assert_failed(u8* file, u32 line)
{
	/* User can add his own implementation to report the file name and linenumber, 
	ex: printf("Wrong parameters value: file %s on line %d\r\n", file,line) */ 
	/* Infinite loop */
	while(1);
}





/*******************************************************************************
									main函数入口
*******************************************************************************/
int main(void)
{
	struct BSPRTC_TIME 	Reset_Time = {0x00,0x00,0x00,0x02,0x01,0x01,0x19}; 		//2019.01.01 周二 0时0分0秒
	OSInterrputSum = 0;	
	
/*系统启动前准备工作*/
	NVIC_Configuration(); 														//中断分配	
	RCC_Configuration4M();														//时钟配置为4MHz
	IO_LowPower();																//IO口低功耗配置，好像要放在时钟配置之后才行
	Power485Pin_Init();															//与485电源相关的引脚配置
	RTC_Init(NRTC_Fre);															//初始化内部RTC，用于低功耗唤醒
	BSP_WDGInit();																//外部硬件看门狗初始化初始化
	BSP_WDGFeedDog(); 															//外部硬件看门狗 喂狗操作，1.6S狗饿	
	BSP_RX8025Init();															//RTC芯片初始化
	if(RtcGetChinaStdTimeStruct(&gRtcTime)==0)									//读取RTC时间失败时
	{
		RtcSetChinaStdTimeStruct(&Reset_Time);									//设置初始值
		Time_Proofread = UNDONE;												//标记为校时未完成
	}
	SysJudgeAndMarkBkp();														//判断当前运行在SYS0还是SYS1，并标记BKP->DR3
	
/*系统初始化*/
	OSInit();																	//初始化OS
	Tmr_TickInit(4000000);														//初始化OS Tick

	// 本地																		//此任务用完就会删除
	OSTaskCreateExt(Task_Local_main, (void *)0, (OS_STK *)&Local_STK[Local_STK_SIZE - 1],
	Local_Task_Prio,
	Local_Task_Prio,
	(OS_STK *)&Local_STK[0],
	Local_STK_SIZE,
	(void *)0,
	OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);	
	
	// LTE
	OSTaskCreateExt(Task_LTE_Main, (void *)0, (OS_STK *)&LTE_STK[LTE_STK_SIZE - 1],
	LTE_Task_Prio,
	LTE_Task_Prio,
	(OS_STK *)&LTE_STK[0],
	LTE_STK_SIZE,
	(void *)0,
	OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);	
	
	// 无线
	OSTaskCreateExt(Task_RF_Main, (void *)0, (OS_STK *)&RF_STK[RF_STK_SIZE - 1],
	RF_Task_Prio,
	RF_Task_Prio,
	(OS_STK *)&RF_STK[0],
	RF_STK_SIZE,
	(void *)0,
	OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	// 看门狗
	OSTaskCreateExt(Task_Wdt_main, (void *)0, (OS_STK *)&WDT_STK[WDT_STK_SIZE - 1],
	Wdt_Task_Prio,
	Wdt_Task_Prio,
	(OS_STK *)&WDT_STK[0],
	WDT_STK_SIZE,
	(void *)0,
	OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);	
	
	
/*系统启动*/
	OSStart();																	//启动多任务环境
}





/*******************************************************************************
名称：void WDTSscn(void)
功能：任务看门狗扫描。大约200分钟，任务看门狗起作用  
入参：无
出参：无
返回：无
*******************************************************************************/
void WDTSscn(void)
{
	unsigned char i;
	for(i=0;i<RWNUM;i++)														//RWNUM=3，当前只创建了三个任务（与Local_ACT等宏定义配合使用，不可随意改动）
	{	
		if(TaskActive&(1<<i)){													//如果该任务处于激活状态，则扫描时对应的任务看门狗加1
		WDT[i]++;																//wdtindex（4~6）对应WDT[0~3]
		}
		if(WDT[i]>1500)															//约15.25分钟
		{
			McuSoftReset(); 													//系统软件复位。
		}
	}
}

/*******************************************************************************
名称：void WDTClear(unsigned char wdtindex)
功能：任务看门狗喂狗函数，由各个任务主程序调用
入参：wdtindex : 
			4 : Task_RF_Main
			5 : Task_LTE_Main
			6 : Task_Local_main 
出参：无
返回：无
*******************************************************************************/
void WDTClear(unsigned char wdtindex)
{
	if( wdtindex>( RWNUM+3 ) ) return;
    WDT[wdtindex-4] = 0;														//wdtindex（4~6）对应WDT[0~3]
}

/*******************************************************************************
名称：void Task_Wdt_main(void *org)
功能：看门狗任务
入参：无
出参：无
返回：无
*******************************************************************************/
void Task_Wdt_main(void *org)
{
	INT8U				CS=0;
	INT8U				Dev_Stat=WAKE_STAT;										//初始设定为在线状态
	
	if(Dev_STAB0X == NULL) Dev_STAB0X = OSMboxCreate(0);						//创建设备状态邮箱
	else Dev_STAB0X->OSEventPtr= (void *)0;										//清消息邮箱
	if(Dev_CMDB0X == NULL) Dev_CMDB0X = OSMboxCreate(0);						//创建设备命令邮箱
	else Dev_CMDB0X->OSEventPtr= (void *)0;										//清消息邮箱
	if(Data_CMDB0X == NULL) Data_CMDB0X = OSMboxCreate(0);						//创建设备命令邮箱
	else Data_CMDB0X->OSEventPtr= (void *)0;									//清消息邮箱
	if(Fault_CMDB0X == NULL) Fault_CMDB0X = OSMboxCreate(0);					//创建故障信息邮箱
	else Fault_CMDB0X->OSEventPtr= (void *)0;									//清消息邮箱
	
	Led_Init();
	IWDG_Init(); 																//内部独立看门狗初始化
	
	OldTime= RtcGetTimeSecond();
	while(1)
	{
		WDTSscn();   															//任务看门狗要求每个任务按时来清除(维护)各自的计数器，
		Feed_Dog();
	
		CS++;
		if(CS%5==0)
		{
			Led_On();
			OSTimeDly(1);
		}
		else
		{
			Led_OFF();
			OSTimeDly(15);
		}
		if(CS%15==0)															//每5个计数为61个OSTimeDly，15个为15/5*61个OSTimeDly=9.15s 轮询一次
		{			
			Dev_Stat = DevStatCtr(Dev_Stat);									//设备状态控制。南网协议有设置休眠时长和唤醒时长。		
			if(CS%(15*6)==0) 													//9.15*6=54.9s 轮询一次
			{
				RtcGetChinaStdTimeStruct(&gRtcTime);							//从RTC获取当前时间，存入gRtcTime
				ClearFlowDataDailyAndMonthly(&gRtcTime);						//每天零点清除日流量统计，每月初清除月流量统计
				Reset_On_Time(&gRtcTime);										//定点重启轮询，放在此处将有一分钟的防重入
				CheckSys2OperatingNormally(&gRtcTime);							//成功运行24h后，认为程序正常，SYS1运行次数清零
				CS=0;
			}
		}			
	}
}
																				
/*******************************************************************************
名称：INT8U DevStatCtr(INT8U state)
功能：设备状态的控制，进行设备在线休眠的计时，及设备状态按命令进行改变
入参：INT8U state
出参：无
返回：返回当前（休眠/唤醒）状态
*******************************************************************************/
INT8U DevStatCtr(INT8U state)
{
	static INT32U		DataTime=0;												//记录每次采集开始的时间（上一次采集数据的时间点）
	static INT8U		msg_data;
	static INT8U		msg_state;
	static INT8U		msg_cmd;
	INT32U				NewTime=0;												//当前时间
	INT32U				Interval_Time=0;										//间隔时间，用于判断是否超过在线或休眠时间，是否改变状态
	INT16U				Sample_Delay_Second=0;
	INT8U				Err=0;
	INT16U				ONLineTime=(INT16U)(Config.OnlineTime[0]<<8)+Config.OnlineTime[1];		//读取在线时长参数
	INT16U				SleepTime =(INT16U)(Config.SleepTime[0]<<8)+Config.SleepTime[1];		//读取休眠时长参数
	INT16U				ScanInterval =(INT16U)(Config.ScanInterval[0]<<8)+Config.ScanInterval[1];//读取休眠时长参数(分钟)
	
	NewTime= RtcGetTimeSecond();		 										//获取当前的时间	单位：秒
	Interval_Time=(NewTime-OldTime)/60;											//间隔时间，单位分钟
	
	if(Net_Fault_Time && !Fault_Manage.F_NETWORK && (NewTime-Net_Fault_Time)/(30*60))			//联网故障计时轮询，大于30分钟则上报联网故障 0202H		产生故障后，计时会继续，但故障状态已标记，再恢复前不会再次产生故障
		NW_Fault_Manage(NETWORK_F, FAULT_STA);
	
	if(Host_No_Reply_Time && !Fault_Manage.F_REPLY && (NewTime-Host_No_Reply_Time)/(30*60))		//主站无应答故障计时轮询，大于30分钟则上报故障 0201H	同上
		NW_Fault_Manage(REPLY_F, FAULT_STA);
	
	if(!DataTime)																//首次上电时，人工推迟采集时间（先让射频接收一会）
	{
		Sample_Delay_Second=Sample_Wait_Time>ScanInterval*60?ScanInterval*60:Sample_Wait_Time;	//（选小的时间）如果设置的Sample_Wait_Time>采集间隔时间，则直接延迟采集间隔时间 单位：秒
		DataTime = NewTime-(ScanInterval*60-Sample_Delay_Second);				//会延时Sample_Wait_Time秒后满足采集时间	单位：秒
	}
	
	/*采集时间到点轮询处理*/
	if((NewTime-DataTime)/60 >= ScanInterval )									//超过采集间隔
	{
		DataTime = NewTime;														//将计时起始时间置为现在
		msg_data = DATA_CMD;													//用于发出，通知RF任务切换到采集数据状态
		OSMboxPost(Data_CMDB0X, &msg_data);										//通过邮箱将采集命令发出
	}
	
	msg_state = *(INT8U *)OSMboxPend(Dev_STAB0X,1,&Err);						//查询装置状态邮箱（会清空原信息）
	switch(state)
	{
		case WAKE_STAT:
				/*根据邮箱消息处理*/
					if((Err==OS_NO_ERR)&&(msg_state==SLEEP_SUCCESS))			//接到休眠成功通知
					{
						state = SLEEP_STAT;										//自用
						OldTime = NewTime;										//将计时起始时间置为现在
					}
					else if((Err==OS_NO_ERR)&(msg_state==WAKE_SUCCESS))			//如果在线状态中接到唤醒指令，则重置起始时间
					{
						OldTime=NewTime;
					}
					
				/*在线时长到期，发出休眠命令*/
					else if(Interval_Time>=ONLineTime)							//在线时长到期
					{
						if(update_start==true)									//还在升级过程中，禁止LTE休眠
						{
							upgrade_timeout++;
							if(upgrade_timeout>33) 								//本函数轮询每次9.15s，额外给5min进行升级
							{
								update_start = false;							//超时，退出升级状态，允许LTE休眠
								upgrade_timeout = 0;
							}
						}
						else													//不在升级状态
						{
							msg_cmd = SLEEP_CMD;								//通知LTE任务切换到休眠状态
							OSMboxPost(Dev_CMDB0X, &msg_cmd);					//发出命令							
						}
					}
					break;
					
		case SLEEP_STAT:	
				/*根据邮箱消息处理*/
					if((Err==OS_NO_ERR)&&(msg_state==WAKE_SUCCESS))				//接到唤醒成功通知													
					{
						state = WAKE_STAT;										//自用
						OldTime = NewTime;										//将计时起始时间置为现在	
					}
					else if((Err==OS_NO_ERR)&(msg_state==SLEEP_SUCCESS))		//如果休眠状态中接到休眠指令，则重置起始时间
					{
						OldTime=NewTime;
					}
					
				/*休眠时长到期，发出唤醒命令*/
					else if(Interval_Time>=SleepTime)							//休眠时长到期													
					{
						msg_cmd = WAKE_CMD;										//通知LTE任务切换到唤醒状态		
						OSMboxPost(Dev_CMDB0X, &msg_cmd);						//发出命令
					}
					break;
					
		default: 
					break;
	}
	return state;	
}	

/*******************************************************************************
名称：void Reset_On_Time(struct BSPRTC_TIME *pTime)
功能：定点重启轮询
入参：HEX时间
出参：无
返回：无
*******************************************************************************/
void Reset_On_Time(struct BSPRTC_TIME *pTime)
{
	INT8U				minute = 0, hour = 0, day = 0;
	
	minute = BcdToHex(pTime->Minute);											
	hour = BcdToHex(pTime->Hour);
	day = BcdToHex(pTime->Day);
	
	if((Config.ResetTime[1]==hour) && (Config.ResetTime[2]==minute))			//时分对应上了
	{
		if((!Config.ResetTime[0]) || (Config.ResetTime[0]==day))				//若日期设置为零则每天定点重启，否则比对日期对应上了设备重启
		{
			McuSoftReset();
		}
	}
}

/*******************************************************************************
名称：void Feed_Dog(void )
功能：喂狗
入参：无
出参：无
返回：无
*******************************************************************************/
void Feed_Dog(void )
{
	IWDG_Reset();
	BSP_WDGFeedDog();
}

/*******************************************************************************
名称：void IO_LowPower(void)
功能：所有IO口复位，设置为低功耗状态。 
入参：无
出参：无
返回：无
*******************************************************************************/
void IO_LowPower(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	
	GPIO_DeInit(GPIOA);     												 	//这会导致所有IO口都复位，IO口电平改变
	GPIO_DeInit(GPIOB);
	GPIO_DeInit(GPIOC);
	GPIO_AFIODeInit();
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC , ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);										//不必要的引脚都初始化为AIN，有需要的逐步打开使用
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/*******************************************************************************
名称：void Led_Init(void)
功能：Led灯初始化程序。 
入参：无
出参：无
返回：无
*******************************************************************************/
void Led_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =Led_PIN;
	GPIO_Init(Led_Port, &GPIO_InitStructure);
}

/*******************************************************************************
名称：void Led_On(void)
功能：点亮Led灯。 
入参：无
出参：无
返回：无
*******************************************************************************/
void Led_On(void)
{
	 GPIO_SetBits(Led_Port,Led_PIN);
}

/*******************************************************************************
名称：Led_OFF(void)
功能：熄灭Led灯。 
入参：无
出参：无
返回：无
*******************************************************************************/
void Led_OFF(void)
{
	 GPIO_ResetBits(Led_Port,Led_PIN);
}

/*******************************************************************************
名称：void SysJudgeAndMarkBkp(void)
功能：根据程序运行地址，判断当前运行在SYS0还是SYS1，并标记BKP->DR3。
入参：无
出参：无
返回：无
*******************************************************************************/
void SysJudgeAndMarkBkp(void)
{
	static int(*fun_P)(void);
	INT32U	adress_now=0;
		
	/**判断在APP1还是在APP2,记在DR3中，指示当前运行程序在flash的位置*/
	fun_P=&main;
	adress_now=(INT32U)fun_P;
	if((adress_now>=0x08006000)&&(adress_now<=0x08027000))
	{
		PWR->CR|=1<<8;																	//DBP位：取消后备区域的写保护。1：允许写入RTC和后备寄存器
		BKP->DR3=0x01;				
		PWR->CR&=~(1<<8);
	}
	else if((adress_now>=0x08027000)&&(adress_now<=0x08060000))
	{
		PWR->CR|=1<<8;																	//DBP位：取消后备区域的写保护。1：允许写入RTC和后备寄存器
		BKP->DR3=0x02;	
		PWR->CR&=~(1<<8);
	}	
}

/***************************** (C) COPYRIGHT 2019 方诚电力 *********END OF FILE**********/
