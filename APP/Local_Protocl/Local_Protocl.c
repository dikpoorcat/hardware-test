/***************************** (C) COPYRIGHT 2019 方诚电力 *****************************
* File Name          : Local_Protocl.c
* Author             : 研发部
* Version            : 见历史版本信息
* Date               : 2019/03/12
* Description        : 本地协议，用于基站设备与上位机进行485通信，设置参数与调试。
************************************  历史版本信息  ************************************
* 2019/03/28    : V4.1.0
* Description   : 南网测温项目初版。基础功能完成，调试中。
*******************************************************************************/
#include "Local_Protocl.h"

/*全局变量*/
INT8U					B485BUF[Buff485_LEN] = { 0 };   						//用于B485通信
INT8U  					Reset_Flag = COLDRST;    								//冷复位（默认值）、热复位、发送中复位
INT8U					Reset_Count = 0;          								//冷启动计数标识
struct Str_Msg 			GgdataBox; 
struct Str_Msg 			*GyMess = (struct Str_Msg *)0;      
struct LOCAL_PROTOCAL	Local_Protocal;      									//用于本地协议通信
OS_EVENT  				*GyBOX = (OS_EVENT *)0;

BYTE					work[FF_MAX_SS];										/* Work area (larger is better for processing time) */   //FF_MAX_SS
FIL						fil;            										/* File object */
FATFS					fs;														/* Filesystem object */
FATFS					*fs0;

																														
/*******************************************************************************
名称：void Task_Local_main(void *arg)
功能：本地协议处理任务主函数，用于与上位机通信并进行参数配置等。（装置自检应该也可以做到这里ZE）
************************************************************************************************************************/
void Task_Local_main(void *arg)
{
    INT8U WaitTime = 6;															//上电期间打开485，用于设置参数 WaitTime*10秒
	INT8U state=0;

/*硬件初始化*/
	BSP_InitFm(LOC_Num);														//初始化铁电存储
    B485_init(38400);															//485初始化波特率38400（当LOCAL任务存在时会强制执行，否则根据B485DIS宏定义判断是否执行）
	DrawSysLogo();																//SYS0：打印BTC	logo	SYS1：打印SYS1
	
/*硬件测试*/
	HardwareTest();
/*===============================不会再往下运行了===============================*/	
	
/*设备自检*/
	WaitTime = Check_Reset_Mod(WaitTime);										//判断启动方式，并相应处理。有铁电操作。返回WaitTime用于跳过上位机通信
	FM_Space_Usage();															//铁电空间占用信息
	Check_Getfree();															//检查是否要格式化（包含新设备写入出厂默认参数）

/*铁电参数读取（需放在格式化之后）*/
	Read_TT_From_FM();															//读取探头信息
	LTE_Get_Config();   														//读取LTE参数
	Print_Config(true);															//打印当前参数
	Get_NW_Info();																//读取Fault_Info故障信息结构体数组、未上报数据索引表等
	GetUpgradeTime();															//读取系统2升级时间
	
/*等待上位机通信及处理*/
	BspUartWrite(2,SIZE_OF("---------->>等待上位机通信<<----------\r\n"));
    while(WaitTime--)															//上电期间打开485，用于设置参数 WaitTime*10秒
    {
		state = Wait_Local_Comm(10);											//最多等待10s	
        if(state==1) WaitTime = 6;   											//一旦发现有人在通过PC工具操作基站的话，立即重置最长等待时间为1分钟。 				
		else if(state==0xFF) break;												//接收到跳过等待命令	68 ff ff ff ff ff 02 00 00 00 65 16
    }
	/*配置参数读取,可能在本地设置中被改变，干脆重新读一次*/
	Read_TT_From_FM();															//读取探头信息			
	LTE_Get_Config();   														//读取LTE参数
	BspUartWrite(2,SIZE_OF("---------->>上位机通信结束<<----------\r\n"));
	
/*FATFS目录维护等*/
	Dir_Maintenance();															//FATFS目录维护，放在上位机通信之后，便于生产
//	Dir_Test();																	//目录测试
//	File_Test();																//文件操作测试

/*低功耗处理*/
	BSP_UART_Close(2); 															//降低串口的消耗（调用了B485_LowPower）
	FM_LowPower(LOC_Num);														//铁电存储低功耗	  

/*恢复RF任务*/
	OSTaskResume(RF_Task_Prio);													//恢复RF任务
	
/*等待电压正常，恢复LTE任务*/
	while(1)																	//现在这个while不做也行，LTE任务里也有节能工作模式判断了
	{
		if(Equipment_state.BAT_Volt>BAT_UP || Equipment_state.FALA_Volt>FALA_UP)//BAT>9.5V，或FALA>5.5V
		{
			OSTaskResume(LTE_Task_Prio);										//恢复LTE任务
			break;
		}
		BspUartWrite(2,SIZE_OF("电源欠压，请充电！\r\n"));
		OSTimeDly(3*60*20);														//等待充电3min	
		Get_Voltage_MCUtemp_Data( 3 );											//获取电池电压数据和单片机温度
		OSTimeDly(10);															//等待电压打印完成
		WDTClear(Local_Task_Prio);
	}
	#ifdef B485DIS
	BspUartWrite(2,SIZE_OF("---------->>B485打印已关闭<<----------\r\n"));
	#endif
//	OSTimeDly(10);																//等待打印完成
	
/*删除自身任务*/
    while(1)
    {	
		WDTClear(Local_Task_Prio);
		TaskActive &= Local_INACT;												//若删除成功，不再轮巡该任务的任务看门狗
		OSTaskDel(OS_PRIO_SELF);												//删除本任务，删除成功之后下面的都不会跑了		
		TaskActive |= Local_ACT;												//程序往下走了说明没删除成功，则继续轮巡清除该任务的任务看门狗
		OSTimeDly(60*20);														//60s
    }
}







/*******************************************************************************
名称：INT8U Check_Reset_Mod(u8 waittime)
功能：判断启动方式，并相应处理。有铁电操作。需要外部先对485初始化。注：Hot_Reset_Flag在整个工程
中，读取后都不应再对其赋值，这样可以在任何地方使用此全局变量。
入参：u8 waittime，上电期间打开485，用于设置参数 waittime*10秒
出参：无
返回：热启动时会返回0，否则返回传入值。返回值用于赋值给WaitTime，热启动时不等待485操作。
*******************************************************************************/
INT8U Check_Reset_Mod(u8 waittime)
{
	INT8U	LTE_Sending_Flag = DONE;
	
	/*串口调试打印*/
    BspUartWrite(2,SIZE_OF("\r\n485 is OK!\r\n")); 									
	
	/*读取铁电中的标志位*/
	BSP_ReadDataFromFm(Reset_Flag_Addr,&Reset_Flag,1);							//读取复位标志
	BSP_ReadDataFromFm(Reset_Count_Addr,&Reset_Count,1);						//读取复位计数
	BSP_ReadDataFromFm(LTE_Sending_Flag_Addr,&LTE_Sending_Flag,1);   			//发送过程标识读取出来
	
	/*判断启动方式并相应处理*/
	if(LTE_Sending_Flag == UNDONE) Reset_Flag = FAULTRST;						//若UNDONE，表明在发送中重启了，写为Fault启动													
	switch(Reset_Flag)
	{
		case COLDRST:															//冷复位
				BspUartWrite(2,SIZE_OF("==============================Cold Reset!==============================\r\n"));
				Reset_Count++;													//系统连续复位2次后，LTE发送前强制打开电池，发送完后关闭并清计数							
				BSP_WriteDataToFm(Reset_Count_Addr,&Reset_Count,1);				//写入铁电
				break;
		
		case FAULTRST:															//发送中复位
				BspUartWrite(2,SIZE_OF("------------------------------Fault Reset!------------------------------\r\n"));
				/*看看要不要断点续传*/
				break;
		
		case HOTRST:															//热启动时，将热启动标志抹掉
				BspUartWrite(2,SIZE_OF("Hot Reset!\r\n")); 
				waittime = 0;
				break;
		
		default:																//若都不是，则存储错乱
				break;
	}
	Reset_Flag = COLDRST;
	BSP_WriteDataToFm(Reset_Flag_Addr,&Reset_Flag,1); 							//写为冷启动标识（重置初始值）
	OSTimeDly(2);

	/*热启动时会返回0，否则返回传入值*/
	return waittime;															//返回WaitTime用于跳过上位机通信
}

/*******************************************************************************
名称：INT8U Wait_Local_Comm(INT8U WaitTime)
功能：通过485等待本地通信，并执行与命令相应的功能。（485一般用于打印输出，接收的话一般只
有上位机下发的配置指令，因此调用Local_Protocol_Process函数）
入参：INT8U WaitTime，串口等待超时时长，单位：秒
出参：无
返回：1：成功接收并处理   0：失败		0xFF：收到跳过等待指令
*******************************************************************************/
INT8U Wait_Local_Comm(INT8U WaitTime)
{
    INT8U Err = 0, state = 0; 
    
    while(WaitTime--)
    {
		StopModeLock++;
        GyMess = (struct Str_Msg *)OSMboxPend(GyBOX,20,&Err);   				//等待串口消息1s
		if(StopModeLock) StopModeLock--;										//打开停机锁
        if (Err == OS_NO_ERR)
    	{
            if( GyMess->MsgID == BSP_MSGID_RS485DataIn)							//检查ID，是不是485串口发来的
    		{
				if(GyMess->DataLen > Buff485_LEN)								//长度超出，异常
				{
					BSP_UART_RxClear(GyMess->DivNum);	
					return 0;
				}
				
			/*公司内部协议解析并处理*/
				state = Local_Protocol_Process(GyMess->pData,GyMess->DataLen,B485BUF);
				BSP_UART_RxClear(GyMess->DivNum);								//解析完成后，串口接收缓冲区清空
				if(state==0) continue;											//不符合本公司协议时继续等待，避免误动作
				else return state;
    		}
    	}
    }
    return 0;  																	//无数据进入
}

/*******************************************************************************
名称：INT8U Local_Protocol_Process(INT8U *In,INT16U inLen,INT8U *pUseBuff)
功能：本公司的内部协议解析并处理。
入参：INT8U *In,待解释内容指针；INT16U inLen,待解释内容长度；INT8U *pUseBuff，用于485发送使用的缓存。
出参：无
返回：1：成功解析并处理   0：失败		0xFF：收到跳过等待指令
*******************************************************************************/
INT8U Local_Protocol_Process(INT8U *In,INT16U inLen,INT8U *pUseBuff)
{
	INT8U *pRet = NULL;
	INT8U Err = 0;
	INT16U Rlen = 0;  															//一帧有效长度

	pRet = Judge_Local_Framing(In,inLen,&Rlen);									//判断是否符合本地协议，并存入Local_Protocal结构体
	if(pRet)  																	//找到一帧有效的数据
	{
		switch(Local_Protocal.DoType)
    	{
            case 0: // 设置参数
					Err = Set_Local_Para(Local_Protocal.CMD,Local_Protocal.Pointer,Local_Protocal.CMDlen);			//本地参数配置
					Local_Protocol_Reply(Err,Local_Protocal.CMD,pUseBuff);   										//组帧并回复
					break;
    		
            case 1: // 读取参数
                    Err = Get_Local_Para(Local_Protocal.CMD,pUseBuff+10,&Rlen);										//读取参数，存入pUseBuff+10
                    if(Err) Local_Protocol_Reply(Err,Local_Protocal.CMD,pUseBuff);									//错误上报
                    else Local_Protocol_Reply_Para(pUseBuff+10,Rlen,Local_Protocal.CMD,pUseBuff);					//参数上报
            		break;
    		
			case 0xFF: //跳过485等待
					return 0xFF;												//表示收到跳过等待指令
			
            case 0x0f: // 文件传输												//可禁用此功能，把升级功能仅仅放在BOOT程序（实际上现在也是这样的）
            		break;
		}
		return 1;
	}
	return 0;
}

/*******************************************************************************
* Function Name : INT8U *Judge_Local_Framing(INT8U *pInBuff,INT16U Len,INT16U *pOutLen)
* Description   : 判断是否符合本公司内部规约协议
* Input         : pInBuff : 输入数据包头指针
*                 Len     : 输入数据包的长度
*                 pOutLen ：主调提供的，用于存储返回有效包长度的空间的指针
*
* Return        : 显式返回 pRet    : 有效协议包头指针，指向CMD参数的首地址
*               : 形参返回 pOutLen : 有效协议包的长度
*               ：隐式返回 Local_Protocal     ：还将这个全局机构体变量填充了。
*******************************************************************************/
INT8U *Judge_Local_Framing(INT8U *pInBuff,INT16U Len,INT16U *pOutLen)
{
	INT16U i = 0;
	INT16U Packet_Length = 0;
	INT8U *pRet = NULL;

/*协议基本校验*/
	if (Len < 8) return 0;  													//协议基本长度不能小于8
	for(i = 0; i<7; i++)
	{
		if(pInBuff[i] == 0x68)
		{
			pRet = &pInBuff[i]; 												//记录包首位置
			Len -= i;           												//修正长度
			break;
		}
	}
	if(i >= 7) return 0;  														//前8字节内未找到包头，当作失败
	if(Len < 8) return 0;														//协议基本长度，不能小于8（长度修正后再次判断）
	Packet_Length = pRet[7];													//报文内容长度，高字节
	Packet_Length <<= 8;
	Packet_Length += pRet[6];													//报文内容总长度
	Packet_Length += 10;														//整体长度
	if (Len < Packet_Length) return 0;  										//协议长度错误
	if (0 == Judge_Device_Addr(pRet+1)) return 0;								//判断是否为有效的地址（通信地址，可选择同一条总线上的不同设备）
	i = RTU_CS(pRet,Packet_Length-2);											//计算累加和
	if (i != pRet[Packet_Length-2]) return 0;									//校验累加和
	if (0x16 != pRet[Packet_Length-1]) return 0;								//校验包尾

/*将接收的数据整理进Local_Protocal结构体*/
	Local_Protocal.DoType = pRet[5];
	switch(Local_Protocal.DoType)
	{
		case 0:	//写
//				if(!memcmp(&Local_Protocal.Password,pRet+8,3)) return 0;		//密码校验（上位机密码功能好像没做吧）
				Local_Protocal.CMD = (pRet[12]<<8) + pRet[11];					//CMD u16
				Local_Protocal.CMDlen = Packet_Length - 15; 					//CMD对应参数的长度
				Local_Protocal.Pointer = &pRet[13];								//指向CMD参数的首地址
				break;

		case 1:	//读
				Local_Protocal.CMD = (pRet[9]<<8) + pRet[8];					//CMD u16
				Local_Protocal.CMDlen = 0;				 									
				Local_Protocal.Pointer = 0;
				break;

		case 0xff:																//指令	68 ff ff ff ff ff 02 00 00 00 65 16
				break;															//跳过等待
		
		case 0x0f:	//升级程序
		case 0x31:	//从装置中读取历史数据
				Local_Protocal.Pointer = &pRet[11];								//指向CMD参数的首地址
				Local_Protocal.CMDlen = Packet_Length-14; 						//ID的长度
				break;
		
		default:
				break;
	}
	if(pOutLen) *pOutLen = Packet_Length;										//传出一包正确的数据长度Packet_Length
	return pRet;																//返回有效协议首地址
}

/*******************************************************************************
名称：INT8U Set_Local_Para(INT16U CMD,INT8U *pInBuff,INT8U Len)
功能：通过本地协议进行参数配置，如时间、IP、端口号、主站卡号、探头ID等。
入参：INT16U CMD,配置命令；INT8U *pInBuff,输入内容的指针；INT8U Len，传入内容的长度。
出参：无
返回：本地协议错误代码
*******************************************************************************/
INT8U Set_Local_Para(INT16U CMD,INT8U *pInBuff,INT8U Len)
{
	INT8U				Temp = 0;
	static INT32U		buff[512] = {0};										//占用空间太大，小心死机
	
	switch(CMD)
	{
		case 0xffff:	/*本地功能*/ 
				if(!Local_Function(pInBuff)) return Other_Err;                 	//本地功能，复位等
				break;
		
		case 0x0001:	/*系统时间*/
				if(!System_Time_Fun(pInBuff,WRITETYPE))
				{
					Time_Proofread = UNDONE;									//标记为校时未完成
					return Other_Err; 	    									//系统时间操作					
				}
				else
				{
					Time_Proofread = DONE;										//标记为校时已完成
				}
				break;
		
		case 0x0004:	/*IP地址+端口号+APN*/
				Temp = pInBuff[5];												//端口号：由于本地采用的是小端写入，而南网采用大端模式，这里双字节的参数需要颠倒一下
				pInBuff[5] = pInBuff[6];
				pInBuff[6] = Temp;
		
				/*使能字节 1个字节，0x50表示 此通道用于上报数据，心跳，等待命令	这个对南网没用，不管它*/
				/*IP与端口*/
				if(!BSP_WriteDataToFm(IP_Config_Addr,pInBuff+1,4)) return Other_Err;						//写入 IP_addr_1&2
				if(!BSP_WriteDataToFm(IP_Config_Addr+6,pInBuff+1,4)) return Other_Err;
				if(!BSP_WriteDataToFm(IP_Config_Addr+4,pInBuff+5,2)) return Other_Err;						//写入 PortNum_1&2 端口号
				if(!BSP_WriteDataToFm(IP_Config_Addr+10,pInBuff+5,2)) return Other_Err;
				/*APN*/
				Temp = pInBuff[7];												//上位机下发APN第一个字节为长度，目前最多下发1+32字节
				pInBuff[8+Temp] = '\0';											//上位机下发APN无结束符，补上
				if(!BSP_WriteDataToFm(APN_Addr,pInBuff+8,APN_Len)) return Other_Err;						//写入 APN	
				break;

		case 0x1000:	/*写装置编号*/
				/*写入内部FLASH*/
				Read_NFlash(0x08006000-0x800, buff, 512);						//读取写有滚码的页	注意：滚码是16进制，Device_Number是BCD
				buff[448] = pInBuff[5]-0x30;									//下发的ascii转hex，并写入滚码对应位置（真实值，即小端模式写入）
				buff[448] += (pInBuff[4]-0x30)*10;
				buff[448] += (pInBuff[3]-0x30)*100;
				buff[448] += (pInBuff[2]-0x30)*1000;
				Wrtie_NFlash(0x08006000-0x800, buff, 512);						//带擦除。把修改好的这一页写回去

				/*写入铁电*/
				if(!BSP_WriteDataToFm(Device_Number_Addr,pInBuff,Device_Number_Len)) return Other_Err;		//长度必须是固定的6位，PC发过来的会是17位，照6位写才不会影响其他存储	
				break;
		
//		case ????:	/*主站卡号*/
//				if(!BSP_WriteDataToFm(IP_Config_Addr+12,pInBuff,Len)) return Other_Err;						//写入 CardNum_1&2 主站卡号
//				if(!BSP_WriteDataToFm(IP_Config_Addr+18,pInBuff,Len)) return Other_Err;
//				break;
					 
		case 0x11FF:	/*探头ID*/
				switch(pInBuff[0])												//根据指令选择相应操作
				{
					case 0: 
						if(! Add_TT_ID(pInBuff+1))return Other_Err;				//增加一个测量点
						break;
					case 1:
						if(! Delete_TT_ID(pInBuff+1))return Other_Err; 			//删除一个测量点		
						break;
					case 2:	
						if(! Delete_All_TT_ID())return Other_Err;	 			//删除全部测量点
						break;
				}
				break;
	}
	return No_Err;																//正常，返回无错误
}

/*******************************************************************************
* Function Name : INT8U Get_Local_Para(INT16U CMD,INT8U *pOutBuff,INT16U *pOutLen)
* Description   : 读取指定CMD项出来到pOutBuff指定的空间中去，读取到的长度“转手返回”给行参pOutLen指定的空间中去。
* Input         : CMD      : 见协议附件A、附件B
*                 pOutBuff : 拷贝的目的地址
*                 pOutLen  : CMD对应的长度, 在结构 ID_STR或Tn_STR 结构体中，ID的长度都增加了1，用于保存校验字节    

*
* Return        : 其他	  ：错误代码
*                 No_Err  ：正确
*******************************************************************************/
INT8U Get_Local_Para(INT16U CMD,INT8U *pOutBuff,INT16U *pOutLen)
{
	INT8U Temp = 0, state = No_Err;
	
	switch(CMD)
	{
		case 0x0000:	/*读版本号*/
				pOutBuff[0]=(VERSION>>24) & 0xFF;
				pOutBuff[1]=(VERSION>>16) & 0xFF;
				pOutBuff[2]=(VERSION>>8) & 0xFF;
				pOutBuff[3]=(VERSION>>0) & 0xFF;
				*pOutLen = 4;
				return No_Err;
		
		case 0x0001:	/*设备时间*/
				 System_Time_Fun(pOutBuff,READTYPE);        					// 读系统时间
				 *pOutLen = 7;	
				 return No_Err;	
	
		case 0x0004:	/*IP地址+端口号+APN*/									//pOutBuff+7是APN，南网没用到，不管
				pOutBuff[0] = 0x50;												//使能字节 1个字节，0x50表示 此通道用于上报数据，心跳，等待命令
				state = BSP_ReadDataFromFm(IP_Config_Addr,pOutBuff+1,4);		//读出 IP_addr_1&2
				if(!state) break;

				state = BSP_ReadDataFromFm(IP_Config_Addr+4,pOutBuff+5,2);		//读出 PortNum_1&2 端口号
				if(!state) break;
		
				state = BSP_ReadDataFromFm(APN_Addr,pOutBuff+8,APN_Len);		//读APN pOutBuff+7是APN长度，目前最大32
				if(!state) break;
		
				Temp = pOutBuff[5];												//由于本地采用的是小端写入，而南网采用大端模式，这里双字节的参数需要颠倒一下
				pOutBuff[5] = pOutBuff[6];
				pOutBuff[6] = Temp;
				pOutBuff[7] = strlen((TCHAR*)APN);								//APN长度
				*pOutLen = 8+32;
				return No_Err;

		case 0x1000:															//读取装置编码
				if(BSP_ReadDataFromFm(Device_Number_Addr,pOutBuff,Device_Number_Len)){
					*pOutLen = Device_Number_Len;
					return No_Err;}
				break;
				
//		case ????: 	/*不知道南网的主站卡号是什么意思，本地协议暂时没有相应的CMD*/
//				if(BSP_ReadDataFromFm(IP_Config_Addr+12,pOutBuff,6))			//读出 CardNum_1&2 主站卡号
//				{
//					*pOutLen =6;
//					return No_Err;
//				}
//				break;
						
		default: break;
	}
/*测量点查询*/
	if((CMD >= 0x1100)&&(CMD <= 0x1137))  										//测量点查询，按最多55个做的
	{	
		*pOutLen = Read_TT_Num_Or_ID(CMD,pOutBuff);
		return No_Err;
	}
	return Other_Err;
}

/*******************************************************************************
* Function Name: INT8U System_Time_Fun(INT8U *pInOutBuff,INT8U Type)             
* Description:   设置或者读取系统时间 
* Input:         pInOutBuff   : 指向实时时钟时间串的指针，这里需要的顺序是：年、月、日、时、分、秒、周
*                Type         ：WRITETYPE：设置系统时间
*                               READTYPE : 读取系统时间
*
* Return:        1：成功   0：失败
*******************************************************************************/
INT8U System_Time_Fun(INT8U *pInOutBuff,INT8U Type)
{
	if (Type == WRITETYPE) 											
	{
		//PC发来的数组顺序是年月日时分秒周的BCD码！！！，而BSPRTC_TIME的顺序是秒分时周日月年的BCD码！！！需要转换一下
		gSetTime.Year=pInOutBuff[0];
		gSetTime.Month=pInOutBuff[1];
		gSetTime.Day=pInOutBuff[2];
		gSetTime.Hour=pInOutBuff[3];
		gSetTime.Minute=pInOutBuff[4];
		gSetTime.Second=pInOutBuff[5];
		gSetTime.Week=pInOutBuff[6];
		return RtcSetChinaStdTimeStruct(&gSetTime);								//写系统时间
	}
	else
	{
		return GetSysTime(pInOutBuff);											//读取系统时间 
	}
}
																				
/*******************************************************************************
* Function Name : INT8U Judge_Device_Addr(INT8U *pAddr)
* Description   : 判断是否为有效的装置地址
* Input         : pAddr : 指向协议中装置地址字节的指针
*
* Return        : 0 : 无效地址
*               : 1 ：有效地址
*******************************************************************************/
INT8U Judge_Device_Addr(INT8U *pAddr)
{
	INT16U len = 0;

	if((pAddr[0]==0xff)&&(pAddr[1]==0xff)&&(pAddr[2]==0xff)&&(pAddr[3]==0xff)) 	//4个0xff表示广播地址，有效
		return 1;   														
	Get_Local_Para(0x0002,Local_Protocal.Addr,&len);							//获取装置地址
	if(0==memcmp(pAddr,&Local_Protocal.Addr,4)) return 1;
	return 0;
}

/*******************************************************************************
* Function Name : INT8U Local_Function(INT8U *pInBuff)                                                               
* Description   : CMD FFFF 对应的处理函数，
* Input         : pInBuff：
*                 
* Return        : 1：成功	0：失败（统一标准ZE）
*******************************************************************************/
INT8U Local_Function(INT8U *InBuff)
{
	switch(*InBuff)
	{
		case 0xff: // 系统复位
				McuSoftReset();													//手动系统复位，手动点击“方诚电力产品参数管理V1.3”-->基本参数-->系统复位 按钮后就会进入到这个case来
				break;
					
		/*其他功能都可以在这里添加*/
		default: 
				break;
	}
	return 1;
}

/*******************************************************************************
名称：void Local_Protocol_Reply(INT8U Err,INT16U CMD,INT8U *pUseBuff)
功能：按本地协议，通过485进行回复通信。
入参：INT8U Err,错误代码；INT8U DOType,0xC0指明是有错误返回，0x80无错误返回；INT16U CMD,INT8U *pUseBuff
出参：无
返回：无
*******************************************************************************/
void Local_Protocol_Reply(INT8U Err,INT16U CMD,INT8U *pUseBuff)
{
	INT8U DOType;
    pUseBuff[0] = 0x68;                            								//协议包头
    memcpy(&pUseBuff[1],Local_Protocal.Addr,4);             					//通信源地址
	if(Err) DOType = 0xC0;   													//0xC0 标识有错误返回
	else DOType = 0x80;      													//0x80 标识无错误返回	
    pUseBuff[5] = DOType;                          								//操作类型
    pUseBuff[6] = 3;                               								//数据域长度，小端模式
    pUseBuff[7] = 0;															//固定3字节
    pUseBuff[8] = CMD&0xff;                         							//CMD低字节
    pUseBuff[9] = (CMD>>8)&0xff;                    							//CMD高字节
    pUseBuff[10] = Err;                            								//错误代码
    pUseBuff[11] = RTU_CS(pUseBuff,11);            								//累加和校验
    pUseBuff[12] = 0x16;                           								//协议包尾

	BspUartWrite(2,pUseBuff,13);												//发送给PC
}

/*******************************************************************************
名称：void Local_Protocol_Reply_Para(INT8U *In,INT16U len,INT16U CMD,INT8U *pUseBuff)
功能：按本地协议，通过485进行回复通信，上报读取的参数。
入参：INT8U *In,INT16U len,INT16U CMD,INT8U *pUseBuff 太坑爹了，懒得改了，自己看吧
出参：无
返回：无
*******************************************************************************/
void Local_Protocol_Reply_Para(INT8U *In,INT16U len,INT16U CMD,INT8U *pUseBuff)
{
	INT16U	i = 0;
	INT8U	DOType = 0x81;														//0x81，表示从装置读取CMD参数
	
    pUseBuff[0] = 0x68;                           								//协议包头
    memcpy(pUseBuff+1,Local_Protocal.Addr,4);            						//通信源地址
    pUseBuff[5] = DOType;                          								//操作类型
    pUseBuff[6] = (len+2) &0xff;
    pUseBuff[7] = ((len+2)>>8) &0xff;
    pUseBuff[8] = CMD&0xff;
    pUseBuff[9] = (CMD>>8)&0xff;
    if(In != pUseBuff+10)														//这么神奇的操作头一次见  不过还能用
    {
        for(i=0;i<len;i++)
    	{
            pUseBuff[10+i] = In[i];
    	}
    }
    pUseBuff[10+len] = RTU_CS(pUseBuff,10+len);
    pUseBuff[11+len] = 0x16;

	BspUartWrite(2,pUseBuff,12+len);											//给PC工具软件回复
}

#if 1 /*============================================================探头录入与读取函数============================================================*/
/*******************************************************************************
* Function Name : INT8U Add_TT_ID(INT8U *pInBuff)
* Description   : 增加一个ID,如果要增加的ID号已经存在，则直接返回。
*
* Input         : pInBuff: 指向ID号字节串的指针。	这里的pInBuff为 FF FF +两字节的TT_ID
*
* Return        : 1 :标识执行成功
*******************************************************************************/
INT8U Add_TT_ID(INT8U *pInBuff)	
{
	INT8U INDEX=0;
	INT8U i=0;
	
	INDEX=CMP_TT_ID(pInBuff+2);													//在索引表中查找，并返回索引
	if(INDEX==0xFF)																//未找到匹配的ID，说明未录入
	{
		for(i=0;i<55;i++)														//寻找第一个空位
		{
			if(TT_Info.TT_ID[i][0]!=0) continue;
			if(TT_Info.TT_ID[i][1]!=0) continue;								//已被占领
			
			memcpy(TT_Info.TT_ID[i],pInBuff+2,2);								//空位置，将ID填入
			TT_Info.TT_Count+=1;												//录入计数加1
			return BSP_WriteDataToFm(TT_Count_Addr,&TT_Info.TT_Count,sizeof(TT_Info));		//写入铁电
		}
	}	
	return 1;																	//在已录入列表中能找到，无需重复录入							
}

/*******************************************************************************
* Function Name : INT8U Delete_TT_ID(INT8U *pInBuff)	
* Description   : 寻找一个已录入的探头ID，若有删除之
*
* Input         : pInBuff: 指向ID号字节串的指针。	这里的pInBuff为 FF FF +两字节的TT_ID
*
* Return        : 1 :标识执行成功
*******************************************************************************/
INT8U Delete_TT_ID(INT8U *pInBuff)	
{
	INT8U 	INDEX=0;
	INT16U	TT_Data_Addr=0;
	
	INDEX=CMP_TT_ID(&pInBuff[2]);
	if(INDEX==0xFF)		return 1;												//未找到匹配的ID，说明未录入，无需删除，直接返回成功
	else{		
		memset(TT_Info.TT_ID[INDEX],0,2);										//删除对应的探头ID
		TT_Info.TT_Count-=1;													//能找到索引则Count一定大于0
		TT_Data_Addr=Sample_Data_Addr+One_TT_Sample_Data_Len*INDEX;				//对应探头在铁电中存储采集数据的起始位置
		BSP_FM_Erase(TT_Data_Addr,One_TT_Sample_Data_Len);						//同时删除对应探头原采集数据，避免再录入新的探头占用了该位置而造成混乱
		return BSP_WriteDataToFm(TT_Count_Addr,&TT_Info.TT_Count,sizeof(TT_Info));			//写入铁电
	}																									
}

/*******************************************************************************
* Function Name : INT8U Delete_All_TT_ID(void)		
* Description   : 删除所有已录入的探头
*
* Input         : 无
* Return        : 1 :标识执行成功
*******************************************************************************/
INT8U Delete_All_TT_ID(void)	
{
	memset(&TT_Info.TT_Count,0,sizeof(TT_Info));								//清空探头信息结构体												
	BSP_FM_Erase(Sample_Manage_Addr,Sample_Manage_Len);							//同时清除采样管理结构体（探头都清空了，保存的数据与新的探头也无法对应上，应丢弃原来的数据）
	return BSP_WriteDataToFm(TT_Count_Addr,&TT_Info.TT_Count,sizeof(TT_Info));	//写入铁电																							
}

/*******************************************************************************
* Function Name : INT8U Read_TT_Num_Or_ID(INT16U ID,INT8U *pOutBuff)		
* Description   : 读取指定探头ID
*
* Input         : ID：PC串口发下来的读取探头个数或ID的代号		pOutBuff:传出探头ID的位置->用于组帧回复
* Return        : pOutBuff数据长度
*******************************************************************************/
INT8U Read_TT_Num_Or_ID(INT16U ID,INT8U *pOutBuff)	
{	
	INT8U INDEX=0;
	if(ID==0x1100)																//读取探头个数
	{
		pOutBuff[0]=TT_Info.TT_Count;	
		return 1;
	}
		
	INDEX=ID-0x1101;															//ID范围0x1101~0x1137对应的是索引位置是0~54	
	pOutBuff[0]=0xFF;															//上报串口的ID前两位需添加FF
	pOutBuff[1]=0xFF;
	memcpy(pOutBuff+2,&TT_Info.TT_ID[INDEX],2);									//按照索引装填对应的探头ID
	return 4;																	//ID为4个字节		
}
#endif

/*******************************************************************************
名称：void FM_Space_Usage(void)
功能：输出铁电空间占用信息。
入参：无
出参：无
返回：无
*******************************************************************************/
void FM_Space_Usage(void)
{
	TCHAR   chars[256]={0};
	
	sprintf(chars, "系统参数配置区K0已使用%d字节（%d%%USED）\r\n",FM_K0_End_Addr,100*FM_K0_End_Addr/0x400);
	BspUartWrite(2,(INT8U*)chars,strlen(chars));OSTimeDly(1);

	sprintf(chars, "温度采样数据区K1-K7已使用%d字节（%d%%USED）\r\n",FM_K1_K7_End_Addr-0x400,100*(FM_K1_K7_End_Addr-0x400)/0x1C00);
	BspUartWrite(2,(INT8U*)chars,strlen(chars));OSTimeDly(1);
	
	sprintf(chars, "统计：铁电已使用%d字节（%d%%USED）\r\n",FM_K1_K7_End_Addr-0x400+FM_K0_End_Addr,100*(FM_K1_K7_End_Addr-0x400+FM_K0_End_Addr)/0x2000);
	BspUartWrite(2,(INT8U*)chars,strlen(chars));OSTimeDly(1);
}

/*******************************************************************************
名称：void Print_Config(INT8U cmd)
功能：从485打印LTE参数：参数配置结构体&主站IP地址、端口号和卡号配置结构体
入参：INT8U cmd，0：关闭打印；1：开启打印
出参：无
返回：无
*******************************************************************************/	
void Print_Config(INT8U cmd)
{
	TCHAR	temp[200]={0},version[4]={0};
	
	if(!cmd) return;															//若打印关闭，返回
	
	version[3] = VERSION & 0xFF;
	version[2] = (VERSION>>8) & 0xFF;
	version[1] = (VERSION>>16) & 0xFF;
	version[0] = (VERSION>>24) & 0xFF;											//版本号宏定义，点魔术棒		
	sprintf(temp, "\r\n---------->>打印设备信息<<----------\r\n");
	sprintf(temp+strlen(temp), "系统版本：%c%c.%c.%c\r\n\r\n",version[0],version[1],version[2],version[3]);
	
	/*打印本地时间*/
	RtcGetChinaStdTimeStruct(&gRtcTime);										//从时钟芯片取得RTC时间
	sprintf(temp+strlen(temp), "本地时间：20%X年%X月%X日 %02X:%02X:%02X\r\n",gRtcTime.Year,gRtcTime.Month,gRtcTime.Day,gRtcTime.Hour,gRtcTime.Minute,gRtcTime.Second);
	BspUartWrite(2,(INT8U*)temp,strlen(temp));

	/*打印装置号码Device_Number*/
	sprintf(temp, "\r\n");
	sprintf(temp+strlen(temp), "装置号码：%c%c%c%c%c%c\r\n",Device_Number[0],Device_Number[1],Device_Number[2],Device_Number[3],Device_Number[4],Device_Number[5]);
	BspUartWrite(2,(INT8U*)temp,strlen(temp));

	/*打印主站IP地址、端口号和卡号配置结构体IP_Config*/
	sprintf(temp, "\r\n");
	sprintf(temp+strlen(temp), "主站IP：%d.%d.%d.%d	",IP_Config.IP_addr_1[0],IP_Config.IP_addr_1[1],IP_Config.IP_addr_1[2],IP_Config.IP_addr_1[3]);
	sprintf(temp+strlen(temp), "端口号：%d	",(IP_Config.PortNum_1[0]<<8)+IP_Config.PortNum_1[1]);
	sprintf(temp+strlen(temp), "APN：%s\r\n", APN);
	sprintf(temp+strlen(temp), "主站卡号：%X%X%X%X%X%X\r\n",IP_Config.CardNum_1[0],IP_Config.CardNum_1[1],IP_Config.CardNum_1[2],IP_Config.CardNum_1[3],IP_Config.CardNum_1[4],IP_Config.CardNum_1[5]);
	BspUartWrite(2,(INT8U*)temp,strlen(temp));	
	
	/*打印参数配置结构体Config*/
	sprintf(temp, "\r\n");
	sprintf(temp+strlen(temp), "装置密码：%c%c%c%c\r\n",Config.Password[0],Config.Password[1],Config.Password[2],Config.Password[3]);
	sprintf(temp+strlen(temp), "心跳间隔：%d分钟\r\n",Config.BeatTime[0]);
	sprintf(temp+strlen(temp), "采集间隔：%d分钟\r\n",(Config.ScanInterval[0]<<8)+Config.ScanInterval[1]);
	sprintf(temp+strlen(temp), "休眠时长：%d分钟\r\n",(Config.SleepTime[0]<<8)+Config.SleepTime[1]);
	sprintf(temp+strlen(temp), "在线时长：%d分钟\r\n",(Config.OnlineTime[0]<<8)+Config.OnlineTime[1]);
	sprintf(temp+strlen(temp), "重启时间：%d日%d时%d分(0日表示每天)\r\n",Config.ResetTime[0],Config.ResetTime[1],Config.ResetTime[2]);
	sprintf(temp+strlen(temp), "密文验证：%c%c%c%c\r\n",Config.SecurityCode[0],Config.SecurityCode[1],Config.SecurityCode[2],Config.SecurityCode[3]);
	BspUartWrite(2,(INT8U*)temp,strlen(temp));
	
	/*打印功能配置参数FUN_Config*/
	sprintf(temp, "\r\n");
	sprintf(temp+strlen(temp), "启用功能：%XH,%XH",FUN_Config[0],FUN_Config[1]);
	BspUartWrite(2,(INT8U*)temp,strlen(temp));
	
	/*打印流量统计信息Local_FLow_Data*/
	sprintf(temp, "\r\n\r\n");
	sprintf(temp+strlen(temp), "每月套餐流量：%d KB\r\n",MONTHLY_FLOW<<10);
	sprintf(temp+strlen(temp), "今日已用流量：%d KB\r\n",Local_FLow_Data.Flow_Day_Used_B/1024);
	sprintf(temp+strlen(temp), "本月已用流量：%d KB\r\n",Local_FLow_Data.Flow_Month_Used_B/1024);
	sprintf(temp+strlen(temp), "本月剩余流量：%d KB\r\n\r\n",(MONTHLY_FLOW<<10)-Local_FLow_Data.Flow_Month_Used_B/1024);
	BspUartWrite(2,(INT8U*)temp,strlen(temp));
	
	/*打印电压信息*/
	Get_Voltage_MCUtemp_Data( 3 );											//获取电池电压数据和单片机温度
	OSTimeDly(1);
}

/*******************************************************************************
名称：void DrawSysLogo(void)
功能：从485打印 BTC logo
入参：无
出参：无
返回：无
*******************************************************************************/
void DrawSysLogo(void)
{
	TCHAR	temp[500]={0};
	
	if(SYS==0)	/*根据宏定义SYS判断*/
	{
		sprintf(temp, 			   "                   / .]]]OOOOOO]]].\r\n");                     
		sprintf(temp+strlen(temp), "                ]OOOOOOOOOOOOOOOOOOOOOO]\r\n");
		sprintf(temp+strlen(temp), "            ,/OOOOOOOOOOOOOOOOOOOOOOOOOOOO\\`\r\n");
		sprintf(temp+strlen(temp), "          /OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\\\r\n");          
		sprintf(temp+strlen(temp), "       ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO`\r\n");       
		sprintf(temp+strlen(temp), "      /OOOOOOOOOOOOOOOOOOO^  [OOOOOOOOOOOOOOOOOOO\\\r\n");      
		sprintf(temp+strlen(temp), "    ,OOOOOOOOOOOOOOOOOOOOO   =OO   =OOOOOOOOOOOOOOO`\r\n");    
		sprintf(temp+strlen(temp), "   =OOOOOOOOOOOOOOO     [`  ,OO^  .OOOOOOOOOOOOOOOOO^\r\n"); 
		BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(2);
		sprintf(temp,			   "  =OOOOOOOOOOOOOOO\\.              =OOOOOOOOOOOOOOOOOO^\r\n");  
		sprintf(temp+strlen(temp), " ,OOOOOOOOOOOOOOOOOOOO.              ,OOOOOOOOOOOOOOOO`\r\n"); 
		sprintf(temp+strlen(temp), " OOOOOOOOOOOOOOOOOOOO/     =OOO\\`      ,OOOOOOOOOOOOOOO\r\n");  
		sprintf(temp+strlen(temp), "=OOOOOOOOOOOOOOOOOOOO`     OOOOOOO`     =OOOOOOOOOOOOOO^\r\n"); 
		sprintf(temp+strlen(temp), "=OOOOOOOOOOOOOOOOOOO/     =OOOOOOO      /OOOOOOOOOOOOOO^\r\n"); 
		sprintf(temp+strlen(temp), "OOOOOOOOOOOOOOOOOOOO`                  =OOOOOOOOOOOOOOOO\r\n"); 
		BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(2);
		sprintf(temp, 			   "OOOOOOOOOOOOOOOOOOO/     ,].         /OOOOOOOOOOOOOOOOOO\r\n"); 
		sprintf(temp+strlen(temp), "OOOOOOOOOOOOOOOOOOO`     OOOOOO\\      ,OOOOOOOOOOOOOOOOO\r\n"); 
		sprintf(temp+strlen(temp), "=OOOOOOOOOOOOOOOOO/     =OOOOOOOO`     ,OOOOOOOOOOOOOOO^\r\n"); 
		sprintf(temp+strlen(temp), ".OOOOOOOOOOOOO^         OOOOOOOOO      =OOOOOOOOOOOOOOO.\r\n"); 
		sprintf(temp+strlen(temp), " =OOOOOOOOOOO\\`                        OOOOOOOOOOOOOOO^\r\n");  
		sprintf(temp+strlen(temp), "  OOOOOOOOOOOOOOOOO^                 ,OOOOOOOOOOOOOOOO\r\n");   
		sprintf(temp+strlen(temp), "   OOOOOOOOOOOOOOOO   =OO   =000000OOOOOOOOOOOOOOOOOO\r\n");
		BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(2);	    
		sprintf(temp,			   "    \\OOOOOOOOOOOOO^   OO^   OOOOOOOOOOOOOOOOOOOOOOO/\r\n");
		sprintf(temp+strlen(temp), "     =OOOOOOOOOOOOOOOOOO]. =OOOOOOOOOOOOOOOOOOOOOO^\r\n");      
		sprintf(temp+strlen(temp), "       \\OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO/\r\n");        
		sprintf(temp+strlen(temp), "        ,\\OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO/`\r\n");         
		sprintf(temp+strlen(temp), "           \\OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO[\r\n");            
		sprintf(temp+strlen(temp), "              [OOOOOOOOOOOOOOOOOOOOOOOOOO[\r\n");               
		sprintf(temp+strlen(temp), "                  ,[OOOOOOOOOOOOOOOO[`\r\n");     
		BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(10);					//延时要够，否则会异常
	}
	else if(SYS==1)
	{	
		sprintf(temp, 			   "   SSSSSSSSSSSSSSS YYYYYYY       YYYYYYY   SSSSSSSSSSSSSSS   1111111\r\n");   
		sprintf(temp+strlen(temp), " SS:::::::::::::::SY:::::Y       Y:::::Y SS:::::::::::::::S 1::::::1\r\n");   
		sprintf(temp+strlen(temp), "S:::::SSSSSS::::::SY:::::Y       Y:::::YS:::::SSSSSS::::::S1:::::::1\r\n");   
		sprintf(temp+strlen(temp), "S:::::S     SSSSSSSY::::::Y     Y::::::YS:::::S     SSSSSSS111:::::1\r\n");   
		BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(2);
		sprintf(temp, 			   "S:::::S            YYY:::::Y   Y:::::YYYS:::::S               1::::1\r\n");   
		sprintf(temp+strlen(temp), "S:::::S               Y:::::Y Y:::::Y   S:::::S               1::::1\r\n");   
		sprintf(temp+strlen(temp), " S::::SSSS             Y:::::Y:::::Y     S::::SSSS            1::::1\r\n");   
		sprintf(temp+strlen(temp), "  SS::::::SSSSS         Y:::::::::Y       SS::::::SSSSS       1::::l\r\n");   
		BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(2);
		sprintf(temp, 			   "    SSS::::::::SS        Y:::::::Y          SSS::::::::SS     1::::l\r\n");   
		sprintf(temp+strlen(temp), "       SSSSSS::::S        Y:::::Y              SSSSSS::::S    1::::l\r\n");   
		sprintf(temp+strlen(temp), "            S:::::S       Y:::::Y                   S:::::S   1::::l\r\n");   
		sprintf(temp+strlen(temp), "            S:::::S       Y:::::Y                   S:::::S   1::::l\r\n");
		BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(2);
		sprintf(temp, 			   "SSSSSSS     S:::::S       Y:::::Y       SSSSSSS     S:::::S111::::::111\r\n");
		sprintf(temp+strlen(temp), "S::::::SSSSSS:::::S    YYYY:::::YYYY    S::::::SSSSSS:::::S1::::::::::1\r\n");
		sprintf(temp+strlen(temp), "S:::::::::::::::SS     Y:::::::::::Y    S:::::::::::::::SS 1::::::::::1\r\n");
		sprintf(temp+strlen(temp), " SSSSSSSSSSSSSSS       YYYYYYYYYYYYY     SSSSSSSSSSSSSSS   111111111111\r\n");
		BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(10);					//延时要够，否则会异常
	}
	
	sprintf(temp,"\r\n编译时间：%s %s\r\n",__DATE__,__TIME__);
	BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(1);						
}

/*******************************************************************************
名称：void DrawErrLogo(void)
功能：从485打印 err logo
入参：无
出参：无
返回：无
*******************************************************************************/
void DrawErrLogo(void)
{
	char	temp[500]={0};

	sprintf(temp, 			   "\r\n");
	sprintf(temp+strlen(temp), "                        *,] *                                   *,] *                         ]`* \r\n");                         
	sprintf(temp+strlen(temp), "                        *=@@@@]  *                   ,/@`        @@@@@]`  *          *  ,]@@@@@^* \r\n");                         
	sprintf(temp+strlen(temp), "                          =@@@@@@@@@@]]]]]]]]]]/@@@@@@@@^       *,@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \r\n");                           
	sprintf(temp+strlen(temp), "                          **\\@@@@@@@@@@@@@@@@@@@@@@@@@@/         *,@@@@@@@@@@@@@@@@@@@@@@@@@@` \r\n");                            
	sprintf(temp+strlen(temp), "                            *=@@@@@@@@@@@@@@@@@@@@@@@[ *           */@@@@@@@@@@@@@@@@@@@@/` *\r\n");
	BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(1);
	sprintf(temp,			   "                             @@@@@@@@@@@@@@@@@@[[  *                @@@@@@@@@@@@@@@[[  * \r\n");                                  
	sprintf(temp+strlen(temp), "                             @@@@@@@@@@@@@@@                        @@@@@@@@@@@@@@^\r\n");                                        
	sprintf(temp+strlen(temp), "                             @@@@@@@@@@@@@@`*                      *=@@@@@@@@@@@@@ \r\n");                                        
	sprintf(temp+strlen(temp), "                             *\\@@@@@@@@@@@**                        *,@@@@@@@@@@/\r\n");                                          
	sprintf(temp+strlen(temp), "                                ,\\@@@@/`**                            *,\\@@@@@[ *\r\n"); 
	BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(5);
	sprintf(temp, 			   "\r\n");                                                      
	sprintf(temp+strlen(temp), "\r\n"); 
	sprintf(temp+strlen(temp), "\r\n");
	sprintf(temp+strlen(temp), "\r\n");
	sprintf(temp+strlen(temp), "\r\n");
	sprintf(temp+strlen(temp), "\r\n");
	sprintf(temp+strlen(temp), "\r\n");
	sprintf(temp+strlen(temp), "                                                        * ,]]]]\r\n");                                                            
	sprintf(temp+strlen(temp), "                                                      ,/@@@@@@@@@@`\r\n");                                                        
	sprintf(temp+strlen(temp), "                                                  **/@@@@@@@@@@@@@@@\\\r\n");                                                      
	sprintf(temp+strlen(temp), "                                                  /@@@@@[[[    ,[[@@@@\r\n");                                                     
	sprintf(temp+strlen(temp), "                                                 ,[[ *               * \r\n");
	sprintf(temp+strlen(temp), "\r\n");
	BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(10);					//延时要够，否则会异常
	
//	sprintf(temp,"\r\n编译时间：%s %s\r\n",__DATE__,__TIME__);
//	BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(1);						
}

/*******************************************************************************
名称：void HardwareTest(void)
功能：用于测温基站、微气象基站等的整机硬件测试。
入参：无
出参：无
返回：无
*******************************************************************************/
void HardwareTest(void)
{
	INT8U		i;
	INT16U		DS;//DS18B20
	INT8U		Temp[10];
	INT8U*		pointer;
	struct HARDWARE_TEST	hardware = {0};										//存放各硬件检测结果，1表示通过，0为异常
	
	Led_Init();
 
	while(1)
	{
	/*485测试，在B485_init中返回结果*/
		hardware.b485 = 1;														//默认为1，有问题的话自然打不出来东西
		BspUartWrite(2,SIZE_OF("\r\n485_Test    ------------------->485_OK\r\n"));OSTimeDly(1);

#if 1  //通用测试（含铁电，WQ256，时钟，电源电压，单片机温度测试）
	/*铁电测试*/
		BspUartWrite(2,SIZE_OF("FM_Test     "));OSTimeDly(1);
		if(FM_test()==1)
		{
			hardware.ferroelectric_ram = 1;
			BspUartWrite(2,SIZE_OF("------------------->FM_OK\r\n"));
		}
		else 
			BspUartWrite(2,SIZE_OF("------------------->FM_ERR\r\n"));
		OSTimeDly(1);	
		
	/* WQ256测试*/
		BspUartWrite(2,SIZE_OF("WQ256_Test  "));OSTimeDly(1);
		if( WQ256_Test(3)==1)
		{
			hardware.flash_memory = 1;
			BspUartWrite(2,SIZE_OF("------------------->WQ256_OK\r\n"));
		}
		else
			BspUartWrite(2,SIZE_OF("------------------->WQ256_ERR\r\n"));		
		OSTimeDly(1);

	/*时钟测试*/	
		BspUartWrite(2,SIZE_OF("RTC_Test    -------------------\r\n"));OSTimeDly(1);			
		hardware.rtc = RTCTaskTest();
		BspUartWrite(2,SIZE_OF("\r\n"));

	/*电池电压、单片机温度测试*/
		BspUartWrite(2,SIZE_OF("VCC_Test    -------------------\r\n"));OSTimeDly(1);
		Get_Voltage_MCUtemp_Data(3);
		BspUartWrite(2,SIZE_OF("\r\n"));
		if(Equipment_state.BAT_Volt>BAT_UP || Equipment_state.FALA_Volt>FALA_UP)//保险点，用上限
		{
			hardware.power_supply = 1;
		}
#endif
#if 1 //1：基站测温测试（含主板温度，射频接收模块测试）   0：基站微气象测试（含风速风向，温湿度，大气压测试）    //PS：如果插了18B20跑了AM2302的驱动（在微气象测试中）会导致系统奔溃！
	/*DS18B20测试*/
		BspUartWrite(2,SIZE_OF("DS18B20_Test"));								//DS18B20
		OSTimeDly(1);
		if(Get_DS18B20Temp(&DS))
		{
			BspUartWrite(2,SIZE_OF("------------------->DS18B20_OK\r\n"));
			hardware.ds18b20 = 1;
			Temp[0]=(DS/100+0x30);//取百位
			Temp[1]=((DS/10%10)+0x30);//取各位
			Temp[2]='.';
			Temp[3]=((DS%10)+0x30);//小数
			BspUartWrite(2,SIZE_OF("Temperature = "));
			BspUartWrite(2,Temp,4);
			BspUartWrite(2,SIZE_OF("℃\r\n"));
		}
		else
		{
			BspUartWrite(2,SIZE_OF("------------------->DS18B20_Err\r\n"));
		}
		OSTimeDly(1);

	/*射频测试*/
		RF_Power_Init();       													//RF模块电源初始化
		PWRFEN();				 												//打开电源（常开）
		RF_Uart_init(1200);														//初始化RF串口
		BspUartWrite(2,SIZE_OF("\r\n"));
		BspUartWrite(2,SIZE_OF("RF_Test     -------------------\r\n"));OSTimeDly(1);
		if(RfModuleTest())
		{	
			hardware.lora = 1;
			BspUartWrite(2,SIZE_OF("------------------------------->RF_OK\r\n"));
		}
		else
		{
			BspUartWrite(2,SIZE_OF("------------------------------->RF_Err\r\n"));
		}
		OSTimeDly(1);
#else
	/*微气象测试*/
		PowerMETPin_Init();														//温湿度传感器初始化
		PWMETEN();																//打开温湿度传感器电源 
		hardware.meteorology = !Test_Meteorology_Data(1);    					//反回0表示无错误
#endif		

	/*华为模块测试*/
		BspUartWrite(2,SIZE_OF("LTE_Test    "));
		OSTimeDly(1);
		if(ME909S_TEST())
		{
			hardware.lte = 1;
			BspUartWrite(2,SIZE_OF("LTE_Test    ------------------->LTE_OK\r\n"));
			OSTimeDly(1);	
		}
		else
		{
			BspUartWrite(2,SIZE_OF("LTE_Test    ------------------->LTE_ERR\r\n"));
			OSTimeDly(1);	
		}
		
#if 0		
	/*加密测试*/
		BspUartWrite(2,SIZE_OF("JM_Test     "));OSTimeDly(1);
		
		if(NAREC300_Test()==1)
		{
			hardware.encryption_chip = 1;
			BspUartWrite(2,SIZE_OF("------------------->JM_Test_OK\r\n"));
		}
		else
		{
			BspUartWrite(2,SIZE_OF("------------------->JM_Test_ERR\r\n"));
		}
		OSTimeDly(1);
#endif

	/*上述硬件全部测试通过时，才进行看门狗测试*/
		if(hardware.b485 & hardware.ferroelectric_ram & hardware.flash_memory & hardware.rtc & hardware.power_supply & hardware.ds18b20 & hardware.lora & hardware.lte)
		{
		/*看门狗测试*/
			BspUartWrite(2,SIZE_OF("WDG_Test\r\n"));
			BspUartWrite(2,SIZE_OF("if Systerm restart,Test is OK\r\n"));		//如果重启则测试通过，结束测试

			PWR->CR |= 1<<8;													//DBP位：取消后备区域的写保护。1：允许写入RTC和后备寄存器
			BKP->DR2 = 0xCC;													//设初值为0xCC，10秒后看门狗若还未复位，设为0，再次上电还进系统2（即本测试程序）
			PWR->CR &= ~(1<<8);													//启用后备区域的写保护
			WDG_En = false;														//禁止init_task_core()中外部看门狗喂狗
			BSP_WDGDeInit();													//禁用外部看门狗喂狗（光上面那个不行，原因未知）
			
			while(1)
			{
				OSTimeDly(10*20);												//延时10秒【不可随意修改】
				BspUartWrite(2,SIZE_OF("若不复位，则外部看门狗异常\r\n"));
				
				PWR->CR |= 1<<8;												//DBP位：取消后备区域的写保护。1：允许写入RTC和后备寄存器			
				BKP->DR2 = 0;													//设为0，下次继续进本硬件测试系统（若10秒内复位，测试通过，以后都进系统1）
				PWR->CR &= ~(1<<8);												//启用后备区域的写保护（看门狗复位会自动保护）
			}			
		}
		
	/*上述硬件测试不通过，打印检测结果*/
		DrawErrLogo();
		
		pointer = (INT8U*)&hardware;
		for(i=0;i<10;i++)														//目前共检测8个模块
		{
			if(*(pointer+i)==0)													//为0，表示测试未通过
			{
				switch(i)
				{
					case 0:
						BspUartWrite(2,SIZE_OF("不合格：485功能\r\n\r\n"));
						break;
					case 1:
//						BspUartWrite(2,SIZE_OF("不合格：加密功能\r\n\r\n"));
						break;
					case 2:
						BspUartWrite(2,SIZE_OF("不合格：铁电存储功能\r\n\r\n"));
						break;
					case 3:
						BspUartWrite(2,SIZE_OF("不合格：外部FLASH功能\r\n\r\n"));
						break;
					case 4:
						BspUartWrite(2,SIZE_OF("不合格：RTC功能\r\n\r\n"));
						break;
					case 5:
						BspUartWrite(2,SIZE_OF("不合格：电源电压功能\r\n\r\n"));
						break;
					case 6:
						BspUartWrite(2,SIZE_OF("不合格：DS18B20功能\r\n\r\n"));
						break;
					case 7:
						BspUartWrite(2,SIZE_OF("不合格：LORA射频接收功能\r\n\r\n"));
						break;
					case 8:
//						BspUartWrite(2,SIZE_OF("不合格：微气象功能\r\n\r\n"));
						break;
					case 9:
						BspUartWrite(2,SIZE_OF("不合格：LTE功能\r\n\r\n"));
						break;
					default:	
						BspUartWrite(2,SIZE_OF("程序异常！\r\n"));
						break;
				}
				OSTimeDly(1);
			}				
		}
		while(1) OSTimeDly(3*20);
	}
}



#if 1 /*============================================================FATFS应用函数============================================================*/
/*******************************************************************************
名称：FRESULT scan_files (char* path)
功能：扫描文件夹，打印所有文件夹、子文件夹、文件名。
入参：char* path，要扫描的路径
出参：无
返回：FRESULT
*******************************************************************************/
FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;


    res = f_opendir(&dir, path);                       							/* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                  						/* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  						/* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    						/* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
				/*打印目录名*/
				BspUartWrite(2,(INT8U*)path,strlen(path));
//				BspUartWrite(2,(INT8U*)fno.fname,strlen(fno.fname));
				BspUartWrite(2,SIZE_OF("\r\n"));
                res = scan_files(path);                    						/* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       						/* It is a file. */
				/*打印文件名*/
//				printf("%s/%s\n", path, fno.fname);
				BspUartWrite(2,(INT8U*)path,strlen(path));
				BspUartWrite(2,SIZE_OF("/"));
				BspUartWrite(2,(INT8U*)fno.fname,strlen(fno.fname));
				BspUartWrite(2,SIZE_OF("\r\n"));
            }
			OSTimeDly(1);
        }
        f_closedir(&dir);
    }

    return res;
}

/*******************************************************************************
名称：FRESULT Dir_Maintenance (void)
功能：目录维护。固定生成SUB1~SUB31共31个文件夹。
入参：无
出参：无
返回：FRESULT
*******************************************************************************/
FRESULT Dir_Maintenance (void)
{
	static FILINFO fno;
	static FRESULT res;															/* API result code */
	UINT i,fnlen=0;
	DIR dir;
	char path[256];
	
	BspUartWrite(2,SIZE_OF("\r\n->开始目录维护<-\r\n"));OSTimeDly(1);
	res = f_mount(&fs, "", 1);	/* Mode option 0:Do not mount (delayed mount), 1:Mount immediately */
	if (res == FR_OK) res = f_opendir(&dir, "/");                       		//进入根目录
	else if(!Fault_Manage.F_STORAGE)											//挂载失败
	{
		NW_Fault_Manage(STORAGE_F, FAULT_STA);									//存储故障发生时处理流程
		BspUartWrite(2,SIZE_OF("挂载失败！<-\r\n"));OSTimeDly(1);
		return res;
	}
	if(Fault_Manage.F_STORAGE) NW_Fault_Manage(STORAGE_F, NOFAULT_STA);			//发生过存储故障，上报故障恢复
    if (res == FR_OK) 
	{
	/*目录维护*/
		for(i=1;i<32;i++)														//遍历31个目录
		{	
		/*生成目录名*/
			sprintf(path, "/SUB%d", i);											//创建目录名到*path
			fnlen = strlen(path)-1;												//目录名的长度，-1指“/”
			
		/*判断目录是否存在*/
			res = f_readdir(&dir, &fno);                   						/* Read a directory item */
			if (res != FR_OK ) 				  									/* Break on error */
			{
				BspUartWrite(2,SIZE_OF("f_readdir()读取目录失败，维护中止\r\n"));OSTimeDly(1);
				return res;
			}
			else if (fno.fname[0] == 0) 				  						/* End of dir */
			{
			/*目录不存在，创建新目录*/
				res = f_mkdir(path);											//创建目录
				BspUartWrite(2,SIZE_OF("创建子目录："));
				BspUartWrite(2,(INT8U*)path+1,fnlen);							/*打印目录名*/				
				if (res != FR_OK) BspUartWrite(2,SIZE_OF("失败\r\n"));
				else BspUartWrite(2,SIZE_OF("成功\r\n"));
				OSTimeDly(1);
			}
			else if (fno.fattrib & AM_DIR)                   					/* It is a directory */
			{  
			/*找到一个目录，判断目录是否为所需并处理*/
				if(memcmp(&fno.fname,path+1,fnlen)) 							//若目录不为SUBi
				{
				/*目录不对，删掉并新建目录*/
					f_unlink(fno.fname);										//移除当前目录
					BspUartWrite(2,SIZE_OF("创建子目录："));
					BspUartWrite(2,(INT8U*)path+1,fnlen);						/*打印目录名*/				
					res = f_mkdir(path);										//创建目录
					if(res==FR_EXIST) BspUartWrite(2,SIZE_OF("已存在（被其他目录插队了）\r\n"));
					else if(res != FR_OK) BspUartWrite(2,SIZE_OF("失败\r\n"));
					else  BspUartWrite(2,SIZE_OF("成功\r\n"));
				} else 															
				{	
				/*目录已存在*/
					BspUartWrite(2,SIZE_OF("子目录："));
					BspUartWrite(2,(INT8U*)fno.fname,strlen(fno.fname));		/*打印目录名*/
					BspUartWrite(2,SIZE_OF("已存在\r\n"));
				}
				OSTimeDly(1);
			} else i--;															//是文件则跳过
		}
		f_closedir(&dir);
	}
	/* Unregister work area */
    f_mount(0, "", 0);
	BspUartWrite(2,SIZE_OF("->目录维护结束<-\r\n\r\n"));OSTimeDly(1);
	return res;
}

/*******************************************************************************
名称：void Dir_Test(void)
功能：目录测试。
入参：无
出参：无
返回：无
*******************************************************************************/
void Dir_Test(void)
{
	TCHAR	chars[256];
	FRESULT res;																/* API result code */
	
    res = f_mount(&fs, "", 1);	/* Mode option 0:Do not mount (delayed mount), 1:Mount immediately */
//    if (res == FR_OK) {
//		/*创建目录*/
//		res = f_mkdir("sub1");
//		if (res) {BspUartWrite(2,SIZE_OF("创建sub1子目录失败\r\n"));OSTimeDly(1);}
//		res = f_mkdir("sub1/sub2");
//		if (res) {BspUartWrite(2,SIZE_OF("创建sub1/sub2子目录失败\r\n"));OSTimeDly(1);}
//		res = f_mkdir("sub1/sub2/sub3");
//		if (res) {BspUartWrite(2,SIZE_OF("创建sub1/sub2/sub3子目录失败\r\n"));OSTimeDly(1);}
//		if (res == FR_OK) {
//			/*在"/sub1/sub2/sub3"目录下查找文件*/
//			strcpy(chars, "/sub1/sub2/sub3");
//			res = scan_files(chars);
//		 }
//	 }

    if (res == FR_OK) {		
		/*在"/"目录下查找文件*/
        strcpy(chars, "");
        res = scan_files(chars);
    }
	
	/* Unregister work area */
    f_mount(0, "", 0);
	
	BspUartWrite(2,SIZE_OF("Dir_Test()测试结束\r\n"));OSTimeDly(1);
}

/*******************************************************************************
名称：void Check_Getfree(void)
功能：检查是否需要格式化并执行。包含新设备写入Config、Unreport_Index出厂默认参数。
将外部FLASH未格式化的设备认定为新设备。
入参：无
出参：无
返回：无
*******************************************************************************/
void Check_Getfree(void)
{
	FRESULT				res;																/* API result code */
	TCHAR				chars[256];
	static DWORD		fre_clust, fre_sect, tot_sect;
	INT32U				id = 0;
	
    /* Register work area */
    res = f_mount(&fs, "", 0);
	if (res) BspUartWrite(2,SIZE_OF("f_open() 挂载失败！\r\n"));
	
    /* Get volume information and free clusters of drive 1 */
    res = f_getfree("", &fre_clust, &fs0);

    if (res)	//res
	{
		BspUartWrite(2,SIZE_OF("f_getfree() 错误，正在格式化外部FLASH……\r\n"));
		/* Create FAT volume */
		res = f_mkfs(	"", 													// If it has no drive number in it, it means the default drive.
						FM_FAT|FM_SFD, 											// Specifies the format option in combination of FM_FAT, FM_FAT32, FM_EXFAT and bitwise-or of these three, FM_ANY. If two or more types are specified, one out of them will be selected depends on the volume size and au.
						4096, 													// The valid value is n times the sector size.
						work, 													// Pointer to the working buffer used for the format process
						sizeof work);											// It needs to be the sector size of the corresponding physical drive at least.
		if (res) BspUartWrite(2,SIZE_OF("f_mkfs() 格式化错误！\r\n"));
		else BspUartWrite(2,SIZE_OF("f_mkfs() 格式化成功！\r\n"));
		
//		/*擦除原有系统参数配置——铁电不能擦除！也无需擦除！*/
//		BSP_FM_Erase(FM_Start_Addr,0x400*8);									//擦除整个铁电储存
//		BspUartWrite(2,SIZE_OF("铁电擦除成功！\r\n"));
		
		/*读取滚码作为装置号码*/
		Read_NFlash(Device_Number_Flash_Addr, &id, 1);							//读取滚码，作为初始装置号码
		NW_DeviceNumberToAscii(id, &Device_Number[2]);							//把16进制转为ASCII，存入Device_Number[2]
		BSP_WriteDataToFm(Device_Number_Addr,Device_Number,Device_Number_Len);	//装置号码写入铁电
		
		/*新设备写入Config、Unreport_Index出厂默认参数*/
		BSP_WriteDataToFm(Config_Addr,(u8*)&Config,Config_Len);					//写一次默认设置，后续上位机联机修改写入铁电之后，每次上电读出来的才是正确的，不然首次读将全是0
		BSP_WriteDataToFm(IP_Config_Addr,(u8*)&IP_Config,IP_Config_Len);		//写一次自定义的IP默认设置
		BSP_WriteDataToFm(APN_Addr, APN, APN_Len);								//写一次自定义的APN默认设置
		BSP_WriteDataToFm(FUN_Config_Addr,FUN_Config,FUN_Config_Len);			//写默认功能配置参数
		memset(Unreport_Index,0xFF,Unreport_Index_Len);							//写FF
		BSP_WriteDataToFm(Unreport_Index_Addr,(u8*)Unreport_Index,Unreport_Index_Len);	//未上报数据索引表全写成已上报（所有位写1）
		BspUartWrite(2,SIZE_OF("铁电写出厂配置成功！\r\n"));
	}
	
	/* Get total sectors and free sectors */
	tot_sect = (fs0->n_fatent - 2) * fs0->csize;
	fre_sect = fre_clust * fs0->csize;
	
	/* Print the free space (assuming 4K bytes/sector) */
    sprintf(chars,"外部FLASH：%10lu KiB total drive space.\n%10lu KiB available.  （%d%%USED）\r\n", tot_sect*4, fre_sect*4, 100*(tot_sect-fre_sect)/tot_sect);	
	BspUartWrite(2,(INT8U*)chars,strlen(chars));								//打印
	
	/* Unregister work area */
    f_mount(0, "", 0);
}

/*******************************************************************************
名称：void Dir_Test(void)
功能：打开关闭文件、读写文件测试。
入参：无
出参：无
返回：无
*******************************************************************************/
void File_Test(void)
{
	FRESULT res;																/* API result code */
	TCHAR	read_buff[256];
	UINT bw;																	/* Bytes written */	

	res = f_mount(&fs, "", 0);
	
    /* Create a file as new */
    res = f_open(&fil, "hello.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE );
    if (res) BspUartWrite(2,SIZE_OF("f_open() 打开文件错误！\r\n"));
	
    /* Write a message */
    f_write(&fil, "Hello, World!\r\n", 15, &bw);
    if (bw != 15) BspUartWrite(2,SIZE_OF("f_write() 写文件错误！\r\n"));
	
	/* File read/write pointer */
	res = f_lseek(&fil, 0);
	
    /* Read a message */
    f_read(&fil, read_buff, 100, &bw);
    if (bw != 15) BspUartWrite(2,SIZE_OF("f_read() 读文件错误！\r\n"));	

    /* Close the file */
    f_close(&fil);

    /* Unregister work area */
    f_mount(0, "", 0);
	
	BspUartWrite(2,SIZE_OF("File_Test()测试结束\r\n"));OSTimeDly(1);
}

#endif
/*============================================================FATFS应用函数 end============================================================*/
