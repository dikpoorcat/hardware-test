#ifndef __SysConfigVC_H
#define __SysConfigVC_H

#include "ucos_ii.h"
//#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_nvic.h"
#include "stm32f10x_systick.h"
#include "stm32f10x_wwdg.h"
#include "stm32f10x_iwdg.h"
#include "Bsp_fm.h"     // zzs add this include
#include "MemConfig.h"  // zzs add this include
#include "NRTC.h"








/*******************************************************************************
* Function Name  : void RCC_Configuration4M(void)
* Description    : 配置系统时钟为4M
* Input          : None		  
* Output         : None
* Contributor: 
* Date First Issued: 
*******************************************************************************/
void RCC_Configuration4M(void);

 
 /***************************************************************************************
** 函数名称: Tmr_TickInit
** 功能描述: OS tick 初始化函数
** 参    数: None
** 返 回 值: None       
** 作　  者: 罗辉联
** 日  　期: 2007年11月28日
****************************************************************************************/
void Tmr_TickInit (unsigned int fhz);

/*******************************************************************************
* Function Name  : void RCC_Configuration32M(void)
* Description    : 配置系统时钟为32M
* Input          : None		  
* Output         : None
* Contributor: 
* Date First Issued: 
*******************************************************************************/
void RCC_Configuration32M(void);

void NVIC_Configuration(void);

void WWdg_Init(void);

void McuSoftReset(void);

 /*******************************************************************************
* Function Name  : void IWDG_Init(void)
* Description    : 内部看门狗启动，要求LSI 内部时钟已经启动
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void IWDG_Init(void);

/*******************************************************************************
* Function Name  : void IWDG_Reset(void)
* Description    : 内部看门狗复位
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void IWDG_Reset(void);

#endif        // end of file
