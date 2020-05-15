#ifndef _NRTC_H
#define _NRTC_H
#include "stm32f10x_nvic.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_pwr.h"
#include "SysConfigVC.h" 
#include "stm32f10x_bkp.h"

#define NRTC_Fre 20    //�����ڲ�RTCƵ��  ��Ƶ��ӦΪϵͳƵ�ʵ�������  ��20 40 80 ���涨����ܳ���1s��Ҳ����1����ȫ��Χ�ǣ�

INT8U RTC_Init(INT16U Fre);
void RTC_EXTI_INITIAL(FunctionalState interrupt_en_or_dis);
void RTC_SET_ALARM(u32 sec);
void RTC_AWU_SET(void);


#endif
