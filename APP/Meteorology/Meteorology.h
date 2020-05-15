#ifndef __Meteorology_H
#define __Meteorology_H

//#include "ucos_ii.h"
#include "stm32f10x_type.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "Bsp_WDSP_JX.h"
#include "Bsp_AM2302.h"
#include "Bsp_BMP180.h"
#include "bsp_RTC.h"
#include <string.h>						//extern _ARMABI void *memcpy(
#include "Bsp_adc.h"


#pragma pack(2)
typedef struct {						//��������(9-21)�ṹ�壬���ݹ�����ԼQ/GDW 242-2010�������ĸ�ʽ����
	float	Ave_WindSpeed;				//10����ƽ�����٣�װ�ð�װ�㴦��	=============���ں�̨��ʾ����
	INT16U	Ave_WindDirection;			//10����ƽ������װ�ð�װ�㴦��	=============���ں�̨��ʾ����
	float	Max_WindSpeed;				//�����٣�װ�ð�װ�㴦��
	float	Ext_WindSpeed;				//������٣�װ�ð�װ�㴦��
	float	Std_WindSpeed;				//��׼���٣����ö���������ת������׼״̬�ķ��٣�
	float	Air_Temperature;			//����								=============���ں�̨��ʾ�¶�
	INT16U	Humidity;					//ʪ��								=============���ں�̨��ʾʪ��
	float	Air_Pressure;				//��ѹ								=============���ں�̨��ʾ��ѹ
	float	Precipitation;				//������							=============���ں�̨��ʾ������
	float	Precipitation_Intensity;	//����ǿ��
	INT16U	Radiation_Intensity;		//�����ǿ��
	INT32U	Reserve1;					//����
	INT32U	Reserve2;					//����
//	INT16U	Rec_WindSpeed;				//
//	INT16U	Rec_WindDirection;			//
}MET_Data_TypeDef;
#pragma pack()




//ȫ�ֱ�������
extern INT16U	WindSpeed;							//��еʽ����������
extern INT16U	WindDirection;						//��еʽ����������
extern MET_Data_TypeDef	MET_Data;					//����΢��������

//��������
void MET_Main(void *org);
INT8U Get_Meteorology_Data( INT8U retry );
INT8U MET_packet_content( INT8U *OutBuff );
void Err_report( INT8U Err );
INT8U Test_Meteorology_Data( INT8U retry );



//��ͷ�ļ����ⲿ��������
extern INT16U  GDGuiYue(INT8U *OutBuff,INT8U *InBuff,INT16U InLen,INT8U Frame_Type,INT8U Packet_Type);
extern 	void B485_init(unsigned int rate);
extern 	INT8U GetSysTime(INT8U *OutBuff);
extern 	void MET_SaveWindToFM(INT16U Speed,INT16U Direction);
extern 	void MET_SaveData(INT8U *Time);
extern 	void SetGPRSON(void);
extern 	void OSTimeDlyLowPowr(INT16U Time);
extern 	void MCUSoftReset(void);
extern 	void WDTClear(unsigned char wdtindex);
extern 	INT8U WQ256_Test(void);




#endif /* __Meteorology_H */
