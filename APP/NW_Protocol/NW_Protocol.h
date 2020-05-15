#ifndef __NW_PROTOCOL_H
#define __NW_PROTOCOL_H

#include "main.h"


/*�궨��*/
#define PROTOCOL_VERSION_H	0x03												//�淶�汾�ţ���һ�ֽڴ���淶���汾�ţ��ڶ��ֽڴ���淶�ΰ汾�ţ���0200H������V2.0�汾��
#define PROTOCOL_VERSION_L	0x00												//��ǰ�汾��ΪV3.0��
#define FI_NUM				30													//��������Ϣ��  ÿ������6�ֽ�
#define FAULT_STA			0x00												//����ʱ���λΪ0
#define NOFAULT_STA			0x80												//���ϻָ�ʱ���λΪ1
/*�Զ������type*/
/*
	0~54������������5�������������޷�������ƵͨѶ��55��̽ͷ��				01H����
	55~109�������������¶������쳣��55��̽ͷ��							02H����
*/
#define RTC_F				110													//ʱ���쳣/�ָ�
#define STORAGE_F			111													//�洢�쳣/�ָ�
#define REPLY_F				112													//30��������վ��Ӧ��/�ָ�
#define NETWORK_F			113													//30�������޷���½��������/�ָ�
#define BAT_F				114													//���ص�ԴǷѹ/�ָ�
//#define RF_F				6													//5�������������޷�������ƵͨѶ/�ָ�
//#define TEM_F				7													//�����¶������쳣
//#define CUR_F				8													//���ߵ��������쳣
//#define POWER_F			9													//���߲�����װ�ù��粻��

#define Start_Code			0x68
#define Epilog_Code			0x16

#define START_UP			0x00
#define TIMMING				0x01
#define SET_PASSWORD		0x02
#define PARA_CONFG			0x03
#define HEARTBEAT			0x05
#define SET_IP				0x06
#define DEMAND_IP			0x07
#define RESET_DEV			0x08
#define SMS_AWAKE			0x09
#define DEMAND_CONFG		0x0A
#define FUN_CONFG			0x0B
#define SLEEP_NOTICE		0x0C
#define DEMAND_DEV_TIM		0x0D
#define SMS_SEND			0x0E
#define DATA_REQUEST		0x21
#define TEM_CUR_UPLOAD		0x26
#define FAULT_INFO			0x30
#define FLOW_DATA_UPLOAD	0x40
#define FILE_LIST_QUERY		0x71
#define FILE_REQUEST		0x72
#define FILE_UL_REQUEST		0x73
#define FILE_UPLOAD			0x74
#define FILE_UL_END			0x75
#define FILE_FILLING		0x76
/*������չЭ��*/
#define EXTEN_ONOFF			0xF0
#define EX_SET_APN			0xF3
#define EX_DEMAND_APN		0xF4
#define EX_HEARTBEAT		0xF5
#define EX_VERSION_SIM		0xF6
#define EX_UPDATA_REQUEST	0xF7
#define EX_UPDATA_DOWNLOAD	0xF8
#define EX_UPDATA_DL_DONE	0xF9
#define EX_UPDATA_FILLING	0xFA
#define EX_FORMAT_FLASH		0xFB
/*������ָ�������ͨ��*/
#define ANY_CMD				0xFF
#define DATA_UNCORRESPOND	0xDE
#define PASSWORD_ERR		0xDD
#define REC_AND_EXE			0xDC
#define QUERY_MAIL			0xDB


/*�ļ������붨��*/
#define JPEG_FILE			0x01												//ͼ���ļ�����
#define FLW_FILE			0x02												//���϶�λ�����ļ�����
#define ANY_FILE			0xFF												//�����ļ�����


/*��С��ת��*/
#define htons(A) \
	((((u16)(A) & 0xff00) >> 8 ) | \
	 (((u16)(A) & 0x00ff) << 8 ))
#define htonl(A) \
	((((u32)(A) & 0xff000000) >> 24) | \
	 (((u32)(A) & 0x00ff0000) >> 8 ) | \
	 (((u32)(A) & 0x0000ff00) << 8 ) | \
	 (((u32)(A) & 0x000000ff) << 24))
#define ntohs     htons
#define ntohl     htohl



/*�ṹ�嶨��*/

#pragma pack(1)	//�趨Ϊ1�ֽڶ���

struct NW_TIME 																	//������ʽʱ��ṹ�� ����+��+��+ʱ+��+�룩��6�ֽڣ�HEX��ʾ��
{
	INT8U	year;																//��ݣ�Ϊ��ǰ��ݼ�ȥ2000
	INT8U	mon;																//�£�1-12
	INT8U	mday;																//�գ�1-31
	INT8U	hour;																//ʱ��0-23
	INT8U	min;																//�֣�0-59
	INT8U	sec;																//�룬0-59
};

struct NW_CONFIG		/*�������ýṹ��*/
{
	INT8U  Password[4];															//��ʼ����
	INT8U  BeatTime[1];															//���������װ��������Ϣ���ͼ������λ����
	INT8U  ScanInterval[2];														//�ɼ��������ÿ�����ٷ��Ӳ���һ�Σ��ɼ����������ʱ���޹أ�����λ����
	INT8U  SleepTime[2];														//����ʱ�������ݲɼ����ܹرջ�ͨ���豸����ʱ�䣬��ʱ���ڿ�֧�ֶ��Ż����绽�ѣ���λ���ӣ���Ϊ0��װ�ò�����
	INT8U  OnlineTime[2];														//����ʱ����ͨ���豸�������ݲɼ�������ͨ���豸����ʱ�䣬��λ����
	INT8U  ResetTime[3];														//Ӳ������ʱ��㣬Ϊ��֤װ������ɿ�����װ��Ӧ֧�ֶ�ʱ������ʱ����ʽ���գ�ʱ���֡��գ�0��28�գ�������Ϊ00H���ÿ�춨ʱ������
	INT8U  SecurityCode[4];														//������֤�룬Ϊȷ��װ�����ݵ���ȷ�ԣ���ֹ�Ƿ��û�������ƭ�����������������ڷ�ֹ�Ƿ�װ���û������ݱ���վ�Ͽɣ���װʱװ���趨Ĭ�����ģ�������װ��ɺ���վ�·�ָ���޸ĸ�װ�����ģ���װ����վ��¼������һ��ʱ�Ӹ����ݺϷ���Ч���������Ρ�
};

/*------------------------------------------------------------
����	��վIP	�˿ں�	��վIP	�˿ں�	��վ����	��վ����
4�ֽ�	4�ֽ�	2�ֽ�	4�ֽ�	2�ֽ�	6�ֽ�		6�ֽ�		
------------------------------------------------------------*/
struct NW_IP_CONFIG		/*��վIP��ַ���˿ںźͿ������ýṹ��*/
{
	INT8U	IP_addr_1[4];														//��վIP����׼4�ֽ�IP
	INT8U	PortNum_1[2];														//�˿ںţ����ֽڳ���256���ϵ��ֽ�
	INT8U	IP_addr_2[4];														//��վIP������ģ�����
	INT8U	PortNum_2[2];														//�˿ں�������ģ�����
	INT8U	CardNum_1[6];														//��վ���ţ�ΪF��ͨ�ſ��ţ�ÿ������ռ����ֽڡ��翨��Ϊ13912345678����������Ϊ��F1H��39H��12H��34H��56H��78H
	INT8U	CardNum_2[6];														//��վ����������ģ�����
};

struct NW_FLOW_DATA
{
	struct NW_TIME	Sample_Time;												//������ʱ�䣨��+��+��+ʱ+��+�룩��6�ֽڣ�HEX��ʾ��
	INT8U			Day_Used[4];												//��������������4�ֽڣ�
	INT8U			Month_Used[4];												//��������������4�ֽڣ�
	INT8U			Month_Surplus[4];											//����ʣ��������4�ֽڣ�
};

struct NW_TEM_DATA
{
	INT8U			Frame_ID;													//֡��ʶ��1�ֽڣ�
	INT8U			Pack_Num;													//������1�ֽڣ�
	INT8U			Unit_ID;													//���ܵ�Ԫʶ���루1�ֽڣ�
	struct NW_TIME	Sample_Time;												//������ʱ�䣨��+��+��+ʱ+��+�룩��6�ֽڣ�HEX��ʾ��
	INT8U			Tem[2];														//����¶ȣ�2�ֽڣ�
	INT8U			Cur[2];														//���ߵ�����2�ֽڣ�
	INT8U			Voltage;													//������������ѹ��1�ֽڣ�
};

struct NW_FAULT_INFO
{
	INT32U	Time;																//�����룬����/�ָ�ʱ��
	INT8U	Function_Code;														//���ܱ���
	INT8U	Fault_Code;															//���ϱ���
};

struct LOCAL_FLOW_DATA
{
	INT8U	Month;																//�·ݼ�¼
	INT8U	Date;																//���ڼ�¼
	INT32U	Flow_Day_Used_B;													//������ʹ����������λ���ֽ�
	INT32U	Flow_Month_Used_B;													//������ʹ����������λ���ֽ�
};

struct NW_FAULT_MANAGE 															//������Ϣ����ṹ�� �����͹��ϣ�0x55��ǹ��Ϸ�����δ�ָ����ָ�������
{
	INT8U	Need_Report;														//�����Ƿ��й�����Ϣ�����ϻָ���Ϣ����Ҫ�ϱ� ���ϱ���ɺ����
	INT8U	F_RTC;																//���ص�Ԫʱ���쳣
	INT8U	F_STORAGE;															//���ص�Ԫ�洢�쳣
	INT8U	F_REPLY;															//DTUģ��30����վ��Ӧ��
	INT8U	F_NETWORK;															//DTUģ��30�����޷���½��������
	INT8U	F_BAT;																//��Դ����ģ�����ص�ԴǷѹ
	INT8U	F_RF[55];															//̽ͷ5�������������޷�������ƵͨѶ���޷��ɼ�����Ч���ݣ�
	INT8U	F_TEM[55];															//̽ͷ�����¶������쳣
};

#pragma pack()	//�趨Ϊ1�ֽڶ���


/*ȫ�ֱ�������*/
extern const INT8U				Unit_ID_Code[55];									//���ܵ�Ԫʶ�����
extern struct NW_CONFIG			Config;
extern struct NW_IP_CONFIG		IP_Config;
extern struct NW_FLOW_DATA		Flow_Data;
extern struct NW_FAULT_MANAGE	Fault_Manage;									//���Ϲ���ṹ��										
extern struct NW_FAULT_INFO		Fault_Info[FI_NUM];									//������Ϣ�ṹ�����ݣ����洢FI_NUM������
extern struct LOCAL_FLOW_DATA	Local_FLow_Data;
extern INT8U 					Unreport_Index[31][3];								//δ�ϱ�����������31�죬ÿСʱ1bit������ֽڴ���0ʱ����1���ϱ���0δ�ϱ�
extern INT8U 					Device_Number[6];									//6Byteװ�ú���
extern INT8U 					FUN_Config[24];										//24Byte�������ò��������24��ܣ�Ĭ����Ч
extern INT8U					Time_Proofread;
extern INT8U					APN[100];											//������չЭ������APN����



/*��������*/
INT8U NW_Comm_Process(void);
INT8U Startup_Comm(u8 times, u16 timeout);
INT8U Timming_Request(u8 times, u16 timeout);
INT8U *Heartbeat(u8 times, u16 timeout);
INT8U Fault_Info_Comm(u8 times, u16 timeout);
INT8U Set_Password_Comm(u8 *InBuff, u16 Len);
INT8U ParaConfigComm(u8 *InBuff, u16 Len);
INT8U Set_IP_Comm(u8 *InBuff, u16 Len);
INT8U Fun_Config_Comm(u8 *InBuff, u16 Len);
INT8U SMS_Send_Comm(u8 *InBuff, u16 Len);
INT8U SMS_Judge(u8 *InBuff, u16 Len);
INT8U Data_Request_Comm(u8 *InBuff, u16 Len);
INT8U Flow_Data_Upload(u8 times, u16 timeout);
INT8U Sleep_Notice(void);
INT8U Tem_Cur_Upload(u8 times, u16 timeout);
INT8U Tem_Cur_Sample_Upload(u8 times, u16 timeout);
INT16U NW_History_Temp_Comm(u16 Bit_Index, u8 times, u16 timeout);
INT16U Search_Info(u8 *InBuff, u16 Len);
//INT16U NW_History_Temp_Framing(u8 Frame_ID, u8 *InBuff, struct SAMP_MANAGE *Info_str, u8 *OutBuff);
void Tem_DS18B20_To_NW(INT8U* Outbuff,INT8U* InBuff);
INT8U File_List_Query_Comm(u8 *InBuff, u16 Len);
INT8U Files_Upload(u8 *FileName,u16 Des_Len);
INT16U NW_File_Framing(u8 Cmd,u8 *FileName,u8 Pac_Num,u8 Des_Len,u8 *OutBuff);
INT8U NW_ReceiveAndExecute(u8 *OutBuff,u16 timeout_s);
bool PassworkCheckAndReport(INT8U *Inbuff);
INT16U NW_Framing(u8 Cmd,u8 *OutBuff);
INT8U *Judge_NW_Framing(u8 Cmd,u8 *InBuff,u16 Len,u16 *OutLen);
INT16U Get_DataField(u8 Cmd,u8 *OutBuff);
INT8U SetTime(u8 *InBuff);
INT8U NW_GetTime(struct NW_TIME *time);
INT8U SecondToNwTime(INT32U sencond,struct NW_TIME *time);
INT32U NwTimeToSecond(struct NW_TIME *ptime);
INT8U Get_HB_INFO(u8 *OutBuff);
INT8U Get_NW_Info(void);
void NW_Fault_Manage(INT8U type, INT8U fault_state);
void NW_DeviceNumberToAscii(INT32U InData,INT8U *pOut);

#endif
