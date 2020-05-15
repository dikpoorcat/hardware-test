/***********************************************************************************************************************                                    
*
*               (c) Copyright 2017-2030, 杭州方诚电力技术有限公司(http://www.fcdl.com.cn)
*                            All Rights Reserved
*
*---------- File Info ---------------------------------------------------------------
* File name   :  bsp_spi.c
*
* Descriptions:  FM25CL64和W25Q256 挂接在同一组的SPI接口上，因此，如果需要同时使用这两颗存储器的话，
*                则需要以总线方式此驱动，为Bsp_fm和Bsp_W25Q256服务
* 					     在使用本文件中的函数 前后需要增加OSSchedLock OSSchedUnlock 进行锁定保护
*
* Created By  :  赵志舜(11207656@qq.com)
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
/* Note：使用读取或者写入的时候，要进行 open 和 close 的操作 */
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
* Description    : 软件模拟SPI
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
		if(val&i) MOSI_H();//数据建立
		else MOSI_L();
		
		//Fmdelay(1);
		SCK_H();       // 上升沿数据被写入到铁电  
		//Fmdelay(1);  // zzs note,这些延时都去掉是不安全的，有待确认一下子??? zzs查明真相，铁电，不需要太担心，内存的速度。
	}
	SCK_L();					//拉低以降功耗
}

 /*******************************************************************************
* Function Name  : BSP_SoftSpiSend.
* Description    : 软件模拟SPI
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
		//Fmdelay(1);   // 下降 接收
		SCK_H();     
		Rd = MISO();
		if(Rd==1) 
		{
			data|=i;    //zhengyu
		}
	
	}
	SCK_L();					//拉低以降功耗	
	return data;				
}

/***************************************************************************************
名称：INT8U  SPI_BufferSend(const INT8U *PtrToBuffer, INT32U Len)
功能：SPI发送Len字节。每1K就喂一次狗。
入参：INT8U *PtrToBuffer, 待发送的数据地址；INT32U Len，发送长度
出参：无
返回：恒为1
****************************************************************************************/
INT8U  SPI_BufferSend(const INT8U *PtrToBuffer, INT32U Len)
{
	INT16U Num=0;
 
	while(Len)
	{	
		Feed_Dog();
		Num = Len/1024 ? 1024:Len%1024;											//每1K就喂一次狗，防止看门狗复位
		Len -= Num;		
		while (Num--)															//写Num字节
		{
			BSP_SoftSpiSend(*PtrToBuffer++);									//写完指针自增1
		}
	}
	return 1;
}

/***************************************************************************************
名称：INT8U SPI_BufferReceive(INT8U *PtrToBuffer, INT32U Len)
功能：SPI接收Len字节。每1K就喂一次狗。
入参：INT8U *PtrToBuffer, 接收数据后的存放地址；INT32U Len，接收长度
出参：无
返回：恒为1
****************************************************************************************/
INT8U SPI_BufferReceive(INT8U *PtrToBuffer, INT32U Len)
{
	INT16U Num=0;
 
	while(Len)
	{	
		Feed_Dog();
		Num = Len/1024 ? 1024:Len%1024;											//每1K就喂一次狗，防止看门狗复位
		Len -= Num;		
		while (Num--)															//读Num字节
		{
			*PtrToBuffer++ = BSP_SoftSpiRece(0xff);								//收完指针自增1
		}
	}
	return 1;
}
