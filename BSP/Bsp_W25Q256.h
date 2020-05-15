#ifndef __W25QXX_H
#define __W25QXX_H	
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_rcc.h"
#include "bsp_spi.h"
#include "Bsp_fm.h"       // zzs add this include
#include "bsp_WDG.h"      // zzs add this include
#include "SysConfigVC.h"  // zzs add this include
#include "delay.h"


#define  W25QXX_TYPE W25Q256	// 默认是W25Q256

#define  W25Q_Page_Size 256            // zzs add this
#define  ADS            0x00010000     // zzs add this 


//W25X系列/Q系列芯片列表	   
//W25Q80  ID  0XEF13
//W25Q16  ID  0XEF14
//W25Q32  ID  0XEF15
//W25Q64  ID  0XEF16	
//W25Q256 ID  0XEF17	
#define W25Q80 	0XEF13 	
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128	0XEF17
#define W25Q256	0XEF18
 	

#define W25RST_PIN       GPIO_Pin_5//复位
#define W25RST_Port      GPIOC

#define W25CS_PIN       GPIO_Pin_0//使能
#define W25CS_Port      GPIOB

#define W25CLK_PIN       GPIO_Pin_4
#define W25CLK_Port      GPIOC

#define W25MOSI_PIN       GPIO_Pin_7
#define W25MOSI_Port      GPIOA

#define W25MISO_PIN       GPIO_Pin_5
#define W25MISO_Port      GPIOA

#define W25CS_H()        GPIO_SetBits(W25CS_Port, W25CS_PIN)
#define W25CS_L()        GPIO_ResetBits(W25CS_Port, W25CS_PIN)
 
#define W25RST_H()       GPIO_SetBits(W25RST_Port, W25RST_PIN)
#define W25RST_L()       GPIO_ResetBits(W25RST_Port, W25RST_PIN)

#define W25SCK_H()      GPIO_SetBits(W25CLK_Port, W25CLK_PIN)
#define W25SCK_L()      GPIO_ResetBits(W25CLK_Port, W25CLK_PIN)
 
#define W25MOSI_H()      GPIO_SetBits(W25MOSI_Port, W25MOSI_PIN)
#define W25MOSI_L()      GPIO_ResetBits(W25MOSI_Port, W25MOSI_PIN)

#define W25MISO()        GPIO_ReadInputDataBit(W25MISO_Port,W25MISO_PIN)



////////////////////////////////////////////////////////////////////////////////// 
//指令表
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 

//#define W25X_ReadStatusReg		0x05   // zzs commented it

#define W25X_ReadStatusReg1		0x05       // zzs add this define
#define W25X_ReadStatusReg2		0x35       // zzs add this define
#define W25X_ReadStatusReg3		0x15       // zzs add this define

//#define W25X_WriteStatusReg		0x01   // zzs commented it

#define W25X_WriteStatusReg1	0x01       // zzs add this define
#define W25X_WriteStatusReg2	0x31       // zzs add this define
#define W25X_WriteStatusReg3	0x11       // zzs add this define

#define W25X_ReadData			0x03
#define W25X_ReadData_With4BytesAddr 0x13      // zzs add this define

#define W25X_FastReadData		0x0B 
#define W25X_FastReadData_With4BytesAddr 0x0C  // zzs add this define

#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 

#define W25X_Enable4ByteAddr    0xB7   // zzs add this define
#define W25X_QuitTo3ByteAddr    0xE9   // zzs add this define




extern unsigned char W25QXX_BUFFER[4096];
extern INT8U WQ256_Flag;

INT8U W25QXX_Init(INT8U Task_Num);
unsigned short  W25QXX_ReadID(void);  	    		// 读取FLASH ID
unsigned int W25QXX_ReadSR(void);        		    // // zzs modified it like this
void W25QXX_Write_SR(unsigned char WriteStatusRegX,unsigned char Value);		// zzs modified it like this
void W25QXX_EnterOrQuit_4Bytes_AddrMode(unsigned char Cmd_Byte);
void W25QXX_Write_Enable(void);  		// 写使能 
void W25QXX_Write_Disable(void);		// 写保护
void W25QXX_Write_NoCheck(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short NumByteToWrite) ;
void W25QXX_Read(unsigned int ReadAddr,unsigned char* pBuffer,unsigned short NumByteToRead) ;   //读取flash
void W25QXX_Write(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short  NumByteToWrite) ;//写入flash
void W25QXX_Write_Page(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short NumByteToWrite);
void W25QXX_Erase_Chip(void);    	  	//整片擦除
void W25QXX_Erase_Sector(unsigned int Dst_Addr);	//扇区擦除
void W25QXX_Wait_Busy(void);           	//等待空闲
void W25QXX_PowerDown(void);        	//进入掉电模式
void W25QXX_WAKEUP(void);				//唤醒
void W25Q256_LowPower(INT8U Task_Num);
INT8U W25QXX_Read_By_Sector(INT8U *OutBuff,INT32U Sector_Index,INT8U Count)	;
INT8U W25QXX_Write_By_Sector(INT8U *InBuff,INT32U Sector_Index,INT8U Count) ;
#endif
















