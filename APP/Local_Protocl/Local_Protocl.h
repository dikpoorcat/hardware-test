#ifndef	__Local_Protocl_H
#define	__Local_Protocl_H
#include "main.h"


/* --------------------------------Private define---------------------------------------------*/
/*ͨ��*/
#define DONE				1													//�����ĳ�ֶ���
#define UNDONE				0													//δ���ĳ�ֶ���

/*��λ*/
#define HOTRST				0xAA												//�ȸ�λ���ɳ�����Ƹ�λ
#define COLDRST				0xBB												//�临λ�����⸴λ
#define FAULTRST			0xCC												//���ϸ�λ����ָLTE��������и�λ��һ��Ϊ�临λ
#define NORST				0xFF												//���ڷǸ�λ�����������������

/*����*/
#define Local_Task_Prio		LOCAL_PRIO
#define Buff485_LEN 		1500
#define BAT_UNDER			9.2
#define BAT_UP				9.5
#define FALA_UNDER			5
#define FALA_UP				5.5

/*����Э��DoType*/
#define READTYPE			1
#define WRITETYPE     		0

/*����Э��������*/
#define No_Err				00													//�޴���
#define Invalid_ID			01													//ID������ЧID ID�б��еģ�ID���������е��豸��֧��
#define Password_Err		02													//������󡾴���3�κ��豸��ס�������Զ�������
#define Data_Err			03													//ID��Ӧ���ݴ���
#define Write_Err			04													//IDΪֻ������д
#define Read_Err			05													//IDΪֻд���ɶ�
#define Other_Err			0xFF												//����Э������Щ����������û�õģ�ͳһʹ���������������


/* --------------------------------Private typedef--------------------------------------------*/
struct LOCAL_PROTOCAL
{
	INT8U  Addr[4];																//����װ�õ�ַ��Ĭ��Ϊ0x00, 0x00, 0x00, 0x00
	INT8U  DoType;  															//0 д��  1 ��ȡ==============CMD
	INT8U  *Pointer;
	INT8U  Password[3];															//�������룬Ĭ��Ϊ0x00, 0x00, 0x00
	INT16U CMD;																	//CMD����
	INT16U CMDlen;																//CMD�����Ӧ���������ݳ���
};

struct HARDWARE_TEST															//��Ÿ�Ӳ�������
{
	INT8U					b485;												//485
	INT8U					encryption_chip;									//����оƬ
	INT8U					ferroelectric_ram;									//����洢оƬ
	INT8U					flash_memory;										//�ⲿFLASH�洢оƬ
	INT8U					rtc;												//RTC
	INT8U					power_supply;  										//��Դ
	INT8U					ds18b20;   											//DS18B20
	INT8U					lora;   											//LORAģ��
	INT8U					meteorology;										//΢����
	INT8U					lte;												//LTEģ��
};

/* ------------------------------Private variables--------------------------------------------*/
extern struct LOCAL_PROTOCAL	Local_Protocal;
extern INT8U					Reset_Flag;    									//�临λ��Ĭ��ֵ�����ȸ�λ�������и�λ
extern INT8U					Reset_Count;          							//������������ʶ

extern INT8U					B485BUF[Buff485_LEN];
extern OS_EVENT  				*GyBOX;

extern BYTE						work[FF_MAX_SS];								/* Work area (larger is better for processing time) */   //FF_MAX_SS
extern FIL						fil;            								/* File object */
extern FATFS					fs;												/* Filesystem object */


/* -----------------------------Private functions---------------------------------------------*/
void Task_Local_main(void *arg);
INT8U Check_Reset_Mod(u8 waittime);
INT8U Wait_Local_Comm(INT8U WaitTime);
INT8U Local_Protocol_Process(INT8U *In,INT16U inLen,INT8U *pUseBuff);
INT8U * Judge_Local_Framing(INT8U *pInBuff,INT16U Len,INT16U *pOutLen);
INT8U Set_Local_Para(INT16U CMD,INT8U *pInBuff,INT8U Len);
INT8U Get_Local_Para(INT16U CMD,INT8U *pOutBuff,INT16U *pOutLen);
INT8U System_Time_Fun(INT8U *pInOutBuff,INT8U Type);
INT8U Judge_Device_Addr(INT8U *pAddr);
INT8U Local_Function(INT8U *InBuff);
void Local_Protocol_Reply(INT8U Err,INT16U CMD,INT8U *pUseBuff);
void Local_Protocol_Reply_Para(INT8U *In,INT16U len,INT16U CMD,INT8U *pUseBuff);
INT8U Add_TT_ID(INT8U *pInBuff)	;
INT8U Delete_TT_ID(INT8U *pInBuff);
INT8U Delete_All_TT_ID(void);
INT8U Read_TT_Num_Or_ID(INT16U ID,INT8U *pOutBuff);
void FM_Space_Usage(void);
void Print_Config(INT8U cmd);
void DrawSysLogo(void);
void DrawErrLogo(void);
void HardwareTest(void);

/*FATFSӦ��*/
FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
);
FRESULT Dir_Maintenance (void);
void Dir_Test(void);
void Check_Getfree(void);
void File_Test(void);

#endif

