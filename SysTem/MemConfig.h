#ifndef	__MemConfig_H
#define	__MemConfig_H
#include "main.h"

/*********************************************************
 ˵�����涨��վ���ͣ�Ӳ���汾�ţ�����汾�š�
 ��վ���ͣ�
 00������
 01��΢����
 02��ͼƬ
 Ӳ���汾���ͣ�
 03��3��
 21��21��
 17��17��
 ����汾���ͣ�
***********************************************************/
/*�汾�ţ�ֻ��ģʽ��   5���ֽ�:
ǰ1���ֽ�Ϊ�豸���� �����豸���Ÿ�����
��2���ֽ�ΪӲ���汾�� 
��2���ֽ�Ϊ����汾�ţ�ǰ1���ֽڴ�汾�� ��1���ֽ�С�汾��*/
#define BAN1					00     											//00����  01 ΢����  02 ͼƬ
#define BANPCB1            		0x18
#define BANPCB2					0x10 		 									// ȫ��ͨ��վӲ���汾ȫ��ͨ��վ_V1.4 20181019
#define BANSOFT1				0x03
#define BANSOFT2				0x02											// ȫ��ͨ��վ����汾V3.1/V3.2  V3.1Ϊ���ڲ���ģʽ���ʺ���ǰ¼��̽ͷ���ڻ�վ�Ͻ���̨����̽ͷ���ݵĹ��ˣ����Լ��ٷ������ݵ�����δ���̨���������ݣ�		V3.2Ϊ����ģʽ���Ὣ48�������յ���̽ͷ����ȫ���ϱ�


/*************************** ���籣������ͳһ��ַ�ļ� ***************************/
/*-----------------------����ʱ������󣬷�ֹӰ�����豸��-----------------------*/
/*-------------------------------ϵͳ����������--------------------------------*///K0
#define FM_Start_Addr			0    											//���������ʼλ��
#define OldDay_Addr				(FM_Start_Addr+1)								//���ڼ�¼�����Ƿ��Ѿ���ȥ���Ѿ��������
#define Reset_Flag_Addr			(FM_Start_Addr+2)								//���ڼ�¼������
#define LTE_Sending_Flag_Addr	(FM_Start_Addr+3)								//���ڱ�ʶһ�����ݶ��ᷢ�͹���
#define Reset_Count_Addr		(FM_Start_Addr+4)								//���ڱ�ʶ����������

#define Config_Addr				(FM_Start_Addr+5)										
#define Config_Len				sizeof(Config)	
#define	IP_Config_Addr			(Config_Addr+Config_Len)
#define	IP_Config_Len			sizeof(IP_Config)
#define Device_Number_Addr		(IP_Config_Addr+IP_Config_Len)
#define	Device_Number_Len		sizeof(Device_Number)
#define FUN_Config_Addr			(Device_Number_Addr+Device_Number_Len)
#define FUN_Config_Len			sizeof(FUN_Config)
#define Flow_Data_Addr			(FUN_Config_Addr+FUN_Config_Len)				//�������ͳ������
#define	Flow_Data_Len			sizeof(Local_FLow_Data)
#define TT_Count_Addr			(Flow_Data_Addr+Flow_Data_Len)
#define TT_ID_Addr				(TT_Count_Addr+1)								//̽ͷ����
#define TT_ID_Len				(2*55)											//ÿ��̽ͷID�����ֽڣ���60000��������װ55��̽ͷ
#define Fault_Manage_Addr		(TT_ID_Addr+TT_ID_Len)							//������Ϣ�ṹ��
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


/*-------------------------���һ��Сʱ�ڵ��¶Ȳ�������--------------------------*///K1-K7		��Ҫռ��7K���ڴ棬������ʣ�޼�
#define Sample_Manage_Addr		(1*0x400)                            
#define Sample_Manage_Len		sizeof(TT_Sample_Manage)		
#define Sample_Data_Addr		(Sample_Manage_Addr+Sample_Manage_Len)
#define One_TT_Sample_Data_Len	(60*2)											//ÿ��̽ͷ����洢�ռ�Ϊ120�ֽڿɴ��60������
#define	All_Sample_Data_Len		(55*60*2)										//(6600���ֽ�)

#define FM_K1_K7_End_Addr		(Sample_Data_Addr+All_Sample_Data_Len)


/************************* W25Q256�洢����ͳһ��ַ�ļ� **************************/
#define Sector_Size				0x1000											//�����ߴ�
#define Sector_Max				256*32-1										//�ܹ�����������������������0��ʼ
#define Device_Number_Flash_Addr	0x08005F00									//0x5F00 �� 0x6000 �ռ�����д�����Ԥ��

#endif
