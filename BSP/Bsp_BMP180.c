/******************************************************************************
�ļ���BSP_BMP180.C  
���ܣ�����ѹ�������ӿ�ͨѶ
Ӳ���ӿڣ�IIC
         BMP180֧�����3.4M�Ĵ����ٶ�
********************************************************************************/


#include "Bsp_Bmp180.h"
#include <math.h> //pow ���� ����



//short ac1,ac2,ac3,b1,b2,mb,mc,md;
//unsigned short ac4,ac5,ac6;


/******************************************************
��������void BMP180Pin_CFG(void)
˵����  BMP180 IIC���ų�ʼ��
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
��������void BMP180StartSignal(void)
˵����  ��ʼ��IIC���ߣ�׼����д����
******************************************************/
static void BMP180StartSignal(void)
{     
	Bmp180_OUT();       //���ģʽ
	
	
	Bmp180_SDA_H() ; 
	Bmp180_SCLK_H() ;	
	delay_1us_32M(5);
	
	Bmp180_SDA_L();     //At start condition, SCL is high and SDA has a falling edge
	delay_1us_32M(5);
	
	Bmp180_SCLK_L() ;   //����ס����
}
/******************************************************
��������void BMP180StopSignal(void)
˵����  ֹͣIIC����
******************************************************/
static void BMP180StopSignal(void)
{
	Bmp180_OUT();       //���ģʽ
	
	Bmp180_SDA_L(); 
	Bmp180_SCLK_L();
	delay_1us_32M(5);
	
	Bmp180_SCLK_H();	
	delay_1us_32M(5);
	
	Bmp180_SDA_H() ;    //At stop condition, SCL is also high, but SDA has a rising edge.
}
/******************************************************
�������� INT8U  BMP180Acknowledge(void)
˵����ÿ����Ĵ���д�����ݺ󣬵ȴ���Ӧ��
��Σ�
���Σ�����0����ʾ��Ӧ�𡣷���1����ʾ��Ӧ��
******************************************************/
static INT8U  BMP180Acknowledge(void)
{
		INT8U i=0 ;
	    Bmp180_IN();
	
		Bmp180_SDA_H() ;    
		Bmp180_SCLK_H() ;
	
		while(Bmp180_SDAIN)
		{
				i++;                      //�����250�㹻����
				if(i>250) return 0;
		}
		
		Bmp180_SCLK_L() ;
		
		return 1;        
}
/******************************************************
��������void BMP180WriteByte(INT8U dataCode)
˵����ͨ��IIC����Ĵ���д���ݡ�
��Σ�dataCode����Ҫд�������
���Σ�
******************************************************/
static void BMP180WriteByte(INT8U dataCode)
{
	INT8U i ;
	Bmp180_OUT();       //���ģʽ
	
	Bmp180_SCLK_L() ; 
	delay_1us_32M(5);
	
	for(i=0x80 ; i>0 ; i>>=1)
	{
		
		if(i&dataCode)
		{Bmp180_SDA_H() ;}    //д1
		else          
		{Bmp180_SDA_L() ;}    //д0    //BMP180_sda_bit = (bit)(temp & (0x80>>i)) ;
		
		delay_1us_32M(5);
		Bmp180_SCLK_H() ;    
		
		delay_1us_32M(5);
		Bmp180_SCLK_L() ; 
//		delay_1us_32M(5);
	}
}
/******************************************************
�������� INT8U BMP180ReadByte(void)
˵����ͨ��IIC�ߴӼĴ����ж�ȡ���ݡ�
��Σ�
���Σ�dataCode�����ض�ȡ������
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
��������void BMP180AddressWrite(unsigned char addresss,unsigned char dataCode)
˵�������ַ��д������
��Σ�addresss����Ҫд�����ݵ�Ŀ�ĵ�ַ��dataCode����Ҫд�������
���Σ�
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
��������INT8U ReadVersion()
˵������ȡBMP180�İ汾�ţ��ɲ��Զ�ȡ������ʱ�����ȷ��
��Σ�
���Σ����ض����İ汾��
***********************************************************************************/

INT8U ReadVersion()
{
	  unsigned char aa=0;
     BMP180StartSignal();    
     BMP180WriteByte(BMP180_DEVICE_ADDRESS_BASE_VALUE);
     if(0==BMP180Acknowledge())//д�豸��ַ���ȴ�Ӧ��
       return 0;
     BMP180WriteByte(0xD0);//д�Ĵ�����ַ���ȴ�Ӧ��
     if(0==BMP180Acknowledge())
        return 0;
      
     BMP180StartSignal(); //��������
      
     BMP180WriteByte(BMP180_DEVICE_ADDRESS_BASE_VALUE|1);//д�豸��ַ�Ӷ��ź�
     if(0==BMP180Acknowledge())//�ȴ�Ӧ��
        return 0;
     
     aa= BMP180ReadByte(); //��һ���ֽ�
  
     BMP180StopSignal();
     return aa;    
}



/********************************************************************
��������INT8U BMP180AddressReadByte(unsigned char address)
˵������ȡĿ�ĵ�ַ�е�����
��Σ�address ��Ҫ��ȡ��Ŀ�ĵ�ַ
���Σ����ظõ�ַ�е�����
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
������:BMP180AddressRead2Byte
˵���� ����Ŀ�ĵ�ַ�е������ֽ�
���:  Ŀ�ĵ�ַ
����:	 ��������ַ��ȡ���ݣ���"��װ"Ϊlong������

******************************************************/
static unsigned int BMP180AddressRead2Byte(unsigned char address)
{
	unsigned char msb , lsb ;

	msb = BMP180AddressReadByte(address)   ;
	
	lsb = BMP180AddressReadByte(address+1) ;

	return ( ((unsigned int)msb) << 8 | lsb) ;
}


/******************************************************
������	: unsigned int BMP180ReadUnsetTemperature(void)
˵��		:��ȡδУ�����¶�ֵ
���		:N/A
���� ������ΪУ�����¶�ֵ
******************************************************/
static unsigned int BMP180ReadUnsetTemperature(void)
{
	  BMP180AddressWrite(0xf4,0x2e) ;
	  delay_SYSus(5000);                  //�ӳ�5ms��ת����Ҫ4.5ms
	  return (BMP180AddressRead2Byte(0xf6));
}

/******************************************************
������	:unsigned long BMP180ReadUnsetPressure(void)
˵��		:��ȡδУ������ѹֵ
���		:N/A
���� ������ΪУ������ѹֵ
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
Description	:��ȡУ������
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
Description	:��ʼ��
Note		:N/A
******************************************************/
void BMP180Init(BMP180_info *p)
{
//	int af,b1,b9,be;
	BMP180Pin_CFG();
	
		if(BMP180AddressReadByte(BMP180_ID_REGISTER_ADDRESS)== BMP180_ID_FIXED_VALUE)
		{//����
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
		{//������
				p->ExistFlag = BMP180_NOT_EXISTENCE ;
		}
}
/******************************************************
��������void BMP180Convert(BMP180_info *temp)
˵��������У��������У���¶�����ѹ��ֵ
��Σ�temp �������
���Σ�
******************************************************/
void BMP180Convert(BMP180_info *temp)
{	
		long x1,  B5, B6, x3, B3, p;
		long x2;
		unsigned long b4, b7;
		double  BP,BP1;
		//δУ�����¶�ֵ
		temp->UnsetTemperature = BMP180ReadUnsetTemperature();
		//δУ������ѹֵ
		temp->UnsetGasPress = BMP180ReadUnsetPressure();

		//�¶�У��
		//x1 = ((temp->UnsetTemperature) - temp->cal_param.AC6) * (temp->cal_param.AC5) >> 15;
		x1=temp->UnsetTemperature - temp->cal_param.AC6;
		x1 = x1 * temp->cal_param.AC5;
		x1 = x1>>15;
		//x2 = ((long)(temp->cal_param.MC) << 11) / (x1 + temp->cal_param.MD);
		x3 = x1+ temp->cal_param.MD;
		x2 = (long)temp->cal_param.MC<<11;//���㷨����
		x2 = x2 /x3;


		B5 = x1 + x2;
		temp->Temperature= (B5 + 8) >> 4;

		//��ѹУ��
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
		if (b4==0)return ;//0������
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
		//���μ���
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
