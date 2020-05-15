/***************************** (C) COPYRIGHT 2019 ���ϵ��� *****************************
* File Name          : LTE.c
* Author             : ��ӱ�ɡ���������
* Version            : ����ʷ�汾��Ϣ
* Date               : 2019/02/21
* Description        : LTEͨ�Ź���ʵ�֣���Ҫ����ͨ��Э����4Gģ�������ļ���
************************************  ��ʷ�汾��Ϣ  ************************************
* 2019/03/28    : V4.1.0
* Description   : ����������Ŀ���档����������ɣ������С�
*******************************************************************************/
#include "LTE.h"

/*ȫ�ֱ�������*/
INT8U 			LTE_Buff[LTE_BUFF_LEN] = {0};									//������֡
INT8U 			LTE_Tx_Buff[LTE_BUFF_LEN] = {0};								//����LTE����
INT8U 			LTE_Rx_Buff[LTE_BUFF_LEN] = {0};								//����LTE����
INT32U			Net_Fault_Time=0;												//������������ʧ�ܼ�ʱ������ʱ��30����ʱ����һ���������Ӵ�������ϱ�����������ʱ��������������ӻָ��ϱ�
INT32U 			Host_No_Reply_Time=0;											//������վ��Ӧ���ʱ������ʱ��30����ʱ����һ����վ��Ӧ���������ϱ����ٴη���ѶϢ�õ��ظ�ʱ�������ϻָ��ϱ�



/************************************************************************************************************************
* Function Name : void Task_LTE_Main(void *arg)                                                    
* Description   : LTE������������LTE�������ݡ�
*************************************************************************************************************************/
void Task_LTE_Main(void *arg)
{
	INT8U	retry = 0;
	INT8U	SOCKID = 1;   														// ֱ��ָ��Socket��Ϊ1
    INT8U	NW_CMD = 0;
	INT8U	LTE_Sending_Flag = UNDONE;
	
	TaskActive &= LTE_INACT;													//������Ѳ������������Ź�
	OSTaskSuspend(OS_PRIO_SELF);												//������������
	TaskActive |= LTE_ACT;       												//����ָ���������Ѳ�������Ź�
	
	B485_init(38400);															//485��ʼ��������38400����LOCAL�������ʱ��ǿ��ִ�У��������B485DIS�궨���ж��Ƿ�ִ�У�
	
	while(1)
	{
	/*��Դ����*/
		if(Reset_Count>2) BAT_CTL_PIN_H();										//���临λ��������2��ʱ��ֱ��ǿ�ƴ�﮵��
		
	/*���ܹ���ģʽ�ж�*/
		while(Equipment_state.BAT_Volt<BAT_UNDER && Equipment_state.FALA_Volt<FALA_UNDER)	//BAT<9.2V��FALA<5V
		{
			BspUartWrite(2,SIZE_OF("����ģʽ�С���\r\n"));
			OSTimeDly(3*60*20);													//3min��ѯ
			Get_Voltage_MCUtemp_Data( 3 );										//��ȡ��ص�ѹ���ݺ͵�Ƭ���¶�
		}
		
	/*LTEģ�鿪��*/
		BspUartWrite(2,SIZE_OF("----------------------------------LTEģ�鿪��������\r\n"));OSTimeDly(1);
		ME909SInit(ME909SBaudrate);
		for(retry=0;retry<3;retry++)											//�������3��
		{
		/*LTEģ�����*/
			if(!ME909S_ON())													//ģ�鿪���������ô���
			{
				ME909S_PW_OFF();												//�ص�Դ
				OSTimeDly(3*20);
				continue;														//ʧ��ʱ���ԣ�����������䣩
			}
//			if(!ME909S_Contact()) continue;										//����������ʧ�������¿���		//����ʧ�ܵĻ������ﻹ���Գ䵱10�����ʱ
			if(!ME909S_SMS_CFG()) continue; 									//��������			
			if(!ME909S_CONFIG()) continue;     						 			//ME909S���б�������
			if(!ME909S_REG()) continue;											//����ע��
			if(!ME909S_Link(SOCKID)) continue;									//���Ӳ���͸��
			
		/*����ʱ���ڴ�ѭ�������˳����ߺ�����ʧ�ܡ���ԴǷѹʱ����*/
			while(1)															//�������߽����ص�����ͨ������
			{
			/*װ�������С���*/
				BspUartWrite(2,SIZE_OF("----------------------------------�ѽ�������״̬\r\n"));OSTimeDly(1);	
				BSP_InitFm(LTE_Num);											//��ʼ������洢

				LTE_Sending_Flag = UNDONE;
				BSP_WriteDataToFm(LTE_Sending_Flag_Addr,&LTE_Sending_Flag,1);   //д������
				Feed_Dog();	
				NW_CMD = NW_Comm_Process();										//��������Э���д��ͨ�����̣�װ��waking��
				if(NW_CMD)														//LTE����δʧ��
				{
					Reset_Count=0;												//�˱�־���ַ��ͳɰ�
					BSP_WriteDataToFm(Reset_Count_Addr,&Reset_Count,1); 		//ÿ�η�����ɺ�reset������գ��ۼ����θ�λ��ǿ�ƴ򿪵��						
				}
				LTE_Sending_Flag = DONE;										//LTE���ͱ�־���˱�־ �� ���ַ��ͳɰ�
				BSP_WriteDataToFm(LTE_Sending_Flag_Addr,&LTE_Sending_Flag,1);   //д������

				BspUartWrite(2,SIZE_OF("\r\n----------------------------------���˳�����״̬\r\n"));OSTimeDly(1);
				
			/*���ܹ���ģʽ�ж�*/
				if(Equipment_state.BAT_Volt<BAT_UNDER && Equipment_state.FALA_Volt<FALA_UNDER)	//BAT<9.2V��FALA<5V
				{
					BspUartWrite(2,SIZE_OF("---------->>��ԴǷѹ��������ܹ���ģʽ<<----------\r\n"));OSTimeDly(1);
					retry = 3;													//��������forѭ��
					break;														//�˳�whileѭ��
				}
				
			/*װ�������С���*/
				Sleep_Or_Reset(NW_CMD);											//ִ�з��ص����߻�����ָ�װ��sleeping���е͹��Ĵ���
				
			/*������������͸��*/
				if(!ME909S_Link(SOCKID)) 	break ;								//�˳����ߺ������������������͸��������ʧ�������¿���								
			}
		}
		
	/*LTEģ��ػ�*/	
		BspUartWrite(2,SIZE_OF("----------------------------------LTEģ��ػ�\r\n"));OSTimeDly(1);
		ME909S_OFF();			  												//�ػ�
		ME909S_LowPower();														//�͹���
	}
}/*end of Task_LTE_Main()*/





/*******************************************************************************
���ƣ�void Sleep_Or_Reset(u8 CMD)
���ܣ����ݴ����ָ��ѡ��ִ�����߻���������
��Σ�u8 CMD�������ָ��
���Σ���
���أ���
*******************************************************************************/
void Sleep_Or_Reset(u8 CMD)
{	
	static INT8U	msg_state;
	static INT8U	msg_cmd;
	INT16U			SMS_Len=0;
	INT8U			Err=0;
	INT8U*			R=NULL;
	
	ME909S_Trans_OFF();															//���ص�����ģʽ,���и�λ����ŵĵȴ������õ�ATָ��
	ME909S_IPCLOSE(7);															//�ر���������
	ME909S_SMS_Delete(1,4);														//����ǰ������ж��ű����ܵ�����
	
	/*װ������*/
	if(CMD==RESET_DEV) 
	{
		BspUartWrite(2,SIZE_OF("----------------------------------װ������������\r\n"));OSTimeDly(1);
		DeviceRstOnCMD();														//��������
	}
	
	/*װ�ý�������״̬���ɹ�ʱ��Dev_STAB0X��Ϣ*/
	BspUartWrite(2,SIZE_OF("----------------------------------�ѽ�������״̬\r\n"));OSTimeDly(1);
	
	/*����͹���*/		
	FM_LowPower(LTE_Num);
	BAT_CTL_PIN_L();            												//�ر�ǿ�ƴ򿪵�Դ���    ����������û��ʱ���������Ҳ�������ʲôӰ�죬�����Դ�������žͻᴦ�ڵ͵�ƽ

	msg_state = SLEEP_SUCCESS;
	OSMboxPost(Dev_STAB0X, &msg_state);											//֪ͨ���Ź������еļ�ʱ������ʼ���߼�ʱ

	while(1)
	{
	/*�ȴ���ʱ����50ms*/
		msg_cmd = *(INT8U *)OSMboxPend(Dev_CMDB0X,1,&Err);						//��ʱ����ѯ���䣨�����ԭ��Ϣ��������ҪPOST��
		if((Err==OS_NO_ERR) && (msg_cmd==WAKE_CMD)) break;						//������ʱ����ʱ���յ��������������
		
	/*�������߲��ȴ����Ż���5s*/
		SMS_Len=ME909S_Waitfor_SMS(LTE_Rx_Buff,5);								//�ȴ�����5�루LTE���������κ����飬���ȴ����Ż����������ߣ�	
		if(SMS_Len!=0) 															//δ�յ����ţ������ȴ�
		{
			R=ME909S_SMS_Extract(LTE_Rx_Buff,SMS_Len,(INT8U*)&SMS_Len);	
			if((R)&&(SMS_Judge(R,SMS_Len))) break;								//ȷ��Ϊ���Ѷ���������
		}
		WDTClear(LTE_PRIO);														//��������Ź�
	}
	
/*�ѱ�����*/
	msg_state = WAKE_SUCCESS;
	OSMboxPost(Dev_STAB0X, &msg_state);											//֪ͨ���Ź������еļ�ʱ������ʼ���Ѽ�ʱ
	BspUartWrite(2,SIZE_OF("----------------------------------���˳�����״̬\r\n"));OSTimeDly(1);
}
