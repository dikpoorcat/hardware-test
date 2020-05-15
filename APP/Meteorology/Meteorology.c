/*****************************************Copyright(C)******************************************
*******************************************���ϵ���*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName          : Meteorology.c
* Author            : Z.E.
* Date First Issued : 2018-08-30
* Version           : V1.0
* Description       : ΢�������ݴ��� 
 *----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
* History       :V1.0
* Description   : 
	               
*-----------------------------------------------------------------------------------------------
*******************************************************************************/
#include "Meteorology.h"

#define TEST	01									//��ӡ����

//ȫ�ֱ�������
INT16U	WindSpeed=0;								//��еʽ����������
INT16U	WindDirection=0;							//��еʽ����������
BMP180_info	BP_info;								//�������ѹ����������
MET_Data_TypeDef	MET_Data;						//����΢��������



/***************************************************************************************
���ƣ�void MET_Main(void *org)
���ܣ�΢��������������
��Σ���
���Σ���
���أ���
****************************************************************************************/
void MET_Main(void *org)
{
#if 0	/*ADC�궨*/
	INT8U	T=100;
	
	while( T-- )
	{
		if( T==0)
		{						
			B485_init(4800);								//B485���ڳ�ʼ����ͳһ������ٷ��򴫸���Ĭ��ֵ��ͬ��
			PWDC485EN();									//DCDC����485��	
			OSTimeDly(2);			
			T=100;
		}
		ReadVCC_Test();
		BSP_UART_Write(2,(INT8U *)("\r\n"),2);
		OSTimeDly(3*20);
	}
#endif
#if TEST
	Test_Meteorology_Data(1);						//΢�������ݲɼ���ʧ������3�Σ���ʪ�ȡ�����ѹ�����ٷ���
//	SetGPRSON();									//Resume�����GPRS����,GPRSON=1--------------------------------------------------------��ֵʱ����Ҫ��������
#endif
	while(1)
	{				
		WDTClear(0);								//ι����
		OSTimeDly(20);
#if TEST
		if( gRtcTime.Minute == 0x49 || gRtcTime.Minute == 0x19 )				//������һСʱ����
#else
		if( gRtcTime.Minute == 0x49 )				//ÿСʱ49��ʱ��ʼ����ʪ�ȡ�����ѹ�����ٷ������ݣ�RTC������0.9���һ��ʱ�䣩
#endif
		{
			Get_Meteorology_Data(5);				//΢�������ݲɼ���ʧ������5�Σ���ʪ�ȡ�����ѹ�����ٷ���
			//MET_SaveData(CurrentTime);			//���浱ǰʱ�䣬ʪ�ȣ��¶ȣ���ѹ�����٣���������		

			/*��ص�ѹ��⣬�����������*/
		#if TEST
			PWDC485EN();							//DCDC����485�򿪣���ʽ���򲻴�ӡ��ȥ����
		#endif
			Get_Voltage_MCUtemp_Data(3);			//��ص�ѹ����Ƭ���¶Ȳɼ�3��ȡƽ��ֵ���ڴ˴�ӡ��Ϣ��			
			if( Equipment_state.BAT_Volt>8.5 || Equipment_state.FALA_Volt>5 )
			{
//				SetGPRSON();						//Resume�����GPRS����,GPRSON=1--------------------------------------------------------��ֵʱ����Ҫ��������
			}

			OSTimeDly(60*20);						//��ʱ1min����ֹ1����������
		}
	}
}

/***************************************************************************************
���ƣ�INT8U Get_Meteorology_Data( INT8U retry )
���ܣ�10minƽ�����ٷ���������١���ʪ�ȡ�����ѹ���ݲɼ����Ȳɼ�����10min�����ɼ��¶ȵ�
��Σ�INT8U	retry���ɼ�ʧ�����Դ���
���Σ���
���أ�bit1��ʪ�Ȳɼ�����bit2����ѹ�ɼ�����bit3���ٴ���������bit4���򴫸������󣬷���0�ɼ���ȷ
****************************************************************************************/
INT8U Get_Meteorology_Data( INT8U retry )
{
	INT8U 	i,rt,err=0;
	INT8U 	MaxIndex=0;								//������
	INT16U	WD_Max=0,WS_Sum=0;						//���ڼ���ƽ��ֵ
	INT16U	WindSpeed_arr[10]={0};					//10min����
	INT16U	WindDirection_arr[10]={0};				//10min����
	INT16S	Temp_16S=0;								//�з������ͣ����ڸ���ת��
	
	Am2302_Init();									//Am2302��������ʼ��
	PowerMETPin_Init();								//��ʪ�ȵ�Դ���� ��������
	PowerWDSPPin_Init();							//���ٷ����Դ���� ��������
	
	/*10minƽ�����ٷ������ݲɼ���err����ʽҪ���Ȳɼ����ٷ���*/
	for(i=0;i<10;i++)								//�����ɼ�10min���ٷ���
	{
		OSTimeDly( 57*20 );							//��ʱ1min���¾�����3�룩�ٲɼ���50��~59�֣�
		for( rt=retry; rt>0; rt-- )
		{
			err= Get_WDSP_Data( WindSpeed_arr+i, WindDirection_arr+i );	//���ٷ������ݲɼ�������¼�����־λ�������һ�εı�־Ϊ׼������д��|=�������޷������ǰ�Ĵ���
			if( err==0 ) break;						//���ٷ������ݲɼ���ȷ������0��ʱ����������ʱ����ѭ��
		}
		
		if( WindSpeed_arr[i] > WD_Max )				//�ж�������
		{
			WD_Max= WindSpeed_arr[i];				//����������
			MaxIndex= i;							//������������Ե�ַ
		}

		WS_Sum += WindSpeed_arr[i];					//�ۼӼ����ܷ���
	}
	MET_Data.Max_WindSpeed= ((float)WD_Max)/10;		//��������ת��Ϊ���㣨��̨Ҫ����������ͣ���ת�浽MET_Data
	MET_Data.Ave_WindSpeed= ((float)WS_Sum)/100;	//����10�ε�ƽ�����٣���ת��Ϊ���㣨��̨Ҫ����������ͣ���ת�浽MET_Data��100��ʾ10��*10����
	MET_Data.Ave_WindDirection= WindDirection_arr[MaxIndex];				//��¼��ǰ����0~7��0~360�ȣ���Ϊ��̨Ҫ����������ͣ�
	
	
	PWMETEN();										//����ʪ�ȡ�����ѹ��Դ
	OSTimeDly(40);									//AM2302�ϵ��Ҫ�ȴ�2S����Խ�����ȶ�״̬��
	
	/*��ʪ�����ݲɼ�*/
	for( rt=retry; rt>0; rt-- )
	{
		if( Read_Median_AM2302(&AM2302_Data,3) ) 	//����3�Σ�ȡ��λ����ʪ�ȣ����룩���ɹ�����1������ȡֵΪʵ���¶ȵ�10����
		{
			err &= ~0x01;							//��־λbit1�������ʪ�ȴ���������
			break;
		}
		else err |= 0x01;							//��־λbit1����¼��ʪ�ȴ���������
	}
	Temp_16S= (AM2302_Data.temp_H<<8)+AM2302_Data.temp_L;					//ȡ�ô����������¶ȣ���Ϊ���룩--------< Ҫ��ת����INT16S >
	MET_Data.Air_Temperature= ( (float)Temp_16S ) / 10;						//ת��Ϊ���㣨��̨Ҫ����������ͣ���������ʵֵ��ת�浽MET_Data��ԭΪ10����
	MET_Data.Humidity= (AM2302_Data.humi_H<<8)+AM2302_Data.humi_L;			//ʪ������ת�浽MET_Data����ʱΪ16����������1000��ʪ��ֵ����652=65.2%�����Ϻ�̨Ҫ����������ͣ�
	
	/*����ѹ���ݲɼ�*/
	BMP180Init(&BP_info);							//����ѹģ���ʼ��
	if( BP_info.ExistFlag==BMP180_EXISTENCE )
	{
		BMP180Convert(&BP_info);
	}
	else
	{
		err |= 0x02;								//��־λbit2����¼����ѹ����������
	}
	MET_Data.Air_Pressure= ((float)BP_info.GasPress)/1000;   				//����ѹ����ת��Ϊ���㣨��̨Ҫ����������ͣ�����ת�浽MET_Data
				
	MET_LowPower();									//�͹���	�ر���ʪ�ȡ�����ѹ��Դ��������IO
#if TEST
	Err_report( err );								//��ӡ������Ϣ�����485��Դ��
#endif
	
	return err;										//��ȷ����0
}

/***************************************************************************************
���ƣ�INT8U MET_packet_content( INT8U *OutBuff )
���ܣ�΢����֡��֡
��Σ�INT8U *OutBuff����ŵ�ַ
���Σ�INT8U *OutBuff����ŵ�ַ
���أ�֡����
****************************************************************************************/
INT8U MET_packet_content( INT8U *OutBuff )
{
	INT32U	T=0;
	INT8U	MET_packet[69]={0};

//С��ģʽ�����ֽ��ڵ͵�ַ������ǰ��
	NB_ReadID(0x1000,MET_packet,(INT16U *)&T);			//6.Component_ID	������豸ID��17λ���룩[0--16]��ʹ�ñ�����ID��ַ
	if( T!=17 ) return 0;

	T=RTC_GetTime_Second();								//��RTCʱ��
	if(T==0)return 0;
	MET_packet[17]=T&0xff;								//7.Time_Stamp	�ɼ�ʱ��
	MET_packet[18]=(T>>8)&0xff;
	MET_packet[19]=(T>>16)&0xff;
	MET_packet[20]=(T>>24)&0xff;							

	MET_packet[21]=0;									//8.Alerm_Flag	������ʶ
	MET_packet[22]=0; 

	//memcpy�����ṹ��ʱע���������
	memcpy(MET_packet+23,&MET_Data,46);					//��MET_Data��΢�������ݣ�����MET_packet��΢���������ݣ���MET_Data������������Get_Meteorology_Data()��ת��Ϊ��̨Ҫ�����������
	return GDGuiYue(OutBuff,MET_packet,69,1,1);			//��Q/GDW 242-2010������Լ��֡�����б�������Դ��*P
}

/***************************************************************************************
���ƣ�void Err_report( INT8U Err )
���ܣ�΢�������ݲɼ�����ʱ485��ӡ��Ӧ������Ϣ
��Σ�INT8U Err���������
���Σ���
���أ���
****************************************************************************************/
void Err_report( INT8U Err )
{	
	B485_init(4800);								//���³�ʼ������ֹ��������ش��ڵ���ʧЧ��������Ĭ�ϲ�����Ϊ4800
	PWDC485EN();									//DCDC����485��	
	OSTimeDly(1);
	
	if( Err==0 )
	{
		BSP_UART_Write(2,(INT8U *)("\r\n��������������������΢���󴫸�����������������������������\r\n"),18+44);
	}
	if( Err&0x01 )
	{
		BSP_UART_Write(2,(INT8U *)("\r\n����������������������ʪ�ȴ��������󣡡�������������������\r\n"),18+44);
	}
	if( Err&0x02 )
	{
		BSP_UART_Write(2,(INT8U *)("\r\n������������������������ѹ���������󣡡�������������������\r\n"),18+44);
	}
	if( Err&0x04 )
	{
		BSP_UART_Write(2,(INT8U *)("\r\n�����������������������ٴ��������󣡡�������������������\r\n"),16+44);
	}
	if( Err&0x08 )
	{
		BSP_UART_Write(2,(INT8U *)("\r\n�����������������������򴫸������󣡡�������������������\r\n"),16+44);
	}

	OSTimeDly(10);									//�ȴ�485��ӡ���
	PWDC485DIS();									//�ر�485��Դ
}

/***************************************************************************************
���ƣ�INT8U Test_Meteorology_Data( INT8U retry )
���ܣ����Է��ٷ���10�Σ���������١���ʪ�ȡ�����ѹ���ݲɼ����ܡ�
��Σ�INT8U	retry���ɼ�ʧ�����Դ���
���Σ���
���أ�bit1��ʪ�Ȳɼ�����bit2����ѹ�ɼ�����bit3���ٴ���������bit4���򴫸������󣬷���0�ɼ���ȷ
****************************************************************************************/
INT8U Test_Meteorology_Data( INT8U retry )
{
	INT8U 	i,rt,err=0;
	INT8U 	MaxIndex=0;								//������
	INT16U	WD_Max=0,WS_Sum=0;						//���ڼ���ƽ��ֵ
	INT16U	WindSpeed_arr[10]={0};					//10min����
	INT16U	WindDirection_arr[10]={0};				//10min����
	INT16S	Temp_16S=0;								//�з������ͣ����ڸ���ת��
	INT8U	temp[7]={0};							//תASCII����ӡ��
	
	Am2302_Init();									//Am2302��������ʼ��
	PowerMETPin_Init();								//��ʪ�ȵ�Դ���� ��������
	PowerWDSPPin_Init();							//���ٷ����Դ���� ��������
	
	/*���ٷ������ݲɼ���err����ʽҪ���Ȳɼ����ٷ���*/
	for(i=0;i<3;i++)								//�����ɼ�3�η��ٷ���
	{
//		OSTimeDly( 57*20 );							//��ʱ1min���¾�����3�룩�ٲɼ���50��~59�֣�
		for( rt=retry; rt>0; rt-- )
		{
			err= Get_WDSP_Data( WindSpeed_arr+i, WindDirection_arr+i );	//���ٷ������ݲɼ�������¼�����־λ�������һ�εı�־Ϊ׼������д��|=�������޷������ǰ�Ĵ���
			if( err==0 ) break;						//���ٷ������ݲɼ���ȷ������0��ʱ����������ʱ����ѭ��
		}
		
		if( WindSpeed_arr[i] > WD_Max )				//�ж�������
		{
			WD_Max= WindSpeed_arr[i];				//����������
			MaxIndex= i;							//������������Ե�ַ
		}

		WS_Sum += WindSpeed_arr[i];					//�ۼӼ����ܷ���
	}
	MET_Data.Max_WindSpeed= ((float)WD_Max)/10;		//��������ת��Ϊ���㣨��̨Ҫ����������ͣ���ת�浽MET_Data
	MET_Data.Ave_WindSpeed= ((float)WS_Sum)/30;		//����3�ε�ƽ�����٣���ת��Ϊ���㣨��̨Ҫ����������ͣ���ת�浽MET_Data��30��ʾ3��*10����
	MET_Data.Ave_WindDirection= WindDirection_arr[MaxIndex];				//��¼��ǰ����0~7��0~360�ȣ���Ϊ��̨Ҫ����������ͣ�
	
	PWDC485EN();									//DCDC����485�򿪣�Get_WDSP_Data�б��رգ�
	WS_Sum *= 10;									//����ƽ�����٣�10�������ͣ�
	ADC_HEXtoASCII( WS_Sum, temp );					//ת��Ϊ�ַ���
	BSP_UART_Write(2,(INT8U *)("\r\n���٣�"),8);
	BSP_UART_Write( 2, temp, 7 );
	BSP_UART_Write(2,(INT8U *)("����"),6);
	WS_Sum= MET_Data.Ave_WindDirection + 0x30;		//����תASCII
	BSP_UART_Write(2,(INT8U *)&WS_Sum,2);
	
	
	
	PWMETEN();										//����ʪ�ȡ�����ѹ��Դ
	OSTimeDly(40);									//AM2302�ϵ��Ҫ�ȴ�2S����Խ�����ȶ�״̬��
	
	/*��ʪ�����ݲɼ�*/
	for( rt=retry; rt>0; rt-- )
	{
		if( Read_Median_AM2302(&AM2302_Data,3) ) 	//����3�Σ�ȡ��λ����ʪ�ȣ����룩���ɹ�����1������ȡֵΪʵ���¶ȵ�10����
		{
			err &= ~0x01;							//��־λbit1�������ʪ�ȴ���������
			break;
		}
		else err |= 0x01;							//��־λbit1����¼��ʪ�ȴ���������
	}
	Temp_16S= (AM2302_Data.temp_H<<8)+AM2302_Data.temp_L;					//ȡ�ô����������¶ȣ���Ϊ���룩--------< Ҫ��ת����INT16S >
	MET_Data.Air_Temperature= ( (float)Temp_16S ) / 10;						//ת��Ϊ���㣨��̨Ҫ����������ͣ���������ʵֵ��ת�浽MET_Data��ԭΪ10����
	MET_Data.Humidity= (AM2302_Data.humi_H<<8)+AM2302_Data.humi_L;			//ʪ������ת�浽MET_Data����ʱΪ16����������1000��ʪ��ֵ����652=65.2%�����Ϻ�̨Ҫ����������ͣ�
	
	Temp_16S *= 10;									//�¶�
	ADC_HEXtoASCII( Temp_16S, temp );				//ת��Ϊ�ַ����������޷�����
	BSP_UART_Write(2,(INT8U *)("\r\n�¶ȣ�"),8);
	BSP_UART_Write( 2, temp, 7 );
	
	Temp_16S= MET_Data.Humidity * 10;				//ʪ��
	ADC_HEXtoASCII( Temp_16S, temp );				//ת��Ϊ�ַ���
	BSP_UART_Write(2,(INT8U *)("ʪ�ȣ�"),6);
	BSP_UART_Write( 2, temp, 7 );	
	
	/*����ѹ���ݲɼ�*/
	BMP180Init(&BP_info);							//����ѹģ���ʼ��
	if( BP_info.ExistFlag==BMP180_EXISTENCE )
	{
		BMP180Convert(&BP_info);
	}
	else
	{
		err |= 0x02;								//��־λbit2����¼����ѹ����������
	}
	MET_Data.Air_Pressure= ((float)BP_info.GasPress)/1000;   				//����ѹ����ת��Ϊ���㣨��̨Ҫ����������ͣ�����ת�浽MET_Data
	
	for(i=0;i<7;i++)								//ת��Ϊ�ַ���
	{
		if(i==2) continue ; 						//����С����λtemp[4]
		temp[6-i]=BP_info.GasPress%10+0x30;
		BP_info.GasPress/=10;
	}
	temp[4]='.';

	BSP_UART_Write(2,(INT8U *)("��ѹ��"),6);
	BSP_UART_Write( 2, temp, 7 );
	OSTimeDly(10);									//�ȴ�485��ӡ���
	
	MET_LowPower();									//�͹���	�ر���ʪ�ȡ�����ѹ��Դ��������IO
	Err_report( err );								//��ӡ������Ϣ	

	return err;										//��ȷ����0
}


#if 0
/***************************************************************************
����: void MET_DataHandle(void)
˵��: 
��Σ�
���Σ�
*****************************************************************************/
void MET_DataHandle(void)
{
	INT8U	WaitTime=3;
	INT8U	CurrentTime[6];							//��ŵ�ǰʱ��
	
//	CRTime=0;										//����ʱ��0		������0��������
	GetSysTime(CurrentTime);						//��ȡ��ǰʱ��------------------------��ע����ZE ΪʲôҲ�ܶ�����
	if( CurrentTime[4] == 0x05 )					//05��ʱ����ʪ�ȡ�����ѹ����
	{
		if(CRTime==CurrentTime[4])return;			//��ֹͬһ���������루CRTime�����Ӱ���״ν��룩
		CRTime=CurrentTime[4];						//��¼��ǰ����
		Get_Meteorology_Data();						//����ʪ�ȣ�����ѹ����		
	}
	if(CurrentTime[4] ==0x10)						//10-5���Ӻ��ȡ���ٷ�������
	{
		if(CRTime==CurrentTime[4])return;			//��ֹͬһ���������루CRTime�����Ӱ���״ν��룩	
		CRTime=CurrentTime[4];						//��¼��ǰ����
		Read_WDSP_Data();							//��485����ʼ��-----------------------δ����ZE  ����û����������ʱ�������Ķ����ݲ����Ž��˺���

		while(WaitTime--)							//�����ݣ����WaitTime��
		{
		/**************��ȡ����ֵ*********************************/	
			BSP_UART_Write(2,FS_CMD,8);				//���ͷ�������

			if(B485WaitData(1)==1)					//������1s������485���յ������ݽ����жϲ������ɹ�����1
			{
				MET_Data.Ave_WindSpeed=WindSpeed;	//��¼��ǰ����
				break;
			}
		}
		/**************��ȡ����ֵ*********************************/				
		WaitTime=3;									//����WaitTime
		while(WaitTime--)							//�����ݣ����WaitTime��
		{
			BSP_UART_Write(2,FX_CMD,8);				//���ͷ�������
			if(B485WaitData(1)==1)					//������1s������485���յ������ݽ����жϲ������ɹ�����1
			{
				MET_Data.Ave_WindDirection=WindDirection;	//��¼��ǰ����
				break;
			}
		}	
		/***********�ص�Դ���������ݣ�����GPRS***********************/
		PWDC485DIS();								//�ر�485��Դ
//		MET_SaveData(CurrentTime);					//���浱ǰʱ�䣬ʪ�ȣ��¶ȣ���ѹ�����٣���������
		PWWDSPDIS();								//�رշ��ٷ����Դ
//		SetGPRSON();								//Resume�����GPRS����,GPRSON=1--------------------------------------------------------��ֵʱ����Ҫ��������
	}
	
	
//	if((CurrentTime[4] ==0x29) &&(CurrentTime[3]==0x03))		//ÿ��3ʱ29��  �жϲ�����SPI FLASH
//	{
//		/****************ÿ��30�ջ�31����ձ��������**********************/
//		if(CurrentTime[1]<8)//ǰ7����
//		{
//			if(CurrentTime[1]%2==0)//30��
//			{
//				if(CurrentTime[3]==0x30)//��
//				{
//					W25QXX_Erase_Chip();			//����һ�鱣�������
//				}
//			}
//			else//31��
//			{
//				if(CurrentTime[3]==0x31)//��
//				{
//					W25QXX_Erase_Chip();			//����һ�鱣�������
//				}
//			}
//		}
//		else//��5����
//		{
//			if(CurrentTime[1]%2==0)//30��
//			{
//				if(CurrentTime[3]==0x30)//��
//				{
//					W25QXX_Erase_Chip();			//����һ�鱣�������
//				}
//			}
//			else//31��
//			{
//				if(CurrentTime[3]==0x31)//��
//				{
//					W25QXX_Erase_Chip();			//����һ�鱣�������
//				}
//			}
//		}
//		OSTimeDly(70);
//		MCUSoftReset();								//ϵͳ������������
//		OSTimeDly(60);
//	}
}
#endif
