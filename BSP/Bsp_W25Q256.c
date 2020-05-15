#include "Bsp_W25Q256.h" 


unsigned char W25QXX_BUFFER[4096] = {0};

//INT8U WQ256_Using=0;
 INT8U WQ256_Flag=0;			//bit	7	6 	5 	4 	3 	2 	1 	0		各任务占用标志位
								//任务	X	X	X	X	Wdt	GY	RF	GPRS


/* --------------------Private functions------------------------------------------------------*/
/************************************************************************************************************************
* Function Name : void SPI1_Init(void)                          
* Description   : SPI1 I/O口模拟，相关I/O口的配置。
* Input         : None
* Return        : None
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
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

////SPI1读写一字节数据
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
* Description   : SPI1接口上读写一个字节，备注：软件模拟SPI接口，而非标准的SPI接口，因此：使用这个器件，会特别慢，不适合大数
*                 据量，并且速度有要求的场合。
*                       
* Input         : val  ：要写入的值
* Return        : data ：读出的值
*                
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
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
		if(val&i) W25MOSI_H();   												// 数据建立
		else W25MOSI_L();
//		Fmdelay(1);																//4M测了没问题，32M没测
		W25SCK_H();           	   												// 上升沿数据被写入到铁电  
//		Fmdelay(1);																//4M测了没问题，32M没测
		Rd = W25MISO();
		if(Rd == 1) 
		{
			data |= i;           
		}
	}
	W25SCK_L();																	//拉低以降功耗
	return data;
}

/************************************************************************************************************************
* Function Name : INT8U W25QXX_Init(void)                              
* Description   : 初始化W25QXX， 主要是，初始化SPI FLASH的IO口，使能4字节地址模式，读取芯片的版本号，以此判断是否初始化成。
*                 备注：
*                 256个字节为一页(page)，由16个页共4K bytes组成为一个扇区(Sector)，8个扇区(共128个页)为1个（32k Bytes）
*                 大小的Block，16个扇区（共256个页）为一个（64k Bytes）大小的Block。
*                 
*                 W25Q256：3字节地址模式时：容量为16M字节, 4096个Sector，块的话，爱以多大算一块自己去分，手册也讲得让人迷糊。
*                          4字节地址模式时：容量为32M字节，8192个Sector，块的话，爱以多大算一块自己去分，手册也讲得让人迷糊。
*                       
* Input         : 锁
* Return        : 1 ：成功
*                 0 ：失败
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : /
*************************************************************************************************************************/
INT8U W25QXX_Init(INT8U Task_Num)
{ 
	unsigned short BANBEN = 0;
	unsigned int Status = 0; 
	
	WQ256_Flag|=(1<<Task_Num);								//
	if( WQ256_Flag&~(1<<Task_Num) ) return 1;				//若其他任务已初始化，FM_Flag非0，直接返回
	
	SPI1_Init();
	W25QXX_WAKEUP();
	W25QXX_EnterOrQuit_4Bytes_AddrMode(W25X_Enable4ByteAddr);    // zzs add ,开启4字节地址模式,本指令写入后，影响的是非易失性位
	Status = W25QXX_ReadSR();
	if(!(Status&ADS)) return 0;     // 4字节地址模式设置失败
	
	BANBEN = W25QXX_ReadID();
	if(BANBEN == W25QXX_TYPE) return 1;  // 成功
	else return 0;                       // 失败
}  






/************************************************************************************************************************
* Function Name : unsigned int W25QXX_ReadSR(void)                                     
* Description   : 读取W25QXX的状态寄存器（默认 : 0x00 zzs???）
*                  Bit23    22   21   20  19   18   17   16  |  15   14   13   12   11   10   9    8  |   7   6   5   4   3    2     1    0
*                 HOLD/RST DRV1 DRV0 (R)  (R)  WPS  ADP  ADS    SUS  CMP  LB3  LB2  LB1  (R)  QE  SRP1  SRP0  TB BP3 BP2  BP1  BP0  WEL  BUSY
*                 备注: ADP : Power Up Address Mode  ---(未列出的，具体查看数据手册，这儿仅将比较主要的备注写明一下)           
*                       SRP0                : 默认0,状态寄存器保护位,配合WP使用
*                       TB,BP3,BP2,BP1,BP0 : FLASH区域写保护设置
*                       WEL                : 写使能锁定
*                       BUSY               : 忙标记位(1,忙;0,空闲)
*                       
* Input         : None
* Return        : None
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
#if 0
unsigned char W25QXX_ReadSR(void)   
{  
	unsigned char byte=0;

	W25CS_L();                               // 使能器件（片选选中） 
	SPI1_ReadWrite(W25X_ReadStatusReg);      // 发送读取状态寄存器命令    
	byte = SPI1_ReadWrite(0Xff);             // 读取一个字节  
	W25CS_H();                               // 禁能器件（取消片选）
	
	return byte;   
} 
#else
unsigned int W25QXX_ReadSR(void)   
{  
	INT8U i = 0;
	unsigned int Stat = 0;
		
	W25CS_L();                               // 使能器件（片选选中） 
	SPI1_ReadWrite(W25X_ReadStatusReg3);     // 发送读取状态寄存器3命令    
	Stat = SPI1_ReadWrite(0xff);
	Stat <<=8;
	
	W25CS_H();     // 禁能器件（取消片选） //与手册描述不一致，非得重新片选，华邦,卧槽尼玛，你手册咋写的？？？这么大公司，这也太不严谨了吧
	for(i = 0; i < 100; i++) {;}
	W25CS_L();                             // 使能器件（片选选中） 
		
	SPI1_ReadWrite(W25X_ReadStatusReg2);   // 发送读取状态寄存器2命令  
	Stat |=SPI1_ReadWrite(0xff);
	Stat <<=8;
	
	W25CS_H();     // 禁能器件（取消片选） //与手册描述不一致，非得重新片选，华邦,卧槽尼玛，你手册咋写的？？？这么大公司，这也太不严谨了吧
	for(i = 0; i < 100; i++) {;}
	W25CS_L();                             // 使能器件（片选选中）
	
	SPI1_ReadWrite(W25X_ReadStatusReg1);   // 发送读取状态寄存器1命令  
	Stat |=SPI1_ReadWrite(0xff);
	
	W25CS_H();                             // 禁能器件（取消片选）
	
	return Stat;   
} 
#endif

/************************************************************************************************************************
* Function Name : void W25QXX_EnterOrQuit_4Bytes_AddrMode(unsigned char Cmd_Byte)                                     
* Description   : W25QXX进入4Bytes地址模式
*                 
* Input         : None
* Return        : None
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
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
* Description   : 写W25QXX状态寄存器
*                 备注：只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写！！！！！！！！！！ zzs??? 真相待查明？？？？？？？？？？
* Input         : sr : 要写入的值
* Return        : None
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Write_SR(unsigned char sr)   
{   
	W25CS_L();                             // 使能器件（片选选中） 
	SPI1_ReadWrite(W25X_WriteStatusReg);   // 发送写取状态寄存器命令    
	SPI1_ReadWrite(sr);                    // 写入一个字节  
	W25CS_H();                             // 禁能器件（取消片选） 	      
}   
#else
/************************************************************************************************************************
* Function Name : void W25QXX_Write_SR(unsigned char WriteStatusRegX,unsigned char Value)                                   
* Description   : 写W25QXX的状态寄存器, 
*                 备注：写使能指令后，禁能又使能，请看手册：“The WEL bit must be set prior to every ...”
*                 而且是：只有当“driving /CS high”之后，芯片才会处在可写的状态，说明写使能指令，需要提前、独立发布先。
*                 “After power-up the device is automatically placed in a write-disabled state with the Status Register 
*                   Write Enable Latch (WEL) set to a 0. A Write Enable instruction must be issued before a Page Program, 
*                   Sector Erase, Block Erase, Chip Erase or Write Status Register instruction will be accepted. After 
*                   completing a program, erase or write instruction the Write Enable Latch (WEL) is automatically cleared 
*                   to a write-disabled state of 0.” 看到这段里头的“must be issued before...”没？
*
* Input         : WriteStatusRegX   : 标明要写的是哪一个状态寄存器
*                 Value             : 值
* Return        : None
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Write_SR(unsigned char WriteStatusRegX,unsigned char Value)   
{   
	unsigned char i = 0;
	W25CS_L();                             // 使能器件（片选选中） 
	SPI1_ReadWrite(W25X_WriteEnable);      // 发送写使能,
	
	W25CS_H();                             // 禁能器件（取消片选）
	for(i = 0; i < 100; i++) {;}
	W25CS_L();                             // 使能器件（片选选中）
		
	SPI1_ReadWrite(WriteStatusRegX);       // 发送写取状态寄存器命令    
	SPI1_ReadWrite(Value);                 // 写入一个字节  
	W25CS_H();                             // 禁能器件（取消片选） 	      
}   
#endif

/************************************************************************************************************************
* Function Name : void W25QXX_Write_Enable(void)                                          
* Description   : W25QXX写使能，将WEL置位
* Input         : None
* Return        : None
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Write_Enable(void)   
{
	W25CS_L();                            // 使能器件（片选选中）   
	SPI1_ReadWrite(W25X_WriteEnable);     // 发送写使能  
	W25CS_H();                            // 禁能器件（取消片选）   	      
} 

/************************************************************************************************************************
* Function Name : void W25QXX_Write_Disable(void)	                                            
* Description   : W25QXX写禁止，将WEL清零 
* Input         : None
* Return        : None
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Write_Disable(void)   
{  
	W25CS_L();                            // 使能器件（片选选中）  
    SPI1_ReadWrite(W25X_WriteDisable);    // 发送写禁止指令    
	W25CS_H();                            // 禁能器件（取消片选） 	      
} 

/************************************************************************************************************************
* Function Name : unsigned short W25QXX_ReadID(void)	                                            
* Description   : 读取芯片ID
* Input         : None
*                
* Return        : Temp : = 0XEF13,表示芯片型号为W25Q80  
*                        = 0XEF14,表示芯片型号为W25Q16
*                        = 0XEF15,表示芯片型号为W25Q32
*                        = 0XEF16,表示芯片型号为W25Q64
*                        = 0XEF17,表示芯片型号为W25Q128
*                        = 0XEF18,表示芯片型号为W25Q256
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
unsigned short W25QXX_ReadID(void)
{
	unsigned short Temp = 0;	  

	W25CS_L();	            
	SPI1_ReadWrite(0x90);         // 发送读取ID命令	    
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
* Description   : 读取SPI FLASH ,W25Qxx,在指定地址开始读取指定长度的数据
* Input         : ReadAddr      : 开始读取的地址(4Bytes Address Mode)
*                 pBuffer       : 主调开辟的空间的指针，用于接收、存储本函数读取返回数据
*                 NumByteToRead : 读取的长度(最大65535)
* Return        : None
*
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Read(unsigned int ReadAddr,unsigned char* pBuffer,unsigned short NumByteToRead)	
{ 
	unsigned short i = 0;  
	
	__disable_irq(); // 禁止总中断     // zzs add 2018.3.31
	
	Feed_Dog();	
	
	W25CS_L();	                                      // 使能器件（片选选中） 
	// SPI1_ReadWrite(W25X_ReadData);                 // 发送读取命令
	SPI1_ReadWrite(W25X_ReadData_With4BytesAddr);     // zzs modified it like this
	SPI1_ReadWrite((unsigned char)((ReadAddr)>>24));  // zzs add this
	SPI1_ReadWrite((unsigned char)((ReadAddr)>>16));  
	SPI1_ReadWrite((unsigned char)((ReadAddr)>>8));   
	SPI1_ReadWrite((unsigned char) ReadAddr ); 
	
	for(i = 0; i < NumByteToRead; i++)
	{ 
		if(i%1024==0) 
		{		
			Feed_Dog();								  //每写1K就喂一次狗，防止看门狗复位
		}
		pBuffer[i] = SPI1_ReadWrite(0XFF);            // 循环读数（先向SPI2->DR写入OXFF以防读取的数据错误）  
	}
	W25CS_H();	                                      // 禁能器件（取消片选）     	      
	
	__enable_irq();	// 开总中断	  // zzs add 2018.3.31
}  

/************************************************************************************************************************
* Function Name : void W25QXX_Write_Page(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short NumByteToWrite)                                                            
* Description   : SPI在一页(0~65535)内写入少于256个字节的数据
*                 在指定地址开始写入最大256字节的数据
*                          
* Input         : WriteAddr      ：开始写入的地址(4Bytes Address Mode)
*                 pBuffer        : 要写入的数据
*                 NumByteToWrite : 要写入的字节数(最大256),该数不应该超过该页的剩余字节数！！！
*                 
* Return        : None
*
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Write_Page(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short NumByteToWrite)
{
 	unsigned short i = 0;  
	
	__disable_irq(); // 禁止总中断     // zzs add 2018.3.31
	
    W25QXX_Write_Enable();                  // SET WEL     // 写使能命令需要一个独立的 “片选拉低 + 移入写使能指令 + 片选拉高”过程
	
	W25CS_L();                              // 使能器件（片选选中） 
    SPI1_ReadWrite(W25X_PageProgram);       // 发送写页命令 
    SPI1_ReadWrite((unsigned char)((WriteAddr)>>24));  // zzs add this	
    SPI1_ReadWrite((unsigned char)((WriteAddr)>>16));  
    SPI1_ReadWrite((unsigned char)((WriteAddr)>>8));   
    SPI1_ReadWrite((unsigned char)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++)
	{
	   SPI1_ReadWrite(pBuffer[i]);   // 循环写数 
	}		
	W25CS_H();	                     // 禁能器件（取消片选） 
	W25QXX_Wait_Busy();				 // 等待写入结束
	
	__enable_irq();	// 开总中断	  // zzs add 2018.3.31
} 

/************************************************************************************************************************
* Function Name : void W25QXX_Write_NoCheck(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short NumByteToWrite)                                                             
* Description   : 无检验写SPI FLASH，在指定地址开始写入指定长度的数据,但是要确保地址不越界!
*                 必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
*                 (zzs总结下：总之一条，1要写为0，行，ok,但是，如果Flash中原始的东西已经是0，但是你要写的数据，在这个位置上又不是0，
*                  而是1的话，那，对不起，没办法写回1的。Flash的写机制就是：数据1，是不需要什么动作的，只需要将需要写为0的地方，从1写为0就可)
*                 具有自动换页功能          
* Input         : WriteAddr      ：开始写入的地址(4Bytes Address Mode)
*                 pBuffer        : 要写入的数据
*                 NumByteToWrite : 要写入的字节数(最大65535)
* Return        : None
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Write_NoCheck(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short NumByteToWrite)   
{ 			 		 
	unsigned short  pageremain = 0;	
    unsigned char Loop_cnt=0; // zzs add ！！！这个不属于W25Q驱动层的东西
	
	pageremain = 256 - WriteAddr%256;  // 单页剩余的字节数		 	    
	if(NumByteToWrite <= pageremain) pageremain = NumByteToWrite; // 不大于256个字节
	while(1)
	{	   
		W25QXX_Write_Page(WriteAddr,pBuffer,pageremain);
		if(NumByteToWrite == pageremain) break;         // 写入结束了
		else  // NumByteToWrite > pageremain
		{
			Loop_cnt++;         // zzs add！！！这个不属于W25Q驱动层的东西
			if(Loop_cnt%4==0)   // zzs add！！！这个不属于W25Q驱动层的东西
			{
				Feed_Dog();						//每写1K就喂一次狗，防止看门狗复位	
				OSTimeDly(2); // 100ms // zzs add！！！这个不属于W25Q驱动层的东西，这个完全是因为加入操作系统后，如果出现满页写时，容易造成CPU占用过长时间而导致其他任务等不及而发生意外，所以在这里让出一会儿。	
			}
			
			pBuffer += pageremain;
			WriteAddr += pageremain;	
			NumByteToWrite -= pageremain;			    // 减去已经写入了的字节数
			if(NumByteToWrite > 256) pageremain = 256;  // 一次最多可以写入256个字节
			else pageremain = NumByteToWrite; 	        // 不够256个字节了
		}
	}	    
} 

/************************************************************************************************************************
* Function Name : void W25QXX_Write(unsigned int WriteAddr,unsigned char* pBuffer,unsigned short  NumByteToWrite)                                                          
* Description   : 写SPI FLASH ,在指定地址开始写入指定长度的数据 ,该函数带擦除操作!
*                
* Input         : WriteAddr      ：开始写入的地址(4Bytes Address Mode)
*                 pBuffer        : 要写入的数据
*                 NumByteToWrite : 要写入的字节数(最大65535)
*                 
* Return        : None
*
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
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
	
 	secpos = WriteAddr/4096;    // 扇区地址 ,0~8192 for w25Q256 
	secoff = WriteAddr%4096;    // 在扇区内的偏移
	secremain = 4096 - secoff;  // 扇区剩余空间大小   
 	// printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//测试用
 	if(NumByteToWrite <= secremain) secremain = NumByteToWrite;    // 不大于4096个字节
	while(1) 
	{	
		W25QXX_Read(secpos*4096,W25QXX_BUF,4096);    // 读出整个扇区的内容---OK
		
		OSTimeDly(4); // 200ms  // ！！！这个不属于W25Q驱动层的东西，这个完全是因为加入操作系统后，如果出现满页写时，容易造成CPU占用过长时间而导致其他任务等不及而发生意外，所以在这里让出一会儿。
		
		for(i = 0;i < secremain; i++)                // 校验数据
		{
			if(W25QXX_BUF[secoff+i] != 0XFF) break;     // 需要擦除  	  
		}
		
		if(i < secremain)    // 需要擦除
		{
			W25QXX_Erase_Sector(secpos);             // 擦除这个扇区---ERROR!
			
			for(i = 0;i < secremain; i++)	         // 复制
			{
				W25QXX_BUF[i + secoff] = pBuffer[i];	  
			}
			
			W25QXX_Write_NoCheck(secpos*4096,W25QXX_BUF,4096);    // 写入整个扇区---ERROR!  

		}else W25QXX_Write_NoCheck(WriteAddr,pBuffer,secremain);  // 写已经擦除了的,直接写入扇区剩余区间. 				   
		
		if(NumByteToWrite == secremain) break;   // 写入结束了
		else             // 写入未结束
		{
			secpos++;    // 扇区地址增1
			secoff = 0;  // 偏移位置为0 	 

		   	pBuffer += secremain;    // 指针偏移
			WriteAddr += secremain;  // 写地址偏移	   
		   	NumByteToWrite -= secremain;				  // 字节数递减
			if(NumByteToWrite > 4096) secremain = 4096;	  // 下一个扇区还是写不完
			else secremain=NumByteToWrite;			      // 下一个扇区可以写完了
		}	 
	}	 
}

/************************************************************************************************************************
* Function Name : void W25QXX_Erase_Chip(void)                                             
* Description   : 擦除整个芯片（备注：等待时间超长:几十秒，准确测过,整片擦除，耗时72秒）
* Input         : None
* Return        : None 
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Erase_Chip(void)   
{                                   
	W25QXX_Write_Enable();                 // SET WEL 
	W25QXX_Wait_Busy();   
	W25CS_L();                             // 使能器件   
	SPI1_ReadWrite(W25X_ChipErase);        // 发送片擦除命令  
	W25CS_H();                             // 取消片选     	      
	W25QXX_Wait_Busy();   				   // 等待芯片擦除结束
}   

/************************************************************************************************************************
* Function Name : void W25QXX_Erase_Sector(unsigned int Dst_Addr)                                              
* Description   : 擦除一个扇区（备注：擦除一个扇区的最少时间:150ms）
* Input         : Dst_Addr : 扇区地址 根据实际容量设置
* Return        : None 
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Erase_Sector(unsigned int Dst_Addr)   
{  	  
	// printf("fe:%x\r\n",Dst_Addr);	  // 监视falsh擦除情况,测试用 
	
	__disable_irq(); // 禁止总中断     // zzs add 2018.3.31
	
	BSP_WDGFeedDog();                 // zzs add 2018.06.12
	
	Dst_Addr *= 4096;
	W25QXX_Write_Enable();                // SET WEL 	 
	W25QXX_Wait_Busy();   
	W25CS_L();                            // 使能器件   
	SPI1_ReadWrite(W25X_SectorErase);     // 发送扇区擦除指令 
	SPI1_ReadWrite((u8)((Dst_Addr)>>24)); // zzs add this 
	SPI1_ReadWrite((u8)((Dst_Addr)>>16));  
	SPI1_ReadWrite((u8)((Dst_Addr)>>8));   
	SPI1_ReadWrite((u8)Dst_Addr);  
	W25CS_H();                            // 取消片选     	      
	W25QXX_Wait_Busy();   				  // 等待擦除完成
	
	__enable_irq();	// 开总中断	  // zzs add 2018.3.31
} 

/************************************************************************************************************************
* Function Name : void W25QXX_PowerDown(void)                                               
* Description   : 等待W25Q256空闲
* Input         : None
* Return        : None 
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_Wait_Busy(void)   
{   
	// printf("W25QXX_ReadSR()=%x\r\n",W25QXX_ReadSR());
	while( (W25QXX_ReadSR()&0x01) == 0x01) {;}           // 等待BUSY位清空
}  

/************************************************************************************************************************
* Function Name : void W25QXX_PowerDown(void)                                               
* Description   : W25Q256进入掉电模式
* Input         : None
* Return        : None 
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_PowerDown(void)   
{ 
	W25CS_L();                          // 使能器件
	SPI1_ReadWrite(W25X_PowerDown);     // 发送掉电命令  
	W25CS_H();                          // 取消片选     	      
	delay_2us_4M(1);                    // 等待TPD	入参为1的单次调用约5.4us，而非2us；
}

/************************************************************************************************************************
* Function Name : void W25QXX_WAKEUP(void)                                                 
* Description   : 唤醒W25Q256。
* Input         : None
* Return        : None 
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
void W25QXX_WAKEUP(void)   
{  
	W25CS_L();                               // 使能器件   
	SPI1_ReadWrite(W25X_ReleasePowerDown);   // send W25X_PowerDown command 0xAB    
	W25CS_H();                               // 取消片选     	      
	delay_2us_4M(1);                         // 等待TRES1	入参为1的单次调用约5.4us，而非2us；
}   

/************************************************************************************************************************
* Function Name : INT8U WQ256_Test(void)                                                  
* Description   : 测试W25Q256。
*                 芯片容量为256M Bits,32M Bytes,有3字节地址模式，和4字节地址模式，zzs note:注意用量，是半片用(3字节地址，
*                 16M Bytes寻址范围)，还是全片用（4字节地址模式，32M Bytes）。
*                 
*                 注：W25Q256，SPI-Flash, 256Bytes以页，最小擦除单位：16页（4kBytes），也可按128页为一块（32kBytes）进行擦除，
*                     也可按256页为一块（64kBytes）进行擦除，也可按整片(entire chip erase)
* Input         : None
* Return        :  0 ：测试失败
*                  1 ：测试成功
*
* History :                               
* First Issued  : 赵志舜于2018年2月7日创建本函数             E-Mail:11207656@qq.com
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
	W25QXX_Erase_Sector(0);          // 擦除第0块
	W25QXX_Read(0,FMTESTBUFF,256);   // 读出看初始擦的情况
	for(i = 0; i < 256; i++)
	{
		FMTESTBUFF[i]=i;    // 填充缓冲
	}
	
	W25QXX_Write_NoCheck(0,FMTESTBUFF,256);   // 无校验，不带擦除，写一次
	for(i = 0; i < 256; i++) FMTESTBUFF[i]=0; // 清除缓冲
	W25QXX_Read(0,FMTESTBUFF,256);            // 读出看第一次写的情况      // 这次写，是能够完全正确的
	
	for(i = 0; i < 256; i++)
	{
		FMTESTBUFF[i] =  255 - i;             // 将缓冲内容前后颠倒过来,  再用无校验，无擦除函数写，也就是写上次每个字节中留下来为1的位，下面读出来只要是全0，就OK了  
		                                      // 也可随便用个其他的测试值，例如：0xAA来填充全部缓冲;但是后面读出来的值就是一些乱乱的值。
	}
	W25QXX_Write_NoCheck(0,FMTESTBUFF,256);       // 无校验，不带擦除，写第二次
	for(i = 0; i < 256; i++) FMTESTBUFF[i] =0xFF; // 涂抹缓冲
	W25QXX_Read(0,FMTESTBUFF,256);                // 读出看第二次写的情况   // 读出全为0就对了。
	
	for(i = 0; i < 256; i++)
	{
		FMTESTBUFF[i]=i;    // 填充缓冲
	}
	W25QXX_Write(0,FMTESTBUFF,256);           // 带擦除写第一次
	for(i = 0; i < 256; i++) FMTESTBUFF[i]=0; // 清除缓冲
	W25QXX_Read(0,FMTESTBUFF,256);            // 读出看带擦除写的情况
	
	W25QXX_Erase_Sector(0);         // 擦除第0块
	W25QXX_Read(0,FMTESTBUFF,256);  // 读出看第0块的的擦除情况
	
	W25QXX_Erase_Chip();                  // 整片擦除
	
		#if 0
		// W25QXX_Erase_Sector(Scetor_No);    // 块擦除
		W25QXX_Erase_Chip();                  // 整片擦除
			
		for(j = 0; j < 65536; j++)  // 前半片
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

		// W25QXX_Erase_Sector(Scetor_No);    // 块擦除
		W25QXX_Erase_Chip();                  // 整片擦除

		for(j = 65536; j < 131072; j++)       // 后半片
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
	
	W25QXX_Erase_Sector(0);          // 擦除第0块
	W25QXX_Read(0,FMTESTBUFF,256);   // 从非页地址上读，是没有太大问题的，
	for(i = 0; i < 256; i++)
	{
		FMTESTBUFF[i] = i;
	}
	
	W25QXX_Write_NoCheck(0,FMTESTBUFF,256);     // 但是从非页地址上写，那就得注意啦，稍不注意就出错！！！！！！！！！！！
	
	for(i = 0; i < 256; i++) FMTESTBUFF[i]=0;
	
	W25QXX_Read(0,FMTESTBUFF,256);  
	for(i = 0; i < 256; i++)
	{
		if(FMTESTBUFF[i] != i) return 0;           // W25Qxx测试失败
	}
    
	W25QXX_Erase_Chip();    // 测试完成后擦除芯片
	
	return 1;  //W25Qxx 测试成功
}



/******************************************************************************* 
* Function Name  : void W25Q256_LowPower(void)
* Description    : W25Q256进入低功耗，并对相应IO口作低功耗处理
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/
void W25Q256_LowPower(INT8U Task_Num)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;	
	
	WQ256_Flag&=~(1<<Task_Num);							//清当前任务bit	
	if(WQ256_Flag) return;								//若其他任务占用，FM_Flag非0，直接返回
	
	/*若未初始化，无法进入关机状态*/
	W25QXX_PowerDown();														//进入关机状态
	

	/*片选口、写保护口模拟输入*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;							//模拟输入
	
	GPIO_InitStructure.GPIO_Pin = W25CS_PIN;								//片选口
	GPIO_Init(W25CS_Port, &GPIO_InitStructure);								//
	
//	GPIO_InitStructure.GPIO_Pin = W25WP_PIN;								//写保护口未使用
//	GPIO_Init(W25WP_Port, &GPIO_InitStructure);								//	
	
	
	/*SPI三个引脚推挽拉低*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;						//推挽输出
	
	GPIO_InitStructure.GPIO_Pin = W25MOSI_PIN;								//MOSI
	GPIO_ResetBits(W25MOSI_Port, W25MOSI_PIN);								//拉低
	GPIO_Init(W25MOSI_Port, &GPIO_InitStructure);							//
	
	GPIO_InitStructure.GPIO_Pin = W25MISO_PIN;								//MISO
	GPIO_ResetBits(W25MISO_Port, W25MISO_PIN);								//拉低
	GPIO_Init(W25MISO_Port, &GPIO_InitStructure);							//
	
	GPIO_InitStructure.GPIO_Pin = W25CLK_PIN;								//SCK
	GPIO_ResetBits(W25CLK_Port, W25CLK_PIN);								//拉低
	GPIO_Init(W25CLK_Port, &GPIO_InitStructure);							//
}








/***********************************************************************************新写的驱动*****************************************************************/
/************************************************************************************************************************
* Function Name : INT8U W25QXX_Read_By_Sector(INT8U *OutBuff,INT32U Sector_Index,INT8U Count)			                                            
* Description   : 读取SPI FLASH ,W25Qxx,按Sector进行读取一个扇区或多个
* Input         : Sector_Index  : 扇区地址索引 0~Sector_Max（8191 for WQ256）
*                 OutBuff       : 主调开辟的空间的指针，用于接收、存储本函数读取返回数据
*                 Count			: 读取的扇区数
* Return        : 1：读取成功 0：参数错误
*************************************************************************************************************************/
INT8U W25QXX_Read_By_Sector(INT8U *OutBuff,INT32U Sector_Index,INT8U Count)	
{
	INT8U i = 0;  
	if(!Count)	return 0;	
	if((Sector_Index>Sector_Max)||(Sector_Index+Count-1>Sector_Max))	return 0;		//读取范围超出最大扇区	

	for(i = 0; i < Count; i++)
	{ 
		W25QXX_Read(Sector_Index*Sector_Size,OutBuff,Sector_Size);						//按扇区读出指定文件，扇区地址转化成存储绝对地址
		Sector_Index++;
		OutBuff+=Sector_Size;
	}
	return 1;
}

/************************************************************************************************************************
* Function Name : INT8U W25QXX_Write_By_Sector(INT8U *InBuff,INT32U Sector_Index,INT8U Count)                                                              
* Description   : 写SPI FLASH，按扇区写入数据，可一次写入一个或多个扇区                 
* Input         : Sector_Index   : 开始写入的扇区地址
*                 InBuff         : 要写入的数据指针
*                 Count			 : 写入的扇区数
* Return        : 1:写入成功 0：参数错误
*************************************************************************************************************************/
INT8U W25QXX_Write_By_Sector(INT8U *InBuff,INT32U Sector_Index,INT8U Count)  
{ 			 		 
	INT8U i = 0;  
	if(!Count)	return 0;	
	if((Sector_Index>Sector_Max)||(Sector_Index+Count-1>Sector_Max))	return 0;		//写入范围超出最大扇区
	
	for(i = 0; i < Count; i++)
	{
		W25QXX_Erase_Sector(Sector_Index); 
		W25QXX_Write_NoCheck(Sector_Index*Sector_Size,InBuff,Sector_Size);    			// 写入整个扇区 
		Sector_Index++;
		InBuff+=Sector_Size;
	}			
	return 1;	
} 









