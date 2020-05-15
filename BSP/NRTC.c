#include "NRTC.h"



/*配置内部RTC中断*/
void RTC_EXTI_INITIAL(FunctionalState interrupt_en_or_dis)
{
	NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure; 

//------------EXTI17 配置 -------------------   
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;          		//内部事件通道，中断连接到RTC闹钟事件
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			//若ALRF=1，如果在EXTI控制器中设置了EXTI线 17的中断模式，则允许产生RTC闹钟中断；如果在EXTI控制器中设置了EXTI线 17的事件模式，则这条线上会产生一个脉冲(不会产生RTC闹钟中断)。
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = interrupt_en_or_dis;
    EXTI_Init(&EXTI_InitStructure); 
//------------设置 中断------------------- 
    NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQChannel;    //设置闹钟中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = interrupt_en_or_dis;
    NVIC_Init(&NVIC_InitStructure);     
//------------------------------------------- 
//	//------------设置 中断------------------- 
//    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQChannel;    //设置闹钟中断
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = interrupt_en_or_dis;
//    NVIC_Init(&NVIC_InitStructure);     
//------------------------------------------- 
}


/*内部RTC时钟初始化*/
INT8U RTC_FisrtSet=0;
INT8U RTC_Init(INT16U Fre)
{
	INT16U Setcount=0;	
	Setcount=(40000/Fre)-1;     //LSI频率约40K   例：要设置50ms为一个计数点，则频率为20Hz，需要设置的计数初值就是40K/20-1=1999.
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟   	   				RTC寄存器位于后备区域							
	PWR_BackupAccessCmd(ENABLE);									//使能后备寄存器访问  
//	BKP_DeInit();													//复位备份区域 	不可开启，会清备份寄存器！
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);							//设置RTC时钟(RTCCLK),选择LSI作为RTC时钟    
	RCC_RTCCLKCmd(ENABLE);											//使能RTC时钟  
	RTC_WaitForLastTask();											//等待最近一次对RTC寄存器的写操作完成
	RTC_WaitForSynchro();											//等待RTC寄存器同步  
	RTC_EnterConfigMode();      									// 允许配置	
	RTC_SetPrescaler(Setcount);    								    //设置RTC预分频的值  
	RTC_WaitForLastTask();  										//等待最近一次对RTC寄存器的写操作完成
	RTC_SetCounter(0);												//随意初始化一个时间，从此时开始计数
	RTC_ExitConfigMode();											//退出配置模式  
		
//	RTC_EXTI_INITIAL(ENABLE);										//RCT中断配置，配置了内部	事件通道，ALARM中断	(放在总的中断分配中)
	RTC_WaitForLastTask();		
	RTC_ITConfig(RTC_IT_SEC|RTC_IT_OW,DISABLE);   		            //初始化时，秒中断溢出中断关闭
	RTC_WaitForLastTask();											//等待最近一次对RTC寄存器的写操作完成
	RTC_ITConfig(RTC_IT_ALR,ENABLE);							    //闹钟中断打开
	RTC_WaitForLastTask();											//等待最近一次对RTC寄存器的写操作完成
	return 1; //ok
}

/*设置闹钟*/
void RTC_SET_ALARM(u32 sec)
{
  //DEBUG_COM_STREAM("-闹钟-",NULL);
  RTC_SetAlarm(RTC_GetCounter()+sec);
  //DEBUG_COM_STREAM("-闹钟1-",NULL);
  RTC_WaitForLastTask();
  //DEBUG_COM_STREAM("-闹钟2-",NULL);
  RTC_ITConfig(RTC_FLAG_ALR,ENABLE);
}


/*最终的闹钟唤醒设置  5S*/
void RTC_AWU_SET(void)
{
  //启用PWR和BKP的时钟（from APB1）
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  //后备域解锁
  PWR_BackupAccessCmd(ENABLE);
  RTC_ITConfig(RTC_IT_SEC, DISABLE);   //关掉秒中断
  RTC_SET_ALARM(5);
  //PWR_BackupAccessCmd(DISABLE);
  RTC_EXTI_INITIAL(ENABLE);
}



