/******************************************************************************
文件：BSP_BMP180.C  
功能：大气压传感器接口通讯
硬件接口：IIC
         BMP180支持最高3.4M的传输速度
********************************************************************************/


#include "Bsp_Bmp180.h"
#include <math.h> //pow 函数 引用



//short ac1,ac2,ac3,b1,b2,mb,mc,md;
//unsigned short ac4,ac5,ac6;


/******************************************************
函数名：void BMP180Pin_CFG(void)
说明：  BMP180 IIC引脚初始化
******************************************************/
void BMP180Pin_CFG(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE)	;
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =Bmp180_SDAPin|Bmp180_SCLKPin;
	GPIO_Init(Bmp180_SDAPort, &GPIO_InitStructure);

	Bmp180_SCLK_H()  ;
	Bmp180_SDA_H() ;


}
/******************************************************
函数名：void BMP180StartSignal(void)
说明：  初始化IIC总线，准备读写数据
******************************************************/
static void BMP180StartSignal(void)
{     
	Bmp180_OUT();       //输出模式
	
	
	Bmp180_SDA_H() ; 
	Bmp180_SCLK_H() ;	
	delay_1us_32M(5);
	
	Bmp180_SDA_L();     //At start condition, SCL is high and SDA has a falling edge
	delay_1us_32M(5);
	
	Bmp180_SCLK_L() ;   //控制住总线
}
/******************************************************
函数名：void BMP180StopSignal(void)
说明：  停止IIC总线
******************************************************/
static void BMP180StopSignal(void)
{
	Bmp180_OUT();       //输出模式
	
	Bmp180_SDA_L(); 
	Bmp180_SCLK_L();
	delay_1us_32M(5);
	
	Bmp180_SCLK_H();	
	delay_1us_32M(5);
	
	Bmp180_SDA_H() ;    //At stop condition, SCL is also high, but SDA has a rising edge.
}
/******************************************************
函数名： INT8U  BMP180Acknowledge(void)
说明：每次想寄存器写完数据后，等待其应答。
入参：
出参：返回0，表示无应答。返回1，表示有应答。
******************************************************/
static INT8U  BMP180Acknowledge(void)
{
		INT8U i=0 ;
	    Bmp180_IN();
	
		Bmp180_SDA_H() ;    
		Bmp180_SCLK_H() ;
	
		while(Bmp180_SDAIN)
		{
				i++;                      //这里的250足够大了
				if(i>250) return 0;
		}
		
		Bmp180_SCLK_L() ;
		
		return 1;        
}
/******************************************************
函数名：void BMP180WriteByte(INT8U dataCode)
说明：通过IIC线向寄存器写数据。
入参：dataCode，需要写入的数据
出参：
******************************************************/
static void BMP180WriteByte(INT8U dataCode)
{
	INT8U i ;
	Bmp180_OUT();       //输出模式
	
	Bmp180_SCLK_L() ; 
	delay_1us_32M(5);
	
	for(i=0x80 ; i>0 ; i>>=1)
	{
		
		if(i&dataCode)
		{Bmp180_SDA_H() ;}    //写1
		else          
		{Bmp180_SDA_L() ;}    //写0    //BMP180_sda_bit = (bit)(temp & (0x80>>i)) ;
		
		delay_1us_32M(5);
		Bmp180_SCLK_H() ;    
		
		delay_1us_32M(5);
		Bmp180_SCLK_L() ; 
//		delay_1us_32M(5);
	}
}
/******************************************************
函数名： INT8U BMP180ReadByte(void)
说明：通过IIC线从寄存器中读取数据。
入参：
出参：dataCode，返回读取的数据
******************************************************/
static INT8U BMP180ReadByte(void)
{
		INT8U i ;
		INT8U dataCode = 0x00 ;
		Bmp180_IN();

		for(i=0x80; i>0 ; i>>=1)
		{
				Bmp180_SCLK_L() ;
			   delay_1us_32M(5);
		     	Bmp180_SCLK_H() ;
				if(Bmp180_SDAIN)
					dataCode|=i;
			   delay_1us_32M(5);

		}
		return dataCode ;
}
/********************************************************************************
函数名：void BMP180AddressWrite(unsigned char addresss,unsigned char dataCode)
说明：向地址中写入数据
入参：addresss，需要写入数据的目的地址。dataCode，需要写入的数据
出参：
***********************************************************************************/
void BMP180AddressWrite(unsigned char addresss,unsigned char dataCode)
{
    BMP180StartSignal();    
    
    BMP180WriteByte(BMP180_DEVICE_ADDRESS_BASE_VALUE | BMP180_WRITE);   
    if (0==BMP180Acknowledge() )return;
    
    BMP180WriteByte(addresss);   
    if (0==BMP180Acknowledge() )return;
    
    BMP180WriteByte(dataCode);       
    if (0==BMP180Acknowledge() )return;
    
    BMP180StopSignal();                   
}
/********************************************************************************
函数名：INT8U ReadVersion()
说明：读取BMP180的版本号，可测试读取函数及时序的正确性
入参：
出参：返回读到的版本号
***********************************************************************************/

INT8U ReadVersion()
{
	  unsigned char aa=0;
     BMP180StartSignal();    
     BMP180WriteByte(BMP180_DEVICE_ADDRESS_BASE_VALUE);
     if(0==BMP180Acknowledge())//写设备地址，等待应答
       return 0;
     BMP180WriteByte(0xD0);//写寄存器地址，等待应答
     if(0==BMP180Acknowledge())
        return 0;
      
     BMP180StartSignal(); //重新启动
      
     BMP180WriteByte(BMP180_DEVICE_ADDRESS_BASE_VALUE|1);//写设备地址加读信号
     if(0==BMP180Acknowledge())//等待应答
        return 0;
     
     aa= BMP180ReadByte(); //读一个字节
  
     BMP180StopSignal();
     return aa;    
}



/********************************************************************
函数名：INT8U BMP180AddressReadByte(unsigned char address)
说明：读取目的地址中的数据
入参：address 需要读取的目的地址
出参：返回该地址中的数据
************************************************************************/
INT8U BMP180AddressReadByte(INT8U address)
{  
	  INT8U dataCode;
	
    BMP180StartSignal();                          
    
    BMP180WriteByte(BMP180_DEVICE_ADDRESS_BASE_VALUE | BMP180_WRITE);  
    
   if (0==BMP180Acknowledge() )return 0;
    
    BMP180WriteByte(address);           
    if (0==BMP180Acknowledge() )return 0;
    
    BMP180StartSignal();                          
    
    BMP180WriteByte(BMP180_DEVICE_ADDRESS_BASE_VALUE | BMP180_READ);  
   if (0==BMP180Acknowledge() )return 0;
    
    dataCode=BMP180ReadByte();             
	  
    BMP180StopSignal();  
	
    return dataCode; 
}

/******************************************************
函数名:BMP180AddressRead2Byte
说明： 读出目的地址中的两个字节
入参:  目的地址
出参:	 从连续地址读取数据，并"组装"为long型数据

******************************************************/
static unsigned int BMP180AddressRead2Byte(unsigned char address)
{
	unsigned char msb , lsb ;

	msb = BMP180AddressReadByte(address)   ;
	
	lsb = BMP180AddressReadByte(address+1) ;

	return ( ((unsigned int)msb) << 8 | lsb) ;
}


/******************************************************
函数名	: unsigned int BMP180ReadUnsetTemperature(void)
说明		:读取未校正的温度值
入参		:N/A
出参 ：返回为校正的温度值
******************************************************/
static unsigned int BMP180ReadUnsetTemperature(void)
{
	  BMP180AddressWrite(0xf4,0x2e) ;
	  delay_SYSus(5000);                  //延迟5ms，转化需要4.5ms
	  return (BMP180AddressRead2Byte(0xf6));
}

/******************************************************
函数名	:unsigned long BMP180ReadUnsetPressure(void)
说明		:读取未校正的气压值
入参		:N/A
出参 ：返回为校正的气压值
******************************************************/
static unsigned long BMP180ReadUnsetPressure(void)
{

		unsigned long pressure = 0;
		//unsigned char MSB,LSB;
		unsigned char data;
		data = (OSS<<6);
		data = 0x34+data;
		BMP180AddressWrite(0xf4,data) ;

	//	delay5msForBMP180();
	//	delay5msForBMP180();
	//	delay5msForBMP180();
       // OSTimeDly(1);
	
	    delay_SYSus(20000); 
	
		pressure = BMP180AddressRead2Byte(0xf6) ;	    
		pressure = (pressure << 8) + BMP180AddressReadByte(0xf8);

		data = (8-OSS);
		pressure = pressure>> data;
		return pressure;	

}


/******************************************************
Function	:BMP180ReadCalibrateParam
Input		:BMP180_info type point
Output		:AC1,AC3,AC3,AC4,AC5,AC6,B1,B2,MB,MC,MD
Return		:N/A
Description	:读取校正参数
Note		:N/A
******************************************************/
static void BMP180ReadCalibrateParam(BMP180_info *p)
{
	p->cal_param.AC1= BMP180AddressRead2Byte(0xAA);
	p->cal_param.AC2= BMP180AddressRead2Byte(0xAC);
	p->cal_param.AC3= BMP180AddressRead2Byte(0xAE);
	p->cal_param.AC4= BMP180AddressRead2Byte(0xB0);
	p->cal_param.AC5= BMP180AddressRead2Byte(0xB2);
	p->cal_param.AC6= BMP180AddressRead2Byte(0xB4);
	p->cal_param.B1=  BMP180AddressRead2Byte(0xB6);
	p->cal_param.B2=  BMP180AddressRead2Byte(0xB8);
	p->cal_param.MB=  BMP180AddressRead2Byte(0xBA);
	p->cal_param.MC=  BMP180AddressRead2Byte(0xBC);
	p->cal_param.MD=  BMP180AddressRead2Byte(0xBE);
}


/******************************************************
Function	:Init_BMP180
Input		:AC1,AC2,AC3,AC4,AC5,AC6,B1,B2,MB,MC,MD
Output		:AC1,AC2,AC3,AC4,AC5,AC6,B1,B2,MB,MC,MD
Return		:N/A
Description	:初始化
Note		:N/A
******************************************************/
void BMP180Init(BMP180_info *p)
{
//	int af,b1,b9,be;
	BMP180Pin_CFG();
	
		if(BMP180AddressReadByte(BMP180_ID_REGISTER_ADDRESS)== BMP180_ID_FIXED_VALUE)
		{//存在
				p->ExistFlag = BMP180_EXISTENCE ;
			
//			
//			  af=BMP180AddressReadByte(0xaf);
//			   b1=BMP180AddressReadByte(0xb1);
//			 b9=BMP180AddressReadByte(0xb9);
//			 be=BMP180AddressReadByte(0xbe);
			
			
			

				BMP180ReadCalibrateParam(p);

				p->Version = BMP180AddressReadByte(BMP180_VERSION_REGISTER_ADDRESS);
		}
		else
		{//不存在
				p->ExistFlag = BMP180_NOT_EXISTENCE ;
		}
}
/******************************************************
函数名：void BMP180Convert(BMP180_info *temp)
说明：根据校正参数，校正温度与气压的值
入参：temp 保存参数
出参：
******************************************************/
void BMP180Convert(BMP180_info *temp)
{	
		long x1,  B5, B6, x3, B3, p;
		long x2;
		unsigned long b4, b7;
		double  BP,BP1;
		//未校正的温度值
		temp->UnsetTemperature = BMP180ReadUnsetTemperature();
		//未校正的气压值
		temp->UnsetGasPress = BMP180ReadUnsetPressure();

		//温度校正
		//x1 = ((temp->UnsetTemperature) - temp->cal_param.AC6) * (temp->cal_param.AC5) >> 15;
		x1=temp->UnsetTemperature - temp->cal_param.AC6;
		x1 = x1 * temp->cal_param.AC5;
		x1 = x1>>15;
		//x2 = ((long)(temp->cal_param.MC) << 11) / (x1 + temp->cal_param.MD);
		x3 = x1+ temp->cal_param.MD;
		x2 = (long)temp->cal_param.MC<<11;//此算法有误
		x2 = x2 /x3;


		B5 = x1 + x2;
		temp->Temperature= (B5 + 8) >> 4;

		//气压校正
		B6 = B5- 4000;
		//x1 = ((long)(temp->cal_param.B2) * (B6 * B6 >> 12)) >> 11;
		x1=B6 * B6;
		x1=x1>>12;
		x1=(long)(temp->cal_param.B2)*x1;
		x1=x1>>11;


		//x2 = ((long)temp->cal_param.AC2) * B6 >> 11;
		x2 =(long)(temp->cal_param.AC2) * B6;
		x2 = x2 >>11;

		x3 = x1 + x2;

		//B3 = ((((long)(temp->cal_param.AC1) * 4 + x3)<<OSS) + 2)/4;
		B3 =(long)(temp->cal_param.AC1) * 4;
		B3 =B3+x3;
		B3=B3<<OSS;
		B3=B3+2;
		B3=B3 / 4 ;

		//x1 = ((long)temp->cal_param.AC3) * B6 >> 13;
		x1 = ((long)temp->cal_param.AC3) * B6 ;
		x1 = x1 >> 13 ;

		//x2 = ((long)(temp->cal_param.B1) * (B6 * B6 >> 12)) >> 16;
		x2 =  B6*B6;
		x2= x2 >>12;
		x2 = (long)(temp->cal_param.B1) * x2;
		x2 = x2 >>16;


		//x3 = ((x1 + x2) + 2) >> 2;
		x3=x1+x2+2;
		x3 =x3>>2;


		//b4 = (temp->cal_param.AC4) * (unsigned long) (x3 + 32768)) >> 15;
		b4=(unsigned long) (x3 + 32768);
		b4 = temp->cal_param.AC4 * b4;
		b4=b4>>15;

		//b7 = ((unsigned long)(temp->UnsetGasPress) - B3) * (50000 >> OSS);
		b7=(unsigned long)(temp->UnsetGasPress) -B3;
		x1=50000 >> OSS;
		b7 =b7*x1;
		if (b4==0)return ;//0除保护
		if( b7 < 0x80000000)
		{
			//p = (b7 * 2) / b4 ;
			 p=b7*2;
			 p=p/b4;
		}
		else
		{
			//p = (b7 / b4) * 2;
				p = (b7 / b4);
				p=p*2;		
		}
		//x1 = (p >> 8) * (p >> 8);
		x1 =p>>8;
		x1=x1*x1;

		//x1 = ((long)x1 * 3038) >> 16;
		x1 = x1 * 3038;
		x1 = x1 >>16;

		//x2 = (-7357 * p) >> 16;
		x2 =(-7357 * p);
		x2 = x2 >> 16;

		//temp->GasPress= p + ((x1 + x2 + 3791) >> 4);
		x1 = x1+x2+3791;
		x1 = x1 >> 4;
		temp->GasPress =x1+p;
		//海拔计算
		//temp->Altitude =(44330.0 * (1.0-pow((float)(temp->GasPress) / 101325.0, 1.0/5.255)) );
#if 1
        BP = (double)(temp->GasPress);
        BP = BP / 101325.0;
        BP1 =1.0/5.255;
        BP=pow(BP,BP1);
        BP1 = 1-BP;
        temp->Altitude=BP1 * 44330.0;
        temp->Altitude = (unsigned int) (temp->Altitude);
#endif    
	
}
