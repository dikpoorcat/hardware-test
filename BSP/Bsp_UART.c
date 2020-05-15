#include "Bsp_UART.h"
 
/*
   串口使用说明
   1--> 电源板控制
   2--> 485 /232 打印 三者只能用其一
   3--> 读头 	  
   4--> 3G 等无线通讯模块
   5--> 备用 
*/

UARTx_Ctrl_Struct   UARTx_Ctrl_Array[5];

/* --------------------Private functions------------------------------------------------------*/
/************************************************************************************************************************
* Function Name : void B485_init(unsigned int rate)
* Description   : B485串口初始化（根据B485DIS宏定义自动判断）。
* Input         : rate : 波特率
*
* Return        : None
*************************************************************************************************************************/
void B485_init(unsigned int rate)
{
    UARTx_Setting_Struct UARTInit = {0};	
    
#ifdef B485DIS 		/*注意，没初始化485时万万不可发送485数据，否则TxBusy非空会导致无法进入STOP模式*/
	if(!(TaskActive & Local_ACT)) return;										//LOCAL已经结束
#endif	
    if(GyBOX == NULL) GyBOX = OSMboxCreate(0);	
	else GyBOX->OSEventPtr= (void *)0;											//清消息邮箱，不清会导致误判 ZE
	
    UARTInit.BaudRate = rate;
    UARTInit.Parity   = BSPUART_PARITY_NO;
    UARTInit.StopBits = BSPUART_STOPBITS_1;
    UARTInit.DataBits = 8;
    UARTInit.RxBuf    = B485BUF;    		   
    UARTInit.RxBufLen = Buff485_LEN;
    UARTInit.TxBuf    = B485BUF;
    UARTInit.TxBufLen = Buff485_LEN;
    UARTInit.Mode     = UART_RS485_MODE;      									// UART_HALFDUP_MODE
    BSP_UART_Init(2,&UARTInit,GyBOX);
	
	Power485Pin_Init();
	PWDC485EN();       															//打开485隔离电源
	OSTimeDly(2);																//等待电源稳定
}

/******************************************************************************* 
* Function Name  : void B485_LowPower(void)
* Description    : 485进入低功耗，并对相应IO口作低功耗处理。关闭U2时钟及复用时钟。
					中断在外面已关闭，关闭可有效降低功耗
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/
void B485_LowPower(void)													//在关闭串口之前一定要保证485输出完成，清除busy标志位，否则将导致不再进入STOP模式，直到下次初始化清空。
{
	INT8U WaitTime = 100; 													//485等待超时时间5秒
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	/*等待485发送完成*/	
	while( BSP_UART_TxState(2) && (WaitTime--) ) OSTimeDly(1);				//发数据数据的过程的延时等待，最多5秒
	if(WaitTime==0xff)
	{
		B485_init(38400);													//重新初始化以清TxBusy，防止无法进入STOP模式
		PWDC485EN();														// DCDC隔离485打开				
		OSTimeDly(10);
		BspUartWrite(2,SIZE_OF("警告：B485_LowPower等待超时！----------------------------------\r\n"));
		OSTimeDly(10);
	}
	
//	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, DISABLE );					//不关复用时钟|RCC_APB2Periph_AFIO（各串口和AD都用到这个）
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2, DISABLE );				//关闭U2时钟
	
	
	/*485EN、TX、RX模拟输入*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;							//模拟输入
	
	GPIO_InitStructure.GPIO_Pin = EN485_PIN;								//485EN（PB15）
	GPIO_Init(EN485_Port, &GPIO_InitStructure);								//
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;								//USART2 Tx (PA2)
	GPIO_Init(GPIOA, &GPIO_InitStructure);									//
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;								//USART2 Rx (PA3) 
	GPIO_Init(GPIOA, &GPIO_InitStructure);									//
	
	
	/*485电源使能推挽输出*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;						//推挽输出
	PWDC485DIS();															//关电源
	GPIO_InitStructure.GPIO_Pin = PWDC485_PIN;								//485电源使能口
	GPIO_Init(PWDC485_Port, &GPIO_InitStructure);							//
	
	/*关中断*/
	USART_ITConfig( USART2, USART_IT_TC, DISABLE);
	USART_ITConfig( USART2, USART_IT_RXNE, DISABLE);
	USART_Cmd( USART2,DISABLE);	
}

/************************************************************************************************************************
* Function Name : void UART1_PIN_CFG()  
* Description   : UART1的Tx、 Rx pin脚配置，使能总线，配置管脚工作模式，速率等参数。
* Input         : None
* Return        : None
*************************************************************************************************************************/
void UART1_PIN_CFG(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure = {0};

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO,ENABLE);

	/* Configure USART1 Tx (PA9) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART1 Rx (PA10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
}

/************************************************************************************************************************
* Function Name : void UART1_PIN_CLOSE()
* Description   : UART1的Tx、 Rx pin脚关闭（禁能）
* Input         : None
* Return        : None
*************************************************************************************************************************/
void UART1_PIN_CLOSE(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,DISABLE);
	/* Configure USART1 Tx (PA9) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART1 Rx (PA10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
}

/************************************************************************************************************************
* Function Name : void UART2_PIN_CFG()
* Description   : UART2的Tx、 Rx pin脚配置，使能总线，配置管脚工作模式，速率等参数。
* Input         : None
* Return        : None
*************************************************************************************************************************/
void UART2_PIN_CFG(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
				
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);		//|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* Configure USART2 Tx (PA2) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART2 Rx (PA3) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/*485EN（PB15）*/
	GPIO_InitStructure.GPIO_Pin = EN485_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(EN485_Port, &GPIO_InitStructure);
}

/************************************************************************************************************************
* Function Name : void UART2_PIN_CLOSE()
* Description   : UART2的Tx、 Rx pin脚关闭（禁能）
* Input         : None
* Return        : None
*************************************************************************************************************************/
void UART2_PIN_CLOSE(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
				
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, DISABLE);

	/* Configure USART2 Tx (PA2) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART2 Rx (PA3) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/*485EN（PB15）*/
	GPIO_InitStructure.GPIO_Pin = EN485_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(EN485_Port, &GPIO_InitStructure);
}

/************************************************************************************************************************
* Function Name : void UART3_PIN_CFG()
* Description   : UART3的Tx、 Rx pin脚配置，使能总线，配置管脚工作模式，速率等参数。
* Input         : None
* Return        : None
*************************************************************************************************************************/
void UART3_PIN_CFG(void)
{
  	GPIO_InitTypeDef 	GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

  	/* Configure USART3 Tx (PB10) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

  	/* Configure USART3 Rx (PB11) as input floating */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/************************************************************************************************************************
* Function Name : void UART3_PIN_CLOSE()
* Description   : UART3的Tx、 Rx pin脚关闭（禁能）
* Input         : None
* Return        : None
*************************************************************************************************************************/
void UART3_PIN_CLOSE(void)
{
  	GPIO_InitTypeDef 	GPIO_InitStructure;

    
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE);

  	/* Configure USART3 Tx (PB10) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

  	/* Configure USART3 Rx (PB11) as input floating */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/************************************************************************************************************************
* Function Name : void UART4_PIN_CFG()
* Description   : UART4的Tx、 Rx pin脚配置，使能总线，配置管脚工作模式，速率等参数。
* Input         : None
* Return        : None
*************************************************************************************************************************/
void UART4_PIN_CFG(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

	/* Configure USART4 Tx (PC10) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Configure USART4 Rx (PC11) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/************************************************************************************************************************
* Function Name : void UART5_PIN_CFG()
* Description   : UART5的Tx、 Rx pin脚配置，使能总线，配置管脚工作模式，速率等参数。
* Input         : None
* Return        : None
*************************************************************************************************************************/
void UART5_PIN_CFG(void)
{
  	GPIO_InitTypeDef 	GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

  	/* Configure USART5 Tx (PC12) as alternate function push-pull */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  	GPIO_Init(GPIOC, &GPIO_InitStructure);

  	/* Configure USART5 Rx (PD2) as input floating */
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(GPIOD, &GPIO_InitStructure);
}

/************************************************************************************************************************
* Function Name : USART_TypeDef *GetCommHandle(INT8U ComPort)
* Description   : 串口号到结构体的映射 
*
* Input         : ComPort : 1...5
*
* Return        : 非空值 ： 1...5号串口对应的资源句柄
*                 NULL   : 入参错误
*************************************************************************************************************************/
USART_TypeDef *GetCommHandle(INT8U ComPort)
{
	switch(ComPort)
	{
		case 1: return USART1; 
		case 2: return USART2;
		case 3: return USART3;
		case 4: return UART4;
		case 5: return UART5;
		default:break;
	}
	
	return (USART_TypeDef *)0;
}


/************************************************************************************************************************
* Function Name : __inline void  MODE485_RxEnable(INT8U port)
* Description   : RS485芯片接收使能函数。zzs note，RS485模式，外接485电平转换芯片. 仅发送数据时使能发送，一帧数据
*                 发送完毕，立即使能接收，发送禁止。
*
* Input         : Port : 这个Port参数，现在只是做做样子罢了，因为限制死了，只能是2
*
* Return        : None
*************************************************************************************************************************/
__inline void  MODE485_RxEnable(INT8U port)
{
	// INT16U i = 0;
	
	if( port ==  2)
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_15);  // PSAM 写入使能
		// for(i=0;i<100;i++) {;}  // zzs commented this ,内联函数中，不允许使用循环体、switch、递归，否则将按照普通函数处理。
	}	
}

/************************************************************************************************************************
* Function Name : __inline void  MODE485_TxEnable(INT8U port)
* Description   : RS485发送数据使能
*
* Input         : Port : 这个Port参数，现在只是做做样子罢了，因为限制死了，只能是2
*
* Return        : None
*************************************************************************************************************************/
__inline void  MODE485_TxEnable(INT8U port)
{
	// INT16U i = 0;
	
	if(port == 2)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_15);   // 485发送使能
	}
	
	// for(i=0;i<100;i++) {;}  // zzs commented this ,内联函数中，不允许使用循环体、switch、递归，否则将按照普通函数处理。
}


/************************************************************************************************************************
* Function Name : __inline void  UART_RxEnable(INT8U Com,INT8U Enable)
* Description   : 接收数据使能,或者除能
*
* Input         : Com    : 串口号
*                 Enable : 1 ：接收使能
*                          0 ：接收除能
*
* Return        : None
*************************************************************************************************************************/
__inline void  UART_RxEnable(INT8U Com,INT8U Enable)
{
	volatile INT16U tmpreg = 0x00;
	USART_TypeDef *pUARTx = NULL;
	
	pUARTx = GetCommHandle(Com);
	if(pUARTx == 0) return;
	
	tmpreg = pUARTx->CR1;
	if(Enable)
	{
		tmpreg |= 0x0004;
	}
	else
	{
		tmpreg &= 0xFFFB;
	}
	pUARTx->CR1 = tmpreg;	
 
}

/************************************************************************************************************************
* Function Name : void SendReceiveMsg(INT8U ComPort,UARTx_Ctrl_Struct *pCtrl_UARTx)          
* Description   : 本函数就只是个干活的，由系统Ticker中断调用分帧扫描函数后，再由分帧扫描函数调用本函数post receive msg，
*                 本函数执行“接收到一帧串口数据”消息的装配填充，维护一下串口管理器，然后对串口绑定的邮箱执行Post操作。
*
* Input         : ComPort :  要初始化的串口号， 1 ... 5
*                 pCtrl_UARTx :  串口管理结构体
*                 
*
* Return        : 隐式返回：抛出一条消息，消息内容在 这个位置 pCtrl_UARTx->pRecvBuf + pCtrl_UARTx->RxOffset
*************************************************************************************************************************/
void SendReceiveMsg(INT8U ComPort,UARTx_Ctrl_Struct *pCtrl_UARTx)
{
	struct Str_Msg *TmpMsg = (struct Str_Msg *)0;
	
	
	if(pCtrl_UARTx->BspMsg[0].DataLen == 0xFFFF)   // zzs??? “0xFFFF，认为是可用的” ，注： 这要是两个的 DataLen 都 ==0xFFFF呢？？？ 
		TmpMsg = &pCtrl_UARTx->BspMsg[0];
	
	if(pCtrl_UARTx->BspMsg[1].DataLen == 0xFFFF)  // zzs??? 卧槽，事实证明，这两个的DataLen都 = 0xFFFF ，这是要干嘛呢？？？
		TmpMsg = &pCtrl_UARTx->BspMsg[1];
	
	if((INT32U) TmpMsg == 0)
		return;
	
	/* 填 充 消 息 */		
	TmpMsg->DivNum  = ComPort ;                      // 设备号，实际就是指定不同的串口号，来区分不同的设备。 
	TmpMsg->pData   = (void *)(pCtrl_UARTx->pRecvBuf + pCtrl_UARTx->RxOffset);                            // zzs!!!消息内容的实际装载位置
	TmpMsg->DataLen = pCtrl_UARTx->RxPointer - pCtrl_UARTx->RxOffset;           // 指明消息的长度 
	if(ComPort == 1)	TmpMsg->MsgID   = BSP_MSGID_RFDataIn;        	 //  Rf串口接收完成消息ID	
	if(ComPort == 2)	TmpMsg->MsgID   = BSP_MSGID_RS485DataIn;  
	if(ComPort == 3)	TmpMsg->MsgID   = BSP_MSGID_UART_RXOVER;         // GPRS 串口接收完成消息ID

	/* 下一帧起始地址 */
	pCtrl_UARTx->RxOffset = pCtrl_UARTx->RxPointer;   // 更新维护RxOffset,仅此一个位置维护了RxOffset
	if(pCtrl_UARTx->MailOrQueue != 0)	
	{
		if(pCtrl_UARTx->MailOrQueue->OSEventType == OS_EVENT_TYPE_Q )
			OSQPost(pCtrl_UARTx->MailOrQueue, TmpMsg);
		else if(pCtrl_UARTx->MailOrQueue->OSEventType == OS_EVENT_TYPE_MBOX )
			OSMboxPost(pCtrl_UARTx->MailOrQueue, TmpMsg);
	}
}

/************************************************************************************************************************
* Function Name : INT8U BSP_UART_Init(INT8U ComPort, UARTx_Setting_Struct *pUartSet, OS_EVENT *Mail_Queue)              
* Description   : 串口初始化
* Input         : ComPort :  要初始化的串口号， 1 ... 5
*                 pUartSet :  串口的配置参数，
*                 Mail_Queue : 给这个串口配备一个邮箱，邮箱或队列指针	不用GPRSBOX的话可以直接用UARTx_Ctrl_Array[x]->MailOrQueue去初始化串口
*
* Return        : 0 : 形参有误
*                 1 ：成功返回
*************************************************************************************************************************/
INT8U BSP_UART_Init(INT8U ComPort, UARTx_Setting_Struct *pUartSet, OS_EVENT *Mail_Queue)
{
	USART_InitTypeDef	USART_InitStructure = {0};
	USART_ClockInitTypeDef	USART_Clock_InitStructure = {0};
	UARTx_Ctrl_Struct *pCtrl_UARTx = NULL;
	USART_TypeDef * pUARTx = NULL;

	/* 参数有效性检查 */
	if(ComPort <= 5 && (pUartSet->DataBits < 7 || pUartSet->DataBits > 8))
		return 0;
	if(pUartSet->Parity > 2)
		return 0;
	if(pUartSet->StopBits < 1 || pUartSet->StopBits > 2)
		return 0;
	//if(pUartSet->BaudRate < 300 || pUartSet->BaudRate > 57600)
	//	return 0;
	if(pUartSet->DataBits == 7 && pUartSet->Parity == 0 && ComPort <= 5)
		return 0;         /* CORTEX串口不支持7位无校验方式，改为有校验的吧。扩展串口支持的 */
	if(ComPort < 1 || ComPort > 5 )
		return 0;	
	
    pCtrl_UARTx = &UARTx_Ctrl_Array[ComPort-1];
 	memcpy(&pCtrl_UARTx->Setting, (void *)pUartSet, sizeof(UARTx_Setting_Struct));

	USART_InitStructure.USART_BaudRate = pUartSet->BaudRate;
	if(pUartSet->DataBits == 7 && pUartSet->Parity == 1)
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_Parity = USART_Parity_Odd;
	}
	
	if(pUartSet->DataBits == 7 && pUartSet->Parity == 2)
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_Parity = USART_Parity_Even;
	}

	if(pUartSet->DataBits == 8 && pUartSet->Parity == 0)
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_Parity = USART_Parity_No;
	}
	
	if(pUartSet->DataBits == 8 && pUartSet->Parity == 1)
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		USART_InitStructure.USART_Parity = USART_Parity_Odd;
	}
	
	if(pUartSet->DataBits == 8 && pUartSet->Parity == 2)
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		USART_InitStructure.USART_Parity = USART_Parity_Even;
	}

	if(pUartSet->StopBits == 1)
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
	if(pUartSet->StopBits == 2)
		USART_InitStructure.USART_StopBits = USART_StopBits_2;
	
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

	USART_Clock_InitStructure.USART_Clock = USART_Clock_Disable;
	USART_Clock_InitStructure.USART_CPOL = USART_CPOL_Low;	          // No use for asynchronous mode
	USART_Clock_InitStructure.USART_CPHA = USART_CPHA_2Edge;		  // No use for asynchronous mode
	USART_Clock_InitStructure.USART_LastBit = USART_LastBit_Disable;  // No use for asynchronous mode

	OS_ENTER_CRITICAL();
	pCtrl_UARTx->pSendBuf = pUartSet->TxBuf;
	pCtrl_UARTx->pRecvBuf = pUartSet->RxBuf;
	pCtrl_UARTx->MaxTxBufLen = pUartSet->TxBufLen;
	pCtrl_UARTx->MaxRxBufLen = pUartSet->RxBufLen;
	pCtrl_UARTx->MailOrQueue = Mail_Queue;

	switch(ComPort)
	{
		case 1: UART1_PIN_CFG();
			    pUARTx = USART1;
			    break;

		case 2: UART2_PIN_CFG();
			    pUARTx = USART2;
			    MODE485_RxEnable(2);
			    break;

		case 3: UART3_PIN_CFG();
			    pUARTx = USART3;
			    break;

		case 4: UART4_PIN_CFG();
			    pUARTx = UART4;
			    break;

		case 5: UART5_PIN_CFG();
			    pUARTx = UART5;
			    break;

 
		default:
			 ;
	}

/* 为了帧间隔时间控制的精确度，OS_TICKS_PER_SEC 应该大于等于 100 */  // zzs note,但是，现在的系统设计，没有达到这个要求。
#if OS_TICKS_PER_SEC < 10
   #error OS_TICKS_PER_SEC should bigger than 100 for UART frame
#endif

	switch(pUartSet->BaudRate)
	{
		case 300: pCtrl_UARTx->RxFrameIntvSet = (OS_TICKS_PER_SEC *4)/5;/* 800ms, 240位时间 */
			      break;
		
		case 600: pCtrl_UARTx->RxFrameIntvSet = (OS_TICKS_PER_SEC *2)/5;/* 400ms, 240位时间 */
			      break;
		
		case 1200: pCtrl_UARTx->RxFrameIntvSet = OS_TICKS_PER_SEC / 5;   /* 200ms, 240位时间 */
				   break;
		
		case 2400: pCtrl_UARTx->RxFrameIntvSet = OS_TICKS_PER_SEC / 10;  /* 100ms, 240位时间 */
				   break;
		
		case 4800: pCtrl_UARTx->RxFrameIntvSet = OS_TICKS_PER_SEC / 20;  /* 50ms,  240位时间 */
			       break;
		
		case 9600: // zzs??? 现在OS_TICKS_PER_SEC = 20，Ticker = 50ms,因此9600波特率以后的几个值都将为0？？？
			       pCtrl_UARTx->RxFrameIntvSet = OS_TICKS_PER_SEC / 50;  /* 20ms,  192位时间 */   
			       break;
		
		case 19200: pCtrl_UARTx->RxFrameIntvSet = OS_TICKS_PER_SEC / 50;  /* 20ms,  384位时间 */
			        break;
		
		case 38400: pCtrl_UARTx->RxFrameIntvSet = OS_TICKS_PER_SEC / 100; /* 10ms,  384位时间 */
			        break;
					
		case 57600: pCtrl_UARTx->RxFrameIntvSet = OS_TICKS_PER_SEC / 100; /* 10ms,  576位时间 */
			        break;

		default: pCtrl_UARTx->RxFrameIntvSet = 9;
		         break; 
	}

	pCtrl_UARTx->RxNum = 0;
	pCtrl_UARTx->TxBusy = 0;            // 忙碌情况，置初值
	pCtrl_UARTx->TxCompletedCnt = 0;    // 初始化为0
	pCtrl_UARTx->RxOffset = 0;           
	pCtrl_UARTx->RxPointer = 0;
	pCtrl_UARTx->FrameRxIntv = 0;
	#if 0
	pCtrl_UARTx->BspMsg[0].MsgID = BSP_MSGID_UART_RXOVER;
	#else   // zzs modified it like this 2018.05.10
	if(ComPort==1)  pCtrl_UARTx->BspMsg[0].MsgID = BSP_MSGID_RFDataIn;
    if(ComPort==2)  pCtrl_UARTx->BspMsg[0].MsgID = BSP_MSGID_RS485DataIn;
	if(ComPort==3)  pCtrl_UARTx->BspMsg[0].MsgID = BSP_MSGID_UART_RXOVER;
	#endif
	pCtrl_UARTx->BspMsg[0].DivNum  = ComPort;
	pCtrl_UARTx->BspMsg[0].DataLen = 0xFFFF;
	
	#if 0
	pCtrl_UARTx->BspMsg[1].MsgID = BSP_MSGID_UART_RXOVER;
	#else   // zzs modified it like this 2018.05.10
	if(ComPort==1)  pCtrl_UARTx->BspMsg[1].MsgID = BSP_MSGID_RFDataIn;
	if(ComPort==2)  pCtrl_UARTx->BspMsg[1].MsgID = BSP_MSGID_RS485DataIn;
	if(ComPort==3)  pCtrl_UARTx->BspMsg[1].MsgID = BSP_MSGID_UART_RXOVER;
	#endif
	pCtrl_UARTx->BspMsg[1].DivNum = ComPort;
	pCtrl_UARTx->BspMsg[1].DataLen= 0xFFFF;
	
	pCtrl_UARTx->TxIntv = 0;
	pCtrl_UARTx->TxIntvSet = pCtrl_UARTx->RxFrameIntvSet + 1;
	USART_Init( pUARTx, &USART_InitStructure);
	if(ComPort < 4)	 /* uart4, uart5 not support */
		USART_ClockInit( pUARTx,&USART_Clock_InitStructure);  
	
	/* Enable the USART Transmoit interrupt: this interrupt is generated when the
	   USART transmit data register is empty after send */
	if(ComPort != 4) USART_ITConfig( pUARTx, USART_IT_TC, ENABLE);  // zzs note,发送中断使能

	/* Enable the USART Receive interrupt: this interrupt is generated when the
	   USART receive data register is not empty */
	USART_ITConfig( pUARTx, USART_IT_RXNE, ENABLE);    // zzs note,接收中断使能

	USART_Cmd( pUARTx,ENABLE);				// Enable USARTx
	OS_EXIT_CRITICAL();
	
	return TRUE;
}

/************************************************************************************************************************
* Function Name:  void SYS_UART_ISR(INT8U ComPort)   
* Description  :  串口中断服务子程序，字节中断。
*
* Input        :  ComPort  : 串口号 
*                 隐式输入 ：串口号对应的管理结构体
*
* Return       :  None
*************************************************************************************************************************/
void SYS_UART_ISR(INT8U ComPort)
{
	FlagStatus TxStatus = RESET;
	INT8U RcvData = 0;
 	UARTx_Ctrl_Struct *pCtrl_UARTx = NULL;
 	USART_TypeDef * pUARTx = NULL;

	pUARTx = GetCommHandle(ComPort);
	if(pUARTx == 0) return ;
	
    pCtrl_UARTx = &UARTx_Ctrl_Array[ComPort-1];
	
    /* 半双工模式, 最后发送字节发\收中断顺序到来 */
	TxStatus = USART_GetITStatus(pUARTx, USART_IT_TC);
 	
	/* 接收中断处理 */ 
    if(USART_GetITStatus( pUARTx, USART_IT_RXNE) == SET)
	{
		/* 清除USART接收中断标志 */
		USART_ClearITPendingBit( pUARTx, USART_IT_RXNE);

		if(pCtrl_UARTx->RxNum < pCtrl_UARTx->MaxRxBufLen)
		{
			/* 从UART接收寄存器读取一个字节的数据 */
			RcvData = USART_ReceiveData( pUARTx);
			
			/* 7位数据位时,Cortex最高位校验会读进来，清理掉 */
	        if(pCtrl_UARTx->Setting.DataBits == 7)
			{
				RcvData	&= 0x7f;	
			}
			pCtrl_UARTx->pRecvBuf[pCtrl_UARTx->RxNum] = RcvData;             // zzs note,转移中断接收到的数据(Byte by Byte)
			pCtrl_UARTx->RxNum++;                                            // zzs note,维护计数器
		}
		else
		{   /* 缓冲区溢出处理, 不收数据须由应用层归0 */
			USART_ReceiveData( pUARTx);
		}
			
		if(pCtrl_UARTx->Setting.Mode)
		{
			 if((TxStatus == SET) || pCtrl_UARTx->TxBusy)
			 	pCtrl_UARTx->RxNum = 0;  
		}
	}

	/* 发送中断处理 */ 
	if(TxStatus == SET)
	{
		/* 清除USART发送中断标志 */
		USART_ClearITPendingBit( pUARTx, USART_IT_TC);
		pCtrl_UARTx->TxCompletedCnt++;                  // 发送完成字节数计数器维护更新
		
		/* insert little timing interval */
 					    
		if(pCtrl_UARTx->TxCompletedCnt < pCtrl_UARTx->TxBusy)	/* 检查发送是否完成 */  //TxCompletedCnt发一个字节加1，TxBusy就是传入的串口输出长度
		{
			USART_SendData( pUARTx, pCtrl_UARTx->pSendBuf[pCtrl_UARTx->TxCompletedCnt]);
		}
		else  // zzs note, (pCtrl_UARTx->TxCompletedCnt >= pCtrl_UARTx->TxBusy)
		{     // send all data ok
			 // 初始化时会进来两次中断？？？？？
		     if(pCtrl_UARTx->Setting.Mode == UART_RS485_MODE)    
			 {
			 	MODE485_RxEnable(2);      // zzs??? 这里为什么要写死为2呢？？？？？？？？？？？？？？？？？
				UART_RxEnable(ComPort,1);
			 }
			 
			 if(pCtrl_UARTx->Setting.Mode == UART_HALFDUP_MODE)
			 {
			 	UART_RxEnable(ComPort,1);
			 }
			
             // if(pCtrl_UARTx->TxBusy) pCtrl_UARTx->TxIntv = pCtrl_UARTx->TxIntvSet; /* 帧间隔设置 */
			 pCtrl_UARTx->TxCompletedCnt = 0;    // 归0发送完成字节数计数器
			 pCtrl_UARTx->TxBusy= 0;             // 归0清除忙碌情况统计器
			 pCtrl_UARTx->TxIntv=0;
		}
	}

	return;
}

/************************************************************************************************************************
* Function Name:  void SYS_UART_FSR(INT8U ComPort)    
* Description  :  串口字节流组帧服务程序，按超时位(时间)区分和字节个数区分。
*                 zzs note: 这个串口分帧处理程序，依赖于系统Ticker不断的调用本函数来扫描串口管理结构体中的各个管理变量的状态，
*                           判断何时是一帧的结束，然后抛出消息。
* Input        :  ComPort : 串口号 1...5
*
* Return       :  
*************************************************************************************************************************/
void SYS_UART_FSR(INT8U ComPort)
{
	UARTx_Ctrl_Struct *pCtrl_UARTx = NULL;

	if(ComPort == 0) return ; 
	if(ComPort > 5) return ;
	
	pCtrl_UARTx = &UARTx_Ctrl_Array[ComPort-1];

	/* 无接收缓冲区，返回 */
	if(pCtrl_UARTx->MaxRxBufLen == 0) return;
		
	/* 发送帧间隔   */
//	if(pCtrl_UARTx->MaxTxBufLen && pCtrl_UARTx->TxIntv)   //zzs??? 这个逻辑也要注意，何时将这个发送间隔重新装载的问题！！！
//	{
//		pCtrl_UARTx->TxIntv--;         // 发送间隔维护 ,// zzs note,查明一个问题，现在这个TxIntv的重新装载的地方都被注释掉了？？？？？？？？？？？？？？
//		                                               // 相当于，现在发送间隔机制压根儿就不起作用。 ！！！！！！！！！！！！！！！！！！！！！！！！！！  
//	}

	/* 数据接收分帧处理 */
	if((pCtrl_UARTx->RxFrameIntvSet & 0x8000) == 0)     // zzs note,这个0x8000最好是定义成一个宏，描述清楚：这个是u16的RxFrameIntvSet的Bit15的Mask项，用于区分是用时间来分帧，还是用字节数来分帧。
	{   /* 以时间分帧 */
		if(pCtrl_UARTx->RxPointer == pCtrl_UARTx->RxNum)
		{
			if(pCtrl_UARTx->RxNum) 
			{	/* 未收到最新数据   */       
				pCtrl_UARTx->FrameRxIntv++;    //哦， 老衲明白了，未收到新的数据超过多久多久，即认为是一帧的末尾，“方法很巧妙，行为不可取” ，因为没见着分帧断句协议啊，这个要是做极限测试，这个程序只能呵呵了。
				if(pCtrl_UARTx->FrameRxIntv >= pCtrl_UARTx->RxFrameIntvSet) // 老衲明白了 “正常字节间的时间”
				{
					if(pCtrl_UARTx->RxOffset != pCtrl_UARTx->RxPointer)
					{   /* 不 是 空 帧 */
						OSSchedLock();    // 进入非调度区
						SendReceiveMsg(ComPort,pCtrl_UARTx);
						OSSchedUnlock();  // 退出非调度区
						pCtrl_UARTx->FrameRxIntv = 0;			
					}
				}
			}
		}
		else
		{ /* 收到最新数据   */
			pCtrl_UARTx->RxPointer = pCtrl_UARTx->RxNum;   // 收到新的n = 1...x个数据，则立马维护RxPointer，为什么说是1...x个，是因为ticker中断一个tick = xx ms下来，有可能一整帧数据都收完了。
			pCtrl_UARTx->FrameRxIntv = 0;
		}
	}  
	else
	{ /*以字节数分帧 */
		if(pCtrl_UARTx->RxNum != pCtrl_UARTx->RxOffset)
		{
			if((pCtrl_UARTx->RxNum - pCtrl_UARTx->RxOffset >= (pCtrl_UARTx->RxFrameIntvSet & 0x3fff)) || 
			(pCtrl_UARTx->RxNum >= pCtrl_UARTx->MaxRxBufLen))
			{
				pCtrl_UARTx->RxPointer = pCtrl_UARTx->RxOffset + (pCtrl_UARTx->RxFrameIntvSet & 0x3fff);
				if(pCtrl_UARTx->RxPointer >= pCtrl_UARTx->MaxRxBufLen)
				{/* 缓冲区不够的情况 */
					pCtrl_UARTx->RxPointer = pCtrl_UARTx->MaxRxBufLen;
					pCtrl_UARTx->RxNum = pCtrl_UARTx->MaxRxBufLen;
				}
				
				SendReceiveMsg(ComPort,pCtrl_UARTx);
			}
		}
	}

	return;
}

/************************************************************************************************************************
* Function Name:  INT8U BspUartWrite(INT8U ComPort, INT8U *pFrameBuf, INT16U FrameLen)
* Description  :  往串口发送一帧数据。若前一帧未发完，附加到前一帧数据末尾发,若缓冲区长度不够，本帧数据不发送，返回失败。帧与帧之间默认有
*                 一定的时间间隔。时间间隔可以 BSP_UART_TxIntvSet 设置，设0为无间隔。
*
* Input        :  ComPort   : 串口号 1...5
*                 pFrameBuf : 要发送出去的内容帧缓冲首地址
*                 FrameLen  ：要发送的长度
*
* Return       :  1 : 输入参数错误
*                 2 ：串口忙碌中，请稍后再试
*                 0 ：成功
*************************************************************************************************************************/
INT8U BspUartWrite(INT8U ComPort, INT8U *pFrameBuf, INT16U FrameLen)
{
	UARTx_Ctrl_Struct *pCtrl_UARTx = NULL;
	USART_TypeDef * pUARTx = NULL;
	
#ifdef B485DIS 		/*注意，没初始化485时万万不可发送485数据，否则TxBusy非空会导致无法进入STOP模式*/
	if(!(TaskActive & Local_ACT))	//LOCAL已经结束
		if(ComPort==2) return 0;
#endif	

	pUARTx = GetCommHandle(ComPort);
	//if(pUARTx == 0) return 0;		   
	if(pUARTx == 0) return 1;   // zzs modified it like this 2018.1.30	 	   
	
    pCtrl_UARTx = &UARTx_Ctrl_Array[ComPort-1];
 	OS_ENTER_CRITICAL();
	
	
	#if 1  // zzs commented this
	if( (FrameLen > (pCtrl_UARTx->MaxTxBufLen - pCtrl_UARTx->TxBusy)) || (FrameLen == 0)  )   //|| BSP_UART_TxState(ComPort)
	
	{   // zzs???,这里有问题，需要改进！！！不单只是失败返回就完了，没见着在哪儿处理 缓冲区长度不够 的问题啊？？？
    	OS_EXIT_CRITICAL();
		return FALSE;
	}
	#else
	if(FrameLen == 0)  return 1;   // 参数错误
	
	if( FrameLen > (pCtrl_UARTx->MaxTxBufLen - pCtrl_UARTx->TxBusy) )  return 2;  // 串口在忙
    
	if( BSP_UART_TxState(ComPort) )	 return 2; // 串口在忙，重复的逻辑，很有意思吗？
	#endif
	
	if(pCtrl_UARTx->Setting.Mode == UART_RS485_MODE)
	{
		MODE485_TxEnable(2);
	    UART_RxEnable(ComPort,0);
	}
	
	if(pCtrl_UARTx->Setting.Mode == UART_HALFDUP_MODE)
	{
		UART_RxEnable(ComPort,0);
	}
	
	if(pCtrl_UARTx->TxBusy == 0)    // zzs note,这个的意思说，有发送需求到来时，串口本身 就 处在空闲状态，
	{/* 空闲状态 */		
		pCtrl_UARTx->TxBusy = FrameLen;  // zzs note,串口这会儿开始要忙起来了.
		pCtrl_UARTx->TxCompletedCnt = 0;
		memcpy((void *)(pCtrl_UARTx->pSendBuf), pFrameBuf, FrameLen);
		
		/* Send the first data with pUARTx, and the next data send
	       by ISR automatic */
 		USART_SendData(pUARTx, pCtrl_UARTx->pSendBuf[0]);   // zzs note！！！注意，INT8U的入参，传给一个u16的参数，这个是没有任何问题的。
	}
	else   // zzs note, 这个的意思是说，“哎，我这儿正忙着呢，后面排队去吧。”
	{
		/* UART数 据 正 在 发 送  */
		memcpy((void *)(pCtrl_UARTx->pSendBuf + pCtrl_UARTx->TxBusy), pFrameBuf, FrameLen);
		pCtrl_UARTx->TxBusy += FrameLen;   // zzs note,串口发送忙不及的情况下就会出现这个样子啰。
	}

	OS_EXIT_CRITICAL();
	
	// return TRUE;
	return 0;         // zzs modified it like this 2018.1.30
	
}


/*******************************************************************************
* Function Name  : BSP_UART_TxAbort
* Description    : 当前帧发送终止
* Input          : ComPort : 串口号1...7
* Output         : None
* Return         : None
*******************************************************************************/ 
//void  BSP_UART_TxAbort(INT8U ComPort)    // zzs commented it ,because of this function didn't invoked in anywhere.
//{
//	UARTx_Ctrl_Struct *pCtrl_UARTx = NULL;
//    if(ComPort>5) return ;
//	if(ComPort==0)return ;
//    pCtrl_UARTx=&UARTx_Ctrl_Array[ComPort-1];

//	OS_ENTER_CRITICAL();
//	pCtrl_UARTx->TxBusy = 0;
//	pCtrl_UARTx->TxCompletedCnt = 0;
//	pCtrl_UARTx->TxIntv = pCtrl_UARTx->TxIntvSet + 4; // 移位寄存器时间4
//	OS_EXIT_CRITICAL();
// 
//	/* 等待移位寄存器空 */
//	OSTimeDly(4);
//	
//	if(pCtrl_UARTx->Setting.Mode == UART_RS485_MODE)
//	{
//		    MODE485_RxEnable(2);
// 			UART_RxEnable(ComPort,1);
//	}
//	if(pCtrl_UARTx->Setting.Mode == UART_HALFDUP_MODE)
//	{
//	 
//			UART_RxEnable(ComPort,1);
//	} 
//}

/******************************************************************************
* Function Name: BSP_UART_RxAll
* Description: 串口接收缓冲区所有数据读入。数据流接收时自动从缓冲区0地址开始重
*              新接收数据。
* Input:  ComPort : 串口号1...7        
* Output: Nothing
* Return: struct Str_Msg 消息指针
******************************************************************************/
//struct Str_Msg  BSP_RxAllMsg[5];                 // zzs commented it ,because of this function didn't invoked in anywhere.
//struct Str_Msg  *BSP_UART_RxAll(INT8U ComPort)
//{
//	UARTx_Ctrl_Struct *pCtrl_UARTx = NULL;
//    if(ComPort > 5) return 0;
//	if(ComPort == 0)return 0;
//	
// 	pCtrl_UARTx=&UARTx_Ctrl_Array[ComPort-1];

// 	OS_ENTER_CRITICAL();
//	pCtrl_UARTx->BspMsg[0].DataLen = 0xFFFF;
//		
//	/* 填 充 消 息 */			
//	BSP_RxAllMsg[ComPort -1].MsgID  = BSP_MSGID_UART_RXOVER;
//	BSP_RxAllMsg[ComPort -1].DivNum = ComPort;
//	BSP_RxAllMsg[ComPort -1].pData  = (void *)(pCtrl_UARTx->pRecvBuf + pCtrl_UARTx->RxOffset);
//	BSP_RxAllMsg[ComPort -1].DataLen= pCtrl_UARTx->RxNum - pCtrl_UARTx->RxOffset;

//	/* 下一帧起始地址 */
//	if(pCtrl_UARTx->RxOffset == 0)
//	{/* 缓冲区前半部分  */
//		pCtrl_UARTx->RxOffset = pCtrl_UARTx->RxNum;
//		pCtrl_UARTx->RxPointer= pCtrl_UARTx->RxNum; 
//	}
//	else
//	{/* 缓冲区后半部分 */
//		pCtrl_UARTx->RxOffset = 0;
//		pCtrl_UARTx->RxNum = 0;
//		pCtrl_UARTx->RxPointer = 0;
//	}

//	OS_EXIT_CRITICAL();
//	return(&BSP_RxAllMsg[ComPort -1]);
//}

/************************************************************************************************************************
* Function Name:  void  BSP_UART_RxClear(INT8U ComPort)
* Description  :  串口接收缓冲区清空。
*                 消息清空，向消息容器的 DataLen字段写 0xFFFF，即认为是象擦黑板一样一样的啦。
*
* Input        :  ComPort   : 串口号 1...5
*                
* Return       :  None
*************************************************************************************************************************/
void  BSP_UART_RxClear(INT8U ComPort)
{
	UARTx_Ctrl_Struct *pCtrl_UARTx = NULL;
	if(ComPort >5) return ;
	if(ComPort == 0) return ;
	
 	pCtrl_UARTx = &UARTx_Ctrl_Array[ComPort-1];
  	/* 普 通 串 口 清 空 */
	OS_ENTER_CRITICAL();
	pCtrl_UARTx->RxNum = 0;
	pCtrl_UARTx->RxOffset = 0;
	pCtrl_UARTx->RxPointer = 0;
	pCtrl_UARTx->FrameRxIntv = 0;

	/* 清空消息 */
	pCtrl_UARTx->BspMsg[0].DataLen = 0xFFFF;
	pCtrl_UARTx->BspMsg[0].pData	= (INT8U *)pCtrl_UARTx->pRecvBuf;
	pCtrl_UARTx->BspMsg[1].DataLen = 0xFFFF;
	pCtrl_UARTx->BspMsg[1].pData	= (INT8U *)pCtrl_UARTx->pRecvBuf;
	OS_EXIT_CRITICAL();
}   

/******************************************************************************
* Function Name: BSP_UART_FrameIntv
* Description: 串口接收帧间隔设置
* Input:  ComPort : 串口号1...5
*         IntvMS_NumBytes: bit15 = 0时	bit13~0	表示分帧时间,以TICK为单位
*                          bit15 = 1时	bit13~0	表示分帧字节数，字节为单位
*
* Output: Nothing
* Return: 0 失败 / 其它：返回原来的设置。
******************************************************************************/
//INT16U BSP_UART_FrameIntv(INT8U ComPort,INT16U IntvMS_NumBytes)   // zzs commented it ,because of this function didn't invoked in anywhere.
//{
//	UARTx_Ctrl_Struct *pCtrl_UARTx = NULL;
//	INT16U OrignSet = 0;

//	if(IntvMS_NumBytes == 0)
//		return 0;

//	if(ComPort > 5) return 0;
//	if(ComPort == 0) return 0;
// 	pCtrl_UARTx = &UARTx_Ctrl_Array[ComPort-1];

// 	if(IntvMS_NumBytes & 0x8000)
//	{
//		if((IntvMS_NumBytes & 0x3fff) >= pCtrl_UARTx->MaxRxBufLen)
//			return 0;
//	}

//	BSP_UART_RxClear(ComPort);

//	OS_ENTER_CRITICAL();
//	OrignSet = pCtrl_UARTx->RxFrameIntvSet;
//    pCtrl_UARTx->RxFrameIntvSet = IntvMS_NumBytes;
//	OS_EXIT_CRITICAL();

//	return OrignSet;
//}

/******************************************************************************
* Function Name: BSP_UART_TxIntvSet
* Description: 串口发送帧间隔设置
* Input:  ComPort : 串口号1...7
*         IntvMS: bit15 = 0时	bit13~0	表示分帧时间TICK为单位
*
* Output: Nothing
* Return: 0xFFFF 失败 / 其它：返回原来的设置。
******************************************************************************/
//INT16U BSP_UART_TxIntvSet(INT8U ComPort,INT16U IntvMS)   // zzs commented it ,because of this function didn't invoked in anywhere.
//{
//	UARTx_Ctrl_Struct *pCtrl_UARTx = NULL;
//	INT16U OrignSet = 0;
//	
//	if(ComPort > 5) return 0;
//	if(ComPort == 0) return 0;
//	
// 	pCtrl_UARTx=&UARTx_Ctrl_Array[ComPort-1];
// 	BSP_UART_TxAbort(ComPort);

//	OS_ENTER_CRITICAL();
//	OrignSet = pCtrl_UARTx->TxIntvSet;
//    pCtrl_UARTx->TxIntvSet = (IntvMS & 0x3fff);
//	OS_EXIT_CRITICAL();

//	return OrignSet;
//}

/************************************************************************************************************************
* Function Name:  INT8U BSP_UART_TxState(INT8U ComPort)
* Description  :  获取串口发送状态
*
* Input        :  ComPort   : 串口号 1...5
*
* Return       :  1 send busy / 0 send free     
*************************************************************************************************************************/
INT8U BSP_UART_TxState(INT8U ComPort)     // zzs note！！！这个函数的返回逻辑有问题，现在只能寄希望于输入形参 ComPort不出错的情况下，
{                                         // 返回逻辑是可以正常工作的 ，这个得改。
	UARTx_Ctrl_Struct * pCtrl_UARTx = NULL;
	
	if(ComPort > 5) return 0;
	if(ComPort == 0) return 0;
 	
	pCtrl_UARTx = &UARTx_Ctrl_Array[ComPort-1];
 	
	if(pCtrl_UARTx->TxCompletedCnt/*FIFO未空*/|| pCtrl_UARTx->TxBusy || pCtrl_UARTx->TxIntv)
		return 1;
	else
		return 0;
}

/************************************************************************************************************************
* Function Name:  INT8U BSP_UART_Close(INT8U ComPort)
* Description  :  关闭对应的串口
*
* Input        :  ComPort   : 串口号 1...5
*
* Return       :  1: 默认返回成功               
*************************************************************************************************************************/
INT8U BSP_UART_Close(INT8U ComPort)
{	
	USART_TypeDef * pUARTx = NULL;
	
	while(BSP_UART_TxState(ComPort));
	
	switch(ComPort)
	{
		case 1: pUARTx = USART1;
				UART1_PIN_CLOSE();
			    break;
		case 2: pUARTx    = USART2;		
//				UART2_PIN_CLOSE();
				B485_LowPower();         //修改485低功耗引脚配置
			    break;
		case 3:	pUARTx    = USART3;
				UART3_PIN_CLOSE();
			    break; 
		
		default: break;
	}
	
	USART_ITConfig( pUARTx, USART_IT_TC, DISABLE);
	USART_ITConfig( pUARTx, USART_IT_RXNE, DISABLE);
	USART_Cmd( pUARTx,DISABLE);			 
	
	return 1;
}
/*******************************************************************************
* Function Name  : INT8U BSP_UART_Close(INT8U ComPort)
* Description    : 关闭对应的串口
* Input          : ComPort:    1 ... 7
*                  Settings:   串口配置
*              	   Mail_Queue：邮箱或队列指针
* Output         : None
* Return         : 1 成功 / 0 失败 
*******************************************************************************/
//INT8U BSP_UART_LP(INT8U ComPort)    // zzs commented it ,because of this function didn't invoked in anywhere.
//{	
//	BSP_UART_Close(ComPort);
//	switch(ComPort)
//	{
//		case 1: RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 ,DISABLE);
//		        break;
//		
//		case 2: RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, DISABLE);
//		        break;
//		
//		case 3: RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3 ,DISABLE);
//		        break;
//		
//		default: break;
//	}
//	
//	return 1;
//}

/************************************************************************************************************************
* Function Name:  INT8U * BSP_UART_GetRecvLen(INT8U ComPort,INT16U *RLEN)             
* Description  :  返回串口接收到的数据的长度，及首地址
* Input        :  ComPort : 串口号 ，取值范围 1,2,3
*
* Return       :  明式返回，UARTx_Ctrl_Array[ComPort-1].pRecvBuf ：ComPort号串口收到的数据的首地址
*                 形参返回，UARTx_Ctrl_Array[ComPort-1].RxPointer : 这个返回，已经是一个长度数据了，而不是指针，不要被英文字面意思弄糊涂了。
*************************************************************************************************************************/
INT8U * BSP_UART_GetRecvLen(INT8U ComPort,INT16U *RLEN)
{
	if(ComPort > 3) return 0;
	if(ComPort == 0) return 0;
	
	*RLEN = UARTx_Ctrl_Array[ComPort-1].RxPointer;
	
	return (INT8U * )UARTx_Ctrl_Array[ComPort-1].pRecvBuf;
}

/******************************************************************************
* Function Name: void Power485Pin_Init(void)
* Description:   与485电源相关的引脚配置
								 
* Input:  Nothing
* Output: Nothing
* Contributor: GuoWei Dong
* Date First Issued: 2010-01-26
******************************************************************************/
void Power485Pin_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);						//开时钟
	PWDC485DIS();																//设置为关闭		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =PWDC485_PIN;
	GPIO_Init(PWDC485_Port, &GPIO_InitStructure);
}

/******************************************************************************
* Function Name: SYS_UART_DeInit
* Description: 复位外部串口， 扩展串口芯片硬件复位
* Input:  Nothing       
* Output: Nothing
* Return: Nothing
******************************************************************************/
//void SYS_UART_DeInit(void)     // zzs commented it ,because of this function didn't invoked in anywhere.
//{
// 	UARTx_Ctrl_Array[0].Setting.Mode = 0xFF;
//	UARTx_Ctrl_Array[0].MaxRxBufLen = 0;
//	UARTx_Ctrl_Array[0].MaxTxBufLen = 0;

//	UARTx_Ctrl_Array[1].Setting.Mode = 0xFF;
//	UARTx_Ctrl_Array[1].MaxRxBufLen = 0;
//	UARTx_Ctrl_Array[1].MaxTxBufLen = 0;

//	UARTx_Ctrl_Array[2].Setting.Mode = 0xFF;
//	UARTx_Ctrl_Array[2].MaxRxBufLen = 0;
//	UARTx_Ctrl_Array[2].MaxTxBufLen = 0;

//	UARTx_Ctrl_Array[3].Setting.Mode = 0xFF;
//	UARTx_Ctrl_Array[3].MaxRxBufLen = 0;
//	UARTx_Ctrl_Array[3].MaxTxBufLen = 0;

//	UARTx_Ctrl_Array[4].Setting.Mode = 0xFF;
//	UARTx_Ctrl_Array[4].MaxRxBufLen = 0;
//	UARTx_Ctrl_Array[4].MaxTxBufLen = 0;

// }
