/*****************************************Copyright(C)******************************************
 
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: bsp_WDG.c
**��    ��    ��: DGW
**��  ��  ��  ��: 150518
**��  ��  ��  ��: V0.1
**��          ��: ���Ź�����
**---------------------------------------��ʷ�汾��Ϣ-------------------------------------------
**��    ��    ��: 
**��          ��: 
**��          ��: 
**��          ��:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
 
 
#include	"bsp_WDG.h"
/* Private define-----------------------------------------------------------------------------*/
// ���Ŷ���
#if  BANPCB2 == 0x10
	#define	WDG_GPIO_CTLR			  GPIOB		// ���ƽ�
	#define WDG_GPIO_CTLR_WDI		GPIO_Pin_2 			// ι��
	#define WDG_GPIO_CK   			RCC_APB2Periph_GPIOC    // zzs note,ûʲô���ð�������������������
#endif

#if  BANPCB2 == 0x21
	#define	WDG_GPIO_CTLR			  GPIOB		// ���ƽ�
	#define WDG_GPIO_CTLR_WDI		GPIO_Pin_2 			// ι��
	#define WDG_GPIO_CK   			RCC_APB2Periph_GPIOB   // zzs note,ûʲô���ð�������������������
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
* Description	: ι������,�����ι�����������
* Input			: 
* Output		: 
* Note(s)		: �ⲿMAX706���Ź�,1.6sι��ʱ��,�����ػ����½��ض�����.
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
		GPIO_WriteBit(WDG_GPIO_CTLR,WDG_GPIO_CTLR_WDI,Bit_SET);      //�˱�д��©�� 1�� 
	}
}
/***********************************************************************************************
* Function		: BSP_WDGInit
* Description	: �ⲿӲ�����Ź���ʼ������ʵ�ǳ�ʼ��ι�����ţ�
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
/************************(C)COPYRIGHT 2008 ǧ�ܵ���*****END OF FILE****************************/
