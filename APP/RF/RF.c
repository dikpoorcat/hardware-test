/***************************** (C) COPYRIGHT 2019 ���ϵ��� *****************************
* File Name          : RF.c
* Author             : ��ӱ�ɡ���������
* Version            : ����ʷ�汾��Ϣ
* Date               : 2019/03/7
* Description        : RFͨ�ż����ݴ�����ʵ�֣���Ҫ���ô洢�����ڵ������ļ���
************************************  ��ʷ�汾��Ϣ  ************************************
* 2019/03/28     : V4.1.0
* Description   : ����������Ŀ���档����������ɣ������С�
*******************************************************************************/
#include "RF.h"

/*ȫ�ֱ�������*/
INT8U 					TT_Sample[55][2]={0};									//55��̽ͷÿ�β���������,�ڴ�λ��
INT8U					TT_RF_Fault_Count[55]={0};								//55��̽ͷ��Ƶ���մ���ֱ����
INT8U					FATFS_Lock=0;
INT8U					RF_Uart_RxBuff[RF_BuffLen] = {0};						//RF���ڽ��ջ���
INT8U					RF_Uart_TxBuff[RF_BuffLen] = {0};						//RF���ڷ��ͻ���
struct TT_STR			TT_Info={0};											//�����¼���̽ͷ��Ϣ
struct SAMP_MANAGE		TT_Sample_Manage={0}; 									//ÿ��Сʱ���¶Ȳɼ�����ṹ��
struct LteWakeupEnable 	wakeup_en={true,true,true,true};						//��ͨ�Ź��ϡ������硢��ԴǷѹ������½�ֹ������ģʽ���ѡ�trueʹ�ܣ�false��ֹ



/*����δ����*/
INT16U HT_Data[2] = {0};        												// ������ʪ�����ݣ�ʪ�ȣ��¶�=================�����𣿣���û�������
OS_EVENT  * RFSGIN = (OS_EVENT *)0;

/*�ڲ���������*/
INT8U RF_Data_Judge(struct Str_Msg * pMsg);
INT8U RF_Received_Data_DealWith(struct Str_Msg * pMsg);

/*******************************************************************************
* Function Name : void Task_LORA_Main(void *arg)                                                    
* Description   : RF��������RFͨ�ż����ݴ�����ʵ�֡�
*******************************************************************************/
void Task_RF_Main(void *arg)
{
	static INT8U	msg_fault = 0;
	static INT8U	msg_cmd=0;
	memset(TT_Sample,0xFF,sizeof(TT_Sample));									//���¶����ݳ�ʼ��Ϊ0xFFFF������Э��涨��
	
	TaskActive &= RF_INACT;														//������Ѳ������������Ź�
	OSTaskSuspend(OS_PRIO_SELF);												//������������
	TaskActive |= RF_ACT;       												//����ָ���������Ѳ�������Ź�
	
	B485_init(38400);															//485��ʼ��������38400������B485DIS�궨���Զ��жϣ���LOCAL�������ʱ��ǿ�ƴ򿪣�
	RF_Power_Init();       														//RFģ���Դ��ʼ��
	PWRFEN();				 													//�򿪵�Դ��������
						
	while(1)
	{
	/*�����Ź�ά������ʱ*/
		WDTClear(RF_Task_Prio);
		OSTimeDly(20);
	
		/*RF��������*/
		if(Time_Proofread==DONE)												//�������Уʱ
		{	
			BSP_InitFm(RF_Num);													//���õ͹��ĺ�������Ҫ���³�ʼ��
			RF_Data_Sample(30*20);												//�ȴ��ɼ���������30�룬���洢�ɼ�������
			RF_Receive_Data(1200,50);											//��RFģ���ȡ���ݣ�1200�����ʣ�50ʱ��Ƭ��ʱ����Ƶģ�鴮������256�ֽڣ�1200������ʱԼ2.13�룩
			FM_LowPower(RF_Num);												//�������ŵ͹�������		
			
			/*������ѯ�������Ϸ����ɼ�ʱ�ϱ�*/
			if((wakeup_en.overtime & wakeup_en.reply & wakeup_en.network & wakeup_en.battle & wakeup_en.rf_tem)&&(Fault_Manage.Need_Report==0x55))	//�����������ģʽ���ѣ��Ҵ��ڹ��ϡ�������Ϣ�ϱ����ڴ˴��ɴ�̽ͷ�����жϵĹ��̣����ܴ��ڶ��̽ͷͬʱ���ϣ���ͬһ���������ϱ���	�������ڿ��Ź������ԭ������Ϊ�п��ܴ�����
			{
				msg_cmd = WAKE_CMD;												//֪ͨLTE�����л�������״̬		
				OSMboxPost(Dev_CMDB0X, &msg_cmd);								//��������		��ʹ����״̬�յ�Ҳ����ı��κζ���������˯��״̬���ѽ��й����ϱ�
				msg_fault = FAULT_CMD;											//���ڷ�����֪ͨLTE�����л���������Ϣ�ϱ�״̬
				OSMboxPost(Fault_CMDB0X, &msg_fault);							//ͨ�����佫������Ϣ����
			}
		}
	}
}

/*******************************************************************************
���ƣ�void RF_Data_Sample(u16 timeout)
����: ���ղ���������������ݴ洢������ ��
��Σ�u16 timeout�����䳬ʱ�ȴ�ʱ����
���Σ���
���أ���
*******************************************************************************/
void RF_Data_Sample(u16 timeout)
{
	INT8U	msg_data;
	INT8U  	Err=0;
	INT32U  time=0;																					//����ʱ��	
	INT8U 	i=0;
	TCHAR   BUFF[50]={0};
	INT16U  TT_Data_Addr=0;																			//̽ͷ���������������еĴ�ŵ�ַ
	
/*�ж����㣬ִ����ʷ����������*/
	time = RtcGetTimeSecond();
	if((time/3600)!=(TT_Sample_Manage.Time[0]/3600) && TT_Sample_Manage.Time[0])					//�Ѿ����¸�Сʱ�ˣ�   (���вɼ���ʱ��)
	{	
		History_Data_Store();																		//���ϸ�Сʱ���ݴ洢��Э�����ϱ���վ�����ݶ�����������
		memset((INT8U*)&TT_Sample_Manage,0,Sample_Manage_Len);										//����ղɼ�����ṹ��
		TT_Sample_Manage.TT_Count=TT_Info.TT_Count;													//���̽ͷ����
		memcpy(TT_Sample_Manage.Newline,SIZE_OF("\r\n"));											//�������
		BSP_WriteDataToFm(Sample_Manage_Addr,(INT8U *)&TT_Sample_Manage.Len,Sample_Manage_Len);		//д����ṹ�嵽����
	}
	
/*���յ����ݲɼ�ָ�����ǰ�ڴ�����д������*/
	msg_data = *(INT8U *)OSMboxPend(Data_CMDB0X,timeout,&Err);										//��ѯ����
	if((Err==OS_NO_ERR)&&(msg_data==DATA_CMD))														//�ӵ��ɼ�����ָ��
	{	
		if(!TT_Sample_Manage.TT_Count||TT_Sample_Manage.Sample_Num>=60)	return;						//����̽ͷ¼�룬��ֱ�Ӳ��������ݲɼ�������Сʱ�ɼ������ѳ���60�����ٲɼ�
		for(i=0;i<55;i++)																			//������д������
		{			
			if(TT_Info.HaveTT[i]==0x55) 															//ֻҪ����̽ͷ����Ҫ�������ݲ�д�����磬��Ч������FFFF���档������ʱ����Read_TT_From_FM�н��¶����ݳ�ʼ��Ϊ0xFFFF��
			{	
				/*�����жϣ����޸��ڴ�����*/
				RF_Fault_Judge(i);																	//�ж�̽ͷ�Ƿ�������ˣ������ݴ������Ƶ���ղ������ݣ�������������Э�飬����Ч�������ΪFFFF��
				
				/*�ڴ�����д������*/
				TT_Data_Addr=Sample_Data_Addr+One_TT_Sample_Data_Len*i+TT_Sample_Manage.Sample_Num*2;
				BSP_WriteDataToFm(TT_Data_Addr,&TT_Sample[i][0],2);									//�����¶�д�����硣����֮��Ž���Sample_Num++�������0λ�ÿ�ʼ	
				
				/*����ΪĬ��ֵ*/
				TT_Sample[i][0]=0xFF;																//д��֮��ͽ����ݳ�ʼ��ΪFFFF������Э��4.3��������Ч������FFH��ʾ��
				TT_Sample[i][1]=0xFF;	
			}				
		}
		TT_Sample_Manage.Time[TT_Sample_Manage.Sample_Num] = time;									//�ɼ�ʱ������
		TT_Sample_Manage.Sample_Num++;																//��Сʱ̽ͷ�ɼ���+1		
		BSP_WriteDataToFm(Sample_Manage_Addr,(INT8U *)&TT_Sample_Manage.Len,Sample_Manage_Len);		//д����ṹ�嵽����
																				
		sprintf(BUFF, "��Сʱ�Ѳɼ�%d��\r\n",TT_Sample_Manage.Sample_Num);
		BspUartWrite(2,(INT8U*)BUFF,strlen(BUFF));
	}
}

/*******************************************************************************
* Function Name : void RF_Fault_Judge(INT8U Index)
* Description   : �ж�̽ͷ�Ƿ�������ˣ������ݴ������Ƶ���ղ������ݣ�������¼��Fault_Manage��Ӧ�ı�־λ�С�����������Э�飬����Ч�������ΪFFFF��
* Input         : Index:̽ͷ������
*
* Return        : None
*******************************************************************************/
void RF_Fault_Judge(INT8U Index)
{	
	INT16U Temp=0;
	TCHAR BUFF[50] = {0};
	
	/*RFδ���յ�����*/
	if( (TT_Sample[Index][0]&TT_Sample[Index][1])==0xFF )											//�յ�FFFF������Ƶδ���յ����ݣ���Ƶ����01H
	{
		if(TT_RF_Fault_Count[Index]<5) TT_RF_Fault_Count[Index]++;									
		if((TT_RF_Fault_Count[Index]==5) && (!Fault_Manage.F_RF[Index]))							//����δ�յ����ݴ�5�Σ����ϱ�����
			NW_Fault_Manage(Index, FAULT_STA);														//��ǹ��Ϸ�����δ�ָ������ô˺�����Ҫ����Need_Report����Task_RF_Main�б���ѯ�����Ӷ����������ϱ�����
		
		#ifdef RF_Test				
		sprintf(BUFF, "�� %d ��̽ͷ��%X��δ���յ����ݣ�\r\n",Index+1,Unit_ID_Code[Index]);			//��1��ʼ����
		BspUartWrite(2,(INT8U*)BUFF,strlen(BUFF));
		#endif
	}
	
	/*���յ�����*/
	else
	{		
		TT_RF_Fault_Count[Index]=0;																	//�յ��������ݼ�����
		if(Fault_Manage.F_RF[Index])																//����ʱ������Ƶδ���յ����ݹ���
			NW_Fault_Manage(Index, NOFAULT_STA);													//��Ƶ���չ��ϻָ���	01H
		
		Temp=(TT_Sample[Index][0]<<8)+TT_Sample[Index][1];
		#if RF_Test
		sprintf(BUFF, "�� %d ��̽ͷ�¶ȣ� %d.%d\r\n",Index+1,Temp/10,Temp%10);						//��1��ʼ����
		BspUartWrite(2,(INT8U*)BUFF,strlen(BUFF));
		#endif
		
		/*��̽ͷ�¶������쳣*/
		if(((0xf800>Temp)&&(Temp>Tem_Upper)) || (Temp>Tem_Lower))									//���¶��쳣�������̻��д�����룩
		{
			/*Temp-3984������4000->0x10����ʾδ���յ����ݣ��������̽ͷ����������<=-55��>=125�ȳ�����ʱ�����������ݣ���������*/
			if((4000>Temp) || (Temp>4019)) Temp=3986;												//�����յ������쳣ֵ����4000~4020��ģ�1250~F800��FA26~FFFF������Ϊ���¶��쳣��ʹ���������¶��쳣���ϱ��롱0x02��3986-3984��
			NW_Fault_Manage(Index+55, Temp - 3984);													//����¶��쳣���Ϸ�����Index+55����������� 02H�¶��쳣�Ĵ���	ԭ��NW_Fault_Manage(Index+55, FAULT_STA);	

			/*ע�⣡�޸��ڴ��е���Ч�¶�����*/
			TT_Sample[Index][0] = 0xFF;																//��������Э�飬��Ч�������ΪFFFF
			TT_Sample[Index][1] = 0xFF;
		}
		/*��̽ͷ�¶���������*/
		else
		{
			if(Fault_Manage.F_TEM[Index])															//��Ҫ�¶��쳣���Ͻ��
				NW_Fault_Manage(Index+55, NOFAULT_STA);												//�¶��쳣���ϻָ���ֱ��д��0x80
		}
	}
}


/*******************************************************************************
* Function Name : void History_Data_Store(void)
* Description   : ���ղ���������������ݴ洢��FLASH�ļ��С�
* Input         : None
*
* Return        : None
*******************************************************************************/
void History_Data_Store(void)
{
	INT8U 				i=0;
	struct NW_TIME		time = {0};												//������ʱ����
	INT8U 				Store_Buff[60*2+1+2+2]={0};								//���Ϊÿ��Сʱ60������+1��̽ͷ������+2(CRC)+2��0d0a��
	INT16U 				crc;																		
	struct SAMP_MANAGE	TT_Store_Manage={0};
	TCHAR 				BUFF[100]={0};
	INT16U				TT_Data_Addr=0;											//̽ͷ���������������еĴ�ŵ�ַ
	INT8U  				Hour_Byte=0;											//ָ��Unreport_Index�ļ������� СʱByte
	INT8U				Hour_Bit=0;												//ָ��Unreport_Index�ļ������� Сʱbit
	TCHAR 				File_Name[20]={0};
	UINT 				bw;
	INT8U				Head[2]={0XFF,0XAA};									//�ļ�ͷ FF AA
	INT8U   			Tail[2]={0XFF,0XBB};									//�ļ�ͷ FF BB
		
	if(!TT_Sample_Manage.Sample_Num) return;									//δ�ɼ����ݣ�ֱ���˳����������ļ�
	if(!SecondToNwTime(TT_Sample_Manage.Time[0],&time)) 						//����ʱ���ʽ���󣬲����д洢
	{																								
		BspUartWrite(2,SIZE_OF("ʱ������޷������ļ�----------------------------------\r\n"));OSTimeDly(2);
		return;
	}
	
	sprintf(File_Name,"SUB%d/%02d%02d",time.mday,time.mday,time.hour);								//�����ļ���
	while(FATFS_Lock)	OSTimeDly(20);
	FATFS_Lock=1;
	BspUartWrite(2,(INT8U*)File_Name,strlen(File_Name));	
	BspUartWrite(2,SIZE_OF("�ļ���ʼд��\r\n"));

	/*�򿪶�Ӧ����Ŀ¼�������ļ���д���ļ�*/
	for(i=0;i<3;i++)
	{
		if (f_mount(&fs, "", 1))continue;													    	//����
		if (f_open(&fil,File_Name, FA_CREATE_ALWAYS | FA_READ | FA_WRITE ))continue;				//�����������ļ����Ծ���·�������������Ե�ǰʱ�䣨��ʱ������

		
		f_write(&fil, Head,2, &bw);																	//д�ļ�ͷ FF AA
		/*�������н�һ��Сʱ�����ݶ�ȡ������д�뵽W25Q256*/
		BSP_ReadDataFromFm(Sample_Manage_Addr,(INT8U *)&TT_Store_Manage,Sample_Manage_Len);
		TT_Store_Manage.Len=TT_Store_Manage.Sample_Num*2+1+2+2;										//ÿ�����ݵĳ���  ��������*2+1��̽ͷ������++2(CRC)+2��0D0A��
		TT_Store_Manage.crc=RTU_CRC((INT8U *)&TT_Store_Manage,Sample_Manage_Len-2-2);				//����CRC�����ڶ�дУ�� ��У�鳤�ȣ��ܳ���-2��0D0A��-2��crc��	
		memcpy(TT_Store_Manage.Newline,SIZE_OF("\r\n"));
		f_write(&fil, (INT8U *)&TT_Store_Manage,Sample_Manage_Len, &bw);							//д��ָ���Զ�ָ����һλ				��Ҫ����������һ�£���
		
		
		for(i=0;i<55;i++)																			//̽ͷ��������д��W25Q256
		{
			if(TT_Info.HaveTT[i]==0x55)
			{
				Store_Buff[0]=Unit_ID_Code[i];														//����̽ͷ��Ӧ�Ĺ��ܵ�Ԫ��
				TT_Data_Addr=Sample_Data_Addr+One_TT_Sample_Data_Len*i;
				BSP_ReadDataFromFm(TT_Data_Addr,&Store_Buff[1],TT_Store_Manage.Sample_Num*2);		//����̽ͷһ��Сʱ�Ĳ�������
				crc=RTU_CRC(Store_Buff,TT_Store_Manage.Len-2-2);									//-2��0D0A��-2��crc��
				Store_Buff[TT_Store_Manage.Len-2-2]=(crc>>8)&0xff;
				Store_Buff[TT_Store_Manage.Len-1-2]=crc&0xff;
				memcpy(&Store_Buff[TT_Store_Manage.Len-2],SIZE_OF("\r\n"));							//����β��0D0A
				f_write(&fil,Store_Buff,TT_Store_Manage.Len, &bw);									//д��ָ���Զ�ָ����һλ
				
				memset(Store_Buff,0,sizeof(Store_Buff));	
			}				
		}
		f_write(&fil, Tail,2, &bw);																	//д�ļ�β FF BB	

		Hour_Byte = time.hour/8;																				
		Hour_Bit  = time.hour%8;
		Unreport_Index[time.mday-1][Hour_Byte]&=~(0x80>>Hour_Bit);									//�����ļ���Ӧ�ļ��������б�־λд0��������ļ�δ�ϱ�
		BSP_WriteDataToFm(Unreport_Index_Addr,(INT8U*)Unreport_Index,Unreport_Index_Len);			//δ�ϱ�����������д������
		
#ifdef RF_Test	
		BspUartWrite(2,(INT8U*)File_Name,strlen(File_Name));
		BspUartWrite(2,SIZE_OF("�ļ�д�����\r\n"));OSTimeDly(2);	
		sprintf(BUFF, "��%d��̽ͷ����ɲɼ�%d��\r\n",TT_Store_Manage.TT_Count,TT_Store_Manage.Sample_Num);			
		BspUartWrite(2,(INT8U*)BUFF,strlen(BUFF));OSTimeDly(2);
		memset(BUFF,0,sizeof(BUFF));
		/*����д��Ч���Ĳ��֣�(�ٿ���ռ���бȽ��������ݻ��ڴ����)ֻ����CRCУ��*/
		memset(&TT_Store_Manage,0,Sample_Manage_Len);
		f_lseek(&fil,2);
		f_read(&fil,&TT_Store_Manage,Sample_Manage_Len,&bw);
		if(TT_Store_Manage.crc!=RTU_CRC((INT8U *)&TT_Store_Manage,Sample_Manage_Len-2-2))			
			BspUartWrite(2,SIZE_OF("�ļ��ɼ���������У�����\r\n"));OSTimeDly(2);	
		
		for(i=0;i<TT_Store_Manage.TT_Count;i++)
		{
			f_read(&fil,Store_Buff,TT_Store_Manage.Len,&bw);
			crc=(Store_Buff[TT_Store_Manage.Len-2-2]<<8)+Store_Buff[TT_Store_Manage.Len-1-2];
			if(crc!=RTU_CRC(Store_Buff,TT_Store_Manage.Len-2-2))
				sprintf(BUFF, "�ļ��ɼ��¶����ݵ�%dУ�����\r\n", i);	
				BspUartWrite(2,(INT8U*)BUFF,strlen(BUFF));OSTimeDly(2);
				memset(BUFF,0,sizeof(BUFF));			
			memset(Store_Buff,0,sizeof(Store_Buff));			
		}
		
		BspUartWrite(2,SIZE_OF("�ļ�����У�����\r\n"));OSTimeDly(5);
#endif			
		f_close(&fil);																				//���ļ�
		f_mount(0, "", 0);																			//ж��
		break;
	}
	FATFS_Lock=0;
}

/*******************************************************************************
���ƣ�void RF_Receive_Data(u32 rate, u16 timeout)
����: ��ȡRF���հ��е��¶����ݽ��н����������Ӧ�����ݽṹ���С�
��Σ�u32 rate������ͨ�Ų����ʣ�u16 timeout����ʱ�ȴ�ʱ����
���Σ���
���أ���
*******************************************************************************/
void RF_Receive_Data(u32 rate, u16 timeout)
{
	INT8U RFCmd[] = RF_READ_CMD;
	INT8U RFCmdLen = sizeof(RFCmd);
	INT8U Err =0;
	struct Str_Msg *pRfMsg = (struct Str_Msg *)0; 
				
	RF_Uart_init(rate);																				//��ʼ��RF����
	memset(RF_Uart_RxBuff,0x00,RF_BuffLen);															//��մ��ڽ��ջ���
	BspUartWrite(1,RFCmd,RFCmdLen);    																//���ͻ�ȡ����ָ��
	StopModeLock++;
	pRfMsg = (struct Str_Msg *)OSMboxPend(RFSGIN,timeout,&Err);   									//�ȴ�������Ϣ timeout��ʱ��Ƭ����Ƶģ�鴮������256�ֽڣ�1200������ʱԼ2.13�룩
	if(StopModeLock) StopModeLock--;

	if(Err==OS_NO_ERR && pRfMsg)
	{
		if( pRfMsg->MsgID == BSP_MSGID_RFDataIn )													//�ж������е���Ϣ�Ƿ�ΪRF���ڸ���
		{
			 if(pRfMsg->DataLen <= 256)																//�����ݲ���������256�ֽڣ�����Ƶģ�������
			 {
				 if(RF_Data_Judge(pRfMsg))															//��Э��У��ͨ��
				 {						
					if(pRfMsg->pData[2]==0x01) 														//��CMD=0X01����ʾ��ȡ���հ�Ĳ�������
						RF_Received_Data_DealWith(pRfMsg);											//���������뵽̽ͷ���ݽṹ�嵱��
				 }
			 }
		}						
	}
	BSP_UART_RxClear(pRfMsg->DivNum);																//����Ϣ����
	RF_LowPower();																					//������RF��ʼ�����ʹ��	
}


/******************************************************************************
* Function Name: void RF_Power_Init(void)
* Description:   RF ��Դ��������								 
* Input:  Nothing
* Output: Nothing
******************************************************************************/
void RF_Power_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(PWRF_Port_CK,ENABLE);													//��ʱ��
	PWRFDIS();																						//����Ϊ�ر�	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =PWRF_PIN;
	GPIO_Init(PWRF_Port, &GPIO_InitStructure);	
}

/*******************************************************************************
* Function Name : void RF_Uart_init(unsigned int rate)
* Description   : Rf���մ��ڳ�ʼ��
* Input         : rate : ������
*
* Return        : None
*******************************************************************************/
void RF_Uart_init(unsigned int rate)
{
	UARTx_Setting_Struct UARTInit = {0};	
	
	if(RFSGIN == NULL) RFSGIN = OSMboxCreate(0);
	else RFSGIN->OSEventPtr= (void *)0;																//����Ϣ���䣬����ᵼ������ ZE
		
	UARTInit.BaudRate = rate;
	UARTInit.Parity   = BSPUART_PARITY_NO;
	UARTInit.StopBits = BSPUART_STOPBITS_1;
	UARTInit.DataBits = 8;
	UARTInit.RxBuf    = RF_Uart_RxBuff;			   
	UARTInit.RxBufLen = RF_BuffLen;     
	UARTInit.TxBuf    = RF_Uart_TxBuff;
	UARTInit.TxBufLen = RF_BuffLen;     
	UARTInit.Mode     = UART_DEFAULT_MODE;	  														//��ͨ����
	
	BSP_UART_Init(1,&UARTInit,RFSGIN);
}

/******************************************************************************* 
* Function Name  : void RF_LowPower(void)
* Description    : RF����͹��ģ�������ӦIO�����͹��Ĵ���
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/
void RF_LowPower(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1 ,DISABLE);										//�ر�U1ʱ�Ӽ�����ʱ�ӣ�ע�⣺ADCҲ�õ������ | RCC_APB2Periph_AFIO	//���ظ���ʱ��|RCC_APB2Periph_AFIO�������ں�AD���õ������
	
	/*��Դʹ�ܡ�TX��RXģ������*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;													//ģ������
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;														///PA9/U1_TX
	GPIO_Init(GPIOA, &GPIO_InitStructure);										
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;														//PA10/U1_RX
	GPIO_Init(GPIOA, &GPIO_InitStructure);										
	
	USART_ITConfig( USART1, USART_IT_TC, DISABLE);
	USART_ITConfig( USART1, USART_IT_RXNE, DISABLE);
	USART_Cmd( USART1,DISABLE);		
}

/*******************************************************************************
* Function Name : INT8U RF_Data_Judge( struct Str_Msg * pMsg)
* Description   : ��Ƶͷ������������Ч���ж�
* Input         : 
* Return        : 
*******************************************************************************/
INT8U RF_Data_Judge(struct Str_Msg * pMsg)
{
	INT8U i = 0;
	INT8U Length = 0;
	INT8U CS = 0;
	
	if(pMsg->DivNum!=1) return 0;
	if(pMsg->DataLen<11) return 0;
	
	Length = pMsg->pData[3] + 4;																	//�����򳤶�+AA 2D 01 DataLen
	for(i = 0;i < Length;i++) CS += pMsg->pData[i];													//�����ۼӺ�
	if(CS != pMsg->pData[Length]) return 0;															//�ۼӺ�У��
	
	return 1;
}

/*******************************************************************************
* Function Name : void RF_Data_Extract(struct Str_Msg * pMsg)
* Description   : ��Ƶͷ�ϴ��������ݵĽ�����������Ч���ݣ������󶨵�̽ͷ����Index������TT_Sample[Index]��
* Input         : pMsg:��Ϣ�ṹ��ָ��
*
* Return        : ��
*******************************************************************************/
INT8U RF_Received_Data_DealWith(struct Str_Msg * pMsg)
{
	INT8U 	i = 0;																					//AA 2B 01 DataLen CNT ����
	INT8U 	Cnt = pMsg->pData[4];																	//������ĵ�һ���ֽڣ���ʾ��Ч���µ����
	INT8U 	*Ptr = &pMsg->pData[5];																	//ָ���������У����µ����ݵ�ָ��
	INT8U	Index = 0;   
	
	if(!TT_Info.TT_Count) return 0;																	//û��¼��̽ͷ
	
	for(i=0; i<Cnt; i++)
	{	
		Index = CMP_TT_ID(Ptr);																		//�õ��ȶԳɹ���̽ͷ����
		if(Index != 0xff)	  																		//0XFF��ʾƥ��ʧ�ܣ�������̽ͷ����
		{
			memcpy(TT_Sample[Index],&Ptr[2],2);														//������������Ӧ̽ͷ���¶����룬��δ���´β����������ȡֵ���µ����ݻḲ�Ǿɵ�����			
		}
		Ptr += 4;  
	}
	return 1;
}
																				
/*******************************************************************************
* Function Name : INT8U CMP_TT_ID(INT8U* pIn)                                                     
* Description   : �ȶ��յ���̽ͷ�����Ƿ�����¼���̽ͷ��Χ��
* Input         : pIn ����������               
* Return        : ̽ͷ����0~54  �������ɶ�Ӧ�����ܵ�Ԫʶ���룩
				  0xff:����������Χ��
*******************************************************************************/
INT8U CMP_TT_ID(INT8U* pIn)
{
	INT8U i=0;
	for(i=0;i<55;i++)
	{
		if(!memcmp(pIn,&TT_Info.TT_ID[i][0],2)) return i;											//������ϣ��򷵻����������ݰ�����λ�ô��				
	}
	return 0xff;																					//δƥ������¼���̽ͷID
}

/*******************************************************************************
* Function Name : void Read_TT_From_FM(void);	                                                   
* Description   : ��¼���̽ͷ��Ϣ�������ж�ȡ����������ƥ���յ���̽ͷ�¶�,����˳�����̽ͷ�¶���Ϣ�ṹ���̽ͷID����,Ϊ����̽ͷ����������׼��
* Input         :              
* Return        : 		 
*******************************************************************************/
void Read_TT_From_FM(void)
{	
	INT8U i=0;
	
	BSP_ReadDataFromFm(TT_Count_Addr,&TT_Info.TT_Count,sizeof(TT_Info));							//�ϵ����¼���̽ͷ��Ϣ��TT_Info�ṹ��
	BSP_ReadDataFromFm(Sample_Manage_Addr,(u8*)&TT_Sample_Manage,Sample_Manage_Len);				//�ϵ�����ɼ�����ṹ��
	memcpy(TT_Sample_Manage.Newline,SIZE_OF("\r\n"));												//�ṹ���β����0x0D 0x0A
	TT_Sample_Manage.TT_Count=TT_Info.TT_Count;
	
	for(i=0;i<55;i++)
	{
		if((TT_Info.TT_ID[i][0]==0)&&(TT_Info.TT_ID[i][1]==0))TT_Info.HaveTT[i]=0;					//��Ǵ�λ����̽ͷ���������ֽڶ���0ʱ�ű��Ϊ�գ�
		else TT_Info.HaveTT[i]=0x55;
	}	
}	
