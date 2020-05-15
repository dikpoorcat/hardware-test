/*****************************************Copyright(C)******************************************
*******************************************方诚电力*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName          : Meteorology.c
* Author            : Z.E.
* Date First Issued : 2018-08-30
* Version           : V1.0
* Description       : 微气象数据处理 
 *----------------------------------------历史版本信息-------------------------------------------
* History       :V1.0
* Description   : 
	               
*-----------------------------------------------------------------------------------------------
*******************************************************************************/
#include "Meteorology.h"

#define TEST	01									//打印开关

//全局变量定义
INT16U	WindSpeed=0;								//机械式传感器风速
INT16U	WindDirection=0;							//机械式传感器风向
BMP180_info	BP_info;								//保存大气压传感器数据
MET_Data_TypeDef	MET_Data;						//保存微气象数据



/***************************************************************************************
名称：void MET_Main(void *org)
功能：微气象任务主函数
入参：无
出参：无
返回：无
****************************************************************************************/
void MET_Main(void *org)
{
#if 0	/*ADC标定*/
	INT8U	T=100;
	
	while( T-- )
	{
		if( T==0)
		{						
			B485_init(4800);								//B485串口初始化（统一成与风速风向传感器默认值相同）
			PWDC485EN();									//DCDC隔离485打开	
			OSTimeDly(2);			
			T=100;
		}
		ReadVCC_Test();
		BSP_UART_Write(2,(INT8U *)("\r\n"),2);
		OSTimeDly(3*20);
	}
#endif
#if TEST
	Test_Meteorology_Data(1);						//微气象数据采集，失败重试3次（温湿度、大气压、风速风向）
//	SetGPRSON();									//Resume挂起的GPRS任务,GPRSON=1--------------------------------------------------------移值时这里要处理！！！
#endif
	while(1)
	{				
		WDTClear(0);								//喂狗？
		OSTimeDly(20);
#if TEST
		if( gRtcTime.Minute == 0x49 || gRtcTime.Minute == 0x19 )				//测试中一小时两次
#else
		if( gRtcTime.Minute == 0x49 )				//每小时49分时开始采温湿度、大气压、风速风向数据（RTC任务中0.9秒读一次时间）
#endif
		{
			Get_Meteorology_Data(5);				//微气象数据采集，失败重试5次（温湿度、大气压、风速风向）
			//MET_SaveData(CurrentTime);			//保存当前时间，湿度，温度，气压，风速，风向数据		

			/*电池电压检测，决定发射与否*/
		#if TEST
			PWDC485EN();							//DCDC隔离485打开（正式程序不打印可去掉）
		#endif
			Get_Voltage_MCUtemp_Data(3);			//电池电压、单片机温度采集3次取平均值（在此打印信息）			
			if( Equipment_state.BAT_Volt>8.5 || Equipment_state.FALA_Volt>5 )
			{
//				SetGPRSON();						//Resume挂起的GPRS任务,GPRSON=1--------------------------------------------------------移值时这里要处理！！！
			}

			OSTimeDly(60*20);						//延时1min，防止1分钟内重入
		}
	}
}

/***************************************************************************************
名称：INT8U Get_Meteorology_Data( INT8U retry )
功能：10min平均风速风向、最大网速、温湿度、大气压数据采集。先采集风速10min，最后采集温度等
入参：INT8U	retry，采集失败重试次数
出参：无
返回：bit1温湿度采集错误，bit2大气压采集错误，bit3风速传感器错误，bit4风向传感器错误，返回0采集正确
****************************************************************************************/
INT8U Get_Meteorology_Data( INT8U retry )
{
	INT8U 	i,rt,err=0;
	INT8U 	MaxIndex=0;								//最大风速
	INT16U	WD_Max=0,WS_Sum=0;						//用于计算平均值
	INT16U	WindSpeed_arr[10]={0};					//10min风速
	INT16U	WindDirection_arr[10]={0};				//10min风向
	INT16S	Temp_16S=0;								//有符号整型，用于浮点转换
	
	Am2302_Init();									//Am2302传感器初始化
	PowerMETPin_Init();								//温湿度电源控制 引脚配置
	PowerWDSPPin_Init();							//风速风向电源控制 引脚配置
	
	/*10min平均风速风向数据采集（err处理方式要求先采集风速风向）*/
	for(i=0;i<10;i++)								//连续采集10min风速风向
	{
		OSTimeDly( 57*20 );							//延时1min（下句内有3秒）再采集（50分~59分）
		for( rt=retry; rt>0; rt-- )
		{
			err= Get_WDSP_Data( WindSpeed_arr+i, WindDirection_arr+i );	//风速风向数据采集，并记录错误标志位（以最后一次的标志为准，不可写成|=，否则无法清掉以前的错误）
			if( err==0 ) break;						//风速风向数据采集正确（返回0）时跳出；错误时继续循环
		}
		
		if( WindSpeed_arr[i] > WD_Max )				//判断最大风速
		{
			WD_Max= WindSpeed_arr[i];				//保存最大风速
			MaxIndex= i;							//保存最大风速相对地址
		}

		WS_Sum += WindSpeed_arr[i];					//累加计算总风速
	}
	MET_Data.Max_WindSpeed= ((float)WD_Max)/10;		//将最大风速转换为浮点（后台要求的数据类型），转存到MET_Data
	MET_Data.Ave_WindSpeed= ((float)WS_Sum)/100;	//计算10次的平均风速，并转换为浮点（后台要求的数据类型），转存到MET_Data（100表示10次*10倍）
	MET_Data.Ave_WindDirection= WindDirection_arr[MaxIndex];				//记录当前风向（0~7或0~360度，已为后台要求的数据类型）
	
	
	PWMETEN();										//打开温湿度、大气压电源
	OSTimeDly(40);									//AM2302上电后要等待2S，以越过不稳定状态，
	
	/*温湿度数据采集*/
	for( rt=retry; rt>0; rt-- )
	{
		if( Read_Median_AM2302(&AM2302_Data,3) ) 	//采样3次，取中位数温湿度（补码）（成功返回1）（读取值为实际温度的10倍）
		{
			err &= ~0x01;							//标志位bit1，清除温湿度传感器错误
			break;
		}
		else err |= 0x01;							//标志位bit1，记录温湿度传感器错误
	}
	Temp_16S= (AM2302_Data.temp_H<<8)+AM2302_Data.temp_L;					//取得带符号整型温度（已为补码）--------< 要先转换成INT16S >
	MET_Data.Air_Temperature= ( (float)Temp_16S ) / 10;						//转换为浮点（后台要求的数据类型），计算真实值并转存到MET_Data（原为10倍）
	MET_Data.Humidity= (AM2302_Data.humi_H<<8)+AM2302_Data.humi_L;			//湿度数据转存到MET_Data（此时为16进制整数，1000倍湿度值，例652=65.2%，符合后台要求的数据类型）
	
	/*大气压数据采集*/
	BMP180Init(&BP_info);							//大气压模块初始化
	if( BP_info.ExistFlag==BMP180_EXISTENCE )
	{
		BMP180Convert(&BP_info);
	}
	else
	{
		err |= 0x02;								//标志位bit2，记录大气压传感器错误
	}
	MET_Data.Air_Pressure= ((float)BP_info.GasPress)/1000;   				//大气压数据转换为浮点（后台要求的数据类型），并转存到MET_Data
				
	MET_LowPower();									//低功耗	关闭温湿度、大气压电源，并配置IO
#if TEST
	Err_report( err );								//打印错误信息（会关485电源）
#endif
	
	return err;										//正确返回0
}

/***************************************************************************************
名称：INT8U MET_packet_content( INT8U *OutBuff )
功能：微气象帧组帧
入参：INT8U *OutBuff，存放地址
出参：INT8U *OutBuff，存放地址
返回：帧长度
****************************************************************************************/
INT8U MET_packet_content( INT8U *OutBuff )
{
	INT32U	T=0;
	INT8U	MET_packet[69]={0};

//小端模式，低字节在低地址（低在前）
	NB_ReadID(0x1000,MET_packet,(INT16U *)&T);			//6.Component_ID	被监测设备ID（17位编码）[0--16]。使用本机的ID地址
	if( T!=17 ) return 0;

	T=RTC_GetTime_Second();								//读RTC时间
	if(T==0)return 0;
	MET_packet[17]=T&0xff;								//7.Time_Stamp	采集时间
	MET_packet[18]=(T>>8)&0xff;
	MET_packet[19]=(T>>16)&0xff;
	MET_packet[20]=(T>>24)&0xff;							

	MET_packet[21]=0;									//8.Alerm_Flag	报警标识
	MET_packet[22]=0; 

	//memcpy操作结构体时注意对齐问题
	memcpy(MET_packet+23,&MET_Data,46);					//将MET_Data（微气象数据）填入MET_packet（微气象报文内容）。MET_Data数据类型已在Get_Meteorology_Data()中转换为后台要求的数据类型
	return GDGuiYue(OutBuff,MET_packet,69,1,1);			//按Q/GDW 242-2010国网规约组帧，其中报文内容源自*P
}

/***************************************************************************************
名称：void Err_report( INT8U Err )
功能：微气象数据采集错误时485打印相应错误信息
入参：INT8U Err，错误代码
出参：无
返回：无
****************************************************************************************/
void Err_report( INT8U Err )
{	
	B485_init(4800);								//重新初始化，防止其他任务关串口导致失效。传感器默认波特率为4800
	PWDC485EN();									//DCDC隔离485打开	
	OSTimeDly(1);
	
	if( Err==0 )
	{
		BSP_UART_Write(2,(INT8U *)("\r\n——————————微气象传感器正常！——————————\r\n"),18+44);
	}
	if( Err&0x01 )
	{
		BSP_UART_Write(2,(INT8U *)("\r\n——————————温湿度传感器错误！——————————\r\n"),18+44);
	}
	if( Err&0x02 )
	{
		BSP_UART_Write(2,(INT8U *)("\r\n——————————大气压传感器错误！——————————\r\n"),18+44);
	}
	if( Err&0x04 )
	{
		BSP_UART_Write(2,(INT8U *)("\r\n——————————风速传感器错误！——————————\r\n"),16+44);
	}
	if( Err&0x08 )
	{
		BSP_UART_Write(2,(INT8U *)("\r\n——————————风向传感器错误！——————————\r\n"),16+44);
	}

	OSTimeDly(10);									//等待485打印完成
	PWDC485DIS();									//关闭485电源
}

/***************************************************************************************
名称：INT8U Test_Meteorology_Data( INT8U retry )
功能：测试风速风向（10次）、最大网速、温湿度、大气压数据采集功能。
入参：INT8U	retry，采集失败重试次数
出参：无
返回：bit1温湿度采集错误，bit2大气压采集错误，bit3风速传感器错误，bit4风向传感器错误，返回0采集正确
****************************************************************************************/
INT8U Test_Meteorology_Data( INT8U retry )
{
	INT8U 	i,rt,err=0;
	INT8U 	MaxIndex=0;								//最大风速
	INT16U	WD_Max=0,WS_Sum=0;						//用于计算平均值
	INT16U	WindSpeed_arr[10]={0};					//10min风速
	INT16U	WindDirection_arr[10]={0};				//10min风向
	INT16S	Temp_16S=0;								//有符号整型，用于浮点转换
	INT8U	temp[7]={0};							//转ASCII，打印用
	
	Am2302_Init();									//Am2302传感器初始化
	PowerMETPin_Init();								//温湿度电源控制 引脚配置
	PowerWDSPPin_Init();							//风速风向电源控制 引脚配置
	
	/*风速风向数据采集（err处理方式要求先采集风速风向）*/
	for(i=0;i<3;i++)								//连续采集3次风速风向
	{
//		OSTimeDly( 57*20 );							//延时1min（下句内有3秒）再采集（50分~59分）
		for( rt=retry; rt>0; rt-- )
		{
			err= Get_WDSP_Data( WindSpeed_arr+i, WindDirection_arr+i );	//风速风向数据采集，并记录错误标志位（以最后一次的标志为准，不可写成|=，否则无法清掉以前的错误）
			if( err==0 ) break;						//风速风向数据采集正确（返回0）时跳出；错误时继续循环
		}
		
		if( WindSpeed_arr[i] > WD_Max )				//判断最大风速
		{
			WD_Max= WindSpeed_arr[i];				//保存最大风速
			MaxIndex= i;							//保存最大风速相对地址
		}

		WS_Sum += WindSpeed_arr[i];					//累加计算总风速
	}
	MET_Data.Max_WindSpeed= ((float)WD_Max)/10;		//将最大风速转换为浮点（后台要求的数据类型），转存到MET_Data
	MET_Data.Ave_WindSpeed= ((float)WS_Sum)/30;		//计算3次的平均风速，并转换为浮点（后台要求的数据类型），转存到MET_Data（30表示3次*10倍）
	MET_Data.Ave_WindDirection= WindDirection_arr[MaxIndex];				//记录当前风向（0~7或0~360度，已为后台要求的数据类型）
	
	PWDC485EN();									//DCDC隔离485打开（Get_WDSP_Data中被关闭）
	WS_Sum *= 10;									//计算平均风速（10倍，整型）
	ADC_HEXtoASCII( WS_Sum, temp );					//转换为字符串
	BSP_UART_Write(2,(INT8U *)("\r\n风速："),8);
	BSP_UART_Write( 2, temp, 7 );
	BSP_UART_Write(2,(INT8U *)("风向："),6);
	WS_Sum= MET_Data.Ave_WindDirection + 0x30;		//风向转ASCII
	BSP_UART_Write(2,(INT8U *)&WS_Sum,2);
	
	
	
	PWMETEN();										//打开温湿度、大气压电源
	OSTimeDly(40);									//AM2302上电后要等待2S，以越过不稳定状态，
	
	/*温湿度数据采集*/
	for( rt=retry; rt>0; rt-- )
	{
		if( Read_Median_AM2302(&AM2302_Data,3) ) 	//采样3次，取中位数温湿度（补码）（成功返回1）（读取值为实际温度的10倍）
		{
			err &= ~0x01;							//标志位bit1，清除温湿度传感器错误
			break;
		}
		else err |= 0x01;							//标志位bit1，记录温湿度传感器错误
	}
	Temp_16S= (AM2302_Data.temp_H<<8)+AM2302_Data.temp_L;					//取得带符号整型温度（已为补码）--------< 要先转换成INT16S >
	MET_Data.Air_Temperature= ( (float)Temp_16S ) / 10;						//转换为浮点（后台要求的数据类型），计算真实值并转存到MET_Data（原为10倍）
	MET_Data.Humidity= (AM2302_Data.humi_H<<8)+AM2302_Data.humi_L;			//湿度数据转存到MET_Data（此时为16进制整数，1000倍湿度值，例652=65.2%，符合后台要求的数据类型）
	
	Temp_16S *= 10;									//温度
	ADC_HEXtoASCII( Temp_16S, temp );				//转换为字符串（负数无法处理）
	BSP_UART_Write(2,(INT8U *)("\r\n温度："),8);
	BSP_UART_Write( 2, temp, 7 );
	
	Temp_16S= MET_Data.Humidity * 10;				//湿度
	ADC_HEXtoASCII( Temp_16S, temp );				//转换为字符串
	BSP_UART_Write(2,(INT8U *)("湿度："),6);
	BSP_UART_Write( 2, temp, 7 );	
	
	/*大气压数据采集*/
	BMP180Init(&BP_info);							//大气压模块初始化
	if( BP_info.ExistFlag==BMP180_EXISTENCE )
	{
		BMP180Convert(&BP_info);
	}
	else
	{
		err |= 0x02;								//标志位bit2，记录大气压传感器错误
	}
	MET_Data.Air_Pressure= ((float)BP_info.GasPress)/1000;   				//大气压数据转换为浮点（后台要求的数据类型），并转存到MET_Data
	
	for(i=0;i<7;i++)								//转换为字符串
	{
		if(i==2) continue ; 						//跳过小数点位temp[4]
		temp[6-i]=BP_info.GasPress%10+0x30;
		BP_info.GasPress/=10;
	}
	temp[4]='.';

	BSP_UART_Write(2,(INT8U *)("气压："),6);
	BSP_UART_Write( 2, temp, 7 );
	OSTimeDly(10);									//等待485打印完成
	
	MET_LowPower();									//低功耗	关闭温湿度、大气压电源，并配置IO
	Err_report( err );								//打印错误信息	

	return err;										//正确返回0
}


#if 0
/***************************************************************************
函数: void MET_DataHandle(void)
说明: 
入参：
出参：
*****************************************************************************/
void MET_DataHandle(void)
{
	INT8U	WaitTime=3;
	INT8U	CurrentTime[6];							//存放当前时间
	
//	CRTime=0;										//调用时清0		不可清0，会重入
	GetSysTime(CurrentTime);						//获取当前时间------------------------被注释了ZE 为什么也能读出？
	if( CurrentTime[4] == 0x05 )					//05分时采温湿度、大气压数据
	{
		if(CRTime==CurrentTime[4])return;			//防止同一分钟内再入（CRTime被清后不影响首次进入）
		CRTime=CurrentTime[4];						//记录当前分钟
		Get_Meteorology_Data();						//读温湿度，大气压数据		
	}
	if(CurrentTime[4] ==0x10)						//10-5分钟后读取风速风向数据
	{
		if(CRTime==CurrentTime[4])return;			//防止同一分钟内再入（CRTime被清后不影响首次进入）	
		CRTime=CurrentTime[4];						//记录当前分钟
		Read_WDSP_Data();							//打开485并初始化-----------------------未读数ZE  函数没做完整，到时候把下面的读数据操作放进此函数

		while(WaitTime--)							//等数据，最多WaitTime次
		{
		/**************获取风速值*********************************/	
			BSP_UART_Write(2,FS_CMD,8);				//发送风速命令

			if(B485WaitData(1)==1)					//等邮箱1s，并对485接收到的数据进行判断并处理，成功返回1
			{
				MET_Data.Ave_WindSpeed=WindSpeed;	//记录当前风速
				break;
			}
		}
		/**************获取风向值*********************************/				
		WaitTime=3;									//重置WaitTime
		while(WaitTime--)							//等数据，最多WaitTime次
		{
			BSP_UART_Write(2,FX_CMD,8);				//发送风向命令
			if(B485WaitData(1)==1)					//等邮箱1s，并对485接收到的数据进行判断并处理，成功返回1
			{
				MET_Data.Ave_WindDirection=WindDirection;	//记录当前风向
				break;
			}
		}	
		/***********关电源，保存数据，启动GPRS***********************/
		PWDC485DIS();								//关闭485电源
//		MET_SaveData(CurrentTime);					//保存当前时间，湿度，温度，气压，风速，风向数据
		PWWDSPDIS();								//关闭风速风向电源
//		SetGPRSON();								//Resume挂起的GPRS任务,GPRSON=1--------------------------------------------------------移值时这里要处理！！！
	}
	
	
//	if((CurrentTime[4] ==0x29) &&(CurrentTime[3]==0x03))		//每日3时29分  判断并擦除SPI FLASH
//	{
//		/****************每月30日或31日清空保存的数据**********************/
//		if(CurrentTime[1]<8)//前7个月
//		{
//			if(CurrentTime[1]%2==0)//30天
//			{
//				if(CurrentTime[3]==0x30)//日
//				{
//					W25QXX_Erase_Chip();			//擦除一遍保存的数据
//				}
//			}
//			else//31天
//			{
//				if(CurrentTime[3]==0x31)//日
//				{
//					W25QXX_Erase_Chip();			//擦除一遍保存的数据
//				}
//			}
//		}
//		else//后5个月
//		{
//			if(CurrentTime[1]%2==0)//30天
//			{
//				if(CurrentTime[3]==0x30)//日
//				{
//					W25QXX_Erase_Chip();			//擦除一遍保存的数据
//				}
//			}
//			else//31天
//			{
//				if(CurrentTime[3]==0x31)//日
//				{
//					W25QXX_Erase_Chip();			//擦除一遍保存的数据
//				}
//			}
//		}
//		OSTimeDly(70);
//		MCUSoftReset();								//系统程序重新启动
//		OSTimeDly(60);
//	}
}
#endif
