#include "Bsp_WDSP_JX.h"

//��������
static INT8U	FS_CMD[8]={0x01,0x03,0x00,0x00,0x00,0x01,0x84,0x0A};//����Э��
static INT8U	FX_CMD[8]={0x02,0x03,0x00,0x00,0x00,0x02,0xC4,0x38};//����Э��





/******************************************************************************* 
* Function Name  : void WDSP_LowPower(void)
* Description    : WDSP����͹��ģ�������ӦIO�����͹��Ĵ����ر�U2ʱ�Ӽ�����ʱ�ӡ�
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/
void WDSP_LowPower(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	B485_LowPower();														//485�͹���

	/*WDSP��Դʹ���������*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;						//�������
	PWWDSPDIS();															//�ص�Դ
	GPIO_InitStructure.GPIO_Pin = PWWDSP_PIN;								//WDSP��Դʹ�ܿ�
	GPIO_Init(PWWDSP_Port, &GPIO_InitStructure);							//
}

/******************************************************************************
* Function Name: void PowerWDSPPin_Init(void)
* Description:   ���ٷ����Դ���� �������� 
* Input:  Nothing
* Output: Nothing
* Contributor: GuoWei Dong
* Date First Issued: 2010-01-26
******************************************************************************/
void PowerWDSPPin_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(PWWDSP_Port_CK,ENABLE);			//��ʱ��
	PWWDSPDIS();											//����Ϊ�ر�
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =PWWDSP_PIN;
	GPIO_Init(PWWDSP_Port, &GPIO_InitStructure);
}

/***************************************************************************************
���ƣ�INT8U Get_WDSP_Data( INT16U OutBuff_WindSpeed, INT16U OutBuff_WindDirection )
���ܣ����ٷ������ݲɼ�
��Σ���
���Σ�INT16U OutBuff_WindSpeed, ����ֵ��INT8U OutBuff_WindDirection������ֵ
���أ�bit3���ٴ���������bit4���򴫸������󣬷���0�ɼ���ȷ
****************************************************************************************/
INT8U Get_WDSP_Data( INT16U *OutBuff_WindSpeed, INT16U *OutBuff_WindDirection )
{
	INT8U 	err=0;
	
	WindSpeed=0;									//��0ȫ�ֱ���
	WindDirection=0;								//��0ȫ�ֱ���
	
	Power485Pin_Init();								//��������485��Դ��������
	PWWDSPEN();										//�������ٷ��򴫸�����Դ	
	PWDC485EN();									//DCDC����485��	
	OSTimeDly(60);									//��ʱ3S��ȡ���ݣ�ʵ��2.5�����ϲ��ܶ������٣���ԣ����3�룩
	B485_init(4800);								//���³�ʼ������ֹ��������ش��ڵ���ʧЧ��������Ĭ�ϲ�����Ϊ4800

	BSP_UART_Write(2,FS_CMD,8);						//���ͷ�������
	if(B485WaitData(1)==1)							//������1s������485���յ������ݽ����жϲ������ɹ�����1
	{
		*OutBuff_WindSpeed=WindSpeed;				//��ǰ���٣�UINT16��10�����٣�
	}else err= 0x04;								//��־λbit3����¼���ٴ���������
	
	BSP_UART_Write(2,FX_CMD,8);						//���ͷ�������
	if(B485WaitData(1)==1)							//������1s������485���յ������ݽ����жϲ������ɹ�����1
	{
		*OutBuff_WindDirection=WindDirection;		//��¼��ǰ����INT8U��0~7��0~360�ȣ���Ϊ��̨Ҫ����������ͣ�
	}else err= 0x08;								//��־λbit4����¼���򴫸�������
	
	if( err&0x04 )									//���ٴ���������
	{
		BSP_UART_Write(2,(INT8U *)("\r\n�����������������������βɼ�����ʧ�ܣ���������������������\r\n"),18+44);		//485��Ϣ��ӡ
		OSTimeDly(10);
	}else if( err&0x08 )							//���򴫸�������
	{
		BSP_UART_Write(2,(INT8U *)("\r\n�����������������������βɼ�����ʧ�ܣ���������������������\r\n"),18+44);		//485��Ϣ��ӡ
		OSTimeDly(10);
	}
	WDSP_LowPower();								//����͹���
	return err;										//��ȷʱ����0
}

/*********************************************************************************************
������INT8U IsProtocol_WDSP( INT8U *In, INT8U Len, INT16U Out_WindSpeed, INT16U Out_WindDirection )
˵������еʽ���ٷ��򴫸�����������Э�鴦��
��Σ�In��������ٷ�������    Len�����ݳ���  	Out_WindSpeed����ŷ���ֵ	Out_WindDirection����ŷ���ֵ
���Σ�1:����Э��   0��������Э��
*******************************************************************************************/
INT8U IsProtocol_WDSP( INT8U *In, INT8U Len, INT16U *Out_WindSpeed, INT16U *Out_WindDirection )
{
	INT8U CRCH=0,CRCL=0;
	if(In[0]==0x01)											//��������
	{
		if(In[1]!=0x03)return 0;
		if(In[2]!=0x02)return 0;
		CRC16_Modbus( In, 5, &CRCL, &CRCH );				//����CRCУ��
		if( (In[5]!=CRCL) && (In[6]!=CRCH) )return 0;		//ע��˴��ߵ��ֽ�λ�ã�С��ģʽ���ֽ��ڵ͵�ַ
		*Out_WindSpeed=(In[3]<<8)+In[4];					//CRCУ��ͨ�����������ֵ��10����
		return 1;
	}
	if(In[0]==0x02)											//��������
	{
		if(In[1]!=0x03)return 0;
		if(In[2]!=0x04)return 0;
		CRC16_Modbus( In, 7, &CRCL, &CRCH );				//����CRCУ��
		if( (In[7]!=CRCL) && (In[8]!=CRCH) )return 0;		//ע��˴��ߵ��ֽ�λ��
		*Out_WindDirection=(In[3]<<8)+In[4];				//CRCУ��ͨ�����������ֵ��0~7��
//		*Out_WindDirection=(In[5]<<8)+In[6];				//CRCУ��ͨ�����������ֵ��0~360�ȣ�
		return 1;
	}
	return 0;
}

/*********************************************************************************************
��������void CRC16_Modbus(INT8U *P,INT16U len,INT8U *LByte,INT8U *HByte)
˵������еʽ���ٷ��򴫸�������У��	��CRC-16 (Modbus)��
��Σ�P��У������    len��У�����ݵĳ���   
���Σ� LByte��У�������ֽ�    HByte��У�������ֽ�
*******************************************************************************************/
void CRC16_Modbus(INT8U *P,INT16U len,INT8U *LByte,INT8U *HByte)
{
	INT8U CRC16Lo,CRC16Hi;
	INT8U CL,CH;
	INT8U SaveHi,SaveLo;
	INT16U i,j;
	CRC16Lo=0xff;
	CRC16Hi=0xff;
	CL=0X01;
	CH=0xA0;//����ʽ��0xA001
	for(i=0;i<len;i++)
	{
		CRC16Lo=CRC16Lo^(P[i]);//ÿһ��������CRC�Ĵ����������
		for(j=0;j<8;j++)
		{
			SaveHi=CRC16Hi;
			SaveLo=CRC16Lo;
			CRC16Hi=(CRC16Hi>>1);//��λ����һλ
			CRC16Lo=(CRC16Lo>>1);//��λ����һλ
			if((SaveHi&0x01)==0x01)//�����λ�ֽ����һλΪ1
			{
					CRC16Lo=CRC16Lo | 0x80;//���λ�ֽ����ƺ�ǰ�油1�������Զ���0
			}
			if((SaveLo&0x01)==0x01)//���LSBΪ1���������ʽ��������
			{
					CRC16Hi=CRC16Hi^CH;
				  CRC16Lo=CRC16Lo^CL;		
		  }
	  }
	}
	*HByte = CRC16Hi;
	*LByte = CRC16Lo;
}
