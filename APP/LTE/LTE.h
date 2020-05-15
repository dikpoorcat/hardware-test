#ifndef __LTE_H
#define __LTE_H

#include "main.h"



/*宏定义*/
#define LTE_Task_Prio 			LTE_PRIO    									//LTE任务优先级
#define LTE_Send_Event_Bit 		0x0001
#define LTE_Send_Holding_Bit	0x0002

#define RETRY				3													//3次
#define TIMEOUT				10*20												//10*20个时间片，共10秒
#define LTE_BUFF_LEN		1500												//控制了4个全局变量的空间
#define DES_LEN				1024												//文件上报时组帧长度，可按需要更改
#define MONTHLY_FLOW		30													//每月流量，单位：M，可按实际套餐更改
#define MF_0				(MONTHLY_FLOW>>24)&0xFF								//高字节
#define MF_1				(MONTHLY_FLOW>>16)&0xFF	
#define MF_2				(MONTHLY_FLOW>>8)&0xFF	
#define MF_3				MONTHLY_FLOW&0xFF									//低字节
																				


/*全局变量声明*/
extern INT8U 				LTE_Buff[LTE_BUFF_LEN];								//用于组帧
extern INT8U 				LTE_Tx_Buff[LTE_BUFF_LEN];							//用于LTE发送
extern INT8U 				LTE_Rx_Buff[LTE_BUFF_LEN];							//用于LTE接收
extern INT32U				Net_Fault_Time;										//用于网络连接失败计时，当计时满30分钟时产生一次网络连接错误故障上报，重新连上时将会产生网络连接恢复上报
extern INT32U 				Host_No_Reply_Time;									//用于主站无应答计时，当计时满30分钟时产生一次主站无应答错误故障上报，再次发送讯息得到回复时产生故障恢复上报

/*函数声明*/
void Task_LTE_Main(void *arg);
void Sleep_Or_Reset(u8 CMD);
#endif
