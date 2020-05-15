#ifndef	__Bsp_NFlash_H
#define	__Bsp_NFlash_H
#include "main.h"



/************************************************************************************************************************
* Function Name : void Wrtie_ErasePage(INT32U Addr,INT8U PageNum)                                                                
* Description   : ����ָ��������(1������)ҳ�棬���������ҳ�����������PageNumָ����
* Input         : Addr    ��Ҫ�����ĵ�һ��ҳ��ĵ�ַ
*                 PageNum : Ҫ�������ܵ�ҳ������
*                 
* Return        : None
*
* Author        :                       
* Date First Issued: ��־˴��2018��1��22�մ���������             E-Mail:11207656@qq.com
* Version:  V1.0
*************************************************************************************************************************/
void Wrtie_ErasePage(INT32U Addr,INT8U PageNum);


/************************************************************************************************************************
* Function Name : void Wrtie_NFlashNoErase(INT32U Addr,INT32U *Buff,INT16U Len)                                                             
* Description   : ��Flash��ָ����ַ��д��ָ�����ȵ�����,������������
* Input         : Addr  ��ָ��д��ĵ�ַ
*                 pBuff : Ҫ��д������� 
*                 Len   : д�볤��
*                 
* Return        : None
*
* Author        :                       
* Date First Issued: ��־˴��2018��1��22�մ���������             E-Mail:11207656@qq.com
* Version:  V1.0
*************************************************************************************************************************/
void Wrtie_NFlashNoErase(INT32U Addr,INT32U *pBuff,INT16U Len);


/************************************************************************************************************************
* Function Name : void Wrtie_NFlash(INT32U Addr,INT32U *pBuff,INT16U Len)                                                            
* Description   : ��Flash��ָ��(��ȷ��ҳ)��ַ��д��ָ�����ȵ�����,����������. ע�⣺���ָ����Addr����ҳ�ף���ᱻҳ��ַ"0x800" ��Ĩ������ҳ��
* Input         : Addr  ��ָ��д��ĵ�ַ
*                 pBuff : Ҫ��д������� 
*                 Len   : д�볤��
*                 
* Return        : None
*
* Author        :                       
* Date First Issued: ��־˴��2018��1��22�մ���������             E-Mail:11207656@qq.com
* Version:  V1.0
*************************************************************************************************************************/
void Wrtie_NFlash(INT32U Addr,INT32U *pBuff,INT16U Len);


/************************************************************************************************************************
* Function Name : void Read_NFlash(INT32U Addr,INT32U *Buff,INT16U Len)                                                         
* Description   : ��Flash��ָ����ַ����ȡָ�����ȵ����ݵ��β�pBuffָ���ĵ�ַ��ȥ
* Input         : Addr  ��ָ����ȡ�ĵ�ַ
*                 pBuff : �������ݵĴ�ŵ�ַ 
*                 Len   : ��ȡ����
*                 
* Return        : ͨ���β�pBuff����
*
* Author        :                       
* Date First Issued: ��־˴��2018��1��22�մ���������             E-Mail:11207656@qq.com
* Version:  V1.0
*************************************************************************************************************************/
void Read_NFlash(INT32U Addr,INT32U *Buff,INT16U Len);

#endif

