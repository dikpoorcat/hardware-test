//对芯片内部FLASH进行读写操作
#include "Bsp_NFlash.h"

 


/* --------------------Private functions------------------------------------------------------*/
/************************************************************************************************************************
* Function Name : void Wrtie_ErasePage(INT32U Addr,INT8U PageNum)                                                                
* Description   : 擦除指定数量的(一个以上)页面，具体擦除的页面数量有入参PageNum指定。
* Input         : Addr    ：要擦除的第一个页面的地址
*                 PageNum : 要擦除的总的页面数量
*                 
* Return        : None
*************************************************************************************************************************/
void Wrtie_ErasePage(INT32U Addr,INT8U PageNum)
{
	INT32U Flash_Destination = 0;
  
	//if(PageNum==0) PageNum=1;       // 这个是做了参数检查了，地址的合法性是否也需要检查一下呢，这里欠严谨
	if(PageNum==0) return;
		
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);//清除标志位
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
* Description   : 向Flash的指定地址，写入指定长度的数据,不带擦除功能
* Input         : Addr  ：指定写入的地址
*                 pBuff : 要被写入的数据 
*                 Len   : 写入长度
*                 
* Return        : None
*************************************************************************************************************************/
void Wrtie_NFlashNoErase(INT32U Addr,INT32U *pBuff,INT16U Len)
{
	INT32U Flash_Destination = 0;
	
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);  // 清除标志位
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
* Description   : 向Flash的指定(精确到页)地址，写入指定长度的数据,带擦除功能. 注意：如果指定的Addr不在页首，则会被页地址"0x800" “抹除”到页首
* Input         : Addr  ：指定写入的地址
*                 pBuff : 要被写入的数据 
*                 Len   : 写入长度
*                 
* Return        : None
*************************************************************************************************************************/
void Wrtie_NFlash(INT32U Addr,INT32U *pBuff,INT16U Len)
{
	INT32U Flash_Destination = 0;

	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); // 清除标志位
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
* Description   : 从Flash的指定地址，读取指定长度的数据到形参pBuff指定的地址中去
* Input         : Addr  ：指定读取的地址
*                 pBuff : 读出数据的存放地址 
*                 Len   : 读取长度
*                 
* Return        : 通过形参pBuff返回
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
