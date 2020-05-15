/***********************(C) COPY RIGHT 2008 Jking Group**************************
* File Name: Bsp_adc.c
* Author: GuoWei Dong
* Date First Issued: 2015-05-18
* Version: 1.0.0
* Description: AD采样
*********************************************************************************
* History:
* 2018.08.30
* Description: Z.E.整理
*							 
********************************************************************************/
#include "Bsp_adc.h"

#define ADC1_DR_Address    ((u32)0x4001244C) 

INT16U		AD_Value[3];													//增设一路采集，用于法拉电容电压采集（DMA用）
Equi_STA	Equipment_state={ 0, 0, 0 };									//按国网规约组帧时，可直接memcpy


/******************************************************************************* 
* Function Name  : void ADC1_Configuration(void) 
* Description    : ADC1配置，转化顺序，DMA开启
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/
 void ADC1_Configuration(void) 
{
	ADC_InitTypeDef ADC_InitStructure; 

	ADC_Cmd(ADC1, DISABLE);  
	ADC_DMACmd(ADC1, DISABLE);    //关闭ADC1,ADC1 DMA

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                                  //独立ADC模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;                       	         		//ADC扫描模式，使用DMA
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                                  //连续转化
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;                 //软件启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                              //右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 3;                                             //使用三个通道
	ADC_Init(ADC1, &ADC_InitStructure);                                                 //ADC初始化
	ADC_TempSensorVrefintCmd(ENABLE);                                                   //开启内部温度传感器通道
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_239Cycles5);        //PC1对应为ADC channel 11 第一顺序转化，采样时间41.5个周期
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 2, ADC_SampleTime_239Cycles5);        //PC2对应为ADC channel 12 第二顺序转化，采样时间41.5个周期
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 3, ADC_SampleTime_239Cycles5);       //内部温度传感器 channel 16 ，第二顺序转化，采样时间239.5个周期
	ADC_Cmd(ADC1, ENABLE);  
	ADC_DMACmd(ADC1, ENABLE);                                                           //使能ADC，使能ADC DMA
	// Enable ADC1 reset calibaration register 进行校准
	ADC_ResetCalibration(ADC1);  // Check the end of ADC1 reset calibration register
	while(ADC_GetResetCalibrationStatus(ADC1)); 
	// Start ADC1 calibaration
	ADC_StartCalibration(ADC1);  
	while(ADC_GetCalibrationStatus(ADC1)); // ADC校准完成
}

/******************************************************************************* 
* Function Name  : void DMA_Configuration(void) 
* Description    : DMA配置。CMAR=AD_Value数组的地址
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/ 
void DMA_Configuration(void) 
{ 
    DMA_InitTypeDef DMA_InitStructure;   
	DMA_Cmd(DMA1_Channel1, DISABLE); 	                                                //ADC1对应DMA channel 1
    DMA_DeInit(DMA1_Channel1);                                                          //复位
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;                         //CPAR= ADC1的DR寄存器地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&AD_Value;                              //CMAR=AD_Value数组的地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                                  //传输方向由ADC到存储器    
    DMA_InitStructure.DMA_BufferSize = 3;                                               //数据量 3个
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;                    //外设地址无增量
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                             //存储器地址增量
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;         //ADC数据宽度16位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;                 //存储器接收数据宽度16位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                                     //循环传输
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;                                 //优先级高
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                        //关闭M2M
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);                                        //初始化 DMA CH1（ADC1）传输
    DMA_Cmd(DMA1_Channel1, ENABLE);                                                     //开启DMA传输
} 

/******************************************************************************* 
* Function Name  : void AD_Init(void)
* Description    : ADC1初始化
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/ 
void AD_Init(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOC ,ENABLE);			//使能ADC1时钟，使能PC口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);					//开启复用时钟，PC
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);						//使能DMA1时钟
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);										//ADC时钟8分频 

	/*ADC引脚模拟输入检测*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;					//PC1 模拟输入用作ADC_BAT检测，PC2 模拟输入用作ADC_FALA检测
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	/*使能引脚推挽输出高电平*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;						//推挽输出
	GPIO_InitStructure.GPIO_Pin = ADCEN_Pin;								//ADC使能
	GPIO_SetBits(ADCEN_Port, ADCEN_Pin);									//拉高
	GPIO_Init(ADCEN_Port, &GPIO_InitStructure);								

	ADC1_Configuration();
	DMA_Configuration();
}

/******************************************************************************* 
* Function Name  : void AD_LowPower(void)
* Description    : ADC1时钟关闭省电
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/ 
void AD_LowPower(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	ADC_DeInit(ADC1);														//没有这个会增加46uA
	ADC_Cmd(ADC1, DISABLE);													//关ADC外设
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,DISABLE);					//关ADC时钟		不关复用时钟|RCC_APB2Periph_AFIO（串口也用到这个）  实测这两个时钟关不关都不影响
	
	ADC_DMACmd(ADC1, DISABLE);												//关闭ADC1 DMA
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);						//关DMA时钟

	/*使能引脚、两个ADC采样引脚模拟输入*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;						//ADC两个引脚本来就是模拟输入，将50MHz改为2MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;					//PC1、PC2
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
//	GPIO_InitStructure.GPIO_Pin = ADCEN_Pin;								//ADC使能
//	GPIO_Init(ADCEN_Port, &GPIO_InitStructure);
	GPIO_ResetBits(ADCEN_Port, ADCEN_Pin);									//拉低
}
	
/******************************************************************************* 
* Function Name  : u32 GET_ADVALUE(u8 ch,u8 CISHU)   
* Description    : 获取AD转化结果
* Input          : ch：通道
					#define CH_BAT 0
					#define CH_FALA 1
					#define CH_MCUWD 2
	               count：取几次的平均值
* Output         : 
* Return         : 转化后12位AD采样值
*******************************************************************************/ 
u32 GET_ADVALUE(u8 ch,u8 count)       
{
	u32		ADvalue=0;														//typedef unsigned long	u32
	INT8U	i;
	INT16U	temp[20]={0};													//count最多20次采样
  
	for(i=0;i<count;i++)
	{
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);								//开启ADC1转化
		delay_2us_4M(500*50);												//延时20ms
//		OSTimeDly(1);
		BSP_WDGFeedDog(); 
		temp[i]=AD_Value[ch];												//开启DMA传输后，ADC CH_X数据直接传输到AD_Value[X]
	}
	for(i=0;i<count;i++)
	{
		ADvalue+=temp[i];			 
	}
	ADvalue /= count;														//取采样平均值，此为12位AD值

	return ADvalue;		
}
	
/******************************************************************************* 
* Function Name  : u32 Read_Voltage(u8 ch,u8 count)  
* Description    : 获取电池或法拉电容的电压，分压系数是1/4，检测范围0-13.2V
* Input          : ch：通道 CH_BAT 0||CH_FALA 1
	               count：取几次的平均值
* Output         : 
* Return         : 电池或法拉电容电压值采样平均值（实际值的100倍），单位10mV,取小数点后两位
*******************************************************************************/ 
u32 Read_Voltage(u8 ch,u8 count)
{
	u32 Voltage;

	Voltage=GET_ADVALUE(ch,count);
	Voltage=Voltage*330/4096*4;												//单位10mV,系数4，默认参考电压3.3V
	
	Voltage = Voltage-4.9;													//补偿
	
	return Voltage;       
}

/******************************************************************************* 
* Function Name  : u32 Read_MCU_Temp( u8 count )
* Description    : 获取MCU内部温度，进行count次采样取平均值，采样周期最长。单位：℃*100
* Input          :  u8 count ，采样次数
* Output         : 
* Return         : 返回MCU温度的100倍，取小数点后两位
*******************************************************************************/ 
u32 Read_MCU_Temp( u8 count )
{
	u32 Temp;
	u32 MCU_Temp; 
	Temp=GET_ADVALUE(CH_MCUWD,1);
	
	MCU_Temp=(u32)((1.43-Temp*3.3/4096)*1000/4.3+25)*100 ; //取小数点后两位
	return MCU_Temp;   //返回温度的100倍
}

/******************************************************************************* 
* Function Name  : void ADC_HEXtoASCII(u32 In,u8 *pOut)
* Description    : 将电压值，温度转化为字符输出
* Input          : u32 In
* Output         : u8 *pOut
* Return         : 无
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
* Description    : 测试打印V_BAT、V_FALA、MCUWD
* Input          : 无
* Output         : 无
* Return         : 无
*******************************************************************************/ 
void ReadVCC_Test(void)
{
	u32 Vbat=0,Vfala=0,Tmcu=0;

	AD_Init();
	Vbat=Read_Voltage(CH_BAT,5);											//取5次采样平均值 
    Print_Voltage( CH_BAT, Vbat );
	
	Vfala=Read_Voltage(CH_FALA,5);
	Print_Voltage( CH_FALA, Vfala );
	
	Tmcu=Read_MCU_Temp( 5 );
	Print_Voltage( CH_MCUWD, Tmcu );
	
	OSTimeDly(5);															//等待打印完成
}

/*******************************************************************************
名称：void Print_Voltage( INT8U Channel, INT32U Voltage )
功能：选择需要打印的类型，并输入值，可从485打印出测试信息
入参：INT8U Channel, 通道；INT32U Voltage，需要打印的值
		#define CH_BAT 0
		#define CH_FALA 1
		#define CH_MCUWD 2
出参：无
返回：无
*******************************************************************************/
void Print_Voltage( INT8U Channel, INT32U Voltage )
{
	INT8U	temp[7]={0};
	
	ADC_HEXtoASCII( Voltage, temp );											//转换为字符串
	
	switch( Channel ){
		case CH_BAT:
			BspUartWrite(2,SIZE_OF("V_BAT---->"));								//输出V_BAT
			break;
		case CH_FALA:
			BspUartWrite(2,SIZE_OF("V_FALA--->"));								//输出V_FALA
			break;
		case CH_MCUWD:
			BspUartWrite(2,SIZE_OF("MCUWD---->"));								//输出单片机温度
			break;
		default:
			BspUartWrite(2,SIZE_OF("未指定通道"));
			break;
	}
	BspUartWrite( 2, temp, 7 );
}

/*******************************************************************************
名称：void Get_Voltage_MCUtemp_Data( INT8U Count )
功能：获取电池电压数据和单片机温度，多次采样取平均值并转换为浮点，存入Equipment_state结构体，
并进行故障判断。若485已开放，可将采集到的数据和故障信息，从485打印出来（函数内无485初始化）。
入参：INT8U Count，采样的次数
出参：无
返回：无
*******************************************************************************/
void Get_Voltage_MCUtemp_Data( INT8U Count )
{
	INT32U	Vbat=0,Vfala=0,Tmcu=0;
	
	/*清上次数据*/
	Equipment_state.BAT_Volt= 0;											
	Equipment_state.FALA_Volt= 0;
	Equipment_state.MCU_Temp= 0;
	
	/*采集*/
	AD_Init();																//初始化ADC
	Vbat= Read_Voltage( CH_BAT, Count );									//电池电压采样Count次，返回平均值（实际值的100倍），INT32U
	Vfala= Read_Voltage( CH_FALA, Count );									//法拉电容电压采样Count次，返回平均值（实际值的100倍），INT32U
	Tmcu= Read_MCU_Temp( Count );											//单片机温度电压采样Count次，返回平均值（实际值的100倍），INT32U
    AD_LowPower();

	/*打印测试信息，默认外面已开启485*/
	Print_Voltage( CH_BAT, Vbat );
	Print_Voltage( CH_FALA, Vfala );
	Print_Voltage( CH_MCUWD, Tmcu );
	
	/*存入结构体*/
	Equipment_state.BAT_Volt= ( (float)Vbat ) / 100;						//转换为浮点（后台要求的数据类型），计算真实值并转存到Equipment_state（原为100倍）
	Equipment_state.FALA_Volt= ( (float)Vfala ) / 100;						//转换为浮点（后台要求的数据类型），计算真实值并转存到Equipment_state（原为100倍）
	Equipment_state.MCU_Temp= ( (float)Tmcu ) / 100;						//转换为浮点（后台无此要求，数据类型与环境温度相同），计算真实值并转存到Equipment_state（原为100倍）
	
	/*故障判断*/
	if((Equipment_state.BAT_Volt<BAT_UNDER) && (!Fault_Manage.F_BAT)) 		//电池电压<9.2V，且无故障标志
		NW_Fault_Manage(BAT_F, FAULT_STA);									//上报电池欠压故障		
	else if((Equipment_state.BAT_Volt>BAT_UP) && Fault_Manage.F_BAT)		//电池电压>9.5V，且有故障标志	
		NW_Fault_Manage(BAT_F, NOFAULT_STA);								//上报电压欠压故障恢复	
}

/*******************************************************************************
名称：INT8U HB_Get_Voltage( INT8U Count )
功能：获取电池电压数据和单片机温度，多次采样取平均值并转换为浮点，存入Equipment_state结构体
	若485已开放，可将采集到的数据从485打印出来（函数内无485初始化）
入参：count：取count次采样的平均值
出参：无
返回：心跳组帧需要的电池电压数值，单位100mv
*******************************************************************************/
INT8U HB_Get_Voltage( INT8U Count )
{
	INT8U Vbat;
	AD_Init();	
	Vbat=(INT8U) (Read_Voltage(CH_BAT,Count)/10);							//需求一个字节，单位100mv														
	AD_LowPower();
	return Vbat;
}
