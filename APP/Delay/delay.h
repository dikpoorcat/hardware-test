#ifndef __DELAY_H
#define __DELAY_H 	
#include <stm32f10x_type.h>
#include "stm32f10x_map.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "ucos_ii.h"


#define	Time_test		0			//测试时选1


//27个机器周期
#define MachineCycle_27 \
{\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();\
}

//25个机器周期
#define MachineCycle_25 \
{\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();\
}

//3个机器周期
#define MachineCycle_3 \
{\
	__nop();__nop();__nop();\
}

//2个机器周期
#define MachineCycle_2 \
{\
	__nop();__nop();\
}

//完整的32个机器周期，32M时钟频率下共1us
#define DELAY1US_32M \
{\
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();\
}

//完整的4个机器周期，4M时钟频率下共1us（实测非常准确）
#define DELAY1US_4M \
{\
	__nop();__nop();__nop();__nop();\
}
//	__NOP;	这条语句会被编译器优化掉


//函数声明

#if	Time_test
void WS2812_init(void);
void time_test(void);
#endif
void delay_1us_32M( u32 us );
void delay_2us_4M(u32 us);
void delay_init(void);
void delay_ms(u32 ms);
void delay_50us(u32 nus);
void TIM3_Int_Init(u16 arr,u16 psc);
void delay_SYSus(u32 nus);


#endif





























