#ifndef __Bsp_AM2302_H
#define __Bsp_AM2302_H
#include "ucos_ii.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_type.h"
#include <string.h>
#include "delay.h"
#include "Bsp_BMP180.h"


//-----------------------------------温湿度电源控制引脚----------------------------------------
//OK	ZE
#define PWMET_PIN		GPIO_Pin_11									//温湿度电源控制引脚
#define PWMET_Port		GPIOA
#define PWMETEN()		GPIO_SetBits(PWMET_Port,PWMET_PIN);
#define PWMETDIS()		GPIO_ResetBits(PWMET_Port,PWMET_PIN);
#define PWMET_Port_CK	RCC_APB2Periph_GPIOA


//-----------------------------------温湿度数据引脚----------------------------------------
//OK	ZE
#define	Am2302Port   	GPIOC
#define	Am2302Pin   	GPIO_Pin_8

//------------------通用性好-------------------//
//#define	Am2302_DataH()		GPIO_SetBits(Am2302Port,Am2302Pin);			//外部上拉为高电平
//#define	Am2302_DataL()		GPIO_ResetBits(Am2302Port,Am2302Pin);		//低电平

//------------------快速翻转-------------------//用这个的话Am2302_Init(void)也要改
#define Am2302_DataH()		(Am2302Port ->BSRR = Am2302Pin) 				//Am2302Pin置1	快速地对GPIOA的位6进行翻转	LED0控制引脚	占用一个机器周期 Machine Cycle Time 0.03125us
#define Am2302_DataL()		(Am2302Port ->BSRR = Am2302Pin<<16)				//Am2302Pin置0	快速地对GPIOA的位6进行翻转	LED0控制引脚	占用一个机器周期 Machine Cycle Time

//#define	AM2302_DATA_IN()	GPIO_ReadInputDataBit(Am2302Port,Am2302Pin)
#define AM2302_DATA_IN()	(Am2302Port->IDR & Am2302Pin)					//读取输入电平，高为1


////带参宏，可以像内联函数一样使用,输出高电平或低电平  
//#define  AM2302_DATA_OUT(a) if (a)  \
//                    GPIO_SetBits(GPIO_AM2302,PIN_AM2302);\
//                    else        \
//                    GPIO_ResetBits(GPIO_AM2302,PIN_AM2302)  
//读取引脚的电平 





typedef struct  
{  
    INT8U  humi_H;				//湿度高8位，1000倍湿度值，例652=65.2%
    INT8U  humi_L;				//湿度低8位 
    INT8U  temp_H;				//温度高8位，10倍，低于0度时最高位置1
    INT8U  temp_L;				//温度低8位 
    INT8U  check_sum;			//校验和                      
}AM2302_Data_TypeDef; 



//全局变量
extern AM2302_Data_TypeDef	AM2302_Data;



//函数声明
void MET_LowPower(void);
void PowerMETPin_Init(void);
void Am2302_Init(void) ;
static void AM2302_Mode_IPU(void);
static INT8U Read_Byte(void);
INT8U Read_AM2302( AM2302_Data_TypeDef *Data );
INT8U Read_Avrage_AM2302( AM2302_Data_TypeDef *Data, INT8U Times );
INT8U Read_Median_AM2302( AM2302_Data_TypeDef *Data, INT8U Times );


#endif /* __Bsp_Am2302_H */
