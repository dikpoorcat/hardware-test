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
* Description    : ����ϵͳʱ��Ϊ4M
* Input          : None		  
* Output         : None
* Contributor: 
* Date First Issued: 
*******************************************************************************/
void RCC_Configuration4M(void);

 
 /***************************************************************************************
** ��������: Tmr_TickInit
** ��������: OS tick ��ʼ������
** ��    ��: None
** �� �� ֵ: None       
** ����  ��: �޻���
** ��  ����: 2007��11��28��
****************************************************************************************/
void Tmr_TickInit (unsigned int fhz);

/*******************************************************************************
* Function Name  : void RCC_Configuration32M(void)
* Description    : ����ϵͳʱ��Ϊ32M
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
* Description    : �ڲ����Ź�������Ҫ��LSI �ڲ�ʱ���Ѿ�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void IWDG_Init(void);

/*******************************************************************************
* Function Name  : void IWDG_Reset(void)
* Description    : �ڲ����Ź���λ
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void IWDG_Reset(void);

#endif        // end of file
