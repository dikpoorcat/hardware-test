#ifndef __bsp_RTC_H
#define __bsp_RTC_H
#include "main.h"




/* Private define-----------------------------------------------------------------------------*/
// IIC引脚定义
#define	SIIC_GPIO				GPIOA
#define SIIC_GPIO_SCL			GPIO_Pin_1
#define SIIC_GPIO_SDA			GPIO_Pin_0
#define RCCRTCEN		        RCC_APB2Periph_GPIOA
#define	IIC_DELAY_TIME			1
#define	IIC_DELAY_TIME_LONG		2

// 设备读写地址
#define	RX8025_ADDR_READ		0x65
#define	RX8025_ADDR_WRITE		0x64

// 设备寄存器地址
#define	RX8025_ADDR_SECONDS		0x00
#define	RX8025_ADDR_WEEK		0x30
#define	RX8025_ADDR_DATES		0x40
#define	RX8025_ADDR_MONTH		0x50
#define	RX8025_ADDR_MINUTES		0x80
#define	RX8025_ADDR_CONTROL1	0xE0
#define	RX8025_ADDR_CONTROL2	0xF0

// 设备操作模式
#define	RX8025_WRITE_MODE		0xF0
#define	RX8025_READ_MODE		0xF0 
#define	RX8025_SIMP_READ_MODE	0x04 





/*全局变量声明*/
extern struct BSPRTC_TIME gRtcTime;												//数据为8421BCD码
extern struct BSPRTC_TIME gSetTime;												//数据为8421BCD码




/* Private function prototypes----------------------------------------------------------------*/
static void iic_delay(INT32U time);
static void iic_clk_high(void);
static void iic_clk_low(void);
static void iic_data_set_in(void);
static void iic_data_set_out(void);
static void iic_data_high(void);
static void iic_data_low(void);
static INT8U iic_data_read(void);


extern void iic_init(void);
extern void iic_start(void);
extern void iic_stop(void);
extern void iic_ack(void);
extern void iic_noack(void);
extern INT8U iic_send_byte(INT8U val);
extern INT8U iic_rec_byte(void);



static void RX8025Write(INT8U addr,INT8U *pData,INT8U len);
static void RX8025Read(INT8U addr,INT8U *pData,INT8U len);
void BSP_RX8025Write(INT8U *pData,INT8U len);
void BSP_RX8025Read(INT8U *pData,INT8U len);
void BSP_RX8025Init(void);
static void BSP_RTCWrite(const struct BSPRTC_TIME *pTime);
INT8U RtcSetChinaStdTimeStruct(const struct BSPRTC_TIME *pTime);
INT8U RtcSetTimeSecond(const INT32U time);
static void BSP_RTCRead(struct BSPRTC_TIME *pTime);
INT8U RtcGetChinaStdTimeStruct(struct BSPRTC_TIME *pTime);
INT32U RtcGetTimeSecond(void);
INT8U RtcCheckTimeStruct(const struct BSPRTC_TIME *pTime);
INT8U GetSysTime(INT8U *pOutBuff);
INT8U BcdToHex(INT8U InData);
INT8U HexToBCD(INT8U InData);
INT8U RTCTaskTest(void);

#endif




