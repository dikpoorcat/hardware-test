#include "Bsp_AM2302.h"

/*
	GPIOX->ODR ---> �˿�X������ݼĴ���
	GPIOX->IDR ---> �˿�X�������ݼĴ���
	GPIOX->DDR ---> �˿�X���ݷ���Ĵ���     0������    1�����
	GPIOX->CR1 ---> ���ƼĴ���1
									DDR=0   ����ʱ  CR1=0  ��������   CR1=1  ��������������
									DDR=1   ����ʱ  CR1=0  ģ�⿪©���   CR1=1  �������
	GPIOX->CR2 ---> ���ƼĴ���2
									DDR=0   ����ʱ  CR2=0  ��ֹ�ⲿ�ж�   CR2=1  ʹ���ⲿ�ж�
									DDR=1   ����ʱ  CR2=0     CR2=1  �������
*/


//ȫ�ֱ���
AM2302_Data_TypeDef	AM2302_Data;



/******************************************************************************* 
* Function Name  : void MET_LowPower(void)
* Description    : MET����͹��ģ�������ӦIO�����͹��Ĵ�������AM2302��BMP180��
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/
void MET_LowPower(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	/*��Դʹ�ܡ�WSD_DA��QY_DATA��QY_SCKģ������*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;							//ģ������
	
	GPIO_InitStructure.GPIO_Pin = PWMET_PIN;								//��Դʹ�ܿڣ��ص�Դ
	GPIO_Init(PWMET_Port, &GPIO_InitStructure);								//

	GPIO_InitStructure.GPIO_Pin = Am2302Pin;								//Am2302Pin(WSD_DA)
	GPIO_Init(Am2302Port, &GPIO_InitStructure);								//
	
	GPIO_InitStructure.GPIO_Pin = Bmp180_SDAPin;							//Bmp180_SDAPin(QY_DATA)
	GPIO_Init(Bmp180_SDAPort, &GPIO_InitStructure);							//
	
	GPIO_InitStructure.GPIO_Pin = Bmp180_SCLKPin;							//Bmp180_SCLKPin(QY_SCK)
	GPIO_Init(Bmp180_SCLKPort, &GPIO_InitStructure);						//
}

/******************************************************************************
* Function Name: void PowerMETPin_Init(void)
* Description:   ��ʪ�ȵ�Դ���� ��������				 
* Input:  Nothing
* Output: Nothing
* Contributor: GuoWei Dong
* Date First Issued: 2010-01-26
******************************************************************************/
void PowerMETPin_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(PWMET_Port_CK,ENABLE);			//��ʱ��
	PWMETDIS();												//����Ϊ�ر�	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =PWMET_PIN;
	GPIO_Init(PWMET_Port, &GPIO_InitStructure);	
}

/**********************************************************************
������void Am2302_Init(void)
���ܣ�Am2302��������ʼ��
��Σ���
���Σ���
***********************************************************************/
void Am2302_Init(void)
{
	//�������ų�ʼ��
	RCC->APB2ENR|=1<<4;  				//PORTCʱ��ʹ�� 	2~8�ֱ�ΪA~G  
	GPIOC->CRH&=0XFFFFFFF0; 
	GPIOC->CRH|=0X00000007;				//PC8(3���죬7��©�����
	Am2302_DataH();						//��ʼ��Ϊ���裬�ⲿ����Ϊ�ߵ�ƽ
	
//�������ų�ʼ��Ϊ���裬�ⲿ����Ϊ�ߵ�ƽ��GPIO_Mode_Out_OD��ͨ���Ժã�
//	GPIO_InitTypeDef GPIO_InitStructure;
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
//	GPIO_InitStructure.GPIO_Pin = Am2302Pin;
//	GPIO_Init(Am2302Port, &GPIO_InitStructure);
//	GPIO_SetBits(Am2302Port,Am2302Pin);								
}

/**********************************************************************
������static void AM2302_Mode_IPU(void) 
���ܣ�ʹAM2302-DATA���ű�Ϊ����ģʽ
��Σ���
���Σ���
***********************************************************************/
static void AM2302_Mode_IPU(void)  
{  
	GPIO_InitTypeDef GPIO_InitStructure;  

	/*ѡ��Ҫ���Ƶ�GPIOD����*/    
	GPIO_InitStructure.GPIO_Pin = Am2302Pin;  

	/*��������ģʽΪ��������ģʽ*/  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ;   

	/*���ÿ⺯������ʼ��GPIOD*/  
	GPIO_Init(Am2302Port, &GPIO_InitStructure); 	
}

/**********************************************************************
������static INT8U Read_Byte(void)
���ܣ���AM2302���߶�ȡһ���ֽ�
��Σ���
���Σ����ض�ȡ�����ֽ�
***********************************************************************/
static INT8U Read_Byte(void)  
{       
	u8 	i,temp=0;
	u8	Timeout=20;										//���ó�ʱ��20*5usδ��⵽��ƽ�仯���˳�

	for(i=0;i<8;i++)      
	{  
		/*ÿbit��50us�͵�ƽ���ÿ�ʼ����ѯֱ���ӻ����� ��50us �͵�ƽ ����*/   
		while( (!AM2302_DATA_IN()) && Timeout )			//��⵽�͵�ƽ��Timeoutδ��ʱ������ѭ��
		{
			delay_1us_32M(5);							//΢�뼶��ʱ����delay_SYSus()׼ȷ�ȸߡ�ֻ����1us�Ļ�����΢��׼����Ϊ�г���ջʱ�䣬����һ�κ�������0.49us
			Timeout--;
			if(Timeout==0) return 0;					//��ʱδ��ȡ��AM2302��Ӧ������δ���ߣ�����Ч���ݣ�
		}

		/*AM2302 ��22~30us�ĸߵ�ƽ��ʾ��0������68~75us�ߵ�ƽ��ʾ��1��,ͨ�����60us��ĵ�ƽ��������������״̬*/  
		delay_1us_32M(50);								//��ʱ50us
		
		Timeout=20;										//����Timeout
		if( AM2302_DATA_IN() )							//50us����Ϊ�ߵ�ƽ��ʾ���ݡ�1��  
		{  
			/*��ѯֱ���ӻ�������ʣ��� 30us �ߵ�ƽ����*/  
			while( AM2302_DATA_IN() && Timeout )		//��⵽�ߵ�ƽ��Timeoutδ��ʱ������ѭ��
			{
				delay_1us_32M(5);						//΢�뼶��ʱ����delay_SYSus()׼ȷ�ȸߡ�ֻ����1us�Ļ�����΢��׼����Ϊ�г���ջʱ�䣬����һ�κ�������0.49us
				Timeout--;
				if(Timeout==0) return 0;				//��ʱδ��ȡ��AM2302��Ӧ
			}
			temp|=(u8)(0x01<<(7-i));					//�ѵ�7-iλ��1   
		}  
		else  											//50us��Ϊ�͵�ƽ��ʾ���ݡ�0��  
		{                 								//ѭ���Ӵ˳�ȥʱ���ǵ͵�ƽ
			temp&=(u8)~(0x01<<(7-i));					//�ѵ�7-iλ��0  
		}
	}  
	return temp;
}

/***************************************************************************************
���ƣ�INT8U Read_AM2302( AM2302_Data_TypeDef *Data )
���ܣ��Ӵ�������ȡ��ʪ�����ݣ���ת��Ϊ������ʽ������INT16Sֱ������
	��ȡ5�ֽ���ҪԼ5ms�����ϻ���1ms������Ҫ6ms���ң������ù��жϷ�ʽ�ᵼ���ж��ӳ�6ms��
	�������жϣ�Ҫ������ʱ�䳤���жϻᵼ�¶�ȡ��ʪ�ȴ�����������ʧ�����Թ��ܣ�������
	��ȡ��ȷ�ʶ���֤ʵʱ�ԣ������ж϶��ý�ֹOS���ȵķ�ʽ��
��Σ���
���Σ�AM2302_Data_TypeDef *Data����Ųɼ�����ʪ������
���أ��ɹ�1��ʧ��0��У��λ���󡢴���������Ӧ���ɼ���ʪ�ȳ�����ʱ��
****************************************************************************************/
INT8U Read_AM2302( AM2302_Data_TypeDef *Data )
{
	INT8U	Timeout=10;									//���ó�ʱ��10*10usδ��⵽��ƽ�仯���˳�
	INT16S	Tem=0;										//�з�������
	
	Am2302_DataL();				
	delay_SYSus(1000);									//��ʱ1ms
	Am2302_DataH();				
	delay_SYSus(30);									//��ʱ30us	��Ҫ��30us�󼴿ɶ�ȡ���͵�ƽ

	/*������Ϊ���� �жϴӻ���Ӧ�ź�*/  
	AM2302_Mode_IPU();									//�������߳�ʼ��Ϊ��������

	/*�жϴӻ��Ƿ��е͵�ƽ��Ӧ�ź� �粻��Ӧ����������Ӧ����������*/    
	if(AM2302_DATA_IN()==0)   							//���͵�ƽ    
	{ 
		OSSchedLock();									//��ֹOS���ȣ���ֹ���ʱ��delay_SYSus���������н�ֹ���ȣ���Read_Byte�����е�delay_1us_32M����û�н�ֹ��
		
		/*��ѯֱ���ӻ����� ��80us �͵�ƽ ��Ӧ�źŽ���*/   
		while( (!AM2302_DATA_IN()) && Timeout )			//��⵽�͵�ƽ��Timeoutδ��ʱ������ѭ��
		{
			delay_1us_32M(10);
			Timeout--;
			if(Timeout==0) return 0;					//��ʱδ��ȡ��AM2302��Ӧ
		}
		
		Timeout=10;										//����Timeout
		/*��ѯֱ���ӻ������� 80us �ߵ�ƽ �����źŽ���*/  
		while( AM2302_DATA_IN() && Timeout )			//��⵽�ߵ�ƽ��Timeoutδ��ʱ������ѭ��
		{
			delay_1us_32M(10);
			Timeout--;
			if(Timeout==0) return 0;					//��ʱδ��ȡ��AM2302��Ӧ
		}

		/*��ʼ��������*/
		Data->humi_H= Read_Byte(); 						//��ȡʪ�ȸ��ֽ�
		Data->humi_L= Read_Byte();						//��ȡʪ�ȵ��ֽ�
		Data->temp_H= Read_Byte();						//��ȡ�¶ȸ��ֽ� 
		Data->temp_L= Read_Byte();						//��ȡ�¶ȵ��ֽ�
		Data->check_sum= Read_Byte();					//��ȡУ��� 
      
		/*��ȡ���������Ÿ�Ϊ���ģʽ*/  
		Am2302_Init();									//��©����ߵ�ƽ
		
		OSSchedUnlock();								//�ָ�OS����
		

		
		/*����ȡ�������Ƿ���ȷ*/  
		if(Data->check_sum != (u8)(Data->humi_H + Data->humi_L + Data->temp_H+ Data->temp_L)) 
			return ERROR; 								//У��ʹ���
		
		Tem= (Data->humi_H<<8)+Data->humi_L;
		if( Tem > 1000 ) return ERROR; 					//ʪ�ȴ���100%����
		
		Tem= (Data->temp_H<<8)+Data->temp_L;
		if( Tem & 0x8000 ) 								//�¶�С��0����ԭ��ת���ɲ���
		{
			Tem= ( ~(Tem << 1) ) >> 1;					//������λ��ȡ��
			Tem |= 0x8000;								//��÷���
			Tem += 1;									//��ò���
			
			Data->temp_H= (Tem&0xFF00)>>8;				//����¶ȸ��ֽڣ����룩
			Data->temp_L= Tem&0x00FF;					//����¶ȵ��ֽڣ����룩
		}
		if( (Tem<-400) || (Tem>800) ) return ERROR;		//�¶�С��-40�ȴ��󡢴���80�ȴ���
			
		return SUCCESS; 								//����Ҫ�����ʪ��
	}  
	else  
		return ERROR;									//����δ���ͣ�δ��ȡ��AM2302��Ӧ
}

/***************************************************************************************
���ƣ�INT8U Read_Avrage_AM2302( AM2302_Data_TypeDef *Data, INT8U Times ) 
���ܣ���β�������������ʪ�����ݵ�ƽ��ֵ������*Data��Ҳ�ǲ�����ʽ��
��Σ�INT8U Times����������
���Σ�AM2302_Data_TypeDef *Data��ƽ����ʪ�����ݣ�INT8U  check_sumУ���Ϊ0��
���أ��ɹ�1��ʧ��0
****************************************************************************************/
INT8U Read_Avrage_AM2302( AM2302_Data_TypeDef *Data, INT8U Times ) 
{
	INT16U	Humi=0;
	INT16S	Temp=0;										//�з���
	INT8U	t=Times;									//�������
	
	while(Times--)										//����Times��
	{
		if( Read_AM2302( Data ) ) 						//�ɹ�����1�������õ�����ת�ɲ��룬��ֱ�����㣩
		{
			Humi += (Data->humi_H<<8)+Data->humi_L;		//�ۼ�ʪ��
			Temp += (Data->temp_H<<8)+Data->temp_L;		//�ۼ��¶ȣ������õ�����ת�ɲ��룬��ֱ�����㣩
		}
		else return 0;									//ĳ�βɼ�ʧ��ʱ����0
		if( Times ) OSTimeDly(60);						//�ȴ�3�� AM2302�ɼ����Ҫ2�룬���һ�β��ȴ�
	}
	
	Humi /= t;											//ƽ��ʪ��
	Temp /= t;											//ƽ���¶�
	Data->humi_L = Humi&0x00FF;							//ȡ���ֽ�
	Data->humi_H = (Humi&0xFF00)>>8;					//ȡ���ֽ�
	Data->temp_L = Temp&0x00FF;							//ȡ���ֽ�
	Data->temp_H = (Temp&0xFF00)>>8;					//ȡ���ֽ�
	return 1;											//��ʾ�ɹ�
}

/***************************************************************************************
���ƣ�INT8U Read_Median_AM2302( AM2302_Data_TypeDef *Data, INT8U Times ) 
���ܣ���β�������������ʪ�����ݵ���λ�������¶����򣩣�����*Data��Ҳ�ǲ�����ʽ��
��Σ�INT8U Times���������������10��
���Σ�AM2302_Data_TypeDef *Data����ʪ��������λ����INT8U  check_sumУ���Ϊ0��
���أ��ɹ�1��ʧ��0
****************************************************************************************/
INT8U Read_Median_AM2302( AM2302_Data_TypeDef *Data, INT8U Times )
{
    INT8U	j ,i, LastExchaneIndex;
    INT8U	Temp[5]={0};								//����ʱ����
	AM2302_Data_TypeDef	Data_Temp[10]={0};				//���10��
	INT8U	Median= Times/2;							//��λ�����磺7��ʱȡData_Temp[3]��8��ʱȡData_Temp[4]
	
    j = Times - 1;										//����������
	
	/*��ʪ�Ȳɼ�*/
	while( Times-- )
	{
		if( !Read_AM2302( &Data_Temp[Times] ) )			//��ת��Ϊ���룬��ֱ������
			return 0;									//ĳ�βɼ�ʧ��ʱ����0
		if( Times ) OSTimeDly(60);						//�ȴ�3�� AM2302�ɼ����Ҫ2�룬���һ�β��ȴ�
	}
	
	/*���¶�����*/
    while(j>0)
    {
        LastExchaneIndex=0;
        for(i = 0; i < j; i++)
        {
            if( (Data_Temp[i].temp_H<<8)+Data_Temp[i].temp_L > (Data_Temp[i+1].temp_H<<8)+Data_Temp[i+1].temp_L )   //>������ں�
            {
                memcpy(Temp,&Data_Temp[i],5);       	//�����鸴�Ƶ�����
                memcpy(&Data_Temp[i],&Data_Temp[i+1],5);
                memcpy(&Data_Temp[i+1],Temp,5);     	//�����ں�
                LastExchaneIndex=i;
            }
        }
        j= LastExchaneIndex;							//ָʾ��󽻻��ĵ�ַ��������δ�ź���
		if( j < Median ) break;							//����λ���������������
    }
	
	memcpy( Data, &Data_Temp[Median], 5 );				//����λ���������
	return 1;											//��ʾ�ɹ�
}
