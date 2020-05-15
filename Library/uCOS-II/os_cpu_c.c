/****************************Copyright (c)*********************************************                                     
*
*               (c) Copyright 2006-2008, hui lian. luo, china, zj. hz 
*                            All Rights Reserved
*
*							 深圳市英蓓特信息技术有限公司
*                            http://www.embedinfo.com
*                            博格达科技有限公司
*                            http://www.bogodtech.com                         
*
*---------- File Info ---------------------------------------------------------------
* File name: 		os_cpu_c.c
* Created By: 		罗辉联(armgcc@foxmail.com, wyuyun@hotmail.com, lhlzjut@hotmail.com) 	
* Created date: 	2007.11.28
* Descriptions: 	The C functions that a uC/OS-II port requires
* Adviser: 			楼东武(professor)  浙江大学信电系
*
*---------- History Info -------------------------------------------------------------
* Version: 			Ver1.0
* Descriptions: 	Create Initilze file
*
*-------------------------------------------------------------------------------------
**************************************************************************************/

#define  OS_CPU_GLOBALS

#include "ucos_ii.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_systick.h"
#include "SysConfigVC.h" 
#include "stm32f10x_rtc.h"

#define  OS_VIEW_MODULE   0


#if OS_TMR_EN > 0
static  INT16U  OSTmrCtr;
#endif

/***************************************************************************************
* 函数名称: OSInitHookBegin 
*
* 功能描述: This function is called by OSInit() at the beginning of OSInit(). 
*            
* 参    数: None
*
* 返 回 值: None
*         
* 作　  者: 罗辉联
* 日  　期: 2007年11月10日
*----------------------------------------------------------------------------------------
*****************************************************************************************/

#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSInitHookBegin (void)
{
	#if OS_TMR_EN > 0 
		OSTmrCtr = 0; 
	#endif
}
#endif

/***************************************************************************************
* 函数名称: OSInitHookEnd, 
*
* 功能描述: This function is called by OSInit() at the end of OSInit().
*            
* 参    数: None
*
* 返 回 值: None
*         
* 作　  者: 罗辉联
* 日  　期: 2007年11月10日
*----------------------------------------------------------------------------------------
*****************************************************************************************/

#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSInitHookEnd (void)
{

}
#endif

/***************************************************************************************
* 函数名称: OSTaskCreateHook 
*
* 功能描述: This function is called when a task is created. 
*            
* 参    数: ptcb   is a pointer to the task control block of the task being created.
*
* 返 回 值: None
*         
* 作　  者: 罗辉联
* 日  　期: 2007年11月10日
*----------------------------------------------------------------------------------------
*****************************************************************************************/

#if OS_CPU_HOOKS_EN > 0
void  OSTaskCreateHook (OS_TCB *ptcb)
{
#if OS_VIEW_MODULE > 0
    OSView_TaskCreateHook(ptcb);
#else
    ptcb = ptcb;                       /* Prevent compiler warning  */
#endif
}
#endif

/***************************************************************************************
* 函数名称: OSTaskDelHook 
*
* 功能描述: This function is called when a task is deleted.
*            
* 参    数: ptcb   is a pointer to the task control block of the task being deleted.
*
* 返 回 值: None
*         
* 作　  者: 罗辉联
* 日  　期: 2007年11月10日
*----------------------------------------------------------------------------------------
*****************************************************************************************/

#if OS_CPU_HOOKS_EN > 0
void  OSTaskDelHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                       /* Prevent compiler warning  */
}
#endif

/***************************************************************************************
* 函数名称: OSTaskIdleHook 
*
* 功能描述: This function is called by the idle task.  This hook has been added to allow 
*           you to do such things as STOP the CPU to conserve power.
*            
* 参    数: None
*
* 返 回 值: None
*         
* 作　  者: 罗辉联
* 日  　期: 2007年11月10日
*----------------------------------------------------------------------------------------
*****************************************************************************************/

#if OS_CPU_HOOKS_EN > 0 && OS_VERSION >= 251
//extern volatile INT32U	WDG_clr_flag;
//extern void DelayUS(volatile INT32U nCount);
void  OSTaskIdleHook (void)
{
    /* Nothing to do, But MUST keep this code to improve INT latency */
	/*在这里进行进入低功耗以及唤醒之后的操作*/
	OS_TCB    *ptcb;
	INT16U 	  DLYmin=0;
	INT16U    alarmcount=0;
	INT16U    temp=0;	
	
	OS_ENTER_CRITICAL();
	ptcb = OSTCBList; 	
	DLYmin=ptcb->OSTCBDly;														//读取任务链表第一个delay time					    
	while (ptcb->OSTCBPrio != OS_TASK_IDLE_PRIO)
	{ 	
		if((ptcb->OSTCBStat & OS_STAT_SUSPEND) == OS_STAT_RDY)	 				//被挂起的任务，delaytime都会被置为0
		{temp=ptcb->OSTCBDly;}
		if(temp<DLYmin)
		{DLYmin=temp;}										   					//查找最小的任务delay time	
		ptcb = ptcb->OSTCBNext;  		 
	 }
	OS_EXIT_CRITICAL();	
	if(DLYmin==0) return;														//有效性检查
	  		        	
//	Led_OFF();	
	 
	SysTick_ITConfig(DISABLE);
    SysTick_CounterCmd(SysTick_Counter_Disable);								//关闭systick,不可提早关闭	 	
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟   	   				RTC寄存器位于后备区域							
	PWR_BackupAccessCmd(ENABLE);												//使能后备寄存器访问  
	RTC_WaitForLastTask();														//等待最近一次对RTC寄存器的写操作完成
	RTC_EnterConfigMode();      												//允许配置	
	RTC_SetCounter(0);															//随意初始化一个时间，从此时开始计数
	RTC_ExitConfigMode();														//退出配置模式  
	RTC_WaitForLastTask();  													//等待最近一次对RTC寄存器的写操作完成
	 
	alarmcount=DLYmin-1; 														//-1,RTC闹钟本身自带+1
	RTC_SetAlarm(RTC_GetCounter()+alarmcount);			        				//进行RTC闹钟设置  50ms基数	
	RTC_WaitForLastTask();  													//等待最近一次对RTC寄存器的写操作完成
	 
	PWR_EnterSTOPMode(PWR_Regulator_LowPower,PWR_STOPEntry_WFI);   				//进入停止模式（PWR_Regulator_LowPower，WFI）
	
	 /*唤醒后从闹钟中断里退出后由此开始继续执行*/
	RCC_Configuration4M();                                     					//主频配置与系统时钟重新设定，这部分不适合放在闹钟中断服务函数中（可能存在其他非闹钟唤醒情况）	
	
	// Led_On();	 
    // OSTime+=DLYmin;    											 			//这个暂时看来是无用的，并没有用这个来计时
    if (OSRunning == OS_TRUE) 
	{
		ptcb = OSTCBList;                                  						/* Point at first TCB in TCB list                 */
        while (ptcb->OSTCBPrio != OS_TASK_IDLE_PRIO) {    						/* Go through all TCBs in TCB list                */      
            OS_ENTER_CRITICAL();
            if (ptcb->OSTCBDly != 0) {                     						/* No, Delayed or waiting for event with TO       */
				if(ptcb->OSTCBDly<=DLYmin)
				{ptcb->OSTCBDly=0;}
				else
				{ptcb->OSTCBDly-=DLYmin;}                  
                if (ptcb->OSTCBDly== 0) 
					{               					   						/* Decrement nbr of ticks to end of delay          */		//减少delay的ticker计数 ，若为0则延时到点了
																				/* Check for timeout                               */
						if ((ptcb->OSTCBStat & OS_STAT_PEND_ANY) != OS_STAT_RDY) 
						{																						     	//若该任务在pend状态
							ptcb->OSTCBStat   &= ~OS_STAT_PEND_ANY;             /* Yes, Clear status flag   */		//清除pend标志
							ptcb->OSTCBPendTO  = OS_TRUE;                       /* Indicate PEND timeout    */	    //pend结束		pend超时
						} 
						else 
						{
							ptcb->OSTCBPendTO  = OS_FALSE;																//
						}

						if ((ptcb->OSTCBStat & OS_STAT_SUSPEND) == OS_STAT_RDY) 
						{  														/* Is task suspended?       */		//该任务在suspend状态
							OSRdyGrp               |= ptcb->OSTCBBitY;          /* No,  Make ready          */		//若没有，置为ready
							OSRdyTbl[ptcb->OSTCBY] |= ptcb->OSTCBBitX;
						}
					}
            }
            ptcb = ptcb->OSTCBNext;                       						/* Point at next TCB in TCB list                */
            OS_EXIT_CRITICAL();
        }
	}	
	Tmr_TickInit(4000000);   													//将systick中断开启放在最后，避免影响前面这些
}
#endif

/***************************************************************************************
* 函数名称: OSTaskStatHook 
*
* 功能描述: This function is called every second by uC/OS-II's statistics task.This allows 
*           your application to add functionality to the statistics task.
*            
* 参    数: none
*
* 返 回 值: None
*         
* 作　  者: 罗辉联
* 日  　期: 2007年11月10日
*----------------------------------------------------------------------------------------
*****************************************************************************************/

#if OS_CPU_HOOKS_EN > 0
void  OSTaskStatHook (void)
{
}
#endif

/****************************************************************************************
* 函数名称: OSTaskStkInit
* 功能描述: OSTaskCreate() 和 OSTaskCreateExt()调用本函数初始化正被创建的任务的堆栈
*            本函数和具体的处理器有关 
* 
* 参    数: task  : 任务开始执行的地址
*           pdata ：传递给任务的参数
*           ptos  ：任务的堆栈开始位置
*           opt   ：附加参数，具体意义参见OSTaskCreateExt()的opt参数
*
* 返 回 值: 栈顶指针位置
*         
* 作　  者: 罗辉联
* 日  　期: 2006年6月10日
*----------------------------------------------------------------------------------------
*****************************************************************************************/ 

OS_STK *OSTaskStkInit (void  (*task)(void  *parg), void  *parg, OS_STK  *ptos, INT16U  opt)
{
    OS_STK *stk;


    (void)opt;                                        /*  'opt' is not used, prevent  */
                                                      /*  warning 没有用'opt'，       */
                                                      /*  防止编译警告                */
                                                                            
    stk = ptos;                                       /*  Load stack pointer          */
                                                      /*  装载堆栈指针                */                                    

                                                      /*  Registers stacked as if     */
                                                      /*  auto-saved on exception     */             
	                                              /*  模拟成异常，自动把寄存器压栈*/

    *(stk) = (INT32U)0x01000000L;                     /*  xPSR                        */ 
    *(--stk) = (INT32U)task;                          /*  Entry Point of the task     */
                                                      /*  任务入口地址                */
    *(--stk) = (INT32U)0xFFFFFFFEL;                   /*  R14 (LR)  (init value will  */
                                                      /*  cause fault if ever used)   */
                                                                           
    *(--stk) = (INT32U)0x12121212L;                   /*  R12                         */
    *(--stk) = (INT32U)0x03030303L;                   /*  R3                          */
    *(--stk) = (INT32U)0x02020202L;                   /*  R2                          */
    *(--stk) = (INT32U)0x01010101L;                   /*  R1                          */
    *(--stk) = (INT32U)parg;                          /*  R0 : argument  输入参数，ARM*/
                                                      /*  使用R0-R3寄存器传递参数，这 */
                                                      /*  里只有一个参数，所以只使用了*/
                                                      /*  R0寄存器                    */
                                                      
                                                      
                                                      /*  Remaining registers saved on*/
                                                      /*  process stack               */   
	                                                  /*  剩下的寄存器保存到堆栈      */

    *(--stk) = (INT32U)0x11111111L;                   /*  R11                         */
    *(--stk) = (INT32U)0x10101010L;                   /*  R10                         */
    *(--stk) = (INT32U)0x09090909L;                   /*  R9                          */
    *(--stk) = (INT32U)0x08080808L;                   /*  R8                          */
    *(--stk) = (INT32U)0x07070707L;                   /*  R7                          */
    *(--stk) = (INT32U)0x06060606L;                   /*  R6                          */
    *(--stk) = (INT32U)0x05050505L;                   /*  R5                          */
    *(--stk) = (INT32U)0x04040404L;                   /*  R4                          */

    return(stk);
}

/***************************************************************************************
* 函数名称: OSTaskSwHook 
*
* 功能描述: This function is called when a task switch is performed.  This allows you
*           perform other operations during a context switch. 
*
*            
* 参    数: None
*
* 返 回 值: None
*         
* 作　  者: 罗辉联
* 日  　期: 2007年11月10日
*----------------------------------------------------------------------------------------
*****************************************************************************************/

#if (OS_CPU_HOOKS_EN > 0) && (OS_TASK_SW_HOOK_EN > 0)
void  OSTaskSwHook (void)
{
#if OS_VIEW_MODULE > 0
    OSView_TaskSwHook();
#endif
}
#endif

/***************************************************************************************
* 函数名称: OSTCBInitHook, 
*
* 功能描述: This function is called by OS_TCBInit() after setting up most of the TCB.
*            
* 参    数: ptcb    is a pointer to the TCB of the task being created.
*
* 返 回 值: None
*         
* 作　  者: 罗辉联
* 日  　期: 2007年11月10日
*----------------------------------------------------------------------------------------
*****************************************************************************************/

#if OS_CPU_HOOKS_EN > 0 && OS_VERSION > 203
void  OSTCBInitHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                            /* Prevent Compiler warning                 */
}
#endif

/***************************************************************************************
* 函数名称: OSTimeTickHook, 
*
* 功能描述: This function is called every tick.
*            
* 参    数: None
*
* 返 回 值: None
*         
* 作　  者: 罗辉联
* 日  　期: 2007年11月10日
*----------------------------------------------------------------------------------------
*****************************************************************************************/

#if (OS_CPU_HOOKS_EN > 0) && (OS_TIME_TICK_HOOK_EN > 0)
void  OSTimeTickHook (void)
{
	#if OS_VIEW_MODULE > 0
		OSView_TickHook(); 
	#endif

        /* wlh:ONLY one timer here,code not perfect!!  #define OS_TMR_EN  0, until the  */
        /* code consummated!!                                                           */
	#if OS_TMR_EN > 0
		OSTmrCtr++; 
		if (OSTmrCtr >= (OS_TICKS_PER_SEC / OS_TMR_CFG_TICKS_PER_SEC)) 
		{
			OSTmrCtr = 0;
			OSTmrSignal(); 
		} 
	#endif
}
#endif

/****************** http://www.bogodtech.com *******End of file ******************/

