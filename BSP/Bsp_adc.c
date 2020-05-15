/***********************(C) COPY RIGHT 2008 Jking Group**************************
* File Name: Bsp_adc.c
* Author: GuoWei Dong
* Date First Issued: 2015-05-18
* Version: 1.0.0
* Description: AD����
*********************************************************************************
* History:
* 2018.08.30
* Description: Z.E.����
*							 
********************************************************************************/
#include "Bsp_adc.h"

#define ADC1_DR_Address    ((u32)0x4001244C) 

INT16U		AD_Value[3];													//����һ·�ɼ������ڷ������ݵ�ѹ�ɼ���DMA�ã�
Equi_STA	Equipment_state={ 0, 0, 0 };									//��������Լ��֡ʱ����ֱ��memcpy


/******************************************************************************* 
* Function Name  : void ADC1_Configuration(void) 
* Description    : ADC1���ã�ת��˳��DMA����
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/
 void ADC1_Configuration(void) 
{
	ADC_InitTypeDef ADC_InitStructure; 

	ADC_Cmd(ADC1, DISABLE);  
	ADC_DMACmd(ADC1, DISABLE);    //�ر�ADC1,ADC1 DMA

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                                  //����ADCģʽ
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;                       	         		//ADCɨ��ģʽ��ʹ��DMA
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                                  //����ת��
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;                 //�������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                              //�Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 3;                                             //ʹ������ͨ��
	ADC_Init(ADC1, &ADC_InitStructure);                                                 //ADC��ʼ��
	ADC_TempSensorVrefintCmd(ENABLE);                                                   //�����ڲ��¶ȴ�����ͨ��
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_239Cycles5);        //PC1��ӦΪADC channel 11 ��һ˳��ת��������ʱ��41.5������
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 2, ADC_SampleTime_239Cycles5);        //PC2��ӦΪADC channel 12 �ڶ�˳��ת��������ʱ��41.5������
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 3, ADC_SampleTime_239Cycles5);       //�ڲ��¶ȴ����� channel 16 ���ڶ�˳��ת��������ʱ��239.5������
	ADC_Cmd(ADC1, ENABLE);  
	ADC_DMACmd(ADC1, ENABLE);                                                           //ʹ��ADC��ʹ��ADC DMA
	// Enable ADC1 reset calibaration register ����У׼
	ADC_ResetCalibration(ADC1);  // Check the end of ADC1 reset calibration register
	while(ADC_GetResetCalibrationStatus(ADC1)); 
	// Start ADC1 calibaration
	ADC_StartCalibration(ADC1);  
	while(ADC_GetCalibrationStatus(ADC1)); // ADCУ׼���
}

/******************************************************************************* 
* Function Name  : void DMA_Configuration(void) 
* Description    : DMA���á�CMAR=AD_Value����ĵ�ַ
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/ 
void DMA_Configuration(void) 
{ 
    DMA_InitTypeDef DMA_InitStructure;   
	DMA_Cmd(DMA1_Channel1, DISABLE); 	                                                //ADC1��ӦDMA channel 1
    DMA_DeInit(DMA1_Channel1);                                                          //��λ
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;                         //CPAR= ADC1��DR�Ĵ�����ַ
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&AD_Value;                              //CMAR=AD_Value����ĵ�ַ
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                                  //���䷽����ADC���洢��    
    DMA_InitStructure.DMA_BufferSize = 3;                                               //������ 3��
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                    //�����ַ������
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                             //�洢����ַ����
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;         //ADC���ݿ��16λ
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;                 //�洢���������ݿ��16λ
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                                     //ѭ������
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;                                 //���ȼ���
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                        //�ر�M2M
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);                                        //��ʼ�� DMA CH1��ADC1������
    DMA_Cmd(DMA1_Channel1, ENABLE);                                                     //����DMA����
} 

/******************************************************************************* 
* Function Name  : void AD_Init(void)
* Description    : ADC1��ʼ��
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/ 
void AD_Init(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOC ,ENABLE);			//ʹ��ADC1ʱ�ӣ�ʹ��PC��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);					//��������ʱ�ӣ�PC
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);						//ʹ��DMA1ʱ��
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);										//ADCʱ��8��Ƶ 

	/*ADC����ģ��������*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;					//PC1 ģ����������ADC_BAT��⣬PC2 ģ����������ADC_FALA���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	/*ʹ��������������ߵ�ƽ*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;						//�������
	GPIO_InitStructure.GPIO_Pin = ADCEN_Pin;								//ADCʹ��
	GPIO_SetBits(ADCEN_Port, ADCEN_Pin);									//����
	GPIO_Init(ADCEN_Port, &GPIO_InitStructure);								

	ADC1_Configuration();
	DMA_Configuration();
}

/******************************************************************************* 
* Function Name  : void AD_LowPower(void)
* Description    : ADC1ʱ�ӹر�ʡ��
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/ 
void AD_LowPower(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	ADC_DeInit(ADC1);														//û�����������46uA
	ADC_Cmd(ADC1, DISABLE);													//��ADC����
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,DISABLE);					//��ADCʱ��		���ظ���ʱ��|RCC_APB2Periph_AFIO������Ҳ�õ������  ʵ��������ʱ�ӹز��ض���Ӱ��
	
	ADC_DMACmd(ADC1, DISABLE);												//�ر�ADC1 DMA
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);						//��DMAʱ��

	/*ʹ�����š�����ADC��������ģ������*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;						//ADC�������ű�������ģ�����룬��50MHz��Ϊ2MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;					//PC1��PC2
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
//	GPIO_InitStructure.GPIO_Pin = ADCEN_Pin;								//ADCʹ��
//	GPIO_Init(ADCEN_Port, &GPIO_InitStructure);
	GPIO_ResetBits(ADCEN_Port, ADCEN_Pin);									//����
}
	
/******************************************************************************* 
* Function Name  : u32 GET_ADVALUE(u8 ch,u8 CISHU)   
* Description    : ��ȡADת�����
* Input          : ch��ͨ��
					#define CH_BAT 0
					#define CH_FALA 1
					#define CH_MCUWD 2
	               count��ȡ���ε�ƽ��ֵ
* Output         : 
* Return         : ת����12λAD����ֵ
*******************************************************************************/ 
u32 GET_ADVALUE(u8 ch,u8 count)       
{
	u32		ADvalue=0;														//typedef unsigned long	u32
	INT8U	i;
	INT16U	temp[20]={0};													//count���20�β���
  
	for(i=0;i<count;i++)
	{
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);								//����ADC1ת��
		delay_2us_4M(500*50);												//��ʱ20ms
//		OSTimeDly(1);
		BSP_WDGFeedDog(); 
		temp[i]=AD_Value[ch];												//����DMA�����ADC CH_X����ֱ�Ӵ��䵽AD_Value[X]
	}
	for(i=0;i<count;i++)
	{
		ADvalue+=temp[i];			 
	}
	ADvalue /= count;														//ȡ����ƽ��ֵ����Ϊ12λADֵ

	return ADvalue;		
}
	
/******************************************************************************* 
* Function Name  : u32 Read_Voltage(u8 ch,u8 count)  
* Description    : ��ȡ��ػ������ݵĵ�ѹ����ѹϵ����1/4����ⷶΧ0-13.2V
* Input          : ch��ͨ�� CH_BAT 0||CH_FALA 1
	               count��ȡ���ε�ƽ��ֵ
* Output         : 
* Return         : ��ػ������ݵ�ѹֵ����ƽ��ֵ��ʵ��ֵ��100��������λ10mV,ȡС�������λ
*******************************************************************************/ 
u32 Read_Voltage(u8 ch,u8 count)
{
	u32 Voltage;

	Voltage=GET_ADVALUE(ch,count);
	Voltage=Voltage*330/4096*4;												//��λ10mV,ϵ��4��Ĭ�ϲο���ѹ3.3V
	
	Voltage = Voltage-4.9;													//����
	
	return Voltage;       
}

/******************************************************************************* 
* Function Name  : u32 Read_MCU_Temp( u8 count )
* Description    : ��ȡMCU�ڲ��¶ȣ�����count�β���ȡƽ��ֵ���������������λ����*100
* Input          :  u8 count ����������
* Output         : 
* Return         : ����MCU�¶ȵ�100����ȡС�������λ
*******************************************************************************/ 
u32 Read_MCU_Temp( u8 count )
{
	u32 Temp;
	u32 MCU_Temp; 
	Temp=GET_ADVALUE(CH_MCUWD,1);
	
	MCU_Temp=(u32)((1.43-Temp*3.3/4096)*1000/4.3+25)*100 ; //ȡС�������λ
	return MCU_Temp;   //�����¶ȵ�100��
}

/******************************************************************************* 
* Function Name  : void ADC_HEXtoASCII(u32 In,u8 *pOut)
* Description    : ����ѹֵ���¶�ת��Ϊ�ַ����
* Input          : u32 In
* Output         : u8 *pOut
* Return         : ��
*******************************************************************************/ 
void ADC_HEXtoASCII(u32 In,u8 *pOut)
{
	int i=0;
	for(i=0;i<5;i++)
	{
		if(i==2) continue ; 
		pOut[4-i]=In%10+0x30;
		In/=10;
	}
	pOut[2]='.';
    pOut[5]=0x0d;
	pOut[6]=0x0a;
}

/******************************************************************************* 
* Function Name  : void ReadVCC_Test(void)
* Description    : ���Դ�ӡV_BAT��V_FALA��MCUWD
* Input          : ��
* Output         : ��
* Return         : ��
*******************************************************************************/ 
void ReadVCC_Test(void)
{
	u32 Vbat=0,Vfala=0,Tmcu=0;

	AD_Init();
	Vbat=Read_Voltage(CH_BAT,5);											//ȡ5�β���ƽ��ֵ 
    Print_Voltage( CH_BAT, Vbat );
	
	Vfala=Read_Voltage(CH_FALA,5);
	Print_Voltage( CH_FALA, Vfala );
	
	Tmcu=Read_MCU_Temp( 5 );
	Print_Voltage( CH_MCUWD, Tmcu );
	
	OSTimeDly(5);															//�ȴ���ӡ���
}

/*******************************************************************************
���ƣ�void Print_Voltage( INT8U Channel, INT32U Voltage )
���ܣ�ѡ����Ҫ��ӡ�����ͣ�������ֵ���ɴ�485��ӡ��������Ϣ
��Σ�INT8U Channel, ͨ����INT32U Voltage����Ҫ��ӡ��ֵ
		#define CH_BAT 0
		#define CH_FALA 1
		#define CH_MCUWD 2
���Σ���
���أ���
*******************************************************************************/
void Print_Voltage( INT8U Channel, INT32U Voltage )
{
	INT8U	temp[7]={0};
	
	ADC_HEXtoASCII( Voltage, temp );											//ת��Ϊ�ַ���
	
	switch( Channel ){
		case CH_BAT:
			BspUartWrite(2,SIZE_OF("V_BAT---->"));								//���V_BAT
			break;
		case CH_FALA:
			BspUartWrite(2,SIZE_OF("V_FALA--->"));								//���V_FALA
			break;
		case CH_MCUWD:
			BspUartWrite(2,SIZE_OF("MCUWD---->"));								//�����Ƭ���¶�
			break;
		default:
			BspUartWrite(2,SIZE_OF("δָ��ͨ��"));
			break;
	}
	BspUartWrite( 2, temp, 7 );
}

/*******************************************************************************
���ƣ�void Get_Voltage_MCUtemp_Data( INT8U Count )
���ܣ���ȡ��ص�ѹ���ݺ͵�Ƭ���¶ȣ���β���ȡƽ��ֵ��ת��Ϊ���㣬����Equipment_state�ṹ�壬
�����й����жϡ���485�ѿ��ţ��ɽ��ɼ��������ݺ͹�����Ϣ����485��ӡ��������������485��ʼ������
��Σ�INT8U Count�������Ĵ���
���Σ���
���أ���
*******************************************************************************/
void Get_Voltage_MCUtemp_Data( INT8U Count )
{
	INT32U	Vbat=0,Vfala=0,Tmcu=0;
	
	/*���ϴ�����*/
	Equipment_state.BAT_Volt= 0;											
	Equipment_state.FALA_Volt= 0;
	Equipment_state.MCU_Temp= 0;
	
	/*�ɼ�*/
	AD_Init();																//��ʼ��ADC
	Vbat= Read_Voltage( CH_BAT, Count );									//��ص�ѹ����Count�Σ�����ƽ��ֵ��ʵ��ֵ��100������INT32U
	Vfala= Read_Voltage( CH_FALA, Count );									//�������ݵ�ѹ����Count�Σ�����ƽ��ֵ��ʵ��ֵ��100������INT32U
	Tmcu= Read_MCU_Temp( Count );											//��Ƭ���¶ȵ�ѹ����Count�Σ�����ƽ��ֵ��ʵ��ֵ��100������INT32U
    AD_LowPower();

	/*��ӡ������Ϣ��Ĭ�������ѿ���485*/
	Print_Voltage( CH_BAT, Vbat );
	Print_Voltage( CH_FALA, Vfala );
	Print_Voltage( CH_MCUWD, Tmcu );
	
	/*����ṹ��*/
	Equipment_state.BAT_Volt= ( (float)Vbat ) / 100;						//ת��Ϊ���㣨��̨Ҫ����������ͣ���������ʵֵ��ת�浽Equipment_state��ԭΪ100����
	Equipment_state.FALA_Volt= ( (float)Vfala ) / 100;						//ת��Ϊ���㣨��̨Ҫ����������ͣ���������ʵֵ��ת�浽Equipment_state��ԭΪ100����
	Equipment_state.MCU_Temp= ( (float)Tmcu ) / 100;						//ת��Ϊ���㣨��̨�޴�Ҫ�����������뻷���¶���ͬ����������ʵֵ��ת�浽Equipment_state��ԭΪ100����
	
	/*�����ж�*/
	if((Equipment_state.BAT_Volt<BAT_UNDER) && (!Fault_Manage.F_BAT)) 		//��ص�ѹ<9.2V�����޹��ϱ�־
		NW_Fault_Manage(BAT_F, FAULT_STA);									//�ϱ����Ƿѹ����		
	else if((Equipment_state.BAT_Volt>BAT_UP) && Fault_Manage.F_BAT)		//��ص�ѹ>9.5V�����й��ϱ�־	
		NW_Fault_Manage(BAT_F, NOFAULT_STA);								//�ϱ���ѹǷѹ���ϻָ�	
}

/*******************************************************************************
���ƣ�INT8U HB_Get_Voltage( INT8U Count )
���ܣ���ȡ��ص�ѹ���ݺ͵�Ƭ���¶ȣ���β���ȡƽ��ֵ��ת��Ϊ���㣬����Equipment_state�ṹ��
	��485�ѿ��ţ��ɽ��ɼ��������ݴ�485��ӡ��������������485��ʼ����
��Σ�count��ȡcount�β�����ƽ��ֵ
���Σ���
���أ�������֡��Ҫ�ĵ�ص�ѹ��ֵ����λ100mv
*******************************************************************************/
INT8U HB_Get_Voltage( INT8U Count )
{
	INT8U Vbat;
	AD_Init();	
	Vbat=(INT8U) (Read_Voltage(CH_BAT,Count)/10);							//����һ���ֽڣ���λ100mv														
	AD_LowPower();
	return Vbat;
}
