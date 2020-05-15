//��оƬ�ڲ�FLASH���ж�д����
#include "Bsp_NFlash.h"

 


/* --------------------Private functions------------------------------------------------------*/
/************************************************************************************************************************
* Function Name : void Wrtie_ErasePage(INT32U Addr,INT8U PageNum)                                                                
* Description   : ����ָ��������(һ������)ҳ�棬���������ҳ�����������PageNumָ����
* Input         : Addr    ��Ҫ�����ĵ�һ��ҳ��ĵ�ַ
*                 PageNum : Ҫ�������ܵ�ҳ������
*                 
* Return        : None
*************************************************************************************************************************/
void Wrtie_ErasePage(INT32U Addr,INT8U PageNum)
{
	INT32U Flash_Destination = 0;
  
	//if(PageNum==0) PageNum=1;       // ��������˲�������ˣ���ַ�ĺϷ����Ƿ�Ҳ��Ҫ���һ���أ�����Ƿ�Ͻ�
	if(PageNum==0) return;
		
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);//�����־λ
	Flash_Destination=Addr&0xFFFFF800;
	while(PageNum--)
	{
		if(FLASH_ErasePage(Flash_Destination) != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return ;
		}
		Flash_Destination+=0x800;
	}
	FLASH_Lock();
}

/************************************************************************************************************************
* Function Name : void Wrtie_NFlashNoErase(INT32U Addr,INT32U *Buff,INT16U Len)                                                             
* Description   : ��Flash��ָ����ַ��д��ָ�����ȵ�����,������������
* Input         : Addr  ��ָ��д��ĵ�ַ
*                 pBuff : Ҫ��д������� 
*                 Len   : д�볤��
*                 
* Return        : None
*************************************************************************************************************************/
void Wrtie_NFlashNoErase(INT32U Addr,INT32U *pBuff,INT16U Len)
{
	INT32U Flash_Destination = 0;
	
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);  // �����־λ
	Flash_Destination = Addr;
	while(Len--)
	{
		FLASH_ProgramWord(Flash_Destination,*pBuff++);
		Flash_Destination+=4;
	}
	FLASH_Lock();
}

/************************************************************************************************************************
* Function Name : void Wrtie_NFlash(INT32U Addr,INT32U *pBuff,INT16U Len)                                                            
* Description   : ��Flash��ָ��(��ȷ��ҳ)��ַ��д��ָ�����ȵ�����,����������. ע�⣺���ָ����Addr����ҳ�ף���ᱻҳ��ַ"0x800" ��Ĩ������ҳ��
* Input         : Addr  ��ָ��д��ĵ�ַ
*                 pBuff : Ҫ��д������� 
*                 Len   : д�볤��
*                 
* Return        : None
*************************************************************************************************************************/
void Wrtie_NFlash(INT32U Addr,INT32U *pBuff,INT16U Len)
{
	INT32U Flash_Destination = 0;

	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); // �����־λ
	Flash_Destination = Addr & 0xFFFFF800;
	
	if(FLASH_ErasePage(Flash_Destination)!=FLASH_COMPLETE)
	{
		FLASH_Lock();
 		return ;
	}
	while(Len--)
	{
		FLASH_ProgramWord(Flash_Destination,*pBuff++);
		Flash_Destination+=4;
	}
	FLASH_Lock();
}

/************************************************************************************************************************
* Function Name : void Read_NFlash(INT32U Addr,INT32U *Buff,INT16U Len)                                                         
* Description   : ��Flash��ָ����ַ����ȡָ�����ȵ����ݵ��β�pBuffָ���ĵ�ַ��ȥ
* Input         : Addr  ��ָ����ȡ�ĵ�ַ
*                 pBuff : �������ݵĴ�ŵ�ַ 
*                 Len   : ��ȡ����
*                 
* Return        : ͨ���β�pBuff����
*************************************************************************************************************************/
void Read_NFlash(INT32U Addr,INT32U *Buff,INT16U Len)
{
	INT32U Flash_Destination = 0;

	Flash_Destination=Addr;
	while(Len--)
	{
	    *Buff++ = *(vu32*) Flash_Destination;	 
			 Flash_Destination+=4;
	}
}
