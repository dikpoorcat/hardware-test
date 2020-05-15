#ifndef _BSP_ADC_H
#define _BSP_ADC_H

#include "stm32f10x_type.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "ucos_ii.h"
#include "bsp_WDG.h"
#include "delay.h"
#include "MemConfig.h"

#define CH_BAT 0
#define CH_FALA 1
#define CH_MCUWD 2

#define ADC1_DR_Address    ((u32)0x4001244C)    //ADC1 DR address


#define ADCEN_Pin       GPIO_Pin_12				//ADC检测控制
#define ADCEN_Port      GPIOC


typedef struct{ 								//设备信息，浮点数据
	float	BAT_Volt;							//锂电池电压
	float	FALA_Volt;							//法拉电容电压
	float	MCU_Temp;							//单片机温度
}Equi_STA;


extern Equi_STA	Equipment_state;



//函数声明
void ADC1_Configuration(void);
void DMA_Configuration(void);
void AD_Init(void);
void AD_LowPower(void);
u32 GET_ADVALUE(u8 ch,u8 count);
u32 Read_Voltage(u8 ch,u8 count); 
u32 Read_MCU_Temp( u8 count );
void ADC_HEXtoASCII(u32 In,u8 *pOut);		
void ReadVCC_Test(void);
void Print_Voltage( INT8U Channel, INT32U Voltage );
void Get_Voltage_MCUtemp_Data( INT8U Count );
INT8U HB_Get_Voltage( INT8U Count );

#endif
