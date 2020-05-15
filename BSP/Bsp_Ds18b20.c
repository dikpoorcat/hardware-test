#include "Bsp_DS18B20.h"



/******************************************************************************* 
* Function Name  : void DS18B20_LowPower(void)
* Description    : DS18B20电路低功耗。
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/ 
void DS18B20_LowPower(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	/*电源使能、DQ(WSD_DA)引脚拟输入*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;							//模拟输入
	
	GPIO_InitStructure.GPIO_Pin = DS18B20_PW_PIN;							//电源使能口，关电源 PA11
	GPIO_Init(DS18B20_PW_PORT, &GPIO_InitStructure);						//

	GPIO_InitStructure.GPIO_Pin = DS18B20_IO_PIN;							//DQ(WSD_DA) PC8
	GPIO_Init(DS18B20_IO_PORT, &GPIO_InitStructure);						//
}

/*===========================================================================
函数：void delay_SYSus(u32 nus)
说明：us延时，利用systick一直在递减，不停读取值比较，当比较值超过入参时则退出              
     需要注意防止OS调度打断延时
	 
	 systick频率太低导致us级别无法做到很准
	 delay_SYS_2us（15）=60us， 10=40us, 1=30us,
入参：
出参：
============================================================================*/
void delay_SYS_2us(u32 n2us)
{		
	u32 ticks = 0;
	u32 told = 0;
	u32 tnow = 0;
	u32 tcnt = 0;
	u32 reload = SysTick->LOAD;					//LOAD的值	    	 
	
	ticks=n2us; 						      	  //需要的节拍数HLCK=32M	  systick=HCLK/8=4M，每us需要的节拍数为4		 //HCLK=4M systick=500K,最小单位为2us，
	tcnt=0;
	OSSchedLock();						       //阻止OS调度，防止打断us延时
	told=SysTick->VAL;        			//刚进入时的计数器值
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;		//如果未递减到0
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;				//时间超过或等于要延迟的时间,则退出.
		}  
	};
	OSSchedUnlock();						//恢复OS调度									    
}




/*===========================================================================
函数：void DS18B20_RST(void)
说明：复位DS18B20，等待应答
入参：
出参：
============================================================================*/
void DS18B20_RST(void)
{
	DS18B20_IO_OUT();
	DS18B20_L();
	DS_delay600us;               
	DS18B20_H();                     //输出低>480us后释放总线
	DS_delay40us ;                    //延迟15-60us等待应答
}

/*===========================================================================
函数：u8 DS18B20_CHECK(void)
说明：DS18B20复位应答
入参：
出参：0:DS18B20 READY
============================================================================*/
u8 DS18B20_CHECK(void)
{
	int i = 0;
	
	DS18B20_IO_IN();         // 输入模式
	while(DS18B20_IN&&(i<100))  // 200us内等待DS18B20拉低电平
	{
	  i++;
	  DS_delay2us ;	//2us
	}
	if(i>=100)
	{ 
		return 1; 
	}
    
	i=0;                
	while(!DS18B20_IN&&(i<120))  //低电平持续不超过240us认定有效
	{
	  i++;
	 DS_delay2us ;	//2us
	}
	
	DS_delay600us ;	//复位应答slot>480us
	
	if(i>=120)
	{
		return 1;
	}
	
	return 0;
}

/*===========================================================================
函数：u8 DS18B20_Init(void)  
说明：初始化引脚,确认DS18B20能够应答
入参：
出参：0:DS18B20 READY
============================================================================*/
u8 DS18B20_Init(void)                                              
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(DS18B20_PW_Port_CK|DS18B20_IO_Port_CK,ENABLE);
 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =DS18B20_PW_PIN;
	GPIO_Init(DS18B20_PW_PORT, &GPIO_InitStructure);           //DS18B20VCC 推挽输出
	
	GPIO_InitStructure.GPIO_Pin =DS18B20_IO_PIN;
	GPIO_Init(DS18B20_IO_PORT, &GPIO_InitStructure);	         //DS18B20 DQ 推挽输出
	
	
	DS18B20_PW_ON();                                           
	DS_delay10us ;	                                              //等待电源稳定？？有无必要？


	DS18B20_H();                                               //拉高
                    	
	DS18B20_RST();
		
	return DS18B20_CHECK();
}

/*===========================================================================
函数：void DS18B20_Wakeup(void)  
说明：唤醒DS18B20，上电后需进行复位
入参：
出参：
============================================================================*/

void DS18B20_Wakeup(void)  
{
    DS18B20_PW_ON();                                           
	DS_delay10us ;                                            //等待电源稳定？？有无必要？


	DS18B20_H();                                               //拉高
                    	
	DS18B20_RST();
		
	DS18B20_CHECK(); 
}
/*===========================================================================
函数：void DS18B20_Sleep(void)  
说明：降低功耗，关闭电源与DQ口
入参：
出参：
============================================================================*/

void DS18B20_Sleep(void)  
{
   DS18B20_PW_OFF();
   DS18B20_L(); 
}

/*===========================================================================
函数：void DS18B20_Write_Byte(u8 dat)
说明：向DS18B20写入一个字节，用于指令输入
入参：u8
出参：
============================================================================*/
void DS18B20_Write_Byte(u8 dat)
{
	int temp,i=0;
    DS18B20_IO_OUT();    //输出模式
	for(i=0;i<8;i++)
	{
			temp=dat&0x01;
			dat=dat>>1;
			if(temp)          //写1    a minimum of 60μs
			{
				DS18B20_L();                     //拉低启动写
				DS_delay2us ;     //2us
				DS18B20_H();  		
				DS_delay60us ;            //60us
			}
			else              //写0
			{
				DS18B20_L();                    //拉低启动写
				DS_delay60us ;               //60us
				DS18B20_H();  		
				DS_delay2us ;	     //2us
			}
  }
}

/*===========================================================================
函数：u8 DS18B20_Read_Bit(void)
说明：读DS18B20一位
入参：
出参：1位
============================================================================*/
u8 DS18B20_Read_Bit(void)
{
	int temp;
    DS18B20_IO_OUT();     //输出模式
	DS18B20_L();          //拉低启动读
	DS_delay2us ;	      //2us
	DS18B20_H();          //释放总线
	DS18B20_IO_IN();      //输入模式
	DS_delay10us ;     //在15us之内采样     2+10=12us
	if(DS18B20_IN)
		temp=1;
	else temp=0;

	DS_delay60us ;         //a minimum of 60μs

	DS18B20_IO_OUT();
	DS18B20_H();          //释放总线
	
	return temp;
}

/*===========================================================================
函数：u8 DS18B20_Read_Byte(void)
说明：读DS18B20一个字节
入参：
出参：1个字节
============================================================================*/
u8 DS18B20_Read_Byte(void)
{
    int i,dat,temp=0;
	for(i=0;i<8;i++)
	{
		temp=DS18B20_Read_Bit();
		dat=(temp<<7)|(dat>>1);
//		dat=dat>>1;
//	    temp=DS18B20_Read_Bit();
//		if(temp)dat|=0x80;	
	}
	return dat;
}
/*===========================================================================
函数：u8 DS18B20_Read_ROM(void)
说明：读ROM的LSB，测试写是否成功
入参：
出参：1个字节
============================================================================*/
u8 DS18B20_Read_ROM(void)
{ 
    u8 temp;
	DS18B20_RST();
	DS18B20_CHECK();
	DS18B20_Write_Byte(0x33);
	temp=DS18B20_Read_Byte();
	return temp;
}


/*===========================================================================
函数：u8 DS18B20_Get_Temperature(u16 *TempBuff)
说明：读DS18B20两个字节，分别为TL与TH，TH高5位为标志位
入参：
出参：返回1表示温度转化成功,TempBuff[0]储存转化后的温度
============================================================================*/
u8 DS18B20_Get_Temperature(u16 *TempBuff)
{
    u16 TL,TH,temp=0;
	u8 over=0;
	

	if( DS18B20_Init()) return 0;        //DS18B20 not ready
  
	DS18B20_Write_Byte(0xcc);             //skip rom
	DS18B20_Write_Byte(0x44);             //convert  start
    OSTimeDly(15); 	                      //延时750ms等待转换完成             
	if(DS18B20_Read_Bit()) over=1;        //转化完成
	
	DS18B20_RST();	
    if(DS18B20_CHECK()) return 0;         //DS18B20 not ready
	DS18B20_Write_Byte(0xcc);             //skip rom
	DS18B20_Write_Byte(0xbe);             //read   
    TL = DS18B20_Read_Byte();              //LSB
	TH = DS18B20_Read_Byte();              //MSB

	
	if(TH>7)                              //负温
	{
		temp=~((TH<<8)+TL)+1;            //DS18B20以补码形式输出负温
		temp=(temp*5)>>3;                //*0.625=5/8  等于温度的10倍  精度0.0625°C
		if(temp>550) temp = 550;
		// TempBuff[0]=temp|=0xf800;
		temp|=0xf800;
		TempBuff[0]=temp;
	}
	else 
	{
		temp=((TH<<8)+TL);                 //正温
		temp=(temp*5)>>3;                  //*0.625=5/8  等于温度的10倍  精度0.0625°C 
		if(temp>1250)temp=1250;
		TempBuff[0]=temp;
	}
	
	
    DS18B20_Sleep(); 
	return over;
}

/*************************************************************
函数：u8 Get_DS18B20Temp(u16 *temp)
说明：对温度3次采样，按大小顺序排列，取中间值
入参：Buff  采集的温度值
出参：正确返回1 错误返回0
**********************************************************************/
u8 Get_DS18B20Temp(u16 *temp)
{
	u16 DStemp,TEMP;
	u16 DSbuff[3]={0};
	u8 count,i,j,k=0;
	u8 over=0;

	DS18B20_Init();
//  ROM_lsb=DS18B20_Read_ROM();            		//测试发送命令能否正常读回数据
	for(count=0;count<3;count++)             	//尝试3次
	{
		for(i=0;i<3;i++)
		{
			over = DS18B20_Get_Temperature(&DStemp);
			if(!over) break;                    //若读取温度失败
			else DSbuff[i]=DStemp;
		}
		
		/*若采集成功*/
		if(over)
		{
			for(j=0;j<2;j++)
			{
				for(k=j+1;k<3;k++)
				{
					if(DSbuff[j]<DSbuff[k])
					{
						TEMP=DSbuff[j];
						DSbuff[j]=DSbuff[k];
						DSbuff[k]=TEMP;
					}
				}	 
			}
			temp[0]=DSbuff[1];                	//取中间值
		}
	}
	DS18B20_Sleep(); 
	return over;
}


/*************************************************************
函数：float TempU16toFloat(INT16U *pIn)
说明：DS18B20得到的温度数据转浮点型
入参：U16数据
出参：返回浮点型温度
**********************************************************************/
float TempU16toFloat(INT16U In)
{
	float Temp=0;
	
	if(In>=0xf800)  					// 高5位为符号位,都有值说明为负温，若=0xf800的时候则为-0，要当成负数来处理将F8去除掉，其实就是个0度
	{
		In = (In&0x07ff);        		// 去掉符号位，即绝对值   
		Temp = (float )In;
		Temp = -Temp;	
		Temp = Temp /10.0 ;      		// 1个小数位		
	}
	else{
		Temp = (float )In;
		Temp =  In /10.0 ;           	// 1个小数位
	}

return Temp;							//返回浮点数
}

/************************************************************************************************************************
* Function Name : INT16U FloatToTempU16(float pIn)                                                                     
* Description   : 将浮点数，按我司的规则，转换成INT16U类型输出
* Input         : None                                                
* Return        : None                        
*
* History :                               
* First Issued  : 赵志舜于2018年11月26日创建本函数             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
INT16U FloatToTempU16(float In)
{
	INT16U Temp = 0;
	
	if(In<0)
	{
		In=-In;
		In*=10;
		Temp=(INT16U)In;
		Temp+=0xF800;
	}else{
	    In*=10;
		Temp=(INT16U)In;
	}
return Temp;					 //整形数
}

