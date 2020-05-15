#ifndef __NW_PROTOCOL_H
#define __NW_PROTOCOL_H

#include "main.h"


/*宏定义*/
#define PROTOCOL_VERSION_H	0x03												//规范版本号：第一字节代表规范主版本号，第二字节代表规范次版本号，如0200H，代表V2.0版本。
#define PROTOCOL_VERSION_L	0x00												//当前版本号为V3.0。
#define FI_NUM				30													//最大故障信息数  每个就是6字节
#define FAULT_STA			0x00												//故障时最高位为0
#define NOFAULT_STA			0x80												//故障恢复时最高位为1
/*自定义故障type*/
/*
	0~54····代表5个采样周期内无法进行射频通讯（55个探头）				01H错误
	55~109···代表导线温度数据异常（55个探头）							02H错误
*/
#define RTC_F				110													//时钟异常/恢复
#define STORAGE_F			111													//存储异常/恢复
#define REPLY_F				112													//30分钟内主站无应答/恢复
#define NETWORK_F			113													//30分钟内无法登陆无线网络/恢复
#define BAT_F				114													//蓄电池电源欠压/恢复
//#define RF_F				6													//5个采样周期内无法进行射频通讯/恢复
//#define TEM_F				7													//导线温度数据异常
//#define CUR_F				8													//导线电流数据异常
//#define POWER_F			9													//导线侧无线装置供电不足

#define Start_Code			0x68
#define Epilog_Code			0x16

#define START_UP			0x00
#define TIMMING				0x01
#define SET_PASSWORD		0x02
#define PARA_CONFG			0x03
#define HEARTBEAT			0x05
#define SET_IP				0x06
#define DEMAND_IP			0x07
#define RESET_DEV			0x08
#define SMS_AWAKE			0x09
#define DEMAND_CONFG		0x0A
#define FUN_CONFG			0x0B
#define SLEEP_NOTICE		0x0C
#define DEMAND_DEV_TIM		0x0D
#define SMS_SEND			0x0E
#define DATA_REQUEST		0x21
#define TEM_CUR_UPLOAD		0x26
#define FAULT_INFO			0x30
#define FLOW_DATA_UPLOAD	0x40
#define FILE_LIST_QUERY		0x71
#define FILE_REQUEST		0x72
#define FILE_UL_REQUEST		0x73
#define FILE_UPLOAD			0x74
#define FILE_UL_END			0x75
#define FILE_FILLING		0x76
/*方诚扩展协议*/
#define EXTEN_ONOFF			0xF0
#define EX_SET_APN			0xF3
#define EX_DEMAND_APN		0xF4
#define EX_HEARTBEAT		0xF5
#define EX_VERSION_SIM		0xF6
#define EX_UPDATA_REQUEST	0xF7
#define EX_UPDATA_DOWNLOAD	0xF8
#define EX_UPDATA_DL_DONE	0xF9
#define EX_UPDATA_FILLING	0xFA
#define EX_FORMAT_FLASH		0xFB
/*自字义指令，不用于通信*/
#define ANY_CMD				0xFF
#define DATA_UNCORRESPOND	0xDE
#define PASSWORD_ERR		0xDD
#define REC_AND_EXE			0xDC
#define QUERY_MAIL			0xDB


/*文件类型码定义*/
#define JPEG_FILE			0x01												//图像文件类型
#define FLW_FILE			0x02												//故障定位波形文件类型
#define ANY_FILE			0xFF												//所有文件类型


/*大小端转换*/
#define htons(A) \
	((((u16)(A) & 0xff00) >> 8 ) | \
	 (((u16)(A) & 0x00ff) << 8 ))
#define htonl(A) \
	((((u32)(A) & 0xff000000) >> 24) | \
	 (((u32)(A) & 0x00ff0000) >> 8 ) | \
	 (((u32)(A) & 0x0000ff00) << 8 ) | \
	 (((u32)(A) & 0x000000ff) << 24))
#define ntohs     htons
#define ntohl     htohl



/*结构体定义*/

#pragma pack(1)	//设定为1字节对齐

struct NW_TIME 																	//南网格式时间结构体 （年+月+日+时+分+秒）（6字节，HEX表示）
{
	INT8U	year;																//年份，为当前年份减去2000
	INT8U	mon;																//月，1-12
	INT8U	mday;																//日，1-31
	INT8U	hour;																//时，0-23
	INT8U	min;																//分，0-59
	INT8U	sec;																//秒，0-59
};

struct NW_CONFIG		/*参数配置结构体*/
{
	INT8U  Password[4];															//初始密码
	INT8U  BeatTime[1];															//心跳间隔，装置心跳信息发送间隔，单位分钟
	INT8U  ScanInterval[2];														//采集间隔，即每隔多少分钟采样一次（采集间隔与拍照时间无关），单位分钟
	INT8U  SleepTime[2];														//休眠时长，数据采集功能关闭或通信设备休眠时间，该时间内可支持短信或网络唤醒；单位分钟，若为0则装置不休眠
	INT8U  OnlineTime[2];														//在线时长，通信设备保持数据采集及网络通信设备在线时间，单位分钟
	INT8U  ResetTime[3];														//硬件重启时间点，为保证装置软件可靠运行装置应支持定时重启，时间点格式：日，时，分。日：0到28日；（若日为00H则就每天定时重启）
	INT8U  SecurityCode[4];														//密文验证码，为确认装置数据的正确性，防止非法用户恶意欺骗服务器。该密文用于防止非法装置用户的数据被主站认可，安装时装置设定默认密文，上塔安装完成后，主站下发指令修改该装置密文，仅装置主站记录的密文一致时视该数据合法有效，否则屏蔽。
};

/*------------------------------------------------------------
密码	主站IP	端口号	主站IP	端口号	主站卡号	主站卡号
4字节	4字节	2字节	4字节	2字节	6字节		6字节		
------------------------------------------------------------*/
struct NW_IP_CONFIG		/*主站IP地址、端口号和卡号配置结构体*/
{
	INT8U	IP_addr_1[4];														//主站IP：标准4字节IP
	INT8U	PortNum_1[2];														//端口号：高字节乘以256加上低字节
	INT8U	IP_addr_2[4];														//主站IP有两组的！！！
	INT8U	PortNum_2[2];														//端口号有两组的！！！
	INT8U	CardNum_1[6];														//主站卡号：为F加通信卡号，每个数字占半个字节。如卡号为13912345678，则发送数据为：F1H，39H，12H，34H，56H，78H
	INT8U	CardNum_2[6];														//主站卡号有两组的！！！
};

struct NW_FLOW_DATA
{
	struct NW_TIME	Sample_Time;												//包采样时间（年+月+日+时+分+秒）（6字节，HEX表示）
	INT8U			Day_Used[4];												//当日已用流量（4字节）
	INT8U			Month_Used[4];												//当月已用流量（4字节）
	INT8U			Month_Surplus[4];											//当月剩余流量（4字节）
};

struct NW_TEM_DATA
{
	INT8U			Frame_ID;													//帧标识（1字节）
	INT8U			Pack_Num;													//包数（1字节）
	INT8U			Unit_ID;													//功能单元识别码（1字节）
	struct NW_TIME	Sample_Time;												//包采样时间（年+月+日+时+分+秒）（6字节，HEX表示）
	INT8U			Tem[2];														//测点温度（2字节）
	INT8U			Cur[2];														//导线电流（2字节）
	INT8U			Voltage;													//传感器工作电压（1字节）
};

struct NW_FAULT_INFO
{
	INT32U	Time;																//世纪秒，故障/恢复时间
	INT8U	Function_Code;														//功能编码
	INT8U	Fault_Code;															//故障编码
};

struct LOCAL_FLOW_DATA
{
	INT8U	Month;																//月份记录
	INT8U	Date;																//日期记录
	INT32U	Flow_Day_Used_B;													//当日已使用流量，单位：字节
	INT32U	Flow_Month_Used_B;													//当月已使用流量，单位：字节
};

struct NW_FAULT_MANAGE 															//故障信息管理结构体 各类型故障：0x55标记故障发生且未恢复，恢复后清零
{
	INT8U	Need_Report;														//表明是否有故障信息（故障恢复信息）需要上报 ，上报完成后清除
	INT8U	F_RTC;																//主控单元时钟异常
	INT8U	F_STORAGE;															//主控单元存储异常
	INT8U	F_REPLY;															//DTU模块30分主站无应答
	INT8U	F_NETWORK;															//DTU模块30分钟无法登陆无线网络
	INT8U	F_BAT;																//电源控制模块蓄电池电源欠压
	INT8U	F_RF[55];															//探头5个采样周期内无法进行射频通讯（无法采集到有效数据）
	INT8U	F_TEM[55];															//探头导线温度数据异常
};

#pragma pack()	//设定为1字节对齐


/*全局变量声明*/
extern const INT8U				Unit_ID_Code[55];									//功能单元识别码表
extern struct NW_CONFIG			Config;
extern struct NW_IP_CONFIG		IP_Config;
extern struct NW_FLOW_DATA		Flow_Data;
extern struct NW_FAULT_MANAGE	Fault_Manage;									//故障管理结构体										
extern struct NW_FAULT_INFO		Fault_Info[FI_NUM];									//故障信息结构体数据，最多存储FI_NUM个故障
extern struct LOCAL_FLOW_DATA	Local_FLow_Data;
extern INT8U 					Unreport_Index[31][3];								//未上报数据索引表，31天，每小时1bit（最高字节代表0时）：1已上报，0未上报
extern INT8U 					Device_Number[6];									//6Byte装置号码
extern INT8U 					FUN_Config[24];										//24Byte功能配置参数，最多24项功能，默认无效
extern INT8U					Time_Proofread;
extern INT8U					APN[100];											//南网扩展协议增加APN配置



/*函数声明*/
INT8U NW_Comm_Process(void);
INT8U Startup_Comm(u8 times, u16 timeout);
INT8U Timming_Request(u8 times, u16 timeout);
INT8U *Heartbeat(u8 times, u16 timeout);
INT8U Fault_Info_Comm(u8 times, u16 timeout);
INT8U Set_Password_Comm(u8 *InBuff, u16 Len);
INT8U ParaConfigComm(u8 *InBuff, u16 Len);
INT8U Set_IP_Comm(u8 *InBuff, u16 Len);
INT8U Fun_Config_Comm(u8 *InBuff, u16 Len);
INT8U SMS_Send_Comm(u8 *InBuff, u16 Len);
INT8U SMS_Judge(u8 *InBuff, u16 Len);
INT8U Data_Request_Comm(u8 *InBuff, u16 Len);
INT8U Flow_Data_Upload(u8 times, u16 timeout);
INT8U Sleep_Notice(void);
INT8U Tem_Cur_Upload(u8 times, u16 timeout);
INT8U Tem_Cur_Sample_Upload(u8 times, u16 timeout);
INT16U NW_History_Temp_Comm(u16 Bit_Index, u8 times, u16 timeout);
INT16U Search_Info(u8 *InBuff, u16 Len);
//INT16U NW_History_Temp_Framing(u8 Frame_ID, u8 *InBuff, struct SAMP_MANAGE *Info_str, u8 *OutBuff);
void Tem_DS18B20_To_NW(INT8U* Outbuff,INT8U* InBuff);
INT8U File_List_Query_Comm(u8 *InBuff, u16 Len);
INT8U Files_Upload(u8 *FileName,u16 Des_Len);
INT16U NW_File_Framing(u8 Cmd,u8 *FileName,u8 Pac_Num,u8 Des_Len,u8 *OutBuff);
INT8U NW_ReceiveAndExecute(u8 *OutBuff,u16 timeout_s);
bool PassworkCheckAndReport(INT8U *Inbuff);
INT16U NW_Framing(u8 Cmd,u8 *OutBuff);
INT8U *Judge_NW_Framing(u8 Cmd,u8 *InBuff,u16 Len,u16 *OutLen);
INT16U Get_DataField(u8 Cmd,u8 *OutBuff);
INT8U SetTime(u8 *InBuff);
INT8U NW_GetTime(struct NW_TIME *time);
INT8U SecondToNwTime(INT32U sencond,struct NW_TIME *time);
INT32U NwTimeToSecond(struct NW_TIME *ptime);
INT8U Get_HB_INFO(u8 *OutBuff);
INT8U Get_NW_Info(void);
void NW_Fault_Manage(INT8U type, INT8U fault_state);
void NW_DeviceNumberToAscii(INT32U InData,INT8U *pOut);

#endif
