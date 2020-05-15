#include "Bsp_UART.h"
 
/*
   ����ʹ��˵��
   1--> ��Դ�����
   2--> 485 /232 ��ӡ ����ֻ������һ
   3--> ��ͷ 	  
   4--> 3G ������ͨѶģ��
   5--> ���� 
*/

UARTx_Ctrl_Struct   UARTx_Ctrl_Array[5];

/* --------------------Private functions------------------------------------------------------*/
/************************************************************************************************************************
* Function Name : void B485_init(unsigned int rate)
* Description   : B485���ڳ�ʼ��������B485DIS�궨���Զ��жϣ���
* Input         : rate : ������
*
* Return        : None
*************************************************************************************************************************/
void B485_init(unsigned int rate)
{
    UARTx_Setting_Struct UARTInit = {0};	
    
#ifdef B485DIS 		/*ע�⣬û��ʼ��485ʱ���򲻿ɷ���485���ݣ�����TxBusy�ǿջᵼ���޷�����STOPģʽ*/
	if(!(TaskActive & Local_ACT)) return;										//LOCAL�Ѿ�����
#endif	
    if(GyBOX == NULL) GyBOX = OSMboxCreate(0);	
	else GyBOX->OSEventPtr= (void *)0;											//����Ϣ���䣬����ᵼ������ ZE
	
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
	PWDC485EN();       															//��485�����Դ
	OSTimeDly(2);																//�ȴ���Դ�ȶ�
}

/******************************************************************************* 
* Function Name  : void B485_LowPower(void)
* Description    : 485����͹��ģ�������ӦIO�����͹��Ĵ����ر�U2ʱ�Ӽ�����ʱ�ӡ�
					�ж��������ѹرգ��رտ���Ч���͹���
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/
void B485_LowPower(void)													//�ڹرմ���֮ǰһ��Ҫ��֤485�����ɣ����busy��־λ�����򽫵��²��ٽ���STOPģʽ��ֱ���´γ�ʼ����ա�
{
	INT8U WaitTime = 100; 													//485�ȴ���ʱʱ��5��
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	/*�ȴ�485�������*/	
	while( BSP_UART_TxState(2) && (WaitTime--) ) OSTimeDly(1);				//���������ݵĹ��̵���ʱ�ȴ������5��
	if(WaitTime==0xff)
	{
		B485_init(38400);													//���³�ʼ������TxBusy����ֹ�޷�����STOPģʽ
		PWDC485EN();														// DCDC����485��				
		OSTimeDly(10);
		BspUartWrite(2,SIZE_OF("���棺B485_LowPower�ȴ���ʱ��----------------------------------\r\n"));
		OSTimeDly(10);
	}
	
//	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, DISABLE );					//���ظ���ʱ��|RCC_APB2Periph_AFIO�������ں�AD���õ������
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2, DISABLE );				//�ر�U2ʱ��
	
	
	/*485EN��TX��RXģ������*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;							//ģ������
	
	GPIO_InitStructure.GPIO_Pin = EN485_PIN;								//485EN��PB15��
	GPIO_Init(EN485_Port, &GPIO_InitStructure);								//
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;								//USART2 Tx (PA2)
	GPIO_Init(GPIOA, &GPIO_InitStructure);									//
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;								//USART2 Rx (PA3) 
	GPIO_Init(GPIOA, &GPIO_InitStructure);									//
	
	
	/*485��Դʹ���������*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;						//�������
	PWDC485DIS();															//�ص�Դ
	GPIO_InitStructure.GPIO_Pin = PWDC485_PIN;								//485��Դʹ�ܿ�
	GPIO_Init(PWDC485_Port, &GPIO_InitStructure);							//
	
	/*���ж�*/
	USART_ITConfig( USART2, USART_IT_TC, DISABLE);
	USART_ITConfig( USART2, USART_IT_RXNE, DISABLE);
	USART_Cmd( USART2,DISABLE);	
}

/************************************************************************************************************************
* Function Name : void UART1_PIN_CFG()  
* Description   : UART1��Tx�� Rx pin�����ã�ʹ�����ߣ����ùܽŹ���ģʽ�����ʵȲ�����
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
* Description   : UART1��Tx�� Rx pin�Źرգ����ܣ�
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
* Description   : UART2��Tx�� Rx pin�����ã�ʹ�����ߣ����ùܽŹ���ģʽ�����ʵȲ�����
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

	/*485EN��PB15��*/
	GPIO_InitStructure.GPIO_Pin = EN485_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(EN485_Port, &GPIO_InitStructure);
}

/************************************************************************************************************************
* Function Name : void UART2_PIN_CLOSE()
* Description   : UART2��Tx�� Rx pin�Źرգ����ܣ�
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

	/*485EN��PB15��*/
	GPIO_InitStructure.GPIO_Pin = EN485_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(EN485_Port, &GPIO_InitStructure);
}

/************************************************************************************************************************
* Function Name : void UART3_PIN_CFG()
* Description   : UART3��Tx�� Rx pin�����ã�ʹ�����ߣ����ùܽŹ���ģʽ�����ʵȲ�����
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
* Description   : UART3��Tx�� Rx pin�Źرգ����ܣ�
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
* Description   : UART4��Tx�� Rx pin�����ã�ʹ�����ߣ����ùܽŹ���ģʽ�����ʵȲ�����
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
* Description   : UART5��Tx�� Rx pin�����ã�ʹ�����ߣ����ùܽŹ���ģʽ�����ʵȲ�����
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
* Description   : ���ںŵ��ṹ���ӳ�� 
*
* Input         : ComPort : 1...5
*
* Return        : �ǿ�ֵ �� 1...5�Ŵ��ڶ�Ӧ����Դ���
*                 NULL   : ��δ���
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
* Description   : RS485оƬ����ʹ�ܺ�����zzs note��RS485ģʽ�����485��ƽת��оƬ. ����������ʱʹ�ܷ��ͣ�һ֡����
*                 ������ϣ�����ʹ�ܽ��գ����ͽ�ֹ��
*
* Input         : Port : ���Port����������ֻ���������Ӱ��ˣ���Ϊ�������ˣ�ֻ����2
*
* Return        : None
*************************************************************************************************************************/
__inline void  MODE485_RxEnable(INT8U port)
{
	// INT16U i = 0;
	
	if( port ==  2)
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_15);  // PSAM д��ʹ��
		// for(i=0;i<100;i++) {;}  // zzs commented this ,���������У�������ʹ��ѭ���塢switch���ݹ飬���򽫰�����ͨ��������
	}	
}

/************************************************************************************************************************
* Function Name : __inline void  MODE485_TxEnable(INT8U port)
* Description   : RS485��������ʹ��
*
* Input         : Port : ���Port����������ֻ���������Ӱ��ˣ���Ϊ�������ˣ�ֻ����2
*
* Return        : None
*************************************************************************************************************************/
__inline void  MODE485_TxEnable(INT8U port)
{
	// INT16U i = 0;
	
	if(port == 2)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_15);   // 485����ʹ��
	}
	
	// for(i=0;i<100;i++) {;}  // zzs commented this ,���������У�������ʹ��ѭ���塢switch���ݹ飬���򽫰�����ͨ��������
}


/************************************************************************************************************************
* Function Name : __inline void  UART_RxEnable(INT8U Com,INT8U Enable)
* Description   : ��������ʹ��,���߳���
*
* Input         : Com    : ���ں�
*                 Enable : 1 ������ʹ��
*                          0 �����ճ���
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
* Description   : ��������ֻ�Ǹ��ɻ�ģ���ϵͳTicker�жϵ��÷�֡ɨ�躯�������ɷ�֡ɨ�躯�����ñ�����post receive msg��
*                 ������ִ�С����յ�һ֡�������ݡ���Ϣ��װ����䣬ά��һ�´��ڹ�������Ȼ��Դ��ڰ󶨵�����ִ��Post������
*
* Input         : ComPort :  Ҫ��ʼ���Ĵ��ںţ� 1 ... 5
*                 pCtrl_UARTx :  ���ڹ���ṹ��
*                 
*
* Return        : ��ʽ���أ��׳�һ����Ϣ����Ϣ������ ���λ�� pCtrl_UARTx->pRecvBuf + pCtrl_UARTx->RxOffset
*************************************************************************************************************************/
void SendReceiveMsg(INT8U ComPort,UARTx_Ctrl_Struct *pCtrl_UARTx)
{
	struct Str_Msg *TmpMsg = (struct Str_Msg *)0;
	
	
	if(pCtrl_UARTx->BspMsg[0].DataLen == 0xFFFF)   // zzs??? ��0xFFFF����Ϊ�ǿ��õġ� ��ע�� ��Ҫ�������� DataLen �� ==0xFFFF�أ����� 
		TmpMsg = &pCtrl_UARTx->BspMsg[0];
	
	if(pCtrl_UARTx->BspMsg[1].DataLen == 0xFFFF)  // zzs??? �Բۣ���ʵ֤������������DataLen�� = 0xFFFF ������Ҫ�����أ�����
		TmpMsg = &pCtrl_UARTx->BspMsg[1];
	
	if((INT32U) TmpMsg == 0)
		return;
	
	/* �� �� �� Ϣ */		
	TmpMsg->DivNum  = ComPort ;                      // �豸�ţ�ʵ�ʾ���ָ����ͬ�Ĵ��ںţ������ֲ�ͬ���豸�� 
	TmpMsg->pData   = (void *)(pCtrl_UARTx->pRecvBuf + pCtrl_UARTx->RxOffset);                            // zzs!!!��Ϣ���ݵ�ʵ��װ��λ��
	TmpMsg->DataLen = pCtrl_UARTx->RxPointer - pCtrl_UARTx->RxOffset;           // ָ����Ϣ�ĳ��� 
	if(ComPort == 1)	TmpMsg->MsgID   = BSP_MSGID_RFDataIn;        	 //  Rf���ڽ��������ϢID	
	if(ComPort == 2)	TmpMsg->MsgID   = BSP_MSGID_RS485DataIn;  
	if(ComPort == 3)	TmpMsg->MsgID   = BSP_MSGID_UART_RXOVER;         // GPRS ���ڽ��������ϢID

	/* ��һ֡��ʼ��ַ */
	pCtrl_UARTx->RxOffset = pCtrl_UARTx->RxPointer;   // ����ά��RxOffset,����һ��λ��ά����RxOffset
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
* Description   : ���ڳ�ʼ��
* Input         : ComPort :  Ҫ��ʼ���Ĵ��ںţ� 1 ... 5
*                 pUartSet :  ���ڵ����ò�����
*                 Mail_Queue : ����������䱸һ�����䣬��������ָ��	����GPRSBOX�Ļ�����ֱ����UARTx_Ctrl_Array[x]->MailOrQueueȥ��ʼ������
*
* Return        : 0 : �β�����
*                 1 ���ɹ�����
*************************************************************************************************************************/
INT8U BSP_UART_Init(INT8U ComPort, UARTx_Setting_Struct *pUartSet, OS_EVENT *Mail_Queue)
{
	USART_InitTypeDef	USART_InitStructure = {0};
	USART_ClockInitTypeDef	USART_Clock_InitStructure = {0};
	UARTx_Ctrl_Struct *pCtrl_UARTx = NULL;
	USART_TypeDef * pUARTx = NULL;

	/* ������Ч�Լ�� */
	if(ComPort <= 5 && (pUartSet->DataBits < 7 || pUartSet->DataBits > 8))
		return 0;
	if(pUartSet->Parity > 2)
		return 0;
	if(pUartSet->StopBits < 1 || pUartSet->StopBits > 2)
		return 0;
	//if(pUartSet->BaudRate < 300 || pUartSet->BaudRate > 57600)
	//	return 0;
	if(pUartSet->DataBits == 7 && pUartSet->Parity == 0 && ComPort <= 5)
		return 0;         /* CORTEX���ڲ�֧��7λ��У�鷽ʽ����Ϊ��У��İɡ���չ����֧�ֵ� */
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

/* Ϊ��֡���ʱ����Ƶľ�ȷ�ȣ�OS_TICKS_PER_SEC Ӧ�ô��ڵ��� 100 */  // zzs note,���ǣ����ڵ�ϵͳ��ƣ�û�дﵽ���Ҫ��
#if OS_TICKS_PER_SEC < 10
   #error OS_TICKS_PER_SEC should bigger than 100 for UART frame
#endif

	switch(pUartSet->BaudRate)
	{
		case 300: pCtrl_UARTx->RxFrameIntvSet = (OS_TICKS_PER_SEC *4)/5;/* 800ms, 240λʱ�� */
			      break;
		
		case 600: pCtrl_UARTx->RxFrameIntvSet = (OS_TICKS_PER_SEC *2)/5;/* 400ms, 240λʱ�� */
			      break;
		
		case 1200: pCtrl_UARTx->RxFrameIntvSet = OS_TICKS_PER_SEC / 5;   /* 200ms, 240λʱ�� */
				   break;
		
		case 2400: pCtrl_UARTx->RxFrameIntvSet = OS_TICKS_PER_SEC / 10;  /* 100ms, 240λʱ�� */
				   break;
		
		case 4800: pCtrl_UARTx->RxFrameIntvSet = OS_TICKS_PER_SEC / 20;  /* 50ms,  240λʱ�� */
			       break;
		
		case 9600: // zzs??? ����OS_TICKS_PER_SEC = 20��Ticker = 50ms,���9600�������Ժ�ļ���ֵ����Ϊ0������
			       pCtrl_UARTx->RxFrameIntvSet = OS_TICKS_PER_SEC / 50;  /* 20ms,  192λʱ�� */   
			       break;
		
		case 19200: pCtrl_UARTx->RxFrameIntvSet = OS_TICKS_PER_SEC / 50;  /* 20ms,  384λʱ�� */
			        break;
		
		case 38400: pCtrl_UARTx->RxFrameIntvSet = OS_TICKS_PER_SEC / 100; /* 10ms,  384λʱ�� */
			        break;
					
		case 57600: pCtrl_UARTx->RxFrameIntvSet = OS_TICKS_PER_SEC / 100; /* 10ms,  576λʱ�� */
			        break;

		default: pCtrl_UARTx->RxFrameIntvSet = 9;
		         break; 
	}

	pCtrl_UARTx->RxNum = 0;
	pCtrl_UARTx->TxBusy = 0;            // æµ������ó�ֵ
	pCtrl_UARTx->TxCompletedCnt = 0;    // ��ʼ��Ϊ0
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
	if(ComPort != 4) USART_ITConfig( pUARTx, USART_IT_TC, ENABLE);  // zzs note,�����ж�ʹ��

	/* Enable the USART Receive interrupt: this interrupt is generated when the
	   USART receive data register is not empty */
	USART_ITConfig( pUARTx, USART_IT_RXNE, ENABLE);    // zzs note,�����ж�ʹ��

	USART_Cmd( pUARTx,ENABLE);				// Enable USARTx
	OS_EXIT_CRITICAL();
	
	return TRUE;
}

/************************************************************************************************************************
* Function Name:  void SYS_UART_ISR(INT8U ComPort)   
* Description  :  �����жϷ����ӳ����ֽ��жϡ�
*
* Input        :  ComPort  : ���ں� 
*                 ��ʽ���� �����ںŶ�Ӧ�Ĺ���ṹ��
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
	
    /* ��˫��ģʽ, ������ֽڷ�\���ж�˳���� */
	TxStatus = USART_GetITStatus(pUARTx, USART_IT_TC);
 	
	/* �����жϴ��� */ 
    if(USART_GetITStatus( pUARTx, USART_IT_RXNE) == SET)
	{
		/* ���USART�����жϱ�־ */
		USART_ClearITPendingBit( pUARTx, USART_IT_RXNE);

		if(pCtrl_UARTx->RxNum < pCtrl_UARTx->MaxRxBufLen)
		{
			/* ��UART���ռĴ�����ȡһ���ֽڵ����� */
			RcvData = USART_ReceiveData( pUARTx);
			
			/* 7λ����λʱ,Cortex���λУ��������������� */
	        if(pCtrl_UARTx->Setting.DataBits == 7)
			{
				RcvData	&= 0x7f;	
			}
			pCtrl_UARTx->pRecvBuf[pCtrl_UARTx->RxNum] = RcvData;             // zzs note,ת���жϽ��յ�������(Byte by Byte)
			pCtrl_UARTx->RxNum++;                                            // zzs note,ά��������
		}
		else
		{   /* �������������, ������������Ӧ�ò��0 */
			USART_ReceiveData( pUARTx);
		}
			
		if(pCtrl_UARTx->Setting.Mode)
		{
			 if((TxStatus == SET) || pCtrl_UARTx->TxBusy)
			 	pCtrl_UARTx->RxNum = 0;  
		}
	}

	/* �����жϴ��� */ 
	if(TxStatus == SET)
	{
		/* ���USART�����жϱ�־ */
		USART_ClearITPendingBit( pUARTx, USART_IT_TC);
		pCtrl_UARTx->TxCompletedCnt++;                  // ��������ֽ���������ά������
		
		/* insert little timing interval */
 					    
		if(pCtrl_UARTx->TxCompletedCnt < pCtrl_UARTx->TxBusy)	/* ��鷢���Ƿ���� */  //TxCompletedCnt��һ���ֽڼ�1��TxBusy���Ǵ���Ĵ����������
		{
			USART_SendData( pUARTx, pCtrl_UARTx->pSendBuf[pCtrl_UARTx->TxCompletedCnt]);
		}
		else  // zzs note, (pCtrl_UARTx->TxCompletedCnt >= pCtrl_UARTx->TxBusy)
		{     // send all data ok
			 // ��ʼ��ʱ����������жϣ���������
		     if(pCtrl_UARTx->Setting.Mode == UART_RS485_MODE)    
			 {
			 	MODE485_RxEnable(2);      // zzs??? ����ΪʲôҪд��Ϊ2�أ���������������������������������
				UART_RxEnable(ComPort,1);
			 }
			 
			 if(pCtrl_UARTx->Setting.Mode == UART_HALFDUP_MODE)
			 {
			 	UART_RxEnable(ComPort,1);
			 }
			
             // if(pCtrl_UARTx->TxBusy) pCtrl_UARTx->TxIntv = pCtrl_UARTx->TxIntvSet; /* ֡������� */
			 pCtrl_UARTx->TxCompletedCnt = 0;    // ��0��������ֽ���������
			 pCtrl_UARTx->TxBusy= 0;             // ��0���æµ���ͳ����
			 pCtrl_UARTx->TxIntv=0;
		}
	}

	return;
}

/************************************************************************************************************************
* Function Name:  void SYS_UART_FSR(INT8U ComPort)    
* Description  :  �����ֽ�����֡������򣬰���ʱλ(ʱ��)���ֺ��ֽڸ������֡�
*                 zzs note: ������ڷ�֡�������������ϵͳTicker���ϵĵ��ñ�������ɨ�贮�ڹ���ṹ���еĸ������������״̬��
*                           �жϺ�ʱ��һ֡�Ľ�����Ȼ���׳���Ϣ��
* Input        :  ComPort : ���ں� 1...5
*
* Return       :  
*************************************************************************************************************************/
void SYS_UART_FSR(INT8U ComPort)
{
	UARTx_Ctrl_Struct *pCtrl_UARTx = NULL;

	if(ComPort == 0) return ; 
	if(ComPort > 5) return ;
	
	pCtrl_UARTx = &UARTx_Ctrl_Array[ComPort-1];

	/* �޽��ջ����������� */
	if(pCtrl_UARTx->MaxRxBufLen == 0) return;
		
	/* ����֡���   */
//	if(pCtrl_UARTx->MaxTxBufLen && pCtrl_UARTx->TxIntv)   //zzs??? ����߼�ҲҪע�⣬��ʱ��������ͼ������װ�ص����⣡����
//	{
//		pCtrl_UARTx->TxIntv--;         // ���ͼ��ά�� ,// zzs note,����һ�����⣬�������TxIntv������װ�صĵط�����ע�͵��ˣ���������������������������
//		                                               // �൱�ڣ����ڷ��ͼ������ѹ�����Ͳ������á� ����������������������������������������������������  
//	}

	/* ���ݽ��շ�֡���� */
	if((pCtrl_UARTx->RxFrameIntvSet & 0x8000) == 0)     // zzs note,���0x8000����Ƕ����һ���꣬��������������u16��RxFrameIntvSet��Bit15��Mask�������������ʱ������֡���������ֽ�������֡��
	{   /* ��ʱ���֡ */
		if(pCtrl_UARTx->RxPointer == pCtrl_UARTx->RxNum)
		{
			if(pCtrl_UARTx->RxNum) 
			{	/* δ�յ���������   */       
				pCtrl_UARTx->FrameRxIntv++;    //Ŷ�� ���������ˣ�δ�յ��µ����ݳ�����ö�ã�����Ϊ��һ֡��ĩβ���������������Ϊ����ȡ�� ����Ϊû���ŷ�֡�Ͼ�Э�鰡�����Ҫ�������޲��ԣ��������ֻ�ܺǺ��ˡ�
				if(pCtrl_UARTx->FrameRxIntv >= pCtrl_UARTx->RxFrameIntvSet) // ���������� �������ֽڼ��ʱ�䡱
				{
					if(pCtrl_UARTx->RxOffset != pCtrl_UARTx->RxPointer)
					{   /* �� �� �� ֡ */
						OSSchedLock();    // ����ǵ�����
						SendReceiveMsg(ComPort,pCtrl_UARTx);
						OSSchedUnlock();  // �˳��ǵ�����
						pCtrl_UARTx->FrameRxIntv = 0;			
					}
				}
			}
		}
		else
		{ /* �յ���������   */
			pCtrl_UARTx->RxPointer = pCtrl_UARTx->RxNum;   // �յ��µ�n = 1...x�����ݣ�������ά��RxPointer��Ϊʲô˵��1...x��������Ϊticker�ж�һ��tick = xx ms�������п���һ��֡���ݶ������ˡ�
			pCtrl_UARTx->FrameRxIntv = 0;
		}
	}  
	else
	{ /*���ֽ�����֡ */
		if(pCtrl_UARTx->RxNum != pCtrl_UARTx->RxOffset)
		{
			if((pCtrl_UARTx->RxNum - pCtrl_UARTx->RxOffset >= (pCtrl_UARTx->RxFrameIntvSet & 0x3fff)) || 
			(pCtrl_UARTx->RxNum >= pCtrl_UARTx->MaxRxBufLen))
			{
				pCtrl_UARTx->RxPointer = pCtrl_UARTx->RxOffset + (pCtrl_UARTx->RxFrameIntvSet & 0x3fff);
				if(pCtrl_UARTx->RxPointer >= pCtrl_UARTx->MaxRxBufLen)
				{/* ��������������� */
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
* Description  :  �����ڷ���һ֡���ݡ���ǰһ֡δ���꣬���ӵ�ǰһ֡����ĩβ��,�����������Ȳ�������֡���ݲ����ͣ�����ʧ�ܡ�֡��֮֡��Ĭ����
*                 һ����ʱ������ʱ�������� BSP_UART_TxIntvSet ���ã���0Ϊ�޼����
*
* Input        :  ComPort   : ���ں� 1...5
*                 pFrameBuf : Ҫ���ͳ�ȥ������֡�����׵�ַ
*                 FrameLen  ��Ҫ���͵ĳ���
*
* Return       :  1 : �����������
*                 2 ������æµ�У����Ժ�����
*                 0 ���ɹ�
*************************************************************************************************************************/
INT8U BspUartWrite(INT8U ComPort, INT8U *pFrameBuf, INT16U FrameLen)
{
	UARTx_Ctrl_Struct *pCtrl_UARTx = NULL;
	USART_TypeDef * pUARTx = NULL;
	
#ifdef B485DIS 		/*ע�⣬û��ʼ��485ʱ���򲻿ɷ���485���ݣ�����TxBusy�ǿջᵼ���޷�����STOPģʽ*/
	if(!(TaskActive & Local_ACT))	//LOCAL�Ѿ�����
		if(ComPort==2) return 0;
#endif	

	pUARTx = GetCommHandle(ComPort);
	//if(pUARTx == 0) return 0;		   
	if(pUARTx == 0) return 1;   // zzs modified it like this 2018.1.30	 	   
	
    pCtrl_UARTx = &UARTx_Ctrl_Array[ComPort-1];
 	OS_ENTER_CRITICAL();
	
	
	#if 1  // zzs commented this
	if( (FrameLen > (pCtrl_UARTx->MaxTxBufLen - pCtrl_UARTx->TxBusy)) || (FrameLen == 0)  )   //|| BSP_UART_TxState(ComPort)
	
	{   // zzs???,���������⣬��Ҫ�Ľ�����������ֻ��ʧ�ܷ��ؾ����ˣ�û�������Ķ����� ���������Ȳ��� �����Ⱑ������
    	OS_EXIT_CRITICAL();
		return FALSE;
	}
	#else
	if(FrameLen == 0)  return 1;   // ��������
	
	if( FrameLen > (pCtrl_UARTx->MaxTxBufLen - pCtrl_UARTx->TxBusy) )  return 2;  // ������æ
    
	if( BSP_UART_TxState(ComPort) )	 return 2; // ������æ���ظ����߼���������˼��
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
	
	if(pCtrl_UARTx->TxBusy == 0)    // zzs note,�������˼˵���з���������ʱ�����ڱ��� �� ���ڿ���״̬��
	{/* ����״̬ */		
		pCtrl_UARTx->TxBusy = FrameLen;  // zzs note,����������ʼҪæ������.
		pCtrl_UARTx->TxCompletedCnt = 0;
		memcpy((void *)(pCtrl_UARTx->pSendBuf), pFrameBuf, FrameLen);
		
		/* Send the first data with pUARTx, and the next data send
	       by ISR automatic */
 		USART_SendData(pUARTx, pCtrl_UARTx->pSendBuf[0]);   // zzs note������ע�⣬INT8U����Σ�����һ��u16�Ĳ����������û���κ�����ġ�
	}
	else   // zzs note, �������˼��˵���������������æ���أ������Ŷ�ȥ�ɡ���
	{
		/* UART�� �� �� �� �� ��  */
		memcpy((void *)(pCtrl_UARTx->pSendBuf + pCtrl_UARTx->TxBusy), pFrameBuf, FrameLen);
		pCtrl_UARTx->TxBusy += FrameLen;   // zzs note,���ڷ���æ����������¾ͻ����������ӆ���
	}

	OS_EXIT_CRITICAL();
	
	// return TRUE;
	return 0;         // zzs modified it like this 2018.1.30
	
}


/*******************************************************************************
* Function Name  : BSP_UART_TxAbort
* Description    : ��ǰ֡������ֹ
* Input          : ComPort : ���ں�1...7
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
//	pCtrl_UARTx->TxIntv = pCtrl_UARTx->TxIntvSet + 4; // ��λ�Ĵ���ʱ��4
//	OS_EXIT_CRITICAL();
// 
//	/* �ȴ���λ�Ĵ����� */
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
* Description: ���ڽ��ջ������������ݶ��롣����������ʱ�Զ��ӻ�����0��ַ��ʼ��
*              �½������ݡ�
* Input:  ComPort : ���ں�1...7        
* Output: Nothing
* Return: struct Str_Msg ��Ϣָ��
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
//	/* �� �� �� Ϣ */			
//	BSP_RxAllMsg[ComPort -1].MsgID  = BSP_MSGID_UART_RXOVER;
//	BSP_RxAllMsg[ComPort -1].DivNum = ComPort;
//	BSP_RxAllMsg[ComPort -1].pData  = (void *)(pCtrl_UARTx->pRecvBuf + pCtrl_UARTx->RxOffset);
//	BSP_RxAllMsg[ComPort -1].DataLen= pCtrl_UARTx->RxNum - pCtrl_UARTx->RxOffset;

//	/* ��һ֡��ʼ��ַ */
//	if(pCtrl_UARTx->RxOffset == 0)
//	{/* ������ǰ�벿��  */
//		pCtrl_UARTx->RxOffset = pCtrl_UARTx->RxNum;
//		pCtrl_UARTx->RxPointer= pCtrl_UARTx->RxNum; 
//	}
//	else
//	{/* ��������벿�� */
//		pCtrl_UARTx->RxOffset = 0;
//		pCtrl_UARTx->RxNum = 0;
//		pCtrl_UARTx->RxPointer = 0;
//	}

//	OS_EXIT_CRITICAL();
//	return(&BSP_RxAllMsg[ComPort -1]);
//}

/************************************************************************************************************************
* Function Name:  void  BSP_UART_RxClear(INT8U ComPort)
* Description  :  ���ڽ��ջ�������ա�
*                 ��Ϣ��գ�����Ϣ������ DataLen�ֶ�д 0xFFFF������Ϊ������ڰ�һ��һ��������
*
* Input        :  ComPort   : ���ں� 1...5
*                
* Return       :  None
*************************************************************************************************************************/
void  BSP_UART_RxClear(INT8U ComPort)
{
	UARTx_Ctrl_Struct *pCtrl_UARTx = NULL;
	if(ComPort >5) return ;
	if(ComPort == 0) return ;
	
 	pCtrl_UARTx = &UARTx_Ctrl_Array[ComPort-1];
  	/* �� ͨ �� �� �� �� */
	OS_ENTER_CRITICAL();
	pCtrl_UARTx->RxNum = 0;
	pCtrl_UARTx->RxOffset = 0;
	pCtrl_UARTx->RxPointer = 0;
	pCtrl_UARTx->FrameRxIntv = 0;

	/* �����Ϣ */
	pCtrl_UARTx->BspMsg[0].DataLen = 0xFFFF;
	pCtrl_UARTx->BspMsg[0].pData	= (INT8U *)pCtrl_UARTx->pRecvBuf;
	pCtrl_UARTx->BspMsg[1].DataLen = 0xFFFF;
	pCtrl_UARTx->BspMsg[1].pData	= (INT8U *)pCtrl_UARTx->pRecvBuf;
	OS_EXIT_CRITICAL();
}   

/******************************************************************************
* Function Name: BSP_UART_FrameIntv
* Description: ���ڽ���֡�������
* Input:  ComPort : ���ں�1...5
*         IntvMS_NumBytes: bit15 = 0ʱ	bit13~0	��ʾ��֡ʱ��,��TICKΪ��λ
*                          bit15 = 1ʱ	bit13~0	��ʾ��֡�ֽ������ֽ�Ϊ��λ
*
* Output: Nothing
* Return: 0 ʧ�� / ����������ԭ�������á�
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
* Description: ���ڷ���֡�������
* Input:  ComPort : ���ں�1...7
*         IntvMS: bit15 = 0ʱ	bit13~0	��ʾ��֡ʱ��TICKΪ��λ
*
* Output: Nothing
* Return: 0xFFFF ʧ�� / ����������ԭ�������á�
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
* Description  :  ��ȡ���ڷ���״̬
*
* Input        :  ComPort   : ���ں� 1...5
*
* Return       :  1 send busy / 0 send free     
*************************************************************************************************************************/
INT8U BSP_UART_TxState(INT8U ComPort)     // zzs note��������������ķ����߼������⣬����ֻ�ܼ�ϣ���������β� ComPort�����������£�
{                                         // �����߼��ǿ������������� ������øġ�
	UARTx_Ctrl_Struct * pCtrl_UARTx = NULL;
	
	if(ComPort > 5) return 0;
	if(ComPort == 0) return 0;
 	
	pCtrl_UARTx = &UARTx_Ctrl_Array[ComPort-1];
 	
	if(pCtrl_UARTx->TxCompletedCnt/*FIFOδ��*/|| pCtrl_UARTx->TxBusy || pCtrl_UARTx->TxIntv)
		return 1;
	else
		return 0;
}

/************************************************************************************************************************
* Function Name:  INT8U BSP_UART_Close(INT8U ComPort)
* Description  :  �رն�Ӧ�Ĵ���
*
* Input        :  ComPort   : ���ں� 1...5
*
* Return       :  1: Ĭ�Ϸ��سɹ�               
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
				B485_LowPower();         //�޸�485�͹�����������
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
* Description    : �رն�Ӧ�Ĵ���
* Input          : ComPort:    1 ... 7
*                  Settings:   ��������
*              	   Mail_Queue����������ָ��
* Output         : None
* Return         : 1 �ɹ� / 0 ʧ�� 
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
* Description  :  ���ش��ڽ��յ������ݵĳ��ȣ����׵�ַ
* Input        :  ComPort : ���ں� ��ȡֵ��Χ 1,2,3
*
* Return       :  ��ʽ���أ�UARTx_Ctrl_Array[ComPort-1].pRecvBuf ��ComPort�Ŵ����յ������ݵ��׵�ַ
*                 �βη��أ�UARTx_Ctrl_Array[ComPort-1].RxPointer : ������أ��Ѿ���һ�����������ˣ�������ָ�룬��Ҫ��Ӣ��������˼Ū��Ϳ�ˡ�
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
* Description:   ��485��Դ��ص���������
								 
* Input:  Nothing
* Output: Nothing
* Contributor: GuoWei Dong
* Date First Issued: 2010-01-26
******************************************************************************/
void Power485Pin_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);						//��ʱ��
	PWDC485DIS();																//����Ϊ�ر�		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =PWDC485_PIN;
	GPIO_Init(PWDC485_Port, &GPIO_InitStructure);
}

/******************************************************************************
* Function Name: SYS_UART_DeInit
* Description: ��λ�ⲿ���ڣ� ��չ����оƬӲ����λ
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
