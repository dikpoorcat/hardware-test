#ifndef	__ME909S_H
#define __ME909S_H
#include "main.h"
/*�������ź궨��*/
////-----------------------------------���ż��绰�������------------------------------------------
//#define SMS_PIN         	GPIO_Pin_1
//#define SMS_Port       	GPIOB

//-----------------------------------��Դ��ؿ�������------------------------------------------				//2018.08.10 CW�޸� ����һ����Դ��ѹ��������
#define BAT_CTL_PIN  		GPIO_Pin_11 													
#define BAT_CTL_Port 		GPIOC 
#define BAT_CTL_PIN_H()		GPIO_SetBits(BAT_CTL_Port,BAT_CTL_PIN);			//ǿ�ƴ򿪵��
#define BAT_CTL_PIN_L()		GPIO_ResetBits(BAT_CTL_Port,BAT_CTL_PIN);

//-----------------------------------GPRS��Դ��������------------------------------------------
#define PWGPRS_PIN			GPIO_Pin_10										//ȫ��ͨ��վ����
#define PWGPRS_Port			GPIOC 
#define PWGPRSEN()			GPIO_SetBits(PWGPRS_Port,PWGPRS_PIN);
#define PWGPRSDIS()			GPIO_ResetBits(PWGPRS_Port,PWGPRS_PIN);


#define ME909S_REST_PIN		GPIO_Pin_12
#define ME909S_REST_PORT 	GPIOB
#define ME909S_REST_H()   	GPIO_ResetBits(ME909S_REST_PORT, ME909S_REST_PIN)				//����ҲҪ������  2018.7.18 CW �޸�
#define ME909S_REST_L()   	GPIO_SetBits(ME909S_REST_PORT, ME909S_REST_PIN)

#define ME909S_ONOFF_PIN    GPIO_Pin_13  
#define ME909S_ONOFF_PORT   GPIOB        
#define ME909S_ONOFF_H()   	GPIO_ResetBits(ME909S_ONOFF_PORT, ME909S_ONOFF_PIN)             //�����Ƿ�������
#define ME909S_ONOFF_L()   	GPIO_SetBits(ME909S_ONOFF_PORT, ME909S_ONOFF_PIN)

#define ME909S_PW_PIN       GPIO_Pin_10       
#define ME909S_PW_Port      GPIOC      
#define ME909S_PW_ON()      GPIO_SetBits(ME909S_PW_Port,ME909S_PW_PIN);
#define ME909S_PW_OFF()     GPIO_ResetBits(ME909S_PW_Port,ME909S_PW_PIN);




/*�����궨��*/
#define ME909SBaudrate		9600
#define ME909SPort			3 //4Gģ��ʹ�ô���3

#define GPRSRXDATA			4 //���յ�����״̬ GPRS ���յ�����
#define GPRSTXDATA   		5 //GPRS ��������

#define GPRSDATA     		6 //���ݴ�����
#define GPRSWAITDATA     	7 //




/*ATָ��궨��*/
#define AT_CNUM				"AT+CNUM\r\n"
#define AT_ICCID			"AT^ICCID?\r\n"
#define AT_IMSI				"AT+CIMI\r\n"
#define AT_AT				"AT\r\n"          			   						//4   ����ͨѶ
#define AT_ATE				"ATE0\r\n"		    								//6   �ر�ָ�����
#define AT_MSO				"AT^MSO\r\n"	          			    			//8   �ػ�(���ȴ�)���ر�ģ���Դ����ʱ����29302�ĵ�Դ�����ţ���ģ���1.8Vû������ˣ�˵����Դ�������뵽ģ��
#define AT_IPRCHECK			"AT+IPR?\r\n"   									//9   ��ѯ������
#define AT_IPR9600			"AT+IPR=9600\r\n"    								//13  �̶�9600������
#define AT_IPR115200		"AT+IPR=115200\r\n"    								//15  �̶�115200������
#define AT_HCSQ				"AT^HCSQ?\r\n"         								//10  ��ѯ�ź�ǿ��
#define AT_CURC				"AT^CURC=0\r\n"    									//11  �ر��Զ��ϱ�^RSSI,^MODE,��һЩ���߰���Ķ���
#define AT_IPCFL5			"AT^IPCFL=5,2\r\n"        							//14  ����͸����ʱ���Ͷ�ʱ������0.1s*2
#define AT_IPCFL10			"AT^IPCFL=10,1472\r\n"  							//18  ����͸����buffer���ȣ���󳤶�1472
#define AT_IPCFL12			"AT^IPCFL=12,0\r\n"     							//15  ����͸��ģʽ0������ͨ����bufferģʽ����buffer�Զ����ͣ�δ���ȴ���ʱ���ͣ��ر�͸��ֱ�ӷ���
#define AT_ANT				"AT^ANTMODE=0,0\r\n"								//16  ��������ģʽ ����10:ȫ����1��������2���ּ� ����2��0��ȫ��ʽ��1��������2��WCDMA��3��LTE  ���籣��
#define AT_ANTCHECK			"AT^ANTMODE?\r\n"									//13  ��ѯ����ģʽ
#define AT_LED				"AT^LEDCTRL=1\r\n"	        						//14  ����LED����ģʽ��Ĭ�ϣ�δע������2s�ڿ������Σ���ע������2s�ڿ���1�Σ��Ѳ������ӳ���  �����õ��籣��
#define AT_LEDCHECK			"AT^LEDCTRL?\r\n"	    							//13  ��ѯLEDģʽ
#define AT_CMEE				"AT+CMEE=2\r\n"    									//11  ���ñ���ֱ������ַ�������Ĭ�ϵ����������룬˭�п�����ȥ���ֲ�
/*����3����������������ʽѡ��Ĭ���Զ�ģʽ*/
#define AT_FREQLOCKCHECK	"AT^FREQLOCK?\r\n"									//��Ƶ��ѯ
#define AT_SYSCFGEX			"AT^SYSCFGEX=\"0102\",3FFFFFFF,1,2,7FFFFFFFFFFFFFFF,,\r\n"	//14  ������չϵͳ  010203:GSM->UMTS->LTE   00:�Զ�ģʽ  ���籣��
#define AT_SYSCFGEXCHECK	"AT^SYSCFGEX?\r\n"									//14  ��չϵͳ���ò�ѯ
#define AT_CPIN				"AT+CPIN?\r\n"		          						//10   ��ѯSIM���Ƿ����
#define AT_COPS				"AT+COPS=0\r\n"		          						//11   ������Ӫ��ѡ��ģʽΪ�Զ�ѡ��	
#define AT_COPSCHECK		"AT+COPS?\r\n"		          						//10   ��ѯ��Ӫ��	
#define AT_CREG				"AT+CREG=1\r\n"										//11   ע��GSM״̬�Զ��ϱ�	
#define AT_CGREG			"AT+CGREG=1\r\n"		         				 	//12   ע��GPRS״̬�Զ��ϱ�	
#define AT_CREGCHECK		"AT+CREG?\r\n"		          						//10   ��ѯ��ǰע��GSM״̬	
#define AT_CGREGCHECK		"AT+CGREG?\r\n"		          						//11   ��ѯ��ǰע��GPRS״̬
#define AT_SYSINFO			"AT^SYSINFOEX\r\n"									//14   ��ѯ��չϵͳ��Ϣ
#define AT_CSQ				"AT+CSQ\r\n"		          			 			//8    ��ѯ�ź�ǿ��
#define AT_CFUN				"AT+CFUN=1,0\r\n"		          					//13   ����ģ��Ϊonline mode���Ҳ�����ģ��
#define AT_CFUNCHECK		"AT+CFUN?\r\n"		          						//10   ��ѯģ��ģʽ
#define AT_IPCFLCHECK		"AT^IPCFL?\r\n"			    						//11   ��ѯTCP/UDP ��̬�������ý��
#define AT_IPINITCHECK		"AT^IPINIT?\r\n"									//12   ��ѯTCP��ʼ�����
#define AT_OPENCHECK		"AT^IPOPEN?\r\n"									//12   ��ѯ�Ѵ򿪵�IP
#define AT_IPENTRANS		"AT^IPENTRANS=1\r\n"								//16 ��͸�����䣬����һ�����ӵ�ʱ�򷽿�ʹ�ã��������
#define AT_IPENTRANSCHEC	"AT^IPENTRANS?\r\n"									//15 ��ѯ͸���Ƿ��
#define AT_IPDISTRANS		"+++"												//3  �˳�͸��ָ��������ڴ�͸��ģʽ�л�������ģʽ  ���ָ��ܼ�\r\n
#define AT_CPMS_CHECK		"AT+CPMS=?\r\n"										//��ѯ����Ϣ�洢λ��
#define AT_CMGD_ALL			"AT+CMGD=1,4\r\n"									//ɾ����ѡ�洢�������ж���
#define AT_CMGF_TEXT		"AT+CMGF=1\r\n"										//����TEXTģʽ
#define AT_CMGL				"AT+CMGL=\"ALL\"\r\n"								//��ȡ���н��յ��Ķ���
#define AT_CNMI				"AT+CNMI=2,1,0,0,0\r\n"								//���ö��Ž���ģʽStores the message on the SIM card or ME, and presents the new message indication.
#define AT_CSCS_GSM			"AT+CSCS=\"GSM\"\r\n"								//����TE�ַ���ΪGSM 7bit	
#define AT_CSMP_GSM			"AT+CSMP=,,,0\r\n"									//����TEXTģʽ�µĲ���,�����������ݱ����ʽΪGSM 7bit
#define AT_CPMS				"AT+CPMS=\"SM\",\"SM\",\"SM\" \r\n"		    		//���ö���Ϣ��ȡ��ɾ������/д��ͷ��ͽ���/���ܽ���		��֧�֣�U��SIM��
#define AT_CMGF_PD			"AT+CMGF=0\r\n"										//���ö���Ϣ��ʽΪPDU��TEXT��ʽ���=1   ���粻���棬ȱʡΪPDU
#define AT_CMGF_TEXT		"AT+CMGF=1\r\n"										//���ö���Ϣ��ʽΪPDU��TEXT��ʽ���=1   ���粻���棬ȱʡΪPDU
#define AT_CSMP_GSM			"AT+CSMP=,,,0\r\n"									//����TEXTģʽ�µĲ���,�����������ݱ����ʽΪGSM 7bit
#define AT_CMGF_TEXT		"AT+CMGF=1\r\n"										//����TEXTģʽ
#define AT_CMGL_UNREAD		"AT+CMGL=\"REC UNREAD\"\r\n"						//��ȡ�յ���δ������
#define AT_CMGD_ALL			"AT+CMGD=1,4\r\n"									//ɾ����ѡ�洢�������ж���
#define AT_CSMP_GSM			"AT+CSMP=,,,0\r\n"									//����TEXTģʽ�µĲ���,�����������ݱ����ʽΪGSM 7bit




/*�ṹ�嶨��*/
typedef struct GPRS_CanHX_
{
	INT8U  YanShiOFFTime;  // ��ʱ�ر�IP����ʱ��
	INT8U  StartHour;
	INT8U  StartMin;
	INT8U  EndHour;
	INT8U  EndMin;	  	
	INT16U WakeUp_cycle;   // ��������   ��λS
	INT16U Wakeing_Time;  // ���ѵ�ʱ�� ��λS
}GPHX_;

typedef struct SMS_Struct
{
	INT8U SaveIndex;    // �����λ��
	INT8U Stat;         // 0��δ����1���Ѷ� ��2���洢δ����3�� �洢�ѷ�
	
	INT8U DataLen;      // ����Ϣ�ĳ��� ������ ���ĺ���
	INT8U CSALen;       // ���ĺ���ĳ���
	INT8U DSCType;      // ��������

	INT8U OaLen   ;     // ��Ϣ��Դ��ַ�ĳ���
	
	INT8U CSTSTime[7];  // ���Ž���ʱ�䣬�������2�죬���Ž���Ч
	
	INT8U OaAddr[32];   // ������Դ��ַ
	
	INT8U SMSDATALen;   // ����Ϣ����
	
	INT8U SMSDATA[140]; // ����Ϣ����
}SMS_S;

typedef struct gprs_str
{
	INT8U  SOCKID;         // �׽���
	
	INT16U RecvLen;        // ���յ��ĳ���
	INT16U Packet_length;  // ���ĵĳ���
	
	INT8U  Frame_type;     // ֡���� 
	INT8U  Packet_Type;    // ��������
	
	INT8U *RecvDataP;      // ���ݴ��ָ��
}GPRS_STRUCT;

typedef struct A58_Struct														//�ṹ�����ָ�һ��
{
		INT8U A58stats;    //5800  ����״̬
		INT8U A58BKPstats; //5800  ����
		
}A58Control;



/*ȫ�ֱ�������*/





/*��������*/
void ME909S_LowPower(void);
void ME909S_PinInit(void);
void ME909SInit(INT32U rate);																	//MES909S �������ã���Դ���������ų�ʼ��
INT8U ME909S_ON(void);																			//MES909S ģ��Ŀ���  
void ME909S_OFF(void)  ;    																	//MES909S ģ��Ĺػ�  �ҹر�ģ���Դ
void DeviceRstOnCMD(void);																		//�ӵ�װ�����������MES909S ģ��Ĺػ�  �ҹر�ģ���Դ �ٽ��е�Ƭ����λ
INT8U ME909S_RateChange(void);																	//�޸Ĳ�����Ϊ9600 ��ʱ�Ȳ��㲨���ʲ����ˣ��̶��޸�Ϊĳֵ����Ȼ����32λhexתascii�����⣬�������Ӧ�ú�����Ҫ�õ�
INT8U ME909S_CONFIG(void);																		//ģ�鿪������б�������
INT8U ME909S_REG(void);																			//ģ���������ע��
INT8U ME909S_IPINIT(void);																		//ģ�����TCP/IP��ʼ��		UDP/TCP����,�����Ӳ�������ѡ��
INT8U ME909S_IPOPEN(INT8U SocketID,INT8U *IP,INT8U *Remote_Port,INT16U Local_Port)  ;			//ģ��ʹ��һ��socket��һ��IP����
INT8U ME909S_IPCLOSE(INT8U SocketID);															//ģ��ر�IP����
INT8U ME909S_Trans_ON(void)	;																	//ģ�����ò���͸�����ͨ��
INT8U ME909S_Trans_OFF(void);																	//ģ��ر�͸��
INT8U ME909S_Link(INT8U SocketID)	;															//ģ�����IP����
INT8U ME9009S_RecvDataDealWith(INT8U *pIn,INT16U InLen)     ;									//ģ����͸��ģʽ�´��� ģ�鷵�ص����� 
INT8U ME909S_TransSendData(INT8U *pIn,INT16U InLen,INT8U SendPacketType);						//ģ��ͨ��͸��ģʽ��������
INT8U ME909S_RecvDataWait(INT16U Time)	;														//GPRSģ�鴦����յ�������
INT8U ME909S_XTSend(void);																		//ģ�鷢������
INT8U ME909S_WDSend(INT8U *pIn,INT16U InLen);													//ģ�鷢���¶����ݣ�ÿ������3��
INT8U ME909S_Get_Signal_Strength(void);															//���ģ����ź�ǿ�Ȱٷֱ�
INT8U HB_Get_Signal_Strength(void);																//Ϊ������֡��ȡһ�������ź�ǿ�Ȱٷֱ�(��ر�͸���������ѯָ��ٴ�͸��)
INT8U *ME909SCommandP(TCHAR *pAtCmd,TCHAR *pExpResponse,INT16U WaitTime);						//��ģ�鷢��ATָ��ȴ����صĽ���Ƿ���ϣ����ֵ

/*����������A8500.C======================================================================================================================*/
INT8U* GPRS_WaitSign(INT16U WaitTime,INT8U *pDivNum,INT16U *pRxLen);							//GPRSģ��ȴ����ؽ�� 
INT8U Read_GPRS_RxDataFromGPFM(void)  ;															// ������յ�������	
INT8U LTE_Get_Config(void);																		//��ȡGPRS����

/*========================================================����Ϊ��д����������	========================================================*/
INT16U LTE_WaitData(INT8U *OutBuff,INT16U timeout);												//ģ��ȴ���λ���������ݣ��������ݴ���
INT16U LteCommunication(INT8U *InBuff,INT16U InLen,INT8U *OutBuff,INT16U timeout);				//ģ�鷢�����ݲ�������������

/*========================================================		��������		========================================================*/
INT8U ME909S_SMS_CFG(void);																		//ģ����ж��Ź�������
INT8U ME909S_SMS_Send(INT8U *pIn, INT8U InLen,INT8U *DstPhoneNum);								//ģ�鷢�Ͷ��ţ���TEXTģʽ�� 
INT16U ME909S_Read_SMS(INT8U *pOut);															//ģ���ȡδ������
INT16U ME909S_Waitfor_SMS(u8 *pOut,u8 timeout_s);												//ģ��ȴ�����
INT8U ME909S_SMS_Delete(INT8U Index,INT8U opt);													//ģ��ɾ������
INT8U *ME909S_SMS_Extract(u8 *pIn,INT8U InLen,INT8U *OutLen);									//ģ�������ȡ

/*========================================================����ͳ����ز�������	========================================================*/
INT8U Update_Flow_DayAndMonth(INT32U In);														//����λ�������е���������������ͳ��
void ClearFlowDataDailyAndMonthly(struct BSPRTC_TIME *pTime);									//ÿ��������������ͳ�ƣ�ÿ�³����������ͳ��	


INT8U *ScanAsicc(INT8U *InR,INT16U inRlen,INT8U *Asicc,INT8U Asicclen);
INT8U INT8UBCDToAscii(INT8U InData,INT8U *pOut);
INT8U INT16UHexToAscii(INT16U InData,INT8U *pOut);
void INT8UHexToAscii(INT8U InData,TCHAR *pOut);
#endif 
