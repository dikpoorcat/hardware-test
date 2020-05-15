#include "Bsp_AM2302.h"

/*
	GPIOX->ODR ---> 端口X输出数据寄存器
	GPIOX->IDR ---> 端口X输入数据寄存器
	GPIOX->DDR ---> 端口X数据方向寄存器     0：输入    1：输出
	GPIOX->CR1 ---> 控制寄存器1
									DDR=0   输入时  CR1=0  浮空输入   CR1=1  带上拉电阻输入
									DDR=1   输入时  CR1=0  模拟开漏输出   CR1=1  推挽输出
	GPIOX->CR2 ---> 控制寄存器2
									DDR=0   输入时  CR2=0  禁止外部中断   CR2=1  使能外部中断
									DDR=1   输入时  CR2=0     CR2=1  推挽输出
*/


//全局变量
AM2302_Data_TypeDef	AM2302_Data;



/******************************************************************************* 
* Function Name  : void MET_LowPower(void)
* Description    : MET进入低功耗，并对相应IO口作低功耗处理。包括AM2302与BMP180。
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/
void MET_LowPower(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	/*电源使能、WSD_DA、QY_DATA、QY_SCK模拟输入*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;							//模拟输入
	
	GPIO_InitStructure.GPIO_Pin = PWMET_PIN;								//电源使能口，关电源
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
* Description:   温湿度电源控制 引脚配置				 
* Input:  Nothing
* Output: Nothing
* Contributor: GuoWei Dong
* Date First Issued: 2010-01-26
******************************************************************************/
void PowerMETPin_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(PWMET_Port_CK,ENABLE);			//开时钟
	PWMETDIS();												//设置为关闭	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =PWMET_PIN;
	GPIO_Init(PWMET_Port, &GPIO_InitStructure);	
}

/**********************************************************************
函数：void Am2302_Init(void)
功能：Am2302传感器初始化
入参：无
出参：无
***********************************************************************/
void Am2302_Init(void)
{
	//数据引脚初始化
	RCC->APB2ENR|=1<<4;  				//PORTC时钟使能 	2~8分别为A~G  
	GPIOC->CRH&=0XFFFFFFF0; 
	GPIOC->CRH|=0X00000007;				//PC8(3推挽，7开漏）输出
	Am2302_DataH();						//初始化为高阻，外部上拉为高电平
	
//数据引脚初始化为高阻，外部上拉为高电平。GPIO_Mode_Out_OD（通用性好）
//	GPIO_InitTypeDef GPIO_InitStructure;
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
//	GPIO_InitStructure.GPIO_Pin = Am2302Pin;
//	GPIO_Init(Am2302Port, &GPIO_InitStructure);
//	GPIO_SetBits(Am2302Port,Am2302Pin);								
}

/**********************************************************************
函数：static void AM2302_Mode_IPU(void) 
功能：使AM2302-DATA引脚变为输入模式
入参：无
出参：无
***********************************************************************/
static void AM2302_Mode_IPU(void)  
{  
	GPIO_InitTypeDef GPIO_InitStructure;  

	/*选择要控制的GPIOD引脚*/    
	GPIO_InitStructure.GPIO_Pin = Am2302Pin;  

	/*设置引脚模式为浮空输入模式*/  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ;   

	/*调用库函数，初始化GPIOD*/  
	GPIO_Init(Am2302Port, &GPIO_InitStructure); 	
}

/**********************************************************************
函数：static INT8U Read_Byte(void)
功能：从AM2302总线读取一个字节
入参：无
出参：返回读取到的字节
***********************************************************************/
static INT8U Read_Byte(void)  
{       
	u8 	i,temp=0;
	u8	Timeout=20;										//设置超时，20*5us未检测到电平变化即退出

	for(i=0;i<8;i++)      
	{  
		/*每bit以50us低电平标置开始，轮询直到从机发出 的50us 低电平 结束*/   
		while( (!AM2302_DATA_IN()) && Timeout )			//检测到低电平且Timeout未超时，进入循环
		{
			delay_1us_32M(5);							//微秒级延时，比delay_SYSus()准确度高。只调用1us的话会稍微不准，因为有出入栈时间，调用一次函数花了0.49us
			Timeout--;
			if(Timeout==0) return 0;					//超时未读取到AM2302响应（总线未拉高，非有效数据）
		}

		/*AM2302 以22~30us的高电平表示“0”，以68~75us高电平表示“1”,通过检测60us后的电平即可区别这两个状态*/  
		delay_1us_32M(50);								//延时50us
		
		Timeout=20;										//重置Timeout
		if( AM2302_DATA_IN() )							//50us后仍为高电平表示数据“1”  
		{  
			/*轮询直到从机发出的剩余的 30us 高电平结束*/  
			while( AM2302_DATA_IN() && Timeout )		//检测到高电平且Timeout未超时，进入循环
			{
				delay_1us_32M(5);						//微秒级延时，比delay_SYSus()准确度高。只调用1us的话会稍微不准，因为有出入栈时间，调用一次函数花了0.49us
				Timeout--;
				if(Timeout==0) return 0;				//超时未读取到AM2302响应
			}
			temp|=(u8)(0x01<<(7-i));					//把第7-i位置1   
		}  
		else  											//50us后为低电平表示数据“0”  
		{                 								//循环从此出去时已是低电平
			temp&=(u8)~(0x01<<(7-i));					//把第7-i位置0  
		}
	}  
	return temp;
}

/***************************************************************************************
名称：INT8U Read_AM2302( AM2302_Data_TypeDef *Data )
功能：从传感器读取温湿度数据，已转换为补码形式，可用INT16S直接运算
	读取5字节需要约5ms，加上唤醒1ms，共需要6ms左右，若采用关中断方式会导致中断延迟6ms，
	若不关中断，要是遇到时间长的中断会导致读取温湿度错误。由于做了失败重试功能，故牺牲
	读取正确率而保证实时性，不关中断而用禁止OS调度的方式。
入参：无
出参：AM2302_Data_TypeDef *Data，存放采集的温湿度数据
返回：成功1，失败0（校验位错误、传感器无响应、采集温湿度超量程时）
****************************************************************************************/
INT8U Read_AM2302( AM2302_Data_TypeDef *Data )
{
	INT8U	Timeout=10;									//设置超时，10*10us未检测到电平变化即退出
	INT16S	Tem=0;										//有符号整数
	
	Am2302_DataL();				
	delay_SYSus(1000);									//延时1ms
	Am2302_DataH();				
	delay_SYSus(30);									//延时30us	按要求30us后即可读取到低电平

	/*主机设为输入 判断从机响应信号*/  
	AM2302_Mode_IPU();									//数据总线初始化为浮空输入

	/*判断从机是否有低电平响应信号 如不响应则跳出，响应则向下运行*/    
	if(AM2302_DATA_IN()==0)   							//检测低电平    
	{ 
		OSSchedLock();									//阻止OS调度，防止打断时序（delay_SYSus函数中已有禁止调度，但Read_Byte函数中的delay_1us_32M函数没有禁止）
		
		/*轮询直到从机发出 的80us 低电平 响应信号结束*/   
		while( (!AM2302_DATA_IN()) && Timeout )			//检测到低电平且Timeout未超时，进入循环
		{
			delay_1us_32M(10);
			Timeout--;
			if(Timeout==0) return 0;					//超时未读取到AM2302响应
		}
		
		Timeout=10;										//重置Timeout
		/*轮询直到从机发出的 80us 高电平 标置信号结束*/  
		while( AM2302_DATA_IN() && Timeout )			//检测到高电平且Timeout未超时，进入循环
		{
			delay_1us_32M(10);
			Timeout--;
			if(Timeout==0) return 0;					//超时未读取到AM2302响应
		}

		/*开始接收数据*/
		Data->humi_H= Read_Byte(); 						//读取湿度高字节
		Data->humi_L= Read_Byte();						//读取湿度低字节
		Data->temp_H= Read_Byte();						//读取温度高字节 
		Data->temp_L= Read_Byte();						//读取温度低字节
		Data->check_sum= Read_Byte();					//读取校验和 
      
		/*读取结束，引脚改为输出模式*/  
		Am2302_Init();									//开漏输出高电平
		
		OSSchedUnlock();								//恢复OS调度
		

		
		/*检查读取的数据是否正确*/  
		if(Data->check_sum != (u8)(Data->humi_H + Data->humi_L + Data->temp_H+ Data->temp_L)) 
			return ERROR; 								//校验和错误
		
		Tem= (Data->humi_H<<8)+Data->humi_L;
		if( Tem > 1000 ) return ERROR; 					//湿度大于100%错误
		
		Tem= (Data->temp_H<<8)+Data->temp_L;
		if( Tem & 0x8000 ) 								//温度小于0，将原码转换成补码
		{
			Tem= ( ~(Tem << 1) ) >> 1;					//除符号位外取反
			Tem |= 0x8000;								//求得反码
			Tem += 1;									//求得补码
			
			Data->temp_H= (Tem&0xFF00)>>8;				//存放温度高字节（补码）
			Data->temp_L= Tem&0x00FF;					//存放温度低字节（补码）
		}
		if( (Tem<-400) || (Tem>800) ) return ERROR;		//温度小于-40度错误、大于80度错误
			
		return SUCCESS; 								//符合要求的温湿度
	}  
	else  
		return ERROR;									//总线未拉低，未读取到AM2302响应
}

/***************************************************************************************
名称：INT8U Read_Avrage_AM2302( AM2302_Data_TypeDef *Data, INT8U Times ) 
功能：多次采样，并计算温湿度数据的平均值，存入*Data（也是补码形式）
入参：INT8U Times，采样次数
出参：AM2302_Data_TypeDef *Data，平均温湿度数据（INT8U  check_sum校验和为0）
返回：成功1，失败0
****************************************************************************************/
INT8U Read_Avrage_AM2302( AM2302_Data_TypeDef *Data, INT8U Times ) 
{
	INT16U	Humi=0;
	INT16S	Temp=0;										//有符号
	INT8U	t=Times;									//保存次数
	
	while(Times--)										//采样Times次
	{
		if( Read_AM2302( Data ) ) 						//成功返回1（采样得到的已转成补码，可直接运算）
		{
			Humi += (Data->humi_H<<8)+Data->humi_L;		//累计湿度
			Temp += (Data->temp_H<<8)+Data->temp_L;		//累计温度（采样得到的已转成补码，可直接运算）
		}
		else return 0;									//某次采集失败时返回0
		if( Times ) OSTimeDly(60);						//等待3秒 AM2302采集间隔要2秒，最后一次不等待
	}
	
	Humi /= t;											//平均湿度
	Temp /= t;											//平均温度
	Data->humi_L = Humi&0x00FF;							//取低字节
	Data->humi_H = (Humi&0xFF00)>>8;					//取高字节
	Data->temp_L = Temp&0x00FF;							//取低字节
	Data->temp_H = (Temp&0xFF00)>>8;					//取高字节
	return 1;											//表示成功
}

/***************************************************************************************
名称：INT8U Read_Median_AM2302( AM2302_Data_TypeDef *Data, INT8U Times ) 
功能：多次采样，并计算温湿度数据的中位数（按温度排序），存入*Data（也是补码形式）
入参：INT8U Times，采样次数，最多10次
出参：AM2302_Data_TypeDef *Data，温湿度数据中位数（INT8U  check_sum校验和为0）
返回：成功1，失败0
****************************************************************************************/
INT8U Read_Median_AM2302( AM2302_Data_TypeDef *Data, INT8U Times )
{
    INT8U	j ,i, LastExchaneIndex;
    INT8U	Temp[5]={0};								//交换时缓存
	AM2302_Data_TypeDef	Data_Temp[10]={0};				//最多10次
	INT8U	Median= Times/2;							//中位数，如：7个时取Data_Temp[3]，8个时取Data_Temp[4]
	
    j = Times - 1;										//需排序轮数
	
	/*温湿度采集*/
	while( Times-- )
	{
		if( !Read_AM2302( &Data_Temp[Times] ) )			//已转换为补码，可直接运算
			return 0;									//某次采集失败时返回0
		if( Times ) OSTimeDly(60);						//等待3秒 AM2302采集间隔要2秒，最后一次不等待
	}
	
	/*按温度排序*/
    while(j>0)
    {
        LastExchaneIndex=0;
        for(i = 0; i < j; i++)
        {
            if( (Data_Temp[i].temp_H<<8)+Data_Temp[i].temp_L > (Data_Temp[i+1].temp_H<<8)+Data_Temp[i+1].temp_L )   //>则大数在后
            {
                memcpy(Temp,&Data_Temp[i],5);       	//将数组复制到缓存
                memcpy(&Data_Temp[i],&Data_Temp[i+1],5);
                memcpy(&Data_Temp[i+1],Temp,5);     	//大数在后
                LastExchaneIndex=i;
            }
        }
        j= LastExchaneIndex;							//指示最后交换的地址，它本身未排好序！
		if( j < Median ) break;							//若中位数已完成排序，跳出
    }
	
	memcpy( Data, &Data_Temp[Median], 5 );				//将中位数存入出参
	return 1;											//表示成功
}
