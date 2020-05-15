#ifndef _NRTC_H
#define _NRTC_H
#include "stm32f10x_nvic.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_pwr.h"
#include "SysConfigVC.h" 
#include "stm32f10x_bkp.h"

#define NRTC_Fre 20    //设置内部RTC频率  此频率应为系统频率的整数倍  如20 40 80 ，规定最长不能超过1s，也即是1（安全范围是）

INT8U RTC_Init(INT16U Fre);
void RTC_EXTI_INITIAL(FunctionalState interrupt_en_or_dis);
void RTC_SET_ALARM(u32 sec);
void RTC_AWU_SET(void);


#endif
