/***************************** (C) COPYRIGHT 2019 ���ϵ��� *****************************
* File Name          : NW_Protocol.c
* Author             : ��ӱ�ɡ���������
* Version            : ����ʷ�汾��Ϣ
* Date               : 2019/02/21
* Description        : ��������Э���д��ͨ�Ź��ܺ�����
************************************  ��ʷ�汾��Ϣ  ************************************
* 2019/03/28    : V4.1.0
* Description   : ����������Ŀ���档����������ɣ������С�
*******************************************************************************/
#include "NW_Protocol.h"

/*ȫ�ֱ���*/
struct NW_CONFIG Config={
	{0x31,0x32,0x33,0x34},														//װ�ó������룺�ַ�����1234����31H32H33H34H��
	{0x01},																		//�����������������ӦΪ1����
	{0x00,0x14},																//�ɼ��������������ӦΪ20����
	{0x00,0x37},																//����ʱ����������Ϊ55����
	{0x00,0x05},																//����ʱ����������Ϊ5����
	{0x00,0x0A,0x1E},															//ʱ����ʽ���գ�ʱ����	�գ�0��28�գ�������Ϊ00H���ÿ�춨ʱ��������ʱ��0��23���֣�0��59��
	{0x31,0x32,0x33,0x34}														//װ�ó�ʼ������֤Ϊ�ַ���1234����31H32H33H34H��
};

/*------------------------------------------------------------
����	��վIP	�˿ں�	��վIP	�˿ں�	��վ����	��վ����
4�ֽ�	4�ֽ�	2�ֽ�	4�ֽ�	2�ֽ�	6�ֽ�		6�ֽ�		
------------------------------------------------------------*/
struct NW_IP_CONFIG IP_Config={
	{118,190,141,140},															//��������Ϊ118.190.141.140
	{0x1D,0xBB},																//��������Ϊ7611
	{0x2F,0x64,0x23,0xC8},														//��������Ϊ
	{0x17,0x71},																//��������Ϊ
	{0x00,0x00,0x00,0x00,0x00,0x00},											//��������Ϊ
	{0x00,0x00,0x00,0x00,0x00,0x00},											//��������Ϊ
};

struct NW_FLOW_DATA Flow_Data={													//��������λMB
	{0x00,0x00,0x00,0x00,0x00,0x00},											//������ʱ�䣨��+��+��+ʱ+��+�룩��6�ֽڣ�
	{0x00,0x00,0x00,0x00},														//��������������4�ֽڣ�
	{0x00,0x00,0x00,0x00},														//��������������4�ֽڣ�
	{MF_0,MF_1,MF_2,MF_3},														//����ʣ��������4�ֽڣ�Ĭ��MONTHLY_FLOW
};

struct NW_TEM_DATA Tem_Cur_Data={												//��������վ����װ�����ݺ������ϴ�ʱ��֡
	0,																			//֡��ʶ��1�ֽڣ�
	1,																			//������1�ֽڣ�
	0,																			//���ܵ�Ԫʶ���루1�ֽڣ�
	{0x00,0x00,0x00,0x00,0x00,0x00},											//������ʱ�䣨��+��+��+ʱ+��+�룩��6�ֽڣ�
	{0x00,0x00},																//����¶ȣ�2�ֽڣ�����ֵ=��ʵ���¶�+50��*10
	{0x00,0x00},																//���ߵ�����2�ֽڣ�����ֵ=ʵ�ʵ���*10
	33,																			//������������ѹ��1�ֽڣ�����ֵ=ʵ�ʵ�ѹ*10
};

const INT8U Unit_ID_Code[55]={													//���ܵ�Ԫʶ�����
	/*���߲ഫ����20��*/
	0x15,	0x16,	0x17,	0x18,												//������λ1
	0x25,	0x26,	0x27,	0x28,												//������λ2
	0x35,	0x36,	0x37,	0x38,												//������λ3
	0x45,	0x46,	0x47,	0x48,												//������λ4
	0x55,	0x56,	0x57,	0x58,												//������λ5
	/*����λ��չ35��*/
	0x19,	0x1A,	0x1B,	0x1C,	0x1D,	0x1E,	0x1F,						//������λ1
	0x29,	0x2A,	0x2B,	0x2C,	0x2D,	0x2E,	0x2F,						//������λ2
	0x39,	0x3A,	0x3B,	0x3C,	0x3D,	0x3E,	0x3F,						//������λ3
	0x49,	0x4A,	0x4B,	0x4C,	0x4D,	0x4E,	0x4F,						//������λ4
	0x59,	0x5A,	0x5B,	0x5C,	0x5D,	0x5E,	0x5F,						//������λ5
};

struct LOCAL_FLOW_DATA Local_FLow_Data={0};										//���ؽ�������ͳ�ƵĽṹ��
struct NW_FAULT_INFO Fault_Info[FI_NUM]={0};									//������Ϣ�ṹ�����ݣ����洢FI_NUM�����ϡ��������Ϻ͹��ϻָ�������δ�ϱ���
struct NW_FAULT_MANAGE  Fault_Manage={0};										//������Ϣ����ṹ�壬ÿ�ֹ��ϵı�־λ����ط�

INT8U 				Unreport_Index[31][3]={0};									//δ�ϱ�����������31�죬ÿСʱ1bit�����bit����0ʱ����1���ϱ���0δ�ϱ�	ע�����豸��ʼ��ʱ��ȫ��д1
INT8U 				Device_Number[6]="FC0000";									//6Byteװ�ú���
INT8U 				FUN_Config[24]={0x26,0x30};									//24Byte�������ò��������24��ܣ�Ĭ��26H�����¶ȡ��������ݼ�⹦��\30H�豸�����Լ칦��	PS�������ݲ�֧�ֹ������ã��̶���������������
INT8U				Time_Proofread = DONE;										//��ʼ��ΪDONE���豸�ϵ��ֻҪRTCʱ���ʽ�������ɲɼ��¶ȣ���ֹ��������ʱ���޷�Уʱ�����ɼ�
INT8U				Tem_Sampled[2] = {0};										//���������ɼ����¶�������֡�ϱ�
INT8U				APN[100] = "CMIOT";											//������չЭ������APN������ܳ���100���ֽڣ�����		"CMNET"	"CMMTM"	"CMIOTGZDWSCSPJK.GZ"






/*******************************************************************************
���ƣ�void NW_Comm_Process(void)
���ܣ���������Э���д��ͨ�����̣�����ͨ�ż�����ִ�еȡ�����ʱ��ʼ״̬Ϊ��������̬��
��Σ���
���Σ���
���أ�1����������ִ�У�0�����쳣
*******************************************************************************/
INT8U NW_Comm_Process(void)														
{
	INT8U			First_Time = 0xFF;											//����ʱ��������һ��TEM_CUR_UPLOAD
	INT16U			WaitTime = 0;
	INT8U			STATE = START_UP;											//��ʼ״̬��Ϊ��������
	static INT8U	msg_cmd;
	static INT8U	msg_state;
	static INT8U	msg_fault;
	INT8U			Err = 0;
	
	while(1)
	{
		WDTClear(LTE_PRIO);														//��������Ź�
		switch(STATE)
		{
			case START_UP:
				/*�ж��Ƿ�Ҫ���Ϳ���������Ϣ*/					
					if(Reset_Flag != NORST){									//���ո�λ�������������޸Ĵ�ȫ�ֱ�����
						BspUartWrite(2,SIZE_OF("\r\n---------->��������<----------\r\n"));
						if(!Startup_Comm(RETRY,TIMEOUT)) return 0;				//��������ͨ�ţ�ʧ��ʱ����0
					}
					STATE = HEARTBEAT;											//��������ͨ�ųɹ�����һ������ͨ��
					break;														
					
			case HEARTBEAT:
					BspUartWrite(2,SIZE_OF("\r\n---------->�����ϱ�<----------\r\n"));
					if(!Heartbeat(RETRY,TIMEOUT)) return 0;						//����ͨ�ţ�ʧ��ʱ����0
				/*�ж��Ƿ�Ҫ���͹�����Ϣ*/
					if(Reset_Flag != NORST||Fault_Manage.Need_Report)			//���ո�λ�������������޸Ĵ�ȫ�ֱ�����  ���й�����Ϣ��Ҫ�ϱ�ʱ�������ϱ��ٽ���ȴ�ָ��
					{									
						Reset_Flag = NORST;										//��ֵΪNORST�����ڷǸ�λ�����������������͹����ϱ������Ǹ�λ�������ٸı��ֵ
						STATE = FAULT_INFO;										//��λ����Ҫ�ϱ�����״̬����
					}
					else STATE = REC_AND_EXE;									//����ͨ�ųɹ�����һ���ȴ���λ��ָ�����һ���������ʱ�䣩	��Ҫֱ�ӽ���QUERY_MAIL״̬����������������
					break;
			
			case FAULT_INFO:
					BspUartWrite(2,SIZE_OF("\r\n---------->������Ϣ<----------\r\n"));
					if(!Fault_Info_Comm(RETRY,TIMEOUT)) return 0;				//������Ϣͨ�ţ�ʧ��ʱ����0
					STATE = REC_AND_EXE;										//����ͨ�ųɹ�����һ���ȴ���λ��ָ�����һ���������ʱ�䣩	��Ҫֱ�ӽ���QUERY_MAIL״̬����������������
					break;
			
			case REC_AND_EXE:
					BspUartWrite(2,SIZE_OF("\r\n---------->�ȴ�����<----------\r\n"));
					WaitTime = *Config.BeatTime*60;								//������ʱʱ�䣬��λ����
				/*��������״̬WaitTime�루������������������������˳��һ����������*/
					STATE = NW_ReceiveAndExecute(LTE_Rx_Buff,WaitTime);	//���ղ�ִ�гɹ�������һ����Ҫִ�е�״̬��������/����/Уʱ�ȣ���δִ���κβ����򷵻�0
					if(STATE==1) STATE = REC_AND_EXE;							//STATE==1��ʾ�Ѿ��ɹ�ִ����ĳЩ���ܣ��ȴ���һ��ָ�����˳��һ��������
					else if(STATE==0) 	 										//����0��ʾ������������ʧ�ܣ���ѯ����ָ��
					{
						if(First_Time) 
						{
							STATE = TEM_CUR_UPLOAD;								//���ε���ʱ��������һ��TEM_CUR_UPLOAD
							First_Time = 0;										//��0��֮���������ϱ�
						}
						else STATE = QUERY_MAIL;								//�������ϱ�ʱ����ѯ����
					}	
					/*����������Զ�����STATE״̬ѡ����RESET_DEV��*/
					break;
					
			case TEM_CUR_UPLOAD:
					BspUartWrite(2,SIZE_OF("\r\n---------->��ʷ����<----------\r\n"));
					Err = Tem_Cur_Upload(RETRY,TIMEOUT);						//������ʷ����
					if(0xFF==Err) STATE = QUERY_MAIL;							//����ʷ���ݣ������εȴ�
					else if(!Err) return 0;										//������ʷ����ʧ��ʱ����0������ʾ������ʷ����δ������ɣ���Ӱ������ɵĲ��֣�
					else STATE = REC_AND_EXE;									//�ϱ��ɹ�����һ���ص��ȴ�״̬
					BspUartWrite(2,SIZE_OF("---------->��ʷ�����ϱ�����<----------\r\n"));
					break;
			
			case SMS_AWAKE:														//��վ��ͨ��IP����UDPͨ�ţ����ʹ˿����֣�Ҳ��ʾ�����ն�
					BspUartWrite(2,SIZE_OF("\r\n---------->����ָ��<----------\r\n"));
					msg_state = WAKE_SUCCESS;
					OSMboxPost(Dev_STAB0X, &msg_state);							//ͨ�����佫״̬�仯�����������������ʱ��
					STATE = REC_AND_EXE;										//�����л�������״̬����Ч�ڻص��ȴ�״̬
					break;	
			
			case SLEEP_NOTICE:
					BspUartWrite(2,SIZE_OF("\r\n---------->����֪ͨ<----------\r\n"));
					Sleep_Notice();												//����Ϣ��װ���豸ÿ������֮ǰ�ϱ���վ
					return SLEEP_NOTICE;										//���أ���������
			
			case RESET_DEV:
					BspUartWrite(2,SIZE_OF("\r\n---------->װ������<----------\r\n"));
					return RESET_DEV;											//���أ�����װ��
									
			case QUERY_MAIL:
					/*���ȴ��������Ϣ*/
					msg_fault = *(INT8U *)OSMboxPend(Fault_CMDB0X,1,&Err);		//���������ѯ��������Ϣ
					if(msg_fault==FAULT_CMD) 									//��Ҫ�ϱ�
					{
						STATE = FAULT_INFO;										//���������Ϣ�ϱ�״̬
						break;													
					}
					/*�ȴ�����ָ��*/
					msg_cmd = *(INT8U *)OSMboxPend(Dev_CMDB0X,1,&Err);			//���������ѯ��ֻ��WAKE��SLEEP���������Ϣ������Ҳ�����ط���
					if(msg_cmd==SLEEP_CMD) STATE = SLEEP_NOTICE;				//��������״̬
//					else if(msg==UPLOAD_CMD) STATE = TEM_CUR_UPLOAD;			//�����¶ȵ�������״̬�������ϱ����������
					else STATE = HEARTBEAT;										//����Ҫ������ָ�����������ڣ�����������
					break;
			
			default:
					BspUartWrite(2,SIZE_OF("\r\n!!!!!!!!!!!!!!!!!!!!>���棡NW_Comm_Process()����δ֪״̬��<!!!!!!!!!!!!!!!!!!!!\r\n"));
					return 0;
		}
	}	
}

/*******************************************************************************
���ƣ�INT8U Startup_Comm(u8 times, u16 timeout)
���ܣ��������磬����times�Σ������֣�00H
װ��ÿ�η��Ϳ���������Ϣ����վ�޷�����Ϣ��ÿ1���ӷ���һ��ֱ���յ���վ������Ϣ��
��Σ�u8 times�����Դ�����u16 timeout����ʱ�ȴ�ʱ�䣬��λ��ʱ��Ƭ
���Σ���
���أ��������粢Уʱ�ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U Startup_Comm(u8 times, u16 timeout)
{
	INT16U	len_frame=0;														//֡����
	INT8U	i=0;
	INT16U	len=0;
	
	len_frame=NW_Framing(START_UP,LTE_Tx_Buff);									//�鿪������֡
	for(i=0;i<times;i++)														//�������times��	װ��ÿ�η��Ϳ���������Ϣ����վ�޷�����Ϣ��ÿ1���ӷ���һ��ֱ���յ���վ������Ϣ��
	{
		len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,timeout);		//����Ϊ0��ʾͨ��ʧ�ܣ�����0xff��ʾû���յ����ݣ���������Ϊ�������ݳ���
		if(!len) 
		{
			BspUartWrite(2,SIZE_OF("δ�յ���վ�ظ���\r\n"));
			if(i<times-1) OSTimeDly(1*3*20-timeout);							//ͨ��ʧ��ʱÿ��3������һ��
			continue;															//����
		}
		if(Judge_NW_Framing(START_UP,LTE_Rx_Buff,len,0))						//�жϽ��������Ƿ��������Э�飬������ʱ����ͨ��
		{
			OSTimeDly(10);														//���ڵĺ�̨������������ɺ�Уʱ��ʱ�ظ������Ӹ���ʱ�ͺ���
			/*���յ��ظ�����������Уʱ*/
			return Timming_Request(10,20);/*timeout����Э��̶�Ϊ20��*/			//���װ���յ���վ�������緵����Ϣ����������Уʱ��ÿ��2��������һ��ֱ��Уʱ�ɹ�Ϊֹ��������ʱ������վӦ��֮����ʱ������20�룬����ܸ��������װ��ʱ�ӡ�
		}
	}
	BspUartWrite(2,SIZE_OF("Startup_Comm���Գ���������������ʧ�ܣ�\r\n"));
	return 0;																	//���Գ��������ȴ���ʱ������0
}

/*******************************************************************************
���ƣ�INT8U Timming_Request(u8 times, u16 timeout)
���ܣ���������Уʱ������times�Σ������֣�01H
���װ���յ���վ�������緵����Ϣ����������Уʱ��ÿ��2��������һ��ֱ��Уʱ�ɹ�Ϊֹ��������
ʱ������վӦ��֮����ʱ������20�룬����ܸ��������װ��ʱ�ӡ�
��Σ�u8 times�����Դ�����u16 timeout����ʱ�ȴ�ʱ�䣬��λ��ʱ��Ƭ  ������Ϊ50ms��ע�⣬��
��Э��Ҫ�󲻳���20��ʱ���ܸ������timeout<=20*20��
���Σ���
���أ�Уʱ�ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U Timming_Request(u8 times, u16 timeout)
{
	INT16U	len_frame=0;														//֡����
	INT8U 	*R;
	INT8U	i=0;
	INT16U	len=0;
	
	len_frame=NW_Framing(TIMMING,LTE_Tx_Buff);									//��Уʱ����֡
	for(i=0;i<times;i++)														//�������times��
	{
		len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,timeout);		//����Ϊ0��ʾͨ��ʧ�ܣ�����0xff��ʾû���յ����ݣ���������Ϊ�������ݳ���
		if(!len) 
		{
			BspUartWrite(2,SIZE_OF("δ�յ���վ�ظ���\r\n"));
			if(i<times-1) OSTimeDly(2*60*20-timeout);							//ͨ��ʧ��ʱÿ��2��������һ��
			continue;															//����
		}
		R = Judge_NW_Framing(TIMMING,LTE_Rx_Buff,len,0);						//�жϽ��������Ƿ��������Э��
		if(R) 																	//������Э��ʱ����ͨ��
		{
			/*ֻҪtimeout���ò�����20�룬������Э��Ҫ�󣬽��ܸ��������װ��ʱ��*/
			return SetTime(R);													//����RTC�����յ���������*R���ɹ�����1		
		}
	}
	BspUartWrite(2,SIZE_OF("Timming_Request���Գ�������Уʱʧ�ܣ�\r\n"));
	return 0;																	//���Գ��������ȴ���ʱ������0
}

/*******************************************************************************
���ƣ�INT8U *Heartbeat(u8 times, u16 timeout)
���ܣ���������������times�Σ�u16 timeout����ʱ�ȴ�ʱ�䣬��λ��ʱ��Ƭ
��Σ�u8 times�����Դ���
���Σ���
���أ�0 ͨ��ʧ�ܣ���0 ͨ�ųɹ����ҷ������ݰ����ֽڵ�ַ INT8U *
*******************************************************************************/
INT8U *Heartbeat(u8 times, u16 timeout)
{
	INT16U	len_frame=0;														//֡����
	INT8U	i=0;
	INT16U	len=0;
	INT8U	*R=0;
	
	/*����Э������*/
	len_frame=NW_Framing(HEARTBEAT,LTE_Tx_Buff);								//������֡
	for(i=0;i<times;i++)														//�������times��
	{	
		len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,timeout);		//����Ϊ0��ʾͨ��ʧ�ܣ�����0xff��ʾû���յ����ݣ���������Ϊ�������ݳ���
		if(!len) 
		{
			BspUartWrite(2,SIZE_OF("δ�յ���վ�ظ���\r\n"));
			continue;															//ͨ��ʧ��ʱ����
		}
		R = Judge_NW_Framing(HEARTBEAT,LTE_Rx_Buff,len,0);						//�жϽ��������Ƿ��������Э�飬�����ؽ��
		break;
	}
	
	/*��չЭ������*/
	len_frame=NW_Framing(EX_HEARTBEAT,LTE_Tx_Buff);								//����չ����֡
	for(i=0;i<times;i++)														//�������times��
	{	
		len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,timeout);		//����Ϊ0��ʾͨ��ʧ�ܣ�����0xff��ʾû���յ����ݣ���������Ϊ�������ݳ���
		if(!len) 
		{
			BspUartWrite(2,SIZE_OF("δ�յ���վ�ظ���\r\n"));
			continue;															//ͨ��ʧ��ʱ����
		}
		break;
	}
	
	if(!R) BspUartWrite(2,SIZE_OF("Heartbeat���Գ�����������ͨ��ʧ�ܣ�\r\n"));
	return R;																	//���Գ��������ȴ���ʱ������0
}

/*******************************************************************************
���ƣ�INT8U *Fault_Info_Comm(u8 times, u16 timeout)
���ܣ����͹�����Ϣ������times�Σ�u16 timeout����ʱ�ȴ�ʱ�䣬��λ��ʱ��Ƭ
��������ϱ�������Ϣ����������+������ɺ��¹��Ϸ�����ָ�ʱ��ͨ�Ź��ϻָ�ʱ��
��Σ�u8 times�����Դ���
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U Fault_Info_Comm(u8 times, u16 timeout)
{
	INT8U			i = 0;
	INT16U			len_frame,len;		
	INT8U			*R;

	/*��֡*/
	len_frame = NW_Framing(FAULT_INFO, LTE_Tx_Buff);							//��FAULT_INFO֡
	
	/*�ϱ����Ȼظ�������times��*/
	for(i=0;i<times;i++)														//�������times��
	{
		len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,timeout);		//����Ϊ0��ʾͨ��ʧ�ܣ�����0xff��ʾû���յ����ݣ���������Ϊ�������ݳ���
		if(!len)
		{
			BspUartWrite(2,SIZE_OF("δ�յ���վ�ظ���\r\n"));
			if(i<times-1) OSTimeDly(2*60*20-timeout);							//ͨ��ʧ��ʱÿ��2��������һ��
			continue;															//����
		}
		R = Judge_NW_Framing(FAULT_INFO,LTE_Rx_Buff,len,0);						//�жϽ��������Ƿ��������Э��
		if(R) 																	//����ʱ���룬������Э��ʱ����ͨ��
		{
			if(R[10]!=1) continue;												//�ж�֡��ʶ 1
			if(R[11]!=0xAA) continue;											//�жϻظ������� AA55H
			if(R[12]!=0x55) continue;											//�жϻظ�������	
			
			Fault_Manage.Need_Report=0;
			if(!BSP_WriteDataToFm(Fault_Manage_Addr,(INT8U*)&Fault_Manage,Fault_Magage_Len)) return 0;		//������Ϲ���ṹ��ġ���Ҫ�ϱ���־λ��
			
			memset(Fault_Info,0,Fault_Info_Len);															//��ṹ������
			if(!BSP_WriteDataToFm(Fault_Info_Addr,(INT8U*)Fault_Info,Fault_Info_Len)) return 0;				//���������Fault_Info������Ϣ�ṹ�����飨���Ͼ���Ľ��д��ȥ��
			
			wakeup_en.overtime = true;											//������LTE����
			return 1;															//�ϱ��ɹ�������1
		}
	}
	
	/*ͨ�ų�ʱ*/
	wakeup_en.overtime = false;													//��վδ�ظ����ϣ���ֹ����LTE����
	BspUartWrite(2,SIZE_OF("Fault_Info_Comm���Գ�������������Ϣͨ��ʧ�ܣ�\r\n"));
	return 0;																	//���Գ�������ͨ��ʧ�ܣ�����0				
}

/*******************************************************************************
���ƣ�INT8U Set_Password_Comm(u8 *InBuff, u16 Len)
���ܣ�����������顢�ظ�ͨ�š��������빦��ִ��
��Σ�u8 *InBuff����������ݣ�����Ϊ���յ���֡��u16 Len������
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U Set_Password_Comm(u8 *InBuff, u16 Len)
{
	if(!PassworkCheckAndReport(InBuff));										//�ж���վ�·������������Ƿ���ȷ������ȷʱ�ϱ����������Ϣ��
	else																		//����ͬ��������ȷ
	{
		memcpy(Config.Password,InBuff+14,4);									//���������Ϊ�����룬д��Config�ṹ�壨Ĭ�ϳɹ���
		BSP_WriteDataToFm(Config_Addr,&Config.Password[0],Config_Len);			//��������
		LteCommunication(InBuff,Len,0,0);										//���óɹ�������ԭ����أ������գ�LteCommunication����0��
		return 1;
	}
	BspUartWrite(2,SIZE_OF("Set_Password_Comm��������ͨ��ʧ�ܣ�\r\n"));
	return 0;
}

/*******************************************************************************
���ƣ�INT8U ParaConfigComm(u8 *InBuff, u16 Len)
���ܣ�����������顢�ظ�ͨ�š��������ù���ִ��
��Σ�u8 *InBuff����������ݣ�����Ϊ���յ���֡��u16 Len������
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U ParaConfigComm(u8 *InBuff, u16 Len)
{
	if(!PassworkCheckAndReport(InBuff));										//�ж���վ�·������������Ƿ���ȷ������ȷʱ�ϱ����������Ϣ��
	else																		//����ͬ��������ȷ
	{
	/*����Ϊ0�����ñ�����Ϊ0ʱ������ԭ����*/
		if(!InBuff[14]) InBuff[14]=Config.BeatTime[0];							//�������������Ϊ0������ԭ����
		if(!(InBuff[15]|InBuff[16])) memcpy(InBuff+15,Config.ScanInterval,2);	//�ɼ����������Ϊ0������ԭ����
		/*17��18 ����ʱ������Ϊ0*/
		if(!(InBuff[19]|InBuff[20])) memcpy(InBuff+19,Config.OnlineTime,2);		//����ʱ��������Ϊ0��������ԭ����
		/*21��22��23 Ӳ������ʱ������Ϊ0*/
		/*24��25��26��27 ������֤�����Ϊ0*/
		memcpy(&Config.BeatTime,InBuff+14,14);									//�������ã�д��Config�ṹ�壨Ĭ�ϳɹ���
		BSP_WriteDataToFm(Config_Addr,(u8 *)&Config,Config_Len);				//��������
		LteCommunication(InBuff,Len,0,0);										//���óɹ�������ԭ����أ������գ�LteCommunication����0��
		return 1;
	}
	BspUartWrite(2,SIZE_OF("ParaConfigComm��������ͨ��ʧ�ܣ�\r\n"));
	return 0;
}

/*******************************************************************************
���ƣ�INT8U Set_IP_Comm(u8 *InBuff, u16 Len)
���ܣ�����������顢�ظ�ͨ�š�����IP��������ִ��
��Σ�u8 *InBuff����������ݣ�����Ϊ���յ���֡��u16 Len������
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U Set_IP_Comm(u8 *InBuff, u16 Len)
{
	INT16U	len_frame=0;														//֡����
	
	if(!PassworkCheckAndReport(InBuff));										//�ж���վ�·������������Ƿ���ȷ������ȷʱ�ϱ����������Ϣ��
	else if(memcmp(InBuff+14,InBuff+20,6) || memcmp(InBuff+26,InBuff+32,6))		//��������վIP���˿ںź���վ���Ŷ�Ӧ�ֽڲ���ȫ��ͬ����ͬʱmemcmp����0��
	{
		len_frame=NW_Framing(DATA_UNCORRESPOND,LTE_Tx_Buff);					//�������Ϣ֡0��������Ϊ0000H��
		LteCommunication(LTE_Tx_Buff,len_frame,0,0);							//�ϱ�������Ϣ0�������գ�LteCommunication����0��
	}
	else																		//����ͬ��������ȷ
	{
		memcpy(&IP_Config,InBuff+14,24);										//IP�������ã�дIP_Config�ṹ�壨Ĭ�ϳɹ���
		BSP_WriteDataToFm(IP_Config_Addr,&IP_Config.IP_addr_1[0],IP_Config_Len);//��������
		LteCommunication(InBuff,Len,0,0);										//���óɹ�������ԭ����أ������գ�LteCommunication����0��
		return 1;
	}
	BspUartWrite(2,SIZE_OF("Set_IP_Comm����IPͨ��ʧ�ܣ�\r\n"));
	return 0;
}

/*******************************************************************************
���ƣ�INT8U Fun_Config_Comm(u8 *InBuff, u16 Len)
���ܣ�����������顢�ظ�ͨ�š���������
��Σ�u8 *InBuff����������ݣ�����Ϊ���յ���֡��u16 Len������
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U Fun_Config_Comm(u8 *InBuff, u16 Len)
{
	INT16U	len_frame=0;														//֡����/�����ڼ��������򳤶�
	
	if(!PassworkCheckAndReport(InBuff));										//�ж���վ�·������������Ƿ���ȷ������ȷʱ�ϱ����������Ϣ��
	else																		//����ͬ��������ȷ
	{
		len_frame = ((INT16U)InBuff[8]<<8) + InBuff[9] -4;						//������Ч���������������������ǿ��ת����
		memset(FUN_Config,0,24);												//��յ�ǰ�������ò���
		memcpy(FUN_Config,InBuff+14,len_frame);									//��Ч�������ã�д��FUN_Config���飨Ĭ�ϳɹ���
		BSP_WriteDataToFm(FUN_Config_Addr,FUN_Config,FUN_Config_Len);			//��������
//		LteCommunication(InBuff,Len,0,0);										//���óɹ�������ԭ����أ������գ�LteCommunication����0��PS��Э����δҪ�󷵻�
		return 1;
	}
	BspUartWrite(2,SIZE_OF("Fun_Config_Comm��������ͨ��ʧ�ܣ�\r\n"));
	return 0;
}

/*******************************************************************************
���ƣ�INT8U SMS_Send_Comm(u8 *InBuff, u16 Len)
���ܣ�����������顢�ظ�ͨ�š���ָ�����뷢�Ͷ���
��Σ�u8 *InBuff����������ݣ�����Ϊ���յ���֡��u16 Len������
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U SMS_Send_Comm(u8 *InBuff, u16 Len)
{
	INT8U 	ST=0,i;
	INT8U	phone_num[12]={0};
	
	if(!PassworkCheckAndReport(InBuff));										//�ж���վ�·������������Ƿ���ȷ������ȷʱ�ϱ����������Ϣ��
	else																		//����ͬ��������ȷ
	{

		LteCommunication(InBuff,Len,0,0);										//���óɹ�������ԭ����أ������գ�LteCommunication����0��
		ME909S_Trans_OFF();
		for(i=0;i<6;i++)														//���Ž��պ��룺ΪF��ͨ�ſ��ţ�ÿ������ռ����ֽڡ����翨��Ϊ13912345678����������Ϊ��F1H��39H��12H��34H��56H��78H��
		{
			INT8UBCDToAscii(InBuff[14+i],phone_num+2*i);						//BCDת��ΪASCII
		}
		ST=ME909S_SMS_Send(Device_Number,6,phone_num+1);						//��ָ�����뷢�ͱ�����װ�ñ�ţ����ز������ phone_num[0]ΪF�����ù�
		ME909S_Trans_ON();
	}	
	if(ST)return 1;	
	BspUartWrite(2,SIZE_OF("SMS_Send_Comm���Ͷ���ͨ��ʧ�ܣ�\r\n"));	
	return 0;
}


/*******************************************************************************
���ƣ�INT8U SMS_Judge(u8 *InBuff, u16 Len)
���ܣ��ж�InBuff�����Ƿ����Э��Ҫ����������ȷ
��Σ�u8 *InBuff����������ݣ����Ž��յ���֡��u16 Len������
���Σ���
���أ����϶��Ż���Ҫ�󷵻�1�����򷵻�0
*******************************************************************************/
INT8U SMS_Judge(u8 *InBuff, u16 Len)
{
	if(Judge_NW_Framing(SMS_AWAKE,InBuff,Len,0)) 								//�ж�Э�飬������
	{
		if(0==memcmp(InBuff+10,Config.Password,4)) return 1;					//���ж�ԭ������ԭ�������벻ͬ����ͬʱmemcmp����0��																	
	}
	BspUartWrite(2,SIZE_OF("SMS_Judgeδͨ����\r\n"));
	return 0;
}

/*******************************************************************************
���ƣ�INT8U Data_Request_Comm(u8 *InBuff, u16 Len)
���ܣ�װ���յ���վ�������������ԭ����أ�������������Ӧ�����ָ�ʽ����������������վ��
���������Ϊ0�ֽڣ��ϴ�δ�ɹ��ϴ�����ʷ���ݣ�������ʷ��Ƭ����װ������ʷ�������ϴ���
���������Ϊ2�ֽ�BBBBH��װ�����̲ɼ��������ݣ�ͼƬ���⣩����ɲɼ��������ϴ����ôβ�����
Ӱ��ԭ�趨�ɼ������ִ�С�
��Σ�u8 *InBuff����������ݣ�����Ϊ���յ���֡��u16 Len������
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U Data_Request_Comm(u8 *InBuff, u16 Len)
{
	INT16U	len_frame=0;														//֡����/�����ڼ��������򳤶�
	
	LteCommunication(InBuff,Len,0,0);											//����ԭ����أ������գ�LteCommunication����0��
	len_frame = ((INT16U)InBuff[8]<<8) + InBuff[9];								//���������򳤶ȣ��������������ǿ��ת����
	if(!len_frame)																//���������Ϊ0�ֽ�
	{
		/*�ϴ�δ�ɹ��ϴ�����ʷ���ݣ�������ʷ��Ƭ����װ������ʷ�������ϴ���*/
		return Tem_Cur_Upload(RETRY,TIMEOUT);									//������ʷ���ݣ�ʧ��ʱ����0������ʾ������ʷ����δ������ɣ���Ӱ������ɵĲ��֣�
	}
	else if((InBuff[10]==0xBB) && (InBuff[11]==0xBB))							//���������Ϊ2�ֽ�BBBBH
	{
		return Tem_Cur_Sample_Upload(RETRY,TIMEOUT);							//װ�����̲ɼ��������ݣ�ͼƬ���⣩����ɲɼ��������ϴ�
	}
	BspUartWrite(2,SIZE_OF("Data_Request_Comm����δ֪�����\r\n"));
	return 0;																	//�����������Ӧ�ó���
}

/*******************************************************************************
���ƣ�INT8U Flow_Data_Upload(u8 times, u16 timeout)
���ܣ��ն������ϴ�װ����������ʹ��������������֣�40H��
�Ȳ�����֡�ͷְ�ʲô�ģ�Ҫ���洢����Ҫ����̫�鷳����ֻ���װ�
��Σ�u8 times�����Դ�����u16 timeout����ʱ�ȴ�ʱ�䣬��λ��ʱ��Ƭ
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U Flow_Data_Upload(u8 times, u16 timeout)
{
	INT8U 	i=0;
	INT8U 	*R;
	INT32U	Temp=0;																//���ڵ�λת���ͷ��س���
	INT16U	f_len=0;															//��֡���ȣ�����Temp������鷳��

/*Flow_Data�ṹ�����*/
	NW_GetTime(&Flow_Data.Sample_Time);											//����Flow_Data�ṹ���еĲ���ʱ��
	Temp = Local_FLow_Data.Flow_Day_Used_B >>20;								//���㵱����ʹ����������λ��Byte -> M Byte
	Temp = htonl(Temp);															//��С��ת��
	memcpy(Flow_Data.Day_Used,&Temp,4);											//��λ��M Byte
	Temp = Local_FLow_Data.Flow_Month_Used_B>>20;								//���㵱����ʹ����������λ��Byte -> M Byte
	Temp = htonl(Temp);															//��С��ת��
	memcpy(Flow_Data.Month_Used,&Temp,4);										//��λ��M Byte
	Temp = MONTHLY_FLOW>(Local_FLow_Data.Flow_Month_Used_B>>20) ? 				//����ʣ����������λ��M Byte
		MONTHLY_FLOW-(Local_FLow_Data.Flow_Month_Used_B>>20) : 0;
	Temp = htonl(Temp);															//��С��ת��
	memcpy(Flow_Data.Month_Surplus,&Temp,4);									//��λ��M Byte

/*��֡�ϱ�*/
	f_len = NW_Framing(FLOW_DATA_UPLOAD,LTE_Tx_Buff);							//��֡
	for(;i<times;i++)
	{
		Temp = LteCommunication(LTE_Tx_Buff,f_len,LTE_Rx_Buff,timeout);			//�ϱ����ȴ��ظ�
		if(!Temp) continue;														//�ն���û���յ���վ��Ӧ�������3��							

		R = Judge_NW_Framing(FLOW_DATA_UPLOAD,LTE_Rx_Buff,Temp,0);				//�жϽ��������Ƿ��������Э��
		if(R)
		{
			if(R[10]!=1) continue;												//�ж�֡��ʶ��Ŀǰ����ֻ���װ��Ҳ���֡���̶�Ϊ1
			if(R[11]!=0xAA) continue;											//�жϻظ������� AA55H
			if(R[12]!=0x55) continue;											//�жϻظ�������
			return 1;															//�ϱ��ɹ�
		}
	}
	BspUartWrite(2,SIZE_OF("Flow_Data_Upload�����ϱ�ͨ��ʧ�ܣ�\r\n"));
	return 0;																	//�ϱ�ʧ�ܣ������ݱ������´δ��͡�//��ֻ���װ�����˿��Բ��ñ�����ÿ�ζ������
}

/*******************************************************************************
���ƣ�INT8U Sleep_Notice(void)
���ܣ�����Ϣ��װ���豸ÿ������֮ǰ���͡���վ�޻ظ����������֣�0CH��
��Σ���
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U Sleep_Notice(void)
{
	INT16U	len=0;																//������֡
	len = NW_Framing(SLEEP_NOTICE,LTE_Tx_Buff);									//��֡
	LteCommunication(LTE_Tx_Buff,len,0,0);										//�ϱ����޻ظ������գ�LteCommunication����0��
	return 1;	
}

/*******************************************************************************
���ƣ�INT8U Tem_Cur_Upload(u8 times, u16 timeout)
���ܣ�װ�������ϴ������¶ȡ�������ʷ���ݣ������֣�26H��������Unreport_Index[31][3]
�������ж��Ƿ�δ�ϱ��ɹ���
��Σ�u8 times�����Դ�����u16 timeout����ʱ�ȴ�ʱ�䣬��λ��ʱ��Ƭ
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0�������ϱ�����0xFF
*******************************************************************************/
INT8U Tem_Cur_Upload(u8 times, u16 timeout)
{
	INT8U 	i=0,j=0,k=0;
	INT16U	Bit_Index = 0;														//δ�ϱ�����ʷ�ļ���ţ�31�����洢744Сʱ����0~743
	INT8U	nodata = 0xFF;
	
	/*����Unreport_Index[31][3]��������Bit_Index*/
	for(i=0;i<31;i++){															//��
		for(j=0;j<3;j++){														//�ֽ�
			for(k=0;k<8;k++){													//ÿ�ֽ�8Сʱ
				if(0==(Unreport_Index[i][j]&(0x80>>k)))							//1���ϱ���0δ�ϱ������bit����0ʱ��
				{
				/*������δ�ϱ���ִ����ز���*/
					nodata = 1;													//���ϱ�����0xFF
					WDTClear(LTE_PRIO);											//��������Ź����������ѭ���ܾã�
					Bit_Index = 3*8*i+8*j+k;									//����Unreport_Index[1][2]�ĵ�5��k��λΪ0����ʾSUB2 ��21Сʱ δ�ϱ�������21+24=45Сʱ�����������������ڵ�0Сʱ��
					if(NW_History_Temp_Comm(Bit_Index,RETRY,TIMEOUT))			//����Bit_Index��֡��ͨ�ţ�����n֡�ϱ��͵ȴ��ظ������ɹ�
					{
						Unreport_Index[i][j] |= (0x80>>k);						//�ϱ��ɹ�������Unreport_Index[][]������
						if(!BSP_WriteDataToFm(Unreport_Index_Addr,(INT8U*)Unreport_Index,Unreport_Index_Len)) 
						{
							BspUartWrite(2,SIZE_OF("δ�ϱ�����������д������ʧ�ܣ�\r\n"));
							return 0;											//δ�ϱ�����������д������
						}
					}
				}
			}
		}
	}
	
	if(nodata==1) BspUartWrite(2,SIZE_OF("������ʷ�����ѳ����ϱ�\r\n"));
	else BspUartWrite(2,SIZE_OF("û����ʷ������Ҫ�ϱ�\r\n"));
	return nodata;																//ȫ����ʷ�����ϱ����ʱ����1�������ϱ�ʱ�᷵��0xFF
}

/*******************************************************************************
���ƣ�INT8U Tem_Cur_Sample_Upload(u8 times, u16 timeout)
���ܣ�װ�����̲ɼ��������ݣ�ͼƬ���⣩����ɲɼ��������ϴ����ôβ�����Ӱ��ԭ�趨�ɼ����
��ִ�С��������֣�26H��
��Σ�u8 times�����Դ�����u16 timeout����ʱ�ȴ�ʱ�䣬��λ��ʱ��Ƭ
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U Tem_Cur_Sample_Upload(u8 times, u16 timeout)
{
	INT8U			i=0,j=0;
	static INT8U	msg_data;
	INT16U			addr;
	INT16U			len=0,len_frame=0;
	INT8U			*R;
	
	msg_data = DATA_CMD;														//���ڷ�����֪ͨRF�����л����ɼ�����״̬
	OSMboxPost(Data_CMDB0X, &msg_data);											//ͨ�����佫�ɼ������
	OSTimeDly(5*20);															//�ȴ�5�룬ȷ��RF�����ڴ�������д������
	
	//�������ж�ȡ���ݲ���֡�ϱ�
	BSP_InitFm(LTE_Num);														//��ʼ��
	for(i=0;i<55;i++)
	{			
		if(TT_Info.HaveTT[i]==0x55)												//��ʾ����������
		{
		/*���ṹ�岢��֡*/
			Tem_Cur_Data.Frame_ID = i;											//֡��ʶ
			Tem_Cur_Data.Pack_Num = 1;											//����
			Tem_Cur_Data.Unit_ID = Unit_ID_Code[i];								//���ܵ�Ԫʶ����
			SecondToNwTime(TT_Sample_Manage.Time[TT_Sample_Manage.Sample_Num-1],&Tem_Cur_Data.Sample_Time);			//����ʱ�䣨��+��+��+ʱ+��+�룩��6�ֽڣ�
			addr = Sample_Data_Addr + One_TT_Sample_Data_Len*i + 2*(TT_Sample_Manage.Sample_Num-1);					//�����¶����ݵ�ַ
			BSP_ReadDataFromFm(addr, Tem_Cur_Data.Tem, 2);						//�������ж�ȡ��Ӧ����¶ȣ�2�ֽڣ�
			Tem_DS18B20_To_NW(Tem_Cur_Data.Tem,Tem_Cur_Data.Tem);				//����������ת���¶�
			//Tem_Cur_Data.Cur													//���ߵ�����2�ֽڣ�����
			Tem_Cur_Data.Voltage = 33;											//������������ѹ��1�ֽڣ�3.3V
			len_frame = NW_Framing(TEM_CUR_UPLOAD, LTE_Tx_Buff);				//��ǰ�ɼ�������֡
			
		/*�ϱ����Ȼظ�������times��*/
			for(j=0;j<times;j++)												//�������times��
			{
				len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,timeout);		//����Ϊ0��ʾͨ��ʧ�ܣ�����0xff��ʾû���յ����ݣ���������Ϊ�������ݳ���
				if(!len) 
				{
					BspUartWrite(2,SIZE_OF("δ�յ���վ�ظ���\r\n"));
					continue;													//ͨ��ʧ��ʱ����
				}

				R = Judge_NW_Framing(TEM_CUR_UPLOAD,LTE_Rx_Buff,len,0);			//�жϽ��������Ƿ��������Э��
				if(R) 															//������Э��ʱ����ͨ��
				{
					if(R[10]!=i) continue;										//�ж�֡��ʶi
					if(R[11]!=0xAA) continue;									//�жϻظ������� AA55H
					if(R[12]!=0x55) continue;									//�жϻظ�������
					break;														//�ϱ��ɹ�	
				}
			}
			if(j==times) 
			{
				FM_LowPower(LTE_Num);											//�������ŵ͹�������
				BspUartWrite(2,SIZE_OF("Tem_Cur_Sample_Upload���Գ��������ɼ��ϱ�ͨ��ʧ�ܣ�\r\n"));
				return 0;														//���Գ�������ͨ��ʧ�ܣ�����0
			}
		}				
	}
	FM_LowPower(LTE_Num);														//�������ŵ͹�������	
	return 1;																	//ȫ��̽ͷ�ϱ����ʱ����1
}

INT16U NW_History_Temp_Framing(u8 Frame_ID, u8 *InBuff, struct SAMP_MANAGE *Info_str, u8 *OutBuff);
/*******************************************************************************
���ƣ�INT16U NW_History_Temp_Comm(u16 Bit_Index, u8 times, u16 timeout)
���ܣ���Bit_Index��ָʾ���ļ���ȡ����֡���ϱ����ظ�����һ��������
��Σ�u16 Bit_Index������ָʾ��ȡ�ĸ���ʷ�����ļ�����֡��0~23��ʾSUB1�е�0~23���ļ���24~47
��ʾSUB2�е�0~23���ļ�������720~743��ʾSUB31�е�0~23���ļ���
u8 times�����Դ�����u16 timeout����ʱ�ȴ�ʱ�䣬��λ��ʱ��Ƭ
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT16U NW_History_Temp_Comm(u16 Bit_Index, u8 times, u16 timeout)
{
	INT8U			day = 0, hour = 0;
	INT8U			i = 0,j = 0;
	TCHAR			path[50];
	UINT			bw;
	INT16U			offset_len,len_frame,len;		
	INT8U			*R;
	struct SAMP_MANAGE	Info_str;												//�ݴ���ļ������Ľṹ����Ϣ
	
	/*����Bit_Index�򿪶�Ӧ�ļ�*/
	day = Bit_Index/24 +1;														//���ڣ���ӦSUBn
	hour = Bit_Index%24;														//Сʱ����Ӧ0~23
	sprintf(path, "/SUB%d/%02d%02d", day, day, hour);							//�������������ļ�·����SUB��/��ʱ��	ע�⣡�ļ���������Ķ���
	
	BspUartWrite(2,SIZE_OF("\r\n��ʼ����"));
	BspUartWrite(2,(INT8U*)path,strlen(path));									//���ϲ��ַ���ԭ��path���ļ���������Ҫ�õ�
	BspUartWrite(2,SIZE_OF("\r\n"));

	OSTimeDly(25);																//��ʱ���ã��˺���������ѭ���е��ö�Σ�����Ҫ���Ͷ��Сʱ����ʱ���ٴε��ã�����ʱ��ʱ����RF��������ѯ���1s���ɱ�֤����RF����ʹ���ļ�ϵͳ
	while(FATFS_Lock)	OSTimeDly(20);
	FATFS_Lock=1;
	
	f_mount(&fs, "", 1);														/* Re-mount the default drive to reinitialize the filesystem */
	if (FR_OK==f_open(&fil, path, FA_OPEN_EXISTING | FA_READ))					//���ļ�	&fil��ȫ�ֱ�����ע�ⲻҪͬʱ��
	{
	/*��ȡSAMP_MANAGE�ṹ����Ϣ*/
		f_read(&fil,&work,FF_MAX_SS,&bw);										//��ȡ�������ռ䣨������ʽ����work���飬�����ط�û�õ���
		offset_len = Search_Info((u8 *)&work,FF_MAX_SS);						//��&work�����ļ���Ϣ�ṹ�壬������Ч�ļ�ͷ��offset���ȣ���δ�ҵ�����0xFFFF
		if(offset_len==0xFFFF) 	
		{
			FATFS_Lock=0;
			f_close(&fil);
			BspUartWrite(2,SIZE_OF("δ�ҵ���Чsample_manage��Ϣ�ṹ�壡\r\n"));
			return 0;															//δ�ҵ���Ч�ṹ����Ϣ
		}			
	/*���ݽṹ����Ϣִ��ͨ��*/
		memcpy(&Info_str,(INT8U*)&work+2+offset_len,Sample_Manage_Len);			//��ַ+offset_lenָ���ļ�ͷ0xFF 0xAA����+2��������ǿ��ת����������+2���ṹ�峤�ȣ�ָ��sample_manage�ṹ��
		offset_len += Sample_Manage_Len+2;										//���ƽṹ�峤��+FFAA����
		f_lseek(&fil, offset_len);												//�����ļ�ָ�룬ָ���׸�̽ͷ����
		for(;i<Info_str.TT_Count;i++)											//�ļ��ж�������̽ͷ����������Ҫ���ļ���ȡ���ݵ�����
		{
			/*��ȡ�ļ�����֡*/
			if(f_read(&fil,&work,Info_str.Len,&bw)) 
			{
				FATFS_Lock=0;
				f_close(&fil);
				BspUartWrite(2,SIZE_OF("SPIFLASH��ȡ�ļ�ʧ�ܣ�\r\n"));
				return 0;														//�����ļ���Ϣ�ṹ���ȡһ�����ݣ��ظ���ȡ���ɣ�ָ���Զ�����������ȡʧ�ܷ���0			
			}		
			len_frame = NW_History_Temp_Framing(i,(u8 *)&work, &Info_str, LTE_Tx_Buff);		//��i��������֡��LTE_Tx_Buff��iΪ֡��ʶ
			
			/*�ϱ����Ȼظ�������times��*/
			for(j=0;j<times;j++)												//�������times��
			{
				len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,timeout);			//����Ϊ0��ʾͨ��ʧ�ܣ�����0xff��ʾû���յ����ݣ���������Ϊ�������ݳ���
				if(!len) 
				{
					BspUartWrite(2,SIZE_OF("δ�յ���վ�ظ���\r\n"));
					continue;													//����
				}

				R = Judge_NW_Framing(TEM_CUR_UPLOAD,LTE_Rx_Buff,len,0);			//�жϽ��������Ƿ��������Э��
				if(R) 															//������Э��ʱ����ͨ��
				{
					j = 0;														//���¿�ʼ�����Դ���
					if(R[10]!=i) continue;										//�ж�֡��ʶi
					if(R[11]!=0xAA) continue;									//�жϻظ������� AA55H
					if(R[12]!=0x55) continue;									//�жϻظ�������
					break;														//�ϱ��ɹ�	
				}
			}
			if(j==times) 
			{
				f_close(&fil);													//�ر��ļ���dismount��Ϊ��the work area can be discarded�����ǲ��ͷ��ڴ棬û��Ҳ��
				FATFS_Lock=0;
				BspUartWrite(2,SIZE_OF("NW_History_Temp_Comm���Գ��������¶��ϱ�ͨ��ʧ�ܣ�\r\n"));
				return 0;														//���Գ�������ͨ��ʧ�ܣ�����0					
			}
		}
		
	/*ͨ�����*/
		f_close(&fil);															//�ر��ļ���dismount��Ϊ��the work area can be discarded�����ǲ��ͷ��ڴ棬û��Ҳ��
		FATFS_Lock=0;
		return 1;																//ȫ��̽ͷ�ϱ���ɣ�����1
	}
	FATFS_Lock=0;
	BspUartWrite(2,SIZE_OF("���ļ�ʧ�ܣ�\r\n"));
	return 0;																	//���ļ�ʧ��
}

/*******************************************************************************
���ƣ�INT16U Search_Info(u8 *InBuff, u16 Len)
���ܣ��ڻ����в�����ЧSAMP_MANAGE�ṹ����Ϣ��
��Σ�u8 *InBuff�������һ��棻u16 Len�����ҷ�Χ
���Σ���
���أ���Ч�ļ�ͷ��offset���ȣ���0xFFFF����ʾδ�ҵ�
*******************************************************************************/
INT16U Search_Info(u8 *InBuff, u16 Len)
{
	INT16U	i,crc;
	
	for(i=0;i<Len;i++)
	{
		if(InBuff[i]==0xFF)														//�ļ���ͷ��0xFF 0xAA
		{
			if(InBuff[i+1]==0xAA)												//�ҵ��ļ�ͷ��
			{
				crc = RTU_CRC(InBuff+i+2,Sample_Manage_Len-2-2);				//����CRC�����ڶ�дУ��
				if(*(InBuff+i+2+Sample_Manage_Len-3)!=((crc>>8)&0xff)) continue;//CRC���ֽڣ�����Ǹߵ�ַ��==>�����ǰ�С��ģʽ�����ڴ�洢U16��ͬ
				if(*(InBuff+i+2+Sample_Manage_Len-4)!=(crc&0xff)) continue;		//CRC���ֽڣ�����ǵ͵�ַ��==>�����ǰ�С��ģʽ�����ڴ�洢U16��ͬ
				else return i;													//��Ч������offset���ȣ�0xFF���ļ�ͷ��ƫ������
			}
		}
	}
	return 0xFFFF;																//û�ҵ���Ч�ṹ����Ϣ
}

/*******************************************************************************
���ƣ�INT16U NW_History_Temp_Framing(u8 Frame_ID, u8 *InBuff, struct SAMP_MANAGE *Info_str, u8 *OutBuff)
���ܣ�������Э�����¶ȵ�������֡����ϸ˵����NW_Framing()������
----------------------------------------------------------------
	��ʼ��	װ�ú���	������	�����򳤶�	������	У����	������	|
	1�ֽ�	6�ֽ�	1�ֽ�	2�ֽ�		�䳤	1�ֽ�	1�ֽ�	|
----------------------------------------------------------------
��Σ�u8 Frame_ID,֡ID��u8 *InBuff,���ļ��ж�ȡ�����ݣ�struct SAMP_MANAGE *Info_str, �ṹ��ָ�롣
���Σ�INT8U *OutBuff����֡���Ĵ�ŵ�ַ�������С���Ҫ443�ֽڡ�
���أ��ܵı��ĳ���
*******************************************************************************/
INT16U NW_History_Temp_Framing(u8 Frame_ID, u8 *InBuff, struct SAMP_MANAGE *Info_str, u8 *OutBuff)
{
	INT8U	i;
	INT16U	delta_T,len;														//ʱ�����ݳ���
	
	len = 7+11+7*(Info_str->Sample_Num-1);										//�����򳤶�
	
/*��ʼ�롢װ�ú��롢�����֡������򳤶�*/
	OutBuff[0]=Start_Code;														//1Byte��ʼ�룬�̶�68H
	OutBuff[1]=Device_Number[0];
	OutBuff[2]=Device_Number[1];												//ǰ���ֽڱ�ʾ���Ҵ��루���Ϸ�������˾ͳһ���䣩�����ô�д��ĸ(ASCII)
	OutBuff[3]=Device_Number[2];												//�����ֽڱ�ʾ���Ҷ�ÿ��״̬���װ�õ�ʶ���루��վ��ַ�������ô�д��ĸ�����֣�����ʹ������
	OutBuff[4]=Device_Number[3];														
	OutBuff[5]=Device_Number[4];														
	OutBuff[6]=Device_Number[5];												//6Byteװ�ú���
	OutBuff[7]=TEM_CUR_UPLOAD;													//1Byte�����֣����������������� �̶�TEM_CUR_UPLOAD
	OutBuff[8]=(len>>8)&0xff;													//
	OutBuff[9]=len&0xff;														//2Byte�����򳤶ȣ����ֽ���ǰ����Ϊ���ʾ��������
	
/*������*/	
	memcpy(OutBuff+10,&Config.SecurityCode,4);									//4Byte������֤
	OutBuff[14]=Frame_ID;														//1Byte֡��ʶ
	OutBuff[15]=Info_str->Sample_Num;											//1Byte����
	OutBuff[16]=InBuff[0];														//1Byte���ܵ�Ԫʶ����
												
	/*�װ�����0����11Byte*/
	SecondToNwTime(Info_str->Time[0],(struct NW_TIME*)(OutBuff+17));			//6Byte����ʱ�䣨��+��+��+ʱ+��+�룩��6�ֽڣ�HEX��ʾ��
	Tem_DS18B20_To_NW(OutBuff+23,InBuff+1);										//2Byte����¶ȣ�����ֵ=��ʵ���¶�+50��*10
	memset(OutBuff+25,0,2);														//2Byte���ߵ���������ֵ=ʵ�ʵ���*10���ȹ̶�д0
	OutBuff[27]=33;																//1Byte������������ѹ������ֵ=ʵ�ʵ�ѹ*10���ȹ̶�д��33

	/*��һ�����Ժ����7Byte*/
	for(i=1;i<Info_str->Sample_Num;i++)											//i��ֵΪ1����Ϊ�װ��������ﴦ��
	{
		delta_T=Info_str->Time[i]-Info_str->Time[i-1];							//�������ϰ�����ʱ���
		OutBuff[28+7*(i-1)]=(delta_T>>8)&0xFF;									//2Byte����ʱ�����ֽ�
		OutBuff[29+7*(i-1)]=delta_T&0xFF;										//���ֽ�
		Tem_DS18B20_To_NW(OutBuff+30+7*(i-1),InBuff+2*i+1);						//2Byte����¶�
		memset(OutBuff+32+7*(i-1),0,2);											//2Byte���ߵ���
		OutBuff[34+7*(i-1)]=33;													//1Byte������������ѹ		
	}
	
/*У���롢������*/
	OutBuff[35+7*(i-2)]=Negation_CS(OutBuff+1,34+7*(i-2));						//1Byte������  ����CSǰ����ʼ��������ֽ�
	OutBuff[36+7*(i-2)]=Epilog_Code;											//1Byte�����룬�̶�16H
	return 37+7*(i-2);		 													//�ܵı��ĳ���
}

/*******************************************************************************
���ƣ�void Tem_DS18B20_To_NW(INT8U* Outbuff,INT8U* InBuff)
���ܣ���18B20��õ����ֽ��¶�ת����������Լ����ĸ�ʽ������ֵ=ʵ���¶�*10+500��
18B20��õ��¶�ֵΪʵ���¶�*10
��Σ�InBuff �������¶�����ָ��
���Σ�Outbuff������¶�����ָ��
���أ���
*******************************************************************************/
void Tem_DS18B20_To_NW(INT8U* Outbuff,INT8U* InBuff)									
{
	INT16U Temp=0;
	
	Temp=(InBuff[0]<<8)+InBuff[1];
	
	if(Temp>=0xf800) 
	{
		if(Temp>0xF9F4)	Temp=0xF9F4;											//�¶�С��-50��ʱ����Ϊ-50�� ��18B20����ΪF9F4��
		Temp=500-Temp&0x07ff;													//����
	}
	else Temp=500+Temp;															//����
	
	Outbuff[0]=(Temp>>8)&0xff;
	Outbuff[1]=Temp&0xff;
}

/*******************************************************************************
���ƣ�INT8U File_List_Query_Comm(u8 *InBuff, u16 Len)
���ܣ���վ��ѯĳ��ʱ�䷶Χ��װ�ô洢���ļ��б�װ���յ�������󣬷��ط��ϲ�ѯ�������ļ�
�б�01H��JPEG�ļ�����Ӧͼ���ļ���02H�����϶�λ�����ļ���....FFH�������ļ����͡�
��Σ�u8 *InBuff����������ݣ�����Ϊ���յ���֡��u16 Len������
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U File_List_Query_Comm(u8 *InBuff, u16 Len)
{
	INT16U	len_frame=0;														//֡����/�����ڼ��������򳤶�
	
	//time_start = time_change(InBuff+11);										//��ʼʱ�����ʱ�����		===============δ���
	//time_end = time_change(InBuff+17);
	switch(InBuff[10])															//�ļ����ͽ���
	{
		case JPEG_FILE:		/*���޴˹���*/
		case FLW_FILE:		/*���޴˹���*/										//����case����һ��
				len_frame = NW_Framing(FLW_FILE,LTE_Tx_Buff);					//��֡�������ϲ�ѯ�������ļ���ʽΪ0������0000H
				LteCommunication(LTE_Tx_Buff,len_frame,0,0);					//�ϱ����޻ظ������գ�LteCommunication����0��
				return 1;
		
		case ANY_FILE:		/*�¶���ϸ���ݿ��Է�����*/
//				len_frame = NW_FileList_framing(FILE_LIST_QUERY,0,time_start,time_end,LTE_Tx_Buff);		//�ļ�File_Listר����֡��������������		===============δ���
				LteCommunication(LTE_Tx_Buff,len_frame,0,0);					//�ϱ����޻ظ������գ�LteCommunication����0��
				return 1;
		
		default:
				break;
	}
	//��ӡ������Ϣ
	return 0;
}

/*******************************************************************************
���ƣ�INT8U Files_Upload(u8 *FileName,u8 Des_Len)
���ܣ�	STEP 1 װ�����������ļ��������֣�73H��
		STEP 2 �ļ����ͣ������֣�74H��
		STEP 3 �ļ����ͽ�����ǣ������֣�75H�������ȴ��ļ����������·��������֣�76H��
		STEP 4 �����������ݲ��жϣ�ѡ��ص�STEP 2 ���в��������������
��Σ�u8 *FilenName���ļ���ָ�룬�����ļ��ϴ�
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U Files_Upload(u8 *FileName,u16 Des_Len)
{
	INT8U	i=0;
	INT8U 	STEP=FILE_UL_REQUEST;												//������ʼ����Ϊ�ϱ�����
	INT16U	len_frame=0;														//֡����
	INT16U	len=0;																//���յ��ĳ���
	INT16U	packages=0;															//�ļ��ܰ���
	INT16U 	Pac_Num=0;															//��ǰҪ��֡�İ���
	INT16U	file_len=0;															//�ļ��ܳ���
	INT8U	*R,*P_pac_num=0;													//�׵�ַ/���������׵�ַ
	
	//file_len = 																//��ȡ�ļ�	===============δ���
	packages = file_len / Des_Len;												//�����ļ��ܰ���
	
	while(1)
	{
		switch(STEP)
		{
			case FILE_UL_REQUEST:	/*STEP 1 װ�����������ļ��������֣�73H��*/
					len_frame = NW_File_Framing(FILE_UL_REQUEST,FileName,0,0,LTE_Tx_Buff);			//�ļ�ר����֡		===============δ���
					for(i=0;i<5;i++)																//�������5�Σ�Э��Ҫ��
					{
						len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,3*20);				//װ�����������ļ������ȴ����գ�ÿ�μ��3�루Э��Ҫ��
						if(!len) continue;															//ͨ��ʧ��ʱ����
						Judge_NW_Framing(FILE_UL_REQUEST,LTE_Rx_Buff,len,0);						//�жϽ��������Ƿ��������Э��
						////��һ����֤�Ƿ�ԭ����أ������ˣ�
						STEP = FILE_UPLOAD;															//��һ�����ļ��ϴ�
						break;																		//����forѭ��																
					}
					//��ӡ������Ϣ
					return 0;																		//���Գ�ʱ������ʧ��
			
			case FILE_UPLOAD:		/*STEP 2 �ļ����ͣ������֣�74H��*/
					for(i=0;i<packages;i++)
					{
						if(P_pac_num==0) Pac_Num=i;													//����Ҫ�����İ��ţ��ǲ���״����
						else Pac_Num = (P_pac_num[2*i]>>8)+P_pac_num[2*i+1];						//����Ҫ�����İ��ţ�����״����
						len_frame = NW_File_Framing(FILE_UPLOAD,FileName,Pac_Num,Des_Len,LTE_Tx_Buff);		//�ļ�ר����֡��ÿ���ļ����ݳ���Des_Len		===============δ���
						LteCommunication(LTE_Tx_Buff,len_frame,0,0);										//ֻ������			
					}
					STEP = FILE_UL_END;																//��һ�����ļ��ϴ��������	
					break;
			
			case FILE_UL_END:		/*STEP 3 �ļ����ͽ�����ǣ������֣�75H�������ȴ��ļ����������·��������֣�76H��*/
					OSTimeDly(2*20);																//װ�������ļ�����ȫ��������2�룬���͸�ָ��
					len_frame = NW_File_Framing(FILE_UL_END,FileName,0,0,LTE_Tx_Buff);				//�ļ�ר����֡		===============δ���
					for(i=0;i<5;i++)																//�������5�Σ�Э��Ҫ��
					{
						len = LteCommunication(LTE_Tx_Buff,len_frame,LTE_Rx_Buff,30*20);			//װ�����������ļ������ȴ����գ�ÿ�μ��30�루Э��Ҫ�󣩣���ʱ�ܿ��ܻ᲻׼����������
						if(!len) continue;															//ͨ��ʧ��ʱ����
						R = Judge_NW_Framing(FILE_FILLING,LTE_Rx_Buff,len,0);						//�жϽ���ָ���Ƿ�Ϊ76H
						if(!R)	continue;															//���ղ���ȷʱ����	
						STEP = 4;																	//��һ���������������ݲ��ж�
						break;																		//����forѭ��																
					}
					//��ӡ������Ϣ
					return 0;																		//���Գ�ʱ����վδ�ظ����ͽ������	
			
			case 4:					/*STEP 4 �����������ݲ��ж�*/
					if(R[110]==0) return 1;															//���貹��ʱ�����ͳɹ�������1
					packages = R[110];																//���㲹������1�ֽڣ�
					P_pac_num = R+111;																//���������׵�ַ
					STEP = FILE_UPLOAD;																//��һ�����ص�STEP 2 �ļ����ͣ������֣�74H��
					break;
			
			default:
					break;
		}
		return 0;
	}
}

/*******************************************************************************
���ƣ�INT16U NW_File_Framing(u8 Cmd,u8 *FileName,u8 Pac_Num,u8 Des_Len,u8 *OutBuff)
���ܣ��ļ���֡ר�ã�������Э����֡��֡�ṹ���������и�ʽ˵��:���ݰ���������֡ģʽ������
��֡������ʼ�롢װ�ú��롢���������롢�����򳤶ȡ�������У����ͽ����롣����֡���Ȳ���
��4000�ֽڡ����ô��ģʽ�����ݵĸ��ֽڱ������ڴ�ĵ͵�ַ�У���У��������ۼӺ�ȡ����У��
��ʽ�����ͷ���װ�ú��롢�����֡������򳤶Ⱥ��������������ֽڽ��������ۼӣ�������λ��ֻ��
������ֽڣ������ֽ�ȡ����
----------------------------------------------------------------
��ʼ��	װ�ú���	������	�����򳤶�	������	У����	������	|
1�ֽ�	6�ֽ�		1�ֽ�	2�ֽ�		�䳤	1�ֽ�	1�ֽ�	|
----------------------------------------------------------------
��Σ�INT8U Cmd,�����֣����������������ͣ�u8 *FileName,�ļ�����u8 Pac_Num,��֡���ţ�
u8 Des_Len,֡���ݳ���
���Σ�INT8U *OutBuff����֡���Ĵ�ŵ�ַ
���أ��ܵı��ĳ���
*******************************************************************************/
INT16U NW_File_Framing(u8 Cmd,u8 *FileName,u8 Pac_Num,u8 Des_Len,u8 *OutBuff)
{
	INT16U len;																	//���ݳ���

	/*��ȡ���ݣ��������ݳ���*/
//	len=Get_DataField(Cmd,&OutBuff[10]);										//��ȡ���ݣ��������ݳ���
	////�����ȡ�ļ�						 		==============================δ���
	
	OutBuff[0]=Start_Code;														//1Byte��ʼ�룬�̶�68H
	OutBuff[1]=Device_Number[0];
	OutBuff[2]=Device_Number[1];												//ǰ���ֽڱ�ʾ���Ҵ��루���Ϸ�������˾ͳһ���䣩�����ô�д��ĸ(ASCII)
	OutBuff[3]=Device_Number[2];												//�����ֽڱ�ʾ���Ҷ�ÿ��״̬���װ�õ�ʶ���루��վ��ַ�������ô�д��ĸ�����֣�����ʹ������
	OutBuff[4]=Device_Number[3];														
	OutBuff[5]=Device_Number[4];														
	OutBuff[6]=Device_Number[5];												//6Byteװ�ú���
	OutBuff[7]=Cmd;																//1Byte�����֣�����������������
	OutBuff[8]=(len>>8)&0xff;													//
	OutBuff[9]=len&0xff;														//2Byte�����򳤶ȣ����ֽ���ǰ����Ϊ���ʾ��������
	//OutBuff[10]��ʼ�����ݣ��ѻ�ȡ
	OutBuff[10+len]=Negation_CS(OutBuff+1,9+len);								//1Byte������  ����OutBuff��ǰ9+len���ֽ�
	OutBuff[11+len]=Epilog_Code;												//1Byte�����룬�̶�16H
	return 12+len;		 														//�ܵı��ĳ���
}

/*******************************************************************************
���ƣ�INT8U NW_ReceiveAndExecute(u8 *OutBuff,u16 timeout_s)
���ܣ��ȴ��������ݣ�������ִ����Ӧ���ܡ�Э��������ڸ���case�·ֱ���С��ɹ�ʱ���أ�ʧ��ʱ
���ȴ�timeout_s�ľ���
��Σ�u8 timeout�����ճ�ʱ�趨����λΪ�롣
���Σ�u8 *OutBuff,�������ݴ��λ��
���أ����ղ�ִ�гɹ�����1�������⶯����ִ��ʱ����״̬�֣��������ȣ���δִ�л����ʧ���򷵻�0
*******************************************************************************/
INT8U NW_ReceiveAndExecute(u8 *OutBuff,u16 timeout_s)
{
	INT8U				*R;														//���Э���׵�ַ
	INT8U				Cmd,state;												//���cmd��return��ֵ
	INT16U				len=0;													//lenΪ���յ����ܳ���/��������֡
	static INT16U		len_frame = 0;											//len_frame֡���ȣ���Judge_NW_framing()��ȡ								

/*LTEģ���޷����׹رջظ������Ե��²�����timeout���г�ʱ�����ʱ����ѭ���ֽ���*/
	while(timeout_s--)															//timeout_s��λΪ��
	{
		/*���ղ��ж������Ƿ����Ҫ��*/
		len = LTE_WaitData(OutBuff,20);											//ÿ��ѭ���̶�����1�룬��LTEģ�鷵�ز��ɿصĻظ�ʱ����ʱ���ж�Ӱ�첻��
		R = Judge_NW_Framing(ANY_CMD,OutBuff,len,&len_frame);					//�ж�Э��
		if(R)																	//������Э�飬��Ҫ��ȴ��������뵱ǰ��������������ͬ������switch��һ���жϺ�ִ��
		{	
			Cmd =*(R+7);														//�����ص��������ȡ����
		/*���ݽ��յ�������ִ����Ӧ����*/
			switch(Cmd)
			{
				case START_UP://00H												//��վ�����·���װ���յ�����󣬷��Ϳ���������Ϣ
						if(Startup_Comm( RETRY, TIMEOUT )) return 1;			//��������ͨ�ţ��������RETRY�Σ�TIMEOUT�볬ʱ
						else break;
				
				case TIMMING://01H												//��վ�·���ʱ���װ���յ����������ԭ�����
						LteCommunication(R,len_frame,0,0);						//����ԭ����أ������գ�LteCommunication����0��
						state = SetTime(R);										//����RTC�����յ���������*R���ɹ�����1
						if(state) return state; else break;
				
				case SET_PASSWORD://02H											//װ���յ���������ж�ԭ�����Ƿ���ԭ����������ͬ������ͬ���������Ϊ�����룬������ԭ����ء�����ͬ���򷵻س�����Ϣ
						state = Set_Password_Comm(R,len_frame);					//�������빦�ܺ�����������R��������û���ж�Э�飩
						if(state) return state; else break;
				
				case PARA_CONFG://03H											//װ����֤����ͨ����ִ�в����������������ԭ����ء�����������򷵻����������Ϣ
						state = ParaConfigComm(R,len_frame);					//�������ù��ܺ�����������R��������û���ж�Э�飩
						if(state) return state; else break;
				
				case SET_IP://06H												//ֻ��������װ��������ͬ��������վIP���˿ںź���վ���Ŷ�Ӧ�ֽ���ȫ��ͬ��ִ�и������װ��ִ�и����������ԭ�����ʽ���ء�����������򷵻����������Ϣ
						state = Set_IP_Comm(R,len_frame);						//����վIP��ַ���˿ںźͿ��Ź��ܺ�����������R��������û���ж�Э�飩
						if(state) return state; else break;
				
				case DEMAND_IP://07H											//װ���յ�������󣬷����䵱ǰ���õ���վIP���˿ںź���վ���š�
						len = NW_Framing(DEMAND_IP,LTE_Tx_Buff);				//��֡
						LteCommunication(LTE_Tx_Buff,len,0,0);					//�ϱ����޻ظ������գ�LteCommunication����0��
						return 1;
				
				case RESET_DEV://08H											//ֻ��װ������ͨ����װ��ԭ����ز�ִ�д�������򷵻س�����Ϣ��
						if(!PassworkCheckAndReport( R )) break;					//�ж���վ�·������������Ƿ���ȷ������ȷʱ�ϱ����������Ϣ��
						return RESET_DEV;										//������ͬʱ����������װ��״̬��

				case SMS_AWAKE://09H											//��վ��ͨ��IP����UDPͨ�ţ����ʹ˿����֣�Ҳ��ʾ�����նˡ���������£���������״̬��װ��Ӧ�ڽӵ���������������л�������״̬��
						if(!PassworkCheckAndReport( R )) break;					//�ж���վ�·������������Ƿ���ȷ������ȷʱ�ϱ����������Ϣ��
						return SMS_AWAKE;										//������ͬʱ����������״̬��
				
				case DEMAND_CONFG://0AH											//������վ��ѯװ�����ò�����
						len = NW_Framing(DEMAND_CONFG,LTE_Tx_Buff);				//��֡
						LteCommunication(LTE_Tx_Buff,len,0,0);					//�ϱ����޻ظ������գ�LteCommunication����0��
						return 1;
				
				case FUN_CONFG://0BH											//������վ��װ���·��������ò�����װ�ù������ý��·���Ч���ܣ���Ч���ܲ����ͣ�Ĭ����Ч��
						state = Fun_Config_Comm(R,len_frame);					//װ�ù������ù��ܺ�����������R��������û���ж�Э�飩
						if(state) return state; else break;
				
				case DEMAND_DEV_TIM://0DH										//��վ������ѯװ��ϵͳʱ�䡣
						len = NW_Framing(DEMAND_DEV_TIM,LTE_Tx_Buff);			//��֡	ʱ��Ӧ�� ��	��	��	ʱ	��	��
						LteCommunication(LTE_Tx_Buff,len,0,0);					//�ϱ����޻ظ������գ�LteCommunication����0��
						return 1;
				
				case SMS_SEND://0EH												//��վҪ��װ����ָ�����ֻ����뷢��ȷ�϶��š�װ�ý��յ������ԭ����أ�Ȼ����ָ���Ķ��Ž��պ��뷢�ͱ�����װ�ñ�ţ��硰CC0011����
						state = SMS_Send_Comm(R,len_frame);						//����ȷ�϶��Ź��ܺ�����������R��������û���ж�Э�飩
						if(state) return state; else break;
				
				case DATA_REQUEST://21H											//������վ����������װ�÷������ݡ�װ���յ��������ԭ����أ�������������Ӧ�����ָ�ʽ����������������վ�����֣���Э��
						state = Data_Request_Comm(R,len_frame);					//��վ����װ�����ݹ��ܺ�����������R��������û���ж�Э�飩
						if(state==1) return state; else break;					//�����ϱ�����0xFF�������ȴ�
				
				case TEM_CUR_UPLOAD://26H										//װ���յ��������������δ��������������վ��
						state = Tem_Cur_Upload(RETRY,TIMEOUT);					//������ʷ���ݹ��ܺ����������ϱ�����0xFF��ʧ��ʱ����0������ʾ������ʷ����δ������ɣ���Ӱ������ɵĲ��֣�
						if(state==0xFF) LteCommunication(R,len_frame,0,0);		//��װ����δ�������ݣ���ԭ����ء�
						else if(state==1) return state; else break;				
				
				case FAULT_INFO://30H											//װ�ý��յ�������͵�ǰ���ڵĹ�����Ϣ��
						state = Fault_Info_Comm(RETRY,TIMEOUT);					//������Ϣͨ�ţ�ʧ��ʱ����0
						if(state) return state; else break;
				
				case FLOW_DATA_UPLOAD://40H										//�ն��յ������������������δ���ͳɹ�������������վ����װ����δ�������ݣ���ԭ����ء�
						state = Flow_Data_Upload(RETRY,TIMEOUT);				//��������ʹ������ϱ����ܺ���
						if(state) return state; else break;
				
			/*�ļ����ܿ���������ϸ�����ϱ�*/
				case FILE_LIST_QUERY://71H										//��վ��ѯĳ��ʱ�䷶Χ��װ�ô洢���ļ��б�װ���յ�������󣬷��ط��ϲ�ѯ�������ļ��б�
						state = File_List_Query_Comm(R,len_frame);
						if(state) return state; else break;
				
				case FILE_REQUEST://72H											//װ���յ��������ִ���ļ��ϱ�����
						LteCommunication(R,len_frame,0,0);						//����ԭ����أ������գ�LteCommunication����0��
						state = Files_Upload(R+10,DES_LEN);						//����������Ӧ�ļ�������վ��
						if(state) return state; else break;
				
			/*������չЭ�� NW_ReceiveAndExecute*/
				case EXTEN_ONOFF://F0H											//ֻ��������װ��������ͬ��ִ�д����FFHΪ���ã�����װ���ϱ���չЭ�飻00HΪ���ã���ֹװ���ϱ���չЭ�顣װ�ý��յ��������ԭ����ء�
						state = ExtendOnOffComm(R,len_frame);					
						if(state) return state; else break;
				
				case EX_SET_APN://F3H											//ֻ��������װ��������ͬ��ִ�д����װ��ִ�и����������ԭ�����ʽ���ء�����������򷵻����������Ϣ
						state = SetApnComm(R,len_frame);						//����APN
						if(state) return state; else break;
				
				case EX_DEMAND_APN://F4H										//ֻ��������װ��������ͬ��ִ�д����װ��ִ�и�ʽ��FLASH����ɹ��󣬰���ԭ�����ʽ���ء�����������򷵻����������Ϣ
						len = NW_Framing(EX_DEMAND_APN,LTE_Tx_Buff);			//��֡
						LteCommunication(LTE_Tx_Buff,len,0,0);					//�ϱ����޻ظ������գ�LteCommunication����0��
				
				case EX_VERSION_SIM://F6H										//װ�ý��յ���վ��ѯָ���������ȡװ����Ϣ���ظ���
						len = NW_Framing(EX_VERSION_SIM,LTE_Tx_Buff);			//���ݿ�������֡    
						LteCommunication(LTE_Tx_Buff,len,0,0);					//�ϱ����޻ظ������գ�LteCommunication����0��
						return 1;
				
				case EX_UPDATA_REQUEST://F7H 									//��վ���յ���չ����5����·���������
						state = UpdataRequestComm( R );							//װ�ý��յ���վ������������̽���������Ⲣ���лظ���
						if(state) return state; else break;					
																				
				case EX_UPDATA_DOWNLOAD://F8H 									//��վ�·�������������������Ϊ���������������ݰ��·����Ϊ1�롣
						state = UpdataDownloadComm( R );						//�����ļ��ͱ��棨д��FLASH��
						if(state) return state; else break;		

				case EX_UPDATA_DL_DONE://F9H 									//ȫ���������·�������2�룬��վ���͸�ָ��
						state = UpdataFinishComm( R );							//װ���յ��������ϴ��ļ�������FAH����
						if(state) return state; else break;
				
				case EX_FORMAT_FLASH://FBH										//ֻ��������װ��������ͬ��ִ�д����װ��ִ�и�ʽ��FLASH����ɹ��󣬰���ԭ�����ʽ���ء�����������򷵻����������Ϣ
						state = FormatFlashComm(R,len_frame);					//��ʽ��FLASH
						if(state) return state; else break;
						
				default:
						break;
			}	
		}	
	}
	BspUartWrite(2,SIZE_OF("NW_Receive_Execute�ȴ��������ݽ���\r\n"));
	return 0;																	//��ʱ������0
}

/*******************************************************************************
���ƣ�bool PassworkCheckAndReport(INT8U *Inbuff)
���ܣ��ж���վ�·������������Ƿ���ȷ������ȷʱ�ϱ����������Ϣ��
��Σ�INT8U *Inbuff��������ʼ��ַ
���Σ���
���أ�������ͬ�������棻���벻ͬ�����ؼ�
*******************************************************************************/
bool PassworkCheckAndReport(INT8U *Inbuff)
{
	INT8U len = 0;
	if(memcmp(Inbuff+10,Config.Password,4))										//���ж�ԭ������ԭ�������벻ͬ����ͬʱmemcmp����0��
	{
		len = NW_Framing( PASSWORD_ERR, LTE_Tx_Buff );							//�����������Ϣ֡
		LteCommunication(LTE_Tx_Buff,len,0,0);											//�ϱ�������Ϣ�������գ�LteCommunication����0��
		return false;															//���벻ͬ�����ؼ�
	}
	return true;																//������ͬ��������
}

/*******************************************************************************
���ƣ�INT16U NW_Framing(u8 FileUpCmd,u8 *OutBuff)
���ܣ�������Э����֡��֡�ṹ���������и�ʽ˵��:���ݰ���������֡ģʽ��������֡������ʼ�롢
װ�ú��롢���������롢�����򳤶ȡ�������У����ͽ����롣����֡���Ȳ�����4000�ֽڣ�����
ͨ�ŷ�ʽ��֡��������130�ֽڡ����ô��ģʽ�����ݵĸ��ֽڱ������ڴ�ĵ͵�ַ�У���У�����
���ۼӺ�ȡ����У�鷽ʽ�����ͷ���װ�ú��롢�����֡������򳤶Ⱥ��������������ֽڽ���������
�ӣ�������λ��ֻ��������ֽڣ������ֽ�ȡ����
----------------------------------------------------------------
��ʼ��	װ�ú���		������	�����򳤶�	������	У����	������	|
1�ֽ�	6�ֽ�		1�ֽ�	2�ֽ�		�䳤	1�ֽ�	1�ֽ�	|
----------------------------------------------------------------
��Σ�INT8U Cmd,�����֣����������������ͣ�
���Σ�INT8U *OutBuff����֡���Ĵ�ŵ�ַ
���أ��ܵı��ĳ���
*******************************************************************************/
INT16U NW_Framing(u8 Cmd,u8 *OutBuff)
{
	INT16U len;																	//���ݳ���

	len=Get_DataField(Cmd,&OutBuff[10]);										//��ȡ���ݣ��������ݳ���
	if(Cmd==DATA_UNCORRESPOND) Cmd=SET_IP;										//�����е��ң��Ȳ����ˣ�Ŀǰ�õ� DATA_UNCORRESPOND ��ֻ��SET_IP
	OutBuff[0]=Start_Code;														//1Byte��ʼ�룬�̶�68H
	OutBuff[1]=Device_Number[0];
	OutBuff[2]=Device_Number[1];												//ǰ���ֽڱ�ʾ���Ҵ��루���Ϸ�������˾ͳһ���䣩�����ô�д��ĸ(ASCII)
	OutBuff[3]=Device_Number[2];												//�����ֽڱ�ʾ���Ҷ�ÿ��״̬���װ�õ�ʶ���루��վ��ַ�������ô�д��ĸ�����֣�����ʹ������
	OutBuff[4]=Device_Number[3];														
	OutBuff[5]=Device_Number[4];														
	OutBuff[6]=Device_Number[5];												//6Byteװ�ú���
	OutBuff[7]=Cmd;																//1Byte�����֣�����������������
	OutBuff[8]=(len>>8)&0xff;													//
	OutBuff[9]=len&0xff;														//2Byte�����򳤶ȣ����ֽ���ǰ����Ϊ���ʾ��������
	//OutBuff[10]��ʼ�����ݣ��ѻ�ȡ
	OutBuff[10+len]=Negation_CS(OutBuff+1,9+len);								//1Byte������  ����OutBuff��ǰ9+len���ֽ�
	OutBuff[11+len]=Epilog_Code;												//1Byte�����룬�̶�16H
	return 12+len;		 														//�ܵı��ĳ���
}

/*******************************************************************************
���ƣ�INT8U *Judge_NW_Framing(u8 Cmd,u8 *InBuff,u16 Len,u16 *OutLen)
���ܣ������ж��Ƿ��������Э�顣У������ʼ�롢װ�ú��롢���������롢�����򳤶ȡ�������
У����ͽ����롣ע��CmdΪ0xffʱ���κο����ֿ�ͨ��
��Σ�INT8U Cmd�������֣�Ϊ0xffʱ���κο����ֿ�ͨ��У��  *InBuff��������  Len���볤��
���Σ�*OutLen����ʵ����Ч�ĳ���  OutLen����Ϊ0ʱ�����
���أ�0 �����ϣ���0 ���ϣ��ҷ������ݰ����ֽڵ�ַ INT8U *
*******************************************************************************/
INT8U *Judge_NW_Framing(u8 Cmd,u8 *InBuff,u16 Len,u16 *OutLen)
{
	INT8U 	i;
	INT16U 	Packet_Length;
	INT8U 	NCS; 
	INT8U 	*R;
	/*���ĳ����ж�*/
	if(Len<12)return 0;															//Э��������� ����С��12
	for(i=0;i<12;i++)															//���ǰ��12�ֽ����ް�ͷ
	{
		if(InBuff[i]==Start_Code)
		{
			R=&InBuff[i];														//��¼��ʼ��λ��
			Len-=i;																//��ʼ�뿪ʼ����
			break;
		}
	}
	if(i>=12)return 0;															//12�ֽ���δ�ҵ���ͷ����Ϊ���ݰ���Ч	
	Packet_Length=R[8]<<8;														//R[9]Ϊ���ݳ��ȵĸ��ֽ�
	Packet_Length+=R[9];														//R[10]Ϊ���ݳ��ȵĵ��ֽ� ��ȡ���ݳ���
	Packet_Length+=12;															//���屨�ĵĳ���
	if(Len<Packet_Length) return 0;												//Э�鳤�ȴ���>ʱ�����ǰ�β����Ч���ݣ�
	/*�����ж�*/
	if(R[Packet_Length-1]!=Epilog_Code) return 0;								//������У��
	if(Cmd!=ANY_CMD && Cmd!=R[7]) return 0;										//������У��  ע��CmdΪ0xffʱ���κο����ֿ�ͨ��
	if(memcmp(&R[1],Device_Number,6)) return 0;									//�Ƚ�6�ֽ�װ�ú��룬����ͬ����0
	NCS=Negation_CS(R+1,Packet_Length-3);										//�ۼӺ�ȡ�����㣬����ǰPacket_Length-3���ֽ�					
	if(NCS==R[Packet_Length-2])													//����ȡ�����е�NCSֵ������ֵ��ͬ��������2�ֽ�ΪNCS��
	{
		if(OutLen) *OutLen=Packet_Length;										//������ĳ��ȣ�OutLen����Ϊ0ʱ�������
		return R;																//������ЧЭ���ͷ��ַ
	}
	else return 0;																//NCSУ��ʧ��
}

/*******************************************************************************
���ƣ�INT16U Get_DataField(u8 Cmd,u8 *OutBuff)
���ܣ�����Cmd��ȡ���ݣ������򣩣����س���
��Σ�u8 Cmd,������
���Σ�u8 *OutBuff����ȡ�������ݣ�������֡��NW_framing���������
���أ���0����ȡ�����ݣ������򣩳���  0����ȡʧ��
*******************************************************************************/
INT16U Get_DataField(u8 Cmd,u8 *OutBuff)
{
	INT8U	i,len,pack_num=0,fault_state=0;
	INT16U	delta_T=0;
	INT8U*	pointer;

	switch(Cmd)
	{
		case START_UP://00H														//������Ϊ���淶�汾��
				OutBuff[0] = PROTOCOL_VERSION_H;
				OutBuff[1] = PROTOCOL_VERSION_L;
				return 2;														//����Ϊ2
		
		case TIMMING://01H														//������Ϊ����
				return 0;														//����Ϊ0
		
		case HEARTBEAT://05H													//������Ϊ���źż�¼ʱ��	�ź�ǿ��	���ص�ѹ
				return Get_HB_INFO(OutBuff);									//��ȡ������Ϣ���ݣ���������Э���ʽ�����������س���8�ֽ�
		
		case DEMAND_IP://07H													//������Ϊ����վIP	�˿ں�	��վ����
				memcpy(OutBuff,IP_Config.IP_addr_1,4);							
				memcpy(OutBuff+4,IP_Config.PortNum_1,2);
				memcpy(OutBuff+6,IP_Config.CardNum_1,6);
				return 12;														//����Ϊ12

		case DEMAND_CONFG://0AH													//������վ��ѯװ�����ò�����
				memcpy(OutBuff,Config.BeatTime,10);								//����������ɼ����������ʱ��������ʱ����Ӳ������ʱ���
				memset(OutBuff+10,0,10);										//ͼ�����10�ֽڣ���0
				OutBuff[20] = TEM_CUR_UPLOAD;									//��Ч����1�������¶ȡ��������ݼ�⹦��26H
				return 21;														//����Ϊ21
		
		case SLEEP_NOTICE://0CH													//������Ϊ����
				return 0;														//����Ϊ0
		
		case DEMAND_DEV_TIM://0DH												//��վ������ѯװ��ϵͳʱ�䡣
				return NW_GetTime((struct NW_TIME *)OutBuff);					//��ȡRTCʱ�䣬����������Э���ʽ�����������س���6�ֽ�
		
		case TEM_CUR_UPLOAD://26H												//������Ϊ��������֤	֡��ʶ	����	�װ�	��һ��	�ڶ�������	��N������������վ����װ�����ݺ������ϴ���
				memcpy(OutBuff,Config.SecurityCode,4);							//������֤
				memcpy(OutBuff+4,&Tem_Cur_Data,14);								//���������������֤������Tem_Cur_Data�ṹ�����ˣ�1+1+1+6+2+2+1=14
				return 18;														//����Ϊ18

		case FAULT_INFO://30H	�пհ���һ��Ҳ����ɺ���
				memcpy(OutBuff,Config.SecurityCode,4);							//4Byte������֤
				OutBuff[4] = 1;													//1Byte֡��ʶ����1֡
		
				pointer = (INT8U*)&Fault_Manage;
				for(i=1;i<Fault_Magage_Len;i++)									//�ṹ���е�һ����Need_Report������Ҫ�жϣ���i=1��ʼ
				{
					if(0x55==*(pointer+i))										
					{
						fault_state = 0xFF;										//ֻҪ��һ�������55��˵���豸���ڹ���״̬
						break;
					}				
				}
		
				for(i=0;i<FI_NUM;i++)											//��ѯFI_NUM��������Ϣ
				{
				/*���ڱ�ʾ��Ҫ�ϱ�������Ϣ*/
					if(Fault_Info[i].Time)										
					{
						/*�װ�����0����8Byte*/
						if(!pack_num)
						{
							SecondToNwTime(Fault_Info[i].Time,(struct NW_TIME*)(OutBuff+7));		//6Byte����ʱ�䣨��+��+��+ʱ+��+�룩��6�ֽڣ�HEX��ʾ��
							OutBuff[13] = Fault_Info[i].Function_Code;			//1Byte���ܱ���
							OutBuff[14] = Fault_Info[i].Fault_Code;				//1Byte���ϱ���	
						}
						/*��һ���Ժ�����ϰ� 4Byte*/
						else 
						{
							delta_T=Fault_Info[i].Time-Fault_Info[i-1].Time;	//�������ϰ�����ʱ���
							if(delta_T>0xFFFF) delta_T=0xFFFF;					//��Ҫ��֡���Ȳ��ܣ��̶�0xFFFF
							OutBuff[15+7*(i-1)]=(delta_T>>8)&0xFF;				//2Byte����ʱ�����ֽ�
							OutBuff[16+7*(i-1)]=delta_T&0xFF;					//���ֽ�
							OutBuff[17+7*(i-1)]=Fault_Info[i].Function_Code;	//1Byte���ܱ���
							OutBuff[18+7*(i-1)]=Fault_Info[i].Fault_Code;		//1Byte���ϱ���	
						}
						
						pack_num++;												//����
					}	
				}
				
				if(!pack_num) BspUartWrite(2,SIZE_OF("��\r\n"));
				else BspUartWrite(2,SIZE_OF("��\r\n"));
				OutBuff[5] = pack_num;											//1Byte���������ڹ�����Ϣ��
				OutBuff[6] = fault_state;										//1Byte�豸״̬��װ���豸��ǰ��״̬��00H������FFH����
				return 7+8*(pack_num?1:0)+4*(pack_num?pack_num-1:0);			//����4��֡��1������1��״̬1���װ�8������ÿ��4
				
		case FLOW_DATA_UPLOAD://40H												//������Ϊ��������֤	֡��ʶ	����	�װ�	��һ��	�ڶ�������	��N��
				memcpy(OutBuff,Config.SecurityCode,4);							//������֤
				OutBuff[4] = 1;													//֡��ʶ����ֻ��1֡��1�װ�������ÿ�����������滻��
				OutBuff[5] = 1;													//��������һ���װ�
				memcpy(OutBuff+6,&Flow_Data,18);								//�װ�����Flow_Data�ṹ�壬6+4+4+4=18
				return 24;														//����Ϊ24
		
	/*������չЭ�� Get_DataField*/
		case EX_DEMAND_APN://F4H												//������Ϊ��APN	0~100�ֽ�
				strncpy((char*)OutBuff, (char*)APN, APN_Len);					//�����ַ���
				len = strlen((char*)APN)+1;										//�����򳤶Ȱ���������'\0'��
				return len;														//���������򳤶�											
		
		case EX_HEARTBEAT://F5H													//������Ϊ����¼ʱ�䡢�������ݵ�ѹ�������¶ȡ�MCU�¶�
				return GetExtendedHeartbeatData(OutBuff);						//��ȡ��չ������Ϣ���ݣ�������չЭ���ʽ�����������س���11�ֽ�
					
		case EX_VERSION_SIM://F6H												//������Ϊ��Ӳ���汾��	����汾��	SIM���ţ�13λ��	ICCID��20λ��	IMSI��15λ��
				return GetDeviceVersionAndCardNumber(OutBuff); 					//��ȡװ�ð汾�����ŵ���Ϣ
		
		case EX_UPDATA_REQUEST://F7H											//������Ϊ����ǰ����汾��	���������
				return GetUpdataRequestData(OutBuff);							//��ȡ��������ظ�֡���ݣ�������չЭ���ʽ�����������������򳤶ȡ�
		
		case EX_UPDATA_FILLING://FAH											//������Ϊ����������汾��	������	��һ������	�ڶ�������	����
				return GetUpdataFillingData(OutBuff);							//��ȡ��ȡ�����ظ�֡���ݣ�������չЭ���ʽ�����������������򳤶ȡ�

	/*�Զ���ġ�������06H���ݲ���Ӧ����ʱ�����س�����Ϣ������Ҫ�ֶ��Ļ�LTE_Tx_Buff[7]=cmd*/
		case DATA_UNCORRESPOND://DEH													
				OutBuff[0] = 0x00;												
				OutBuff[1] = 0x00;
				return 2;
		
	/*���漸��������֡����ʾ������FFFF������ԭ����أ��������*/
		case PASSWORD_ERR:	//DDH���������Ҳ�����ڷ�����չЭ�顣
		case SET_PASSWORD:	//02H		
		case PARA_CONFG:	//03H
		case SET_IP:		//06H
		case RESET_DEV:		//08H
		case SMS_AWAKE:		//09H	
		case FUN_CONFG:		//0BH
		case SMS_SEND:		//0EH
				OutBuff[0] = 0xFF;											
				OutBuff[1] = 0xFF;
				return 2;
		
		default:
				break;
	}
	return 0;
}

/*******************************************************************************
���ƣ�INT8U SetTime(u8 *InBuff)
���ܣ���������Э������ʱ�䡣
��Σ�u8 *InBuff����վ�·��������Ķ�ʱ֡�׵�ַ
���Σ���
���أ�1���ɹ�   0��ʧ��
*******************************************************************************/
INT8U SetTime(u8 *InBuff)
{
	TCHAR				chars[40];
	INT32U 				second=0;

	second =NwTimeToSecond((struct NW_TIME *)(InBuff+10));						//����λ���·�������ʱ��ת��Ϊ������
	if(RtcSetTimeSecond(second))												//��ʱ��д��RTC
	{
		Time_Proofread = DONE;													//���ΪУʱ�����
		sprintf(chars, "Уʱ�ɹ���20%02d��%02d��%02d�� %02d:%02d:%02d\r\n", InBuff[10], InBuff[11], InBuff[12], InBuff[13], InBuff[14], InBuff[15]);
		BspUartWrite(2,(INT8U*)chars,strlen(chars));							//��ӡ��ǰʱ��
		return 1;
	}
	Time_Proofread = UNDONE;													//���ΪУʱδ���
	sprintf(chars, "RTCоƬд���쳣��Уʱʧ�ܣ�\r\n");	
	BspUartWrite(2,(INT8U*)chars,strlen(chars));								//��ӡУʱʧ��
	return 0;
}

/*******************************************************************************
���ƣ�INT8U NW_GetTime(struct NW_TIME *time)
���ܣ���ȡRTCʱ�䣬���RTC����/�ָ�������������Э���ʽ�����������س���6�ֽڡ�
���RTC�Ƿ��д���/�ָ����ܣ�ԭ�ȷ���Task_Wdt_main�У�����Ϊ��ִ������������Ϣ��ȡ��
��˻�����쳣����Ҫ�Ƶ������ȡ֮��
��Σ�struct NW_TIME *time����+��+��+ʱ+��+�룩��HEX��ʾ��
���Σ���
���أ�6���ɹ�/����   0��ʧ��
*******************************************************************************/
INT8U NW_GetTime(struct NW_TIME *time)
{
	if(!RtcGetChinaStdTimeStruct(&gRtcTime)) 									//����ȡ��ǰ��ʱ�䵽gRtcTime�ṹ��ʧ��
	{
		if(!Fault_Manage.F_RTC) NW_Fault_Manage(RTC_F, FAULT_STA);				//����ǰ�޹��ϣ�дRTC���ϲ�����Ϣ
		return 0;		 														//0��ʧ��
	}	
	else if(Fault_Manage.F_RTC) NW_Fault_Manage(RTC_F, NOFAULT_STA);			//RTC�������ҳ������ϡ�����RTC�ָ��ˣ�д��RTC���ϻָ���Ϣ

	time->year = BcdToHex(gRtcTime.Year);										//��Ϊ��ǰ��ݼ�ȥ2000����2017-2000=17 ʮ������
	time->mon = BcdToHex(gRtcTime.Month);										//��ǰ�� ʮ������
	time->mday = BcdToHex(gRtcTime.Day);										//��ǰ�� ʮ������
	time->hour = BcdToHex(gRtcTime.Hour);										//��ǰʱ ʮ������
	time->min = BcdToHex(gRtcTime.Minute);										//��ǰ�� ʮ������
	time->sec = BcdToHex(gRtcTime.Second);										//��ǰ�� ʮ������
	return 6;
}

/*******************************************************************************
* Function Name:  INT8U SecondToNwTime(INT32U sencond,INT8U *outbuff)              
* Description  :  ������ת��Ϊ������ʽʱ�䣬��Ҫ����ʱ����
* Input        :  sencond : ������
*				  Time	  : �����ʱ�����飬˳��Ϊ������ʱ����
* Return       :  ��     
*******************************************************************************/
INT8U SecondToNwTime(INT32U sencond,struct NW_TIME *time)
{
	struct tm 	*TTME = 0;														//���������Զ�ʶ��Ϊ��ָ

	sencond += 8*3600;															//��������0��ʱ�䣬ת��Ϊ������ ( UTC +8 )
	TTME =localtime(&sencond);													//������ת��Ϊʱ������
	time->year  = TTME->tm_year-100;											//��1900 ��ʼ����
	time->mon  = TTME->tm_mon+1;												//�·ݼ���1(localtime�ļ�������0��ʼ)
	time->mday  = TTME->tm_mday;
	time->hour  = TTME->tm_hour;
	time->min  = TTME->tm_min;
	time->sec  = TTME->tm_sec;

	if((time->year<=99)
		||(0<time->mon<13)
		||(0<time->mday<32)
		||(time->hour<24)
		||(time->min<60)
		||(time->sec<60))	return 1;
	return 0;
}

/*******************************************************************************
* Function Name:  INT32U NwTimeToSecond(struct NW_TIME *time)           
* Description  :  �й���׼ʱ�䣨CST����¼������ʱ�䣬ת��Ϊ��1970 1.1.0ʱ��ʼ�����
				  �����������롢ʱ��������ء�
* Input        :  struct NW_TIME *ptime��������ʽʱ��
* Return       :  sencond   �����롢ʱ���
*******************************************************************************/
INT32U NwTimeToSecond(struct NW_TIME *time)
{
	time_t sencond = 0;															//typedef unsigned int time_t;     /* date/time in unix secs past 1-Jan-70 */
	struct tm TTM = {0};      								

	TTM.tm_year = (time->year)+100;  											// ��
	TTM.tm_mon  = (time->mon)-1;   												// ��
	TTM.tm_mday = (time->mday);       											// ��
	TTM.tm_hour = (time->hour);      											// ʱ
	TTM.tm_min  = (time->min);    												// ��
	TTM.tm_sec  = (time->sec);    												// ��
	sencond = mktime(&TTM)-8*3600;                  							//������ ( UTC +8 )ʱ��ת����������
	
	if (sencond==0xffffffff) return 0;											//�쳣
	return sencond;
}

/*******************************************************************************
���ƣ�INT8U Get_HB_INFO(u8 *OutBuff)
���ܣ���ȡ������Ϣ���ݣ���������Э���ʽ�����������س���8�ֽڡ�
��Σ���
���Σ�u8 *OutBuff��������Ϣ���ݴ�ŵ�ַ
���أ�8���ɹ�/����   0��ʧ�ܣ�����ʱû�õ����Ȳ����ˣ�
*******************************************************************************/
INT8U Get_HB_INFO(u8 *OutBuff)
{
	NW_GetTime((struct NW_TIME *)OutBuff);										//��ȡ��ǰʱ��
	Get_Voltage_MCUtemp_Data( 3 );												//��ȡ��ص�ѹ���ݺ͵�Ƭ���¶�
	OutBuff[6]=HB_Get_Signal_Strength();										//��ȡ��ǰ�ź�ǿ��
	OutBuff[7]=(INT8U)(Equipment_state.BAT_Volt*10);							//��ȡ��ص�ѹ��10����
	return 8;
}

/*******************************************************************************
���ƣ�INT8U Get_NW_Info(void)
���ܣ��������ж�ȡFault_Info������Ϣ�ṹ�����顢δ�ϱ�����������
��Σ���
���Σ���
���أ�0��ʧ�ܣ�1���ɹ�
*******************************************************************************/	
INT8U Get_NW_Info(void)
{
	if(!BSP_ReadDataFromFm(Fault_Manage_Addr,(INT8U*)&Fault_Manage,Fault_Magage_Len)) return 0;			//�������ж�ȡFault_Manage������Ϣ����ṹ��
	memset(Fault_Manage.F_RF,0,110);
	BSP_WriteDataToFm(Fault_Manage_Addr,(INT8U*)&Fault_Manage,Fault_Magage_Len);						//ÿ���ϵ��̽ͷ���ϱ�־������ⲻ���ڿ����Լ죨��λ�Իָ�������Ŀ���������̽ͷ���ϱ�־λ�����ܻᵼ�¹�����Ϣ���ң��������Ϣ�ϱ��������ˣ����������־λ�����ϵ��⵽�й��ϣ���ȴ�޹�����Ϣ���ϱ���
	
	if(!BSP_ReadDataFromFm(Fault_Info_Addr,(INT8U*)Fault_Info,Fault_Info_Len)) return 0;				//�������ж�ȡFault_Info������Ϣ�ṹ������
	if(!BSP_ReadDataFromFm(Unreport_Index_Addr,(INT8U*)Unreport_Index,Unreport_Index_Len)) return 0;	//δ�ϱ�����������
	return 1;	
}

/*******************************************************************************
���ƣ�void NW_Fault_Manage(INT8U type, INT8U fault_state)
���ܣ����Ϸ����ͻָ�ʱ���ã����ڴ�����Ӧ������Ϣ����������ر�־λ��
��Σ�INT8U fault_state, ����״̬��1�ָ���0�쳣
���Σ���
���أ�0��ʧ�ܣ�1���ɹ�
*******************************************************************************/
void NW_Fault_Manage(INT8U type, INT8U fault_state)
{
	INT8U			i,Function_Code,Fault_Code;
	INT32U			Time;
	char			Buff[256]={0};
	
/*���ݴ��������ֵ*/
	Time = RtcGetTimeSecond();													//����ʱ�䣬������
	switch(type)
	{
		case RTC_F:
				if(!Time) Time = 1577808000;									//ʱ���쳣Ϊ0ʱ��д1577808000��2020-01-01 00:00:00��������д0������ᱻ�жϳɿ��У�
				Function_Code = 0x01;											//01H�����ص�Ԫ
				Fault_Code = 0x01|fault_state;									//01H��ʱ�ӹ���/�ָ�
				if(fault_state) Fault_Manage.F_RTC=0;							//���ϻָ����
				else Fault_Manage.F_RTC=0x55;									//���ϴ�����
				strcpy(Buff,"RTC����"); 
				break;
		
		case STORAGE_F:
				Function_Code = 0x01;											//01H�����ص�Ԫ
				Fault_Code = 0x02|fault_state;									//02H���洢����/�ָ�
				if(fault_state) Fault_Manage.F_STORAGE=0;						//���ϻָ����
				else Fault_Manage.F_STORAGE=0x55;								//���ϴ�����
				strcpy(Buff,"�洢����"); 
				break;
		
		case REPLY_F:
				Function_Code = 0x02;											//01H��DTUģ��
				Fault_Code = 0x01|fault_state;									//01H��30��������վ��Ӧ�����/�ָ�
				if(fault_state) 
				{
					Fault_Manage.F_REPLY=0;										//���ϻָ����
					wakeup_en.reply = true;										//����LTEģ��������л���
				}
				else 
				{
					Fault_Manage.F_REPLY=0x55;									//���ϴ�����
					wakeup_en.reply = false;									//��ֹLTEģ��������л���
				}
				strcpy(Buff,"��վ��Ӧ�����");
				break;
		
		case NETWORK_F:
				Function_Code = 0x02;											//01H��DTUģ��
				Fault_Code = 0x02|fault_state;									//01H��30�������޷���½�����������/�ָ�
				if(fault_state) 
				{
					Fault_Manage.F_NETWORK=0;									//���ϻָ����
					wakeup_en.network = true;									//����LTEģ��������л���
				}
				else 
				{
					Fault_Manage.F_NETWORK=0x55;								//���ϴ�����
					wakeup_en.network = false;									//��ֹLTEģ��������л���
				}
				strcpy(Buff,"�����������"); 
				break;
		
		case BAT_F:
				Function_Code = 0x03;											//03H����Դ����ģ��
				Fault_Code = 0x01|fault_state;									//01H�����ص�ԴǷѹ����/�ָ�
				if(fault_state) 
				{
					Fault_Manage.F_BAT=0;										//���ϻָ����
					wakeup_en.battle = true;									//����LTEģ��������л���
				}
				else 
				{
					Fault_Manage.F_BAT=0x55;									//���ϴ�����
					wakeup_en.battle = false;									//��ֹLTEģ��������л���
				}
				strcpy(Buff,"��ԴǷѹ����"); 
				break;
		
		default:
				break;
	}
	
	/*̽ͷ��ƵͨѶʧ�ܣ����ϱ�־д��Fault_Manage.F_RF*/
	if(type<=54)						
	{
		Function_Code = Unit_ID_Code[type];										//װ�ù��ܵ�Ԫʶ����10H-5FH
		Fault_Code = 0x01|fault_state;											//01H��5�������������޷�������ƵͨѶ����/�ָ�
		if(fault_state) Fault_Manage.F_RF[type]=0;								//���ϻָ����
		else Fault_Manage.F_RF[type]=0x55;										//���ϴ�����	
		strcpy(Buff,"��ƵͨѶ����");
	}
	
	/*̽ͷ�յ������¶������쳣�����ϱ�־д��Fault_Manage.F_TEM*/
	else if(55<=type&&type<=109)	
	{		
		Function_Code =Unit_ID_Code[type-55];									//װ�ù��ܵ�Ԫʶ����10H-5FH
		Fault_Code = fault_state;												//02H�������¶������쳣����/�ָ�	ԭ��Fault_Code = 0x02|fault_state;
		if(fault_state==NOFAULT_STA) 
		{
			Fault_Manage.F_TEM[type-55]=0;										//���ϻָ���ǡ������������ָ����������Ӧ�Ĺ��ϱ�ǡ�
			wakeup_en.rf_tem = true;											//����LTEģ��������л��ѣ��������⣺�����̽ͷͬʱ����ʱ����ʱ�ỽ�ѣ�
		}
		else 
		{
			Fault_Manage.F_TEM[type-55]=0x55;									//���ϴ����ǡ��������������ϡ������ö�Ӧ�Ĺ��ϱ�ǡ�
			wakeup_en.rf_tem = false;											//��ֹLTEģ��������л��ѣ��������⣺�����̽ͷͬʱ����ʱ����ʱ�ỽ�ѣ�
		}
		strcpy(Buff,"�¶������쳣����");
	}		

//		case CUR_F:
//				Function_Code = ���ܵ�Ԫʶ����;												//01H�����߲�����װ��	װ�ù��ܵ�Ԫʶ����10H-5FH
//				Fault_Code = 0x03|fault_state;									//03H�����ߵ��������쳣����/�ָ�
//				break;

//		case POWER_F:
//				Function_Code = ���ܵ�Ԫʶ����;												//01H�����߲�����װ��	װ�ù��ܵ�Ԫʶ����10H-5FH
//				Fault_Code = 0x04|fault_state;									//04H�����粻�����/�ָ�
//				break;
		
	Fault_Manage.Need_Report=0x55;												//�������Ҫ���й����ϱ�(�����ǹ��Ϸ������ǹ��ϻָ�����Ҫ�ϱ�)
	
	if(Fault_Code & 0x80) strcpy(Buff+strlen(Buff),"�ѻָ������룺");			//���λ��1����ʾ�ָ�
	else strcpy(Buff+strlen(Buff),"���������ϴ��룺");
	sprintf(Buff+strlen(Buff),"%02X %02X\r\n",Function_Code,Fault_Code);
	BspUartWrite(2,(INT8U*)Buff,strlen(Buff));									//��ӡ
	
/*���ҿ��нṹ�岢���*/
	for(i=0;i<FI_NUM;i++)
	{
		if(!Fault_Info[i].Time)													//Ϊ0ʱ��ʾ��λ�ÿ���
		{
		/*дȫ�ֱ���*/
			Fault_Info[i].Time = Time;											//ʱ���Ѿ��쳣������д0������ᱻ�жϳɿ���
			Fault_Info[i].Function_Code = Function_Code;						//���ܱ���
			Fault_Info[i].Fault_Code = Fault_Code;								//���ϱ���
		/*д������*/
			BSP_InitFm(Fault_Num);												//���õ͹��ĺ�������Ҫ���³�ʼ��
			BSP_WriteDataToFm(Fault_Manage_Addr,(INT8U*)&Fault_Manage,Fault_Magage_Len);	//дFault_Manage������
			BSP_WriteDataToFm(Fault_Info_Addr,(u8*)&Fault_Info,Fault_Info_Len);	//дFault_Info[]������
			FM_LowPower(Fault_Num);												//�������ŵ͹�������
			break;
		}
	}
}

/*******************************************************************************
���ƣ�void NW_DeviceNumberToAscii(INT32U InData,INT8U *pOut)
���ܣ��������16���Ʊ�ʾ��װ�ñ��룬ת����4�ֽڳ���ASCII�롣
��Σ�INT16U InData��16���Ʊ�ʾ��װ�ñ���
���Σ�INT8U *pOut�������ASCII��
���أ���
*******************************************************************************/
void NW_DeviceNumberToAscii(INT32U InData,INT8U *pOut)
{
	INT8U				gTemp = 0, i = 0;

	if(InData>9999) return;														//�������뷶Χ
	for(i = 3; i != 0xff; i--)
	{
		gTemp = InData%10;     													//ȡ��λ
		pOut[i] =  gTemp + 0x30;
		InData -= gTemp;
		InData = InData/10;														//ȥ����λ
	}
}
