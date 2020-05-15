#include "Bsp_W25Q256.h" 


unsigned char W25QXX_BUFFER[4096] = {0};

//INT8U WQ256_Using=0;
 INT8U WQ256_Flag=0;			//bit	7	6 	5 	4 	3 	2 	1 	0		������ռ�ñ�־λ
								//����	X	X	X	X	Wdt	GY	RF	GPRS


/* --------------------Private functions------------------------------------------------------*/
/************************************************************************************************************************
* Function Name : void SPI1_Init(void)                          
* Description   : SPI1 I/O��ģ�⣬���I/O�ڵ����á�
* Input         : None
* Return        : None
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void SPI1_Init(void)
{ 
	GPIO_InitTypeDef GPIO_InitStructure = {0};

	/* Enable SPI1 and GPIOA clocks */
	//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);
	//	W25CS_H();
	GPIO_InitStructure.GPIO_Pin = W25CS_PIN;  // PB0
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(W25CS_Port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = W25RST_PIN; // PC5
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(W25RST_Port, &GPIO_InitStructure);

	W25CS_H();
	W25RST_H();

	SPI_1_Init();
	
	//GPIO_InitStructure.GPIO_Pin = W25CS_PIN;//PB0
	//GPIO_Init(W25CS_Port, &GPIO_InitStructure);

	//GPIO_InitStructure.GPIO_Pin = W25CLK_PIN;
	//GPIO_Init(W25CLK_Port, &GPIO_InitStructure);

	//GPIO_InitStructure.GPIO_Pin = W25MOSI_PIN;
	//GPIO_Init(W25MOSI_Port, &GPIO_InitStructure);

	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	//GPIO_InitStructure.GPIO_Pin = W25MISO_PIN;
	//GPIO_Init(W25MISO_Port, &GPIO_InitStructure);
		

}

////SPI1��дһ�ֽ�����
//unsigned char SPI1_ReadWrite(unsigned char writedat)
//{
//   /* Loop while DR register in not emplty */
//   while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

//   /* Send byte through the SPI1 peripheral */
//   SPI_I2S_SendData(SPI1, writedat);

//   /* Wait to receive a byte */
//   while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

//   /* Return the byte read from the SPI bus */
//   return SPI_I2S_ReceiveData(SPI1);
//}

/************************************************************************************************************************
* Function Name : unsigned char  SPI1_ReadWrite(unsigned char val)                            
* Description   : SPI1�ӿ��϶�дһ���ֽڣ���ע�����ģ��SPI�ӿڣ����Ǳ�׼��SPI�ӿڣ���ˣ�ʹ��������������ر��������ʺϴ���
*                 �����������ٶ���Ҫ��ĳ��ϡ�
*                       
* Input         : val  ��Ҫд���ֵ
* Return        : data ��������ֵ
*                
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
unsigned char  SPI1_ReadWrite(unsigned char val)
{
	unsigned char i = 0;
	unsigned char data = 0;
	unsigned char Rd = 0;

	for(i = 0x80; i != 0;i >>= 1)
	{	
		W25SCK_L();
		if(val&i) W25MOSI_H();   												// ���ݽ���
		else W25MOSI_L();
//		Fmdelay(1);																//4M����û���⣬32Mû��
		W25SCK_H();           	   												// ���������ݱ�д�뵽����  
//		Fmdelay(1);																//4M����û���⣬32Mû��
		Rd = W25MISO();
		if(Rd == 1) 
		{
			data |= i;           
		}
	}
	W25SCK_L();																	//�����Խ�����
	return data;
}

/************************************************************************************************************************
* Function Name : INT8U W25QXX_Init(void)                              
* Description   : ��ʼ��W25QXX�� ��Ҫ�ǣ���ʼ��SPI FLASH��IO�ڣ�ʹ��4�ֽڵ�ַģʽ����ȡоƬ�İ汾�ţ��Դ��ж��Ƿ��ʼ���ɡ�
*                 ��ע��
*                 256���ֽ�Ϊһҳ(page)����16��ҳ��4K bytes���Ϊһ������(Sector)��8������(��128��ҳ)Ϊ1����32k Bytes��
*                 ��С��Block��16����������256��ҳ��Ϊһ����64k Bytes����С��Block��
*                 
*                 W25Q256��3�ֽڵ�ַģʽʱ������Ϊ16M�ֽ�, 4096��Sector����Ļ������Զ����һ���Լ�ȥ�֣��ֲ�Ҳ���������Ժ���
*                          4�ֽڵ�ַģʽʱ������Ϊ32M�ֽڣ�8192��Sector����Ļ������Զ����һ���Լ�ȥ�֣��ֲ�Ҳ���������Ժ���
*                       
* Input         : ��
* Return        : 1 ���ɹ�
*                 0 ��ʧ��
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : /
*************************************************************************************************************************/
INT8U W25QXX_Init(INT8U Task_Num)
{ 
	unsigned short BANBEN = 0;
	unsigned int Status = 0; 
	
	WQ256_Flag|=(1<<Task_Num);								//
	if( WQ256_Flag&~(1<<Task_Num) ) return 1;				//�����������ѳ�ʼ����FM_Flag��0��ֱ�ӷ���
	
	SPI1_Init();
	W25QXX_WAKEUP();
	W25QXX_EnterOrQuit_4Bytes_AddrMode(W25X_Enable4ByteAddr);    // zzs add ,����4�ֽڵ�ַģʽ,��ָ��д���Ӱ����Ƿ���ʧ��λ
	Status = W25QXX_ReadSR();
	if(!(Status&ADS)) return 0;     // 4�ֽڵ�ַģʽ����ʧ��
	
	BANBEN = W25QXX_ReadID();
	if(BANBEN == W25QXX_TYPE) return 1;  // �ɹ�
	else return 0;                       // ʧ��
}  






/************************************************************************************************************************
* Function Name : unsigned int W25QXX_ReadSR(void)                                     
* Description   : ��ȡW25QXX��״̬�Ĵ�����Ĭ�� : 0x00 zzs???��
*                  Bit23    22   21   20  19   18   17   16  |  15   14   13   12   11   10   9    8  |   7   6   5   4   3    2     1    0
*                 HOLD/RST DRV1 DRV0 (R)  (R)  WPS  ADP  ADS    SUS  CMP  LB3  LB2  LB1  (R)  QE  SRP1  SRP0  TB BP3 BP2  BP1  BP0  WEL  BUSY
*                 ��ע: ADP : Power Up Address Mode  ---(δ�г��ģ�����鿴�����ֲᣬ��������Ƚ���Ҫ�ı�עд��һ��)           
*                       SRP0                : Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
*                       TB,BP3,BP2,BP1,BP0 : FLASH����д��������
*                       WEL                : дʹ������
*                       BUSY               : æ���λ(1,æ;0,����)
*                       
* Input         : None
* Return        : None
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
#if 0
unsigned char W25QXX_ReadSR(void)   
{  
	unsigned char byte=0;

	W25CS_L();                               // ʹ��������Ƭѡѡ�У� 
	SPI1_ReadWrite(W25X_ReadStatusReg);      // ���Ͷ�ȡ״̬�Ĵ�������    
	byte = SPI1_ReadWrite(0Xff);             // ��ȡһ���ֽ�  
	W25CS_H();                               // ����������ȡ��Ƭѡ��
	
	return byte;   
} 
#else
unsigned int W25QXX_ReadSR(void)   
{  
	INT8U i = 0;
	unsigned int Stat = 0;
		
	W25CS_L();                               // ʹ��������Ƭѡѡ�У� 
	SPI1_ReadWrite(W25X_ReadStatusReg3);     // ���Ͷ�ȡ״̬�Ĵ���3����    
	Stat = SPI1_ReadWrite(0xff);
	Stat <<=8;
	
	W25CS_H();     // ����������ȡ��Ƭѡ�� //���ֲ�������һ�£��ǵ�����Ƭѡ������,�Բ����꣬���ֲ�զд�ģ�������ô��˾����Ҳ̫���Ͻ��˰�
	for(i = 0; i < 100; i++) {;}
	W25CS_L();                             // ʹ��������Ƭѡѡ�У� 
		
	SPI1_ReadWrite(W25X_ReadStatusReg2);   // ���Ͷ�ȡ״̬�Ĵ���2����  
	Stat |=SPI1_ReadWrite(0xff);
	Stat <<=8;
	
	W25CS_H();     // ����������ȡ��Ƭѡ�� //���ֲ�������һ�£��ǵ�����Ƭѡ������,�Բ����꣬���ֲ�զд�ģ�������ô��˾����Ҳ̫���Ͻ��˰�
	for(i = 0; i < 100; i++) {;}
	W25CS_L();                             // ʹ��������Ƭѡѡ�У�
	
	SPI1_ReadWrite(W25X_ReadStatusReg1);   // ���Ͷ�ȡ״̬�Ĵ���1����  
	Stat |=SPI1_ReadWrite(0xff);
	
	W25CS_H();                             // ����������ȡ��Ƭѡ��
	
	return Stat;   
} 
#endif

/************************************************************************************************************************
* Function Name : void W25QXX_EnterOrQuit_4Bytes_AddrMode(unsigned char Cmd_Byte)                                     
* Description   : W25QXX����4Bytes��ַģʽ
*                 
* Input         : None
* Return        : None
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_EnterOrQuit_4Bytes_AddrMode(unsigned char Cmd_Byte)
{
	unsigned char i = 0;
	W25QXX_Write_Enable();   
	for(i = 0;i < 100; i++) {;}
		
	W25CS_L();                             
	SPI1_ReadWrite(Cmd_Byte);  
	W25CS_H();                             
}



#if 0
/************************************************************************************************************************
* Function Name : void W25QXX_Write_SR(unsigned char sr)                                       
* Description   : дW25QXX״̬�Ĵ���
*                 ��ע��ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д�������������������� zzs??? �����������������������������
* Input         : sr : Ҫд���ֵ
* Return        : None
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Write_SR(unsigned char sr)   
{   
	W25CS_L();                             // ʹ��������Ƭѡѡ�У� 
	SPI1_ReadWrite(W25X_WriteStatusReg);   // ����дȡ״̬�Ĵ�������    
	SPI1_ReadWrite(sr);                    // д��һ���ֽ�  
	W25CS_H();                             // ����������ȡ��Ƭѡ�� 	      
}   
#else
/************************************************************************************************************************
* Function Name : void W25QXX_Write_SR(unsigned char WriteStatusRegX,unsigned char Value)                                   
* Description   : дW25QXX��״̬�Ĵ���, 
*                 ��ע��дʹ��ָ��󣬽�����ʹ�ܣ��뿴�ֲ᣺��The WEL bit must be set prior to every ...��
*                 �����ǣ�ֻ�е���driving /CS high��֮��оƬ�Żᴦ�ڿ�д��״̬��˵��дʹ��ָ���Ҫ��ǰ�����������ȡ�
*                 ��After power-up the device is automatically placed in a write-disabled state with the Status Register 
*                   Write Enable Latch (WEL) set to a 0. A Write Enable instruction must be issued before a Page Program, 
*                   Sector Erase, Block Erase, Chip Erase or Write Status Register instruction will be accepted. After 
*                   completing a program, erase or write instruction the Write Enable Latch (WEL) is automatically cleared 
*                   to a write-disabled state of 0.�� ���������ͷ�ġ�must be issued before...��û��
*
* Input         : WriteStatusRegX   : ����Ҫд������һ��״̬�Ĵ���
*                 Value             : ֵ
* Return        : None
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Write_SR(unsigned char WriteStatusRegX,unsigned char Value)   
{   
	unsigned char i = 0;
	W25CS_L();                             // ʹ��������Ƭѡѡ�У� 
	SPI1_ReadWrite(W25X_WriteEnable);      // ����дʹ��,
	
	W25CS_H();                             // ����������ȡ��Ƭѡ��
	for(i = 0; i < 100; i++) {;}
	W25CS_L();                             // ʹ��������Ƭѡѡ�У�
		
	SPI1_ReadWrite(WriteStatusRegX);       // ����дȡ״̬�Ĵ�������    
	SPI1_ReadWrite(Value);                 // д��һ���ֽ�  
	W25CS_H();                             // ����������ȡ��Ƭѡ�� 	      
}   
#endif

/************************************************************************************************************************
* Function Name : void W25QXX_Write_Enable(void)                                          
* Description   : W25QXXдʹ�ܣ���WEL��λ
* Input         : None
* Return        : None
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Write_Enable(void)   
{
	W25CS_L();                            // ʹ��������Ƭѡѡ�У�   
	SPI1_ReadWrite(W25X_WriteEnable);     // ����дʹ��  
	W25CS_H();                            // ����������ȡ��Ƭѡ��   	      
} 

/************************************************************************************************************************
* Function Name : void W25QXX_Write_Disable(void)	                                            
* Description   : W25QXXд��ֹ����WEL���� 
* Input         : None
* Return        : None
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Write_Disable(void)   
{  
	W25CS_L();                            // ʹ��������Ƭѡѡ�У�  
    SPI1_ReadWrite(W25X_WriteDisable);    // ����д��ָֹ��    
	W25CS_H();                            // ����������ȡ��Ƭѡ�� 	      
} 

/************************************************************************************************************************
* Function Name : unsigned short W25QXX_ReadID(void)	                                            
* Description   : ��ȡоƬID
* Input         : None
*                
* Return        : Temp : = 0XEF13,��ʾоƬ�ͺ�ΪW25Q80  
*                        = 0XEF14,��ʾоƬ�ͺ�ΪW25Q16
*                        = 0XEF15,��ʾоƬ�ͺ�ΪW25Q32
*                        = 0XEF16,��ʾоƬ�ͺ�ΪW25Q64
*                        = 0XEF17,��ʾоƬ�ͺ�ΪW25Q128
*                        = 0XEF18,��ʾоƬ�ͺ�ΪW25Q256
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
unsigned short W25QXX_ReadID(void)
{
	unsigned short Temp = 0;	  

	W25CS_L();	            
	SPI1_ReadWrite(0x90);         // ���Ͷ�ȡID����	    
	SPI1_ReadWrite(0x00); 	    
	SPI1_ReadWrite(0x00); 	    
	SPI1_ReadWrite(0x00);
	
	Temp = SPI1_ReadWrite(0xFF);  
	Temp <<= 8;
	Temp |= SPI1_ReadWrite(0xFF);	 	 
	W25CS_H();

	return Temp;
}   


/************************************************************************************************************************
* Function Name : void W25QXX_Read(unsigned int ReadAddr,unsigned char* pBuffer,unsigned short NumByteToRead)	                                            
* Description   : ��ȡSPI FLASH ,W25Qxx,��ָ����ַ��ʼ��ȡָ�����ȵ�����
* Input         : ReadAddr      : ��ʼ��ȡ�ĵ�ַ(4Bytes Address Mode)
*                 pBuffer       : �������ٵĿռ��ָ�룬���ڽ��ա��洢��������ȡ��������
*                 NumByteToRead : ��ȡ�ĳ���(���65535)
* Return        : None
*
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Read(unsigned int ReadAddr,unsigned char* pBuffer,unsigned short NumByteToRead)	
{ 
	unsigned short i = 0;  
	
	__disable_irq(); // ��ֹ���ж�     // zzs add 2018.3.31
	
	Feed_Dog();	
	
	W25CS_L();	                                      // ʹ��������Ƭѡѡ�У� 
	// SPI1_ReadWrite(W25X_ReadData);                 // ���Ͷ�ȡ����
	SPI1_ReadWrite(W25X_ReadData_With4BytesAddr);     // zzs modified it like this
	SPI1_ReadWrite((unsigned char)((ReadAddr)>>24));  // zzs add this
	SPI1_ReadWrite((unsigned char)((ReadAddr)>>16));  
	SPI1_ReadWrite((unsigned char)((ReadAddr)>>8));   
	SPI1_ReadWrite((unsigned char) ReadAddr ); 
	
	for(i = 0; i < NumByteToRead; i++)
	{ 
		if(i%1024==0) 
		{		
			Feed_Dog();								  //ÿд1K��ιһ�ι�����ֹ���Ź���λ
		}
		pBuffer[i] = SPI1_ReadWrite(0XFF);            // ѭ������������SPI2->DRд��OXFF�Է���ȡ�����ݴ���  
	}
	W25CS_H();	                                      // ����������ȡ��Ƭѡ��     	      
	
	__enable_irq();	// �����ж�	  // zzs add 2018.3.31
}  

/************************************************************************************************************************
* Function Name : void W25QXX_Write_Page(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short NumByteToWrite)                                                            
* Description   : SPI��һҳ(0~65535)��д������256���ֽڵ�����
*                 ��ָ����ַ��ʼд�����256�ֽڵ�����
*                          
* Input         : WriteAddr      ����ʼд��ĵ�ַ(4Bytes Address Mode)
*                 pBuffer        : Ҫд�������
*                 NumByteToWrite : Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���������
*                 
* Return        : None
*
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Write_Page(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short NumByteToWrite)
{
 	unsigned short i = 0;  
	
	__disable_irq(); // ��ֹ���ж�     // zzs add 2018.3.31
	
    W25QXX_Write_Enable();                  // SET WEL     // дʹ��������Ҫһ�������� ��Ƭѡ���� + ����дʹ��ָ�� + Ƭѡ���ߡ�����
	
	W25CS_L();                              // ʹ��������Ƭѡѡ�У� 
    SPI1_ReadWrite(W25X_PageProgram);       // ����дҳ���� 
    SPI1_ReadWrite((unsigned char)((WriteAddr)>>24));  // zzs add this	
    SPI1_ReadWrite((unsigned char)((WriteAddr)>>16));  
    SPI1_ReadWrite((unsigned char)((WriteAddr)>>8));   
    SPI1_ReadWrite((unsigned char)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++)
	{
	   SPI1_ReadWrite(pBuffer[i]);   // ѭ��д�� 
	}		
	W25CS_H();	                     // ����������ȡ��Ƭѡ�� 
	W25QXX_Wait_Busy();				 // �ȴ�д�����
	
	__enable_irq();	// �����ж�	  // zzs add 2018.3.31
} 

/************************************************************************************************************************
* Function Name : void W25QXX_Write_NoCheck(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short NumByteToWrite)                                                             
* Description   : �޼���дSPI FLASH����ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
*                 ����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
*                 (zzs�ܽ��£���֮һ����1ҪдΪ0���У�ok,���ǣ����Flash��ԭʼ�Ķ����Ѿ���0��������Ҫд�����ݣ������λ�����ֲ���0��
*                  ����1�Ļ����ǣ��Բ���û�취д��1�ġ�Flash��д���ƾ��ǣ�����1���ǲ���Ҫʲô�����ģ�ֻ��Ҫ����ҪдΪ0�ĵط�����1дΪ0�Ϳ�)
*                 �����Զ���ҳ����          
* Input         : WriteAddr      ����ʼд��ĵ�ַ(4Bytes Address Mode)
*                 pBuffer        : Ҫд�������
*                 NumByteToWrite : Ҫд����ֽ���(���65535)
* Return        : None
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Write_NoCheck(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short NumByteToWrite)   
{ 			 		 
	unsigned short  pageremain = 0;	
    unsigned char Loop_cnt=0; // zzs add ���������������W25Q������Ķ���
	
	pageremain = 256 - WriteAddr%256;  // ��ҳʣ����ֽ���		 	    
	if(NumByteToWrite <= pageremain) pageremain = NumByteToWrite; // ������256���ֽ�
	while(1)
	{	   
		W25QXX_Write_Page(WriteAddr,pBuffer,pageremain);
		if(NumByteToWrite == pageremain) break;         // д�������
		else  // NumByteToWrite > pageremain
		{
			Loop_cnt++;         // zzs add���������������W25Q������Ķ���
			if(Loop_cnt%4==0)   // zzs add���������������W25Q������Ķ���
			{
				Feed_Dog();						//ÿд1K��ιһ�ι�����ֹ���Ź���λ	
				OSTimeDly(2); // 100ms // zzs add���������������W25Q������Ķ����������ȫ����Ϊ�������ϵͳ�����������ҳдʱ���������CPUռ�ù���ʱ���������������Ȳ������������⣬�����������ó�һ�����	
			}
			
			pBuffer += pageremain;
			WriteAddr += pageremain;	
			NumByteToWrite -= pageremain;			    // ��ȥ�Ѿ�д���˵��ֽ���
			if(NumByteToWrite > 256) pageremain = 256;  // һ��������д��256���ֽ�
			else pageremain = NumByteToWrite; 	        // ����256���ֽ���
		}
	}	    
} 

/************************************************************************************************************************
* Function Name : void W25QXX_Write(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short  NumByteToWrite)                                                          
* Description   : дSPI FLASH ,��ָ����ַ��ʼд��ָ�����ȵ����� ,�ú�������������!
*                
* Input         : WriteAddr      ����ʼд��ĵ�ַ(4Bytes Address Mode)
*                 pBuffer        : Ҫд�������
*                 NumByteToWrite : Ҫд����ֽ���(���65535)
*                 
* Return        : None
*
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Write(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short  NumByteToWrite)   
{ 
	unsigned int secpos = 0;
	unsigned short  secoff = 0;
	unsigned short  secremain = 0;	   
 	unsigned short  i = 0;    
	unsigned char * W25QXX_BUF = W25QXX_BUFFER;	  
   
	Feed_Dog();	
	
 	secpos = WriteAddr/4096;    // ������ַ ,0~8192 for w25Q256 
	secoff = WriteAddr%4096;    // �������ڵ�ƫ��
	secremain = 4096 - secoff;  // ����ʣ��ռ��С   
 	// printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//������
 	if(NumByteToWrite <= secremain) secremain = NumByteToWrite;    // ������4096���ֽ�
	while(1) 
	{	
		W25QXX_Read(secpos*4096,W25QXX_BUF,4096);    // ������������������---OK
		
		OSTimeDly(4); // 200ms  // ���������������W25Q������Ķ����������ȫ����Ϊ�������ϵͳ�����������ҳдʱ���������CPUռ�ù���ʱ���������������Ȳ������������⣬�����������ó�һ�����
		
		for(i = 0;i < secremain; i++)                // У������
		{
			if(W25QXX_BUF[secoff+i] != 0XFF) break;     // ��Ҫ����  	  
		}
		
		if(i < secremain)    // ��Ҫ����
		{
			W25QXX_Erase_Sector(secpos);             // �����������---ERROR!
			
			for(i = 0;i < secremain; i++)	         // ����
			{
				W25QXX_BUF[i + secoff] = pBuffer[i];	  
			}
			
			W25QXX_Write_NoCheck(secpos*4096,W25QXX_BUF,4096);    // д����������---ERROR!  

		}else W25QXX_Write_NoCheck(WriteAddr,pBuffer,secremain);  // д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		
		if(NumByteToWrite == secremain) break;   // д�������
		else             // д��δ����
		{
			secpos++;    // ������ַ��1
			secoff = 0;  // ƫ��λ��Ϊ0 	 

		   	pBuffer += secremain;    // ָ��ƫ��
			WriteAddr += secremain;  // д��ַƫ��	   
		   	NumByteToWrite -= secremain;				  // �ֽ����ݼ�
			if(NumByteToWrite > 4096) secremain = 4096;	  // ��һ����������д����
			else secremain=NumByteToWrite;			      // ��һ����������д����
		}	 
	}	 
}

/************************************************************************************************************************
* Function Name : void W25QXX_Erase_Chip(void)                                             
* Description   : ��������оƬ����ע���ȴ�ʱ�䳬��:��ʮ�룬׼ȷ���,��Ƭ��������ʱ72�룩
* Input         : None
* Return        : None 
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Erase_Chip(void)   
{                                   
	W25QXX_Write_Enable();                 // SET WEL 
	W25QXX_Wait_Busy();   
	W25CS_L();                             // ʹ������   
	SPI1_ReadWrite(W25X_ChipErase);        // ����Ƭ��������  
	W25CS_H();                             // ȡ��Ƭѡ     	      
	W25QXX_Wait_Busy();   				   // �ȴ�оƬ��������
}   

/************************************************************************************************************************
* Function Name : void W25QXX_Erase_Sector(unsigned int Dst_Addr)                                              
* Description   : ����һ����������ע������һ������������ʱ��:150ms��
* Input         : Dst_Addr : ������ַ ����ʵ����������
* Return        : None 
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Erase_Sector(unsigned int Dst_Addr)   
{  	  
	// printf("fe:%x\r\n",Dst_Addr);	  // ����falsh�������,������ 
	
	__disable_irq(); // ��ֹ���ж�     // zzs add 2018.3.31
	
	BSP_WDGFeedDog();                 // zzs add 2018.06.12
	
	Dst_Addr *= 4096;
	W25QXX_Write_Enable();                // SET WEL 	 
	W25QXX_Wait_Busy();   
	W25CS_L();                            // ʹ������   
	SPI1_ReadWrite(W25X_SectorErase);     // ������������ָ�� 
	SPI1_ReadWrite((u8)((Dst_Addr)>>24)); // zzs add this 
	SPI1_ReadWrite((u8)((Dst_Addr)>>16));  
	SPI1_ReadWrite((u8)((Dst_Addr)>>8));   
	SPI1_ReadWrite((u8)Dst_Addr);  
	W25CS_H();                            // ȡ��Ƭѡ     	      
	W25QXX_Wait_Busy();   				  // �ȴ��������
	
	__enable_irq();	// �����ж�	  // zzs add 2018.3.31
} 

/************************************************************************************************************************
* Function Name : void W25QXX_PowerDown(void)                                               
* Description   : �ȴ�W25Q256����
* Input         : None
* Return        : None 
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Wait_Busy(void)   
{   
	// printf("W25QXX_ReadSR()=%x\r\n",W25QXX_ReadSR());
	while( (W25QXX_ReadSR()&0x01) == 0x01) {;}           // �ȴ�BUSYλ���
}  

/************************************************************************************************************************
* Function Name : void W25QXX_PowerDown(void)                                               
* Description   : W25Q256�������ģʽ
* Input         : None
* Return        : None 
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_PowerDown(void)   
{ 
	W25CS_L();                          // ʹ������
	SPI1_ReadWrite(W25X_PowerDown);     // ���͵�������  
	W25CS_H();                          // ȡ��Ƭѡ     	      
	delay_2us_4M(1);                    // �ȴ�TPD	���Ϊ1�ĵ��ε���Լ5.4us������2us��
}

/************************************************************************************************************************
* Function Name : void W25QXX_WAKEUP(void)                                                 
* Description   : ����W25Q256��
* Input         : None
* Return        : None 
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_WAKEUP(void)   
{  
	W25CS_L();                               // ʹ������   
	SPI1_ReadWrite(W25X_ReleasePowerDown);   // send W25X_PowerDown command 0xAB    
	W25CS_H();                               // ȡ��Ƭѡ     	      
	delay_2us_4M(1);                         // �ȴ�TRES1	���Ϊ1�ĵ��ε���Լ5.4us������2us��
}   

/************************************************************************************************************************
* Function Name : INT8U WQ256_Test(void)                                                  
* Description   : ����W25Q256��
*                 оƬ����Ϊ256M Bits,32M Bytes,��3�ֽڵ�ַģʽ����4�ֽڵ�ַģʽ��zzs note:ע���������ǰ�Ƭ��(3�ֽڵ�ַ��
*                 16M BytesѰַ��Χ)������ȫƬ�ã�4�ֽڵ�ַģʽ��32M Bytes����
*                 
*                 ע��W25Q256��SPI-Flash, 256Bytes��ҳ����С������λ��16ҳ��4kBytes����Ҳ�ɰ�128ҳΪһ�飨32kBytes�����в�����
*                     Ҳ�ɰ�256ҳΪһ�飨64kBytes�����в�����Ҳ�ɰ���Ƭ(entire chip erase)
* Input         : None
* Return        :  0 ������ʧ��
*                  1 �����Գɹ�
*
* History :                               
* First Issued  : ��־˴��2018��2��7�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
#define zzs_Test_W25Q 0
INT8U FMTESTBUFF[256] = {0};
INT8U WQ256_Test(void)
{
	INT16U i = 0;
	INT8U ERR = 0;
	
	ERR = W25QXX_Init(5);
	if(ERR == 0) return 0;
	
	#if zzs_Test_W25Q
	W25QXX_Erase_Sector(0);          // ������0��
	W25QXX_Read(0,FMTESTBUFF,256);   // ��������ʼ�������
	for(i = 0; i < 256; i++)
	{
		FMTESTBUFF[i]=i;    // ��仺��
	}
	
	W25QXX_Write_NoCheck(0,FMTESTBUFF,256);   // ��У�飬����������дһ��
	for(i = 0; i < 256; i++) FMTESTBUFF[i]=0; // �������
	W25QXX_Read(0,FMTESTBUFF,256);            // ��������һ��д�����      // ���д�����ܹ���ȫ��ȷ��
	
	for(i = 0; i < 256; i++)
	{
		FMTESTBUFF[i] =  255 - i;             // ����������ǰ��ߵ�����,  ������У�飬�޲�������д��Ҳ����д�ϴ�ÿ���ֽ���������Ϊ1��λ�����������ֻҪ��ȫ0����OK��  
		                                      // Ҳ������ø������Ĳ���ֵ�����磺0xAA�����ȫ������;���Ǻ����������ֵ����һЩ���ҵ�ֵ��
	}
	W25QXX_Write_NoCheck(0,FMTESTBUFF,256);       // ��У�飬����������д�ڶ���
	for(i = 0; i < 256; i++) FMTESTBUFF[i] =0xFF; // ͿĨ����
	W25QXX_Read(0,FMTESTBUFF,256);                // �������ڶ���д�����   // ����ȫΪ0�Ͷ��ˡ�
	
	for(i = 0; i < 256; i++)
	{
		FMTESTBUFF[i]=i;    // ��仺��
	}
	W25QXX_Write(0,FMTESTBUFF,256);           // ������д��һ��
	for(i = 0; i < 256; i++) FMTESTBUFF[i]=0; // �������
	W25QXX_Read(0,FMTESTBUFF,256);            // ������������д�����
	
	W25QXX_Erase_Sector(0);         // ������0��
	W25QXX_Read(0,FMTESTBUFF,256);  // ��������0��ĵĲ������
	
	W25QXX_Erase_Chip();                  // ��Ƭ����
	
		#if 0
		// W25QXX_Erase_Sector(Scetor_No);    // �����
		W25QXX_Erase_Chip();                  // ��Ƭ����
			
		for(j = 0; j < 65536; j++)  // ǰ��Ƭ
		{
			W25QXX_Read(j*256,FMTESTBUFF,256);
			for(i = 0;i < 256;i++)
			{
				FMTESTBUFF[i] = i;
			}
			W25QXX_Write_NoCheck(j*W25Q_Page_Size,FMTESTBUFF,256);   // Write
			
			for(i = 0;i < 256; i++)
			{
				FMTESTBUFF[i] = 0;
			}
			
			W25QXX_Read(j*W25Q_Page_Size,FMTESTBUFF,256);            // Read to check
		}
		#else

		// W25QXX_Erase_Sector(Scetor_No);    // �����
		W25QXX_Erase_Chip();                  // ��Ƭ����

		for(j = 65536; j < 131072; j++)       // ���Ƭ
		{
			W25QXX_Read(j*256,FMTESTBUFF,256);         // zzs note,
			for(i = 0;i < 256; i++)
			{
				FMTESTBUFF[i] = i;
			}
			
			W25QXX_Write_NoCheck(j*W25Q_Page_Size,FMTESTBUFF,256);   // Write
			
			for(i = 0;i < 256; i++)
			{
				FMTESTBUFF[i] = 0;
			}
			
			W25QXX_Read(j*W25Q_Page_Size,FMTESTBUFF,256);            // Read to check
		}
		#endif
	
	#endif
	
	W25QXX_Erase_Sector(0);          // ������0��
	W25QXX_Read(0,FMTESTBUFF,256);   // �ӷ�ҳ��ַ�϶�����û��̫������ģ�
	for(i = 0; i < 256; i++)
	{
		FMTESTBUFF[i] = i;
	}
	
	W25QXX_Write_NoCheck(0,FMTESTBUFF,256);     // ���Ǵӷ�ҳ��ַ��д���Ǿ͵�ע�������Բ�ע��ͳ�����������������������
	
	for(i = 0; i < 256; i++) FMTESTBUFF[i]=0;
	
	W25QXX_Read(0,FMTESTBUFF,256);  
	for(i = 0; i < 256; i++)
	{
		if(FMTESTBUFF[i] != i) return 0;           // W25Qxx����ʧ��
	}
    
	W25QXX_Erase_Chip();    // ������ɺ����оƬ
	
	return 1;  //W25Qxx ���Գɹ�
}



/******************************************************************************* 
* Function Name  : void W25Q256_LowPower(void)
* Description    : W25Q256����͹��ģ�������ӦIO�����͹��Ĵ���
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/
void W25Q256_LowPower(INT8U Task_Num)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;	
	
	WQ256_Flag&=~(1<<Task_Num);							//�嵱ǰ����bit	
	if(WQ256_Flag) return;								//����������ռ�ã�FM_Flag��0��ֱ�ӷ���
	
	/*��δ��ʼ�����޷�����ػ�״̬*/
	W25QXX_PowerDown();														//����ػ�״̬
	

	/*Ƭѡ�ڡ�д������ģ������*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;							//ģ������
	
	GPIO_InitStructure.GPIO_Pin = W25CS_PIN;								//Ƭѡ��
	GPIO_Init(W25CS_Port, &GPIO_InitStructure);								//
	
//	GPIO_InitStructure.GPIO_Pin = W25WP_PIN;								//д������δʹ��
//	GPIO_Init(W25WP_Port, &GPIO_InitStructure);								//	
	
	
	/*SPI����������������*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;						//�������
	
	GPIO_InitStructure.GPIO_Pin = W25MOSI_PIN;								//MOSI
	GPIO_ResetBits(W25MOSI_Port, W25MOSI_PIN);								//����
	GPIO_Init(W25MOSI_Port, &GPIO_InitStructure);							//
	
	GPIO_InitStructure.GPIO_Pin = W25MISO_PIN;								//MISO
	GPIO_ResetBits(W25MISO_Port, W25MISO_PIN);								//����
	GPIO_Init(W25MISO_Port, &GPIO_InitStructure);							//
	
	GPIO_InitStructure.GPIO_Pin = W25CLK_PIN;								//SCK
	GPIO_ResetBits(W25CLK_Port, W25CLK_PIN);								//����
	GPIO_Init(W25CLK_Port, &GPIO_InitStructure);							//
}








/***********************************************************************************��д������*****************************************************************/
/************************************************************************************************************************
* Function Name : INT8U W25QXX_Read_By_Sector(INT8U *OutBuff,INT32U Sector_Index,INT8U Count)			                                            
* Description   : ��ȡSPI FLASH ,W25Qxx,��Sector���ж�ȡһ����������
* Input         : Sector_Index  : ������ַ���� 0~Sector_Max��8191 for WQ256��
*                 OutBuff       : �������ٵĿռ��ָ�룬���ڽ��ա��洢��������ȡ��������
*                 Count			: ��ȡ��������
* Return        : 1����ȡ�ɹ� 0����������
*************************************************************************************************************************/
INT8U W25QXX_Read_By_Sector(INT8U *OutBuff,INT32U Sector_Index,INT8U Count)	
{
	INT8U i = 0;  
	if(!Count)	return 0;	
	if((Sector_Index>Sector_Max)||(Sector_Index+Count-1>Sector_Max))	return 0;		//��ȡ��Χ�����������	

	for(i = 0; i < Count; i++)
	{ 
		W25QXX_Read(Sector_Index*Sector_Size,OutBuff,Sector_Size);						//����������ָ���ļ���������ַת���ɴ洢���Ե�ַ
		Sector_Index++;
		OutBuff+=Sector_Size;
	}
	return 1;
}

/************************************************************************************************************************
* Function Name : INT8U W25QXX_Write_By_Sector(INT8U *InBuff,INT32U Sector_Index,INT8U Count)                                                              
* Description   : дSPI FLASH��������д�����ݣ���һ��д��һ����������                 
* Input         : Sector_Index   : ��ʼд���������ַ
*                 InBuff         : Ҫд�������ָ��
*                 Count			 : д���������
* Return        : 1:д��ɹ� 0����������
*************************************************************************************************************************/
INT8U W25QXX_Write_By_Sector(INT8U *InBuff,INT32U Sector_Index,INT8U Count)  
{ 			 		 
	INT8U i = 0;  
	if(!Count)	return 0;	
	if((Sector_Index>Sector_Max)||(Sector_Index+Count-1>Sector_Max))	return 0;		//д�뷶Χ�����������
	
	for(i = 0; i < Count; i++)
	{
		W25QXX_Erase_Sector(Sector_Index); 
		W25QXX_Write_NoCheck(Sector_Index*Sector_Size,InBuff,Sector_Size);    			// д���������� 
		Sector_Index++;
		InBuff+=Sector_Size;
	}			
	return 1;	
} 









