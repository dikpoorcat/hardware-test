#ifndef __RF_H
#define __RF_H
#include "main.h"


/*�궨��*/
#define RF_BuffLen 			500
#define RF_Task_Prio		RF_PRIO												//RF_Main_Task���������ȼ�

#define Sample_Wait_Time	60*5												//�ϵ�����ȴ�ʱ�䣬��λ����

#define	Tem_Upper			1250												//�������ޣ�125��	(0xf800>Temp)&&(Temp>Tem_Upper)
#define	Tem_Lower			0xfa26												//�������ޣ�-55��	Temp>Tem_Lower


#define	RF_READ_CMD 															/*������ģ������*/	\
{\
	0xff,0xff,0xff,0xff,0xff,	\
	0xff,0xff,0xff,0xff,0xff,	\
	0xff,0xff,0xff,0xff,0xff,													/*���ڻ��Ѵ���ģ��*/	\
	0xAA,0x2D,0x01,0x00,0xD8,0xBB,												/*��Ƶģ���ȡָ��*/	\
	0x0d,0x0a																	/*���ڽ�����־*/		\
}


//-----------------------------------RF��Դ��������----------------------------------------
#define PWRF_PIN			GPIO_Pin_2
#define PWRF_Port			GPIOD              									//С�������
#define PWRFDIS()			GPIO_ResetBits(PWRF_Port,PWRF_PIN);
#define PWRFEN()			GPIO_SetBits(PWRF_Port,PWRF_PIN);
#define PWRF_Port_CK		RCC_APB2Periph_GPIOD

#pragma pack(1)//�趨Ϊ1�ֽڶ���

/*�ṹ������*/
struct TT_STR{
	INT8U TT_Count;																//¼��̽ͷ�ĸ���
	INT8U TT_ID[55][2];															//����55��̽ͷID��ÿ��Ϊ����INT8U����
	INT8U HaveTT[55];															//���̽ͷ����λ���Ƿ���¼��̽ͷ
};

struct SAMP_MANAGE{
	INT8U 	Len;																//ÿ��̽ͷ�����ݳ��ȣ�(2*Sample_Num+5)�ֽ�
	INT8U	TT_Count;															//¼��̽ͷ�ĸ���
	INT8U 	Sample_Num;															//ʵ�ʲ����������ɳ���ͳ��
//	INT8U   Report_Num;															//��¼��Сʱ�Ĳ����������ϱ�������Report_Num<=Sample_Num;��ÿ��Сʱ�洢���ݵ�WQ256ʱ����Report_Num=Sample_Num�����Ǹ�Сʱ�ļ����ϴ���ɣ���δ��ɣ������´��ϱ�ʱ��ȡ�ļ���Report_Num��ʼ����
	INT32U 	Time[60];															//ÿ�εĲ���ʱ�䣬�����룬��������Э��ÿ��Сʱ����60��
	INT16U 	crc;
	INT8U	Newline[2];															//�̶�0x0D 0x0A
};

struct LteWakeupEnable{															//trueʹ�ܣ�false��ֹ
	bool	overtime;															//�ϱ������޻ظ�����2�����ղ���ȷ���ź��ٴη��ͣ�����3�β��ɹ���ֹͣ���ͣ����´η��Ͳɼ����ݳɹ����ٴδ��͡�
	bool	reply;																//��վ��Ӧ����ϣ���ֹ����LTE
	bool	network;															//����������ϣ���ֹ����LTE
	bool	battle;																//��ԴǷѹ���ϣ���ֹ����LTE
	bool	rf_tem;																//��Ƶ�¶������쳣���ϣ�400+�������̵ȣ�����ֹ����LTE
};

#pragma pack()//�趨Ϊ1�ֽڶ���

/*ȫ�ֱ�������*/
extern struct TT_STR				TT_Info;
extern struct SAMP_MANAGE			TT_Sample_Manage;
extern struct LteWakeupEnable 		wakeup_en;									//��ͨ�Ź��ϡ������硢��ԴǷѹ������½�ֹ������ģʽ���ѡ�

extern INT8U				First_Flag;
extern INT16U				HT_Data[2];											//������ʪ�����ݣ�ʪ�ȣ��¶�=================�����𣿣���û�������
extern INT8U				FATFS_Lock;


/*��������*/
void Task_RF_Main(void *arg);
void RF_Receive_Data(u32 rate, u16 timeout);
void RF_Power_Init(void);
void RF_Uart_init(unsigned int rate);
void RF_LowPower(void);
INT8U CMP_TT_ID(INT8U* pIn);
void Read_TT_From_FM(void);
void History_Data_Store(void);
void RF_Data_Sample(u16 timeout);
void RF_Fault_Judge(INT8U Index);
INT8U RfModuleTest(void);
INT8U RfReceivedPrint(struct Str_Msg * pMsg);
#endif
