
#include "ucos_ii.h" 
#include "stm32f10x_map.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_RCC.h"
//#include "bsp_cc1101_reg.h"
#include "bsp_cc1101.h"

#include "string.h"
#include "MemConfig.h"
INT8U Get_Local_Para(INT16U ID,INT8U *inBuff,INT16U *OutLen);
// CC1100 STROBE, CONTROL AND STATUS REGSITER
#if  BANPCB2 ==  0x21
		#define CC1101_GD0_Pin   GPIO_Pin_10
		#define CC1101_GD0_Port  GPIOC        //PC10  GPD0
		#define CC1101_NSS_Pin   GPIO_Pin_11
		#define CC1101_NSS_Port  GPIOC        //PC11  NSS
		#define CC1101_SCK_Pin   GPIO_Pin_12
		#define CC1101_SCK_Port  GPIOC        //PC12  SCK

		#define CC1101_MOSI_Pin  GPIO_Pin_5
		#define CC1101_MOSI_Port GPIOB

		#define CC1101_SOMI_Pin  GPIO_Pin_6
		#define CC1101_SOMI_Port GPIOB

		#define CC1101_GD2_Pin   GPIO_Pin_7
		#define CC1101_GD2_Port  GPIOB
		#define CC1101CKEN  (RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC)
#endif
#if  BANPCB2 ==  0x10
		#define CC1101_GD0_Pin   GPIO_Pin_10
		#define CC1101_GD0_Port  GPIOC        //PC10  GPD0
		#define CC1101_NSS_Pin   GPIO_Pin_11
		#define CC1101_NSS_Port  GPIOC        //PC11  NSS
		#define CC1101_SCK_Pin   GPIO_Pin_12
		#define CC1101_SCK_Port  GPIOC        //PC12  SCK

		#define CC1101_MOSI_Pin  GPIO_Pin_5
		#define CC1101_MOSI_Port GPIOB

		#define CC1101_SOMI_Pin  GPIO_Pin_6
		#define CC1101_SOMI_Port GPIOB

		#define CC1101_GD2_Pin   GPIO_Pin_7
		#define CC1101_GD2_Port  GPIOB
		#define CC1101CKEN  (RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC)
#endif

#define CC_CSN_HIGH()  	GPIO_SetBits(CC1101_NSS_Port, CC1101_NSS_Pin)

#define CC_CLK_HIGH()   GPIO_SetBits(CC1101_SCK_Port, CC1101_SCK_Pin)
#define CC_CLK_LOW()    GPIO_ResetBits(CC1101_SCK_Port, CC1101_SCK_Pin)

#define CC_MOSI_HIGH()   GPIO_SetBits(CC1101_MOSI_Port, CC1101_MOSI_Pin)
#define CC_MOSI_LOW()    GPIO_ResetBits(CC1101_MOSI_Port, CC1101_MOSI_Pin)

#define CC_SOMI()       (CC1101_SOMI_Port->IDR&CC1101_SOMI_Pin) 

#define CC_IRQ_READ()   (CC1101_GD0_Port->IDR&CC1101_GD0_Pin)

#define CCCLKEN       (RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC)
 INT8U PaTabel[] = { 0xc0, 0x60, 0x60, 0x60,0x60, 0x60, 0x60, 0x60};
 INT8U CC1100RxBuff[64];
 //INT8U PaTabel[] = { 0xc0, 0xC8, 0x84, 0x60, 0x68, 0x34, 0x1D, 0x0E};
 #define INITNUM 25
#if INITNUM > 255
    #error "INITNUMErr"
#endif
static const INT8U CC1101InitData[INITNUM][2]= 
{
  {CC1101_IOCFG0,      0x46},
  {CC1101_IOCFG2,      0x09},
  {CC1101_FIFOTHR,     0x4f},//定为0x4f时，接收字节数可扩大，不影响发送
  {CC1101_PKTCTRL1,    0x0f},
  {CC1101_PKTCTRL0,    0x05},
  {CC1101_ADDR,        0x05},
  {CC1101_PKTLEN,      0x24},
  {CC1101_CHANNR,      0x01},
  {CC1101_FSCTRL1,     0x0F},
	
  //{CC1101_FREQ2,       0x10},
  //{CC1101_FREQ1,       0xA7},
  //{CC1101_FREQ0,       0x84},//70
	
  {CC1101_MDMCFG4,     0xf6},//38.2K(fa,81),2.4k(F6,83)
  {CC1101_MDMCFG3,     0x83},
  {CC1101_MDMCFG2,     0x12},
  {CC1101_MDMCFG1,     0x02},//??????0x82
  {CC1101_MDMCFG0,     0xF8},
  {CC1101_DEVIATN,     0x00}, //?????????1.5k 
  {CC1101_FOCCFG,      0x16},
  {CC1101_WORCTRL,     0xFB},
  {CC1101_FSCAL3,      0xE9},
  {CC1101_FSCAL2,      0x2A},
  {CC1101_FSCAL1,      0x00},
  {CC1101_FSCAL0,      0x1F},
  {CC1101_TEST2,       0x81},
  {CC1101_TEST1,       0x35},
  {CC1101_MCSM1,       0x3F},
  {CC1101_MCSM0,       0x18},
};


/* --------------------Private functions------------------------------------------------------*/
void CC1101_Pin_Init(void)
{
		GPIO_InitTypeDef 	GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(CCCLKEN,ENABLE);
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Pin =CC1101_NSS_Pin;
		CC_CSN_HIGH();
		GPIO_Init(CC1101_NSS_Port, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin =CC1101_MOSI_Pin;
		GPIO_Init(CC1101_MOSI_Port, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin =CC1101_SCK_Pin;
		GPIO_Init(CC1101_SCK_Port, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Pin =CC1101_SOMI_Pin;
		GPIO_Init(CC1101_SOMI_Port, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Pin =CC1101_GD0_Pin;
		GPIO_Init(CC1101_GD0_Port, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Pin =CC1101_GD2_Pin;
		GPIO_Init(CC1101_GD2_Port, &GPIO_InitStructure);
}
void CC1101_Pin_Close(void)
{
		GPIO_InitTypeDef 	GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(CCCLKEN,ENABLE);
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Pin =CC1101_NSS_Pin;
		CC_CSN_HIGH();
		GPIO_Init(CC1101_NSS_Port, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin =CC1101_MOSI_Pin;
		GPIO_Init(CC1101_MOSI_Port, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin =CC1101_SCK_Pin;
		GPIO_Init(CC1101_SCK_Port, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Pin =CC1101_SOMI_Pin;
		GPIO_Init(CC1101_SOMI_Port, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Pin =CC1101_GD0_Pin;
		GPIO_Init(CC1101_GD0_Port, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Pin =CC1101_GD2_Pin;
		GPIO_Init(CC1101_GD2_Port, &GPIO_InitStructure);
}
void RF_dealy_ms(INT16U ms)
{	
		INT16U us;
		while(ms--)
		{
				us=1000;
				while(us--);
		}		
}
void RF_dealy_us(INT16U us)
{	
	INT8U i=100;
				while(us--)
				{
						while(i--);
				}					
}
 /*******************************************************************************
* Function Name  : SPI_ExchangeByte.
* Description    :  
* Input 1        :  SPI  空余  与STM8L 程序吻合，
* Input 2        : PtrToBuffer is an u8 pointer to the first word to be transmitted.
* Input 3        : NbOfWords parameter indicates the number of words to be sent.
* Output         : None.
* Return         : TRUE / FALSE.
*******************************************************************************/

 INT8U SPI_ExchangeByte(INT8U val)
{
			INT8U i;
			INT8U RxData;
			RxData=0;
			for(i=0x80;i!=0;i>>=1)
			{	
				CC_CLK_LOW();
				if(val&i)    	CC_MOSI_HIGH();//数据建立
				else         	CC_MOSI_LOW();
 
				CC_CLK_HIGH();      //上升沿数据被写入到铁电  
				if(CC_SOMI())
				{
						RxData|=i;
				}
			}
			return RxData;
 }
void CC_CSN_LOW()
{
    INT16U i=60000;
    GPIO_ResetBits(CC1101_NSS_Port, CC1101_NSS_Pin);
    while ((CC_SOMI())&&(i--));
}

/*
================================================================================
Function : CC1101WriteReg( )
    Write a byte to the specified register
INPUT    : addr, The address of the register
           value, the byte you want to write
OUTPUT   : None
================================================================================
*/
void CC1101WriteReg( INT8U addr, INT8U value )
{
    CC_CSN_LOW( );
    SPI_ExchangeByte( addr );
    SPI_ExchangeByte( value );
    CC_CSN_HIGH( );
}
/*================================================================================
Function : CC1101WORInit( )
    Initialize the WOR function of CC1101
INPUT    : None
OUTPUT   : None
================================================================================*/
void  CC1101WORInit(void)
{
		CC1101WriteReg(CC1101_MCSM0,0x18);
    CC1101WriteReg(CC1101_WORCTRL,0x78); //Wake On Radio Control
    CC1101WriteReg(CC1101_MCSM2,0x00);
    CC1101WriteReg(CC1101_WOREVT1,0x8C);
    CC1101WriteReg(CC1101_WOREVT0,0xA0);
		CC1101WriteCmd( CC1101_SWORRST );
}
/*
================================================================================
Function : CC1101ReadReg( )
    read a byte from the specified register
INPUT    : addr, The address of the register
OUTPUT   : the byte read from the rigister
================================================================================
*/
INT8U CC1101ReadReg( INT8U addr )
{
    INT8U i;
    CC_CSN_LOW( );
    SPI_ExchangeByte(  addr | READ_SINGLE);
    i = SPI_ExchangeByte( 0xFF );
    CC_CSN_HIGH( );
    return i;
}
/*
================================================================================
Function : CC1101ReadMultiReg( )
    Read some bytes from the rigisters continously
INPUT    : addr, The address of the register
           buff, The buffer stores the data
           size, How many bytes should be read
OUTPUT   : None
================================================================================
*/
void CC1101ReadMultiReg( INT8U addr, INT8U *buff, INT8U size )
{
    INT8U i, j;
    CC_CSN_LOW( );
    SPI_ExchangeByte(  addr | READ_BURST);
    for( i = 0; i < size; i ++ )
    {
        for( j = 0; j < 20; j ++ );
        *( buff + i ) = SPI_ExchangeByte(  0xFF );
    }
    CC_CSN_HIGH( );
}
/*
================================================================================
Function : CC1101ReadStatus( )
    Read a status register
INPUT    : addr, The address of the register
OUTPUT   : the value read from the status register
================================================================================
*/
INT8U CC1101ReadStatus( INT8U addr )
{
    INT8U i;
    CC_CSN_LOW( );
    SPI_ExchangeByte(  addr | READ_BURST);
    i = SPI_ExchangeByte(  0xFF );
    CC_CSN_HIGH( );
    return i;
}
/*
================================================================================
Function : CC1101SetTRMode( )
    Set the device as TX mode or RX mode
INPUT    : mode selection
OUTPUT   : None
================================================================================
*/
void CC1101SetTRMode( INT8U mode )
{
    if( mode == TX_MODE )
    {
        CC1101WriteReg(CC1101_IOCFG0,0x46);
        CC1101WriteCmd( CC1101_STX );
    }
    else if( mode == RX_MODE )
    {
        CC1101WriteReg(CC1101_IOCFG0,0x41);
        CC1101WriteCmd( CC1101_SRX );
    }
}
 
/*
================================================================================
Function : CC1101WriteMultiReg( )
    Write some bytes to the specified register
INPUT    : addr, The address of the register
           buff, a buffer stores the values
           size, How many byte should be written
OUTPUT   : None
================================================================================
*/
void CC1101WriteMultiReg( INT8U addr, INT8U *buff, INT8U size )
{
    INT8U i;
    CC_CSN_LOW( );
    SPI_ExchangeByte(  addr | WRITE_BURST );
    for( i = 0; i < size; i ++ )
    {
        SPI_ExchangeByte(  *( buff + i ) );
    }
    CC_CSN_HIGH( );
}
/*
================================================================================
Function : CC1101WriteCmd( )
    Write a command byte to the device
INPUT    : command, the byte you want to write
OUTPUT   : None
================================================================================
*/
void CC1101WriteCmd( INT8U command )
{
    CC_CSN_LOW( );
    SPI_ExchangeByte( command );
    CC_CSN_HIGH( );
}
/*
================================================================================
Function : CC1101Reset( )
    Reset the CC1101 device
INPUT    : None
OUTPUT   : None
================================================================================
*/
void CC1101Reset( void )
{
    INT8U x;

    CC_CSN_HIGH( );
    CC_CSN_LOW( );
    CC_CSN_HIGH( );
    for( x = 0; x < 100; x ++ );
    CC1101WriteCmd( CC1101_SRES );
}
/*
================================================================================
Function : CC1101SetIdle( )
    Set the CC1101 into IDLE mode
INPUT    : None
OUTPUT   : None
================================================================================
*/
void CC1101SetIdle( void )
{
    CC1101WriteCmd(CC1101_SIDLE);
}
/*
================================================================================
Function : CC1101ClrTXBuff( )
    Flush the TX buffer of CC1101
INPUT    : None
OUTPUT   : None
================================================================================
*/
void CC1101ClrTXBuff( void )
{
    CC1101SetIdle();//MUST BE IDLE MODE
    CC1101WriteCmd( CC1101_SFTX );
}
/*
================================================================================
Function : CC1101ClrRXBuff( )
    Flush the RX buffer of CC1101
INPUT    : None
OUTPUT   : None
================================================================================
*/
void CC1101ClrRXBuff( void )
{
    CC1101SetIdle();//MUST BE IDLE MODE
    CC1101WriteCmd( CC1101_SFRX );
}
/*
================================================================================
Function : CC1101SendPacket( )
    Send a packet
INPUT    : txbuffer, The buffer stores data to be sent
           size, How many bytes should be sent
           mode, Broadcast or address check packet
OUTPUT   : None
================================================================================
*/
#if 0
void CC1101SendPacket( INT8U *txbuffer, INT8U size)
{
     CC1101ClrTXBuff( );
     CC1101WriteReg( CC1101_TXFIFO, size + 1 );
     CC1101WriteReg( CC1101_TXFIFO, 0x05 );
     CC1101WriteMultiReg( CC1101_TXFIFO, txbuffer, size );
     CC1101SetTRMode( TX_MODE );
     
    // while( GPIO_ReadInputDataBit( GPIOB, GPIO_Pin_3 ) != 0 );
    // while( GPIO_ReadInputDataBit( GPIOB, GPIO_Pin_3 ) == 0 );//GDO0
      Delay_10Ms(15);
     if( GPIO_ReadInputDataBit( GPIOB, GPIO_Pin_3 ) == 0 )
       Delay_10Ms(5);;//GDO0
     CC1101ClrTXBuff( );
}
#endif
 
void CC1101SendPacket( INT8U *txbuffer, INT8U size )
{
      INT16U CiShu;
      INT8U lxcs=4;
      CC1101ClrTXBuff( );
      while(lxcs--)
      {
          CC1101WriteReg( CC1101_TXFIFO, size + 1 );
          CC1101WriteReg( CC1101_TXFIFO, 0x05 );
          CC1101WriteMultiReg( CC1101_TXFIFO, txbuffer, size );
          CC1101SetTRMode( TX_MODE );
          CiShu=3000;
          while( GPIO_ReadInputDataBit(CC1101_GD0_Port, CC1101_GD0_Pin  ) != 0 )
          { 
              RF_dealy_ms(1);
              CiShu--;
              if(CiShu==0)break;
          }
          while( GPIO_ReadInputDataBit( CC1101_GD0_Port, CC1101_GD0_Pin ) == 0 )
          { 
                RF_dealy_ms(1);//GDO0
                CiShu--;
                if(CiShu==0)break;
          }

          CC1101ClrTXBuff( );
          //if(WaitAck(Recive,Len))//??????
            break;
      }
}

/*
================================================================================
Function : CC1101GetRXCnt( )
    Get received count of CC1101
INPUT    : None
OUTPUT   : How many bytes hae been received
================================================================================
*/
INT8U CC1101GetRXCnt( void )
{
    return ( CC1101ReadStatus( CC1101_RXBYTES )  & BYTES_IN_RXFIFO );
}
/*
================================================================================
Function : CC1101SetAddress( )
    Set the address and address mode of the CC1101
INPUT    : address, The address byte
           AddressMode, the address check mode
OUTPUT   : None
================================================================================
*/
void CC1101SetAddress( INT8U address, INT8U AddressMode)
{
    INT8U btmp = CC1101ReadReg( CC1101_PKTCTRL1 ) & ~0x03;
    CC1101WriteReg(CC1101_ADDR, address);
    if     ( AddressMode == BROAD_ALL )     {}
    else if( AddressMode == BROAD_NO  )     { btmp |= 0x01; }
    else if( AddressMode == BROAD_0   )     { btmp |= 0x02; }
    else if( AddressMode == BROAD_0AND255 ) { btmp |= 0x03; }   
}
/*
================================================================================
Function : CC1101SetSYNC( )
    Set the SYNC bytes of the CC1101
INPUT    : sync, 16bit sync 
OUTPUT   : None
================================================================================
*/
void CC1101SetSYNC( INT16U sync )
{
    CC1101WriteReg(CC1101_SYNC1, 0xFF & ( sync>>8 ) );
    CC1101WriteReg(CC1101_SYNC0, 0xFF & sync ); 
}
/*
================================================================================
Function : CC1101SetSYNC( )
    Set the SYNC bytes of the CC1101
INPUT    : sync, 16bit sync 
OUTPUT   : None
================================================================================
*/
void CC1101SetSYNCForByte( INT8U hsync,INT8U Lsync )
{
    CC1101WriteReg(CC1101_SYNC1, hsync );
    CC1101WriteReg(CC1101_SYNC0, Lsync ); 
}
/*
================================================================================
Function : void CC1101SetFREQ(INT8U FREQ2,INT8U FREQ1,INT8U FREQ0)
            设置CC1101的通信频点
INPUT    : 
OUTPUT   : None
================================================================================
*/
void CC1101SetFREQ(INT8U FREQ2,INT8U FREQ1,INT8U FREQ0)
{
	CC1101WriteReg(CC1101_FREQ2,  FREQ2);
	CC1101WriteReg(CC1101_FREQ1,  FREQ1);
	CC1101WriteReg(CC1101_FREQ0,  FREQ0); 
}

/*
================================================================================
Function : CC1101RecPacket( )
    Receive a packet
INPUT    : rxBuffer, A buffer store the received data
OUTPUT   : 1:received count, 0:no data
================================================================================
*/
INT8U CC1101RecPacket( INT8U *rxBuffer,INT8U RXLen)//?????????
{
    INT8U status[2] = {0};
    INT8U pktLen = 0;
    INT16U x = 0;
    
    if ( CC1101GetRXCnt( ) != 0 )
    {
        pktLen = CC1101ReadReg(CC1101_RXFIFO);      // Read length byte
			
        if( ( CC1101ReadReg( CC1101_PKTCTRL1 ) & ~0x03 ) != 0 )
        {
            x = CC1101ReadReg(CC1101_RXFIFO);
					  x=x;       // zzs add this to avoid warning.
        }
				
        if( pktLen == 0 )           
        { 
          return 0; 
        }
        else                        
        { 
          pktLen --; 
        }
        if (pktLen<=RXLen) 
        {
              CC1101ReadMultiReg(CC1101_RXFIFO, rxBuffer, pktLen); // Pull data
              CC1101ReadMultiReg(CC1101_RXFIFO, status, 2);   // Read  status bytes
             // CC1101ClrRXBuff( );
        }
        else
        {
            CC1101ReadMultiReg(CC1101_RXFIFO, rxBuffer, RXLen); // Pull data            
            //?????????
        }
         CC1101ClrRXBuff( );
         return pktLen;        
    }
    else   
    {  
      return 0; 
    }                               // Error
}
/*
================================================================================
Function : CC1101Init( )
    Initialize the CC1101, User can modify it
INPUT    : None
OUTPUT   : None
================================================================================
*/
INT8U test=0;
INT8U version=0;
INT8U CC1101Init( void )
{
	volatile INT8U i = 0, j = 0;
	INT16U Le = 0;
	INT8U Temp[3] = {0};

	 
	CC1101Reset( );    

	for( i = 0; i < INITNUM; i++ )
	{
		CC1101WriteReg( CC1101InitData[i][0], CC1101InitData[i][1] );
	}
	
	Get_Local_Para(0x000B,Temp,&Le);//AYNC                  默认值 8799
	CC1101SetSYNCForByte(Temp[0],Temp[1]);
	Get_Local_Para(0x000C,Temp,&Le);//频率                  频率 默认10 A7 84
	CC1101SetFREQ(Temp[0],Temp[1],Temp[2]);//设置频率 
	// CC1101SetAddress( 0x05, BROAD_0AND255 );
	// CC1101SetSYNC( 0xAA55);
	CC1101WriteReg(CC1101_MDMCFG1,   0x72); //Modem Configuration
	CC1101WriteMultiReg(CC1101_PATABLE, PaTabel, 8 );

	i = CC1101ReadStatus( CC1101_PARTNUM );//for test, must be 0x80
	version = CC1101ReadStatus( CC1101_VERSION );//for test, refer to the datasheet
	
	if((version==0x04)||(version==0x14))
	{
		return 1;
	}
	
	return 0;
}
void CCA_Config(void)
{
      CC1101WriteReg(CC1101_IOCFG2,0x0E);//????????0x0E
      CC1101WriteReg(CC1101_MCSM1,0x3F);
      CC1101WriteReg(CC1101_AGCCTRL1,0x40);
      CC1101WriteReg(CC1101_AGCCTRL1,0x07);
}

/**********************************************************************
???:void RF_RxHandler()
??:  RF?????,?????????Uart_In?
??:   
??: 
***********************************************************************/
INT8U RF_RxHandler(INT8U *OutBuff)
{
	INT8U len=0; 

	if(0==CC_IRQ_READ())
	{
		 len=CC1101RecPacket(CC1100RxBuff,64);
		 if(len)
		 {
			 if(len>64)return 0;
			 memcpy(OutBuff,CC1100RxBuff,len);         
			 return len;
		 }
	} 
	return 0;
}

/********************************************************************
函数名：INT8U RF_SignCS(INT16U CS_Time)
功能：  检测RF信道是否占用
入参：  CS_Time 检测时间，检测GD2（PB2）是否为高电平 以100US 检测一次
出参：  返回值 1:检测到空间内有信号，0 ：未检测到无线信号
***********************************************************************/
INT8U RF_SignCS(INT16U CS_Time)
{
  INT16U HciShu = 0;
  INT16U i = 0;

  CC1101WriteReg(CC1101_IOCFG2,0x09);//???????
  CC1101WriteReg(CC1101_AGCCTRL1,0x03);
  CC1101SetTRMode(RX_MODE);
	while(CS_Time--)
	{
		//	if (GPIO_ReadInputDataBit(CC1101_GD2_Port,CC1101_GD2_Pin)==Bit_SET)
		for(i=0;i<500;i++);//500
		if((CC1101_GD2_Port->IDR&CC1101_GD2_Pin))    continue;				
			HciShu++;
		 if(HciShu>10)
					return 1; 
	}
	 
	return 0;
}

/************************************************************************************************************************
* Function Name:  INT8U RF_SoftWOR(void)            
* Description  :  RF_软件的WOR功能使用。检测信道需要打开RX功能，打工RX功能需要等待2MS，才能检测信道
* Input        :  time : 世纪秒
*
* Return       :  1 : 检测到空间内有信号
*                 0 ：未检测到无线信号
*
* Author:                               
* Date First Issued: 赵志舜于2018年1月125日创建本函数             E-Mail:11207656@qq.com
* Version:  V1.0
*************************************************************************************************************************/
INT8U RF_SoftWOR(void)
{
	if(RF_SignCS(30))
	{
		return 1;  // 有信号
	}
	
	CC1101WriteCmd(CC1101_SIDLE);
	CC1101WriteCmd( CC1101_SPWD );
	CC1101WriteCmd(CC1101_SIDLE);
	CC1101WriteCmd( CC1101_SXOFF );
 
	return 0;
}
