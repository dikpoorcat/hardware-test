#ifndef	__Local_Protocl_H
#define	__Local_Protocl_H
#include "main.h"


/* --------------------------------Private define---------------------------------------------*/
/*通用*/
#define DONE				1													//已完成某种动作
#define UNDONE				0													//未完成某种动作

/*复位*/
#define HOTRST				0xAA												//热复位，由程序控制复位
#define COLDRST				0xBB												//冷复位，意外复位
#define FAULTRST			0xCC												//故障复位，特指LTE发射过程中复位，一般为冷复位
#define NORST				0xFF												//用于非复位情况下跳过开机联络

/*常量*/
#define Local_Task_Prio		LOCAL_PRIO
#define Buff485_LEN 		1500
#define BAT_UNDER			9.2
#define BAT_UP				9.5
#define FALA_UNDER			5
#define FALA_UP				5.5

/*本地协议DoType*/
#define READTYPE			1
#define WRITETYPE     		0

/*本地协议错误代码*/
#define No_Err				00													//无错误
#define Invalid_ID			01													//ID错误无效ID ID列表中的，ID并不是所有的设备都支持
#define Password_Err		02													//密码错误【错误3次后设备锁住，隔天自动解锁】
#define Data_Err			03													//ID对应数据错误
#define Write_Err			04													//ID为只读不可写
#define Read_Err			05													//ID为只写不可读
#define Other_Err			0xFF												//本地协议中这些错误代码基本没用的，统一使用其他错误就行了


/* --------------------------------Private typedef--------------------------------------------*/
struct LOCAL_PROTOCAL
{
	INT8U  Addr[4];																//本机装置地址，默认为0x00, 0x00, 0x00, 0x00
	INT8U  DoType;  															//0 写入  1 读取==============CMD
	INT8U  *Pointer;
	INT8U  Password[3];															//本机密码，默认为0x00, 0x00, 0x00
	INT16U CMD;																	//CMD命令
	INT16U CMDlen;																//CMD命令对应参数的数据长度
};

struct HARDWARE_TEST															//存放各硬件检测结果
{
	INT8U					b485;												//485
	INT8U					encryption_chip;									//加密芯片
	INT8U					ferroelectric_ram;									//铁电存储芯片
	INT8U					flash_memory;										//外部FLASH存储芯片
	INT8U					rtc;												//RTC
	INT8U					power_supply;  										//电源
	INT8U					ds18b20;   											//DS18B20
	INT8U					lora;   											//LORA模块
	INT8U					meteorology;										//微气象
	INT8U					lte;												//LTE模块
};

/* ------------------------------Private variables--------------------------------------------*/
extern struct LOCAL_PROTOCAL	Local_Protocal;
extern INT8U					Reset_Flag;    									//冷复位（默认值）、热复位、发送中复位
extern INT8U					Reset_Count;          							//冷启动计数标识

extern INT8U					B485BUF[Buff485_LEN];
extern OS_EVENT  				*GyBOX;

extern BYTE						work[FF_MAX_SS];								/* Work area (larger is better for processing time) */   //FF_MAX_SS
extern FIL						fil;            								/* File object */
extern FATFS					fs;												/* Filesystem object */


/* -----------------------------Private functions---------------------------------------------*/
void Task_Local_main(void *arg);
INT8U Check_Reset_Mod(u8 waittime);
INT8U Wait_Local_Comm(INT8U WaitTime);
INT8U Local_Protocol_Process(INT8U *In,INT16U inLen,INT8U *pUseBuff);
INT8U * Judge_Local_Framing(INT8U *pInBuff,INT16U Len,INT16U *pOutLen);
INT8U Set_Local_Para(INT16U CMD,INT8U *pInBuff,INT8U Len);
INT8U Get_Local_Para(INT16U CMD,INT8U *pOutBuff,INT16U *pOutLen);
INT8U System_Time_Fun(INT8U *pInOutBuff,INT8U Type);
INT8U Judge_Device_Addr(INT8U *pAddr);
INT8U Local_Function(INT8U *InBuff);
void Local_Protocol_Reply(INT8U Err,INT16U CMD,INT8U *pUseBuff);
void Local_Protocol_Reply_Para(INT8U *In,INT16U len,INT16U CMD,INT8U *pUseBuff);
INT8U Add_TT_ID(INT8U *pInBuff)	;
INT8U Delete_TT_ID(INT8U *pInBuff);
INT8U Delete_All_TT_ID(void);
INT8U Read_TT_Num_Or_ID(INT16U ID,INT8U *pOutBuff);
void FM_Space_Usage(void);
void Print_Config(INT8U cmd);
void DrawSysLogo(void);
void DrawErrLogo(void);
void HardwareTest(void);

/*FATFS应用*/
FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
);
FRESULT Dir_Maintenance (void);
void Dir_Test(void);
void Check_Getfree(void);
void File_Test(void);

#endif

