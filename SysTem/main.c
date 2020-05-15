/***************************** (C) COPYRIGHT 2019 ���ϵ��� *****************************
* File Name          : main.c
* Author             : �з���
* Version            : ����ʷ�汾��Ϣ
* Date               : 2019/02/21
* Description        : �����Ϸ����������·���߼��ͨ�Ź�Լ��V3.0��׼���ܴ��вɼ����߻�
��߱����¶ȣ��������ͨ��ͨ�����紫��������װ�û�״̬�����վ�������Զ��ɼ��¶ȹ��ܡ�
�߱��¶��ܿزɼ����ܣ�����ӦԶ��ָ����趨�ɼ���ʽ���Զ��ɼ�ʱ�䡢�ɼ�ʱ���������ɼ���
��ѭ����������30����¶�״̬���ݡ�
************************************  ��ʷ�汾��Ϣ  ************************************/
#include "main.h"




/*ȫ�ֱ���*/
unsigned int		OSInterrputSum;
INT8U				TaskActive=RF_ACT|LTE_ACT|Local_ACT;						//bit0\1\2�ֱ�ָʾ RF,LTE,Local�����Ƿ��ڼ���״̬������/ɾ������ʱ��Ӧλ��0���ϵ��ʼ��3��bit����1
INT32U				WDT[RWNUM] = {0};
INT8U				StopModeLock=0;												//�����ж��Ƿ�Ҫ����STOPģʽ
OS_STK				RF_STK[RF_STK_SIZE] = {0};
OS_STK				Local_STK[Local_STK_SIZE] = {0};
OS_STK				LTE_STK[LTE_STK_SIZE] = {0};
OS_STK				WDT_STK[WDT_STK_SIZE] = {0};

/*��̬ȫ�ֱ���*/
static INT32U		OldTime=0;													//��¼ÿ��״̬��ʼ��ʱ�䣨��һ���л�״̬��ʱ��㣩

/*UCOSII����*/
OS_EVENT			*Dev_CMDB0X = (OS_EVENT *)0;								//�豸�������䣬�����豸������Ϣ����
OS_EVENT			*Dev_STAB0X = (OS_EVENT *)0;								//�豸״̬���䣬�����豸״̬��Ϣ����
OS_EVENT			*Data_CMDB0X = (OS_EVENT *)0;								//�ɼ�ָ�����䣬���ڲɼ�ָ����Ϣ����
OS_EVENT			*Fault_CMDB0X = (OS_EVENT *)0;								//������Ϣ���䣬���ڹ�����Ϣ��Ϣ����

/*DEBUGģʽ�£�assert_failed()���ڼ�⴫�ݸ������Ĳ����Ƿ�����Ч�Ĳ��������Խ�������ʹ��Releaseģʽ���±��룬���������Ч��*/
void assert_failed(u8* file, u32 line)
{
	/* User can add his own implementation to report the file name and linenumber, 
	ex: printf("Wrong parameters value: file %s on line %d\r\n", file,line) */ 
	/* Infinite loop */
	while(1);
}





/*******************************************************************************
									main�������
*******************************************************************************/
int main(void)
{
	struct BSPRTC_TIME 	Reset_Time = {0x00,0x00,0x00,0x02,0x01,0x01,0x19}; 		//2019.01.01 �ܶ� 0ʱ0��0��
	OSInterrputSum = 0;	
	
/*ϵͳ����ǰ׼������*/
	NVIC_Configuration(); 														//�жϷ���	
	RCC_Configuration4M();														//ʱ������Ϊ4MHz
	IO_LowPower();																//IO�ڵ͹������ã�����Ҫ����ʱ������֮�����
	Power485Pin_Init();															//��485��Դ��ص���������
	RTC_Init(NRTC_Fre);															//��ʼ���ڲ�RTC�����ڵ͹��Ļ���
	BSP_WDGInit();																//�ⲿӲ�����Ź���ʼ����ʼ��
	BSP_WDGFeedDog(); 															//�ⲿӲ�����Ź� ι��������1.6S����	
	BSP_RX8025Init();															//RTCоƬ��ʼ��
	if(RtcGetChinaStdTimeStruct(&gRtcTime)==0)									//��ȡRTCʱ��ʧ��ʱ
	{
		RtcSetChinaStdTimeStruct(&Reset_Time);									//���ó�ʼֵ
		Time_Proofread = UNDONE;												//���ΪУʱδ���
	}
	SysJudgeAndMarkBkp();														//�жϵ�ǰ������SYS0����SYS1�������BKP->DR3
	
/*ϵͳ��ʼ��*/
	OSInit();																	//��ʼ��OS
	Tmr_TickInit(4000000);														//��ʼ��OS Tick

	// ����																		//����������ͻ�ɾ��
	OSTaskCreateExt(Task_Local_main, (void *)0, (OS_STK *)&Local_STK[Local_STK_SIZE - 1],
	Local_Task_Prio,
	Local_Task_Prio,
	(OS_STK *)&Local_STK[0],
	Local_STK_SIZE,
	(void *)0,
	OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);	
	
	// LTE
	OSTaskCreateExt(Task_LTE_Main, (void *)0, (OS_STK *)&LTE_STK[LTE_STK_SIZE - 1],
	LTE_Task_Prio,
	LTE_Task_Prio,
	(OS_STK *)&LTE_STK[0],
	LTE_STK_SIZE,
	(void *)0,
	OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);	
	
	// ����
	OSTaskCreateExt(Task_RF_Main, (void *)0, (OS_STK *)&RF_STK[RF_STK_SIZE - 1],
	RF_Task_Prio,
	RF_Task_Prio,
	(OS_STK *)&RF_STK[0],
	RF_STK_SIZE,
	(void *)0,
	OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

	// ���Ź�
	OSTaskCreateExt(Task_Wdt_main, (void *)0, (OS_STK *)&WDT_STK[WDT_STK_SIZE - 1],
	Wdt_Task_Prio,
	Wdt_Task_Prio,
	(OS_STK *)&WDT_STK[0],
	WDT_STK_SIZE,
	(void *)0,
	OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);	
	
	
/*ϵͳ����*/
	OSStart();																	//���������񻷾�
}





/*******************************************************************************
���ƣ�void WDTSscn(void)
���ܣ������Ź�ɨ�衣��Լ200���ӣ������Ź�������  
��Σ���
���Σ���
���أ���
*******************************************************************************/
void WDTSscn(void)
{
	unsigned char i;
	for(i=0;i<RWNUM;i++)														//RWNUM=3����ǰֻ����������������Local_ACT�Ⱥ궨�����ʹ�ã���������Ķ���
	{	
		if(TaskActive&(1<<i)){													//����������ڼ���״̬����ɨ��ʱ��Ӧ�������Ź���1
		WDT[i]++;																//wdtindex��4~6����ӦWDT[0~3]
		}
		if(WDT[i]>1500)															//Լ15.25����
		{
			McuSoftReset(); 													//ϵͳ�����λ��
		}
	}
}

/*******************************************************************************
���ƣ�void WDTClear(unsigned char wdtindex)
���ܣ������Ź�ι���������ɸ����������������
��Σ�wdtindex : 
			4 : Task_RF_Main
			5 : Task_LTE_Main
			6 : Task_Local_main 
���Σ���
���أ���
*******************************************************************************/
void WDTClear(unsigned char wdtindex)
{
	if( wdtindex>( RWNUM+3 ) ) return;
    WDT[wdtindex-4] = 0;														//wdtindex��4~6����ӦWDT[0~3]
}

/*******************************************************************************
���ƣ�void Task_Wdt_main(void *org)
���ܣ����Ź�����
��Σ���
���Σ���
���أ���
*******************************************************************************/
void Task_Wdt_main(void *org)
{
	INT8U				CS=0;
	INT8U				Dev_Stat=WAKE_STAT;										//��ʼ�趨Ϊ����״̬
	
	if(Dev_STAB0X == NULL) Dev_STAB0X = OSMboxCreate(0);						//�����豸״̬����
	else Dev_STAB0X->OSEventPtr= (void *)0;										//����Ϣ����
	if(Dev_CMDB0X == NULL) Dev_CMDB0X = OSMboxCreate(0);						//�����豸��������
	else Dev_CMDB0X->OSEventPtr= (void *)0;										//����Ϣ����
	if(Data_CMDB0X == NULL) Data_CMDB0X = OSMboxCreate(0);						//�����豸��������
	else Data_CMDB0X->OSEventPtr= (void *)0;									//����Ϣ����
	if(Fault_CMDB0X == NULL) Fault_CMDB0X = OSMboxCreate(0);					//����������Ϣ����
	else Fault_CMDB0X->OSEventPtr= (void *)0;									//����Ϣ����
	
	Led_Init();
	IWDG_Init(); 																//�ڲ��������Ź���ʼ��
	
	OldTime= RtcGetTimeSecond();
	while(1)
	{
		WDTSscn();   															//�����Ź�Ҫ��ÿ������ʱ�����(ά��)���Եļ�������
		Feed_Dog();
	
		CS++;
		if(CS%5==0)
		{
			Led_On();
			OSTimeDly(1);
		}
		else
		{
			Led_OFF();
			OSTimeDly(15);
		}
		if(CS%15==0)															//ÿ5������Ϊ61��OSTimeDly��15��Ϊ15/5*61��OSTimeDly=9.15s ��ѯһ��
		{			
			Dev_Stat = DevStatCtr(Dev_Stat);									//�豸״̬���ơ�����Э������������ʱ���ͻ���ʱ����		
			if(CS%(15*6)==0) 													//9.15*6=54.9s ��ѯһ��
			{
				RtcGetChinaStdTimeStruct(&gRtcTime);							//��RTC��ȡ��ǰʱ�䣬����gRtcTime
				ClearFlowDataDailyAndMonthly(&gRtcTime);						//ÿ��������������ͳ�ƣ�ÿ�³����������ͳ��
				Reset_On_Time(&gRtcTime);										//����������ѯ�����ڴ˴�����һ���ӵķ�����
				CheckSys2OperatingNormally(&gRtcTime);							//�ɹ�����24h����Ϊ����������SYS1���д�������
				CS=0;
			}
		}			
	}
}
																				
/*******************************************************************************
���ƣ�INT8U DevStatCtr(INT8U state)
���ܣ��豸״̬�Ŀ��ƣ������豸�������ߵļ�ʱ�����豸״̬��������иı�
��Σ�INT8U state
���Σ���
���أ����ص�ǰ������/���ѣ�״̬
*******************************************************************************/
INT8U DevStatCtr(INT8U state)
{
	static INT32U		DataTime=0;												//��¼ÿ�βɼ���ʼ��ʱ�䣨��һ�βɼ����ݵ�ʱ��㣩
	static INT8U		msg_data;
	static INT8U		msg_state;
	static INT8U		msg_cmd;
	INT32U				NewTime=0;												//��ǰʱ��
	INT32U				Interval_Time=0;										//���ʱ�䣬�����ж��Ƿ񳬹����߻�����ʱ�䣬�Ƿ�ı�״̬
	INT16U				Sample_Delay_Second=0;
	INT8U				Err=0;
	INT16U				ONLineTime=(INT16U)(Config.OnlineTime[0]<<8)+Config.OnlineTime[1];		//��ȡ����ʱ������
	INT16U				SleepTime =(INT16U)(Config.SleepTime[0]<<8)+Config.SleepTime[1];		//��ȡ����ʱ������
	INT16U				ScanInterval =(INT16U)(Config.ScanInterval[0]<<8)+Config.ScanInterval[1];//��ȡ����ʱ������(����)
	
	NewTime= RtcGetTimeSecond();		 										//��ȡ��ǰ��ʱ��	��λ����
	Interval_Time=(NewTime-OldTime)/60;											//���ʱ�䣬��λ����
	
	if(Net_Fault_Time && !Fault_Manage.F_NETWORK && (NewTime-Net_Fault_Time)/(30*60))			//�������ϼ�ʱ��ѯ������30�������ϱ��������� 0202H		�������Ϻ󣬼�ʱ�������������״̬�ѱ�ǣ��ٻָ�ǰ�����ٴβ�������
		NW_Fault_Manage(NETWORK_F, FAULT_STA);
	
	if(Host_No_Reply_Time && !Fault_Manage.F_REPLY && (NewTime-Host_No_Reply_Time)/(30*60))		//��վ��Ӧ����ϼ�ʱ��ѯ������30�������ϱ����� 0201H	ͬ��
		NW_Fault_Manage(REPLY_F, FAULT_STA);
	
	if(!DataTime)																//�״��ϵ�ʱ���˹��Ƴٲɼ�ʱ�䣨������Ƶ����һ�ᣩ
	{
		Sample_Delay_Second=Sample_Wait_Time>ScanInterval*60?ScanInterval*60:Sample_Wait_Time;	//��ѡС��ʱ�䣩������õ�Sample_Wait_Time>�ɼ����ʱ�䣬��ֱ���ӳٲɼ����ʱ�� ��λ����
		DataTime = NewTime-(ScanInterval*60-Sample_Delay_Second);				//����ʱSample_Wait_Time�������ɼ�ʱ��	��λ����
	}
	
	/*�ɼ�ʱ�䵽����ѯ����*/
	if((NewTime-DataTime)/60 >= ScanInterval )									//�����ɼ����
	{
		DataTime = NewTime;														//����ʱ��ʼʱ����Ϊ����
		msg_data = DATA_CMD;													//���ڷ�����֪ͨRF�����л����ɼ�����״̬
		OSMboxPost(Data_CMDB0X, &msg_data);										//ͨ�����佫�ɼ������
	}
	
	msg_state = *(INT8U *)OSMboxPend(Dev_STAB0X,1,&Err);						//��ѯװ��״̬���䣨�����ԭ��Ϣ��
	switch(state)
	{
		case WAKE_STAT:
				/*����������Ϣ����*/
					if((Err==OS_NO_ERR)&&(msg_state==SLEEP_SUCCESS))			//�ӵ����߳ɹ�֪ͨ
					{
						state = SLEEP_STAT;										//����
						OldTime = NewTime;										//����ʱ��ʼʱ����Ϊ����
					}
					else if((Err==OS_NO_ERR)&(msg_state==WAKE_SUCCESS))			//�������״̬�нӵ�����ָ���������ʼʱ��
					{
						OldTime=NewTime;
					}
					
				/*����ʱ�����ڣ�������������*/
					else if(Interval_Time>=ONLineTime)							//����ʱ������
					{
						if(update_start==true)									//�������������У���ֹLTE����
						{
							upgrade_timeout++;
							if(upgrade_timeout>33) 								//��������ѯÿ��9.15s�������5min��������
							{
								update_start = false;							//��ʱ���˳�����״̬������LTE����
								upgrade_timeout = 0;
							}
						}
						else													//��������״̬
						{
							msg_cmd = SLEEP_CMD;								//֪ͨLTE�����л�������״̬
							OSMboxPost(Dev_CMDB0X, &msg_cmd);					//��������							
						}
					}
					break;
					
		case SLEEP_STAT:	
				/*����������Ϣ����*/
					if((Err==OS_NO_ERR)&&(msg_state==WAKE_SUCCESS))				//�ӵ����ѳɹ�֪ͨ													
					{
						state = WAKE_STAT;										//����
						OldTime = NewTime;										//����ʱ��ʼʱ����Ϊ����	
					}
					else if((Err==OS_NO_ERR)&(msg_state==SLEEP_SUCCESS))		//�������״̬�нӵ�����ָ���������ʼʱ��
					{
						OldTime=NewTime;
					}
					
				/*����ʱ�����ڣ�������������*/
					else if(Interval_Time>=SleepTime)							//����ʱ������													
					{
						msg_cmd = WAKE_CMD;										//֪ͨLTE�����л�������״̬		
						OSMboxPost(Dev_CMDB0X, &msg_cmd);						//��������
					}
					break;
					
		default: 
					break;
	}
	return state;	
}	

/*******************************************************************************
���ƣ�void Reset_On_Time(struct BSPRTC_TIME *pTime)
���ܣ�����������ѯ
��Σ�HEXʱ��
���Σ���
���أ���
*******************************************************************************/
void Reset_On_Time(struct BSPRTC_TIME *pTime)
{
	INT8U				minute = 0, hour = 0, day = 0;
	
	minute = BcdToHex(pTime->Minute);											
	hour = BcdToHex(pTime->Hour);
	day = BcdToHex(pTime->Day);
	
	if((Config.ResetTime[1]==hour) && (Config.ResetTime[2]==minute))			//ʱ�ֶ�Ӧ����
	{
		if((!Config.ResetTime[0]) || (Config.ResetTime[0]==day))				//����������Ϊ����ÿ�춨������������ȶ����ڶ�Ӧ�����豸����
		{
			McuSoftReset();
		}
	}
}

/*******************************************************************************
���ƣ�void Feed_Dog(void )
���ܣ�ι��
��Σ���
���Σ���
���أ���
*******************************************************************************/
void Feed_Dog(void )
{
	IWDG_Reset();
	BSP_WDGFeedDog();
}

/*******************************************************************************
���ƣ�void IO_LowPower(void)
���ܣ�����IO�ڸ�λ������Ϊ�͹���״̬�� 
��Σ���
���Σ���
���أ���
*******************************************************************************/
void IO_LowPower(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	
	GPIO_DeInit(GPIOA);     												 	//��ᵼ������IO�ڶ���λ��IO�ڵ�ƽ�ı�
	GPIO_DeInit(GPIOB);
	GPIO_DeInit(GPIOC);
	GPIO_AFIODeInit();
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC , ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);										//����Ҫ�����Ŷ���ʼ��ΪAIN������Ҫ���𲽴�ʹ��
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/*******************************************************************************
���ƣ�void Led_Init(void)
���ܣ�Led�Ƴ�ʼ������ 
��Σ���
���Σ���
���أ���
*******************************************************************************/
void Led_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =Led_PIN;
	GPIO_Init(Led_Port, &GPIO_InitStructure);
}

/*******************************************************************************
���ƣ�void Led_On(void)
���ܣ�����Led�ơ� 
��Σ���
���Σ���
���أ���
*******************************************************************************/
void Led_On(void)
{
	 GPIO_SetBits(Led_Port,Led_PIN);
}

/*******************************************************************************
���ƣ�Led_OFF(void)
���ܣ�Ϩ��Led�ơ� 
��Σ���
���Σ���
���أ���
*******************************************************************************/
void Led_OFF(void)
{
	 GPIO_ResetBits(Led_Port,Led_PIN);
}

/*******************************************************************************
���ƣ�void SysJudgeAndMarkBkp(void)
���ܣ����ݳ������е�ַ���жϵ�ǰ������SYS0����SYS1�������BKP->DR3��
��Σ���
���Σ���
���أ���
*******************************************************************************/
void SysJudgeAndMarkBkp(void)
{
	static int(*fun_P)(void);
	INT32U	adress_now=0;
		
	/**�ж���APP1������APP2,����DR3�У�ָʾ��ǰ���г�����flash��λ��*/
	fun_P=&main;
	adress_now=(INT32U)fun_P;
	if((adress_now>=0x08006000)&&(adress_now<=0x08027000))
	{
		PWR->CR|=1<<8;																	//DBPλ��ȡ���������д������1������д��RTC�ͺ󱸼Ĵ���
		BKP->DR3=0x01;				
		PWR->CR&=~(1<<8);
	}
	else if((adress_now>=0x08027000)&&(adress_now<=0x08060000))
	{
		PWR->CR|=1<<8;																	//DBPλ��ȡ���������д������1������д��RTC�ͺ󱸼Ĵ���
		BKP->DR3=0x02;	
		PWR->CR&=~(1<<8);
	}	
}

/***************************** (C) COPYRIGHT 2019 ���ϵ��� *********END OF FILE**********/
