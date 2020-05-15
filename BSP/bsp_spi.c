/***********************************************************************************************************************                                    
*
*               (c) Copyright 2017-2030, ���ݷ��ϵ����������޹�˾(http://www.fcdl.com.cn)
*                            All Rights Reserved
*
*---------- File Info ---------------------------------------------------------------
* File name   :  bsp_spi.c
*
* Descriptions:  FM25CL64��W25Q256 �ҽ���ͬһ���SPI�ӿ��ϣ���ˣ������Ҫͬʱʹ�������Ŵ洢���Ļ���
*                ����Ҫ�����߷�ʽ��������ΪBsp_fm��Bsp_W25Q256����
* 					     ��ʹ�ñ��ļ��еĺ��� ǰ����Ҫ����OSSchedLock OSSchedUnlock ������������
*
* Created By  :  ��־˴(11207656@qq.com)
* Created date:  2017.12.21
*
*---------- History Info -------------------------------------------------------------
* Version: 			Ver1.0
* Descriptions: 	
*
*-------------------------------------------------------------------------------------
************************************************************************************************************************/
#include "bsp_spi.h"
//#include "stm32l1xx.h"
//#include "stm32l1xx_gpio.h"
//#include "stm32l1xx_rcc.h"

/* --------------------------------Private define---------------------------------------------*/
/* Note��ʹ�ö�ȡ����д���ʱ��Ҫ���� open �� close �Ĳ��� */
#define MAX_FM_LEN 0x2000
#define FM_WREN 0x06
#define FM_WRDI 0x04
#define FM_RDSR 0x05
#define FM_WRSR 0x01
#define FM_READ 0x03
#define FM_WRITE 0x02

#define SCK_PIN     GPIO_Pin_4
#define SCK_Port    GPIOC

#define MOSI_PIN    GPIO_Pin_7
#define MOSI_Port   GPIOA

#define MISO_PIN    GPIO_Pin_5  //MISO
#define MISO_Port   GPIOA     

/* ------------------------------Private macro------------------------------------------------*/
#define  SCK_H()		GPIO_SetBits(SCK_Port, SCK_PIN)
#define  SCK_L()		GPIO_ResetBits(SCK_Port, SCK_PIN)
#define  MOSI_H()		GPIO_SetBits(MOSI_Port, MOSI_PIN)
#define  MOSI_L()		GPIO_ResetBits(MOSI_Port, MOSI_PIN)
#define  MISO()		GPIO_ReadInputDataBit(MISO_Port,MISO_PIN)


/* --------------------Private functions------------------------------------------------------*/
/******************************************************************************
* Function Name: SPI_1_Init
* Description:   SPI 1 Init as MasterMode SPI MODE 0
* Input:  Nothing
* Output: Nothing
* Contributor: Liehua Wang
* Date First Issued: 2008-12-24
******************************************************************************/
void SPI_1_Init(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure = {0};
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA ,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	//GPIOA->MODER|=0x01<<4;
	/* Configure SPI1 pins: SCK, MISO and MOSI ------------- */
	GPIO_InitStructure.GPIO_Pin = SCK_PIN  ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

	GPIO_Init(SCK_Port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = MOSI_PIN  ;
	GPIO_Init(MOSI_Port, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = MISO_PIN  ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(MISO_Port, &GPIO_InitStructure);
	SCK_L();
	MOSI_L();
}
 /*******************************************************************************
* Function Name  : BSP_SoftSpiSend.
* Description    : ���ģ��SPI
* Input 1        : SPIx where x can be 0,1,2 to select the SPI peripheral.
* Input 2        : PtrToBuffer is an u8 pointer to the first word to be transmitted.
* Input 3        : NbOfWords parameter indicates the number of words to be sent.
* Output         : None.
* Return         : TRUE / FALSE.
*******************************************************************************/
void BSP_SoftSpiSend(INT8U val)
{
	INT8U i = 0;
  
	for(i=0x80;i!=0;i>>=1)
	{	
 		SCK_L();
		if(val&i) MOSI_H();//���ݽ���
		else MOSI_L();
		
		//Fmdelay(1);
		SCK_H();       // ���������ݱ�д�뵽����  
		//Fmdelay(1);  // zzs note,��Щ��ʱ��ȥ���ǲ���ȫ�ģ��д�ȷ��һ����??? zzs�������࣬���磬����Ҫ̫���ģ��ڴ���ٶȡ�
	}
	SCK_L();					//�����Խ�����
}

 /*******************************************************************************
* Function Name  : BSP_SoftSpiSend.
* Description    : ���ģ��SPI
* Input 1        : SPIx where x can be 0,1,2 to select the SPI peripheral.
* Input 2        : PtrToBuffer is an u8 pointer to the first word to be transmitted.
* Input 3        : NbOfWords parameter indicates the number of words to be sent.
* Output         : None.
* Return         : TRUE / FALSE.
*******************************************************************************/
INT8U BSP_SoftSpiRece(INT8U val)
{
	INT8U i = 0;
	INT8U data = 0;
	INT8U Rd = 0;
	
 	for(i=0x80;i!=0;i>>=1)
	{	
		//Fmdelay(1);
		SCK_L();
		//Fmdelay(1);   // �½� ����
		SCK_H();     
		Rd = MISO();
		if(Rd==1) 
		{
			data|=i;    //zhengyu
		}
	
	}
	SCK_L();					//�����Խ�����	
	return data;				
}

/***************************************************************************************
���ƣ�INT8U  SPI_BufferSend(const INT8U *PtrToBuffer, INT32U Len)
���ܣ�SPI����Len�ֽڡ�ÿ1K��ιһ�ι���
��Σ�INT8U *PtrToBuffer, �����͵����ݵ�ַ��INT32U Len�����ͳ���
���Σ���
���أ���Ϊ1
****************************************************************************************/
INT8U  SPI_BufferSend(const INT8U *PtrToBuffer, INT32U Len)
{
	INT16U Num=0;
 
	while(Len)
	{	
		Feed_Dog();
		Num = Len/1024 ? 1024:Len%1024;											//ÿ1K��ιһ�ι�����ֹ���Ź���λ
		Len -= Num;		
		while (Num--)															//дNum�ֽ�
		{
			BSP_SoftSpiSend(*PtrToBuffer++);									//д��ָ������1
		}
	}
	return 1;
}

/***************************************************************************************
���ƣ�INT8U SPI_BufferReceive(INT8U *PtrToBuffer, INT32U Len)
���ܣ�SPI����Len�ֽڡ�ÿ1K��ιһ�ι���
��Σ�INT8U *PtrToBuffer, �������ݺ�Ĵ�ŵ�ַ��INT32U Len�����ճ���
���Σ���
���أ���Ϊ1
****************************************************************************************/
INT8U SPI_BufferReceive(INT8U *PtrToBuffer, INT32U Len)
{
	INT16U Num=0;
 
	while(Len)
	{	
		Feed_Dog();
		Num = Len/1024 ? 1024:Len%1024;											//ÿ1K��ιһ�ι�����ֹ���Ź���λ
		Len -= Num;		
		while (Num--)															//��Num�ֽ�
		{
			*PtrToBuffer++ = BSP_SoftSpiRece(0xff);								//����ָ������1
		}
	}
	return 1;
}
