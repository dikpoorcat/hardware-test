#ifndef __Bsp_AM2302_H
#define __Bsp_AM2302_H
#include "ucos_ii.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_type.h"
#include <string.h>
#include "delay.h"
#include "Bsp_BMP180.h"


//-----------------------------------��ʪ�ȵ�Դ��������----------------------------------------
//OK	ZE
#define PWMET_PIN		GPIO_Pin_11									//��ʪ�ȵ�Դ��������
#define PWMET_Port		GPIOA
#define PWMETEN()		GPIO_SetBits(PWMET_Port,PWMET_PIN);
#define PWMETDIS()		GPIO_ResetBits(PWMET_Port,PWMET_PIN);
#define PWMET_Port_CK	RCC_APB2Periph_GPIOA


//-----------------------------------��ʪ����������----------------------------------------
//OK	ZE
#define	Am2302Port   	GPIOC
#define	Am2302Pin   	GPIO_Pin_8

//------------------ͨ���Ժ�-------------------//
//#define	Am2302_DataH()		GPIO_SetBits(Am2302Port,Am2302Pin);			//�ⲿ����Ϊ�ߵ�ƽ
//#define	Am2302_DataL()		GPIO_ResetBits(Am2302Port,Am2302Pin);		//�͵�ƽ

//------------------���ٷ�ת-------------------//������Ļ�Am2302_Init(void)ҲҪ��
#define Am2302_DataH()		(Am2302Port ->BSRR = Am2302Pin) 				//Am2302Pin��1	���ٵض�GPIOA��λ6���з�ת	LED0��������	ռ��һ���������� Machine Cycle Time 0.03125us
#define Am2302_DataL()		(Am2302Port ->BSRR = Am2302Pin<<16)				//Am2302Pin��0	���ٵض�GPIOA��λ6���з�ת	LED0��������	ռ��һ���������� Machine Cycle Time

//#define	AM2302_DATA_IN()	GPIO_ReadInputDataBit(Am2302Port,Am2302Pin)
#define AM2302_DATA_IN()	(Am2302Port->IDR & Am2302Pin)					//��ȡ�����ƽ����Ϊ1


////���κ꣬��������������һ��ʹ��,����ߵ�ƽ��͵�ƽ  
//#define  AM2302_DATA_OUT(a) if (a)  \
//                    GPIO_SetBits(GPIO_AM2302,PIN_AM2302);\
//                    else        \
//                    GPIO_ResetBits(GPIO_AM2302,PIN_AM2302)  
//��ȡ���ŵĵ�ƽ 





typedef struct  
{  
    INT8U  humi_H;				//ʪ�ȸ�8λ��1000��ʪ��ֵ����652=65.2%
    INT8U  humi_L;				//ʪ�ȵ�8λ 
    INT8U  temp_H;				//�¶ȸ�8λ��10��������0��ʱ���λ��1
    INT8U  temp_L;				//�¶ȵ�8λ 
    INT8U  check_sum;			//У���                      
}AM2302_Data_TypeDef; 



//ȫ�ֱ���
extern AM2302_Data_TypeDef	AM2302_Data;



//��������
void MET_LowPower(void);
void PowerMETPin_Init(void);
void Am2302_Init(void) ;
static void AM2302_Mode_IPU(void);
static INT8U Read_Byte(void);
INT8U Read_AM2302( AM2302_Data_TypeDef *Data );
INT8U Read_Avrage_AM2302( AM2302_Data_TypeDef *Data, INT8U Times );
INT8U Read_Median_AM2302( AM2302_Data_TypeDef *Data, INT8U Times );


#endif /* __Bsp_Am2302_H */
