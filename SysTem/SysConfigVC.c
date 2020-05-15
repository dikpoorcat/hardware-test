#include "SysConfigVC.h" 




/* --------------------Private functions------------------------------------------------------*/
#if 0
/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None		  
* Output         : None
* Contributor: 
* Date First Issued: 
*******************************************************************************/
void RCC_Configuration(void)
{   
		ErrorStatus       HSEStartUpStatus;
		INT16U i;
  /* RCC system reset(for debug purpose) */
		RCC_DeInit();
		RCC_HSICmd(RCC_HSE_ON);   
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	/*000b Zero wait state, if 0 < SYSCLK�� 24 MHz
      001b One wait state, if 24 MHz < SYSCLK �� 48 MHz
      010b Two wait states, if 48 MHz < SYSCLK �� 72 MHz*/
    /* Flash 1 wait state */
    FLASH_SetLatency(FLASH_Latency_1);
 	
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1); 
  
    /* PCLK2 = HCLK / 2 */
    RCC_PCLK2Config(RCC_HCLK_Div1); 

    /* PCLK1 = HCLK / 8 */
    RCC_PCLK1Config(RCC_HCLK_Div1);

	/* ADCCLK = PCLK2 / 4 */
		//RCC_ADCCLKConfig(RCC_HCLK_Div1);
 
    RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x00)//ֱ��ʹ��HSI 8M
    {
    }
 }
#else
/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None		  
* Output         : None
* Contributor: 
* Date First Issued: 
*******************************************************************************/
void RCC_Configuration(void)
{   
  ErrorStatus       HSEStartUpStatus;
  INT16U i;
  /* RCC system reset(for debug purpose) */
  RCC_DeInit();

  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);
  for(i=0;i<100;i++);
  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();
  if(HSEStartUpStatus==ERROR)
  {
 LSEON:
 	  HSEStartUpStatus=ERROR; 	  
	  RCC_HSEConfig(RCC_HSE_OFF);//�ر�
	  RCC_LSEConfig(RCC_LSE_ON);//�� �ڲ�RC 
	  for(i=0;i<100;i++);
  }
     /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

	/*000b Zero wait state, if 0 < SYSCLK�� 24 MHz
      001b One wait state, if 24 MHz < SYSCLK �� 48 MHz
      010b Two wait states, if 48 MHz < SYSCLK �� 72 MHz*/
    /* Flash 1 wait state */
    FLASH_SetLatency(FLASH_Latency_1);
 	
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1); 
  
    /* PCLK2 = HCLK / 2 */
    RCC_PCLK2Config(RCC_HCLK_Div1); 

    /* PCLK1 = HCLK / 8 */
    RCC_PCLK1Config(RCC_HCLK_Div2);

	/* ADCCLK = PCLK2 / 4 */
	RCC_ADCCLKConfig(RCC_PCLK2_Div4);

    /* HSE oscillator clock selected as PLL input clock */
    /* PLLCLK = 4MHz * 9 = 36 MHz */
	if(HSEStartUpStatus==ERROR)//ʹ���ڲ� 
    	RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_2);
	else
		  RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_2);//ʹ���ⲿ

    /* Enable PLL */ 
    RCC_PLLCmd(ENABLE);
	for(i=0;i<100;i++){;}
//	RCC_SYSCLKConfig(RCC_FLAG_PLLRDY);
    /* Wait till PLL is ready */
    i=2000;
	while(i--)
	{
		 if(RCC_GetFlagStatus(RCC_FLAG_PLLRDY)==SET)break;
	}
 	if(i==0xffff)goto LSEON;
    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
 }
#endif
 
 
 /***************************************************************************************
** ��������: Tmr_TickInit
** ��������: OS tick ��ʼ������
** ��    ��: None
** �� �� ֵ: None       
** ����  ��: �޻���
** ��  ����: 2007��11��28��
****************************************************************************************/
void Tmr_TickInit (unsigned int fhz)
{
    /* SysTick end of count event each 1s with input clock equal to 5MHz (HCLK/8, default) */
	  fhz = fhz /8;
  	SysTick_SetReload(fhz/OS_TICKS_PER_SEC);

  	/* Enable SysTick interrupt */
  	SysTick_ITConfig(ENABLE);

  	/* Enable the SysTick Counter */
  	SysTick_CounterCmd(SysTick_Counter_Enable);	
}


/*******************************************************************************
* Function Name  : void RCC_Configuration4M(void)
* Description    : ����ϵͳʱ��Ϊ4M
* Input          : None		  
* Output         : None
* Contributor: 
* Date First Issued: 
*******************************************************************************/
void RCC_Configuration4M(void)
{   
  ErrorStatus       HSEStartUpStatus;
  INT16U i;
  INT16U temp=0;
  /* RCC system reset(for debug purpose) */
  RCC_DeInit();

  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);
  for(i=0;i<100;i++);
  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();
  if(HSEStartUpStatus==ERROR)
  {
 
			HSEStartUpStatus=ERROR; 	  
			RCC_HSEConfig(RCC_HSE_OFF);//�ر�
//			RCC_LSEConfig(RCC_LSE_ON);//�� �ڲ�RC              //����������������LSE??	  
			RCC_HSICmd(ENABLE);									//�滻��HSI   	
			for(i=0;i<100;i++);
			RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);				//ʹ��HSI 8M
			while(RCC_GetSYSCLKSource() != 0x00)
			{
			}
  }
	else
	{
			RCC_SYSCLKConfig(RCC_SYSCLKSource_HSE);
			while(RCC_GetSYSCLKSource() != 0x04)
			{
			}
	}
     /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

    FLASH_SetLatency(FLASH_Latency_1);
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1); 
    /* PCLK2 = HCLK / 2 */
    RCC_PCLK2Config(RCC_HCLK_Div1); 
    /* PCLK1 = HCLK / 8 */
    RCC_PCLK1Config(RCC_HCLK_Div2);

	/* ADCCLK = PCLK2 / 4 */
	RCC_ADCCLKConfig(RCC_PCLK2_Div4);
	
	RCC_LSICmd(ENABLE);   //ʹ���ڲ�����ʱ��   ��Լ40KHz            //LSI���ã��ڶ������Ź����ڲ�RTCʱ����
	temp=0;
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET&&temp<1000)	//���ָ����RCC��־λ�������,�ȴ����پ������
		{
		temp++;			
		}
	
		
 }

 
/*******************************************************************************
* Function Name  : void RCC_Configuration32M(void)
* Description    : ����ϵͳʱ��Ϊ32M
* Input          : None		  
* Output         : None
* Contributor: 
* Date First Issued: 
*******************************************************************************/
void RCC_Configuration32M(void)
{   
  ErrorStatus       HSEStartUpStatus;
  INT16U i;
  /* RCC system reset(for debug purpose) */
  RCC_DeInit();

  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);
  for(i=0;i<100;i++){;}
  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();
  if(HSEStartUpStatus==ERROR)
  {
 LSEON:
 	  HSEStartUpStatus=ERROR; 	  
	  RCC_HSEConfig(RCC_HSE_OFF);//�ر�
	  RCC_LSEConfig(RCC_LSE_ON);//�� �ڲ�RC 
	  for(i=0;i<100;i++);
  }
     /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

	  /*000b Zero wait state, if 0 < SYSCLK�� 24 MHz
      001b One wait state, if 24 MHz < SYSCLK �� 48 MHz
      010b Two wait states, if 48 MHz < SYSCLK �� 72 MHz*/
    /* Flash 1 wait state */
    FLASH_SetLatency(FLASH_Latency_1);
 	
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1); 
  
    /* PCLK2 = HCLK / 2 */         // zzs note,����ע���Ǵ��
    RCC_PCLK2Config(RCC_HCLK_Div1);   // zzs note, ʵ����RCC_HCLK_Div1��˵����û�г�2������PCLK2 = HCLK

    /* PCLK1 = HCLK / 8 */        // zzs nott,����ע���Ǵ��
    RCC_PCLK1Config(RCC_HCLK_Div2);   // zzs note, ʵ����RCC_HCLK_Div2��˵�����ǳ�2������PCLK2 = HCLK/2

	/* ADCCLK = PCLK2 / 4 */
	RCC_ADCCLKConfig(RCC_PCLK2_Div4); 

    /* HSE oscillator clock selected as PLL input clock */
    /* PLLCLK = 4MHz * 8 = 32 MHz */
	if(HSEStartUpStatus==ERROR)//ʹ���ڲ� 
    	RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_8);
	else
		  RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_8);//zzs note ,ʹ���ⲿ��4M�ľ���Ȼ���������PLL��Ƶ������Ϊ8�����õ�HCLK=SYSCLK=4M*8=32M

    /* Enable PLL */ 
    RCC_PLLCmd(ENABLE);
	//for(i=0;i<100;i++);  
	for(i=0;i<100;i++){;}
  //RCC_SYSCLKConfig(RCC_FLAG_PLLRDY);
  /* Wait till PLL is ready */
  
	i=2000;
	while(i--)
	{
		 if(RCC_GetFlagStatus(RCC_FLAG_PLLRDY)==SET)break;
	}
 	if(i==0xffff)goto LSEON;
    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
 }
/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_DeInit();

#ifdef  VECT_TAB_RAM  
  	/* Set the Vector Table base location at 0x20000000 */ 
  	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  /* VECT_TAB_FLASH  */
 	/* Set the Vector Table base location at 0x08044800 */ 
	if(SYS==0) NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x06000);  				//SYS0:0x06000
	else if(SYS==1) NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x27000);  			//SYS1:0x27000
	else NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00000);						//BOOT:0x00000
#endif

	/* Configure three bit for preemption priority */
	/* one bit for sub priority                    */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);

	/* Enable the WWDG Interrupt��lowest priority */
//	NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQChannel;           // zzs open this IRQChannel
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);

	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQChannel;        
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the USART2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable the USART3 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	RTC_EXTI_INITIAL(ENABLE);										//RCT�ж����ã��������ڲ�	�¼�ͨ����ALARM�ж�	
	
	/* Enable the UART4 Interrupt */
//	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQChannel;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);

	/* Enable the UART5 Interrupt */
//	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQChannel;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;  /* 485�ӿ�����ʱ���������ȼ� */
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);

	/* Enable the TIM3 Interrupt */
//	// ��ʱ��3�ж�,���ڰ���ɨ��
//	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQChannel;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);

	/* Enable the EXTI0 Interrupt: BATT_GPIO_TEST_LINE (NPFO) */
//	 NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQChannel;
//	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	 NVIC_Init(&NVIC_InitStructure); 

// //  DMA1_Channel2_IRQChannel �Դ洢����ȡDMA���� 
//	  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQChannel;
//	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
//	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	  NVIC_Init(&NVIC_InitStructure); 

//	  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel3_IRQChannel;
//	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
//	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	  NVIC_Init(&NVIC_InitStructure);
		
//		NVIC_InitStructure.NVIC_IRQChannel= RTCAlarm_IRQChannel ;        // zzs commented this 2018.01.19  ,����Ŀû�õĆ��¶����������ó�Ϊ���ȼ���ߵ��жϡ�
//		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 0;    
//		NVIC_InitStructure.NVIC_IRQChannelSubPriority= 0;
//		NVIC_InitStructure.NVIC_IRQChannelCmd= ENABLE;
//		NVIC_Init(&NVIC_InitStructure); 
		
				
		//ʹ��RF�����жϣ�GDO0�½����ж�
//		NVIC_InitStructure.NVIC_IRQChannel= EXTI15_10_IRQChannel ;   // zzs??? ���������������ǲ����ظ��ˣ�����
//		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
//		NVIC_InitStructure.NVIC_IRQChannelSubPriority= 1;
//		NVIC_InitStructure.NVIC_IRQChannelCmd= ENABLE;
//		NVIC_Init(&NVIC_InitStructure); 
	
}
 void WWdg_Init(void)
{
		/* Enable WDG clocks */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);

		// /* PCLK1: 5MHZ */    // zzs???,û���κ�һ���ĵ�˵�����5MHZ��ô���ģ�����������������������������������������������������������������
	
	  // zzs note,��������� PLCK1=16M
	  // WWDG clock counter = (PCLK1/4096)/8 =  (~6.5 ms)  */   //zzs note, �����ע��
		/*��ˣ�zzs note, WWDG clock counter = (PCLK1/4096)/8 = ~488.28 Hz (~2.048 ms)  */
		WWDG_SetPrescaler(WWDG_Prescaler_8);

		/* Set Window value to 0x44 */
		WWDG_SetWindowValue(0x44);      
    //WWDG_SetWindowValue(0x4F);   // zzs note, ����ֵx��ȡֵ��Χ   0x40 <= x <= 0x7F  
	
		// /* Enable WWDG and set counter value to 0x7F, WWDG timeout = ~6.5 ms * (0x7F - 0x3F) = ~419 ms */   //zzs note, �����ע��
	  /*zzs note, Enable WWDG and set counter value to 0x7F, WWDG timeout = ~2.048 ms * (0x7F - 0x3F) = ~131.072 ms */
		WWDG_Enable(0x7F); // zzs note,�����WWDG�ļ�����ֵ���������ڵ�ʹ�þ��������0x7F������0x40��������䵽0x3F�����ܣ���Ҫ��������40��3F�����ĸ�λ�ź��ˡ�
	                       // ˵���ˣ���ֻ�ǰ�������ڿ��Ź�����һ����ͨ���Ź���ʹ����һ�¡�  

		/* Clear EWI flag */
		WWDG_ClearFlag();

		/* Enable EW interrupt */
		WWDG_EnableIT();
}

void McuSoftReset(void)
{
	INT8U  	Flag_temp = HOTRST;
	
	BSP_InitFm(WDT_Num);
	BSP_WriteDataToFm(Reset_Flag_Addr,&Flag_temp,1); 							//���Ϊ��������д������
	FM_LowPower(7);
	
    NVIC_SETFAULTMASK();														//��λǰֹͣ�ж���Ӧ
	NVIC_GenerateSystemReset();													//ϵͳ�����λ��������λ��
}

/*******************************************************************************
* Function Name  : void IWDG_Init(void)
* Description    : �ڲ����Ź�������Ҫ��LSI �ڲ�ʱ���Ѿ�����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void IWDG_Init(void)
{
	#ifndef	IWDGDIS
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);								//�����ڲ�����
	IWDG_SetPrescaler(IWDG_Prescaler_64);										//6.5S��λ
	IWDG_SetReload(0XFFF);
	IWDG_ReloadCounter();														//��λIWDG����FFF��ʼ����
	IWDG_Enable();																//��ʼ��ʱ
	#endif
}

/*******************************************************************************
* Function Name  : void IWDG_Reset(void)
* Description    : �ڲ����Ź���λ
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void IWDG_Reset(void)
{
		IWDG_ReloadCounter();
}
