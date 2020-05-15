/*****************************************Copyright(C)******************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: NumToAscii.c
**创    建    人: 杜颖成
**创  建  日  期: 2020.04.26
**最  新  版  本: V1.0
**描          述: 数值转换为ASCII。
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
#include "NumToAscii.h"





/*******************************************************************
函数名：INT8U *ScanAsicc(INT8U *InR,INT8U *Asicc,INT8U Asicclen)
功能：  搜索指定的字符串，并指向字符串后一个
*********************************************************************/
INT8U *ScanAsicc(INT8U *InR,INT16U inRlen,INT8U *Asicc,INT8U Asicclen)
{
	INT16U i = 0;	
	INT8U *pRet = InR;
	
	for(i = 0; i < inRlen-Asicclen; i++)
	{
		if (!(memcmp(pRet,Asicc,Asicclen)))
		{			 	 
			return pRet + Asicclen;
		}
		pRet++;
	}
	return 0;
}

/*******************************************************************************
* Function Name : INT8U INT8UBCDToAscii(INT8U InData,INT8U *pOut)
* Description   : INT8U的BCD码格式数据转换到Ascii码（这个函数只输出A~F,大写格式的Ascii字符）
*
* Input         : InData : 被转换的BCD格式的数据
*
* Return        : 形参返回 ：pOut : 转换后的Ascii码格式数据
*                 显式返回 ：2 ：2位宽度
*******************************************************************************/
INT8U INT8UBCDToAscii(INT8U InData,INT8U *pOut)
{
	INT8U Temp = 0; 
	INT8U Temp1 = InData; 
	
	Temp = (Temp1>>4&0x0f);
	if(Temp <= 9) pOut[0] = Temp + 0x30;
	else
	{	
		pOut[0] = (Temp - 10) + 'A';
	}
	Temp = (Temp1&0x0f);
	if(Temp <= 9) pOut[1] = Temp + 0x30;
	else
	{	
		pOut[1] = (Temp -10) + 'A';
	}
	return 2;
}

/*******************************************************************************
* Function Name : INT8U INT16UHexToAscii(INT16U InData,INT8U *pOut)
* Description   : INT16U的Hex格式数据转换到Ascii
*
* Input         : InData : 被转换的字符(ASCII字符)
*
* Return        : 形参返回 ：pOut : 转换后的Ascii码串
*                 显式返回 ：5 ：5位宽度
*                           4 ：4位宽度
*                           3 ：3位宽度
*                           2 ：2位宽度
*                           1 ：1位宽度
*******************************************************************************/
INT8U INT16UHexToAscii(INT16U InData,INT8U *pOut)
{
	INT8U Temp[5] = {0};
	INT8U gTemp = 0;
	INT8U i = 0;
	
	for(i = 4; i != 0xff; i--)
	{
		gTemp = InData%10;														//个位
		Temp[i] = gTemp;
		InData -= gTemp;
		InData = InData/10;														//去掉个位
	}
	
	if(Temp[0])  
	{
		pOut[4] = Temp[4] + 0x30;
		pOut[3] = Temp[3] + 0x30;
		pOut[2] = Temp[2] + 0x30;
		pOut[1] = Temp[1] + 0x30;
		pOut[0] = Temp[0] + 0x30;
		return 5;
	}
	
	if(Temp[1])
	{				 
		pOut[3] = Temp[4] + 0x30;
		pOut[2] = Temp[3] + 0x30;
		pOut[1] = Temp[2] + 0x30;
		pOut[0] = Temp[1] + 0x30;
		return 4;
	}
	
	if(Temp[2])
	{				 
		pOut[2] = Temp[4] + 0x30;
		pOut[1] = Temp[3] + 0x30;
		pOut[0] = Temp[2] + 0x30;
		return 3;
	}
	if(Temp[3])
	{				 
		pOut[1] = Temp[4] + 0x30;
		pOut[0] = Temp[3] + 0x30;
		return 2;
	}
			 
	pOut[0] = Temp[4] + 0x30;

	return 1;
}

/*******************************************************************************
* Function Name : void INT8UHexToAscii(INT8U InData,TCHAR *pOut)
* Description   : INT8U的hex数字转换到Ascii码
*
* Input         : InData : 被转化的数字
*
* Return        : 形参返回 ：pOut : 转换后的Ascii码格式数据
*******************************************************************************/
void INT8UHexToAscii(INT8U InData,TCHAR *pOut)
{
	pOut[0]=InData/10+0x30;
	pOut[1]=InData%10+0x30;
}

/**********************************************************************
名称：void FloatToStr(double fda,char *pString,INT8U dNum)
功能：浮点型转字符串
入参：double fda, 浮点数；char *pString, 输出数组；INT8U dNum，指定长度
出参：无
***********************************************************************/
void FloatToStr(double fda,char *pString,INT8U dNum)
{
    INT8U i;
    INT8U negative=0;    //负数标志位
    INT8U X999        = 0;        //小数部分四舍五入进位标志
    INT8U XtoZ        = 0;        //小数到整数的进位标志
    INT8U intLen=5;
    INT8U cdat[6]={0};        //分部分时的字符串
    INT8U whole[18]={0};    //整个数的字符串,其中留多一位为0x00    

    int ida;                //整数部分
    double dec;                //小数部分
    
    if (fda < 0){                //若为负数取绝对值
        fda = -fda;
        negative = 1;    
    }
    ida = (int) (fda) ;
    dec = fda - ida;
    
///////////////////小数部分转换//////////////    
    if (dNum >= 6)            //小数最多显示5位
        dNum = 5;
    switch (dNum +1){
        case 6:{
            cdat[5] = (char)  (((long) (dec *1000000l))%10);    //0.0000001位    
            whole[15+ 6-dNum] = cdat[5] + 0x30;    
            //四舍五入算法
            if (X999 == 1){
                if (whole[15+ 6-dNum] < '9'){        //小于9就加1
                    whole[15+ 6-dNum] += 1;    
                    X999 = 0;
                }else{            //否则继续进位,本位置0
                    whole[15+ 6-dNum] = '0';
                }                    
            }
            
            if ( dNum==5){                
                if (whole[15+ 6-dNum] >= '5')
                    X999 = 1;
                whole[15+ 6-dNum] = 0x00;
            }
            //////////////////////////
            
        }
        case 5:{
            cdat[4] = (char)  (((long) (dec *100000l))%10);        //0.000001位    
            whole[15+ 5-dNum] = cdat[4] + 0x30;    
            //四舍五入算法
            if (X999 == 1){
                if (whole[15+ 5-dNum] < '9'){        //小于9就加1
                    whole[15+ 5-dNum] += 1;    
                    X999 = 0;
                }else{            //否则继续进位,本位置0
                    whole[15+ 5-dNum]  = '0';
                }                    
            }
            
            if ( dNum==4){                
                if (whole[15+ 5-dNum] >= '5')
                    X999 = 1;
                whole[15+ 5-dNum] = 0x00;
            }
            //////////////////////////
    
        }
        case 4:{
            cdat[3] = (char)  (((long) (dec *10000l))%10);        //0.00001位
            whole[15+ 4-dNum] = cdat[3] + 0x30;        
            //四舍五入算法
            if (X999 == 1){
                if (whole[15+ 4-dNum] < 0x39){        //小于9就加1
                    whole[15+ 4-dNum] += 1;    
                    X999 = 0;
                }else{            //否则继续进位,本位置0
                    whole[15+ 4-dNum]  = '0';
                }                    
            }
            
            if ( dNum==3){                
                if (whole[15+ 4-dNum] >= '5')
                    X999 = 1;
                whole[15+ 4-dNum] = 0x00;
            }
            //////////////////////////

        }
        case 3:    {
            cdat[2] = (char)  (((long) (dec *1000l))%10);            //0.001位
            whole[15+ 3-dNum] = cdat[2] + 0x30;        
            //四舍五入算法
            if (X999 == 1){
                if (whole[15+ 3-dNum] < 0x39){        //小于9就加1
                    whole[15+ 3-dNum] += 1;    
                    X999 = 0;
                }else{            //否则继续进位,本位置0
                    whole[15+ 3-dNum]  = '0';
                }
            }
            if ( dNum==2){                
                if (whole[15+ 3-dNum] >= '5')
                    X999 = 1;
                whole[15+ 3-dNum] = 0x00;
            }
            //////////////////////////
            
        }
        case 2:{
            cdat[1] = (char)  (((long) (dec *100l))%10);            //0.01位
            whole[15+ 2-dNum] = cdat[1] + 0x30;    
            //四舍五入算法
            if (X999 == 1)    {
                if (whole[15+ 2-dNum] < 0x39){        //小于9就加1
                    whole[15+ 2-dNum] += 1;
                    X999 = 0;
                }else{            //否则继续进位,本位置0
                    whole[15+ 2-dNum]  = '0';
                }
            }
            if ( dNum==1){                
                if (whole[15+ 2-dNum] >= '5')
                    X999 = 1;
                whole[15+ 2-dNum] = 0x00;
            }
            //////////////////////////
            
        }    
        case 1:{
            cdat[0] = (char)  (((long) (dec *10l))%10);                //0.1位
            whole[15+ 1-dNum] = cdat[0] + 0x30;    
            //四舍五入算法
            if (X999 == 1){
                if (whole[15+ 1-dNum] < 0x39)
                    whole[15+ 1-dNum] += 1;    
                else{
                    XtoZ = 1;
                    whole[15+ 1-dNum] = '0';    
                }
                X999 = 0;
            }
            
            if ( dNum==0){                
                if (whole[15+ 1-dNum] >= '5')
                    XtoZ = 1;
                whole[15+ 1-dNum] = 0x00;
            }
            /////////////////////////
            
        }            
    }
    
/////////////////////添加小数点////////////////    
    whole[15 - dNum] = '.' ;
    
///////////////////整数部分转换//////////////    
    cdat [0] = (char)(ida / 10000 ) ;
    cdat [1] = (char)((ida % 10000) /1000);
    cdat [2] = (char)((ida % 1000) /100);
    cdat [3] = (char)((ida % 100) /10);
    cdat [4] = (char)((ida % 10) /1);
    for (i=0;i<5;i++){                        //转换成ASCII码
        cdat[i] = cdat[i] + 48;
    }
    
    //四舍五入算法,整数部分(未完)
    if (XtoZ == 1){
        if (cdat[4] < '9'){                //个位小于9
            cdat[4] += 1;            
        }else{
            cdat[4] = '0';
            if (cdat[3] < '9'){            //十位小于9
                cdat[3] += 1;
            }else{
                cdat[3] = '0';
                if (cdat[2] < '9'){        //百位小于9
                    cdat[2] += 1;
                }else{
                    cdat[2] = '0';
                    if (cdat[1] < '9'){    //千位小于9
                        cdat[1] += 1;
                    }else{
                        cdat[1] = '0';
                        cdat[0] += 1;        //万位加1
                    }
                }                
            }
        }
        XtoZ = 0;
    }
    
    ////////////////////////////////////////////////////    
    if (cdat[0] == '0'){
        intLen = 4;
        if (cdat[1] == '0'){
            intLen = 3;
            if (cdat[2] == '0'){
                intLen = 2;
                if (cdat[3] == '0')
                    intLen = 1;            
            }
        }
    }
    
    for (i=0;i<5;i++){
        whole[10 + i - dNum] = cdat[i];        
    }        
///////////////////////拼合符点数/////////////////////////////////    
    if (negative == 1){
        whole [ 14 - intLen - dNum] = '-';
        for ( i=(14 - intLen - dNum) ;i<19; i++){
            *pString = whole[i];        
            pString ++;
        }
    }else{
        for ( i=(15 - intLen - dNum) ;i<19; i++){
            *pString = whole[i];    
            pString ++;
        }        
    }    
    
}
