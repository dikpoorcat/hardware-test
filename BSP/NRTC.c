#include "NRTC.h"



/*�����ڲ�RTC�ж�*/
void RTC_EXTI_INITIAL(FunctionalState interrupt_en_or_dis)
{
	NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure; 

//------------EXTI17 ���� -------------------   
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;          		//�ڲ��¼�ͨ�����ж����ӵ�RTC�����¼�
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			//��ALRF=1�������EXTI��������������EXTI�� 17���ж�ģʽ�����������RTC�����жϣ������EXTI��������������EXTI�� 17���¼�ģʽ�����������ϻ����һ������(�������RTC�����ж�)��
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = interrupt_en_or_dis;
    EXTI_Init(&EXTI_InitStructure); 
//------------���� �ж�------------------- 
    NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQChannel;    //���������ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = interrupt_en_or_dis;
    NVIC_Init(&NVIC_InitStructure);     
//------------------------------------------- 
//	//------------���� �ж�------------------- 
//    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQChannel;    //���������ж�
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = interrupt_en_or_dis;
//    NVIC_Init(&NVIC_InitStructure);     
//------------------------------------------- 
}


/*�ڲ�RTCʱ�ӳ�ʼ��*/
INT8U RTC_FisrtSet=0;
INT8U RTC_Init(INT16U Fre)
{
	INT16U Setcount=0;	
	Setcount=(40000/Fre)-1;     //LSIƵ��Լ40K   ����Ҫ����50msΪһ�������㣬��Ƶ��Ϊ20Hz����Ҫ���õļ�����ֵ����40K/20-1=1999.
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��   	   				RTC�Ĵ���λ�ں�����							
	PWR_BackupAccessCmd(ENABLE);									//ʹ�ܺ󱸼Ĵ�������  
//	BKP_DeInit();													//��λ�������� 	���ɿ��������屸�ݼĴ�����
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);							//����RTCʱ��(RTCCLK),ѡ��LSI��ΪRTCʱ��    
	RCC_RTCCLKCmd(ENABLE);											//ʹ��RTCʱ��  
	RTC_WaitForLastTask();											//�ȴ����һ�ζ�RTC�Ĵ�����д�������
	RTC_WaitForSynchro();											//�ȴ�RTC�Ĵ���ͬ��  
	RTC_EnterConfigMode();      									// ��������	
	RTC_SetPrescaler(Setcount);    								    //����RTCԤ��Ƶ��ֵ  
	RTC_WaitForLastTask();  										//�ȴ����һ�ζ�RTC�Ĵ�����д�������
	RTC_SetCounter(0);												//�����ʼ��һ��ʱ�䣬�Ӵ�ʱ��ʼ����
	RTC_ExitConfigMode();											//�˳�����ģʽ  
		
//	RTC_EXTI_INITIAL(ENABLE);										//RCT�ж����ã��������ڲ�	�¼�ͨ����ALARM�ж�	(�����ܵ��жϷ�����)
	RTC_WaitForLastTask();		
	RTC_ITConfig(RTC_IT_SEC|RTC_IT_OW,DISABLE);   		            //��ʼ��ʱ�����ж�����жϹر�
	RTC_WaitForLastTask();											//�ȴ����һ�ζ�RTC�Ĵ�����д�������
	RTC_ITConfig(RTC_IT_ALR,ENABLE);							    //�����жϴ�
	RTC_WaitForLastTask();											//�ȴ����һ�ζ�RTC�Ĵ�����д�������
	return 1; //ok
}

/*��������*/
void RTC_SET_ALARM(u32 sec)
{
  //DEBUG_COM_STREAM("-����-",NULL);
  RTC_SetAlarm(RTC_GetCounter()+sec);
  //DEBUG_COM_STREAM("-����1-",NULL);
  RTC_WaitForLastTask();
  //DEBUG_COM_STREAM("-����2-",NULL);
  RTC_ITConfig(RTC_FLAG_ALR,ENABLE);
}


/*���յ����ӻ�������  5S*/
void RTC_AWU_SET(void)
{
  //����PWR��BKP��ʱ�ӣ�from APB1��
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  //�������
  PWR_BackupAccessCmd(ENABLE);
  RTC_ITConfig(RTC_IT_SEC, DISABLE);   //�ص����ж�
  RTC_SET_ALARM(5);
  //PWR_BackupAccessCmd(DISABLE);
  RTC_EXTI_INITIAL(ENABLE);
}



