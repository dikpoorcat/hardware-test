#ifndef	__NumToAscii_H
#define	__NumToAscii_H
#include "main.h"






/*º¯ÊýÉùÃ÷*/
INT8U *ScanAsicc(INT8U *InR,INT16U inRlen,INT8U *Asicc,INT8U Asicclen);
INT8U INT8UBCDToAscii(INT8U InData,INT8U *pOut);
INT8U INT16UHexToAscii(INT16U InData,INT8U *pOut);
void Int8uHexToAscii(INT8U InData,INT8U *pOut);
void FloatToStr(double fda,char *pString,INT8U dNum);

#endif
