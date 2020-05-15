/*****************************************Copyright(C)******************************************
 
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: bsp_WDG.c
**创    建    人: DGW
**创  建  日  期: 150518
**最  新  版  本: V0.1
**描          述: 看门狗驱动
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
 
 
#include	"bsp_WDG.h"
/* Private define-----------------------------------------------------------------------------*/
// 引脚定义
#if  BANPCB2 == 0x10
	#define	WDG_GPIO_CTLR			  GPIOB		// 控制脚
	#define WDG_GPIO_CTLR_WDI		GPIO_Pin_2 			// 喂狗
	#define WDG_GPIO_CK   			RCC_APB2Periph_GPIOC    // zzs note,没什么鸟用啊，定义在这里干嘛？？？
#endif

#if  BANPCB2 == 0x21
	#define	WDG_GPIO_CTLR			  GPIOB		// 控制脚
	#define WDG_GPIO_CTLR_WDI		GPIO_Pin_2 			// 喂狗
	#define WDG_GPIO_CK   			RCC_APB2Periph_GPIOB   // zzs note,没什么鸟用啊，定义在这里干嘛？？？
#endif

/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------- -----------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
//INT8U WDTSTATE;

/* --------------------Private functions------------------------------------------------------*/
/***********************************************************************************************
* Function		: BSP_WDGFeedDog
* Description	: 喂狗程序,这里对喂狗脚输出反向
* Input			: 
* Output		: 
* Note(s)		: 外部MAX706看门狗,1.6s喂狗时间,上升沿或者下降沿都可以.
* Contributor	: 090318	andydriver
***********************************************************************************************/
void BSP_WDGFeedDog(void)
{
	//BSP_WDGInit();
	//GPIO_WriteBit(WDG_GPIO_CTLR,WDG_GPIO_CTLR_WDI,Bit_SET);  
	//GPIO_WriteBit(WDG_GPIO_CTLR,WDG_GPIO_CTLR_WDI,Bit_RESET);
	if(GPIO_ReadOutputDataBit(WDG_GPIO_CTLR,WDG_GPIO_CTLR_WDI) == Bit_SET)
	{
		GPIO_WriteBit(WDG_GPIO_CTLR,WDG_GPIO_CTLR_WDI,Bit_RESET);
	}
	else
	{
		GPIO_WriteBit(WDG_GPIO_CTLR,WDG_GPIO_CTLR_WDI,Bit_SET);      //此编写会漏掉 1个 
	}
}
/***********************************************************************************************
* Function		: BSP_WDGInit
* Description	: 外部硬件看门狗初始化（其实是初始化喂狗引脚）
* Input			: 
* Output		: GPIO_WriteBit(GPIOB,GPIO_Pin_9,Bit_RESET);
* Note(s)		: 
* Contributor	: 090318	andydriver
***********************************************************************************************/
void BSP_WDGInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =WDG_GPIO_CTLR_WDI;
	GPIO_Init(WDG_GPIO_CTLR, &GPIO_InitStructure);
}
/************************(C)COPYRIGHT 2008 千能电力*****END OF FILE****************************/
