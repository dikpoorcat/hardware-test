#ifndef	__Bsp_fm_H
#define	__Bsp_fm_H
/***********************************************************************************************************************                                    
*
*               (c) Copyright 2017-2030, 杭州方诚电力技术有限公司(http://www.fcdl.com.cn)
*                            All Rights Reserved
*
*---------- File Info ---------------------------------------------------------------
* File name   :  Bsp_Fm.h
*
* Descriptions:  This file define all the function used to control FM25CL64
*
* Created By  :  赵志舜(11207656@qq.com)
* Created date:  2018.1.23
*
*---------- History Info -------------------------------------------------------------
* Version: 			Ver1.0
* Descriptions: FM25CL64和W25Q256共用同一个SPI接口	
*
*-------------------------------------------------------------------------------------
************************************************************************************************************************/

#include "bsp_spi.h"

/*任务占用标志位（FM与WQ256使用）*/
#define	LTE_Num		0
#define	RF_Num		1
#define	LOC_Num		2
#define	WDT_Num		3
#define	FF_Num		4															//FatFs文件系统专用
#define Fault_Num	5															//故障信息存储专用



/* --------------------------------Private define---------------------------------------------*/
/* Note：使用读取或者写入的时候，要进行 open 和 close 的操作 */
#define MAX_FM_LEN 0x2000
#define FM_WREN 0x06
#define FM_WRDI 0x04
#define FM_RDSR 0x05     // 读取状态命令
#define FM_WRSR 0x01
#define FM_READ 0x03
#define FM_WRITE 0x02

#define SCK_FM_PIN     GPIO_Pin_4
#define SCK_FM_Port    GPIOC

#define MOSI_FM_PIN    GPIO_Pin_7
#define MOSI_FM_Port   GPIOA

#define MISO_FM_PIN    GPIO_Pin_5  // MISO
#define MISO_FM_Port   GPIOA     

#define NCS_FM_PIN    GPIO_Pin_4   // zzs note,铁电片选线
#define NCS_FM_Port   GPIOA

#define NWP_FM_PIN 	  GPIO_Pin_6   // zzs note,铁电写保护线
#define NWP_FM_Port   GPIOA

/* ------------------------------Private macro------------------------------------------------*/
#define  FMCS_H()   	GPIO_SetBits(NCS_FM_Port, NCS_FM_PIN)
#define  FMCS_L()   	GPIO_ResetBits(NCS_FM_Port, NCS_FM_PIN)
 
#define  FMWP_H()	    GPIO_SetBits(NWP_FM_Port, NWP_FM_PIN)
#define  FMWP_L()	    GPIO_ResetBits(NWP_FM_Port, NWP_FM_PIN)

#define  FMSCK_H()		GPIO_SetBits(SCK_FM_Port, SCK_FM_PIN)
#define  FMSCK_L()		GPIO_ResetBits(SCK_FM_Port, SCK_FM_PIN)

#define  FMMOSI_H()		GPIO_SetBits(MOSI_FM_Port, MOSI_FM_PIN)
#define  FMMOSI_L()		GPIO_ResetBits(MOSI_FM_Port, MOSI_FM_PIN)

#define  FMMISO()		GPIO_ReadInputDataBit(MISO_FM_Port,MISO_FM_PIN)
 






/*函数声明*/
void Fmdelay(INT16U US);
INT8U FmReadState(void);
void BSP_InitFm( INT8U Task_Num );
INT8U BSP_WriteDataToFm(INT16U FlashAddr,INT8U *pDataAddr,INT32U Len);
INT8U BSP_ReadDataFromFm(INT16U FlashAddr,INT8U *pDataAddr,INT32U Len);
INT8U FM_test(void);
void FM_LowPower( INT8U Task_Num );
INT8U BSP_FM_Erase(INT16U FlashAddr,INT32U Len);
#endif

