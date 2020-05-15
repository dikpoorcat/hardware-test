/*****************************************Copyright(C)******************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_RTC.c
**创    建    人: andydriver
**创  建  日  期: 081210
**最  新  版  本: V1.0
**描          述: RS8025驱动
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 杜颖成
**日          期: 2019.03.06
**版          本: V1.0
**描          述: 整理修订，分离掉国电规约部分函数。
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
#include "bsp_RTC.h"



/*全局变量*/
struct BSPRTC_TIME gRtcTime;
struct BSPRTC_TIME gSetTime;




/* --------------------Private functions begin-------------------------------------------------------------------------*/
#if 1
/*******************************************************************************
* Function Name: static void iic_delay(INT32U time)                                        
* Description:   本IIC模块延时函数
* Input:         time：延时值，1 or 2 instruction clock,32M的话，Tclk=31.25ns
* Return:        None
*******************************************************************************/
static void iic_delay(INT32U time)
{
	while(time--);
}

/*******************************************************************************
* Function Name: static void iic_clk_high(void)                                         
* Description:   IIC_CLK输出高电平
* Input:         None
* Return:        None
*******************************************************************************/
static void iic_clk_high(void)
{
	GPIO_WriteBit(SIIC_GPIO,SIIC_GPIO_SCL,Bit_SET);   
	iic_delay(IIC_DELAY_TIME);
}

/*******************************************************************************
* Function Name: static void iic_clk_low(void)                                          
* Description:   IIC_CLK输出低电平
* Input:         None
* Return:        None
*******************************************************************************/
static void iic_clk_low(void)
{
	GPIO_WriteBit(SIIC_GPIO,SIIC_GPIO_SCL,Bit_RESET);  
	iic_delay(IIC_DELAY_TIME);
}

/*******************************************************************************
* Function Name: static void iic_data_set_in(void)                                                  
* Description:   配置IIC_SDA引脚为输入模式
* Input:         None
* Return:        None
*******************************************************************************/
static void iic_data_set_in(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	// 配置SDA
	GPIO_InitStructure.GPIO_Pin = SIIC_GPIO_SDA;
 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;      					//浮空输入
	
	GPIO_Init(SIIC_GPIO, &GPIO_InitStructure);	
}
																				
/*******************************************************************************
* Function Name: static void iic_data_set_out(void)                                           
* Description:   配置IIC_SDA引脚为输出模式
* Input:         None
* Return:        None
*******************************************************************************/
static void iic_data_set_out(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	// 配置SDA
	GPIO_InitStructure.GPIO_Pin = SIIC_GPIO_SDA;
 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       						//推挽输出
	GPIO_Init(SIIC_GPIO, &GPIO_InitStructure);	
}

/*******************************************************************************
* Function Name: static void iic_data_high(void)                                        
* Description:   IIC_SDA引脚输出高电平(数据1)
* Input:         None
* Return:        None
*******************************************************************************/
static void iic_data_high(void)
{
	GPIO_WriteBit(SIIC_GPIO,SIIC_GPIO_SDA,Bit_SET);  
	iic_delay(IIC_DELAY_TIME);
}

/*******************************************************************************
* Function Name: static void iic_data_low(void)                                          
* Description:   IIC_SDA引脚输出低电平(数据0)
* Input:         None
* Return:        None
*******************************************************************************/
static void iic_data_low(void)
{
	GPIO_WriteBit(SIIC_GPIO,SIIC_GPIO_SDA,Bit_RESET);  
	iic_delay(IIC_DELAY_TIME);
}

/*******************************************************************************
* Function Name: static INT8U iic_data_read(void)                                       
* Description:   本IIC模块延时函数
* Input:         None
* Return:        I/O口上的一个位的当前读入电平状态，1：高  0：低
*******************************************************************************/
static INT8U iic_data_read(void)
{
	iic_delay(IIC_DELAY_TIME);
	return GPIO_ReadInputDataBit(SIIC_GPIO,SIIC_GPIO_SDA);
}

/*******************************************************************************
* Function Name: void iic_init(void)                                    
* Description:   外部RTC(Rx8025)芯片接口I2C初始化，用单片机的IO口模拟的I2C
* Input:         None
* Return:        None
*******************************************************************************/
void iic_init(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	// 配置SCL为输出脚	  
	RCC_APB2PeriphClockCmd(RCCRTCEN ,ENABLE);
	GPIO_InitStructure.GPIO_Pin = SIIC_GPIO_SCL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SIIC_GPIO, &GPIO_InitStructure);
	
	iic_data_high();
	iic_data_set_out();
	iic_clk_high();
}

/*******************************************************************************
* Function Name: void iic_start(void)                                   
* Description:   I2C开始
* Input:         None
* Return:        None
*******************************************************************************/
void iic_start(void)
{ 
	iic_delay(IIC_DELAY_TIME_LONG);
	iic_delay(IIC_DELAY_TIME_LONG);
	iic_delay(IIC_DELAY_TIME_LONG);
	iic_delay(IIC_DELAY_TIME_LONG);
	iic_data_high();
	iic_data_set_out();
	iic_clk_high();
	iic_data_low();
	iic_clk_low();
}

/*******************************************************************************
* Function Name: void iic_start(void)                                   
* Description:   I2C停止
* Input:         None
* Return:        None
*******************************************************************************/
void iic_stop(void)
{
	iic_clk_low();
	iic_data_low();
	iic_data_set_out();
	iic_clk_high();
	iic_data_high();
}

/*******************************************************************************
* Function Name: void iic_ack(void)                               
* Description:    应答:数据接收成功
* Input:         None
* Return:        None
*******************************************************************************/
void iic_ack(void)
{
	iic_data_low();
	iic_data_set_out();
	iic_clk_high();
}

/*******************************************************************************
* Function Name: void iic_noack(void)                          
* Description:   应答:数据接收不成功
* Input:         None
* Return:        None
*******************************************************************************/
void iic_noack(void)
{
	iic_data_high();
	iic_data_set_out();
	iic_clk_high();
}

/*******************************************************************************
* Function Name: INT8U iic_send_byte(INT8U val)                                        
* Description:   I/O口模拟的IIC发送一个字节的数据
* Input:         val:要发送的值（1Byte）
* Return:        None
*******************************************************************************/
INT8U iic_send_byte(INT8U val)
{
	INT8U i=0;
	
	iic_clk_low();           // 先拉低时钟
	iic_data_set_out();      // 设置SDA脚为输出模式
	for(i=0;i<8;i++)
	{
		if(val&0x80)
		{
			iic_data_high();
		}
		else
		{
			iic_data_low();
		}
		iic_clk_high();
		                  //zzs note??? CNM，信号都还没建立起来就又置低，这样也行？？？
		iic_clk_low();
		val = val<<1;
	}
	
	/*等待应答*/
	iic_data_set_in();
	iic_data_read();  //zzs??? read了干嘛呢？也不用个变量接收一下？
	iic_clk_high();
	i=0;
	while(iic_data_read())
	{
		if(++i > 12)
		{
			iic_clk_low();
			return 0;
		}
	}
	iic_clk_low();
	return 1;
}

/*******************************************************************************
* Function Name: INT8U iic_rec_byte(void)                                 
* Description:   I/O口模拟的IIC接收一个字节的数据
* Input:         None
* Return:        val: 接收到的值(1Byte)
*******************************************************************************/
INT8U iic_rec_byte(void)
{
	INT8U val=0;
	INT8U i=0;
	
	val = 0;
	iic_clk_low();
	iic_data_set_in();														// SDA输入,必须要
	for(i=0;i<8;i++)
	{
		iic_delay(IIC_DELAY_TIME_LONG);
		iic_clk_high();
		val = val<<1;
		val |= iic_data_read();
		iic_clk_low();
	}
	return val;
}

/*******************************************************************************
* Function Name: static void RX8025Write(INT8U addr,INT8U *pData,INT8U len)                                 
* Description:   I/O口模拟的IIC，向RX8025指定地址写入一串数据
* Input:         addr  :  RX8025内部寄存器的地址
                 pData : 主调函数中指定的指向要被本函数发送的数据串的首地址的指针
                 len   : 发送的数据串长度 
*
* Return:        None
*******************************************************************************/
static void RX8025Write(INT8U addr,INT8U *pData,INT8U len)
{
	INT8U i=0;
	
	iic_start();
	if(iic_send_byte(RX8025_ADDR_WRITE)==0)
	{
		iic_stop();
		return;
	}
	if(iic_send_byte(addr)==0)
	{
		iic_stop();
		return;
	}
	for(i=0;i<len;i++)
	{
		if(iic_send_byte(pData[i])==0)
		{
			iic_stop();
			return;
		}
	}
	iic_stop();
}

/*******************************************************************************
* Function Name: static void RX8025Read(INT8U addr,INT8U *pData,INT8U len)                        
* Description:   I/O口模拟的IIC，从RX8025指定地址读取一串数据（读出来应该就是8421BCD码 ZE）
* Input:         addr  :  RX8025内部寄存器的地址
                 pData : 主调函数中指定的用于接收来自RX8025的数据 的接收空间的首地址指针
                 len   : 接收的数据串长度 
*
*
* Return:        通过形参pData返回
*******************************************************************************/
static void RX8025Read(INT8U addr,INT8U *pData,INT8U len)
{
	INT8U i=0;
	
	iic_start();
	if(iic_send_byte(RX8025_ADDR_WRITE)==0)
	{
		iic_stop();
		return;
	}
	
	if(iic_send_byte(addr)==0)
	{
		iic_stop();
		return;
	}
	
	iic_start();
	if(iic_send_byte(RX8025_ADDR_READ)==0)
	{
		iic_stop();
		return;
	}
	
	for(i=0;i<len-1;i++)
	{
		pData[i] = iic_rec_byte();
		iic_ack();
	}
	pData[i] = iic_rec_byte();
	iic_noack();
	iic_stop();
}

/*******************************************************************************
* Function Name: void BSP_RX8025Write(INT8U *pData,INT8U len)                                     
* Description:   向时钟芯片RX8025写入数据
* Input:         pData : 要写入的数据
                 len   : 写入长度
* Return:        None
*******************************************************************************/
void BSP_RX8025Write(INT8U *pData,INT8U len)
{
	RX8025Write((RX8025_ADDR_SECONDS&RX8025_WRITE_MODE),pData,len);
}

/*******************************************************************************
* Function Name: void BSP_RX8025Read(INT8U *pData,INT8U len)                              
* Description:   从时钟芯片RX8025读取数据
* Input:         pData : 读出数据的存放地点
                 len   : 读取长度
* Return:        None
*******************************************************************************/
void BSP_RX8025Read(INT8U *pData,INT8U len)
{
	RX8025Read((RX8025_ADDR_SECONDS&RX8025_READ_MODE),pData,len);
}

#if 0
/*******************************************************************************
* Function Name: void BSP_RX8025ControlINTA(_BSPRX8025_INTAOUT state)                            
* Description:   控制芯片的INTA脚状态
* Input:         state:状态(_BSPRX8025_INTAOUT)
								 BSPRX8025_INTAOUT_HIZ:高阻
								 BSPRX8025_INTAOUT_LOW:输出低
								 BSPRX8025_INTAOUT_2HZ:输出2Hz(50%)的脉冲
								 BSPRX8025_INTAOUT_1HZ:输出1Hz(50%)的脉冲
								 BSPRX8025_INTAOUT_SEC:每秒的第0秒翻转?还是脉冲?
								 BSPRX8025_INTAOUT_MIN:每分的第0秒翻转?还是脉冲?
								 BSPRX8025_INTAOUT_HOUR:每时的第0秒翻转?还是脉冲?
								 BSPRX8025_INTAOUT_MONTH:每月的第0秒翻转?还是脉冲?
* Return:        None
*
* Author:                               
* Date First Issued: 赵志舜于2018年1月18日创建本函数             E-Mail:11207656@qq.com
* Version:  V1.0
*******************************************************************************/
void BSP_RX8025ControlINTA(_BSPRX8025_INTAOUT state)
{
	union RX8025_REG_CONTROL1	data={0};
	
	RX8025Read((RX8025_ADDR_CONTROL1&RX8025_READ_MODE),(INT8U *)(&data),1);
	data.bits.CT = state;							// INTA输出1Hz方波
	RX8025Write((RX8025_ADDR_CONTROL1&RX8025_WRITE_MODE),(INT8U *)(&data),1);
}
#endif

/*******************************************************************************
* Function Name: void BSP_RX8025Init(void)                                
* Description:   时钟芯片RX8025的初始化
* Input:         None
* Return:        None
*******************************************************************************/
void BSP_RX8025Init(void)
{
	INT8U buf[16] = {0};
	
	OSSchedLock();    															
	
	iic_init();
	RX8025Read((RX8025_ADDR_SECONDS&RX8025_READ_MODE),buf,8);
	buf[0] = 0x20;
	RX8025Write((RX8025_ADDR_CONTROL1&RX8025_WRITE_MODE),buf,1);				//24小时制
	RX8025Read((RX8025_ADDR_SECONDS&RX8025_READ_MODE),buf,16);
	iic_delay(50000);
	
	OSSchedUnlock();
}

#endif




/*===========================================================以上为RX8025底层操作函数===========================================================*/


/*===========================================================以下为RTC通用API接口函数===========================================================*/






/*******************************************************************************
* Function Name: static void BSP_RTCWrite(const struct BSPRTC_TIME *pTime)                          
* Description:   向RTC写入结构体中的时间
* Input:         pTime : 主调指定的，指向RTC时间结构体的首地址 {秒；分；时；周；日；月；年}，一般是校对时钟时的标准时间副本。
* Return:        None
*******************************************************************************/ 
static void BSP_RTCWrite(const struct BSPRTC_TIME *pTime)
{
	OSSchedLock();
	BSP_RX8025Write((INT8U *)pTime,7);									
	OSSchedUnlock();
}

/*******************************************************************************
名称：static void BSP_RTCRead(struct BSPRTC_TIME *pTime)
功能：读取RTC时间的API。
入参：无
出参：struct BSPRTC_TIME *pTime，RTC时间结构体的首地址 {秒；分；时；周；日；月；年}，其中年是
减去2000的，如0x19（8421BCD）代表2019年
返回：无
*******************************************************************************/
static void BSP_RTCRead(struct BSPRTC_TIME *pTime)
{
	OSSchedLock();
	BSP_RX8025Read((INT8U *)pTime,7);									
	OSSchedUnlock();
}

/*******************************************************************************
* Function Name: INT8U RtcSetChinaStdTimeStruct(const struct BSPRTC_TIME *pTime)                           
* Description:   对时钟芯片RX8025设置RTC时间（UTC +8 中国标准时间 （CST）），本函数添加了写入数据合法性校验过程，以及失败多尝试写入2次的处理。
                 写入之后立即读取出来，检验合法性，以及对比接入前后的内容是否一致。
* Input:         pTime : CST时间结构体的首地址 {秒；分；时；周；日；月；年}，一般是校对时钟时的标准时间副本。
* Return: 		 1：成功   0：失败
*******************************************************************************/
INT8U RtcSetChinaStdTimeStruct(const struct BSPRTC_TIME *pTime)
{   
	INT8U i=3;
	struct BSPRTC_TIME time={0};
	
	if(RtcCheckTimeStruct(pTime)==0) return 0;									//写入前数据合法性查验
	while(i--)
	{
		BSP_RTCWrite(pTime);    												//将时间写入RTC芯片
		if(RtcGetChinaStdTimeStruct(&time)==1) return 1;  						//读回并校验合法性，若通过
		BSP_RX8025Init();														//失败则重新初始化
	}
	return 0;
}

/*******************************************************************************
* Function Name: INT8U RtcGetChinaStdTimeStruct(struct BSPRTC_TIME *pTime)                              
* Description:   从时钟芯片RX8025取得RTC时间（UTC +8 中国标准时间 （CST）），本函数添加了数据合法性校验过程，以及失败多尝试读取2次的处理。
* Input:         pTime : 主调指定的，用于存放从时钟芯片RX8025读取到的RTC时间的，RTC时间结构体的首地址 {秒；分；时；周；日；月；年}
                         主调准备好这个空间。
* Return:       1：标识取得合理正确的时间，具体内容由形参pTime接收返回
                0：读取RTC时间失败
*******************************************************************************/
INT8U RtcGetChinaStdTimeStruct(struct BSPRTC_TIME *pTime)
{
	INT8U 			i=3;
	
	while(i--)																	//最多重试3次
	{
		BSP_RTCRead(pTime);														//调用读取RTC时间的API
		if(RtcCheckTimeStruct(pTime)==1) return 1;								//数据合法
		BSP_RX8025Init();														//失败则重新初始化
	}
	return 0;
}

/*******************************************************************************
* Function Name:  INT8U RtcSetTimeSecond(INT32U time)                   
* Description  :  世纪秒转换，需要调整时区。向RTC写入世纪秒，RTC芯片API接口函数。传入世纪秒，转换成中国标准时间（CST）8421BCD后写入RTC芯片
* Input        :  time : 世纪秒
*
* Return       :  1 :标识写RTC成功      
*******************************************************************************/
INT8U RtcSetTimeSecond(const INT32U time)
{
	struct tm 			*TTM = 0;												//编译器会自动识别为空指针
	struct BSPRTC_TIME 	TM = {0}; 
	INT32U time_e8 = time+8*3600;												//世纪秒是0区时间，转换为东八区 ( UTC +8 )
	
	TTM =localtime(&time_e8);													//世纪秒转换为本地时间（没有经过时区变换）
	TM.Year   = HexToBCD(TTM->tm_year-100);										//从1900 开始计算
	TM.Month  = HexToBCD(TTM->tm_mon+1);										//月份加上1(localtime的计算结果由0开始)
	TM.Day    = HexToBCD(TTM->tm_mday);
	TM.Hour   = HexToBCD(TTM->tm_hour);
	TM.Minute = HexToBCD(TTM->tm_min);
	TM.Second = HexToBCD(TTM->tm_sec);
	TM.Week   =	HexToBCD(TTM->tm_wday);											//周
	
	return RtcSetChinaStdTimeStruct(&TM);										// 1：成功   0：失败
}

/*******************************************************************************
名称：INT32U RtcGetTimeSecond(void)
功能：从RTC读取中国标准时间（CST）记录的时间，转换为从1970 1.1.0时开始计算的秒数（世纪秒、时间戳）返回。
入参：无
出参：无
返回：读取成功返回世纪秒，读取失败返回0
*******************************************************************************/
INT32U RtcGetTimeSecond(void)
{
	time_t time = 0;															//typedef unsigned int time_t;     /* date/time in unix secs past 1-Jan-70 */
	struct tm TTM = {0};      								

	RtcGetChinaStdTimeStruct(&gRtcTime);										//从时钟芯片RX8025取得RTC时间
	TTM.tm_year = BcdToHex(gRtcTime.Year)+100;  								// 年
	TTM.tm_mon  = BcdToHex(gRtcTime.Month)-1;   								// 月
	TTM.tm_mday = BcdToHex(gRtcTime.Day);       								// 日
	TTM.tm_hour = BcdToHex(gRtcTime.Hour);      								// 时
	TTM.tm_min  = BcdToHex(gRtcTime.Minute);    								// 分
	TTM.tm_sec  = BcdToHex(gRtcTime.Second);    								// 秒
	time = mktime(&TTM)-8*3600;                  								//东八区 ( UTC +8 )时间转换成世纪秒
	
	if (time==0xffffffff) return 0;												//异常
	return time;
}

/*******************************************************************************
* Function Name: INT8U RtcCheckTimeStruct(struct BSPRTC_TIME *pTime)                         
* Description:   检查数据合理性
* Input:         pTime : 主调指定的,指向RTC时间结构体的首地址 {秒；分；时；周；日；月；年}
                         
* Return:        0: RTC时间有不合理的数据    
                 1：RTC时间合理性查验正确返回
*******************************************************************************/
INT8U RtcCheckTimeStruct(const struct BSPRTC_TIME *pTime)
{ 
	if((pTime->Year > 0x99) || ((pTime->Year&0x0f) > 0x09))   					//超过2099年 或 &0x0f出现大于9的数，则异常（8421BCD不会大于9）
		return 0;
	
	if(((pTime->Month) == 0) || ((pTime->Month) > 0x12) || ((pTime->Month&0x0f) > 0x09))	//月份异常
		return 0;
	
	if((pTime->Day == 0) || (pTime->Day > 0x31) || ((pTime->Day&0x0f) > 0x09))	//日异常
		return 0;
	
	if(pTime->Week>6) return 0;             									//周日~周六：0~6
		
	if((pTime->Hour > 0x23) || ((pTime->Hour&0x0f) > 0x09))						//时异常
		return 0;
	
	if((pTime->Minute > 0x59) || ((pTime->Minute&0x0f) > 0x09))					//分异常
		return 0;
	
	if((pTime->Second > 0x59) || ((pTime->Second&0x0f) > 0x09))					//秒异常
		return 0;
	
	return 1;
}

/*******************************************************************************
名称：INT8U GetSysTime(INT8U *pOutBuff)
功能：取得系统时间:取得时间以年月日时分秒周的顺寻填入一个所需的结构体，底层读出的顺序为年月日周时分秒
入参：无
出参：pOutBuff : 用来接收返回数据的指针
返回：固定为1
*******************************************************************************/
INT8U GetSysTime(INT8U *pOutBuff)
{
	RtcGetChinaStdTimeStruct(&gRtcTime);										//从时钟芯片取得RTC时间
	
	pOutBuff[0] = gRtcTime.Year;												// 年
	pOutBuff[1] = gRtcTime.Month;												// 月
	pOutBuff[2] = gRtcTime.Day;													// 日
	pOutBuff[3] = gRtcTime.Hour;												// 时
	pOutBuff[4] = gRtcTime.Minute;												// 分
	pOutBuff[5] = gRtcTime.Second;												// 秒
	pOutBuff[6] = gRtcTime.Week; 												// 周	========注意它的位置变了，原结构体中时是第4字节
	
	return 1;
}

/*******************************************************************************
名称：INT8U BcdToHex(INT8U InData)
功能：BCD码到Hex的转换
入参：INT8U InData,要转换的数据(BCD格式 1Byte)
出参：无
返回：转换后的数据(Hex格式 1Byte)
*******************************************************************************/
INT8U BcdToHex(INT8U InData)
{
	INT8U Bits = 0;
	Bits = InData&0x0F;															//取BCD码个位（低4位）
	InData = InData&0xF0;														//取BCD码十位（高4位）
	InData >>= 4;        														//将十位移入个位
	InData *= 10;       														//计算出十位的值
	InData += Bits;																//加回个位（8421BCD码，直接加就行了）
	return InData;
}
																				
/*******************************************************************************
名称：INT8U HexToBCD(INT8U InData)
功能：Hex到BCD码的转换
入参：InData: 要转换的数据(Hex格式 1Byte)
出参：无
返回：转换后的数据(BCD格式 1Byte)，若传入超过99则返回0xff，表示错误
*******************************************************************************/
INT8U HexToBCD(INT8U InData)
{
	INT8U Temp = 0;
	if (InData>99) return 0xff;													//返回异常
	Temp = InData%10;           												//个位
	InData = InData-Temp;
	InData /= 10;             													//十位
	Temp += ((InData<<4)&0xf0);
	return Temp;
}

/******************************************************************************* 
* Function Name  : void RTC_LowPower(void)
* Description    : RTC进入低功耗，并对相应IO口作低功耗处理
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/
void RTC_LowPower(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = SIIC_GPIO_SCL|SIIC_GPIO_SDA;					//PA0、PA1
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;								//模拟输入
	GPIO_Init(SIIC_GPIO, &GPIO_InitStructure);									//PA口
}

/******************************************************************************* 
* Function Name  : INT8U RTCTaskTest(void)
* Description    : RTC硬件测试函数
* Input          : None 
* Output         : None 
* Return         : 成功返回1，失败返回0
*******************************************************************************/
INT8U RTCTaskTest(void)
{
	INT8U				times = 5;
	TCHAR				temp[100] = {0};
	
	BSP_RX8025Init();
	if(!RtcSetTimeSecond(1602331994)) return 0;									//2020.10.10 20:13:14
	while(times--)
	{
		if(!RtcGetChinaStdTimeStruct(&gRtcTime)) return 0;						//从时钟芯片取得RTC时间
		sprintf(temp+strlen(temp), "本地时间：20%X年%X月%X日 %02X:%02X:%02X\r\n",gRtcTime.Year,gRtcTime.Month,gRtcTime.Day,gRtcTime.Hour,gRtcTime.Minute,gRtcTime.Second);
		BspUartWrite(2,(INT8U*)temp,strlen(temp));
		OSTimeDly(20);
	}
	return 1;
}

/************************(C)COPYRIGHT 2018 方诚电力*****END OF FILE****************************/
