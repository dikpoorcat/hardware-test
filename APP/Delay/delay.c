//Z.E. 20180525
#include "delay.h"

#if Time_test//测试用
#include "Bsp_AM2302.h"
//更换数据输出引脚时只需修改WS2812_init()和以下几行
#define LED0_H 	(GPIOC->BSRR = 0x00000100) 			//PC8置1	快速地对GPIOA的位6进行翻转	LED0控制引脚	占用一个机器周期 Machine Cycle Time 0.03125us
#define LED0_L 	(GPIOC->BSRR = 0x01000000)			//PC8置0	快速地对GPIOA的位6进行翻转	LED0控制引脚	占用一个机器周期 Machine Cycle Time
//#define LED1_H 	(GPIOC->BSRR = 0x00000080) 		//PC7置1	快速地对GPIOA的位7进行翻转	LED1控制引脚
//#define LED1_L 	(GPIOC->BSRR = 0x00800000)		//PC7置0	快速地对GPIOA的位7进行翻转	LED1控制引脚
void WS2812_init(void)
{
	RCC->APB2ENR|=1<<4;  		//PORTC时钟使能 	2~8分别为A~G  
	GPIOC->CRH&=0XFFFFFFF0; 
	GPIOC->CRH|=0X00000007;		//PC8(3推挽，7开漏）输出
//	GPIOA->CRL&=0XFFFF00FF; 
//	GPIOA->CRL|=0X00003300;		//PA3/2(3推挽，7开漏）输出		
}
void time_test(void)
{
	PowerMETPin_Init();			//电源控制引脚初始化
	PWMETEN();					//开启上拉的电源
	WS2812_init();				//观察引脚PC8的初始化
	
#if	0	//测试systick延时（100us实测周期104us，100ms实测100ms，基本准）
	
	while(1)
	{
		LED0_H;				//占用一个机器周期 Machine Cycle Time，约0.03us
		delay_SYSus(50000);
		LED0_L;				//占用一个机器周期 Machine Cycle Time
		delay_SYSus(50000);
	}	

#elif	0	//测试TIM3做的延时（不准）
	
	delay_init();			//初始化定时器TIM3
	while(1)
	{
		LED0_H;				//占用一个机器周期 Machine Cycle Time，约0.03us
		delay_50us(1);
		LED0_L;				//占用一个机器周期 Machine Cycle Time
		delay_50us(1);
	}
	
#elif 0		//测试nop循环做的延时
	while(1)
	{	
		#if 1
		LED0_H;				//占用一个机器周期 Machine Cycle Time，约0.03us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		//100us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		//100us
		//字面上200us，实测300us
		
		LED0_L;				//占用一个机器周期 Machine Cycle Time
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us	
		//100us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		//100us
		//字面上200us，实测280us，故入参为1的单次调用约1.4us，而非1us
		#else
		LED0_H;				//占用一个机器周期 Machine Cycle Time，约0.03us
		delay_1us_32M(10);
		
		LED0_L;				//占用一个机器周期 Machine Cycle Time
		delay_1us_32M(10);	//实测周期21.4us，7%偏差
		#endif
	}
#elif 0
	while(1)				//用示波器观察并记录
	{	
		LED0_H;				//占用一个机器周期 Machine Cycle Time，约0.03us
		delay_1us_32M(1000);//1000us
		
		LED0_L;				//占用一个机器周期 Machine Cycle Time
		delay_1us_32M(1000);	
		
		LED0_H;				//占用一个机器周期 Machine Cycle Time，约0.03us
		delay_1us_32M(100);	//100.8us
		
		LED0_L;				//占用一个机器周期 Machine Cycle Time
		delay_1us_32M(100);	
		
		LED0_H;				//占用一个机器周期 Machine Cycle Time，约0.03us
		delay_1us_32M(50);	//50.6us
		
		LED0_L;				//占用一个机器周期 Machine Cycle Time
		delay_1us_32M(50);
		
		LED0_H;				//占用一个机器周期 Machine Cycle Time，约0.03us
		delay_1us_32M(10);	//10.74us
		
		LED0_L;				//占用一个机器周期 Machine Cycle Time
		delay_1us_32M(10);	
		
		LED0_H;				//占用一个机器周期 Machine Cycle Time，约0.03us
		delay_1us_32M(1);	//1.74us
		
		LED0_L;				//占用一个机器周期 Machine Cycle Time
		delay_1us_32M(1);	
	}
#elif 0
	while(1)				//用示波器观察，刚好2us周期 （每个半周期内，有25个nop+1个寄存器操作指令，共26个机器周期，
	{						//while(1)的判断+执行共2个指令周期，共2*6=12个机器周期。26+12=32个机器周期，对应32M时钟频率刚好1us）
		LED0_H;				//占用一个机器周期 Machine Cycle Time
		MachineCycle_25;
		LED0_L;				//占用一个机器周期 Machine Cycle Time
		MachineCycle_25;
	}
#elif 1
	while(1)
	{	
		#if 0
		LED0_H;				//占用一个机器周期 Machine Cycle Time，约0.03us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		//200us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		//200us
		//字面400us，实测1080us，故入参为1的单次调用约5.4us，而非2us
		
		LED0_L;				//占用一个机器周期 Machine Cycle Time
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		//200us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		//200us
		//字面400us，实测1080us，故入参为1的单次调用约5.4us，而非2us
		#else
		LED0_H;				//占用一个机器周期 Machine Cycle Time
		delay_2us_4M(50);
		
		LED0_L;				//占用一个机器周期 Machine Cycle Time
		delay_2us_4M(50);	//(10)实测周期51.6us，29%偏差；(20)实测周期91us，13.75%偏差；(30)实测周期132us，10%偏差；(40)实测周期172us，7.5%偏差；(50)实测周期212us，6%偏差；
		#endif
	}
	#elif 0
	while(1)	//4M，纯nop测试
	{			//while(1)的判断+执行共2个指令周期，共2*6=12个机器周期，加上LED0_H和LED0_L2个机器周期，共14个，14/4=3.5us，与实测结果接近
		LED0_H;				//占用一个机器周期 Machine Cycle Time
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		//50us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		//50us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		//50us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		//50us
		//50*4=200us
		
		LED0_L;				//占用一个机器周期 Machine Cycle Time
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		//50us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		//50us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		//50us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		//50us
		//50*4=200us
	}
#endif
}
#endif


//------------------------------------------------------------利用nop延时（还有个 DELAY1US_32M 可用，在头文件）------------------------------------------------------------//
/**********************************************************************
函数：void delay_1us_32M(u32 us)
功能：可供调用的1us延时。只调用1us的话会稍微不准，因为有出入栈时间，调用一次函数花了1.49-1=0.49us
（100次调用1us共占用150us，每次0.5us；单次调测得0.74、0.6、0.8），约15.68个机器周期(15.68/32=0.49us)
由于可精确到1us延时，故不禁止OS调度，需要在调用时注意，自行在外部禁止OS调度
	2018/08/30测试，入参为1的单次调用约1.4us；入参为10两次调用约21.4us
入参：延时的微秒数，建议( us > 10 )，7%偏差
出参：无
***********************************************************************/
void delay_1us_32M(u32 us)
{
	while(us--)				//实测每次循环(us*5)个机器周期
	{
		MachineCycle_27;	//5+27=32个机器周期，32M时钟频率下为1us
	}
}

/**********************************************************************
函数：void delay_2us_4M(u32 us)------4M无法做到1us，一次while循环就1.25us了
功能：可供调用的2us延时。
由于可精确到1us延时，故不禁止OS调度，需要在调用时注意，自行在外部禁止OS调度
	2018/08/30测试，入参为1的单次调用约5.4us，而非2us；
入参：延时的微秒数，建议( us > 50 )，6%偏差
出参：无
***********************************************************************/
void delay_2us_4M(u32 us)
{
//	u32	i=0;
//	for(;i<us;i++)			//实测每次循环(us*6)个机器周期
//	{
//		MachineCycle_2;		//6+2个机器周期，4M时钟频率下为2us
//	}
	while(us--)				//实测每次循环(us*5)个机器周期
	{
		MachineCycle_3;		//5+3个机器周期，4M时钟频率下为2us
	}
}


//------------------------------------------------------------利用TIM3延时（从STM32F407移过来，还没修改，不准，134us）------------------------------------------------------------//
/**********************************************************************
函数：void delay_init(void)
功能：TIM3进行初始化，用来计时
入参：无
出参：无
***********************************************************************/
void delay_init(void)
{
	TIM3_Int_Init(49,83);//10Khz计数,50us一次中断
}

/**********************************************************************
函数：void delay_50us(u32 us)
功能：利用TIM3进行50us延时
入参：延时的（50微秒）数
出参：无
***********************************************************************/
void delay_50us(u32 us)
{		
	TIM_Cmd(TIM3,ENABLE);  //使能定时器3
	while(us--)
	{
		while(TIM_GetFlagStatus(TIM3,TIM_IT_Update)!=SET); //等待溢出
		TIM_ClearFlag(TIM3,TIM_IT_Update);                 //清除标志位
	}
	TIM_Cmd(TIM3,DISABLE); //停止定时器3
}

/**********************************************************************
函数：void delay_ms(u32 ms)
功能：利用TIM3进行1ms延时
入参：延时的毫秒数
出参：无
***********************************************************************/
void delay_ms(u32 ms)
{	 	 
	while(ms--)
	{
		delay_50us(20);
	}
} 

/**********************************************************************
函数：void TIM3_Int_Init(u16 arr,u16 psc)
功能：通用定时器3中断初始化
入参：u16 arr，自动重装载值；u16 psc，定时器分频
出参：无
***********************************************************************/
void TIM3_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);           //使能TIM3时钟
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;                  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_Period=arr;                     //自动重装载值
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
}



//------------------------------------------------------------利用systick延时------------------------------------------------------------//
/*===========================================================================
函数：void delay_SYSus(u32 nus)
说明：us延时，利用systick一直在递减，不停读取值比较，当比较值超过入参时则退出               因进行us级别的延时，指令执行时间会超出10us，导致无法进行10us以内的延时。放弃使用，当系统时钟频率够高的时候才可使用。
     需要注意防止OS调度打断延时。（占用CPU、OS的死等待）
入参：
出参：
============================================================================*/
void delay_SYSus(u32 nus)
{		
	u32 ticks=0;
	u32 told=0;
	u32 tnow=0;
	u32 tcnt=0;
	u32 reload=SysTick->LOAD;					//读取LOAD的值	    	 
	ticks=nus*4;								//需要的节拍数（nus需要的ticks数）	  系统定时器时钟systick=HCLK/8=4M，每us需要的节拍数为4	HCLK：给AHB总线、内核、内存和DMA使用的HCLK时钟。高速外设时钟（一般不分频，等于系统时钟），是给外部设备的，比如内存，flash，DMA	
	tcnt=0;
	OSSchedLock();								//阻止OS调度，防止打断us延时
	told=SysTick->VAL;							//刚进入时的计数器值
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told) tcnt+=told-tnow;		//如果未递减到0
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;				//时间超过或等于要延迟的时间,则退出.
		}  
	};
	OSSchedUnlock();							//恢复OS调度									    
}
