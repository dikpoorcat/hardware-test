#include "ME909S.h"


/*ȫ�ֱ���*/
OS_EVENT	*GPRSBOX = (OS_EVENT *)0;
struct Str_Msg 	*pGprsMess=(struct Str_Msg *)0;//GPRS ͳһ��Ϣ
INT8U UART3_Tx_Buff[LTE_BUFF_LEN] = {0};										//���ڴ���3����
INT8U UART3_Rx_Buff[LTE_BUFF_LEN] = {0};										//���ڴ���3����
INT8U ME909SCOMMANDID=0;														//0x40�����յ�������Ϣ���洢�� 0x10��������
GPRS_STRUCT GPRS = {0};
A58Control A58Cont;																// zzs add this for �ڴ���ͼƬ��ʱ�򣬱�ʶһ��GPRSģ�鴦��ʲô״̬����ʵ��ȫ�Ϳ����ñ�����ԭʼ�� A8500COMMANDID����ͼƬ����Э����ȫ��ͬ�ڱ����̵��������ϴ����̣����Ծͽ���ͼƬ���͵ĺܶණ����ģʽ��
																				// ��ҪĿ���ǲ����ͼƬ���͹��̵ĳ���ṹ��̫��ĸĶ���������������һ�׹��̿���״̬�����ˡ�=============================��ʱ�������






/******************************************************************************* 
* Function Name  : void ME909S_LowPower(void)
* Description    : ME909Sȫ��ͨģ���·�͹��ġ��ش���ʱ�ӣ����ÿ���IO�ڡ�
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/ 
void ME909S_LowPower(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;

//	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, DISABLE );						//���ظ���ʱ��|RCC_APB2Periph_AFIO�������ں�AD���õ������
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3, DISABLE );					//�ر�U3ʱ��

	/*GPRSEN��TX��RXģ������*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;								//ģ������
	
	GPIO_InitStructure.GPIO_Pin = ME909S_PW_PIN;								//GPRSEN��PC10��
	GPIO_Init(ME909S_PW_Port, &GPIO_InitStructure);								//
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;									//USART3 Tx (PB10)
	GPIO_Init(GPIOB, &GPIO_InitStructure);										//
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;									//USART3 Rx (PB11)
	GPIO_Init(GPIOB, &GPIO_InitStructure);										//
	
	/*G1RESET��G1ONOFF�������*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;							//�������
	
	GPIO_ResetBits(ME909S_REST_PORT, ME909S_REST_PIN);							//����
	GPIO_InitStructure.GPIO_Pin = ME909S_REST_PIN;								//G1RESET��PB12��
	GPIO_Init(ME909S_REST_PORT, &GPIO_InitStructure);							//
	
	GPIO_ResetBits(ME909S_ONOFF_PORT, ME909S_ONOFF_PIN);						//����				
	GPIO_InitStructure.GPIO_Pin   = ME909S_ONOFF_PIN;							//G1ONOFF��PB13��
	GPIO_Init(ME909S_ONOFF_PORT, &GPIO_InitStructure);            				//
}

/*******************************************************************************
* Function Name : void ME909S_PinInit(void)                                   
* Description   : MES909S �������ã���Դ���������ų�ʼ��
* Input         : 
*                 
* Return        :
*******************************************************************************/
void ME909S_PinInit(void)
{
    GPIO_InitTypeDef 	GPIO_InitStructure = {0};	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC,ENABLE);
		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	
	GPIO_InitStructure.GPIO_Pin   = ME909S_PW_PIN;
	GPIO_Init(ME909S_PW_Port, &GPIO_InitStructure);								//PWEN ����
	
	GPIO_InitStructure.GPIO_Pin   = ME909S_ONOFF_PIN;
	GPIO_Init(ME909S_ONOFF_PORT, &GPIO_InitStructure);            				//ONOFF ����
	
	GPIO_InitStructure.GPIO_Pin = ME909S_REST_PIN;
	GPIO_Init(ME909S_REST_PORT, &GPIO_InitStructure);							//RESET ����

	ME909S_REST_H();
	ME909S_ONOFF_H();     														//����ʵ��������
	ME909S_PW_OFF();     														//��Դ�ر�
}


/*******************************************************************************
* Function Name : void ME909SInit(INT32U rate)                                        
* Description   : MES909S �������ã���Դ���������ų�ʼ��
* Input         : rate    �����ڲ���������
*                 
* Return        :
*******************************************************************************/

void ME909SInit(INT32U rate)
{
	UARTx_Setting_Struct UARTInit = {0};
	
	if(GPRSBOX == NULL) GPRSBOX = OSMboxCreate(0);								//GPRS������������
	else GPRSBOX->OSEventPtr= (void *)0;										//����Ϣ���䣬����ᵼ������ ZE		
	
	UARTInit.BaudRate = rate;
	UARTInit.Parity   = BSPUART_PARITY_NO;
	UARTInit.StopBits = BSPUART_STOPBITS_1;
	UARTInit.DataBits = 8;
	UARTInit.RxBuf    = UART3_Rx_Buff;	   
	UARTInit.RxBufLen = LTE_BUFF_LEN ;
	UARTInit.TxBuf    = UART3_Tx_Buff;   
	UARTInit.TxBufLen = LTE_BUFF_LEN ;
	UARTInit.Mode     = UART_DEFAULT_MODE;										//UART_HALFDUP_MODE
	BSP_UART_Init(ME909SPort,&UARTInit,GPRSBOX);								//����4Gģ�鴮�ڣ����ں�ME909SPort=3
	
    ME909S_PinInit();            												//GPRS��Դ���ų�ʼ��
}

/*******************************************************************************
* Function Name : INT8U ME909S_ON(void)                                       
* Description   : MES909S ģ��Ŀ���   �򿪵�Դ�����ӳ�300MS���ϣ���POWERONOFF����1s����������
* Input         :
*                 
* Return        : 
*******************************************************************************/
INT8U ME909S_ON(void)
{
	INT8U				*pRet = NULL, ST=0;
		
	ME909S_PW_OFF();     
	ME909S_REST_H(); 
    ME909S_ONOFF_H();
	ME909S_PW_ON() 	
	OSTimeDly(20);																//�ȴ�300ms����

	ME909S_ONOFF_L();
	OSTimeDly(30);																//���� ����1s����
	ME909S_ONOFF_H();
	OSTimeDly(20);												
	
	pRet=ME909SCommandP(AT_AT, "SYSSTART", 2);     								//ʹ�÷���AT����MAX3373��ͬʱ�ȴ������ɹ����أ�>7S��,(��ģ�鲨����û���趨��ʱ�򣬷���AT�ɽ��в���������Ӧ��֧��9600����Ӧ)
	if(pRet) 	ST= 1;
	else ST=0;
	
	if(!ST){
		pRet=ME909SCommandP(AT_AT, "OK", 2);             						//�����ѷ���SYSSTART��δ�յ������ʱ���ܻظ�OKҲ�ɼ�������
		if(pRet) 	ST= 1;
		else ST=0;}
	
	if(ST){
		pRet=ME909SCommandP(AT_ATE, "OK", 2);           	    				//�رջ��ԡ������ʱ�ϳ�
		if(pRet) return 1;}
			
	BspUartWrite(2,SIZE_OF("����ʧ��----------------------------------\r\n"));OSTimeDly(1);
	return 0;																	//����ʧ��	
}

/*******************************************************************************
* Function Name : void ME909S_OFF(void)                                  
* Description   : MES909S ģ��Ĺػ�  �ҹر�ģ���Դ
* Input         :
*                 
* Return        : 
*******************************************************************************/
void ME909S_OFF(void)      														//������Ҫ�ص�Դ���ز��ػ���ɶ���𣿣���
{
#if 1
    ME909SCommandP(AT_MSO, "0", 2); 	
	OSTimeDly(10);																//Ԥ�������û�����ʱ��
#else						 							
    ME909S_REST_H(); 
    ME909S_ONOFF_H();
	OSTimeDly(5);
	
	ME909S_ONOFF_L();
	OSTimeDly(100);	 															//�ػ� ����4s����
	ME909S_ONOFF_H();
	OSTimeDly(5);	
#endif	
	ME909S_PW_OFF();    														//�ر�ģ���Դ
}

/*******************************************************************************
* Function Name :void DeviceRstOnCMD(void)                            
* Description   :�ӵ�װ�����������MES909S ģ��Ĺػ�  �ҹر�ģ���Դ �ٽ��е�Ƭ����λ
* Input         :
*                 
* Return        : 
*******************************************************************************/
void DeviceRstOnCMD(void)
{
	ME909S_Trans_OFF();			//�˳�͸��ģʽ��������ģʽ
	ME909S_OFF();				//ģ��ػ��ص�Դ
	McuSoftReset();				//������λ��Ƭ��
}

/*******************************************************************************
* Function Name :INT8U ME909S_RateChange(void)                                
* Description   :�޸Ĳ�����Ϊ9600 ��ʱ�Ȳ��㲨���ʲ����ˣ��̶��޸�Ϊĳֵ����Ȼ����32λhexתascii�����⣬�������Ӧ�ú�����Ҫ�õ�
* Input         :
*                 
* Return        : 
*******************************************************************************/
INT8U ME909S_RateChange(void)
{
	INT8U				*pRet=NULL, ST=0;
	
	pRet=ME909SCommandP(AT_AT, "OK", 2);      									//���Թ�ͨ
	if(pRet) 	ST=1;
	else ST=0;
	
	if(ST)
	{ pRet=ME909SCommandP(AT_IPR9600, "OK", 2);    							//���ù̶�9600������
	if(pRet) 	ST=1;
	else ST=0;}
	
	
	if(ST)   
	OSTimeDly(300);		                                    					//�ŵ�
	ME909SInit(9600);
	ST=ME909S_ON();                                  							//���¿���
	
	
	if(ST)
	{ pRet=ME909SCommandP(AT_AT, "OK", 2);      								//���Թ�ͨ
	if(pRet) 	ST=1;
	else ST=0;}
				
					
	if(ST)
	{ pRet=ME909SCommandP(AT_IPRCHECK, "+IPR", 2);    						//��ѯ������
	if(pRet) 	ST=1;
	else ST=0;}

	if(ST) return 1;
	return 0;
}

/*******************************************************************************
* Function Name :INT8U ME909S_Get_Signal_Strength(void)                            
* Description   :���ģ����ź�ǿ�ȣ�������������� һ���ֽ�
* Input         :��
*                 
* Return        : �ź�ǿ�Ȱٷֱ�		0:���źŻ��ź�ǿ��δ֪�򲻿ɲ�	
*******************************************************************************/
INT8U ME909S_Get_Signal_Strength(void)
{
	
	INT8U				*pRet=NULL;
	INT8U				Signal_Strength=0, i=0;

	for(i=0;i<3;i++)
	{
		pRet=ME909SCommandP(AT_HCSQ, "^HCSQ:", 2);      						//��ѯ�ź�ǿ��   ģ��Ӳ�������ߣ�������������������
		if(pRet) 
		{
			pRet = ScanAsicc(pRet,15,(INT8U *)"\",",2); 						//��λRSSI���ݵ�ָ��
			if(pRet)  
			{
				if((pRet[0]==0x32)&&(pRet[1]==0x35)&&(pRet[2]==0x35)) continue;	//�����RSSI=255ʱ���ź�ǿ��Ϊδ֪�򲻿ɲ�,���²�
				Signal_Strength=100*((pRet[0]-0x30)*10+(pRet[1]-0x30))/96;		//��Ϊģ���õ�RSSI_MAX=96��-25dbm������������ٷֱ�
				return Signal_Strength;
			}					
		}
	}
	return 0;
}

/*******************************************************************************
* Function Name :INT8U HB_Get_Signal_Strength(void)                           
* Description   :Ϊ������֡��ȡһ�������ź�ǿ�Ȱٷֱ�(��ر�͸���������ѯָ��ٴ�͸��)
* Input         :
*                 
* Return        : 0-100:�����ź�ǿ�ȣ�0��ʾ���źŻ��źŲ��ɲ⣩ 0xFF����͸��ʧ�ܣ���Ҫ���³���
*******************************************************************************/
INT8U HB_Get_Signal_Strength(void)
{
	INT8U Signal_Strength=0;
	
	ME909S_Trans_OFF();										//�˳�͸��ģʽ  
	Signal_Strength=ME909S_Get_Signal_Strength();			//
	if(ME909S_Trans_ON()) return Signal_Strength;			//�ػ�͸��ģʽ
	return 0XFF;											//�ػ�͸��ģʽʧ��
}

/*******************************************************************************
* Function Name :INT8U ME909S_CONFIG(void)                               
* Description   :ģ�鿪������б������ã���������ָ��ȴ�ʱ�䶼����һЩ
* Input         :
*                 
* Return        : 
*******************************************************************************/
INT8U ME909S_CONFIG(void)
{
	INT8U				*pRet=NULL, ST=0;
	
	{ pRet=ME909SCommandP(AT_CURC, "OK", 2);       		 					//�ر��Զ��ϱ�ģ��״̬
	if(pRet) 		ST= 1;
	else ST=0;}
	
#if 0		//��������ģʽ
	if(ST)																									
	{ pRet=ME909SCommandP(AT_ANT, "OK", 2);           	 					//��������ģʽ       //�����ûᵼ���豸��������Ѱ�������ҵ���Ѱ��֮��رջ��Ժ͹ر��Զ��ϱ����û�ʧЧ����Ҫ��������
	if(pRet) 	ST=1;												 			//Ĭ��������ʽʹ��˫���ߣ��Ǳ�Ҫ����������
	else ST=0;}

	if(ST)
	{ pRet=ME909SCommandP(AT_ANTCHECK, "OK", 2);      	 					//��ѯ����ģʽ
	if(pRet) 	ST=1;
	else ST=0;}
	
	//����LED����ģʽ 
	if(ST)  //��ʡ��
	{ pRet=ME909SCommandP(AT_LED, "OK", 2);          	 						//����LED����ģʽ��Ĭ�ϣ������õ��籣�棬ֻ������һ��
	if(pRet) 	ST=1;
	else ST=0;}

	if(ST)  //��ʡ��
	{ pRet=ME909SCommandP(AT_LEDCHECK, "OK", 2);     	 						//��ѯLEDģʽ  
	if(pRet) 	ST=1;
	else ST=0;}
#endif	
	
#if 0	//���������������ȼ�
	if(ST)
	{ pRet=ME909SCommandP(AT_FREQLOCKCHECK, "OK", 2);     					//��Ƶ��ѯ
	if(pRet) 	ST=1;
	else ST=0;}
	
	if(ST)
	{ pRet=ME909SCommandP(AT_SYSCFGEXCHECK, "OK", 2);    						//��ѯϵͳ��չ���� 
	if(pRet) 	ST=1;
	else ST=0;}	
	
	if(ST)
	{ pRet=ME909SCommandP(AT_SYSCFGEX, "OK", 2);         						//����ϵͳ��չ   ��������ǵ��籣��ģ�����һ�μ���
	if(pRet) 	ST=1;
	else ST=0;}
	
//	if(ST)  //��ʡ��
//	{ pRet=ME909SCommandP(AT_SYSCFGEXCHECK, "OK", 2);     					//��ѯϵͳ��չ����  
//	if(pRet) 	ST=1;
//	else ST=0;}
#endif	
	
	
	/*͸����ز�������*/		
	if(ST)
	{pRet=ME909SCommandP(AT_IPCFL5, "OK", 2);       							//͸����ʱʱ������
	if(pRet) 	ST= 1;	
	else ST=0;}
	
	if(ST)
	{pRet=ME909SCommandP(AT_IPCFL10, "OK", 2);     							//͸��buffer����   
	if(pRet) 	ST= 1;
	else ST=0;}
	
	if(ST)
	{pRet=ME909SCommandP(AT_IPCFL12, "OK", 2);     							//͸��ģʽ����
	if(pRet) 	ST= 1;
	else ST=0;}
	
	if(ST) return 1;
	return 0;
}

/*******************************************************************************
* Function Name :INT8U ME909S_REG(void)                             
* Description   :ģ���������ע��
* Input         :
*                 
* Return        : 
*******************************************************************************/
INT8U ME909S_REG(void)
{
	INT8U *pRet=NULL;
	INT8U ST=0;
	INT8U i=0;
	INT8U j=0;
	INT8U retry=2;
	
	for(i=0;i<retry;i++)
	{
//		pRet=ME909SCommandP(AT_CFUN, "OK", 2);          						//����ģ��Ϊonline mode���Ҳ�����ģ��
//		if(pRet) 	ST=1;
//		else ST=0;

//		pRet=ME909SCommandP(AT_CFUNCHECK, "OK", 2);    						//ģ��ģʽ��ѯ
//		if(pRet) 	ST=1;
//		else ST=0;
		
//		if(ST)
		{ pRet=ME909SCommandP(AT_CPIN, "OK", 2);       						//��ѯSIM���Ƿ�����		#define SZOF(a)   a,sizeof(a)-1  
		if(pRet) 	ST=1;
		else ST=0;}
		
		if(ST)
		{ pRet=ME909SCommandP(AT_COPS, "OK", 2);      	 	   				//������Ӫ��Ϊ�Զ�ѡ��
		if(pRet) 	ST=1;
		else ST=0;}		
		
		j=6;          //������ö�ζ�ʱ��Ĳ�ѯ��Ŀ������SIM��ֻ֧��2G/3G��ʱ��������Ӫ�̻��������ζ�ʱ������������ϵķ�ʽ ��������108s
		while(j--)
		{
			if(ST)
			{ pRet=ME909SCommandP(AT_COPSCHECK, "0,0,", 1);     				//ȷ����Ӫ����������     ���������ֻ�ظ�COPS:0˵����δ������Ӫ��
				if(pRet) 	
				{
					ST=1;
					break;
				}													   		   //��ȷ����Ӧ����+COPS: 0,0,"CMCC",7 �����4G��ܿ��ҵ��������Ŀ�����Ҫ������ʱ�������ʵ����ӳ��Դ����� 7��4G��0��GSM��2��UTRAN ��3G��			
			}				
			if(j==0) ST=0;
	    }

		if(ST)
		{ pRet=ME909SCommandP(AT_CREG, "OK", 2);    			   				//��������ע��״̬�Զ��ϱ�	GSM
		if(pRet) 	ST=1;
		else ST=0;}	

		if(ST)
		{ pRet=ME909SCommandP(AT_CGREG, "OK", 2);    		   					//��������ע��״̬�Զ��ϱ�	GPRS
		if(pRet) 	ST=1;
		else ST=0;}
		
		j=3;
		while(j--)
		{
			if(ST)  //��һ��ע���Ͼ�OK�ˣ�����GPRS
			{ 
				pRet=ME909SCommandP(AT_CGREGCHECK, "+CGREG:", 1);    			//��ѯ��ǰGPRS����ע��״̬  ��ȷ�ظ�+CGREG: 1,1    //GPRS
				if(pRet) 	
				{
					pRet = ScanAsicc(pRet,5,(INT8U *)"1,1",3); 					//��","
		            if(pRet)  
					{
						ST=1;         //ע��ɹ�����	
						break;
					}					
				}	
				
				pRet=ME909SCommandP(AT_CREGCHECK, "+CREG:", 1);    			//��ѯ��ǰGSM����ע��״̬  ��ȷ�ظ�+CREG: 1,1		/*��ʹ��GSM����ʱ��2G��*/
				if(pRet) 	
				{
					pRet = ScanAsicc(pRet,5,(INT8U *)"1,1",3); 					//��","
		            if(pRet)  
					{
						ST=1;         											//ע��ɹ�����	
						break;
					}					
				}	
				
				OSTimeDly(5*20);       											//������û���ػ��Ƿ��صĲ�����Ч���ݣ�����5s��ѯ��
			}	
		    if(j==0) ST=0;           											//���λ��ᶼ�ù���	
		}

#if 0		
		if(ST) //��ʡ��
		{ pRet=ME909SCommandP(AT_SYSINFO, "OK", 2);    						//��ѯ��չϵͳ��Ϣ   ע����֮��ɲ�ѯע���ϵ����ĸ�����
		if(pRet) ST=1;
		else ST=0;}			 													//^SYSINFOEX: 2,3,0,1,,1,"GSM",3,"EDGE"��������Ч��PS+CS���񣬷�����״̬����ЧSIM������GSM��ʽ���룬GSM��ʽ����ʽ����EDGE���룬��ʽ����EDGE
#endif

		if(ST) return 1;
		OSTimeDly(100);
    }

	if(!Net_Fault_Time) Net_Fault_Time=RtcGetTimeSecond();						//����ע��ʧ�� ��ȡ��ǰ��ʱ��	��λ���� 
	return 0;
}

/*******************************************************************************
* Function Name :INT8U ME909S_IPINIT(void)                         
* Description   :ģ�����TCP/IP��ʼ��
* Input         :��
*                
* Return        : 1�����óɹ�
*******************************************************************************/
INT8U ME909S_IPINIT(void)
{
	INT8U				*pRet=NULL;
	TCHAR				AT_IPINIT[120] = {0};									//��ǰ��˾ʹ�õ���������CMMTM��CMIOT(֧��4G)�����Ӹ��죬��ʹ��CMNET�����

	snprintf(AT_IPINIT, APN_Len+12, "AT^IPINIT=\"%s\"\r\n", APN);				//д�뵽���飬���ڷ���ATָ��
	pRet=ME909SCommandP(AT_IPINIT, "OK", 2);    								//��ʼ��TCP/IP     //���ʱ����ܽϳ�
	//��һ��û�ȵ�OK�Ļ������ܵȴ�ʱ�䲻�����������ٷ����ܻ᷵��ERR:1013��The network has been opened already ,�����ٲ�״̬���Ѵ���ֱ�Ӽ�����
	if(pRet) 	return 1;

	else{
	pRet=ME909SCommandP(AT_IPINITCHECK, "^IPINIT:", 3);   						//��ѯTCP��ʼ����� 
	if(pRet){
		pRet = ScanAsicc(pRet,5,(INT8U *)"1,",2); 								//��","
		if(pRet) return 1;                                              		//The network has been opened already 	
		}
	}	
	return 0;
}

/*******************************************************************************
* Function Name :INT8U ME909S_IPOPEN(INT8U SocketID,INT8U *IP,INT8U *Remote_Port,INT16U Local_Port)                        
* Description   :ģ��ʹ��һ��socket��һ��IP����
* Input         :SocketID:���Ӻţ�1-5��
				 IP��д��flash�������ӵ�IP��ַ    remote Port   :Զ�̶˿ڣ������þ���  
				 local Port�����ж������Ҫ��ʱ�����ָ�������ͻ������ָ��Ϊ1000�����߿���ʡ�Ըò�����ģ���Զ����䣩               
* Return        : 1:���óɹ�
*******************************************************************************/
INT8U ME909S_IPOPEN(INT8U SocketID,INT8U *IP,INT8U *Remote_Port,INT16U Local_Port)  
{
	INT8U				*pRet=NULL, *P=NULL;
	INT8U				Num=0, MaxNum=0;
	INT16U				Re_Port=(Remote_Port[0]<<8)+Remote_Port[1];
	INT8U				ATIPOPEN[50] = {0};   //���ڴ�ָ��IP
	
	if((SocketID<1)||(SocketID>5)) return 0;
	
	/*���ATIPOPEN[]*/
	memcpy(ATIPOPEN,(INT8U *)"AT^IPOPEN=",10);                          	
	ATIPOPEN[10]=SocketID+0X30;
	memcpy(&ATIPOPEN[11],(INT8U *)",\"UDP\",\"",8);                    			//������д����AT^IPOPEN=1,"TCP","   ��19���ֽ�			//�����UDP�����������޸�ΪUDP
	
	/*�пո�д��snprintf*/
	MaxNum=19;
	P=&ATIPOPEN[19];
	Num=INT16UHexToAscii(IP[0],P);
	P=P+Num;
	MaxNum+=Num;
	*P++='.';
	MaxNum++;
	Num=INT16UHexToAscii(IP[1],P);
	P=P+Num;
	MaxNum+=Num;
	*P++='.';
	MaxNum++;
	Num=INT16UHexToAscii(IP[2],P);
	P=P+Num;
	MaxNum+=Num;
	*P++='.';
	MaxNum++;
	Num=INT16UHexToAscii(IP[3],P);
	P=P+Num;
	MaxNum+=Num;
	*P++='\"';														  
	MaxNum++;
	*P++=',';
	MaxNum++;
	Num=INT16UHexToAscii(Re_Port,P);
	P=P+Num;
	MaxNum+=Num;
	*P++=',';
	MaxNum++;
	Num=INT16UHexToAscii(Local_Port,P);               
	P=P+Num;
	MaxNum+=Num;                                                 				//������д����AT^IPOPEN=1,"TCP","121.199.13.107",8301,1000
	*P++='\r';
	*P='\n';
	MaxNum+=2;                           

	pRet=ME909SCommandP((TCHAR*)ATIPOPEN, "OK", 2);         					//��ʼ��TCP/IP
	if(pRet) 	return 1;

	else{
	pRet=ME909SCommandP(AT_OPENCHECK, "^IPOPEN", 3);      						//��ѯIP�������ȷ��IP���ӳɹ�       
	if(pRet) 	return 1;														//�ظ��ǣ�^IPOPEN: 1,"TCP",1000,"121.199.13.107",8301,1,1400  ����1ָ���Ƕ˿�UART������ֳ�1400
	}
	return 0;
}

/*******************************************************************************
* Function Name :INT8U ME909S_IPCLOSE(INT8U SocketID)             
* Description   :ģ��ر�����IP����
* Input         : Port:ģ�鴮�ں�
*                 SOCKID:1-5��5�����ӣ�
						 6���رշ�������������, ���ر��������������
						 7���ر������������ӡ�����������ӻ������������Ҳ�ᱻ�رա�
* Return        : 
*******************************************************************************/
INT8U ME909S_IPCLOSE(INT8U SocketID)
{
	INT8U				*pRet=NULL;
	TCHAR				ATIPCLOSE[14]={0};
	
	if((SocketID<1)||(SocketID>7)) return 0;
	
	memcpy(ATIPCLOSE,(INT8U *)"AT^IPCLOSE=",11);   
	ATIPCLOSE[11]=SocketID+0x30;
	ATIPCLOSE[12]=0x0d;
	ATIPCLOSE[13]=0x0a;
	
	pRet=ME909SCommandP(ATIPCLOSE, "OK", 5);    								//�Ͼ� ����Ϊ��^IPCLOSE: 0,0,0,0,0 ����5�����Ӷ��ر��ˣ����sockid=7������������
	if(pRet) 	return 1;
	else return 0;	
}

/*******************************************************************************
* Function Name :INT8U ME909S_Trans_ON(void)                         
* Description   :ģ�����ò���͸�����ͨ��
* Input         :
*                 
* Return        : 1����͸��ģʽ	0����͸��ʧ��
*******************************************************************************/
INT8U ME909S_Trans_ON(void)
{
	INT8U *pRet=NULL;
	
	/*��͸��ģʽ�Ĳ��������Ƶ�CFG�����ֻҪ���ϵ磬����һ�μ���*/
//	pRet=ME909SCommandP(AT_IPCFLCHECK, "OK", 2);								//��ѯģʽ�������ý��	��ʡ��
	pRet=ME909SCommandP(AT_IPENTRANS, "OK", 2);									//��͸��ģʽ
	if(pRet) return 1;
	return 0;
}

/*******************************************************************************
* Function Name :INT8U ME909S_Trans_OFF(void)                     
* Description   :ģ��ر�͸��
* Input         :
*                 
* Return        : 
*******************************************************************************/
INT8U ME909S_Trans_OFF(void)
{
	INT8U				*pRet=NULL;

	OSTimeDly(20);
	pRet=ME909SCommandP(AT_IPDISTRANS, "OK", 2);								//ǰ����Ҫ��ʱ900ms
	if(pRet) 	return 1;
	else return 0;
}

/*******************************************************************************
* Function Name :INT8U ME909S_Link(INT8U SocketID)                             
* Description   :ģ���������ע��
* Input         :
*                 
* Return        : 
*******************************************************************************/
INT8U ME909S_Link(INT8U SocketID)
{
	INT8U ST=0;
	INT8U retry=3;
	INT8U i=0;

	for(i=0;i<retry;i++)
	{	
//		ST=ME909S_IPCLOSE(1);
//		ST=ME909S_IPCLOSE(6);													//6���رշ�������������, ���ر��������������
		ST=ME909S_IPCLOSE(7);       											//7���ر������������ӡ�����������ӻ������������Ҳ�ᱻ�ر�
		if(ST==0) continue ;
		ST=ME909S_IPINIT();														//ģ�����TCP/IP��ʼ��
		if(ST) ST=ME909S_IPOPEN(SocketID,IP_Config.IP_addr_1,IP_Config.PortNum_1,1000);
		if(ST) ST=ME909S_Trans_ON();
		if(ST)																	//�ɹ�
		{
			if(Fault_Manage.F_NETWORK || Net_Fault_Time)						//�ѹ�30���Ӳ��������ϱ���δ���������ϱ��������������������ﲻ�ܵ���Net_Fault_Time����ֵΪ�ж������������ϲ�����ϵͳ��λ����ֵһ��Ϊ0���ᵼ���޷������������ϻָ���
			{
				Net_Fault_Time=0;												//����	
				if(Fault_Manage.F_NETWORK)										//������������
					NW_Fault_Manage(NETWORK_F, NOFAULT_STA);					//�������ϻָ��ϱ�
			}
			return 1;	    												
		}
	}	
	if(!Net_Fault_Time) Net_Fault_Time=RtcGetTimeSecond();						//��������ʧ�� ��ȡ��ǰ��ʱ��	��λ���� 
	return 0;                  													//���γ��Զ�ʧ����	
}

/*******************************************************************************
* Function Name : INT8U *ME909SCommandP(TCHAR *pAtCmd,TCHAR *pExpResponse,INT16U WaitTime)                                                    
* Description   : ��AT�������ʽ�ٿ�A8500ִ����Ӧ�Ķ������ȴ����صĽ���Ƿ���ϣ����ֵ
* Input         : Port          : A8500ģ�����ӵĴ��ں�
*                 pAtCmd   		: ָ��һ��AT�����Buffer��ַ
*                 AtCmdLen      : AT����ĳ���
*                 pExpResponse	: �����еķ������ݣ�����ָ���ķ������ݣ�������Ǹ���A8500ģ��Ĳ����ֲ���ָ���ġ�
*                 ExpBkLen      : �����������ݵĳ���,����ָ���ģ���������Ҳ֪������������س����Ƕ���
*                 WaitTime      : �ȴ��������ݵ��������ʱ�䣬��λ����
*                 
* Return        : �βη��أ�pExpResponse	: A8500ģ�鷵����������
*                 ��ʽ���أ�NULL			: �ط������ζ�ʧ�ܣ����ؿ�ָ��
*                          pRet			: ���ͺ����յ�GPRSģ��Ļظ�������Я�������ݵ�ָ�롣
*******************************************************************************/
INT8U *ME909SCommandP(TCHAR *pAtCmd,TCHAR *pExpResponse,INT16U WaitTime)
{
	INT8U				*pRet = NULL, Retry_Cnt = 0, Num = 0, ST = 0, WaitCnt = 0;         
	INT16U				i, RBLen = 0, AtCmdLen = 0, ExpBkLen = 0;

	AtCmdLen = strlen(pAtCmd);
	ExpBkLen = strlen(pExpResponse);
	for(Retry_Cnt=0;Retry_Cnt<3;Retry_Cnt++)  
	{		
		StopModeLock++;
//		Feed_Dog();
		OSTimeDly(2);                         						
		if(pAtCmd && AtCmdLen)													//�������ݷǿ��ж�
		{	
			ST=60;																//3�볬ʱ
			while( BSP_UART_TxState(2) && (ST--) ) OSTimeDly(1);				//�ȴ����ڿ���
			BspUartWrite(2,(INT8U*)pAtCmd,AtCmdLen); 							//���Դ�ӡ
			ST=60;																//3�볬ʱ
			while(BSP_UART_TxState(2)&&(ST--))OSTimeDly(1); 					//�ȴ���ӡ���

			BSP_UART_RxClear(ME909SPort);  										//���
			ST=60;
			BspUartWrite(ME909SPort,(INT8U*)pAtCmd,AtCmdLen);  					//������������ķ���ATָ��	
			while(BSP_UART_TxState(ME909SPort)&&(ST--))OSTimeDly(1);			//�ȴ�����3�������
		}

		for(WaitCnt=0;WaitCnt<WaitTime;WaitCnt++)  
		{
			pRet = GPRS_WaitSign(20,&Num,&RBLen); 								//ÿ�εȴ�1�룬���WaitTime��
			if(StopModeLock) StopModeLock--;
			if(pRet && pExpResponse && ExpBkLen )
			{		
				if((RBLen >= ExpBkLen))
				{
					for(i=0;i <= (RBLen - ExpBkLen);i++)
					{
						if (!memcmp(pExpResponse,pRet,ExpBkLen))				//ȥ�ظ�����������ڴ��Ļظ�
						{
							pRet += ExpBkLen;								
							return pRet;   										//�ɹ������ﷵ�ص��ǱȶԳɹ�֮��ĺ�һλ�ĵ�ַ
						}
						pRet++;
					}
					continue;  													//û���ҵ�����ֵ�������ȴ�
				}
			}
			if( (ExpBkLen == 0) && (pRet) )
			{
				return pRet;
			}
		}	
		if(Retry_Cnt<2)
			OSTimeDly(20);
	}
	return NULL;  
}






/////����������A8500.C==========================================================================================================================
/*******************************************************************************
* Function Name : INT8U* GPRS_WaitSign(INT16U WaitTime,INT8U *pDivNum,INT16U *pRxLen)                                                   
* Description   : 4Gģ��ȴ����ؽ��
* Input         : Port          : 4Gģ�����ӵĴ��ں�
*                 WaitTime      : ѭ���ȴ�ɨ��Ĵ���������βδ�����֮����Ҫ��ϱ������ڲ��� OSTimeDly(1)ʹ�ã����һֱû�еȵ����ݣ�
*                                 ���վ���ִ��WaitTime��ѭ���ȴ����ֱ���˳������ˡ��ܵȴ�ʱ�� = ��WaitTime * OSTimeDly(1)
*                 pDivNum       : �������ٿռ�󣬽���ַ���뱾�������ɱ��������һ��ֵ����������˼������һ���豸�ţ�ʲô�ġ�
*                 pRxLen        : �������ٿռ�󣬽���ַ���뱾�������ɱ��������һ��GPRSģ����յ������ݵĳ���ֵ��
*                 
* Return        : �βη��أ�pDivNum ���豸��,����ֻ��һ���������� GPRS_RecvDataWait()����ʹ����������ص��豸��,���������������в�δʹ�ã������������
*                           pRxLen ��GPRSģ����յ������ݳ��� 
*                 ��ʽ����: pRet    : GPRSģ�鷵�����ݵ��׵�ַ 
*                           NULL   : �β�WaitTime�������󣬻��ߵȴ���ʱ������
*******************************************************************************/
INT8U* GPRS_WaitSign(INT16U WaitTime,INT8U *pDivNum,INT16U *pRxLen)
{
	INT8U Err=0;
	INT8U *pRet = NULL;   														//����ָ��GPRSģ�鷵�����ݵ��׵�ַ 
	INT16U RBLen = 0;
	INT8U ST=0;

	if( WaitTime == 0) return NULL; 											// �����Ϸ��Լ��
		
		StopModeLock++;
		pGprsMess = (struct Str_Msg *)OSMboxPend(GPRSBOX,WaitTime,&Err);		// �ȴ�������Ϣ  
		if(StopModeLock) StopModeLock--;
		if((Err==OS_NO_ERR) && pGprsMess)										//����Ϣ����
		{
			if( pGprsMess->MsgID == BSP_MSGID_UART_RXOVER )						//�Ǵ���3����Ϣ
			{
				RBLen= pGprsMess->DataLen;										//��ȡ��Ϣ����
				pRet=pGprsMess->pData;											//��Ϣָ�봫��
				BSP_UART_RxClear(pGprsMess->DivNum);							//������ڣ����ָ�룬���ݻ����Ѵ���pRet��
			}
			
		}
		else return NULL;
		
		if( pRet && RBLen )      												// ����Ƿ��ж���
		{		
			if( RBLen > LTE_BUFF_LEN) BSP_UART_RxClear(ME909SPort);				// ���յ������ݳ������ˡ�
			
			else
			{
				BSP_UART_RxClear(ME909SPort);
				*pRxLen=RBLen;
				*pDivNum = ME909SPort;  		
//				ST=ME9009S_RecvDataDealWith(pRet,RBLen); 						//͸��ģʽ�µ����ݽ��մ�����һ���µĺ���     2018.08.02  CW�޸�
		
				ST=60;
				while( BSP_UART_TxState(2) && (ST--) ) OSTimeDly(1);
				BspUartWrite(2,pRet,RBLen);    									// ��GPRSģ����յ������ݣ���2�Ŵ��ڣ����Դ�ӡ��        //�����յ�ʲô����ӡ����
				ST=60;
				while(BSP_UART_TxState(2)&&(ST--))OSTimeDly(1); 				//�ȴ���ӡ���
				
				return pRet;              
			}
		}
	return NULL;
}

/*******************************************************************************
���ƣ�INT8U LTE_Get_Config(void)
���ܣ���ȡLTE�������������ýṹ��&��վIP��ַ���˿ںźͿ������ýṹ��
��Σ���
���Σ���
���أ�0��ʧ�ܣ�1���ɹ�
*******************************************************************************/	
INT8U LTE_Get_Config(void)
{
	if(!BSP_ReadDataFromFm(Config_Addr,(u8*)&Config,Config_Len)) return 0;							//�������ýṹ��Config
	if(!BSP_ReadDataFromFm(IP_Config_Addr,(u8*)&IP_Config,IP_Config_Len)) return 0;					//��վIP��ַ���˿ںźͿ������ýṹ��IP_Config
	if(!BSP_ReadDataFromFm(APN_Addr, APN, APN_Len)) return 0;										//APN��Ϣ
	if(!BSP_ReadDataFromFm(Device_Number_Addr,Device_Number,Device_Number_Len)) return 0;			//װ�ú���Device_Number
	if(!BSP_ReadDataFromFm(FUN_Config_Addr,FUN_Config,FUN_Config_Len)) return 0;					//�������ò���FUN_Config
	if(!BSP_ReadDataFromFm(Flow_Data_Addr,(u8*)&Local_FLow_Data,Flow_Data_Len)) return 0;			//������ȡ����ͳ����Ϣ
	return 1;		// ������Ч����
}

/*========================================================����Ϊ��д����������========================================================*/
/*******************************************************************************
* Function Name :INT16U LTE_WaitData(INT8U *OutBuff,INT16U timeout)       
* Description   :ģ��ȴ���λ���������ݣ��������ݴ���
* Input         :pOutBuff��ָ��������ݵ�ַ  timeout���ȵ���ʱʱ�� ������ʱ��Ƭ 1=20ms��
*                 
* Return        :RBlen:�������ݳ���    0:�ȴ��޷���  0XFFFF:�������
*******************************************************************************/
INT16U LTE_WaitData(INT8U *OutBuff,INT16U timeout)
{
    INT8U i=0,Err=0;
	INT16U RBLen =0;
	INT8U *pRB=NULL;
	INT8U ST=0;

	if( OutBuff == 0) return 0; 												// �����Ϸ��Լ��
	if( timeout == 0) return 0; 												// �����Ϸ��Լ��
		
	StopModeLock++;
	pGprsMess = (struct Str_Msg *)OSMboxPend(GPRSBOX,timeout,&Err);				//���ʾ��ǵȴ�����
	if(StopModeLock) StopModeLock--;
	
	if((Err==OS_NO_ERR) && pGprsMess)											//����Ϣ����
	{
		if( pGprsMess->MsgID == BSP_MSGID_UART_RXOVER )							//�Ǵ���3����Ϣ
		{
			RBLen= pGprsMess->DataLen;											//��ȡ��Ϣ����
			pRB=pGprsMess->pData;												//��Ϣָ�봫��			
			BSP_UART_RxClear(pGprsMess->DivNum);								//������ڣ����ָ�룬���ݻ����Ѵ�pRB��
		}		
	}	
	
	if( pRB && RBLen )      													//����Ƿ��ж�����
	{	
		if(RBLen>LTE_BUFF_LEN)RBLen=LTE_BUFF_LEN;
		memcpy(OutBuff,pRB,RBLen);
		BspUartWrite(2,SIZE_OF("����<<��"));		
		ST=60;
		while( BSP_UART_TxState(2) && (ST--) ) OSTimeDly(1);
		if(update_start==false)													//������������ʱ������ӡ���յ�������
		{
			BspUartWrite(2,pRB,RBLen);    										//��GPRSģ����յ������ݣ���2�Ŵ��ڣ����Դ�ӡ��
			BspUartWrite(2,SIZE_OF("\r\n"));
		}
		ST=60;
		while(BSP_UART_TxState(2)&&(ST--))OSTimeDly(1); 						//�ȴ���ӡ���
		
		/*���ж����ж�*/
		for(i=0;i<30;i++)                 										//����Ƿ����   ^IPSTATE�����ϱ�����״̬���ϱ���ʱ����Ƕ����ˣ�ֻ������վ̨�ı�ʱ��Ż��յ����ϱ�
		{
			if (!memcmp(pRB,SIZE_OF("^IPSTATE")))  
			{
				if(!Net_Fault_Time) Net_Fault_Time=RtcGetTimeSecond();			//��������ʧ�� ��ȡ��ǰ��ʱ��	��λ���� 
				return 0xFFFF;	   
			}
			pRB++;
		}

		Update_Flow_DayAndMonth(RBLen);											//����������ͳ�ƽ�ȥ
		BSP_UART_RxClear(3); 		
		return RBLen;															//�����Ч����	   		
	}
	return 0;													
}

/*******************************************************************************
* Function Name :INT16U LteCommunication(INT8U *pIn,INT16U InLen,INT8U *pOut,INT16U timeout)        
* Description   :ģ�鷢�����ݲ�������������
* Input         :pIn���������� InLen���������ݳ���      pOut����������   timeout:�ȴ��ظ���ʱ�䣨����ʱ��Ƭ 1=20ms��
*                 
* Return        : ���ؽ��յ������ݳ��ȡ������н��ջ�û���յ�ʱ����0
*******************************************************************************/
INT16U LteCommunication(INT8U *InBuff,INT16U InLen,INT8U *OutBuff,INT16U timeout)
{		
	INT16U RBLen = 0;      														//���ص����ݳ���  
	INT8U Count = 0;
	
	if((!InBuff)||(InLen==0))return 0;
	if(InLen>1472) return 0; 													//͸��������볤��
	
	/*���Ͳ���*/	
	if ((InBuff)&&(InLen))
	{	
		StopModeLock++;  
		Count=60;
		while( BSP_UART_TxState(2) && (Count--) ) OSTimeDly(1);
		BspUartWrite(2,SIZE_OF("����>>��"));
		BspUartWrite(2,InBuff,InLen); 
		BspUartWrite(2,SIZE_OF("\r\n"));
		Count=60;
		while(BSP_UART_TxState(2)&&(Count--)) OSTimeDly(1);

			
		BSP_UART_RxClear(3);  													// ��� ���ջ��棬׼�����շ��ص�����
		Count=60;		
		BspUartWrite(3,InBuff,InLen);   										// ��4Gģ��д��ATָ�������������	
		while(BSP_UART_TxState(3)&&(Count--)) OSTimeDly(1);
		if(StopModeLock) StopModeLock--;	
	}
	Update_Flow_DayAndMonth(InLen);												//����������ͳ�ƽ�ȥ
	
	/*���ղ���*/
	if( OutBuff == 0) return 0; 												//�����н���
	RBLen=LTE_WaitData(OutBuff,timeout);										//�����ѽ���������ͳ�ƽ�ȥ
	
	/*��λ��δ�ظ������жϲ���*/
	if(!RBLen)																	//RBLen=0
	{
		if(!Host_No_Reply_Time)
			Host_No_Reply_Time=	RtcGetTimeSecond();								//��վ�޻ظ�����¼δ�ظ���ʱ��,����¼��һ��
	}
	else if( RBLen!=0xFFFF && (Fault_Manage.F_REPLY ||Host_No_Reply_Time))		//��վ��ȷ�ظ��ҷ�������������  �����ﲻ�ܵ���Host_No_Reply_Time�Ƿ���ֵΪ�ж��������������Ѳ�����ϵͳ��λ����ֵһ��Ϊ0���ᵼ���޷�������վδ�ظ����ϻָ���
	{
		Host_No_Reply_Time=0;													//��վ�ظ��������վδ�ظ���ʱ
		if(Fault_Manage.F_REPLY)												//�����Ѳ�����30����δ�������ɹ���
			NW_Fault_Manage(REPLY_F, NOFAULT_STA);								//�Ѳ�����վδ�ظ����ϣ��ϱ����ϻָ�
		
	}
	return RBLen;
}

/*========================================================		��������		========================================================*/
/*******************************************************************************
* Function Name :INT8U ME909S_SMS_CFG(void) 	                             
* Description   :ģ����ж��Ź������� 
* Input         :pIn:ָ����Ķ������ݣ�InLen���������ݵĳ���
*                 
* Return        : 1�����óɹ� 0�����ó���
*******************************************************************************/
INT8U ME909S_SMS_CFG(void)
{
	INT8U				*pRet=NULL;
	INT8U				ST=0;

	pRet=ME909SCommandP(AT_CPMS_CHECK, "OK", 2);      	 	    				//��ѯ���Ŵ洢����  Ĭ��ΪSM,SM,SM
	if(pRet) 	ST=1;
	else ST=0;	
	
	if(ST)
	{
		pRet=ME909SCommandP(AT_CMGF_TEXT, "OK", 2);     						//���ö���ģʽΪTEXTģʽ  ���粻���� ����TEXTģʽ������ÿ�ζ���������
		if(pRet) 	ST= 1;
		else ST=0;}		
	
	if(ST)
	{
		pRet=ME909SCommandP(AT_CSMP_GSM, "OK", 2);     							//�����ı����ŵı��뷽ʽ GSM  
		if(pRet) 	ST= 1;
		else ST=0;}		
		
	if(ST)
	{
		pRet=ME909SCommandP(AT_CSCS_GSM, "OK", 2);     							//����TE �ַ���
		if(pRet) 	ST= 1;
		else ST=0;}	
	
	if(ST)
	{
		pRet=ME909SCommandP(AT_CNMI, "OK", 2);     								//���ö���ģʽΪTEXTģʽ  ���粻���� ����TEXTģʽ������ÿ�ζ���������
		if(pRet) 	ST= 1;
		else ST=0;}		
		
//	if(ST)
//	{
//		pRet=ME909SCommandP(AT_CMGL, "OK", 2);        							// ��ȡ����   
//		if(pRet) 	ST=1;
//		else ST=0;
//	}
	
	/*
	������Լ�����Ŷ�ȡ�������׳�ȥ�������ϵ��ȡ���������Ҫ�Ļ���
	*/
	
	if(ST) 
	{
		pRet=ME909SCommandP(AT_CMGD_ALL, "OK", 2);        						// ɾ��ȫ������	(����������ҵ�����)
		if(pRet) 	return 1;}
	
	return 0;	
}
/*******************************************************************************
* Function Name :INT8U ME909S_SMS_Delete(INT8U Index,INT8U opt)	                             
* Description   :ɾ������
* Input         :	Index:����λ������
					opt������ɾ������ѡ��  
						0 ɾ���� <index> ָ��λ�õĶ���
						1 ɾ����ѡ�洢�������е��Ѷ�����
						2 ɾ����ѡ�洢�������е��Ѷ����ź��ѷ��Ͷ���
						3 ɾ����ѡ�洢�������е��Ѷ����š��ѷ��Ͷ��ź�δ���Ͷ���
						4 ɾ����ѡ�洢�������ж���
*                 
* Return        : 1���ɹ� 0������
*******************************************************************************/
INT8U ME909S_SMS_Delete(INT8U Index,INT8U opt)
{
	TCHAR  Buff[20]={0};
	
	sprintf(Buff, "AT+CMGD=%d,%d\r\n",Index,opt);
	
	if(ME909SCommandP(Buff, "OK", 5)) return 1;
	return 0;
}

/*******************************************************************************
* Function Name :INT8U ME909S_SMS_Send(INT8U *pIn, INT8U InLen,INT8U *DstPhoneNum)	                             
* Description   :ģ�鷢�Ͷ��ţ���TEXTģʽ��  
* Input         :pIn:ָ����Ķ������ݣ�InLen���������ݵĳ��ȣ�DstPhoneNum��Ŀ��绰����
*                 
* Return        : 1:���ͳɹ� 0������ʧ��
*******************************************************************************/
INT8U ME909S_SMS_Send(INT8U *pIn, INT8U InLen,INT8U *DstPhoneNum)
{
	INT8U *pRet=NULL;
	INT8U ST=0;
	TCHAR ATCMGS[]="AT+CMGS=\"00000000000\"\r\n";
	TCHAR TEXT[200]={0};
	
	memcpy(&ATCMGS[9],DstPhoneNum,11);											//����Ŀ��绰����
	memcpy(TEXT,pIn,InLen); 													//��������
	TEXT[InLen]=0x1A;															//��ӷ��ͱ�ʶ��
	InLen+=1;

	pRet=ME909SCommandP(AT_CMGF_TEXT, "OK", 2);     							//���ö���ģʽΪTEXTģʽ  ���粻���� ����TEXTģʽ������ÿ�ζ���������
		if(pRet) 	ST= 1;
		else ST=0;
	
	if(ST)
	{
		pRet=ME909SCommandP(AT_CSMP_GSM, "OK", 2);     							//�����ı����ŵı��뷽ʽ GSM  
		if(pRet) 	ST= 1;
		else ST=0;}	
	
	if(ST)
	{
	pRet=ME909SCommandP(ATCMGS, ">", 2);     			        				//����TEXT����
		if(pRet) 	ST= 1;
		else ST=0;}
	
	if(ST)
	{
		pRet=ME909SCommandP(TEXT, "OK", 2);     			           			//����TEXT����
		if(pRet) 	ST= 1;
		else ST=0;}		
		
	if(ST)	return 1;
		return 0;
}

/*******************************************************************************
* Function Name :INT16U ME909S_Read_SMS(INT8U *pOut)                         
* Description   :ģ���ȡδ������
* Input         :pOut:ָ�򷵻صĶ��Ŵ洢λ��
*                 
* Return        :�������ݳ���   ���ܲ�ֹһ��	0��
*******************************************************************************/
INT16U ME909S_Read_SMS(INT8U *pOut)
{
	INT8U 	ST=0;
	INT8U 	*pRet=NULL;
	INT16U 	SMSLen=0;
	
	pRet=ME909SCommandP(AT_CMGF_TEXT, "OK", 2);     							//���ö���ģʽΪTEXTģʽ  ���粻���� ����TEXTģʽ������ÿ�ζ���������
		if(pRet) 	ST= 1;
		else ST=0;
	
	if(ST)
	{
		pRet=ME909SCommandP(AT_CSMP_GSM, "OK", 2);     							//�����ı����ŵı��뷽ʽ GSM  
		if(pRet) 	ST= 1;
		else ST=0;
	}	
	
	if(ST) SMSLen=LteCommunication((INT8U*)AT_CMGL_UNREAD,sizeof(AT_CMGL_UNREAD)-1,pOut,100);		// ��ȡδ������ 
		
	if(SMSLen>10)																//�����δ����Ϣ��᷵��\r\nOK\r\n������10
	{
		pRet=ME909SCommandP(AT_CMGD_ALL, "OK", 2); 				
		if(pRet) 
		{
			if(SMSLen>200) SMSLen=200;
			return SMSLen;
		}
	}																			//�������Ϣ���س���
	return 0;
																				
}
/*******************************************************************************
* Function Name :INT16U ME909S_Waitfor_SMS(u8 *pOut,u8 timeout_s)                         
* Description   :�ȴ�ģ��Ķ��ŵ���֪ͨ���ܵ�֪ͨ���ȡ���ţ����غ�ģ�������Ϣ�Ķ��ţ�����װ��
* Input         :pOut:ָ�򷵻صĶ��Ŵ洢λ�ã�u8 timeout����ʱʱ�䣬��λΪ�롣
*                 
* Return        :0����ʱ��SMSLen���������ݳ���
*******************************************************************************/
INT16U ME909S_Waitfor_SMS(u8 *pOut,u8 timeout_s)
{
	INT8U 	i=0,Err=0;
	INT8U 	*pRet=NULL;
	INT16U 	SMSLen=0;
	
	/*LTEģ���޷����׹رջظ������Ե��²�����timeout���г�ʱ�����ʱ����ѭ���ֽ���*/
	while(timeout_s--)															//timeout_s��λΪ��
	{
		StopModeLock++;
		/*װ������״̬���ȴ�����*/
		pGprsMess = (struct Str_Msg *)OSMboxPend(GPRSBOX,20,&Err);   			//�ȴ�������Ϣ1��
		if(StopModeLock) StopModeLock--;
		if(Err==OS_NO_ERR && pGprsMess)											//����յ�����֪ͨ�����ȡ����
		{	
			if((pGprsMess->DivNum==3)&&(pGprsMess->DataLen>10))					//��΢�����´��󴮿ڴ���ѶϢ
			{	
				pRet=pGprsMess->pData;
				for(i=0;i<pGprsMess->DataLen;i++)
				{
					if(!memcmp(pRet,(INT8U*)"+CMTI:",6))						//����Ƕ��Ŵﵽ�Զ��ϱ�
					{	
						SMSLen=ME909S_Read_SMS(pOut);							//���ȡδ�����ţ���������ɾ���Ѷ�����	
						BSP_UART_RxClear(pGprsMess->DivNum);					//�������ָ��λ��
						return SMSLen;					
					}
					pRet++;
				}
			}
			BSP_UART_RxClear(pGprsMess->DivNum);								//�������ָ��λ��
		}
	}
	return 0;
}
																				

/*******************************************************************************
* Function Name :INT8U *ME909S_SMS_Extract(u8 *pIn,INT8U InLen,INT8U *OutLen)                        
* Description   :�Ӵ�ģ���ʽ�ķ����У���ȡ������Ч����
* Input         :pIn:�������ָ�루��ģ���ʽ��  InLen�����볤��
*                OutLen�����ض�����Ч���ݵĳ���
* Return        :��������ָ��
*******************************************************************************/
INT8U *ME909S_SMS_Extract(u8 *pIn,INT8U InLen,INT8U *OutLen)
{
	INT8U i=0;
	
	if(!InLen||!pIn) return NULL;
	
	for(i=0;i<InLen;i++)
	{
		if(!memcmp(pIn,(INT8U*)"\"\r\n",3))
		{
			InLen-=3;
			pIn+=3;
			OutLen[0]=InLen;
			return pIn;	
		}
		pIn++;
		InLen--;
	}	
	return NULL;
}



/*========================================================����ͳ����ز�������========================================================*/

/*******************************************************************************
* Function Name :INT8U Update_Flow_DayAndMonth(INT32U In)   
* Description   :����λ�������е���������������ͳ��
* Input         :In�������ĵ����� byte
*                 
* Return        :U32���ݣ�0����ȡʧ�ܣ�
*******************************************************************************/
INT8U Update_Flow_DayAndMonth(INT32U In)
{
	Local_FLow_Data.Flow_Day_Used_B		+=	In;									//������
	Local_FLow_Data.Flow_Month_Used_B	+=	In;									//������

	if(BSP_WriteDataToFm(Flow_Data_Addr,(u8*)&Local_FLow_Data,Flow_Data_Len))	//д������
		return 1;
	return 0;
}

/*******************************************************************************
* Function Name :void ClearFlowDataDailyAndMonthly(struct BSPRTC_TIME *pTime)	 
* Description   :ÿ��������������ͳ�ƣ�ÿ�³����������ͳ�ơ�
* Input         :*ptime��������ʱ����            
* Return        :��
*******************************************************************************/
void ClearFlowDataDailyAndMonthly(struct BSPRTC_TIME *pTime)										
{	
	if(Local_FLow_Data.Date != pTime->Day)										//���ڱ��ʱ
	{
		Local_FLow_Data.Flow_Day_Used_B=0;										//����������
		Local_FLow_Data.Date = pTime->Day;
		if(Local_FLow_Data.Month != pTime->Month)								//�·ݱ��ʱ
		{
			Local_FLow_Data.Flow_Month_Used_B=0;								//����������
			Local_FLow_Data.Month = pTime->Month;
		}
	}	
	else return ;																//�¡���δ�����ı�						
	BSP_WriteDataToFm(Flow_Data_Addr,(u8*)&Local_FLow_Data,Flow_Data_Len);		//�����д������
}


/*******************************************************************************
* Function Name :INT8U ME909S_TEST(void)                  
* Description   :ģ�����
* Input         :��           
* Return        :�ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U ME909S_TEST(void)
{
	INT8U	retry = 0;
	INT8U	SOCKID = 1;   														// ֱ��ָ��Socket��Ϊ1

/*���ܹ���ģʽ�ж�*/
	while(Equipment_state.BAT_Volt<BAT_UNDER && Equipment_state.FALA_Volt<FALA_UNDER)	//BAT<9.2V��FALA<5V
	{
		BspUartWrite(2,SIZE_OF("����ģʽ�С���\r\n"));
		OSTimeDly(3*60*20);														//3min��ѯ
		Get_Voltage_MCUtemp_Data( 3 );											//��ȡ��ص�ѹ���ݺ͵�Ƭ���¶�
	}
	
/*LTEģ�鿪��*/
	BspUartWrite(2,SIZE_OF("----------------------------------LTEģ�鿪��������\r\n"));OSTimeDly(1);
	ME909SInit(ME909SBaudrate);
	for(retry=0;retry<3;retry++)												//�������3��
	{
	/*LTEģ�����*/
		if(!ME909S_ON())														//ģ�鿪���������ô���
		{
			ME909S_PW_OFF();													//�ص�Դ
			OSTimeDly(3*20);
			continue;															//ʧ��ʱ���ԣ�����������䣩
		}
		if(!ME909S_SMS_CFG()) continue; 										//��������			
		if(!ME909S_CONFIG()) continue;     						 				//ME909S���б�������
		if(!ME909S_REG()) continue;												//����ע��
		if(!ME909S_Link(SOCKID)) continue;										//���Ӳ���͸��
		
	/*���Կ�������*/
		BspUartWrite(2,SIZE_OF("----------------------------------�ѽ�������״̬\r\n"));OSTimeDly(1);	
		if(!Startup_Comm(RETRY,TIMEOUT)) return 0;								//��������ͨ�ţ�ʧ��ʱ����0
		BspUartWrite(2,SIZE_OF("\r\n----------------------------------���˳�����״̬\r\n"));OSTimeDly(1);
		break;
	}
	
/*LTEģ��ػ�*/	
	BspUartWrite(2,SIZE_OF("----------------------------------LTEģ��ػ�\r\n"));OSTimeDly(1);
	ME909S_OFF();			  													//�ػ�
	ME909S_LowPower();															//�͹���
	return 1;
}
