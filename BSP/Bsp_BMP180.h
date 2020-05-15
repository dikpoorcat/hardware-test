#ifndef __Bsp_Bmp180_H
#define __Bsp_Bmp180_H
#include "ucos_ii.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "delay.h"

#define Bmp180_SDAPort      GPIOC
#define Bmp180_SDAPin       GPIO_Pin_6

#define Bmp180_SCLKPort     GPIOC
#define Bmp180_SCLKPin      GPIO_Pin_7

#define Bmp180_IN()        {Bmp180_SDAPort->CRL&=0XF0FFFFFF;Bmp180_SDAPort->CRL|=(u32)8<<24;}       //PC6输入模式1000
#define Bmp180_OUT()       {Bmp180_SDAPort->CRL&=0XF0FFFFFF;Bmp180_SDAPort->CRL|=(u32)3<<24;}       //PC6输出模式0011

#define Bmp180_SCLK_H()     Bmp180_SCLKPort->BSRR=Bmp180_SCLKPin;
#define Bmp180_SCLK_L()	    Bmp180_SCLKPort->BRR=Bmp180_SCLKPin;

#define Bmp180_SDA_H()      Bmp180_SDAPort->BSRR=Bmp180_SDAPin;
#define Bmp180_SDA_L()		Bmp180_SDAPort->BRR=Bmp180_SDAPin;

#define Bmp180_SDAIN        Bmp180_SDAPort->IDR & Bmp180_SDAPin



//BMP180校正参数(calibration param)
typedef struct {
	 short AC1 ;
	 short AC2 ;
	 short AC3 ;
	unsigned short AC4 ;
	unsigned short AC5 ;
	unsigned short AC6 ;
	 short B1 ;
	 short  B2 ;
	 short MB ;
	 short MC ;
	 short MD ;
}BMP180_cal_param;

typedef struct {
	unsigned char  ExistFlag ;  //存在标志

	BMP180_cal_param  cal_param;//修正系数

	unsigned char Version ;				//版本

	unsigned long UnsetTemperature ;		//未校正的温度值
	unsigned long UnsetGasPress	  ;		//未校正的气压值

	unsigned long Temperature ;			/*校正后的温度值*/
	unsigned long GasPress ;				/*校正后的气压值*/

	float Altitude ;				/*海拔*/
	
}BMP180_info ;





#define BMP180_NOT_EXISTENCE 0	/*不存在*/
#define BMP180_EXISTENCE     1	/*存在*/

#define OSS  2	//范围(0~3)

#define BMP180_READ		        0x01
#define BMP180_WRITE	        0x00	

#define	BMP180_DEVICE_ADDRESS_BASE_VALUE   0xee			 /*器件地址基值*/                  
//#define BMP180_CONTROL_REGISTER_ADDRESS_BASE_VALUE	0xf4 /*控制寄存器地址*/
#define BMP180_ID_REGISTER_ADDRESS		0xd0 /*ID编号寄存器(0x55固定)*/
#define BMP180_VERSION_REGISTER_ADDRESS	0XD1 /*版本编号*/
//#define BMP180_SOFT_RESET_REGISTER_BASE_VALUE	    0xe0 /*软件复位寄存器,只写，设置0xb6*/


//control register
//#define BMP180_CONTROL_REGISTER_SCO_BIT (0X01<<5)

//id register 
#define BMP180_ID_FIXED_VALUE		0x55 /*id固定编号(0x55)*/



/*****************内部函数******************/
//初始化
extern void BMP180Init(BMP180_info *p);				

//转换，修正温度、气压，计算海拔
extern void BMP180Convert(BMP180_info *temp) ;		

/*下面两个函数一般不在外部使用，也可以直接声明为BMP180.c的内部函数*/
//地址写数据
extern void BMP180AddressWrite(unsigned char addresss,unsigned char dataCode) ;

//地址读数据
extern unsigned char BMP180AddressReadByte(unsigned char address) ;


/**********************************************/



#endif /* __Bsp_Bmp180_H */
