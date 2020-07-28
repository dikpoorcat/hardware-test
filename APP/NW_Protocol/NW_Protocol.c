/***************************** (C) COPYRIGHT 2019 方诚电力 *****************************
* File Name          : NW_Protocol.c
* Author             : 杜颖成、陈威、等
* Version            : 见历史版本信息
* Date               : 2019/02/21
* Description        : 根据南网协议编写的通信功能函数。
************************************  历史版本信息  ************************************
* 2019/03/28    : V4.1.0
* Description   : 南网测温项目初版。基础功能完成，调试中。
*******************************************************************************/
#include "NW_Protocol.h"

/*全局变量*/
struct NW_CONFIG Config={
	{0x31,0x32,0x33,0x34},														//装置出厂密码：字符：‘1234’（31H32H33H34H）
	{0x01},																		//心跳间隔，出厂配置应为1分钟
	{0x00,0x14},																//采集间隔，出厂配置应为20分钟
	{0x00,0x37},																//休眠时长，可设置为55分钟
	{0x00,0x05},																//在线时长，可设置为5分钟
	{0x00,0x0A,0x1E},															//时间点格式：日，时，分	日：0到28日；（若日为00H则就每天定时重启）；时：0到23；分：0到59；
	{0x31,0x32,0x33,0x34}														//装置初始密文认证为字符‘1234’（31H32H33H34H）
};

/*------------------------------------------------------------
密码	主站IP	端口号	主站IP	端口号	主站卡号	主站卡号
4字节	4字节	2字节	4字节	2字节	6字节		6字节		
------------------------------------------------------------*/
struct NW_IP_CONFIG IP_Config={
	{118,190,141,140},															//出厂配置为118.190.141.140
	{0x1D,0xBB},																//出厂配置为7611
	{0x2F,0x64,0x23,0xC8},														//出厂配置为
	{0x17,0x71},																//出厂配置为
	{0x00,0x00,0x00,0x00,0x00,0x00},											//出厂配置为
	{0x00,0x00,0x00,0x00,0x00,0x00},											//出厂配置为
};

struct NW_FLOW_DATA Flow_Data={													//整数，单位MB
	{0x00,0x00,0x00,0x00,0x00,0x00},											//包采样时间（年+月+日+时+分+秒）（6字节）
	{0x00,0x00,0x00,0x00},														//当日已用流量（4字节）
	{0x00,0x00,0x00,0x00},														//当月已用流量（4字节）
	{MF_0,MF_1,MF_2,MF_3},														//当月剩余流量（4字节）默认MONTHLY_FLOW
};

struct NW_TEM_DATA Tem_Cur_Data={												//仅用于主站请求装置数据后立刻上传时组帧
	0,																			//帧标识（1字节）
	1,																			//包数（1字节）
	0,																			//功能单元识别码（1字节）
	{0x00,0x00,0x00,0x00,0x00,0x00},											//包采样时间（年+月+日+时+分+秒）（6字节）
	{0x00,0x00},																//测点温度（2字节）上送值=（实际温度+50）*10
	{0x00,0x00},																//导线电流（2字节）上送值=实际电流*10
	33,																			//传感器工作电压（1字节）上送值=实际电压*10
};

const INT8U Unit_ID_Code[55]={													//功能单元识别码表
	/*导线侧传感器20个*/
	0x15,	0x16,	0x17,	0x18,												//导线相位1
	0x25,	0x26,	0x27,	0x28,												//导线相位2
	0x35,	0x36,	0x37,	0x38,												//导线相位3
	0x45,	0x46,	0x47,	0x48,												//地线相位4
	0x55,	0x56,	0x57,	0x58,												//地线相位5
	/*保留位扩展35个*/
	0x19,	0x1A,	0x1B,	0x1C,	0x1D,	0x1E,	0x1F,						//导线相位1
	0x29,	0x2A,	0x2B,	0x2C,	0x2D,	0x2E,	0x2F,						//导线相位2
	0x39,	0x3A,	0x3B,	0x3C,	0x3D,	0x3E,	0x3F,						//导线相位3
	0x49,	0x4A,	0x4B,	0x4C,	0x4D,	0x4E,	0x4F,						//地线相位4
	0x59,	0x5A,	0x5B,	0x5C,	0x5D,	0x5E,	0x5F,						//地线相位5
};

struct LOCAL_FLOW_DATA Local_FLow_Data={0};										//本地进行流量统计的结构体
struct NW_FAULT_INFO Fault_Info[FI_NUM]={0};									//故障信息结构体数据，最多存储FI_NUM个故障。包括故障和故障恢复等所有未上报的
struct NW_FAULT_MANAGE  Fault_Manage={0};										//故障信息管理结构体，每种故障的标志位储存地方

INT8U 				Unreport_Index[31][3]={0};									//未上报数据索引表，31天，每小时1bit（最高bit代表0时）：1已上报，0未上报	注：新设备初始化时已全部写1
INT8U 				Device_Number[6]="FC0000";									//6Byte装置号码
INT8U 				FUN_Config[24]={0x26,0x30};									//24Byte功能配置参数，最多24项功能，默认26H导线温度、电流数据监测功能\30H设备故障自检功能	PS：现在暂不支持功能配置，固定开启这两个功能
INT8U				Time_Proofread = DONE;										//初始化为DONE：设备上电后只要RTC时间格式正常即可采集温度，防止电量不足时因无法校时而不采集
INT8U				Tem_Sampled[2] = {0};										//用于立即采集的温度数据组帧上报
INT8U				APN[100] = "CMIOT";											//南网扩展协议增加APN（最大不能超过100个字节）配置		"CMNET"	"CMMTM"	"CMIOTGZDWSCSPJK.GZ"






/*******************************************************************************
名称：void NW_Comm_Process(void)
功能：根据南网协议编写的通信流程，包括通信及功能执行等。调用时初始状态为开机联络态。
入参：无
出参：无
返回：1，功能正常执行；0，有异常
*******************************************************************************/
INT8U NW_Comm_Process(void)														
{
	INT8U			First_Time = 0xFF;											//调用时主动进入一次TEM_CUR_UPLOAD
	INT16U			WaitTime = 0;
	INT8U			STATE = START_UP;											//初始状态定为开机联络
	static INT8U	msg_cmd;
	static INT8U	msg_state;
	static INT8U	msg_fault;
	INT8U			Err = 0;
	
	while(1)
	{
		WDTClear(LTE_PRIO);														//清除任务看门狗
		switch(STATE)
		{
			case START_UP:
				/*判断是否要发送开机联络信息*/					
					if(Reset_Flag != NORST){									//若刚复位过（请勿随意修改此全局变量）
						BspUartWrite(2,SIZE_OF("\r\n---------->开机联络<----------\r\n"));
						if(!Startup_Comm(RETRY,TIMEOUT)) return 0;				//开机联络通信，失败时返回0
					}
					STATE = HEARTBEAT;											//开机联络通信成功，下一步心跳通信
					break;														
					
			case HEARTBEAT:
					BspUartWrite(2,SIZE_OF("\r\n---------->心跳上报<----------\r\n"));
					if(!Heartbeat(RETRY,TIMEOUT)) return 0;						//心跳通信，失败时返回0
				/*判断是否要发送故障信息*/
					if(Reset_Flag != NORST||Fault_Manage.Need_Report)			//若刚复位过（请勿随意修改此全局变量）  或有故障信息需要上报时，优先上报再进入等待指令
					{									
						Reset_Flag = NORST;										//赋值为NORST，用于非复位情况下跳过开机联络和故障上报。除非复位，否则不再改变此值
						STATE = FAULT_INFO;										//复位后需要上报故障状态代码
					}
					else STATE = REC_AND_EXE;									//心跳通信成功，下一步等待上位机指令并处理（一个心跳间隔时间）	不要直接进入QUERY_MAIL状态，有无限心跳风险
					break;
			
			case FAULT_INFO:
					BspUartWrite(2,SIZE_OF("\r\n---------->故障信息<----------\r\n"));
					if(!Fault_Info_Comm(RETRY,TIMEOUT)) return 0;				//故障信息通信，失败时返回0
					STATE = REC_AND_EXE;										//心跳通信成功，下一步等待上位机指令并处理（一个心跳间隔时间）	不要直接进入QUERY_MAIL状态，有无限心跳风险
					break;
			
			case REC_AND_EXE:
					BspUartWrite(2,SIZE_OF("\r\n---------->等待接收<----------\r\n"));
					WaitTime = *Config.BeatTime*60;								//计算延时时间，单位：秒
				/*保持在线状态WaitTime秒（用于心跳间隔，遇其他命令后顺延一个联络间隔）*/
					STATE = NW_ReceiveAndExecute(LTE_Rx_Buff,WaitTime);	//接收并执行成功返回下一个需要执行的状态（如重启/休眠/校时等），未执行任何操作则返回0
					if(STATE==1) STATE = REC_AND_EXE;							//STATE==1表示已经成功执行了某些功能，等待下一个指令，心跳顺延一个联络间隔
					else if(STATE==0) 	 										//返回0表示无需操作或操作失败，查询邮箱指令
					{
						if(First_Time) 
						{
							STATE = TEM_CUR_UPLOAD;								//初次调用时主动进入一次TEM_CUR_UPLOAD
							First_Time = 0;										//清0，之后不再主动上报
						}
						else STATE = QUERY_MAIL;								//不主动上报时，查询邮箱
					}	
					/*其他情况会自动根据STATE状态选择，如RESET_DEV等*/
					break;
					
			case TEM_CUR_UPLOAD:
					BspUartWrite(2,SIZE_OF("\r\n---------->历史数据<----------\r\n"));
					Err = Tem_Cur_Upload(RETRY,TIMEOUT);						//发送历史数据
					if(0xFF==Err) STATE = QUERY_MAIL;							//无历史数据，不二次等待
					else if(!Err) return 0;										//发送历史数据失败时返回0（仅表示还有历史数据未发送完成，不影响已完成的部分）
					else STATE = REC_AND_EXE;									//上报成功，下一步回到等待状态
					BspUartWrite(2,SIZE_OF("---------->历史数据上报结束<----------\r\n"));
					break;
			
			case SMS_AWAKE:														//主站若通过IP网络UDP通信，发送此控制字，也表示唤醒终端
					BspUartWrite(2,SIZE_OF("\r\n---------->唤醒指令<----------\r\n"));
					msg_state = WAKE_SUCCESS;
					OSMboxPost(Dev_STAB0X, &msg_state);							//通过邮箱将状态变化命令发出，以重置在线时间
					STATE = REC_AND_EXE;										//立即切换到在线状态，等效于回到等待状态
					break;	
			
			case SLEEP_NOTICE:
					BspUartWrite(2,SIZE_OF("\r\n---------->休眠通知<----------\r\n"));
					Sleep_Notice();												//该信息在装置设备每次休眠之前上报主站
					return SLEEP_NOTICE;										//返回，进入休眠
			
			case RESET_DEV:
					BspUartWrite(2,SIZE_OF("\r\n---------->装置重启<----------\r\n"));
					return RESET_DEV;											//返回，重启装置
									
			case QUERY_MAIL:
					/*优先处理故障信息*/
					msg_fault = *(INT8U *)OSMboxPend(Fault_CMDB0X,1,&Err);		//进行邮箱查询，故障信息
					if(msg_fault==FAULT_CMD) 									//需要上报
					{
						STATE = FAULT_INFO;										//进入故障信息上报状态
						break;													
					}
					/*等待休眠指令*/
					msg_cmd = *(INT8U *)OSMboxPend(Dev_CMDB0X,1,&Err);			//进行邮箱查询（只有WAKE和SLEEP两种命令，消息被清了也不用重发）
					if(msg_cmd==SLEEP_CMD) STATE = SLEEP_NOTICE;				//进入休眠状态
//					else if(msg==UPLOAD_CMD) STATE = TEM_CUR_UPLOAD;			//进入温度电流发送状态（特殊上报可以做这里）
					else STATE = HEARTBEAT;										//无需要动作的指令。心跳间隔到期，该上心跳了
					break;
			
			default:
					BspUartWrite(2,SIZE_OF("\r\n!!!!!!!!!!!!!!!!!!!!>警告！NW_Comm_Process()进入未知状态！<!!!!!!!!!!!!!!!!!!!!\r\n"));
					return 0;
		}
	}	
}

/*******************************************************************************
名称：INT8U Startup_Comm(u8 times, u16 timeout)
功能：开机联络，重试times次，控制字：00H
装置每次发送开机联络信息，主站无返回信息则每1分钟发送一次直到收到主站返回信息。
入参：u8 times，重试次数；u16 timeout，超时等待时间，单位：时间片
出参：无
返回：开机联络并校时成功返回1，失败返回0
*******************************************************************************/
INT8U Startup_Comm(u8 times, u16 timeout)
{
	INT16U	len_frame=0;														//帧长度
	INT8U	i=0;
	INT16U	len=0;
	
	len_frame=NW_Framing(START_UP,LTE_Tx_Buff);									//组开机联络帧
	for(i=0;i<times;i++)														//最多重试times次	装置每次发送开机联络信息，主站无返回信息则每1分钟发送一次直到收到主站返回信息。
	{
		len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,timeout);		//返回为0表示通信失败，返回0xff表示没接收到数据，其他返回为接收数据长度
		if(!len) 
		{
			BspUartWrite(2,SIZE_OF("未收到主站回复！\r\n"));
			if(i<times-1) OSTimeDly(1*3*20-timeout);							//通信失败时每隔3秒请求一次
			continue;															//重试
		}
		if(Judge_NW_Framing(START_UP,LTE_Rx_Buff,len,0))						//判断接收内容是否符合南网协议，不符合时重新通信
		{
			OSTimeDly(10);														//现在的后台，开机联络完成后，校时有时回复慢，加个延时就好了
			/*接收到回复后，主动请求校时*/
			return Timming_Request(10,20);/*timeout根据协议固定为20秒*/			//监测装置收到主站开机联络返回信息后主动请求校时，每隔2分钟请求一次直到校时成功为止，若请求时间与主站应答之间延时不超过20秒，则接受该命令，更改装置时钟。
		}
	}
	BspUartWrite(2,SIZE_OF("Startup_Comm重试超次数，开机联络失败！\r\n"));
	return 0;																	//重试超次数、等待超时都返回0
}

/*******************************************************************************
名称：INT8U Timming_Request(u8 times, u16 timeout)
功能：主动请求校时，重试times次，控制字：01H
监测装置收到主站开机联络返回信息后主动请求校时，每隔2分钟请求一次直到校时成功为止，若请求
时间与主站应答之间延时不超过20秒，则接受该命令，更改装置时钟。
入参：u8 times，重试次数；u16 timeout，超时等待时间，单位：时间片  本工程为50ms（注意，根
据协议要求不超过20秒时接受该命令，故timeout<=20*20）
出参：无
返回：校时成功返回1，失败返回0
*******************************************************************************/
INT8U Timming_Request(u8 times, u16 timeout)
{
	INT16U	len_frame=0;														//帧长度
	INT8U 	*R;
	INT8U	i=0;
	INT16U	len=0;
	
	len_frame=NW_Framing(TIMMING,LTE_Tx_Buff);									//组校时请求帧
	for(i=0;i<times;i++)														//最多重试times次
	{
		len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,timeout);		//返回为0表示通信失败，返回0xff表示没接收到数据，其他返回为接收数据长度
		if(!len) 
		{
			BspUartWrite(2,SIZE_OF("未收到主站回复！\r\n"));
			if(i<times-1) OSTimeDly(2*60*20-timeout);							//通信失败时每隔2分钟请求一次
			continue;															//重试
		}
		R = Judge_NW_Framing(TIMMING,LTE_Rx_Buff,len,0);						//判断接收内容是否符合南网协议
		if(R) 																	//不符合协议时重新通信
		{
			/*只要timeout设置不超过20秒，都符合协议要求，接受该命令，更改装置时钟*/
			return SetTime(R);													//更改RTC，接收到的内容在*R，成功返回1		
		}
	}
	BspUartWrite(2,SIZE_OF("Timming_Request重试超次数，校时失败！\r\n"));
	return 0;																	//重试超次数、等待超时都返回0
}

/*******************************************************************************
名称：INT8U *Heartbeat(u8 times, u16 timeout)
功能：发送心跳，重试times次；u16 timeout，超时等待时间，单位：时间片
入参：u8 times，重试次数
出参：无
返回：0 通信失败；非0 通信成功，且返回数据包首字节地址 INT8U *
*******************************************************************************/
INT8U *Heartbeat(u8 times, u16 timeout)
{
	INT16U	len_frame=0;														//帧长度
	INT8U	i=0;
	INT16U	len=0;
	INT8U	*R=0;
	
	/*南网协议心跳*/
	len_frame=NW_Framing(HEARTBEAT,LTE_Tx_Buff);								//组心跳帧
	for(i=0;i<times;i++)														//最多重试times次
	{	
		len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,timeout);		//返回为0表示通信失败，返回0xff表示没接收到数据，其他返回为接收数据长度
		if(!len) 
		{
			BspUartWrite(2,SIZE_OF("未收到主站回复！\r\n"));
			continue;															//通信失败时重试
		}
		R = Judge_NW_Framing(HEARTBEAT,LTE_Rx_Buff,len,0);						//判断接收内容是否符合南网协议，并返回结果
		break;
	}
	
	/*扩展协议心跳*/
	len_frame=NW_Framing(EX_HEARTBEAT,LTE_Tx_Buff);								//组扩展心跳帧
	for(i=0;i<times;i++)														//最多重试times次
	{	
		len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,timeout);		//返回为0表示通信失败，返回0xff表示没接收到数据，其他返回为接收数据长度
		if(!len) 
		{
			BspUartWrite(2,SIZE_OF("未收到主站回复！\r\n"));
			continue;															//通信失败时重试
		}
		break;
	}
	
	if(!R) BspUartWrite(2,SIZE_OF("Heartbeat重试超次数，心跳通信失败！\r\n"));
	return R;																	//重试超次数、等待超时都返回0
}

/*******************************************************************************
名称：INT8U *Fault_Info_Comm(u8 times, u16 timeout)
功能：发送故障信息，重试times次；u16 timeout，超时等待时间，单位：时间片
三种情况上报故障信息：开机联络+心跳完成后、新故障发生或恢复时、通信故障恢复时。
入参：u8 times，重试次数
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U Fault_Info_Comm(u8 times, u16 timeout)
{
	INT8U			i = 0;
	INT16U			len_frame,len;		
	INT8U			*R;

	/*组帧*/
	len_frame = NW_Framing(FAULT_INFO, LTE_Tx_Buff);							//组FAULT_INFO帧
	
	/*上报并等回复，重试times次*/
	for(i=0;i<times;i++)														//最多重试times次
	{
		len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,timeout);		//返回为0表示通信失败，返回0xff表示没接收到数据，其他返回为接收数据长度
		if(!len)
		{
			BspUartWrite(2,SIZE_OF("未收到主站回复！\r\n"));
			if(i<times-1) OSTimeDly(2*60*20-timeout);							//通信失败时每隔2分钟请求一次
			continue;															//重试
		}
		R = Judge_NW_Framing(FAULT_INFO,LTE_Rx_Buff,len,0);						//判断接收内容是否符合南网协议
		if(R) 																	//符合时进入，不符合协议时重新通信
		{
			if(R[10]!=1) continue;												//判断帧标识 1
			if(R[11]!=0xAA) continue;											//判断回复数据域 AA55H
			if(R[12]!=0x55) continue;											//判断回复数据域	
			
			Fault_Manage.Need_Report=0;
			if(!BSP_WriteDataToFm(Fault_Manage_Addr,(INT8U*)&Fault_Manage,Fault_Magage_Len)) return 0;		//清除故障管理结构体的“需要上报标志位”
			
			memset(Fault_Info,0,Fault_Info_Len);															//清结构体数组
			if(!BSP_WriteDataToFm(Fault_Info_Addr,(INT8U*)Fault_Info,Fault_Info_Len)) return 0;				//清除铁电中Fault_Info故障信息结构体数组（把上句清的结果写进去）
			
			wakeup_en.overtime = true;											//允许唤醒LTE休眠
			return 1;															//上报成功，返回1
		}
	}
	
	/*通信超时*/
	wakeup_en.overtime = false;													//主站未回复故障，禁止唤醒LTE休眠
	BspUartWrite(2,SIZE_OF("Fault_Info_Comm重试超次数，故障信息通信失败！\r\n"));
	return 0;																	//重试超次数，通信失败，返回0				
}

/*******************************************************************************
名称：INT8U Set_Password_Comm(u8 *InBuff, u16 Len)
功能：包括密码检验、回复通信、设置密码功能执行
入参：u8 *InBuff，传入的内容，正常为接收到的帧；u16 Len，长度
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U Set_Password_Comm(u8 *InBuff, u16 Len)
{
	if(!PassworkCheckAndReport(InBuff));										//判断主站下发报文中密码是否正确，不正确时上报密码出错信息。
	else																		//若相同，密码正确
	{
		memcpy(Config.Password,InBuff+14,4);									//将密码更改为新密码，写入Config结构体（默认成功）
		BSP_WriteDataToFm(Config_Addr,&Config.Password[0],Config_Len);			//定入铁电
		LteCommunication(InBuff,Len,0,0);										//配置成功，按照原命令返回，不接收（LteCommunication返回0）
		return 1;
	}
	BspUartWrite(2,SIZE_OF("Set_Password_Comm设置密码通信失败！\r\n"));
	return 0;
}

/*******************************************************************************
名称：INT8U ParaConfigComm(u8 *InBuff, u16 Len)
功能：包括密码检验、回复通信、参数配置功能执行
入参：u8 *InBuff，传入的内容，正常为接收到的帧；u16 Len，长度
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U ParaConfigComm(u8 *InBuff, u16 Len)
{
	if(!PassworkCheckAndReport(InBuff));										//判断主站下发报文中密码是否正确，不正确时上报密码出错信息。
	else																		//若相同，密码正确
	{
	/*不能为0的设置被误设为0时，采用原设置*/
		if(!InBuff[14]) InBuff[14]=Config.BeatTime[0];							//心跳间隔，不能为0，采用原设置
		if(!(InBuff[15]|InBuff[16])) memcpy(InBuff+15,Config.ScanInterval,2);	//采集间隔，不能为0，采用原设置
		/*17、18 休眠时长可以为0*/
		if(!(InBuff[19]|InBuff[20])) memcpy(InBuff+19,Config.OnlineTime,2);		//在线时长，不能为0？，采用原设置
		/*21、22、23 硬件重启时间点可以为0*/
		/*24、25、26、27 密文验证码可以为0*/
		memcpy(&Config.BeatTime,InBuff+14,14);									//参数配置，写入Config结构体（默认成功）
		BSP_WriteDataToFm(Config_Addr,(u8 *)&Config,Config_Len);				//定入铁电
		LteCommunication(InBuff,Len,0,0);										//配置成功，按照原命令返回，不接收（LteCommunication返回0）
		return 1;
	}
	BspUartWrite(2,SIZE_OF("ParaConfigComm参数配置通信失败！\r\n"));
	return 0;
}

/*******************************************************************************
名称：INT8U Set_IP_Comm(u8 *InBuff, u16 Len)
功能：包括密码检验、回复通信、设置IP参数功能执行
入参：u8 *InBuff，传入的内容，正常为接收到的帧；u16 Len，长度
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U Set_IP_Comm(u8 *InBuff, u16 Len)
{
	INT16U	len_frame=0;														//帧长度
	
	if(!PassworkCheckAndReport(InBuff));										//判断主站下发报文中密码是否正确，不正确时上报密码出错信息。
	else if(memcmp(InBuff+14,InBuff+20,6) || memcmp(InBuff+26,InBuff+32,6))		//若两组主站IP、端口号和主站卡号对应字节不完全相同（相同时memcmp返回0）
	{
		len_frame=NW_Framing(DATA_UNCORRESPOND,LTE_Tx_Buff);					//组出错信息帧0（数据域为0000H）
		LteCommunication(LTE_Tx_Buff,len_frame,0,0);							//上报出错信息0，不接收（LteCommunication返回0）
	}
	else																		//若相同，密码正确
	{
		memcpy(&IP_Config,InBuff+14,24);										//IP参数配置，写IP_Config结构体（默认成功）
		BSP_WriteDataToFm(IP_Config_Addr,&IP_Config.IP_addr_1[0],IP_Config_Len);//定入铁电
		LteCommunication(InBuff,Len,0,0);										//配置成功，按照原命令返回，不接收（LteCommunication返回0）
		return 1;
	}
	BspUartWrite(2,SIZE_OF("Set_IP_Comm设置IP通信失败！\r\n"));
	return 0;
}

/*******************************************************************************
名称：INT8U Fun_Config_Comm(u8 *InBuff, u16 Len)
功能：包括密码检验、回复通信、功能配置
入参：u8 *InBuff，传入的内容，正常为接收到的帧；u16 Len，长度
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U Fun_Config_Comm(u8 *InBuff, u16 Len)
{
	INT16U	len_frame=0;														//帧长度/或用于计算数据域长度
	
	if(!PassworkCheckAndReport(InBuff));										//判断主站下发报文中密码是否正确，不正确时上报密码出错信息。
	else																		//若相同，密码正确
	{
		len_frame = ((INT16U)InBuff[8]<<8) + InBuff[9] -4;						//计算有效功能数（保险起见，进行强制转换）
		memset(FUN_Config,0,24);												//清空当前功能配置参数
		memcpy(FUN_Config,InBuff+14,len_frame);									//有效功能配置，写入FUN_Config数组（默认成功）
		BSP_WriteDataToFm(FUN_Config_Addr,FUN_Config,FUN_Config_Len);			//定入铁电
//		LteCommunication(InBuff,Len,0,0);										//配置成功，按照原命令返回，不接收（LteCommunication返回0）PS：协议上未要求返回
		return 1;
	}
	BspUartWrite(2,SIZE_OF("Fun_Config_Comm功能配置通信失败！\r\n"));
	return 0;
}

/*******************************************************************************
名称：INT8U SMS_Send_Comm(u8 *InBuff, u16 Len)
功能：包括密码检验、回复通信、向指定号码发送短信
入参：u8 *InBuff，传入的内容，正常为接收到的帧；u16 Len，长度
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U SMS_Send_Comm(u8 *InBuff, u16 Len)
{
	INT8U 	ST=0,i;
	INT8U	phone_num[12]={0};
	
	if(!PassworkCheckAndReport(InBuff));										//判断主站下发报文中密码是否正确，不正确时上报密码出错信息。
	else																		//若相同，密码正确
	{

		LteCommunication(InBuff,Len,0,0);										//配置成功，按照原命令返回，不接收（LteCommunication返回0）
		ME909S_Trans_OFF();
		for(i=0;i<6;i++)														//短信接收号码：为F加通信卡号，每个数字占半个字节。例如卡号为13912345678，则发送数据为：F1H，39H，12H，34H，56H，78H。
		{
			INT8UBCDToAscii(InBuff[14+i],phone_num+2*i);						//BCD转换为ASCII
		}
		ST=ME909S_SMS_Send(Device_Number,6,phone_num+1);						//向指定号码发送本机的装置编号，返回操作结果 phone_num[0]为F，不用管
		ME909S_Trans_ON();
	}	
	if(ST)return 1;	
	BspUartWrite(2,SIZE_OF("SMS_Send_Comm发送短信通信失败！\r\n"));	
	return 0;
}


/*******************************************************************************
名称：INT8U SMS_Judge(u8 *InBuff, u16 Len)
功能：判断InBuff内容是否符合协议要求，且密码正确
入参：u8 *InBuff，传入的内容，短信接收到的帧；u16 Len，长度
出参：无
返回：符合短信唤醒要求返回1，否则返回0
*******************************************************************************/
INT8U SMS_Judge(u8 *InBuff, u16 Len)
{
	if(Judge_NW_Framing(SMS_AWAKE,InBuff,Len,0)) 								//判断协议，若符合
	{
		if(0==memcmp(InBuff+10,Config.Password,4)) return 1;					//若判断原密码与原设置密码不同（相同时memcmp返回0）																	
	}
	BspUartWrite(2,SIZE_OF("SMS_Judge未通过！\r\n"));
	return 0;
}

/*******************************************************************************
名称：INT8U Data_Request_Comm(u8 *InBuff, u16 Len)
功能：装置收到主站数据请求命令后按原命令返回，并立即按照相应控制字格式将数据依次上送主站。
如果数据域为0字节，上传未成功上传的历史数据，包含历史照片，若装置无历史数据则不上传。
如果数据域为2字节BBBBH，装置立刻采集所有数据（图片除外），完成采集后立刻上传。该次采样不
影响原设定采集间隔的执行。
入参：u8 *InBuff，传入的内容，正常为接收到的帧；u16 Len，长度
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U Data_Request_Comm(u8 *InBuff, u16 Len)
{
	INT16U	len_frame=0;														//帧长度/或用于计算数据域长度
	
	LteCommunication(InBuff,Len,0,0);											//按照原命令返回，不接收（LteCommunication返回0）
	len_frame = ((INT16U)InBuff[8]<<8) + InBuff[9];								//计算数据域长度（保险起见，进行强制转换）
	if(!len_frame)																//如果数据域为0字节
	{
		/*上传未成功上传的历史数据，包含历史照片，若装置无历史数据则不上传。*/
		return Tem_Cur_Upload(RETRY,TIMEOUT);									//发送历史数据，失败时返回0（仅表示还有历史数据未发送完成，不影响已完成的部分）
	}
	else if((InBuff[10]==0xBB) && (InBuff[11]==0xBB))							//如果数据域为2字节BBBBH
	{
		return Tem_Cur_Sample_Upload(RETRY,TIMEOUT);							//装置立刻采集所有数据（图片除外），完成采集后立刻上传
	}
	BspUartWrite(2,SIZE_OF("Data_Request_Comm出现未知情况！\r\n"));
	return 0;																	//其他情况，不应该出现
}

/*******************************************************************************
名称：INT8U Flow_Data_Upload(u8 times, u16 timeout)
功能：终端主动上传装置流量数据使用情况。（控制字：40H）
先不做分帧和分包什么的，要做存储，还要管理，太麻烦。先只做首包
入参：u8 times，重试次数；u16 timeout，超时等待时间，单位：时间片
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U Flow_Data_Upload(u8 times, u16 timeout)
{
	INT8U 	i=0;
	INT8U 	*R;
	INT32U	Temp=0;																//用于单位转换和返回长度
	INT16U	f_len=0;															//组帧长度（共用Temp会带来麻烦）

/*Flow_Data结构体填充*/
	NW_GetTime(&Flow_Data.Sample_Time);											//更新Flow_Data结构体中的采样时间
	Temp = Local_FLow_Data.Flow_Day_Used_B >>20;								//计算当日已使用流量，单位：Byte -> M Byte
	Temp = htonl(Temp);															//大小端转换
	memcpy(Flow_Data.Day_Used,&Temp,4);											//单位：M Byte
	Temp = Local_FLow_Data.Flow_Month_Used_B>>20;								//计算当月已使用流量，单位：Byte -> M Byte
	Temp = htonl(Temp);															//大小端转换
	memcpy(Flow_Data.Month_Used,&Temp,4);										//单位：M Byte
	Temp = MONTHLY_FLOW>(Local_FLow_Data.Flow_Month_Used_B>>20) ? 				//本月剩余流量，单位：M Byte
		MONTHLY_FLOW-(Local_FLow_Data.Flow_Month_Used_B>>20) : 0;
	Temp = htonl(Temp);															//大小端转换
	memcpy(Flow_Data.Month_Surplus,&Temp,4);									//单位：M Byte

/*组帧上报*/
	f_len = NW_Framing(FLOW_DATA_UPLOAD,LTE_Tx_Buff);							//组帧
	for(;i<times;i++)
	{
		Temp = LteCommunication(LTE_Tx_Buff,f_len,LTE_Rx_Buff,timeout);			//上报并等待回复
		if(!Temp) continue;														//终端若没有收到主站回应命令，重试3次							

		R = Judge_NW_Framing(FLOW_DATA_UPLOAD,LTE_Rx_Buff,Temp,0);				//判断接收内容是否符合南网协议
		if(R)
		{
			if(R[10]!=1) continue;												//判断帧标识，目前做的只有首包且不分帧，固定为1
			if(R[11]!=0xAA) continue;											//判断回复数据域 AA55H
			if(R[12]!=0x55) continue;											//判断回复数据域
			return 1;															//上报成功
		}
	}
	BspUartWrite(2,SIZE_OF("Flow_Data_Upload流量上报通信失败！\r\n"));
	return 0;																	//上报失败，则将数据保留，下次传送。//先只做首包，因此可以不用保留，每次都会更新
}

/*******************************************************************************
名称：INT8U Sleep_Notice(void)
功能：该信息在装置设备每次休眠之前发送。主站无回复。（控制字：0CH）
入参：无
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U Sleep_Notice(void)
{
	INT16U	len=0;																//用于组帧
	len = NW_Framing(SLEEP_NOTICE,LTE_Tx_Buff);									//组帧
	LteCommunication(LTE_Tx_Buff,len,0,0);										//上报，无回复不接收（LteCommunication返回0）
	return 1;	
}

/*******************************************************************************
名称：INT8U Tem_Cur_Upload(u8 times, u16 timeout)
功能：装置主动上传导线温度、电流历史数据（控制字：26H），根据Unreport_Index[31][3]
索引表判断是否未上报成功。
入参：u8 times，重试次数；u16 timeout，超时等待时间，单位：时间片
出参：无
返回：成功返回1，失败返回0，无需上报返回0xFF
*******************************************************************************/
INT8U Tem_Cur_Upload(u8 times, u16 timeout)
{
	INT8U 	i=0,j=0,k=0;
	INT16U	Bit_Index = 0;														//未上报的历史文件序号，31天最多存储744小时，即0~743
	INT8U	nodata = 0xFF;
	
	/*检索Unreport_Index[31][3]，并计算Bit_Index*/
	for(i=0;i<31;i++){															//天
		for(j=0;j<3;j++){														//字节
			for(k=0;k<8;k++){													//每字节8小时
				if(0==(Unreport_Index[i][j]&(0x80>>k)))							//1已上报，0未上报（最高bit代表0时）
				{
				/*检索到未上报，执行相关操作*/
					nodata = 1;													//有上报，清0xFF
					WDTClear(LTE_PRIO);											//清除任务看门狗（这里可能循环很久）
					Bit_Index = 3*8*i+8*j+k;									//例：Unreport_Index[1][2]的第5（k）位为0，表示SUB2 第21小时 未上报，即第21+24=45小时，与计算结果相符（存在第0小时）
					if(NW_History_Temp_Comm(Bit_Index,RETRY,TIMEOUT))			//根据Bit_Index组帧并通信，包含n帧上报和等待回复，若成功
					{
						Unreport_Index[i][j] |= (0x80>>k);						//上报成功，更改Unreport_Index[][]索引表
						if(!BSP_WriteDataToFm(Unreport_Index_Addr,(INT8U*)Unreport_Index,Unreport_Index_Len)) 
						{
							BspUartWrite(2,SIZE_OF("未上报数据索引表写入铁电失败！\r\n"));
							return 0;											//未上报数据索引表写入铁电
						}
					}
				}
			}
		}
	}
	
	if(nodata==1) BspUartWrite(2,SIZE_OF("所有历史数据已尝试上报\r\n"));
	else BspUartWrite(2,SIZE_OF("没有历史数据需要上报\r\n"));
	return nodata;																//全部历史数据上报完成时返回1；无需上报时会返回0xFF
}

/*******************************************************************************
名称：INT8U Tem_Cur_Sample_Upload(u8 times, u16 timeout)
功能：装置立刻采集所有数据（图片除外），完成采集后立刻上传。该次采样不影响原设定采集间隔
的执行。（控制字：26H）
入参：u8 times，重试次数；u16 timeout，超时等待时间，单位：时间片
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U Tem_Cur_Sample_Upload(u8 times, u16 timeout)
{
	INT8U			i=0,j=0;
	static INT8U	msg_data;
	INT16U			addr;
	INT16U			len=0,len_frame=0;
	INT8U			*R;
	
	msg_data = DATA_CMD;														//用于发出，通知RF任务切换到采集数据状态
	OSMboxPost(Data_CMDB0X, &msg_data);											//通过邮箱将采集命令发出
	OSTimeDly(5*20);															//等待5秒，确保RF任务将内存中数据写入铁电
	
	//从铁电中读取数据并组帧上报
	BSP_InitFm(LTE_Num);														//初始化
	for(i=0;i<55;i++)
	{			
		if(TT_Info.HaveTT[i]==0x55)												//表示这里有数据
		{
		/*填充结构体并组帧*/
			Tem_Cur_Data.Frame_ID = i;											//帧标识
			Tem_Cur_Data.Pack_Num = 1;											//包数
			Tem_Cur_Data.Unit_ID = Unit_ID_Code[i];								//功能单元识别码
			SecondToNwTime(TT_Sample_Manage.Time[TT_Sample_Manage.Sample_Num-1],&Tem_Cur_Data.Sample_Time);			//采样时间（年+月+日+时+分+秒）（6字节）
			addr = Sample_Data_Addr + One_TT_Sample_Data_Len*i + 2*(TT_Sample_Manage.Sample_Num-1);					//计算温度数据地址
			BSP_ReadDataFromFm(addr, Tem_Cur_Data.Tem, 2);						//从铁电中读取对应测点温度（2字节）
			Tem_DS18B20_To_NW(Tem_Cur_Data.Tem,Tem_Cur_Data.Tem);				//按南网规则转换温度
			//Tem_Cur_Data.Cur													//导线电流（2字节）暂无
			Tem_Cur_Data.Voltage = 33;											//传感器工作电压（1字节）3.3V
			len_frame = NW_Framing(TEM_CUR_UPLOAD, LTE_Tx_Buff);				//当前采集内容组帧
			
		/*上报并等回复，重试times次*/
			for(j=0;j<times;j++)												//最多重试times次
			{
				len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,timeout);		//返回为0表示通信失败，返回0xff表示没接收到数据，其他返回为接收数据长度
				if(!len) 
				{
					BspUartWrite(2,SIZE_OF("未收到主站回复！\r\n"));
					continue;													//通信失败时重试
				}

				R = Judge_NW_Framing(TEM_CUR_UPLOAD,LTE_Rx_Buff,len,0);			//判断接收内容是否符合南网协议
				if(R) 															//不符合协议时重新通信
				{
					if(R[10]!=i) continue;										//判断帧标识i
					if(R[11]!=0xAA) continue;									//判断回复数据域 AA55H
					if(R[12]!=0x55) continue;									//判断回复数据域
					break;														//上报成功	
				}
			}
			if(j==times) 
			{
				FM_LowPower(LTE_Num);											//铁电引脚低功耗配置
				BspUartWrite(2,SIZE_OF("Tem_Cur_Sample_Upload重试超次数，采集上报通信失败！\r\n"));
				return 0;														//重试超次数，通信失败，返回0
			}
		}				
	}
	FM_LowPower(LTE_Num);														//铁电引脚低功耗配置	
	return 1;																	//全部探头上报完成时返回1
}

INT16U NW_History_Temp_Framing(u8 Frame_ID, u8 *InBuff, struct SAMP_MANAGE *Info_str, u8 *OutBuff);
/*******************************************************************************
名称：INT16U NW_History_Temp_Comm(u16 Bit_Index, u8 times, u16 timeout)
功能：将Bit_Index所指示的文件读取、组帧、上报、回复处理一条龙服务
入参：u16 Bit_Index，用于指示读取哪个历史数据文件来组帧。0~23表示SUB1中的0~23号文件；24~47
表示SUB2中的0~23号文件；……720~743表示SUB31中的0~23号文件。
u8 times，重试次数；u16 timeout，超时等待时间，单位：时间片
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT16U NW_History_Temp_Comm(u16 Bit_Index, u8 times, u16 timeout)
{
	INT8U			day = 0, hour = 0;
	INT8U			i = 0,j = 0;
	TCHAR			path[50];
	UINT			bw;
	INT16U			offset_len,len_frame,len;		
	INT8U			*R;
	struct SAMP_MANAGE	Info_str;												//暂存从文件读出的结构体信息
	
	/*根据Bit_Index打开对应文件*/
	day = Bit_Index/24 +1;														//日期，对应SUBn
	hour = Bit_Index%24;														//小时，对应0~23
	sprintf(path, "/SUB%d/%02d%02d", day, day, hour);							//根据索引生成文件路径“SUB日/日时”	注意！文件名勿随意改动！
	
	BspUartWrite(2,SIZE_OF("\r\n开始传输"));
	BspUartWrite(2,(INT8U*)path,strlen(path));									//不合并字符的原因：path是文件名，下面要用到
	BspUartWrite(2,SIZE_OF("\r\n"));

	OSTimeDly(25);																//延时作用：此函数在外面循环中调用多次（当需要发送多个小时数据时会再次调用），此时延时大于RF任务中轮询间隔1s，可保证优先RF任务使用文件系统
	while(FATFS_Lock)	OSTimeDly(20);
	FATFS_Lock=1;
	
	f_mount(&fs, "", 1);														/* Re-mount the default drive to reinitialize the filesystem */
	if (FR_OK==f_open(&fil, path, FA_OPEN_EXISTING | FA_READ))					//打开文件	&fil是全局变量，注意不要同时打开
	{
	/*读取SAMP_MANAGE结构体信息*/
		f_read(&fil,&work,FF_MAX_SS,&bw);										//读取到工作空间（用来格式化的work数组，其他地方没用到）
		offset_len = Search_Info((u8 *)&work,FF_MAX_SS);						//在&work查找文件信息结构体，返回有效文件头的offset长度；若未找到返回0xFFFF
		if(offset_len==0xFFFF) 	
		{
			FATFS_Lock=0;
			f_close(&fil);
			BspUartWrite(2,SIZE_OF("未找到有效sample_manage信息结构体！\r\n"));
			return 0;															//未找到有效结构体信息
		}			
	/*根据结构体信息执行通信*/
		memcpy(&Info_str,(INT8U*)&work+2+offset_len,Sample_Manage_Len);			//地址+offset_len指向文件头0xFF 0xAA，再+2（必须先强制转换，否则是+2个结构体长度）指向sample_manage结构体
		offset_len += Sample_Manage_Len+2;										//后移结构体长度+FFAA长度
		f_lseek(&fil, offset_len);												//调整文件指针，指向首个探头数据
		for(;i<Info_str.TT_Count;i++)											//文件中读出来的探头数，代表需要从文件读取数据的条数
		{
			/*读取文件并组帧*/
			if(f_read(&fil,&work,Info_str.Len,&bw)) 
			{
				FATFS_Lock=0;
				f_close(&fil);
				BspUartWrite(2,SIZE_OF("SPIFLASH读取文件失败！\r\n"));
				return 0;														//根据文件信息结构体读取一条数据（重复读取即可，指针自动增长）。读取失败返回0			
			}		
			len_frame = NW_History_Temp_Framing(i,(u8 *)&work, &Info_str, LTE_Tx_Buff);		//第i条数据组帧到LTE_Tx_Buff，i为帧标识
			
			/*上报并等回复，重试times次*/
			for(j=0;j<times;j++)												//最多重试times次
			{
				len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,timeout);			//返回为0表示通信失败，返回0xff表示没接收到数据，其他返回为接收数据长度
				if(!len) 
				{
					BspUartWrite(2,SIZE_OF("未收到主站回复！\r\n"));
					continue;													//重试
				}

				R = Judge_NW_Framing(TEM_CUR_UPLOAD,LTE_Rx_Buff,len,0);			//判断接收内容是否符合南网协议
				if(R) 															//不符合协议时重新通信
				{
					j = 0;														//重新开始计重试次数
					if(R[10]!=i) continue;										//判断帧标识i
					if(R[11]!=0xAA) continue;									//判断回复数据域 AA55H
					if(R[12]!=0x55) continue;									//判断回复数据域
					break;														//上报成功	
				}
			}
			if(j==times) 
			{
				f_close(&fil);													//关闭文件。dismount是为了the work area can be discarded，我们不释放内存，没有也行
				FATFS_Lock=0;
				BspUartWrite(2,SIZE_OF("NW_History_Temp_Comm重试超次数，温度上报通信失败！\r\n"));
				return 0;														//重试超次数，通信失败，返回0					
			}
		}
		
	/*通信完成*/
		f_close(&fil);															//关闭文件。dismount是为了the work area can be discarded，我们不释放内存，没有也行
		FATFS_Lock=0;
		return 1;																//全部探头上报完成，返回1
	}
	FATFS_Lock=0;
	BspUartWrite(2,SIZE_OF("打开文件失败！\r\n"));
	return 0;																	//打开文件失败
}

/*******************************************************************************
名称：INT16U Search_Info(u8 *InBuff, u16 Len)
功能：在缓存中查找有效SAMP_MANAGE结构体信息。
入参：u8 *InBuff，待查找缓存；u16 Len，查找范围
出参：无
返回：有效文件头的offset长度；或0xFFFF，表示未找到
*******************************************************************************/
INT16U Search_Info(u8 *InBuff, u16 Len)
{
	INT16U	i,crc;
	
	for(i=0;i<Len;i++)
	{
		if(InBuff[i]==0xFF)														//文件开头是0xFF 0xAA
		{
			if(InBuff[i+1]==0xAA)												//找到文件头了
			{
				crc = RTU_CRC(InBuff+i+2,Sample_Manage_Len-2-2);				//计算CRC，用于读写校验
				if(*(InBuff+i+2+Sample_Manage_Len-3)!=((crc>>8)&0xff)) continue;//CRC高字节（这个是高地址）==>这样是按小端模式，和内存存储U16相同
				if(*(InBuff+i+2+Sample_Manage_Len-4)!=(crc&0xff)) continue;		//CRC低字节（这个是低地址）==>这样是按小端模式，和内存存储U16相同
				else return i;													//有效，返回offset长度（0xFF到文件头的偏移量）
			}
		}
	}
	return 0xFFFF;																//没找到有效结构体信息
}

/*******************************************************************************
名称：INT16U NW_History_Temp_Framing(u8 Frame_ID, u8 *InBuff, struct SAMP_MANAGE *Info_str, u8 *OutBuff)
功能：按南网协议组温度电流数据帧。详细说明见NW_Framing()函数。
----------------------------------------------------------------
	起始码	装置号码	控制字	数据域长度	数据域	校验码	结束码	|
	1字节	6字节	1字节	2字节		变长	1字节	1字节	|
----------------------------------------------------------------
入参：u8 Frame_ID,帧ID；u8 *InBuff,从文件中读取的内容；struct SAMP_MANAGE *Info_str, 结构体指针。
出参：INT8U *OutBuff，组帧完后的存放地址。缓存大小最大要443字节。
返回：总的报文长度
*******************************************************************************/
INT16U NW_History_Temp_Framing(u8 Frame_ID, u8 *InBuff, struct SAMP_MANAGE *Info_str, u8 *OutBuff)
{
	INT8U	i;
	INT16U	delta_T,len;														//时间差，内容长度
	
	len = 7+11+7*(Info_str->Sample_Num-1);										//数据域长度
	
/*起始码、装置号码、控制字、数据域长度*/
	OutBuff[0]=Start_Code;														//1Byte起始码，固定68H
	OutBuff[1]=Device_Number[0];
	OutBuff[2]=Device_Number[1];												//前两字节表示厂家代码（由南方电网公司统一分配），采用大写字母(ASCII)
	OutBuff[3]=Device_Number[2];												//后四字节表示厂家对每套状态监测装置的识别码（基站地址），采用大写字母及数字，优先使用数字
	OutBuff[4]=Device_Number[3];														
	OutBuff[5]=Device_Number[4];														
	OutBuff[6]=Device_Number[5];												//6Byte装置号码
	OutBuff[7]=TEM_CUR_UPLOAD;													//1Byte控制字，用于区分数据类型 固定TEM_CUR_UPLOAD
	OutBuff[8]=(len>>8)&0xff;													//
	OutBuff[9]=len&0xff;														//2Byte数据域长度，高字节在前，若为零表示无数据域
	
/*数据域*/	
	memcpy(OutBuff+10,&Config.SecurityCode,4);									//4Byte密文认证
	OutBuff[14]=Frame_ID;														//1Byte帧标识
	OutBuff[15]=Info_str->Sample_Num;											//1Byte包数
	OutBuff[16]=InBuff[0];														//1Byte功能单元识别码
												
	/*首包（第0包）11Byte*/
	SecondToNwTime(Info_str->Time[0],(struct NW_TIME*)(OutBuff+17));			//6Byte采样时间（年+月+日+时+分+秒）（6字节，HEX表示）
	Tem_DS18B20_To_NW(OutBuff+23,InBuff+1);										//2Byte测点温度，上送值=（实际温度+50）*10
	memset(OutBuff+25,0,2);														//2Byte导线电流，上送值=实际电流*10，先固定写0
	OutBuff[27]=33;																//1Byte传感器工作电压，上送值=实际电压*10，先固定写个33

	/*第一包及以后各包7Byte*/
	for(i=1;i<Info_str->Sample_Num;i++)											//i初值为1，因为首包不在这里处理
	{
		delta_T=Info_str->Time[i]-Info_str->Time[i-1];							//计算与上包采样时间差
		OutBuff[28+7*(i-1)]=(delta_T>>8)&0xFF;									//2Byte采样时间差，高字节
		OutBuff[29+7*(i-1)]=delta_T&0xFF;										//低字节
		Tem_DS18B20_To_NW(OutBuff+30+7*(i-1),InBuff+2*i+1);						//2Byte测点温度
		memset(OutBuff+32+7*(i-1),0,2);											//2Byte导线电流
		OutBuff[34+7*(i-1)]=33;													//1Byte传感器工作电压		
	}
	
/*校验码、结束码*/
	OutBuff[35+7*(i-2)]=Negation_CS(OutBuff+1,34+7*(i-2));						//1Byte检验码  计算CS前除起始码的所有字节
	OutBuff[36+7*(i-2)]=Epilog_Code;											//1Byte结束码，固定16H
	return 37+7*(i-2);		 													//总的报文长度
}

/*******************************************************************************
名称：void Tem_DS18B20_To_NW(INT8U* Outbuff,INT8U* InBuff)
功能：将18B20测得的两字节温度转化成南网规约需求的格式（上送值=实际温度*10+500）
18B20测得的温度值为实际温度*10
入参：InBuff ：输入温度数组指针
出参：Outbuff：输出温度数组指针
返回：无
*******************************************************************************/
void Tem_DS18B20_To_NW(INT8U* Outbuff,INT8U* InBuff)									
{
	INT16U Temp=0;
	
	Temp=(InBuff[0]<<8)+InBuff[1];
	
	if(Temp>=0xf800) 
	{
		if(Temp>0xF9F4)	Temp=0xF9F4;											//温度小于-50°时都设为-50° （18B20读数为F9F4）
		Temp=500-Temp&0x07ff;													//负温
	}
	else Temp=500+Temp;															//正温
	
	Outbuff[0]=(Temp>>8)&0xff;
	Outbuff[1]=Temp&0xff;
}

/*******************************************************************************
名称：INT8U File_List_Query_Comm(u8 *InBuff, u16 Len)
功能：主站查询某个时间范围内装置存储的文件列表，装置收到该命令后，返回符合查询条件的文件
列表。01H，JPEG文件，对应图像文件；02H，故障定位波形文件；....FFH，所有文件类型。
入参：u8 *InBuff，传入的内容，正常为接收到的帧；u16 Len，长度
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U File_List_Query_Comm(u8 *InBuff, u16 Len)
{
	INT16U	len_frame=0;														//帧长度/或用于计算数据域长度
	
	//time_start = time_change(InBuff+11);										//起始时间结束时间解析		===============未完成
	//time_end = time_change(InBuff+17);
	switch(InBuff[10])															//文件类型解析
	{
		case JPEG_FILE:		/*暂无此功能*/
		case FLW_FILE:		/*暂无此功能*/										//两个case共用一个
				len_frame = NW_Framing(FLW_FILE,LTE_Tx_Buff);					//组帧，若符合查询条件的文件格式为0，返回0000H
				LteCommunication(LTE_Tx_Buff,len_frame,0,0);					//上报，无回复不接收（LteCommunication返回0）
				return 1;
		
		case ANY_FILE:		/*温度详细数据可以放这里*/
//				len_frame = NW_FileList_framing(FILE_LIST_QUERY,0,time_start,time_end,LTE_Tx_Buff);		//文件File_List专用组帧，再做个函数吧		===============未完成
				LteCommunication(LTE_Tx_Buff,len_frame,0,0);					//上报，无回复不接收（LteCommunication返回0）
				return 1;
		
		default:
				break;
	}
	//打印调试信息
	return 0;
}

/*******************************************************************************
名称：INT8U Files_Upload(u8 *FileName,u8 Des_Len)
功能：	STEP 1 装置请求上送文件（控制字：73H）
		STEP 2 文件上送（控制字：74H）
		STEP 3 文件上送结束标记（控制字：75H），并等待文件补包数据下发（控制字：76H）
		STEP 4 解析接收内容并判断，选择回到STEP 2 进行补发，或结束发送
入参：u8 *FilenName，文件名指针，将此文件上传
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U Files_Upload(u8 *FileName,u16 Des_Len)
{
	INT8U	i=0;
	INT8U 	STEP=FILE_UL_REQUEST;												//发送起始步骤为上报请求
	INT16U	len_frame=0;														//帧长度
	INT16U	len=0;																//接收到的长度
	INT16U	packages=0;															//文件总包数
	INT16U 	Pac_Num=0;															//当前要组帧的包号
	INT16U	file_len=0;															//文件总长度
	INT8U	*R,*P_pac_num=0;													//首地址/补包包号首地址
	
	//file_len = 																//读取文件	===============未完成
	packages = file_len / Des_Len;												//计算文件总包数
	
	while(1)
	{
		switch(STEP)
		{
			case FILE_UL_REQUEST:	/*STEP 1 装置请求上送文件（控制字：73H）*/
					len_frame = NW_File_Framing(FILE_UL_REQUEST,FileName,0,0,LTE_Tx_Buff);			//文件专用组帧		===============未完成
					for(i=0;i<5;i++)																//最多重试5次（协议要求）
					{
						len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,3*20);				//装置请求上送文件，并等待接收，每次间隔3秒（协议要求）
						if(!len) continue;															//通信失败时重试
						Judge_NW_Framing(FILE_UL_REQUEST,LTE_Rx_Buff,len,0);						//判断接收内容是否符合南网协议
						////进一步验证是否原命令返回（不做了）
						STEP = FILE_UPLOAD;															//下一步，文件上传
						break;																		//跳出for循环																
					}
					//打印调试信息
					return 0;																		//重试超时，请求失败
			
			case FILE_UPLOAD:		/*STEP 2 文件上送（控制字：74H）*/
					for(i=0;i<packages;i++)
					{
						if(P_pac_num==0) Pac_Num=i;													//计算要补包的包号（非补包状况）
						else Pac_Num = (P_pac_num[2*i]>>8)+P_pac_num[2*i+1];						//计算要补包的包号（补包状况）
						len_frame = NW_File_Framing(FILE_UPLOAD,FileName,Pac_Num,Des_Len,LTE_Tx_Buff);		//文件专用组帧，每包文件内容长度Des_Len		===============未完成
						LteCommunication(LTE_Tx_Buff,len_frame,0,0);										//只发不收			
					}
					STEP = FILE_UL_END;																//下一步，文件上传结束标记	
					break;
			
			case FILE_UL_END:		/*STEP 3 文件上送结束标记（控制字：75H），并等待文件补包数据下发（控制字：76H）*/
					OSTimeDly(2*20);																//装置上送文件数据全部结束后2秒，发送该指令
					len_frame = NW_File_Framing(FILE_UL_END,FileName,0,0,LTE_Tx_Buff);				//文件专用组帧		===============未完成
					for(i=0;i<5;i++)																//最多重试5次（协议要求）
					{
						len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,30*20);			//装置请求上送文件，并等待接收，每次间隔30秒（协议要求），延时很可能会不准，不管它了
						if(!len) continue;															//通信失败时重试
						R = Judge_NW_Framing(FILE_FILLING,LTE_Rx_Buff,len,0);						//判断接收指令是否为76H
						if(!R)	continue;															//接收不正确时重试	
						STEP = 4;																	//下一步，解析接收内容并判断
						break;																		//跳出for循环																
					}
					//打印调试信息
					return 0;																		//重试超时，主站未回复上送结束标记	
			
			case 4:					/*STEP 4 解析接收内容并判断*/
					if(R[110]==0) return 1;															//无需补包时，发送成功，返回1
					packages = R[110];																//计算补包数（1字节）
					P_pac_num = R+111;																//补包包号首地址
					STEP = FILE_UPLOAD;																//下一步，回到STEP 2 文件上送（控制字：74H）
					break;
			
			default:
					break;
		}
		return 0;
	}
}

/*******************************************************************************
名称：INT16U NW_File_Framing(u8 Cmd,u8 *FileName,u8 Pac_Num,u8 Des_Len,u8 *OutBuff)
功能：文件组帧专用，按南网协议组帧。帧结构及数据排列格式说明:数据包采用数据帧模式，对数
据帧定义起始码、装置号码、控制类型码、数据域长度、数据域、校验码和结束码。数据帧长度不大
于4000字节。采用大端模式（数据的高字节保存在内存的低地址中）。校验码采用累加和取反的校验
方式，发送方将装置号码、控制字、数据域长度和数据区的所有字节进行算术累加，抛弃高位，只保
留最后单字节，将单字节取反。
----------------------------------------------------------------
起始码	装置号码	控制字	数据域长度	数据域	校验码	结束码	|
1字节	6字节		1字节	2字节		变长	1字节	1字节	|
----------------------------------------------------------------
入参：INT8U Cmd,控制字，用于区分数据类型；u8 *FileName,文件名；u8 Pac_Num,组帧包号；
u8 Des_Len,帧内容长度
出参：INT8U *OutBuff，组帧完后的存放地址
返回：总的报文长度
*******************************************************************************/
INT16U NW_File_Framing(u8 Cmd,u8 *FileName,u8 Pac_Num,u8 Des_Len,u8 *OutBuff)
{
	INT16U len;																	//内容长度

	/*获取内容，返回内容长度*/
//	len=Get_DataField(Cmd,&OutBuff[10]);										//获取内容，返回内容长度
	////这里读取文件						 		==============================未完成
	
	OutBuff[0]=Start_Code;														//1Byte起始码，固定68H
	OutBuff[1]=Device_Number[0];
	OutBuff[2]=Device_Number[1];												//前两字节表示厂家代码（由南方电网公司统一分配），采用大写字母(ASCII)
	OutBuff[3]=Device_Number[2];												//后四字节表示厂家对每套状态监测装置的识别码（基站地址），采用大写字母及数字，优先使用数字
	OutBuff[4]=Device_Number[3];														
	OutBuff[5]=Device_Number[4];														
	OutBuff[6]=Device_Number[5];												//6Byte装置号码
	OutBuff[7]=Cmd;																//1Byte控制字，用于区分数据类型
	OutBuff[8]=(len>>8)&0xff;													//
	OutBuff[9]=len&0xff;														//2Byte数据域长度，高字节在前，若为零表示无数据域
	//OutBuff[10]开始是内容，已获取
	OutBuff[10+len]=Negation_CS(OutBuff+1,9+len);								//1Byte检验码  计算OutBuff的前9+len个字节
	OutBuff[11+len]=Epilog_Code;												//1Byte结束码，固定16H
	return 12+len;		 														//总的报文长度
}

/*******************************************************************************
名称：INT8U NW_ReceiveAndExecute(u8 *OutBuff,u16 timeout_s)
功能：等待接收数据，解析并执行相应功能。协议解析放在各个case下分别进行。成功时返回，失败时
将等待timeout_s耗尽。
入参：u8 timeout，接收超时设定，单位为秒。
出参：u8 *OutBuff,接收内容存放位置
返回：接收并执行成功返回1，有特殊动作待执行时返回状态字（如重启等），未执行或操作失败则返回0
*******************************************************************************/
INT8U NW_ReceiveAndExecute(u8 *OutBuff,u16 timeout_s)
{
	INT8U				*R;														//存放协议首地址
	INT8U				Cmd,state;												//存放cmd和return的值
	INT16U				len=0;													//len为接收到的总长度/或用于组帧
	static INT16U		len_frame = 0;											//len_frame帧长度，由Judge_NW_framing()获取								

/*LTE模块无法彻底关闭回复的特性导致不可用timeout进行长时间的延时，用循环分解下*/
	while(timeout_s--)															//timeout_s单位为秒
	{
		/*接收并判断数据是否符合要求*/
		len = LTE_WaitData(OutBuff,20);											//每次循环固定接收1秒，当LTE模块返回不可控的回复时，对时间判断影响不大
		R = Judge_NW_Framing(ANY_CMD,OutBuff,len,&len_frame);					//判断协议
		if(R)																	//若符合协议，且要求等待的数据与当前接收数据类型相同，进入switch进一步判断和执行
		{	
			Cmd =*(R+7);														//将返回的命令符提取出来
		/*根据接收到的命令执行相应功能*/
			switch(Cmd)
			{
				case START_UP://00H												//主站主动下发，装置收到请求后，发送开机联络信息
						if(Startup_Comm( RETRY, TIMEOUT )) return 1;			//开机联络通信，最多重试RETRY次，TIMEOUT秒超时
						else break;
				
				case TIMMING://01H												//主站下发对时命令，装置收到此命令后按照原命令返回
						LteCommunication(R,len_frame,0,0);						//按照原命令返回，不接收（LteCommunication返回0）
						state = SetTime(R);										//更改RTC，接收到的内容在*R，成功返回1
						if(state) return state; else break;
				
				case SET_PASSWORD://02H											//装置收到该命令后判断原密码是否与原设置密码相同，若相同则将密码更改为新密码，并按照原命令返回。若不同，则返回出错信息
						state = Set_Password_Comm(R,len_frame);					//更改密码功能函数（必须用R，函数内没有判断协议）
						if(state) return state; else break;
				
				case PARA_CONFG://03H											//装置验证密码通过后，执行参数配置命令，并按照原命令返回。若密码错误，则返回密码出错信息
						state = ParaConfigComm(R,len_frame);					//参数配置功能函数（必须用R，函数内没有判断协议）
						if(state) return state; else break;
				
				case SET_IP://06H												//只有密码与装置密码相同且两组主站IP、端口号和主站卡号对应字节完全相同才执行更改命令。装置执行更改命令后按照原命令格式返回。若密码错误，则返回密码出错信息
						state = Set_IP_Comm(R,len_frame);						//改主站IP地址、端口号和卡号功能函数（必须用R，函数内没有判断协议）
						if(state) return state; else break;
				
				case DEMAND_IP://07H											//装置收到该命令后，返回其当前设置的主站IP、端口号和主站卡号。
						len = NW_Framing(DEMAND_IP,LTE_Tx_Buff);				//组帧
						LteCommunication(LTE_Tx_Buff,len,0,0);					//上报，无回复不接收（LteCommunication返回0）
						return 1;
				
				case RESET_DEV://08H											//只有装置密码通过后，装置原命令返回并执行此命令，否则返回出错信息。
						if(!PassworkCheckAndReport( R )) break;					//判断主站下发报文中密码是否正确，不正确时上报密码出错信息。
						return RESET_DEV;										//密码相同时，返回重启装置状态字

				case SMS_AWAKE://09H											//主站若通过IP网络UDP通信，发送此控制字，也表示唤醒终端。正常情况下，处于休眠状态的装置应在接到唤醒命令后，立即切换到在线状态。
						if(!PassworkCheckAndReport( R )) break;					//判断主站下发报文中密码是否正确，不正确时上报密码出错信息。
						return SMS_AWAKE;										//密码相同时，返回在线状态字
				
				case DEMAND_CONFG://0AH											//用于主站查询装置配置参数。
						len = NW_Framing(DEMAND_CONFG,LTE_Tx_Buff);				//组帧
						LteCommunication(LTE_Tx_Buff,len,0,0);					//上报，无回复不接收（LteCommunication返回0）
						return 1;
				
				case FUN_CONFG://0BH											//用于主站向装置下发功能配置参数。装置功能配置仅下发有效功能，无效功能不发送，默认无效。
						state = Fun_Config_Comm(R,len_frame);					//装置功能配置功能函数（必须用R，函数内没有判断协议）
						if(state) return state; else break;
				
				case DEMAND_DEV_TIM://0DH										//主站主动查询装置系统时间。
						len = NW_Framing(DEMAND_DEV_TIM,LTE_Tx_Buff);			//组帧	时间应答 年	月	日	时	分	秒
						LteCommunication(LTE_Tx_Buff,len,0,0);					//上报，无回复不接收（LteCommunication返回0）
						return 1;
				
				case SMS_SEND://0EH												//主站要求装置向指定的手机号码发送确认短信。装置接收到请求后原命令返回，然后向指定的短信接收号码发送本机的装置编号，如“CC0011”。
						state = SMS_Send_Comm(R,len_frame);						//发送确认短信功能函数（必须用R，函数内没有判断协议）
						if(state) return state; else break;
				
				case DATA_REQUEST://21H											//用于主站主动请求监测装置发送数据。装置收到该命令后按原命令返回，并立即按照相应控制字格式将数据依次上送主站。两种：见协议
						state = Data_Request_Comm(R,len_frame);					//主站请求装置数据功能函数（必须用R，函数内没有判断协议）
						if(state==1) return state; else break;					//无需上报返回0xFF，继续等待
				
				case TEM_CUR_UPLOAD://26H										//装置收到该命令后立即将未传送数据上送主站。
						state = Tem_Cur_Upload(RETRY,TIMEOUT);					//发送历史数据功能函数，无需上报返回0xFF，失败时返回0（仅表示还有历史数据未发送完成，不影响已完成的部分）
						if(state==0xFF) LteCommunication(R,len_frame,0,0);		//若装置无未上送数据，则原命令返回。
						else if(state==1) return state; else break;				
				
				case FAULT_INFO://30H											//装置接收到请求后发送当前存在的故障信息。
						state = Fault_Info_Comm(RETRY,TIMEOUT);					//故障信息通信，失败时返回0
						if(state) return state; else break;
				
				case FLOW_DATA_UPLOAD://40H										//终端收到该命令后立即将所有未传送成功的数据上送主站。若装置无未上送数据，则原命令返回。
						state = Flow_Data_Upload(RETRY,TIMEOUT);				//流量数据使用情况上报功能函数
						if(state) return state; else break;
				
			/*文件功能可以用在详细数据上报*/
				case FILE_LIST_QUERY://71H										//主站查询某个时间范围内装置存储的文件列表，装置收到该命令后，返回符合查询条件的文件列表。
						state = File_List_Query_Comm(R,len_frame);
						if(state) return state; else break;
				
				case FILE_REQUEST://72H											//装置收到该命令后，执行文件上报操作
						LteCommunication(R,len_frame,0,0);						//按照原命令返回，不接收（LteCommunication返回0）
						state = Files_Upload(R+10,DES_LEN);						//并立即将相应文件上送主站。
						if(state) return state; else break;
				
			/*方诚扩展协议 NW_ReceiveAndExecute*/
				case EXTEN_ONOFF://F0H											//只有密码与装置密码相同才执行此命令。FFH为启用，允许装置上报扩展协议；00H为禁用，禁止装置上报扩展协议。装置接收到命令后，以原命令返回。
						state = ExtendOnOffComm(R,len_frame);					
						if(state) return state; else break;
				
				case EX_SET_APN://F3H											//只有密码与装置密码相同才执行此命令，装置执行更改命令后按照原命令格式返回。若密码错误，则返回密码出错信息
						state = SetApnComm(R,len_frame);						//设置APN
						if(state) return state; else break;
				
				case EX_DEMAND_APN://F4H										//只有密码与装置密码相同才执行此命令，装置执行格式化FLASH命令成功后，按照原命令格式返回。若密码错误，则返回密码出错信息
						len = NW_Framing(EX_DEMAND_APN,LTE_Tx_Buff);			//组帧
						LteCommunication(LTE_Tx_Buff,len,0,0);					//上报，无回复不接收（LteCommunication返回0）
				
				case EX_VERSION_SIM://F6H										//装置接收到主站查询指令后，立即读取装置信息并回复。
						len = NW_Framing(EX_VERSION_SIM,LTE_Tx_Buff);			//根据控制字组帧    
						LteCommunication(LTE_Tx_Buff,len,0,0);					//上报，无回复不接收（LteCommunication返回0）
						return 1;
				
				case EX_UPDATA_REQUEST://F7H 									//主站在收到扩展心跳5秒后，下发升级请求。
						state = UpdataRequestComm( R );							//装置接收到主站升级请求后，立刻进行升级检测并进行回复。
						if(state) return state; else break;					
																				
				case EX_UPDATA_DOWNLOAD://F8H 									//主站下发升级包，升级包数据为二进制码流。数据包下发间隔为1秒。
						state = UpdataDownloadComm( R );						//解析文件和保存（写入FLASH）
						if(state) return state; else break;		

				case EX_UPDATA_DL_DONE://F9H 									//全部升级包下发结束后2秒，主站发送该指令
						state = UpdataFinishComm( R );							//装置收到后立即上传文件补包（FAH）。
						if(state) return state; else break;
				
				case EX_FORMAT_FLASH://FBH										//只有密码与装置密码相同才执行此命令，装置执行格式化FLASH命令成功后，按照原命令格式返回。若密码错误，则返回密码出错信息
						state = FormatFlashComm(R,len_frame);					//格式化FLASH
						if(state) return state; else break;
						
				default:
						break;
			}	
		}	
	}
	BspUartWrite(2,SIZE_OF("NW_Receive_Execute等待接收数据结束\r\n"));
	return 0;																	//超时，返回0
}

/*******************************************************************************
名称：bool PassworkCheckAndReport(INT8U *Inbuff)
功能：判断主站下发报文中密码是否正确，不正确时上报密码出错信息。
入参：INT8U *Inbuff，报文起始地址
出参：无
返回：密码相同，返回真；密码不同，返回假
*******************************************************************************/
bool PassworkCheckAndReport(INT8U *Inbuff)
{
	INT8U len = 0;
	if(memcmp(Inbuff+10,Config.Password,4))										//若判断原密码与原设置密码不同（相同时memcmp返回0）
	{
		len = NW_Framing( PASSWORD_ERR, LTE_Tx_Buff );							//组密码出错信息帧
		LteCommunication(LTE_Tx_Buff,len,0,0);											//上报出错信息，不接收（LteCommunication返回0）
		return false;															//密码不同，返回假
	}
	return true;																//密码相同，返回真
}

/*******************************************************************************
名称：INT16U NW_Framing(u8 FileUpCmd,u8 *OutBuff)
功能：按南网协议组帧。帧结构及数据排列格式说明:数据包采用数据帧模式，对数据帧定义起始码、
装置号码、控制类型码、数据域长度、数据域、校验码和结束码。数据帧长度不大于4000字节；短信
通信方式，帧长不大于130字节。采用大端模式（数据的高字节保存在内存的低地址中）。校验码采
用累加和取反的校验方式，发送方将装置号码、控制字、数据域长度和数据区的所有字节进行算术累
加，抛弃高位，只保留最后单字节，将单字节取反。
----------------------------------------------------------------
起始码	装置号码		控制字	数据域长度	数据域	校验码	结束码	|
1字节	6字节		1字节	2字节		变长	1字节	1字节	|
----------------------------------------------------------------
入参：INT8U Cmd,控制字，用于区分数据类型；
出参：INT8U *OutBuff，组帧完后的存放地址
返回：总的报文长度
*******************************************************************************/
INT16U NW_Framing(u8 Cmd,u8 *OutBuff)
{
	INT16U len;																	//内容长度

	len=Get_DataField(Cmd,&OutBuff[10]);										//获取内容，返回内容长度
	if(Cmd==DATA_UNCORRESPOND) Cmd=SET_IP;										//这里有点乱，先不改了，目前用到 DATA_UNCORRESPOND 的只有SET_IP
	OutBuff[0]=Start_Code;														//1Byte起始码，固定68H
	OutBuff[1]=Device_Number[0];
	OutBuff[2]=Device_Number[1];												//前两字节表示厂家代码（由南方电网公司统一分配），采用大写字母(ASCII)
	OutBuff[3]=Device_Number[2];												//后四字节表示厂家对每套状态监测装置的识别码（基站地址），采用大写字母及数字，优先使用数字
	OutBuff[4]=Device_Number[3];														
	OutBuff[5]=Device_Number[4];														
	OutBuff[6]=Device_Number[5];												//6Byte装置号码
	OutBuff[7]=Cmd;																//1Byte控制字，用于区分数据类型
	OutBuff[8]=(len>>8)&0xff;													//
	OutBuff[9]=len&0xff;														//2Byte数据域长度，高字节在前，若为零表示无数据域
	//OutBuff[10]开始是内容，已获取
	OutBuff[10+len]=Negation_CS(OutBuff+1,9+len);								//1Byte检验码  计算OutBuff的前9+len个字节
	OutBuff[11+len]=Epilog_Code;												//1Byte结束码，固定16H
	return 12+len;		 														//总的报文长度
}

/*******************************************************************************
名称：INT8U *Judge_NW_Framing(u8 Cmd,u8 *InBuff,u16 Len,u16 *OutLen)
功能：用于判断是否符合南网协议。校验了起始码、装置号码、控制类型码、数据域长度、数据域、
校验码和结束码。注：Cmd为0xff时，任何控制字可通过
入参：INT8U Cmd，控制字，为0xff时，任何控制字可通过校验  *InBuff输入数据  Len输入长度
出参：*OutLen报文实际有效的长度  OutLen输入为0时不输出
返回：0 不符合；非0 符合，且返回数据包首字节地址 INT8U *
*******************************************************************************/
INT8U *Judge_NW_Framing(u8 Cmd,u8 *InBuff,u16 Len,u16 *OutLen)
{
	INT8U 	i;
	INT16U 	Packet_Length;
	INT8U 	NCS; 
	INT8U 	*R;
	/*报文长度判断*/
	if(Len<12)return 0;															//协议基本长度 不能小于12
	for(i=0;i<12;i++)															//检查前面12字节有无包头
	{
		if(InBuff[i]==Start_Code)
		{
			R=&InBuff[i];														//记录起始码位置
			Len-=i;																//起始码开始长度
			break;
		}
	}
	if(i>=12)return 0;															//12字节内未找到包头，认为数据包无效	
	Packet_Length=R[8]<<8;														//R[9]为数据长度的高字节
	Packet_Length+=R[9];														//R[10]为数据长度的低字节 获取数据长度
	Packet_Length+=12;															//整体报文的长度
	if(Len<Packet_Length) return 0;												//协议长度错误（>时可能是包尾有无效数据）
	/*其他判断*/
	if(R[Packet_Length-1]!=Epilog_Code) return 0;								//结束码校验
	if(Cmd!=ANY_CMD && Cmd!=R[7]) return 0;										//控制字校验  注：Cmd为0xff时，任何控制字可通过
	if(memcmp(&R[1],Device_Number,6)) return 0;									//比较6字节装置号码，不相同返回0
	NCS=Negation_CS(R+1,Packet_Length-3);										//累加和取反运算，计算前Packet_Length-3个字节					
	if(NCS==R[Packet_Length-2])													//若读取报文中的NCS值与运算值相同（倒数第2字节为NCS）
	{
		if(OutLen) *OutLen=Packet_Length;										//输出报文长度（OutLen输入为0时不输出）
		return R;																//返回有效协议包头地址
	}
	else return 0;																//NCS校验失败
}

/*******************************************************************************
名称：INT16U Get_DataField(u8 Cmd,u8 *OutBuff)
功能：根据Cmd获取内容（数据域），返回长度
入参：u8 Cmd,控制字
出参：u8 *OutBuff，获取到的内容，用于组帧函NW_framing填充数据域
返回：非0：获取的内容（数据域）长度  0：获取失败
*******************************************************************************/
INT16U Get_DataField(u8 Cmd,u8 *OutBuff)
{
	INT8U	i,len,pack_num=0,fault_state=0;
	INT16U	delta_T=0;
	INT8U*	pointer;

	switch(Cmd)
	{
		case START_UP://00H														//数据域为：规范版本号
				OutBuff[0] = PROTOCOL_VERSION_H;
				OutBuff[1] = PROTOCOL_VERSION_L;
				return 2;														//长度为2
		
		case TIMMING://01H														//数据域为：空
				return 0;														//长度为0
		
		case HEARTBEAT://05H													//数据域为：信号记录时间	信号强度	蓄电池电压
				return Get_HB_INFO(OutBuff);									//获取心跳信息数据，根据南网协议格式传出，并返回长度8字节
		
		case DEMAND_IP://07H													//数据域为：主站IP	端口号	主站卡号
				memcpy(OutBuff,IP_Config.IP_addr_1,4);							
				memcpy(OutBuff+4,IP_Config.PortNum_1,2);
				memcpy(OutBuff+6,IP_Config.CardNum_1,6);
				return 12;														//长度为12

		case DEMAND_CONFG://0AH													//用于主站查询装置配置参数。
				memcpy(OutBuff,Config.BeatTime,10);								//心跳间隔、采集间隔、休眠时长、在线时长、硬件重启时间点
				memset(OutBuff+10,0,10);										//图像相关10字节，清0
				OutBuff[20] = TEM_CUR_UPLOAD;									//有效功能1，导线温度、电流数据监测功能26H
				return 21;														//长度为21
		
		case SLEEP_NOTICE://0CH													//数据域为：空
				return 0;														//长度为0
		
		case DEMAND_DEV_TIM://0DH												//主站主动查询装置系统时间。
				return NW_GetTime((struct NW_TIME *)OutBuff);					//获取RTC时间，并根据南网协议格式传出，并返回长度6字节
		
		case TEM_CUR_UPLOAD://26H												//数据域为：密文认证	帧标识	包数	首包	第一包	第二包……	第N包（仅用于主站请求装置数据后立刻上传）
				memcpy(OutBuff,Config.SecurityCode,4);							//密文认证
				memcpy(OutBuff+4,&Tem_Cur_Data,14);								//数据域除了密文认证，都在Tem_Cur_Data结构体中了，1+1+1+6+2+2+1=14
				return 18;														//长度为18

		case FAULT_INFO://30H	有空把这一堆也整理成函数
				memcpy(OutBuff,Config.SecurityCode,4);							//4Byte密文认证
				OutBuff[4] = 1;													//1Byte帧标识：仅1帧
		
				pointer = (INT8U*)&Fault_Manage;
				for(i=1;i<Fault_Magage_Len;i++)									//结构体中第一个是Need_Report，不需要判断，故i=1开始
				{
					if(0x55==*(pointer+i))										
					{
						fault_state = 0xFF;										//只要有一个标记是55就说明设备处于故障状态
						break;
					}				
				}
		
				for(i=0;i<FI_NUM;i++)											//轮询FI_NUM个故障信息
				{
				/*存在表示需要上报故障信息*/
					if(Fault_Info[i].Time)										
					{
						/*首包（第0包）8Byte*/
						if(!pack_num)
						{
							SecondToNwTime(Fault_Info[i].Time,(struct NW_TIME*)(OutBuff+7));		//6Byte采样时间（年+月+日+时+分+秒）（6字节，HEX表示）
							OutBuff[13] = Fault_Info[i].Function_Code;			//1Byte功能编码
							OutBuff[14] = Fault_Info[i].Fault_Code;				//1Byte故障编码	
						}
						/*第一及以后各故障包 4Byte*/
						else 
						{
							delta_T=Fault_Info[i].Time-Fault_Info[i-1].Time;	//计算与上包采样时间差
							if(delta_T>0xFFFF) delta_T=0xFFFF;					//需要分帧，先不管，固定0xFFFF
							OutBuff[15+7*(i-1)]=(delta_T>>8)&0xFF;				//2Byte采样时间差，高字节
							OutBuff[16+7*(i-1)]=delta_T&0xFF;					//低字节
							OutBuff[17+7*(i-1)]=Fault_Info[i].Function_Code;	//1Byte功能编码
							OutBuff[18+7*(i-1)]=Fault_Info[i].Fault_Code;		//1Byte故障编码	
						}
						
						pack_num++;												//包数
					}	
				}
				
				if(!pack_num) BspUartWrite(2,SIZE_OF("无\r\n"));
				else BspUartWrite(2,SIZE_OF("有\r\n"));
				OutBuff[5] = pack_num;											//1Byte包数：等于故障信息数
				OutBuff[6] = fault_state;										//1Byte设备状态：装置设备当前的状态：00H正常，FFH故障
				return 7+8*(pack_num?1:0)+4*(pack_num?pack_num-1:0);			//密文4，帧数1，包数1，状态1，首包8，后续每包4
				
		case FLOW_DATA_UPLOAD://40H												//数据域为：密文认证	帧标识	包数	首包	第一包	第二包……	第N包
				memcpy(OutBuff,Config.SecurityCode,4);							//密文认证
				OutBuff[4] = 1;													//帧标识：先只按1帧，1首包来做，每次用新数据替换掉
				OutBuff[5] = 1;													//包数：仅一个首包
				memcpy(OutBuff+6,&Flow_Data,18);								//首包放在Flow_Data结构体，6+4+4+4=18
				return 24;														//长度为24
		
	/*方诚扩展协议 Get_DataField*/
		case EX_DEMAND_APN://F4H												//数据域为：APN	0~100字节
				strncpy((char*)OutBuff, (char*)APN, APN_Len);					//复制字符串
				len = strlen((char*)APN)+1;										//数据域长度包括结束符'\0'。
				return len;														//返回数据域长度											
		
		case EX_HEARTBEAT://F5H													//数据域为：记录时间、超级电容电压、环境温度、MCU温度
				return GetExtendedHeartbeatData(OutBuff);						//获取扩展心跳信息数据，根据扩展协议格式传出，并返回长度11字节
					
		case EX_VERSION_SIM://F6H												//数据域为：硬件版本号	软件版本号	SIM卡号（13位）	ICCID（20位）	IMSI（15位）
				return GetDeviceVersionAndCardNumber(OutBuff); 					//获取装置版本及卡号等信息
		
		case EX_UPDATA_REQUEST://F7H											//数据域为：当前软件版本号	升级检测结果
				return GetUpdataRequestData(OutBuff);							//获取升级请求回复帧内容，根据扩展协议格式传出，并返回数据域长度。
		
		case EX_UPDATA_FILLING://FAH											//数据域为：升级软件版本号	补包数	第一包包号	第二包包号	……
				return GetUpdataFillingData(OutBuff);							//获取获取补包回复帧内容，根据扩展协议格式传出，并返回数据域长度。

	/*自定义的。仅用于06H数据不对应错误时，返回出错信息，外面要手动改回LTE_Tx_Buff[7]=cmd*/
		case DATA_UNCORRESPOND://DEH													
				OutBuff[0] = 0x00;												
				OutBuff[1] = 0x00;
				return 2;
		
	/*下面几个进来组帧都表示出错，报FFFF；否则原命令返回（不在这里）*/
		case PASSWORD_ERR:	//DDH，密码错误。也可用于方诚扩展协议。
		case SET_PASSWORD:	//02H		
		case PARA_CONFG:	//03H
		case SET_IP:		//06H
		case RESET_DEV:		//08H
		case SMS_AWAKE:		//09H	
		case FUN_CONFG:		//0BH
		case SMS_SEND:		//0EH
				OutBuff[0] = 0xFF;											
				OutBuff[1] = 0xFF;
				return 2;
		
		default:
				break;
	}
	return 0;
}

/*******************************************************************************
名称：INT8U SetTime(u8 *InBuff)
功能：根据南网协议设置时间。
入参：u8 *InBuff，主站下发的完整的对时帧首地址
出参：无
返回：1：成功   0：失败
*******************************************************************************/
INT8U SetTime(u8 *InBuff)
{
	TCHAR				chars[40];
	INT32U 				second=0;

	second =NwTimeToSecond((struct NW_TIME *)(InBuff+10));						//将上位机下发的南网时间转换为世纪秒
	if(RtcSetTimeSecond(second))												//将时间写入RTC
	{
		Time_Proofread = DONE;													//标记为校时已完成
		sprintf(chars, "校时成功：20%02d年%02d月%02d日 %02d:%02d:%02d\r\n", InBuff[10], InBuff[11], InBuff[12], InBuff[13], InBuff[14], InBuff[15]);
		BspUartWrite(2,(INT8U*)chars,strlen(chars));							//打印当前时间
		return 1;
	}
	Time_Proofread = UNDONE;													//标记为校时未完成
	sprintf(chars, "RTC芯片写入异常，校时失败！\r\n");	
	BspUartWrite(2,(INT8U*)chars,strlen(chars));								//打印校时失败
	return 0;
}

/*******************************************************************************
名称：INT8U NW_GetTime(struct NW_TIME *time)
功能：获取RTC时间，检测RTC故障/恢复，并根据南网协议格式传出，并返回长度6字节。
检测RTC是否有错误/恢复功能，原先放在Task_Wdt_main中，但因为其执行早于铁电信息读取，
因此会产生异常，需要移到铁电读取之后。
入参：struct NW_TIME *time（年+月+日+时+分+秒）（HEX表示）
出参：无
返回：6：成功/长度   0：失败
*******************************************************************************/
INT8U NW_GetTime(struct NW_TIME *time)
{
	if(!RtcGetChinaStdTimeStruct(&gRtcTime)) 									//若获取当前的时间到gRtcTime结构体失败
	{
		if(!Fault_Manage.F_RTC) NW_Fault_Manage(RTC_F, FAULT_STA);				//若当前无故障，写RTC故障产生信息
		return 0;		 														//0：失败
	}	
	else if(Fault_Manage.F_RTC) NW_Fault_Manage(RTC_F, NOFAULT_STA);			//RTC正常，且出过故障。表明RTC恢复了，写入RTC故障恢复信息

	time->year = BcdToHex(gRtcTime.Year);										//年为当前年份减去2000，如2017-2000=17 十六进制
	time->mon = BcdToHex(gRtcTime.Month);										//当前月 十六进制
	time->mday = BcdToHex(gRtcTime.Day);										//当前日 十六进制
	time->hour = BcdToHex(gRtcTime.Hour);										//当前时 十六进制
	time->min = BcdToHex(gRtcTime.Minute);										//当前分 十六进制
	time->sec = BcdToHex(gRtcTime.Second);										//当前秒 十六进制
	return 6;
}

/*******************************************************************************
* Function Name:  INT8U SecondToNwTime(INT32U sencond,INT8U *outbuff)              
* Description  :  世纪秒转换为南网格式时间，需要调整时区。
* Input        :  sencond : 世纪秒
*				  Time	  : 输出的时间数组，顺序为年月日时分秒
* Return       :  无     
*******************************************************************************/
INT8U SecondToNwTime(INT32U sencond,struct NW_TIME *time)
{
	struct tm 	*TTME = 0;														//编译器会自动识别为空指

	sencond += 8*3600;															//世纪秒是0区时间，转换为东八区 ( UTC +8 )
	TTME =localtime(&sencond);													//世纪秒转换为时间数组
	time->year  = TTME->tm_year-100;											//从1900 开始计算
	time->mon  = TTME->tm_mon+1;												//月份加上1(localtime的计算结果由0开始)
	time->mday  = TTME->tm_mday;
	time->hour  = TTME->tm_hour;
	time->min  = TTME->tm_min;
	time->sec  = TTME->tm_sec;

	if((time->year<=99)
		||(0<time->mon<13)
		||(0<time->mday<32)
		||(time->hour<24)
		||(time->min<60)
		||(time->sec<60))	return 1;
	return 0;
}

/*******************************************************************************
* Function Name:  INT32U NwTimeToSecond(struct NW_TIME *time)           
* Description  :  中国标准时间（CST）记录的南网时间，转换为从1970 1.1.0时开始计算的
				  秒数（世纪秒、时间戳）返回。
* Input        :  struct NW_TIME *ptime，南网格式时间
* Return       :  sencond   世纪秒、时间戳
*******************************************************************************/
INT32U NwTimeToSecond(struct NW_TIME *time)
{
	time_t sencond = 0;															//typedef unsigned int time_t;     /* date/time in unix secs past 1-Jan-70 */
	struct tm TTM = {0};      								

	TTM.tm_year = (time->year)+100;  											// 年
	TTM.tm_mon  = (time->mon)-1;   												// 月
	TTM.tm_mday = (time->mday);       											// 日
	TTM.tm_hour = (time->hour);      											// 时
	TTM.tm_min  = (time->min);    												// 分
	TTM.tm_sec  = (time->sec);    												// 秒
	sencond = mktime(&TTM)-8*3600;                  							//东八区 ( UTC +8 )时间转换成世纪秒
	
	if (sencond==0xffffffff) return 0;											//异常
	return sencond;
}

/*******************************************************************************
名称：INT8U Get_HB_INFO(u8 *OutBuff)
功能：获取心跳信息数据，根据南网协议格式传出，并返回长度8字节。
入参：无
出参：u8 *OutBuff，心跳信息数据存放地址
返回：8：成功/长度   0：失败（调用时没用到，先不做了）
*******************************************************************************/
INT8U Get_HB_INFO(u8 *OutBuff)
{
	NW_GetTime((struct NW_TIME *)OutBuff);										//获取当前时间
	Get_Voltage_MCUtemp_Data( 3 );												//获取电池电压数据和单片机温度
	OutBuff[6]=HB_Get_Signal_Strength();										//获取当前信号强度
	OutBuff[7]=(INT8U)(Equipment_state.BAT_Volt*10);							//获取电池电压（10倍）
	return 8;
}

/*******************************************************************************
名称：INT8U Get_NW_Info(void)
功能：从铁电中读取Fault_Info故障信息结构体数组、未上报数据索引表。
入参：无
出参：无
返回：0：失败；1：成功
*******************************************************************************/	
INT8U Get_NW_Info(void)
{
	if(!BSP_ReadDataFromFm(Fault_Manage_Addr,(INT8U*)&Fault_Manage,Fault_Magage_Len)) return 0;			//从铁电中读取Fault_Manage故障信息管理结构体
	memset(Fault_Manage.F_RF,0,110);
	BSP_WriteDataToFm(Fault_Manage_Addr,(INT8U*)&Fault_Manage,Fault_Magage_Len);						//每次上电后将探头故障标志清除，这不属于开机自检（复位自恢复）的项目，如果保留探头故障标志位，可能会导致故障信息错乱（因故障信息上报后就清除了，若不清除标志位，则上电检测到有故障，但却无故障信息可上报）
	
	if(!BSP_ReadDataFromFm(Fault_Info_Addr,(INT8U*)Fault_Info,Fault_Info_Len)) return 0;				//从铁电中读取Fault_Info故障信息结构体数组
	if(!BSP_ReadDataFromFm(Unreport_Index_Addr,(INT8U*)Unreport_Index,Unreport_Index_Len)) return 0;	//未上报数据索引表
	return 1;	
}

/*******************************************************************************
名称：void NW_Fault_Manage(INT8U type, INT8U fault_state)
功能：故障发生和恢复时调用，用于处理相应故障信息，并管理相关标志位。
入参：INT8U fault_state, 故障状态：1恢复，0异常
出参：无
返回：0：失败；1：成功
*******************************************************************************/
void NW_Fault_Manage(INT8U type, INT8U fault_state)
{
	INT8U			i,Function_Code,Fault_Code;
	INT32U			Time;
	char			Buff[256]={0};
	
/*根据传入参数赋值*/
	Time = RtcGetTimeSecond();													//故障时间，世纪秒
	switch(type)
	{
		case RTC_F:
				if(!Time) Time = 1577808000;									//时间异常为0时，写1577808000：2020-01-01 00:00:00。（不能写0，否则会被判断成空闲）
				Function_Code = 0x01;											//01H，主控单元
				Fault_Code = 0x01|fault_state;									//01H，时钟故障/恢复
				if(fault_state) Fault_Manage.F_RTC=0;							//故障恢复标记
				else Fault_Manage.F_RTC=0x55;									//故障错误标记
				strcpy(Buff,"RTC故障"); 
				break;
		
		case STORAGE_F:
				Function_Code = 0x01;											//01H，主控单元
				Fault_Code = 0x02|fault_state;									//02H，存储故障/恢复
				if(fault_state) Fault_Manage.F_STORAGE=0;						//故障恢复标记
				else Fault_Manage.F_STORAGE=0x55;								//故障错误标记
				strcpy(Buff,"存储故障"); 
				break;
		
		case REPLY_F:
				Function_Code = 0x02;											//01H，DTU模块
				Fault_Code = 0x01|fault_state;									//01H，30分钟内主站无应答故障/恢复
				if(fault_state) 
				{
					Fault_Manage.F_REPLY=0;										//故障恢复标记
					wakeup_en.reply = true;										//允许LTE模块从休眠中唤醒
				}
				else 
				{
					Fault_Manage.F_REPLY=0x55;									//故障错误标记
					wakeup_en.reply = false;									//禁止LTE模块从休眠中唤醒
				}
				strcpy(Buff,"主站无应答故障");
				break;
		
		case NETWORK_F:
				Function_Code = 0x02;											//01H，DTU模块
				Fault_Code = 0x02|fault_state;									//01H，30分钟内无法登陆无线网络故障/恢复
				if(fault_state) 
				{
					Fault_Manage.F_NETWORK=0;									//故障恢复标记
					wakeup_en.network = true;									//允许LTE模块从休眠中唤醒
				}
				else 
				{
					Fault_Manage.F_NETWORK=0x55;								//故障错误标记
					wakeup_en.network = false;									//禁止LTE模块从休眠中唤醒
				}
				strcpy(Buff,"无线网络故障"); 
				break;
		
		case BAT_F:
				Function_Code = 0x03;											//03H，电源控制模块
				Fault_Code = 0x01|fault_state;									//01H，蓄电池电源欠压故障/恢复
				if(fault_state) 
				{
					Fault_Manage.F_BAT=0;										//故障恢复标记
					wakeup_en.battle = true;									//允许LTE模块从休眠中唤醒
				}
				else 
				{
					Fault_Manage.F_BAT=0x55;									//故障错误标记
					wakeup_en.battle = false;									//禁止LTE模块从休眠中唤醒
				}
				strcpy(Buff,"电源欠压故障"); 
				break;
		
		default:
				break;
	}
	
	/*探头射频通讯失败，故障标志写入Fault_Manage.F_RF*/
	if(type<=54)						
	{
		Function_Code = Unit_ID_Code[type];										//装置功能单元识别码10H-5FH
		Fault_Code = 0x01|fault_state;											//01H，5个采样周期内无法进行射频通讯故障/恢复
		if(fault_state) Fault_Manage.F_RF[type]=0;								//故障恢复标记
		else Fault_Manage.F_RF[type]=0x55;										//故障错误标记	
		strcpy(Buff,"射频通讯故障");
	}
	
	/*探头收到导线温度数据异常，故障标志写入Fault_Manage.F_TEM*/
	else if(55<=type&&type<=109)	
	{		
		Function_Code =Unit_ID_Code[type-55];									//装置功能单元识别码10H-5FH
		Fault_Code = fault_state;												//02H，导线温度数据异常故障/恢复	原：Fault_Code = 0x02|fault_state;
		if(fault_state==NOFAULT_STA) 
		{
			Fault_Manage.F_TEM[type-55]=0;										//故障恢复标记。若传进来“恢复”，则清对应的故障标记。
			wakeup_en.rf_tem = true;											//允许LTE模块从休眠中唤醒（存在问题：若多个探头同时故障时，有时会唤醒）
		}
		else 
		{
			Fault_Manage.F_TEM[type-55]=0x55;									//故障错误标记。若传进来“故障”，则置对应的故障标记。
			wakeup_en.rf_tem = false;											//禁止LTE模块从休眠中唤醒（存在问题：若多个探头同时故障时，有时会唤醒）
		}
		strcpy(Buff,"温度数据异常故障");
	}		

//		case CUR_F:
//				Function_Code = 功能单元识别码;												//01H，导线侧无线装置	装置功能单元识别码10H-5FH
//				Fault_Code = 0x03|fault_state;									//03H，导线电流数据异常故障/恢复
//				break;

//		case POWER_F:
//				Function_Code = 功能单元识别码;												//01H，导线侧无线装置	装置功能单元识别码10H-5FH
//				Fault_Code = 0x04|fault_state;									//04H，供电不足故障/恢复
//				break;
		
	Fault_Manage.Need_Report=0x55;												//标记有需要进行故障上报(不管是故障发生还是故障恢复都需要上报)
	
	if(Fault_Code & 0x80) strcpy(Buff+strlen(Buff),"已恢复，代码：");			//最高位是1，表示恢复
	else strcpy(Buff+strlen(Buff),"发生！故障代码：");
	sprintf(Buff+strlen(Buff),"%02X %02X\r\n",Function_Code,Fault_Code);
	BspUartWrite(2,(INT8U*)Buff,strlen(Buff));									//打印
	
/*查找空闲结构体并填充*/
	for(i=0;i<FI_NUM;i++)
	{
		if(!Fault_Info[i].Time)													//为0时表示此位置空闲
		{
		/*写全局变量*/
			Fault_Info[i].Time = Time;											//时间已经异常，不能写0，否则会被判断成空闲
			Fault_Info[i].Function_Code = Function_Code;						//功能编码
			Fault_Info[i].Fault_Code = Fault_Code;								//故障编码
		/*写入铁电*/
			BSP_InitFm(Fault_Num);												//调用低功耗函数后需要重新初始化
			BSP_WriteDataToFm(Fault_Manage_Addr,(INT8U*)&Fault_Manage,Fault_Magage_Len);	//写Fault_Manage到铁电
			BSP_WriteDataToFm(Fault_Info_Addr,(u8*)&Fault_Info,Fault_Info_Len);	//写Fault_Info[]到铁电
			FM_LowPower(Fault_Num);												//铁电引脚低功耗配置
			break;
		}
	}
}

/*******************************************************************************
名称：void NW_DeviceNumberToAscii(INT32U InData,INT8U *pOut)
功能：将滚码等16进制表示的装置编码，转换成4字节长度ASCII码。
入参：INT16U InData，16进制表示的装置编码
出参：INT8U *pOut，输出的ASCII码
返回：无
*******************************************************************************/
void NW_DeviceNumberToAscii(INT32U InData,INT8U *pOut)
{
	INT8U				gTemp = 0, i = 0;

	if(InData>9999) return;														//超出编码范围
	for(i = 3; i != 0xff; i--)
	{
		gTemp = InData%10;     													//取个位
		pOut[i] =  gTemp + 0x30;
		InData -= gTemp;
		InData = InData/10;														//去掉个位
	}
}
