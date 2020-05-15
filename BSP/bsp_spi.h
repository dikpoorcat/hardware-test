#ifndef __bsp_spi_H
#define __bsp_spi_H
#include "main.h"




/*º¯ÊýÉùÃ÷*/
void SPI_1_Init(void);
void BSP_SoftSpiSend(INT8U val);
INT8U BSP_SoftSpiRece(INT8U val);
INT8U  SPI_BufferSend(const INT8U *PtrToBuffer, INT32U Len);
INT8U SPI_BufferReceive(INT8U *PtrToBuffer, INT32U Len);

#endif

