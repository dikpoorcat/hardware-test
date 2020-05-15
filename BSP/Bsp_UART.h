#ifndef __BSP_UART_H
#define __BSP_UART_H
#include "main.h"


/*宏定义*/
#define BSP_MSGID_RFDataIn	0x08    											// RF串口接收完成消息ID 
#define BSP_MSGID_RS485DataIn 0x09    											// 485串口收到数据消息ID
#define BSP_MSGID_UART_RXOVER 0x0a    											// GPRS串口接收完成消息ID
#define UART_DEFAULT_MODE	0x00     											// 普通UART口使用    
#define UART_HALFDUP_MODE	0x01	   											// 半双工模式, (IRDA), 扩展串口有20MS的转换延时 
#define UART_RS485_MODE		0x33	   											// 半双工模式, RS485, 扩展串口有20MS的转换延时  

																				
//-----------------------------------485电源控制引脚定义--------------------------------------
#define PWDC485_PIN      	GPIO_Pin_14
#define PWDC485_Port     	GPIOB		            //485电源隔离电源 输出5V/200ma
#define PWDC485EN()      	GPIO_SetBits(PWDC485_Port,PWDC485_PIN)
#define PWDC485DIS()     	GPIO_ResetBits(PWDC485_Port,PWDC485_PIN)
#define PWDC485_Port_CK  	RCC_APB2Periph_GPIOB

//-----------------------------------485控制引脚定义------------------------------------------
#define EN485_PIN			GPIO_Pin_15
#define EN485_Port			GPIOB		            //485电源隔离电源 输出5V/200ma
#define EN485_EN()      	GPIO_SetBits(EN485_Port,EN485_PIN)
#define EN485_DIS()     	GPIO_ResetBits(EN485_Port,EN485_PIN)
#define EN485_Port_CK  		RCC_APB2Periph_GPIOB





/*结构体定义*/
struct Str_Msg{
	unsigned char	DivNum;														// 设备号,  // zzs note,用于指明消息是从哪里来的。
	unsigned char	MsgID;														// 系统的ID
	unsigned int	DataLen;													// 数据长度
	unsigned char	*pData;														// 数据存放的缓冲区指针
};

typedef struct UARTx_Setting_Str{
	INT32U BaudRate;															/* 波特率 300 ~ 57600 */
	INT8U  DataBits;															/* 串口1..5支持7 - 8数据位，串口6,7支持5 - 8数据位       */
	INT8U  Parity;	    														/* 0: No parity, 1: Odd parity, 2: Even parity           */
	INT8U  StopBits;															/* 1 - 2                                                 */
	INT8U  *RxBuf;																/* 接收缓冲区                                            */
	INT16U RxBufLen;															/* 接收缓冲区长度                                        */
	INT8U  *TxBuf;																/* 发送缓冲区                                            */
	INT16U TxBufLen; 															/* 发送缓冲区长度                                        */
	INT8U  Mode;																/* UART_DEFAULT_MODE, UART_HALFDUP_MODE, UART_RS485_MODE */
																				/* 串口1..5不支持7位、无校验模式                         */
}UARTx_Setting_Struct;

// 串口初始化参数
// 串口设置,不要修改以下定义
typedef enum
{
	BSPUART_PARITY_NO   = 0x00,													// 无校验(默认)
	BSPUART_PARITY_ODD  = 0x01,													// 奇校验
	BSPUART_PARITY_EVEN = 0x02,													// 偶校验
}_BSPUART_PARITY;

// 停止位
typedef enum
{
    // BSPUART_STOPBITS_0_5=0x05,												// 0.5个停止位
	BSPUART_STOPBITS_1  =0x01,													// 1个停止位(默认)
    // BSPUART_STOPBITS_1_5=0x15,												// 1.5个停止位
	BSPUART_STOPBITS_2  =0x02,													// 2个停止位
}_BSPUART_STOPBITS;

//数据长度
typedef enum
{
	BSPUART_WORDLENGTH_5 = 0x05,												// 5位数据+校验位
	BSPUART_WORDLENGTH_6 = 0x06,												// 6位数据
	BSPUART_WORDLENGTH_7 = 0x07,												// 7位数据+唤醒位?
	BSPUART_WORDLENGTH_8 = 0x08,												// 8位数据+校验位(默认)
}_BSPUART_WORDLENGTH;


//*	   串口收发管理器
typedef struct STRUCT_BSP_UARTX{
	volatile      INT16U     RxNum;
	
	volatile      INT16U     TxBusy;             								// 串口发送忙碌情况统计器，在发送完所有发送请求后，清除归 0。
	volatile      INT16U     TxCompletedCnt;     								// 串口发送完成字节数计数器
		
	volatile      INT8U      *pSendBuf;          								// 指向发送缓冲
	volatile      INT8U      *pRecvBuf;          								// 指向接收缓冲 

	INT16U        MaxTxBufLen;    
	INT16U        MaxRxBufLen;
		
	INT16U        RxOffset;         											// 下一帧起始地址,移动速度总是滞后于RxPointer,
	INT16U        RxPointer;        											// 总是这个先向前推进的。

	INT16U        FrameRxIntv;
	INT16U        RxFrameIntvSet;   											// 原始命名：FrameIntv // zzs modified it to “RxFrameIntvSet” ，设置指定Rx的帧间隔，这个值有可能是以系统Tick为单位的时间间隔，和以字节数为单位的字节数目
	                                											// 由本变量的Bit15决定，0：用时间来设置间隔  1：用字节数来设置间隔 。
	                                											// Bit14,暂时闲置无用， 用Bit15选定了间隔设置方式后，间隔值是多少，则由Bit13~Bit0指定。
	INT16U        TxIntv;
	INT16U        TxIntvSet;        											// 原始命名：TISet   // zzs modified it to “TxIntvSet” ，  
		
	UARTx_Setting_Struct   Setting; 											// 串口的配置
	OS_EVENT      *MailOrQueue;     											// 配备个邮箱
	struct Str_Msg		  BspMsg[2];											// 循环缓存 Z.E.注
}UARTx_Ctrl_Struct;                 											// 串口操作控制结构体



/*全局变量*/





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
