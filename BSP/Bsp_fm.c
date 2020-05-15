/***********************************************************************************************************************                                    
*
*               (c) Copyright 2017-2030, 杭州方诚电力技术有限公司(http://www.fcdl.com.cn)
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
* Descriptions: FM25CL64和W25Q256共用同一个SPI接口	
*
*-------------------------------------------------------------------------------------
************************************************************************************************************************/
 #include "Bsp_fm.h"
 
INT8U FM_Flag=0;				//bit	7	6 	5 	4 	3 	2 	1 	0		各任务占用标志位
								//任务	X	X	X	X	WDT	GY	RF	GPRS	









/* --------------------Private functions------------------------------------------------------*/
/************************************************************************************************************************
* Function Name : void Fmdelay(INT16U US)
* Description   : 延时函数
* Input         : US : 等待US个时钟周期
* Return        : None
*************************************************************************************************************************/
void Fmdelay(INT16U US)
{
	while(US--);
}

/************************************************************************************************************************
* Function Name : INT8U FmbitReadState(void)
* Description   : 读取状态字
* Input         : None
* Return        : status : FM25CL64从SPI口返回的状态字节
*************************************************************************************************************************/
INT8U FmReadState(void)
{
	INT8U status = 0;

	FMCS_H();
	Fmdelay(400);																// 等待稳定
	FMCS_L();
	
	BSP_SoftSpiSend(FM_RDSR);
	status = BSP_SoftSpiRece(0xff);
	FMCS_H();
	
	return status;
 }

/************************************************************************************************************************
* Function Name : void BSP_InitFm(void)
* Description   : 铁电存储器FM25CL64 ，相关驱动硬件的初始化
* Input         : None
*
* Return        : None
*************************************************************************************************************************/
void BSP_InitFm( INT8U Task_Num )												//7~0	任务	X	X	X	X	Wdt	GY	RF	GPRS	
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
		
	FM_Flag|=(1<<Task_Num);
	if( FM_Flag&~(1<<Task_Num) ) return;										//若其他任务已初始化，FM_Flag非0，直接返回	
	
	__disable_irq();															// 禁止总中断     // zzs Modified it 2018.06.11
	
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
	Fmdelay(10);																//等待稳定
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

	Fmdelay(20);																//等待稳定
	
	__enable_irq();																// 开总中断	  // zzs Modified it 2018.06.11
}

/************************************************************************************************************************
* Function Name : INT8U BSP_WriteDataToFm(INT16U FlashAddr,INT8U *DataAddr,INT32U Len)
* Description   : 将数据写入到铁电中去(Write Data To Fm25CL64)
* Input         : FlashAddr : 指示铁电存储器的地址 范围：0~ MAX_FM_LEN
*                 pDataAddr : 要写入数据的存放地址
*                 Len       : 写入的数据长度
*
* Return        : 显式返回： 返回变量 SPI_statu 永远恒等于 1了。永远返回 真
*************************************************************************************************************************/
INT8U BSP_WriteDataToFm(INT16U FlashAddr,INT8U *pDataAddr,INT32U Len)
{
	INT8U SPI_statu = 1;

	if((FlashAddr >= MAX_FM_LEN) || ((FlashAddr + Len) > MAX_FM_LEN) || (pDataAddr == 0))  //zzs note,如果用其他编译器,则(pDataAddr == NULL)
	{
		return 0;
	}

	__disable_irq();															// 禁止总中断     // zzs Modified it 2018.06.11

	FMCS_H();
	FMWP_H();
	FMSCK_L();
	Fmdelay(10);																// 等待稳定
	FMCS_L(); 
	BSP_SoftSpiSend(FM_WREN);

	FMCS_H(); 
	FMCS_L();

	BSP_SoftSpiSend(FM_WRITE);
	BSP_SoftSpiSend(FlashAddr>>8);
	BSP_SoftSpiSend(FlashAddr);
	SPI_statu = SPI_BufferSend(pDataAddr,Len);									// zzs note, 这个SPI_statu 永远 = 1，为恒真了

	FMCS_H();
	FMWP_L();

	__enable_irq();																// 开总中断	  // zzs Modified it 2018.06.11

	return SPI_statu;
}

/************************************************************************************************************************
* Function Name : INT8U BSP_ReadDataFromFm(INT16U FlashAddr,INT8U *pDataAddr,INT32U Len)
* Description   : 从铁电中读出数据(Read Data From Fm25CL64)
* Input         : FlashAddr : 指示铁电存储器的地址 范围：0~ MAX_FM_LEN
*                 pDataAddr : 读取出来的存放地址
*                 Len       : 读取的数据长度
*
* Return        : 隐式返回： 通过形参pDataAddr返回读取的数据
*                 显式返回： 返回变量 SPI_statu 永远恒等于 1了。永远返回 真
*************************************************************************************************************************/
INT8U BSP_ReadDataFromFm(INT16U FlashAddr,INT8U *pDataAddr,INT32U Len)
{
	u8 SPI_statu = 1;

	if((FlashAddr >= MAX_FM_LEN) || ((FlashAddr + Len) > MAX_FM_LEN) || (pDataAddr == 0))   //zzs note,如果用其他编译器,则(pDataAddr == NULL)
	{
	 return 0;
	}

	__disable_irq();															// 禁止总中断     // zzs Modified it 2018.06.11

	FMCS_H();
	FMSCK_L();
	Fmdelay(10);																//等待稳定
	FMCS_L();
	BSP_SoftSpiSend(FM_READ);
	BSP_SoftSpiSend(FlashAddr>>8);
	BSP_SoftSpiSend(FlashAddr);
	SPI_statu = SPI_BufferReceive(pDataAddr,Len);								// zzs note, 这个SPI_statu 永远 = 1，为恒真了
	FMCS_H();

	__enable_irq();																// 开总中断	  // zzs Modified it 2018.06.11

	return SPI_statu;
 }

#if 0	//INT8U FM_test(void)
/************************************************************************************************************************
* Function Name : INT8U FM_test(void)
* Description   : 测试铁电存储器。1、执行一下相关驱动I/O口的初始化；
*                                2、读取铁电存储器的状态；
*                                3、准备好一个测试数组，填充测试数据后，然后将其写入到铁电中去；
*                                4、再从铁电中读出刚才写入的测试数据。
*                                5、对比检查读出的数据是否与写入的相同，若相同，判为测试成功，返回1，若有不同，判为测试失，败返回0
* Input         : None        
* Return        : 0: 测试失败
*                 1：测试成功                   
*************************************************************************************************************************/
extern INT8U FMTESTBUFF[];  // zzs add this
INT16U ValueCnt = 0; 
INT8U FM_test(void)
{
	//INT8U FMTESTBUFF[256];   // zzs note重复定义的内部数组，因为已经有一个完全一样的外部全局数组了
	INT16U i = 0;
	INT8U j = 0;
	
	
	BSP_InitFm(5);
 	FmReadState();

	#if 0
 	for(i = 0;i < 256; i++) FMTESTBUFF[i]=i;
	BSP_WriteDataToFm(0x1800,FMTESTBUFF,256);

	for(i = 0;i < 256; i++) FMTESTBUFF[i]=0;
	BSP_ReadDataFromFm(0x1800,FMTESTBUFF,256);

	for(i = 0;i < 256; i++)
	{
		if (FMTESTBUFF[i]!=i)
			return 0;//FM测试失败
	}
	#else
	for(j=0;j<32;j++)
	{
		for(i = 0;i < 256; i++) FMTESTBUFF[i] = 0x00; // 0xFF;
		
		BSP_WriteDataToFm(j*256,FMTESTBUFF,256);
		#if 1
		BSP_ReadDataFromFm(j*256,FMTESTBUFF,256);
		for(i = 0;i < 256; i++)
		{
			if (FMTESTBUFF[i]!=0x00) // 0xFF 
			{
				ValueCnt++;
			}
		}
		#else
		BSP_WriteDataToFm(j*256,FMTESTBUFF,256);
		#endif
	}
	
	#endif

	return 1;//FM 测试成功
}
#endif

/******************************************************************************* 
* Function Name  : void FM_LowPower(void)
* Description    : FM进入低功耗，并对相应IO口作低功耗处理
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/
void FM_LowPower( INT8U Task_Num )												//7~0	任务	X	X	X	X	Wdt	GY	RF	GPRS	
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	FM_Flag&=~(1<<Task_Num);													//清当前任务bit	
	if(FM_Flag) return;															//若其他任务占用，FM_Flag非0，直接返回   
	
	/*片选口、写保护口模拟输入*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;								//模拟输入
	
	GPIO_InitStructure.GPIO_Pin = NCS_FM_PIN;									//片选口
	GPIO_Init(NCS_FM_Port, &GPIO_InitStructure);								//
	
	GPIO_InitStructure.GPIO_Pin = NWP_FM_PIN;									//写保护口
	GPIO_Init(NWP_FM_Port, &GPIO_InitStructure);								//	

	
	/*SPI三个引脚推挽拉低*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;							//推挽输出
	
	GPIO_InitStructure.GPIO_Pin = MOSI_FM_PIN;									//MOSI
	GPIO_ResetBits(MOSI_FM_Port, MOSI_FM_PIN);									//拉低
	GPIO_Init(MOSI_FM_Port, &GPIO_InitStructure);								//
	
	GPIO_InitStructure.GPIO_Pin = MISO_FM_PIN;									//MISO
	GPIO_ResetBits(MISO_FM_Port, MISO_FM_PIN);									//拉低
	GPIO_Init(MISO_FM_Port, &GPIO_InitStructure);								//
	
	GPIO_InitStructure.GPIO_Pin = SCK_FM_PIN;									//SCK
	GPIO_ResetBits(SCK_FM_Port, SCK_FM_PIN);									//拉低
	GPIO_Init(SCK_FM_Port, &GPIO_InitStructure);								//
}

/************************************************************************************************************************
* Function Name : INT8U BSP_FM_Erase(INT16U FlashAddr,INT32U Len)
* Description   : 从铁电某起始地址开始向后擦除Len长度空间（填0）
* Input         : FlashAddr : 指示铁电存储器的地址 范围：0~ MAX_FM_LEN
*                 Len       : 需要擦除的长度
* Return        : 1：成功	0：失败
*************************************************************************************************************************/
INT8U BSP_FM_Erase(INT16U FlashAddr,INT32U Len)
{
	INT32U Num=0; 
	
	if((FlashAddr >= MAX_FM_LEN) || ((FlashAddr + Len) > MAX_FM_LEN)) return 0; //zzs note,如果用其他编译器,则(pDataAddr == NULL)

	__disable_irq(); 															// 禁止总中断     // zzs Modified it 2018.06.11

	FMCS_H();
	FMWP_H();
	FMSCK_L();
	Fmdelay(10);  																// 等待稳定
	FMCS_L(); 
	BSP_SoftSpiSend(FM_WREN);

	FMCS_H(); 
	FMCS_L();
	
	/*写入待擦除起始地址*/
	BSP_SoftSpiSend(FM_WRITE);
	BSP_SoftSpiSend(FlashAddr>>8);
	BSP_SoftSpiSend(FlashAddr);

	/*开始按字节擦除*/
	while(Len)
	{	
		Feed_Dog();
		Num = Len/1024 ? 1024:Len%1024;											//每1K就喂一次狗，防止看门狗复位
		Len -= Num;		
		while(Num--)															//写Num个0
		{
			BSP_SoftSpiSend(0);
		}
	}
	FMCS_H();
	FMWP_L();
	
	__enable_irq();																// 开总中断	  // zzs Modified it 2018.06.11
	
	return 1;
}
