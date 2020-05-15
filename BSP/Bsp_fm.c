/***********************************************************************************************************************                                    
*
*               (c) Copyright 2017-2030, ���ݷ��ϵ����������޹�˾(http://www.fcdl.com.cn)
*                            All Rights Reserved
*
*---------- File Info ---------------------------------------------------------------
* File name   :  Bsp_Fm.c
*
* Descriptions:  This file define all the function used to control FM25CL64
*
* Created By  : 
* Created date:  
*
*---------- History Info -------------------------------------------------------------
* Version: 			Ver1.0
* Descriptions: FM25CL64��W25Q256����ͬһ��SPI�ӿ�	
*
*-------------------------------------------------------------------------------------
************************************************************************************************************************/
 #include "Bsp_fm.h"
 
INT8U FM_Flag=0;				//bit	7	6 	5 	4 	3 	2 	1 	0		������ռ�ñ�־λ
								//����	X	X	X	X	WDT	GY	RF	GPRS	









/* --------------------Private functions------------------------------------------------------*/
/************************************************************************************************************************
* Function Name : void Fmdelay(INT16U US)
* Description   : ��ʱ����
* Input         : US : �ȴ�US��ʱ������
* Return        : None
*************************************************************************************************************************/
void Fmdelay(INT16U US)
{
	while(US--);
}

/************************************************************************************************************************
* Function Name : INT8U FmbitReadState(void)
* Description   : ��ȡ״̬��
* Input         : None
* Return        : status : FM25CL64��SPI�ڷ��ص�״̬�ֽ�
*************************************************************************************************************************/
INT8U FmReadState(void)
{
	INT8U status = 0;

	FMCS_H();
	Fmdelay(400);																// �ȴ��ȶ�
	FMCS_L();
	
	BSP_SoftSpiSend(FM_RDSR);
	status = BSP_SoftSpiRece(0xff);
	FMCS_H();
	
	return status;
 }

/************************************************************************************************************************
* Function Name : void BSP_InitFm(void)
* Description   : ����洢��FM25CL64 ���������Ӳ���ĳ�ʼ��
* Input         : None
*
* Return        : None
*************************************************************************************************************************/
void BSP_InitFm( INT8U Task_Num )												//7~0	����	X	X	X	X	Wdt	GY	RF	GPRS	
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
		
	FM_Flag|=(1<<Task_Num);
	if( FM_Flag&~(1<<Task_Num) ) return;										//�����������ѳ�ʼ����FM_Flag��0��ֱ�ӷ���	
	
	__disable_irq();															// ��ֹ���ж�     // zzs Modified it 2018.06.11
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC,ENABLE);
	FMCS_L(); 
	FMWP_L();

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

	GPIO_InitStructure.GPIO_Pin =NCS_FM_PIN;
	GPIO_Init(NCS_FM_Port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =NWP_FM_PIN;
	GPIO_Init(NWP_FM_Port, &GPIO_InitStructure);
	
	SPI_1_Init();

	FMCS_H(); 
	Fmdelay(10);																//�ȴ��ȶ�
	FMCS_L(); 
	BSP_SoftSpiSend(FM_WREN);
	FMCS_H();
	FMWP_H(); 
	FMCS_L(); 
	BSP_SoftSpiSend(FM_WRSR);
	BSP_SoftSpiSend(0x80);														//WPEN = 0 ,else is 0;
	FMCS_H(); 
	FMWP_L();

	FMCS_H(); 

	Fmdelay(20);																//�ȴ��ȶ�
	
	__enable_irq();																// �����ж�	  // zzs Modified it 2018.06.11
}

/************************************************************************************************************************
* Function Name : INT8U BSP_WriteDataToFm(INT16U FlashAddr,INT8U *DataAddr,INT32U Len)
* Description   : ������д�뵽������ȥ(Write Data To Fm25CL64)
* Input         : FlashAddr : ָʾ����洢���ĵ�ַ ��Χ��0~ MAX_FM_LEN
*                 pDataAddr : Ҫд�����ݵĴ�ŵ�ַ
*                 Len       : д������ݳ���
*
* Return        : ��ʽ���أ� ���ر��� SPI_statu ��Զ����� 1�ˡ���Զ���� ��
*************************************************************************************************************************/
INT8U BSP_WriteDataToFm(INT16U FlashAddr,INT8U *pDataAddr,INT32U Len)
{
	INT8U SPI_statu = 1;

	if((FlashAddr >= MAX_FM_LEN) || ((FlashAddr + Len) > MAX_FM_LEN) || (pDataAddr == 0))  //zzs note,���������������,��(pDataAddr == NULL)
	{
		return 0;
	}

	__disable_irq();															// ��ֹ���ж�     // zzs Modified it 2018.06.11

	FMCS_H();
	FMWP_H();
	FMSCK_L();
	Fmdelay(10);																// �ȴ��ȶ�
	FMCS_L(); 
	BSP_SoftSpiSend(FM_WREN);

	FMCS_H(); 
	FMCS_L();

	BSP_SoftSpiSend(FM_WRITE);
	BSP_SoftSpiSend(FlashAddr>>8);
	BSP_SoftSpiSend(FlashAddr);
	SPI_statu = SPI_BufferSend(pDataAddr,Len);									// zzs note, ���SPI_statu ��Զ = 1��Ϊ������

	FMCS_H();
	FMWP_L();

	__enable_irq();																// �����ж�	  // zzs Modified it 2018.06.11

	return SPI_statu;
}

/************************************************************************************************************************
* Function Name : INT8U BSP_ReadDataFromFm(INT16U FlashAddr,INT8U *pDataAddr,INT32U Len)
* Description   : �������ж�������(Read Data From Fm25CL64)
* Input         : FlashAddr : ָʾ����洢���ĵ�ַ ��Χ��0~ MAX_FM_LEN
*                 pDataAddr : ��ȡ�����Ĵ�ŵ�ַ
*                 Len       : ��ȡ�����ݳ���
*
* Return        : ��ʽ���أ� ͨ���β�pDataAddr���ض�ȡ������
*                 ��ʽ���أ� ���ر��� SPI_statu ��Զ����� 1�ˡ���Զ���� ��
*************************************************************************************************************************/
INT8U BSP_ReadDataFromFm(INT16U FlashAddr,INT8U *pDataAddr,INT32U Len)
{
	u8 SPI_statu = 1;

	if((FlashAddr >= MAX_FM_LEN) || ((FlashAddr + Len) > MAX_FM_LEN) || (pDataAddr == 0))   //zzs note,���������������,��(pDataAddr == NULL)
	{
	 return 0;
	}

	__disable_irq();															// ��ֹ���ж�     // zzs Modified it 2018.06.11

	FMCS_H();
	FMSCK_L();
	Fmdelay(10);																//�ȴ��ȶ�
	FMCS_L();
	BSP_SoftSpiSend(FM_READ);
	BSP_SoftSpiSend(FlashAddr>>8);
	BSP_SoftSpiSend(FlashAddr);
	SPI_statu = SPI_BufferReceive(pDataAddr,Len);								// zzs note, ���SPI_statu ��Զ = 1��Ϊ������
	FMCS_H();

	__enable_irq();																// �����ж�	  // zzs Modified it 2018.06.11

	return SPI_statu;
 }

/*******************************************************************************
* Function Name  : INT8U FM_test(void)
* Description    : ������ԣ������Գɹ�������1��������ʧ�ܷ���0
* Input          : 
* Output         : 
*******************************************************************************/
INT8U FM_test(void)
{
	INT8U				test[255] = {0}, original[255] = {0};
	INT16U				i = 0, state = 1;
	
	BSP_InitFm(LOC_Num);
	
	/*��ȡԭ������*/
	BSP_ReadDataFromFm(0x1800, original, 256);									//��ȡ����ԭ����Ϣ	
	
	/*д���������*/
 	for(i=0;i<256;i++) test[i]=i;
	BSP_WriteDataToFm(0x1800, test, 256);
	
	/*���ز��Ƚ�*/
	memset(test, 0, 256);														//��0
	BSP_ReadDataFromFm(0x1800, test, 256);										//����
	for(i=0;i<256;i++)
	{
		if (test[i]!=i)
		{
			state = 0;															//FM����ʧ��
			break;
		}	
	}
	
	/*д��ԭ������*/
	BSP_WriteDataToFm(0x1800, original, 256);
	return state;
}

/******************************************************************************* 
* Function Name  : void FM_LowPower(void)
* Description    : FM����͹��ģ�������ӦIO�����͹��Ĵ���
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/
void FM_LowPower( INT8U Task_Num )												//7~0	����	X	X	X	X	Wdt	GY	RF	GPRS	
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	FM_Flag&=~(1<<Task_Num);													//�嵱ǰ����bit	
	if(FM_Flag) return;															//����������ռ�ã�FM_Flag��0��ֱ�ӷ���   
	
	/*Ƭѡ�ڡ�д������ģ������*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;								//ģ������
	
	GPIO_InitStructure.GPIO_Pin = NCS_FM_PIN;									//Ƭѡ��
	GPIO_Init(NCS_FM_Port, &GPIO_InitStructure);								//
	
	GPIO_InitStructure.GPIO_Pin = NWP_FM_PIN;									//д������
	GPIO_Init(NWP_FM_Port, &GPIO_InitStructure);								//	

	
	/*SPI����������������*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;							//�������
	
	GPIO_InitStructure.GPIO_Pin = MOSI_FM_PIN;									//MOSI
	GPIO_ResetBits(MOSI_FM_Port, MOSI_FM_PIN);									//����
	GPIO_Init(MOSI_FM_Port, &GPIO_InitStructure);								//
	
	GPIO_InitStructure.GPIO_Pin = MISO_FM_PIN;									//MISO
	GPIO_ResetBits(MISO_FM_Port, MISO_FM_PIN);									//����
	GPIO_Init(MISO_FM_Port, &GPIO_InitStructure);								//
	
	GPIO_InitStructure.GPIO_Pin = SCK_FM_PIN;									//SCK
	GPIO_ResetBits(SCK_FM_Port, SCK_FM_PIN);									//����
	GPIO_Init(SCK_FM_Port, &GPIO_InitStructure);								//
}

/************************************************************************************************************************
* Function Name : INT8U BSP_FM_Erase(INT16U FlashAddr,INT32U Len)
* Description   : ������ĳ��ʼ��ַ��ʼ������Len���ȿռ䣨��0��
* Input         : FlashAddr : ָʾ����洢���ĵ�ַ ��Χ��0~ MAX_FM_LEN
*                 Len       : ��Ҫ�����ĳ���
* Return        : 1���ɹ�	0��ʧ��
*************************************************************************************************************************/
INT8U BSP_FM_Erase(INT16U FlashAddr,INT32U Len)
{
	INT32U Num=0; 
	
	if((FlashAddr >= MAX_FM_LEN) || ((FlashAddr + Len) > MAX_FM_LEN)) return 0; //zzs note,���������������,��(pDataAddr == NULL)

	__disable_irq(); 															// ��ֹ���ж�     // zzs Modified it 2018.06.11

	FMCS_H();
	FMWP_H();
	FMSCK_L();
	Fmdelay(10);  																// �ȴ��ȶ�
	FMCS_L(); 
	BSP_SoftSpiSend(FM_WREN);

	FMCS_H(); 
	FMCS_L();
	
	/*д���������ʼ��ַ*/
	BSP_SoftSpiSend(FM_WRITE);
	BSP_SoftSpiSend(FlashAddr>>8);
	BSP_SoftSpiSend(FlashAddr);

	/*��ʼ���ֽڲ���*/
	while(Len)
	{	
		Feed_Dog();
		Num = Len/1024 ? 1024:Len%1024;											//ÿ1K��ιһ�ι�����ֹ���Ź���λ
		Len -= Num;		
		while(Num--)															//дNum��0
		{
			BSP_SoftSpiSend(0);
		}
	}
	FMCS_H();
	FMWP_L();
	
	__enable_irq();																// �����ж�	  // zzs Modified it 2018.06.11
	
	return 1;
}
