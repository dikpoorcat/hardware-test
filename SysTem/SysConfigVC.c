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
	/*000b Zero wait state, if 0 < SYSCLK≤ 24 MHz
      001b One wait state, if 24 MHz < SYSCLK ≤ 48 MHz
      010b Two wait states, if 48 MHz < SYSCLK ≤ 72 MHz*/
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
    while(RCC_GetSYSCLKSource() != 0x00)//直接使用HSI 8M
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
	  RCC_HSEConfig(RCC_HSE_OFF);//关闭
	  RCC_LSEConfig(RCC_LSE_ON);//打开 内部RC 
	  for(i=0;i<100;i++);
  }
     /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

	/*000b Zero wait state, if 0 < SYSCLK≤ 24 MHz
      001b One wait state, if 24 MHz < SYSCLK ≤ 48 MHz
      010b Two wait states, if 48 MHz < SYSCLK ≤ 72 MHz*/
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
	if(HSEStartUpStatus==ERROR)//使用内部 
    	RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_2);
	else
		  RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_2);//使用外部

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
** 函数名称: Tmr_TickInit
** 功能描述: OS tick 初始化函数
** 参    数: None
** 返 回 值: None       
** 作　  者: 罗辉联
** 日  　期: 2007年11月28日
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
* Description    : 配置系统时钟为4M
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
			RCC_HSEConfig(RCC_HSE_OFF);//关闭
//			RCC_LSEConfig(RCC_LSE_ON);//打开 内部RC              //？？？？？？？？LSE??	  
			RCC_HSICmd(ENABLE);									//替换成HSI   	
			for(i=0;i<100;i++);
			RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);				//使用HSI 8M
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
	
	RCC_LSICmd(ENABLE);   //使能内部低速时钟   大约40KHz            //LSI启用，在独立看门狗和内部RTC时可用
	temp=0;
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET&&temp<1000)	//检查指定的RCC标志位设置与否,等待低速晶振就绪
		{
		temp++;			
		}
	
		
 }

 
/*******************************************************************************
* Function Name  : void RCC_Configuration32M(void)
* Description    : 配置系统时钟为32M
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
	  RCC_HSEConfig(RCC_HSE_OFF);//关闭
	  RCC_LSEConfig(RCC_LSE_ON);//打开 内部RC 
	  for(i=0;i<100;i++);
  }
     /* Enable Prefetch Buffer */
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

	  /*000b Zero wait state, if 0 < SYSCLK≤ 24 MHz
      001b One wait state, if 24 MHz < SYSCLK ≤ 48 MHz
      010b Two wait states, if 48 MHz < SYSCLK ≤ 72 MHz*/
    /* Flash 1 wait state */
    FLASH_SetLatency(FLASH_Latency_1);
 	
    /* HCLK = SYSCLK */
    RCC_HCLKConfig(RCC_SYSCLK_Div1); 
  
    /* PCLK2 = HCLK / 2 */         // zzs note,本行注释是错的
    RCC_PCLK2Config(RCC_HCLK_Div1);   // zzs note, 实际用RCC_HCLK_Div1，说明就没有除2，而是PCLK2 = HCLK

    /* PCLK1 = HCLK / 8 */        // zzs nott,本行注释是错的
    RCC_PCLK1Config(RCC_HCLK_Div2);   // zzs note, 实际用RCC_HCLK_Div2，说明不是除2，而是PCLK2 = HCLK/2

	/* ADCCLK = PCLK2 / 4 */
	RCC_ADCCLKConfig(RCC_PCLK2_Div4); 

    /* HSE oscillator clock selected as PLL input clock */
    /* PLLCLK = 4MHz * 8 = 32 MHz */
	if(HSEStartUpStatus==ERROR)//使用内部 
    	RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_8);
	else
		  RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_8);//zzs note ,使用外部的4M的晶振，然后在这儿将PLL倍频因子设为8，最后得到HCLK=SYSCLK=4M*8=32M

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

	/* Enable the WWDG Interrupt，lowest priority */
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
	
	RTC_EXTI_INITIAL(ENABLE);										//RCT中断配置，配置了内部	事件通道，ALARM中断	
	
	/* Enable the UART4 Interrupt */
//	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQChannel;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);

	/* Enable the UART5 Interrupt */
//	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQChannel;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;  /* 485接口有延时，降低优先级 */
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);

	/* Enable the TIM3 Interrupt */
//	// 定时器3中断,用于按键扫描
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

// //  DMA1_Channel2_IRQChannel 对存储器读取DMA操作 
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
		
//		NVIC_InitStructure.NVIC_IRQChannel= RTCAlarm_IRQChannel ;        // zzs commented this 2018.01.19  ,对项目没用的啰嗦东西，还配置成为优先级最高的中断。
//		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 0;    
//		NVIC_InitStructure.NVIC_IRQChannelSubPriority= 0;
//		NVIC_InitStructure.NVIC_IRQChannelCmd= ENABLE;
//		NVIC_Init(&NVIC_InitStructure); 
		
				
		//使能RF接收中断，GDO0下降沿中断
//		NVIC_InitStructure.NVIC_IRQChannel= EXTI15_10_IRQChannel ;   // zzs??? 这个和上面的配置是不是重复了？？？
//		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
//		NVIC_InitStructure.NVIC_IRQChannelSubPriority= 1;
//		NVIC_InitStructure.NVIC_IRQChannelCmd= ENABLE;
//		NVIC_Init(&NVIC_InitStructure); 
	
}
 void WWdg_Init(void)
{
		/* Enable WDG clocks */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);

		// /* PCLK1: 5MHZ */    // zzs???,没有任何一份文档说明这个5MHZ怎么来的？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
	
	  // zzs note,真相查明： PLCK1=16M
	  // WWDG clock counter = (PCLK1/4096)/8 =  (~6.5 ms)  */   //zzs note, 错误的注释
		/*因此：zzs note, WWDG clock counter = (PCLK1/4096)/8 = ~488.28 Hz (~2.048 ms)  */
		WWDG_SetPrescaler(WWDG_Prescaler_8);

		/* Set Window value to 0x44 */
		WWDG_SetWindowValue(0x44);      
    //WWDG_SetWindowValue(0x4F);   // zzs note, 窗口值x的取值范围   0x40 <= x <= 0x7F  
	
		// /* Enable WWDG and set counter value to 0x7F, WWDG timeout = ~6.5 ms * (0x7F - 0x3F) = ~419 ms */   //zzs note, 错误的注释
	  /*zzs note, Enable WWDG and set counter value to 0x7F, WWDG timeout = ~2.048 ms * (0x7F - 0x3F) = ~131.072 ms */
		WWDG_Enable(0x7F); // zzs note,这个是WWDG的计数初值，我们现在的使用就是让其从0x7F计数到0x40后，任由其变到0x3F而不管，主要就用它从40变3F产生的复位信号了。
	                       // 说白了，还只是把这个窗口看门狗当做一个普通看门狗来使用了一下。  

		/* Clear EWI flag */
		WWDG_ClearFlag();

		/* Enable EW interrupt */
		WWDG_EnableIT();
}

void McuSoftReset(void)
{
	INT8U  	Flag_temp = HOTRST;
	
	BSP_InitFm(WDT_Num);
	BSP_WriteDataToFm(Reset_Flag_Addr,&Flag_temp,1); 							//标记为热启动，写入铁电
	FM_LowPower(7);
	
    NVIC_SETFAULTMASK();														//复位前停止中断响应
	NVIC_GenerateSystemReset();													//系统软件复位（立即复位）
}

/*******************************************************************************
* Function Name  : void IWDG_Init(void)
* Description    : 内部看门狗启动，要求LSI 内部时钟已经启动
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void IWDG_Init(void)
{
	#ifndef	IWDGDIS
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);								//访问内部参数
	IWDG_SetPrescaler(IWDG_Prescaler_64);										//6.5S复位
	IWDG_SetReload(0XFFF);
	IWDG_ReloadCounter();														//复位IWDG，从FFF开始重新
	IWDG_Enable();																//开始计时
	#endif
}

/*******************************************************************************
* Function Name  : void IWDG_Reset(void)
* Description    : 内部看门狗复位
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void IWDG_Reset(void)
{
		IWDG_ReloadCounter();
}
