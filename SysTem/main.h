#ifndef __main_H
#define __main_H
/*STD*/
#include <time.h> 
#include "string.h"
#include "stdbool.h"
#include "stdio.h"	
/*UCOSII*/
#include "ucos_ii.h"
#include "os_cpu.h"
/*STM32*/
#include "stm32f10x_type.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_crc.h"
#include "stm32f10x_dbgmcu.h"
#include "stm32f10x_map.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_flash.h"
/*CONFIG*/
#include "MemConfig.h"
#include "SysConfigVC.h" 
/*BSP*/
#include "Bsp_NFlash.H"
#include "Bsp_adc.h" 
#include "Bsp_W25Q256.h"  
#include "Bsp_fm.h"  
#include "bsp_WDG.h"
#include "bsp_RTC.h"
#include "Bsp_DS18B20.h"
#include "Bsp_UART.h"
#include "NRTC.h"
#include "CRC.h"
#include "NumToAscii.h"
/*APP*/
#include "ff.h"			/* Declarations of FatFs API */
#include "Local_Protocl.h" 
#include "LTE.h"
#include "ME909S.h"
#include "NW_protocol.h"
#include "Extend_Protocol.h"
#include "RF.h"
#include "delay.h"




/* ----------------------Private define---------------------------------------------------------------------------*/
#if SYS==1
#define B485DIS
#endif

#define SIZE_OF(a)			(INT8U *)a,strlen(a) 								//用于需要传入长度的AT指令函数，减少书写量
	
#define	WDT_PRIO 			3
#define	RF_PRIO	 			4
#define	LTE_PRIO			5
#define	LOCAL_PRIO			6

#define RF_ACT				0x01
#define RF_INACT			0xFE
#define LTE_ACT				0x02
#define LTE_INACT			0xFD
#define Local_ACT			0x04
#define Local_INACT			0xFB

#define	WDT_STK_SIZE 		64
#define	RF_STK_SIZE	 		256
#define	LTE_STK_SIZE		256
#define	Local_STK_SIZE		256



#define Wdt_Task_Prio		WDT_PRIO
#define RWNUM 				3													//由4改为3，删除RTC任务

/*设备状态、状态指令定义*/
#define WAKE_STAT			0X01
#define SLEEP_STAT			0X02
#define WAKE_CMD			0XF1
#define SLEEP_CMD			0XF2
#define UPLOAD_CMD			0XF3
#define DATA_CMD			0XF4
#define FAULT_CMD			0XF5
#define WAKE_SUCCESS		0X84
#define SLEEP_SUCCESS		0X85

//-----------------------------------LED控制引脚----------------------------------------
#define Led_PIN				GPIO_Pin_3
#define Led_Port			GPIOC



/* ----------------------Private variables------------------------------------------------------------------------*/
extern INT8U				Time_proofreading;
extern INT8U				LTE_Sending_Flag;
extern INT8U				Reset_Flag;
extern INT8U				Reset_Flag;
extern OS_EVENT				*Dev_CMDB0X;
extern OS_EVENT				*Dev_STAB0X;
extern OS_EVENT				*Data_CMDB0X;
extern OS_EVENT				*Fault_CMDB0X;
extern INT8U				StopModeLock;
extern INT8U				TaskActive;




/* --------------------Private function prototypes----------------------------------------------------------------*/
void WDTSscn(void);
void WDTClear(unsigned char wdtindex);
void Task_Wdt_main(void *org);
void Feed_Dog(void );
void IO_LowPower(void);
INT8U DevStatCtr(u8 stat);
void Led_Init(void);
void Led_On(void);
void Led_OFF(void);
void Reset_On_Time(struct BSPRTC_TIME *pTime);
void SysJudgeAndMarkBkp(void);
#endif
