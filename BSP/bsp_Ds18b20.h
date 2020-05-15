#ifndef __Bsp_DS18B20_H
#define __Bsp_DS18B20_H
#include "ucos_ii.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"



//-----------------------------------DS18B20电源控制引脚----------------------------------------
#define DS18B20_PW_PIN		  GPIO_Pin_11														//温湿度电源控制引脚
#define DS18B20_PW_PORT		  GPIOA
#define DS18B20_PW_ON()		  GPIO_SetBits(DS18B20_PW_PORT,DS18B20_PW_PIN);
#define DS18B20_PW_OFF()	  GPIO_ResetBits(DS18B20_PW_PORT,DS18B20_PW_PIN);
#define DS18B20_PW_Port_CK    RCC_APB2Periph_GPIOA

//-----------------------------------DS18B20数据引脚----------------------------------------
#define DS18B20_IO_PIN        GPIO_Pin_8
#define DS18B20_IO_PORT       GPIOC
#define DS18B20_H()           DS18B20_IO_PORT->BSRR = DS18B20_IO_PIN;							//快速翻转
#define DS18B20_L()           DS18B20_IO_PORT->BRR = DS18B20_IO_PIN;							//快速翻转
#define DS18B20_IO_Port_CK	  RCC_APB2Periph_GPIOC
#define DS18B20_IN            DS18B20_IO_PORT->IDR&DS18B20_IO_PIN

#define DS18B20_IO_IN()       {DS18B20_IO_PORT->CRH&=0XFFFFFFF0;DS18B20_IO_PORT->CRH|=8;}       // PC8输入模式1000
#define DS18B20_IO_OUT()      {DS18B20_IO_PORT->CRH&=0XFFFFFFF0;DS18B20_IO_PORT->CRH|=3;}       // PC8输出模式0011





//28个机器周期
#define MachineCycle_27 \
{\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();\
}


#if 0	/*32M主频时选1*/
	#define SYSfre 32 //32:主频32M；4：主频4M
#else
	#define SYSfre 4 //32:主频32M；4：主频4M
#endif

#if SYSfre == 4

	 #define DS_delay2us     {__nop();__nop();__nop();}	//2us
	 #define DS_delay10us    {__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();\
							  __nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();\
							  __nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();\
							  __nop();__nop();__nop();__nop();__nop();}
								 //10us

	#define DS_delay40us   delay_SYS_2us(10);          //40us
	#define DS_delay60us   delay_SYS_2us(15);
	#define DS_delay600us  delay_SYS_2us(300);
#endif

					 
						 
 #if SYSfre == 32					 
	#define DS_delay2us     delay_1us_32M(2);	//2us
	#define DS_delay10us    delay_1us_32M(10);//10us

	#define DS_delay40us   delay_1us_32M(40);          //40us
	#define DS_delay60us   delay_1us_32M(60);
	#define DS_delay600us  delay_1us_32M(600);
#endif

							  

void DS18B20_LowPower(void);
void delay_SYS_2us(u32 n2us);                          //systick延时
void DS18B20_RST(void);                             //复位
u8 DS18B20_CHECK(void);                             //DS18B20复位应答应答0：DS18B20 READY
u8 DS18B20_Init(void);                              //初始化：POWER引脚初始化，及复位应答，返回应答0：DS18B20 READY
void DS18B20_Write_Byte(u8 dat);                    //写DS18B20一个字节  
u8 DS18B20_Read_Bit(void);                          //读一位
u8 DS18B20_Read_Byte(void);                         //读一个字节
u8 DS18B20_Get_Temperature(u16 *TempBuff);          //读一次温度数据
u8 Get_DS18B20Temp(u16 *temp);                      //连续读五次温度，取中间值

void DS18B20_Wakeup(void) ;                         //唤醒DS18B20，上电后需进行复位
void DS18B20_Sleep(void)  ;                         //降低功耗，关闭电源与DQ口
float TempU16toFloat(INT16U In);					//将DS18B20读出的U16转化成浮点型
INT16U FloatToTempU16(float In);					//浮点转整形,DS18B20			
#endif /* __Bsp_Ds18b20_H */
