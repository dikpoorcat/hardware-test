#ifndef __Bsp_DS18B20_H
#define __Bsp_DS18B20_H
#include "ucos_ii.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"



//-----------------------------------DS18B20��Դ��������----------------------------------------
#define DS18B20_PW_PIN		  GPIO_Pin_11														//��ʪ�ȵ�Դ��������
#define DS18B20_PW_PORT		  GPIOA
#define DS18B20_PW_ON()		  GPIO_SetBits(DS18B20_PW_PORT,DS18B20_PW_PIN);
#define DS18B20_PW_OFF()	  GPIO_ResetBits(DS18B20_PW_PORT,DS18B20_PW_PIN);
#define DS18B20_PW_Port_CK    RCC_APB2Periph_GPIOA

//-----------------------------------DS18B20��������----------------------------------------
#define DS18B20_IO_PIN        GPIO_Pin_8
#define DS18B20_IO_PORT       GPIOC
#define DS18B20_H()           DS18B20_IO_PORT->BSRR = DS18B20_IO_PIN;							//���ٷ�ת
#define DS18B20_L()           DS18B20_IO_PORT->BRR = DS18B20_IO_PIN;							//���ٷ�ת
#define DS18B20_IO_Port_CK	  RCC_APB2Periph_GPIOC
#define DS18B20_IN            DS18B20_IO_PORT->IDR&DS18B20_IO_PIN

#define DS18B20_IO_IN()       {DS18B20_IO_PORT->CRH&=0XFFFFFFF0;DS18B20_IO_PORT->CRH|=8;}       // PC8����ģʽ1000
#define DS18B20_IO_OUT()      {DS18B20_IO_PORT->CRH&=0XFFFFFFF0;DS18B20_IO_PORT->CRH|=3;}       // PC8���ģʽ0011





//28����������
#define MachineCycle_27 \
{\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();__nop();__nop();__nop();\
	__nop();__nop();\
}


#if 0	/*32M��Ƶʱѡ1*/
	#define SYSfre 32 //32:��Ƶ32M��4����Ƶ4M
#else
	#define SYSfre 4 //32:��Ƶ32M��4����Ƶ4M
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
void delay_SYS_2us(u32 n2us);                          //systick��ʱ
void DS18B20_RST(void);                             //��λ
u8 DS18B20_CHECK(void);                             //DS18B20��λӦ��Ӧ��0��DS18B20 READY
u8 DS18B20_Init(void);                              //��ʼ����POWER���ų�ʼ��������λӦ�𣬷���Ӧ��0��DS18B20 READY
void DS18B20_Write_Byte(u8 dat);                    //дDS18B20һ���ֽ�  
u8 DS18B20_Read_Bit(void);                          //��һλ
u8 DS18B20_Read_Byte(void);                         //��һ���ֽ�
u8 DS18B20_Get_Temperature(u16 *TempBuff);          //��һ���¶�����
u8 Get_DS18B20Temp(u16 *temp);                      //����������¶ȣ�ȡ�м�ֵ

void DS18B20_Wakeup(void) ;                         //����DS18B20���ϵ������и�λ
void DS18B20_Sleep(void)  ;                         //���͹��ģ��رյ�Դ��DQ��
float TempU16toFloat(INT16U In);					//��DS18B20������U16ת���ɸ�����
INT16U FloatToTempU16(float In);					//����ת����,DS18B20			
#endif /* __Bsp_Ds18b20_H */
