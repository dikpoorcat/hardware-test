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
typedef struct {						//报文内容(9-21)结构体，根据国网规约Q/GDW 242-2010中气象报文格式定义
	float	Ave_WindSpeed;				//10分钟平均风速（装置安装点处）	=============用于后台显示风速
	INT16U	Ave_WindDirection;			//10分钟平均风向（装置安装点处）	=============用于后台显示风向
	float	Max_WindSpeed;				//最大风速（装置安装点处）
	float	Ext_WindSpeed;				//极大风速（装置安装点处）
	float	Std_WindSpeed;				//标准风速（利用对数风廓线转换到标准状态的风速）
	float	Air_Temperature;			//气温								=============用于后台显示温度
	INT16U	Humidity;					//湿度								=============用于后台显示湿度
	float	Air_Pressure;				//气压								=============用于后台显示气压
	float	Precipitation;				//降雨量							=============用于后台显示降雨量
	float	Precipitation_Intensity;	//降雨强度
	INT16U	Radiation_Intensity;		//光辐射强度
	INT32U	Reserve1;					//备用
	INT32U	Reserve2;					//备用
//	INT16U	Rec_WindSpeed;				//
//	INT16U	Rec_WindDirection;			//
}MET_Data_TypeDef;
#pragma pack()




//全局变量声明
extern INT16U	WindSpeed;							//机械式传感器风速
extern INT16U	WindDirection;						//机械式传感器风向
extern MET_Data_TypeDef	MET_Data;					//保存微气象数据

//函数声明
void MET_Main(void *org);
INT8U Get_Meteorology_Data( INT8U retry );
INT8U MET_packet_content( INT8U *OutBuff );
void Err_report( INT8U Err );
INT8U Test_Meteorology_Data( INT8U retry );



//无头文件的外部函数声明
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
