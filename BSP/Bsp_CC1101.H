#ifndef	__bsp_cc1101_H
#define	__bsp_cc1101_H

#include "os_cpu.h"
#include "stm32f10x_type.h"

#define TX_MODE  1
#define RX_MODE  0
#include "bsp_cc1101_reg.h"

typedef enum { BROAD_ALL, BROAD_NO, BROAD_0, BROAD_0AND255 } ADDR_MODE;
typedef enum { BROADCAST, ADDRESS_CHECK} TX_DATA_MODE;
//10, 7, 5, 0, -5, -10, -15, -20, dbm output power, 0x12 == -30dbm
//INT8U PaTabel[] = { 0xc0, 0xC8, 0x84, 0x60, 0x68, 0x34, 0x1D, 0x0E};


// Sync word qualifier mode = 30/32 sync word bits detected 
// CRC autoflush = false 
// Channel spacing = 199.951172 
// Data format = Normal mode 
// Data rate = 2.00224 
// RX filter BW = 58.035714 
// PA ramping = false 
// Preamble count = 4 
// Whitening = false 
// Address config = No address check 
// Carrier frequency = 400.199890 
// Device address = 0 
// TX power = 10 
// Manchester enable = false 
// CRC enable = true 
// Deviation = 5.157471 
// Packet length mode = Variable packet length mode. Packet length configured by the first byte after sync word 
// Packet length = 255 
// Modulation format = GFSK 
// Base frequency = 399.999939 
// Modulated = true 
// Channel number = 1 
// PA table 
//#define PA_TABLE {0xc2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,}

 

//***************************************************************************
// CC1100 STROBE, CONTROL AND STATUS REGSITER

void CC1101_Pin_Init(void);
 /*******************************************************************************
* Function Name  : SPI_ExchangeByte.
* Description    :  
* Input 1        :  SPI  ¿ÕÓà  ÓëSTM8L ³ÌÐòÎÇºÏ£¬
* Input 2        : PtrToBuffer is an u8 pointer to the first word to be transmitted.
* Input 3        : NbOfWords parameter indicates the number of words to be sent.
* Output         : None.
* Return         : TRUE / FALSE.
*******************************************************************************/
INT8U SPI_ExchangeByte(INT8U val);
 


/*================================================================================
Function : CC1101WORInit( )
    Initialize the WOR function of CC1101
INPUT    : None
OUTPUT   : None
================================================================================*/
void  CC1101WORInit(void);
/*
================================================================================
Function : CC1101ReadReg( )
    read a byte from the specified register
INPUT    : addr, The address of the register
OUTPUT   : the byte read from the rigister
================================================================================
*/
INT8U CC1101ReadReg( INT8U addr );
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
void CC1101ReadMultiReg( INT8U addr, INT8U *buff, INT8U size );
/*
================================================================================
Function : CC1101ReadStatus( )
    Read a status register
INPUT    : addr, The address of the register
OUTPUT   : the value read from the status register
================================================================================
*/
INT8U CC1101ReadStatus( INT8U addr );
/*
================================================================================
Function : CC1101SetTRMode( )
    Set the device as TX mode or RX mode
INPUT    : mode selection
OUTPUT   : None
================================================================================
*/
void CC1101SetTRMode( INT8U mode );
/*
================================================================================
Function : CC1101WriteReg( )
    Write a byte to the specified register
INPUT    : addr, The address of the register
           value, the byte you want to write
OUTPUT   : None
================================================================================
*/
void CC1101WriteReg( INT8U addr, INT8U value );
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
void CC1101WriteMultiReg( INT8U addr, INT8U *buff, INT8U size );
/*
================================================================================
Function : CC1101WriteCmd( )
    Write a command byte to the device
INPUT    : command, the byte you want to write
OUTPUT   : None
================================================================================
*/
void CC1101WriteCmd( INT8U command );
/*
================================================================================
Function : CC1101Reset( )
    Reset the CC1101 device
INPUT    : None
OUTPUT   : None
================================================================================
*/
void CC1101Reset( void );
/*
================================================================================
Function : CC1101SetIdle( )
    Set the CC1101 into IDLE mode
INPUT    : None
OUTPUT   : None
================================================================================
*/
void CC1101SetIdle( void );
/*
================================================================================
Function : CC1101ClrTXBuff( )
    Flush the TX buffer of CC1101
INPUT    : None
OUTPUT   : None
================================================================================
*/
void CC1101ClrTXBuff( void );
/*
================================================================================
Function : CC1101ClrRXBuff( )
    Flush the RX buffer of CC1101
INPUT    : None
OUTPUT   : None
================================================================================
*/
void CC1101ClrRXBuff( void );
extern INT8U WaitAck(INT8U *Recv_Buff,INT8U Recv_Len);
void CC1101SendPacket( INT8U *txbuffer, INT8U size );
/*
================================================================================
Function : CC1101GetRXCnt( )
    Get received count of CC1101
INPUT    : None
OUTPUT   : How many bytes hae been received
================================================================================
*/
INT8U CC1101GetRXCnt( void );
/*
================================================================================
Function : CC1101SetAddress( )
    Set the address and address mode of the CC1101
INPUT    : address, The address byte
           AddressMode, the address check mode
OUTPUT   : None
================================================================================
*/
void CC1101SetAddress( INT8U address, INT8U AddressMode);
/*
================================================================================
Function : CC1101SetSYNC( )
    Set the SYNC bytes of the CC1101
INPUT    : sync, 16bit sync 
OUTPUT   : None
================================================================================
*/
void CC1101SetSYNC( INT16U sync );
/*
================================================================================
Function : CC1101RecPacket( )
    Receive a packet
INPUT    : rxBuffer, A buffer store the received data
OUTPUT   : 1:received count, 0:no data
================================================================================
*/
INT8U CC1101RecPacket( INT8U *rxBuffer,INT8U RXLen);//?????????

/**************************************************************

***************************************************************/
INT8U RF_RxHandler(INT8U *B);
INT8U CC1101Init( void );
void CCA_Config(void);

INT8U RF_SoftWOR(void);

#endif   // end of file


