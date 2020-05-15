#ifndef __RF_H
#define __RF_H
#include "main.h"


/*宏定义*/
#define RF_BuffLen 			500
#define RF_Task_Prio		RF_PRIO												//RF_Main_Task的任务优先级

#define Sample_Wait_Time	60*5												//上电采样等待时间，单位：秒

#define	Tem_Upper			1250												//量程上限，125度	(0xf800>Temp)&&(Temp>Tem_Upper)
#define	Tem_Lower			0xfa26												//量程下限，-55度	Temp>Tem_Lower


#define	RF_READ_CMD 															/*读串口模块命令*/	\
{\
	0xff,0xff,0xff,0xff,0xff,	\
	0xff,0xff,0xff,0xff,0xff,	\
	0xff,0xff,0xff,0xff,0xff,													/*用于唤醒串口模块*/	\
	0xAA,0x2D,0x01,0x00,0xD8,0xBB,												/*射频模块读取指令*/	\
	0x0d,0x0a																	/*串口结束标志*/		\
}


//-----------------------------------RF电源控制引脚----------------------------------------
#define PWRF_PIN			GPIO_Pin_2
#define PWRF_Port			GPIOD              									//小无线输出
#define PWRFDIS()			GPIO_ResetBits(PWRF_Port,PWRF_PIN);
#define PWRFEN()			GPIO_SetBits(PWRF_Port,PWRF_PIN);
#define PWRF_Port_CK		RCC_APB2Periph_GPIOD

#pragma pack(1)//设定为1字节对齐

/*结构体声明*/
struct TT_STR{
	INT8U TT_Count;																//录入探头的个数
	INT8U TT_ID[55][2];															//分配55个探头ID，每个为两个INT8U长度
	INT8U HaveTT[55];															//标记探头索引位置是否有录入探头
};

struct SAMP_MANAGE{
	INT8U 	Len;																//每个探头的数据长度，(2*Sample_Num+5)字节
	INT8U	TT_Count;															//录入探头的个数
	INT8U 	Sample_Num;															//实际采样次数，由程序统计
//	INT8U   Report_Num;															//记录本小时的采样数据已上报个数，Report_Num<=Sample_Num;当每个小时存储数据到WQ256时，若Report_Num=Sample_Num，则标记该小时文件已上传完成，若未完成，则在下次上报时读取文件由Report_Num后开始续传
	INT32U 	Time[60];															//每次的采样时间，世纪秒，根据南网协议每个小时最多采60次
	INT16U 	crc;
	INT8U	Newline[2];															//固定0x0D 0x0A
};

struct LteWakeupEnable{															//true使能，false禁止
	bool	overtime;															//上报故障无回复。若2分钟收不到确认信号再次发送，发送3次不成功后，停止发送，待下次发送采集数据成功后再次传送。
	bool	reply;																//主站无应答故障，禁止唤醒LTE
	bool	network;															//无线网络故障，禁止唤醒LTE
	bool	battle;																//电源欠压故障，禁止唤醒LTE
	bool	rf_tem;																//射频温度数据异常故障（400+、超量程等），禁止唤醒LTE
};

#pragma pack()//设定为1字节对齐

/*全局变量声明*/
extern struct TT_STR				TT_Info;
extern struct SAMP_MANAGE			TT_Sample_Manage;
extern struct LteWakeupEnable 		wakeup_en;									//当通信故障、无网络、电源欠压等情况下禁止从休眠模式唤醒。

extern INT8U				First_Flag;
extern INT16U				HT_Data[2];											//保存温湿度数据，湿度，温度=================有用吗？？？没用清理掉
extern INT8U				FATFS_Lock;


/*函数声明*/
void Task_RF_Main(void *arg);
void RF_Receive_Data(u32 rate, u16 timeout);
void RF_Power_Init(void);
void RF_Uart_init(unsigned int rate);
void RF_LowPower(void);
INT8U CMP_TT_ID(INT8U* pIn);
void Read_TT_From_FM(void);
void History_Data_Store(void);
void RF_Data_Sample(u16 timeout);
void RF_Fault_Judge(INT8U Index);
#endif
