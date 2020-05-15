#ifndef __BSP_UART_H
#define __BSP_UART_H
#include "main.h"


/*�궨��*/
#define BSP_MSGID_RFDataIn	0x08    											// RF���ڽ��������ϢID 
#define BSP_MSGID_RS485DataIn 0x09    											// 485�����յ�������ϢID
#define BSP_MSGID_UART_RXOVER 0x0a    											// GPRS���ڽ��������ϢID
#define UART_DEFAULT_MODE	0x00     											// ��ͨUART��ʹ��    
#define UART_HALFDUP_MODE	0x01	   											// ��˫��ģʽ, (IRDA), ��չ������20MS��ת����ʱ 
#define UART_RS485_MODE		0x33	   											// ��˫��ģʽ, RS485, ��չ������20MS��ת����ʱ  

																				
//-----------------------------------485��Դ�������Ŷ���--------------------------------------
#define PWDC485_PIN      	GPIO_Pin_14
#define PWDC485_Port     	GPIOB		            //485��Դ�����Դ ���5V/200ma
#define PWDC485EN()      	GPIO_SetBits(PWDC485_Port,PWDC485_PIN)
#define PWDC485DIS()     	GPIO_ResetBits(PWDC485_Port,PWDC485_PIN)
#define PWDC485_Port_CK  	RCC_APB2Periph_GPIOB

//-----------------------------------485�������Ŷ���------------------------------------------
#define EN485_PIN			GPIO_Pin_15
#define EN485_Port			GPIOB		            //485��Դ�����Դ ���5V/200ma
#define EN485_EN()      	GPIO_SetBits(EN485_Port,EN485_PIN)
#define EN485_DIS()     	GPIO_ResetBits(EN485_Port,EN485_PIN)
#define EN485_Port_CK  		RCC_APB2Periph_GPIOB





/*�ṹ�嶨��*/
struct Str_Msg{
	unsigned char	DivNum;														// �豸��,  // zzs note,����ָ����Ϣ�Ǵ��������ġ�
	unsigned char	MsgID;														// ϵͳ��ID
	unsigned int	DataLen;													// ���ݳ���
	unsigned char	*pData;														// ���ݴ�ŵĻ�����ָ��
};

typedef struct UARTx_Setting_Str{
	INT32U BaudRate;															/* ������ 300 ~ 57600 */
	INT8U  DataBits;															/* ����1..5֧��7 - 8����λ������6,7֧��5 - 8����λ       */
	INT8U  Parity;	    														/* 0: No parity, 1: Odd parity, 2: Even parity           */
	INT8U  StopBits;															/* 1 - 2                                                 */
	INT8U  *RxBuf;																/* ���ջ�����                                            */
	INT16U RxBufLen;															/* ���ջ���������                                        */
	INT8U  *TxBuf;																/* ���ͻ�����                                            */
	INT16U TxBufLen; 															/* ���ͻ���������                                        */
	INT8U  Mode;																/* UART_DEFAULT_MODE, UART_HALFDUP_MODE, UART_RS485_MODE */
																				/* ����1..5��֧��7λ����У��ģʽ                         */
}UARTx_Setting_Struct;

// ���ڳ�ʼ������
// ��������,��Ҫ�޸����¶���
typedef enum
{
	BSPUART_PARITY_NO   = 0x00,													// ��У��(Ĭ��)
	BSPUART_PARITY_ODD  = 0x01,													// ��У��
	BSPUART_PARITY_EVEN = 0x02,													// żУ��
}_BSPUART_PARITY;

// ֹͣλ
typedef enum
{
    // BSPUART_STOPBITS_0_5=0x05,												// 0.5��ֹͣλ
	BSPUART_STOPBITS_1  =0x01,													// 1��ֹͣλ(Ĭ��)
    // BSPUART_STOPBITS_1_5=0x15,												// 1.5��ֹͣλ
	BSPUART_STOPBITS_2  =0x02,													// 2��ֹͣλ
}_BSPUART_STOPBITS;

//���ݳ���
typedef enum
{
	BSPUART_WORDLENGTH_5 = 0x05,												// 5λ����+У��λ
	BSPUART_WORDLENGTH_6 = 0x06,												// 6λ����
	BSPUART_WORDLENGTH_7 = 0x07,												// 7λ����+����λ?
	BSPUART_WORDLENGTH_8 = 0x08,												// 8λ����+У��λ(Ĭ��)
}_BSPUART_WORDLENGTH;


//*	   �����շ�������
typedef struct STRUCT_BSP_UARTX{
	volatile      INT16U     RxNum;
	
	volatile      INT16U     TxBusy;             								// ���ڷ���æµ���ͳ�������ڷ��������з������������� 0��
	volatile      INT16U     TxCompletedCnt;     								// ���ڷ�������ֽ���������
		
	volatile      INT8U      *pSendBuf;          								// ָ���ͻ���
	volatile      INT8U      *pRecvBuf;          								// ָ����ջ��� 

	INT16U        MaxTxBufLen;    
	INT16U        MaxRxBufLen;
		
	INT16U        RxOffset;         											// ��һ֡��ʼ��ַ,�ƶ��ٶ������ͺ���RxPointer,
	INT16U        RxPointer;        											// �����������ǰ�ƽ��ġ�

	INT16U        FrameRxIntv;
	INT16U        RxFrameIntvSet;   											// ԭʼ������FrameIntv // zzs modified it to ��RxFrameIntvSet�� ������ָ��Rx��֡��������ֵ�п�������ϵͳTickΪ��λ��ʱ�����������ֽ���Ϊ��λ���ֽ���Ŀ
	                                											// �ɱ�������Bit15������0����ʱ�������ü��  1�����ֽ��������ü�� ��
	                                											// Bit14,��ʱ�������ã� ��Bit15ѡ���˼�����÷�ʽ�󣬼��ֵ�Ƕ��٣�����Bit13~Bit0ָ����
	INT16U        TxIntv;
	INT16U        TxIntvSet;        											// ԭʼ������TISet   // zzs modified it to ��TxIntvSet�� ��  
		
	UARTx_Setting_Struct   Setting; 											// ���ڵ�����
	OS_EVENT      *MailOrQueue;     											// �䱸������
	struct Str_Msg		  BspMsg[2];											// ѭ������ Z.E.ע
}UARTx_Ctrl_Struct;                 											// ���ڲ������ƽṹ��



/*ȫ�ֱ���*/





/* --------------------Private functions prototypes------------------------------------------------*/
void B485_init(unsigned int rate);
void B485_LowPower(void);
void UART1_PIN_CFG(void);
void UART1_PIN_CLOSE(void);
void UART2_PIN_CFG(void);
void UART2_PIN_CLOSE(void);
void UART3_PIN_CFG(void);
void UART3_PIN_CLOSE(void);
void UART4_PIN_CFG(void);
void UART5_PIN_CFG(void);
USART_TypeDef *GetCommHandle(INT8U ComPort);
__inline void  MODE485_RxEnable(INT8U port);
__inline void  MODE485_TxEnable(INT8U port);
__inline void  UART_RxEnable(INT8U Com,INT8U Enable);
void SendReceiveMsg(INT8U ComPort,UARTx_Ctrl_Struct *pCtrl_UARTx);
INT8U BSP_UART_Init(INT8U ComPort, UARTx_Setting_Struct *pUartSet, OS_EVENT *Mail_Queue);
void SYS_UART_ISR(INT8U ComPort);
void SYS_UART_FSR(INT8U ComPort);
INT8U BspUartWrite(INT8U ComPort, INT8U *pFrameBuf, INT16U FrameLen);
void  BSP_UART_RxClear(INT8U ComPort);
INT8U BSP_UART_TxState(INT8U ComPort);
INT8U BSP_UART_Close(INT8U ComPort);
INT8U * BSP_UART_GetRecvLen(INT8U ComPort,INT16U *RLEN);
void Power485Pin_Init(void);

#endif //__BSP_UART_H_
