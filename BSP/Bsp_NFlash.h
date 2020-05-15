#ifndef	__Bsp_NFlash_H
#define	__Bsp_NFlash_H
#include "main.h"



/************************************************************************************************************************
* Function Name : void Wrtie_ErasePage(INT32U Addr,INT8U PageNum)                                                                
* Description   : 擦除指定数量的(1个以上)页面，具体擦除的页面数量有入参PageNum指定。
* Input         : Addr    ：要擦除的第一个页面的地址
*                 PageNum : 要擦除的总的页面数量
*                 
* Return        : None
*
* Author        :                       
* Date First Issued: 赵志舜于2018年1月22日创建本函数             E-Mail:11207656@qq.com
* Version:  V1.0
*************************************************************************************************************************/
void Wrtie_ErasePage(INT32U Addr,INT8U PageNum);


/************************************************************************************************************************
* Function Name : void Wrtie_NFlashNoErase(INT32U Addr,INT32U *Buff,INT16U Len)                                                             
* Description   : 向Flash的指定地址，写入指定长度的数据,不带擦除功能
* Input         : Addr  ：指定写入的地址
*                 pBuff : 要被写入的数据 
*                 Len   : 写入长度
*                 
* Return        : None
*
* Author        :                       
* Date First Issued: 赵志舜于2018年1月22日创建本函数             E-Mail:11207656@qq.com
* Version:  V1.0
*************************************************************************************************************************/
void Wrtie_NFlashNoErase(INT32U Addr,INT32U *pBuff,INT16U Len);


/************************************************************************************************************************
* Function Name : void Wrtie_NFlash(INT32U Addr,INT32U *pBuff,INT16U Len)                                                            
* Description   : 向Flash的指定(精确到页)地址，写入指定长度的数据,带擦除功能. 注意：如果指定的Addr不在页首，则会被页地址"0x800" “抹除”到页首
* Input         : Addr  ：指定写入的地址
*                 pBuff : 要被写入的数据 
*                 Len   : 写入长度
*                 
* Return        : None
*
* Author        :                       
* Date First Issued: 赵志舜于2018年1月22日创建本函数             E-Mail:11207656@qq.com
* Version:  V1.0
*************************************************************************************************************************/
void Wrtie_NFlash(INT32U Addr,INT32U *pBuff,INT16U Len);


/************************************************************************************************************************
* Function Name : void Read_NFlash(INT32U Addr,INT32U *Buff,INT16U Len)                                                         
* Description   : 从Flash的指定地址，读取指定长度的数据到形参pBuff指定的地址中去
* Input         : Addr  ：指定读取的地址
*                 pBuff : 读出数据的存放地址 
*                 Len   : 读取长度
*                 
* Return        : 通过形参pBuff返回
*
* Author        :                       
* Date First Issued: 赵志舜于2018年1月22日创建本函数             E-Mail:11207656@qq.com
* Version:  V1.0
*************************************************************************************************************************/
void Read_NFlash(INT32U Addr,INT32U *Buff,INT16U Len);

#endif

