#ifndef __Bsp_WDSP_JX_H
#define __Bsp_WDSP_JX_H
#include "ucos_ii.h"
#include "Meteorology.h"
#include "Bsp_UART.h"

//-----------------------------------风速风向电源控制引脚----------------------------------------
//OK	ZE
#define PWWDSP_PIN		GPIO_Pin_12									//风速风向电源使能引脚
#define PWWDSP_Port		GPIOA
#define PWWDSPEN()		GPIO_SetBits(PWWDSP_Port,PWWDSP_PIN)		//开启电源
#define PWWDSPDIS()		GPIO_ResetBits(PWWDSP_Port,PWWDSP_PIN)		//关闭电源
#define PWWDSP_Port_CK	RCC_APB2Periph_GPIOA



//无头文件的外部函数声明
extern void B485_init(unsigned int rate);
extern INT8U B485WaitData(INT8U WaitTime);


//函数声明
void WDSP_LowPower(void);
void PowerWDSPPin_Init(void);
INT8U Get_WDSP_Data( INT16U *OutBuff_WindSpeed, INT16U *OutBuff_WindDirection );
INT8U IsProtocol_WDSP( INT8U *In, INT8U Len, INT16U *Out_WindSpeed, INT16U *Out_WindDirection );
void CRC16_Modbus(INT8U *P,INT16U len,INT8U *LByte,INT8U *HByte);


#endif /* __Bsp_WDSP_JX_H */
