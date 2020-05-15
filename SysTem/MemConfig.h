#ifndef	__MemConfig_H
#define	__MemConfig_H
#include "main.h"

/*********************************************************
 说明：规定基站类型，硬件版本号，软件版本号。
 基站类型：
 00：测温
 01：微气象
 02：图片
 硬件版本类型：
 03：3号
 21：21号
 17：17号
 软件版本类型：
***********************************************************/
/*版本号（只读模式）   5个字节:
前1个字节为设备类型 【见设备符号附件】
中2个字节为硬件版本号 
后2个字节为软件版本号，前1个字节大版本号 后1个字节小版本号*/
#define BAN1					00     											//00测温  01 微气象  02 图片
#define BANPCB1            		0x18
#define BANPCB2					0x10 		 									// 全网通基站硬件版本全网通基站_V1.4 20181019
#define BANSOFT1				0x03
#define BANSOFT2				0x02											// 全网通基站软件版本V3.1/V3.2  V3.1为场内测试模式，适合提前录入探头，在基站上进行台账外探头数据的过滤，可以减少发送数据的量（未与后台关联的数据）		V3.2为出厂模式，会将48个以内收到的探头数据全部上报


/*************************** 铁电保存配置统一编址文件 ***************************/
/*-----------------------新增时加在最后，防止影响老设备！-----------------------*/
/*-------------------------------系统参数配置区--------------------------------*///K0
#define FM_Start_Addr			0    											//标记铁电起始位置
#define OldDay_Addr				(FM_Start_Addr+1)								//用于记录今天是否已经过去，已经变成昨天
#define Reset_Flag_Addr			(FM_Start_Addr+2)								//用于记录热启动
#define LTE_Sending_Flag_Addr	(FM_Start_Addr+3)								//用于标识一个数据冻结发送过程
#define Reset_Count_Addr		(FM_Start_Addr+4)								//用于标识冷启动计数

#define Config_Addr				(FM_Start_Addr+5)										
#define Config_Len				sizeof(Config)	
#define	IP_Config_Addr			(Config_Addr+Config_Len)
#define	IP_Config_Len			sizeof(IP_Config)
#define Device_Number_Addr		(IP_Config_Addr+IP_Config_Len)
#define	Device_Number_Len		sizeof(Device_Number)
#define FUN_Config_Addr			(Device_Number_Addr+Device_Number_Len)
#define FUN_Config_Len			sizeof(FUN_Config)
#define Flow_Data_Addr			(FUN_Config_Addr+FUN_Config_Len)				//存放流量统计数据
#define	Flow_Data_Len			sizeof(Local_FLow_Data)
#define TT_Count_Addr			(Flow_Data_Addr+Flow_Data_Len)
#define TT_ID_Addr				(TT_Count_Addr+1)								//探头数量
#define TT_ID_Len				(2*55)											//每个探头ID两个字节（如60000），最多可装55个探头
#define Fault_Manage_Addr		(TT_ID_Addr+TT_ID_Len)							//故障信息结构体
#define Fault_Magage_Len		sizeof(Fault_Manage )
#define Fault_Info_Addr			(Fault_Manage_Addr+Fault_Magage_Len)
#define Fault_Info_Len			sizeof(Fault_Info)
#define Unreport_Index_Addr		(Fault_Info_Addr+Fault_Info_Len)
#define Unreport_Index_Len		sizeof(Unreport_Index)
#define sys2_upgrade_time_Addr	(Unreport_Index_Addr+Unreport_Index_Len)		
#define sys2_upgrade_time_Len	sizeof(sys2_upgrade_time)
#define	APN_Addr				(sys2_upgrade_time_Addr+sys2_upgrade_time_Len)	//APN
#define	APN_Len					sizeof(APN)
	
#define FM_K0_End_Addr			(APN_Addr+APN_Len)


/*-------------------------存放一个小时内的温度采样数据--------------------------*///K1-K7		需要占用7K的内存，铁电所剩无几
#define Sample_Manage_Addr		(1*0x400)                            
#define Sample_Manage_Len		sizeof(TT_Sample_Manage)		
#define Sample_Data_Addr		(Sample_Manage_Addr+Sample_Manage_Len)
#define One_TT_Sample_Data_Len	(60*2)											//每个探头分配存储空间为120字节可存放60个数据
#define	All_Sample_Data_Len		(55*60*2)										//(6600个字节)

#define FM_K1_K7_End_Addr		(Sample_Data_Addr+All_Sample_Data_Len)


/************************* W25Q256存储配置统一编址文件 **************************/
#define Sector_Size				0x1000											//扇区尺寸
#define Sector_Max				256*32-1										//总共包含的扇区数，索引号由0开始
#define Device_Number_Flash_Addr	0x08005F00									//0x5F00 到 0x6000 空间用于写滚码和预留

#endif
