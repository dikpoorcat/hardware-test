#ifndef	__CRC_H
#define	__CRC_H

/*º¯ÊýÉùÃ÷*/
unsigned short RTU_CRC( unsigned char * puchMsg,unsigned short usDataLen );
unsigned char RTU_CS( unsigned char * puchMsg,unsigned short usDataLen );
unsigned long RTU_BYTE(unsigned char *inbuff,unsigned char len);
unsigned char Negation_CS( unsigned char * puchMsg,unsigned short usDataLen );

#endif

