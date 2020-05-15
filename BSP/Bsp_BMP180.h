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

#define Bmp180_IN()        {Bmp180_SDAPort->CRL&=0XF0FFFFFF;Bmp180_SDAPort->CRL|=(u32)8<<24;}       //PC6����ģʽ1000
#define Bmp180_OUT()       {Bmp180_SDAPort->CRL&=0XF0FFFFFF;Bmp180_SDAPort->CRL|=(u32)3<<24;}       //PC6���ģʽ0011

#define Bmp180_SCLK_H()     Bmp180_SCLKPort->BSRR=Bmp180_SCLKPin;
#define Bmp180_SCLK_L()	    Bmp180_SCLKPort->BRR=Bmp180_SCLKPin;

#define Bmp180_SDA_H()      Bmp180_SDAPort->BSRR=Bmp180_SDAPin;
#define Bmp180_SDA_L()		Bmp180_SDAPort->BRR=Bmp180_SDAPin;

#define Bmp180_SDAIN        Bmp180_SDAPort->IDR & Bmp180_SDAPin



//BMP180У������(calibration param)
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
	unsigned char  ExistFlag ;  //���ڱ�־

	BMP180_cal_param  cal_param;//����ϵ��

	unsigned char Version ;				//�汾

	unsigned long UnsetTemperature ;		//δУ�����¶�ֵ
	unsigned long UnsetGasPress	  ;		//δУ������ѹֵ

	unsigned long Temperature ;			/*У������¶�ֵ*/
	unsigned long GasPress ;				/*У�������ѹֵ*/

	float Altitude ;				/*����*/
	
}BMP180_info ;





#define BMP180_NOT_EXISTENCE 0	/*������*/
#define BMP180_EXISTENCE     1	/*����*/

#define OSS  2	//��Χ(0~3)

#define BMP180_READ		        0x01
#define BMP180_WRITE	        0x00	

#define	BMP180_DEVICE_ADDRESS_BASE_VALUE   0xee			 /*������ַ��ֵ*/                  
//#define BMP180_CONTROL_REGISTER_ADDRESS_BASE_VALUE	0xf4 /*���ƼĴ�����ַ*/
#define BMP180_ID_REGISTER_ADDRESS		0xd0 /*ID��żĴ���(0x55�̶�)*/
#define BMP180_VERSION_REGISTER_ADDRESS	0XD1 /*�汾���*/
//#define BMP180_SOFT_RESET_REGISTER_BASE_VALUE	    0xe0 /*�����λ�Ĵ���,ֻд������0xb6*/


//control register
//#define BMP180_CONTROL_REGISTER_SCO_BIT (0X01<<5)

//id register 
#define BMP180_ID_FIXED_VALUE		0x55 /*id�̶����(0x55)*/



/*****************�ڲ�����******************/
//��ʼ��
extern void BMP180Init(BMP180_info *p);				

//ת���������¶ȡ���ѹ�����㺣��
extern void BMP180Convert(BMP180_info *temp) ;		

/*������������һ�㲻���ⲿʹ�ã�Ҳ����ֱ������ΪBMP180.c���ڲ�����*/
//��ַд����
extern void BMP180AddressWrite(unsigned char addresss,unsigned char dataCode) ;

//��ַ������
extern unsigned char BMP180AddressReadByte(unsigned char address) ;


/**********************************************/



#endif /* __Bsp_Bmp180_H */
