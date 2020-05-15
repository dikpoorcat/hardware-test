/***************************** (C) COPYRIGHT 2019 方诚电力 *****************************
* File Name          : RF.c
* Author             : 杜颖成、陈威、等
* Version            : 见历史版本信息
* Date               : 2019/03/7
* Description        : RF通信及数据处理功能实现，需要调用存储、串口等驱动文件。
************************************  历史版本信息  ************************************
* 2019/03/28     : V4.1.0
* Description   : 南网测温项目初版。基础功能完成，调试中。
*******************************************************************************/
#include "RF.h"

/*全局变量定义*/
INT8U 					TT_Sample[55][2]={0};									//55个探头每次采样的数据,内存位置
INT8U					TT_RF_Fault_Count[55]={0};								//55个探头射频接收错误分别计数
INT8U					FATFS_Lock=0;
INT8U					RF_Uart_RxBuff[RF_BuffLen] = {0};						//RF串口接收缓存
INT8U					RF_Uart_TxBuff[RF_BuffLen] = {0};						//RF串口发送缓存
struct TT_STR			TT_Info={0};											//存放已录入的探头信息
struct SAMP_MANAGE		TT_Sample_Manage={0}; 									//每个小时的温度采集管理结构体
struct LteWakeupEnable 	wakeup_en={true,true,true,true};						//当通信故障、无网络、电源欠压等情况下禁止从休眠模式唤醒。true使能，false禁止



/*遗留未整理*/
INT16U HT_Data[2] = {0};        												// 保存温湿度数据，湿度，温度=================有用吗？？？没用清理掉
OS_EVENT  * RFSGIN = (OS_EVENT *)0;

/*内部函数申明*/
INT8U RF_Data_Judge(struct Str_Msg * pMsg);
INT8U RF_Received_Data_DealWith(struct Str_Msg * pMsg);

/*******************************************************************************
* Function Name : void Task_LORA_Main(void *arg)                                                    
* Description   : RF任务函数，RF通信及数据处理功能实现。
*******************************************************************************/
void Task_RF_Main(void *arg)
{
	static INT8U	msg_fault = 0;
	static INT8U	msg_cmd=0;
	memset(TT_Sample,0xFF,sizeof(TT_Sample));									//将温度数据初始化为0xFFFF（南网协议规定）
	
	TaskActive &= RF_INACT;														//不再轮巡该任务的任务看门狗
	OSTaskSuspend(OS_PRIO_SELF);												//挂起自身任务
	TaskActive |= RF_ACT;       												//任务恢复，继续轮巡该任务看门狗
	
	B485_init(38400);															//485初始化波特率38400（根据B485DIS宏定义自动判断，但LOCAL任务存在时会强制打开）
	RF_Power_Init();       														//RF模块电源初始化
	PWRFEN();				 													//打开电源（常开）
						
	while(1)
	{
	/*任务看门狗维护并延时*/
		WDTClear(RF_Task_Prio);
		OSTimeDly(20);
	
		/*RF接收数据*/
		if(Time_Proofread==DONE)												//若已完成校时
		{	
			BSP_InitFm(RF_Num);													//调用低功耗函数后需要重新初始化
			RF_Data_Sample(30*20);												//等待采集命令邮箱30秒，并存储采集的数据
			RF_Receive_Data(1200,50);											//从RF模块获取数据，1200波特率，50时间片超时（射频模块串口最多给256字节，1200波特率时约2.13秒）
			FM_LowPower(RF_Num);												//铁电引脚低功耗配置		
			
			/*故障轮询，当故障发生可及时上报*/
			if((wakeup_en.overtime & wakeup_en.reply & wakeup_en.network & wakeup_en.battle & wakeup_en.rf_tem)&&(Fault_Manage.Need_Report==0x55))	//若允许从休眠模式唤醒，且存在故障。故障信息上报放在此处可错开探头故障判断的过程（可能存在多个探头同时故障，就同一处理完再上报）	（不放在看门狗任务的原因，是因为有可能错不开）
			{
				msg_cmd = WAKE_CMD;												//通知LTE任务切换到唤醒状态		
				OSMboxPost(Dev_CMDB0X, &msg_cmd);								//发出命令		即使在线状态收到也不会改变任何东西，若在睡眠状态则唤醒进行故障上报
				msg_fault = FAULT_CMD;											//用于发出，通知LTE任务切换到故障信息上报状态
				OSMboxPost(Fault_CMDB0X, &msg_fault);							//通过邮箱将故障信息发出
			}
		}
	}
}

/*******************************************************************************
名称：void RF_Data_Sample(u16 timeout)
功能: 接收采样命令，将采样数据存储到铁电 。
入参：u16 timeout，邮箱超时等待时长。
出参：无
返回：无
*******************************************************************************/
void RF_Data_Sample(u16 timeout)
{
	INT8U	msg_data;
	INT8U  	Err=0;
	INT32U  time=0;																					//采样时间	
	INT8U 	i=0;
	TCHAR   BUFF[50]={0};
	INT16U  TT_Data_Addr=0;																			//探头采样数据再铁电中的存放地址
	
/*判断整点，执行历史数据整理工作*/
	time = RtcGetTimeSecond();
	if((time/3600)!=(TT_Sample_Manage.Time[0]/3600) && TT_Sample_Manage.Time[0])					//已经是下个小时了！   (当有采集的时候)
	{	
		History_Data_Store();																		//将上个小时数据存储（协议中上报主站的数据都从这里来）
		memset((INT8U*)&TT_Sample_Manage,0,Sample_Manage_Len);										//并清空采集管理结构体
		TT_Sample_Manage.TT_Count=TT_Info.TT_Count;													//填充探头个数
		memcpy(TT_Sample_Manage.Newline,SIZE_OF("\r\n"));											//添加新行
		BSP_WriteDataToFm(Sample_Manage_Addr,(INT8U *)&TT_Sample_Manage.Len,Sample_Manage_Len);		//写管理结构体到铁电
	}
	
/*若收到数据采集指令，将当前内存数据写入铁电*/
	msg_data = *(INT8U *)OSMboxPend(Data_CMDB0X,timeout,&Err);										//查询邮箱
	if((Err==OS_NO_ERR)&&(msg_data==DATA_CMD))														//接到采集数据指令
	{	
		if(!TT_Sample_Manage.TT_Count||TT_Sample_Manage.Sample_Num>=60)	return;						//若无探头录入，则直接不进行数据采集，若本小时采集次数已超过60次则不再采集
		for(i=0;i<55;i++)																			//将数据写入铁电
		{			
			if(TT_Info.HaveTT[i]==0x55) 															//只要存在探头，就要填入数据并写入铁电，无效数据用FFFF代替。（开机时已在Read_TT_From_FM中将温度数据初始化为0xFFFF）
			{	
				/*故障判断，并修改内存数据*/
				RF_Fault_Judge(i);																	//判断探头是否出故障了（含数据错误和射频接收不到数据），并根据南网协议，将无效数据填充为FFFF。
				
				/*内存数据写入铁电*/
				TT_Data_Addr=Sample_Data_Addr+One_TT_Sample_Data_Len*i+TT_Sample_Manage.Sample_Num*2;
				BSP_WriteDataToFm(TT_Data_Addr,&TT_Sample[i][0],2);									//将此温度写入铁电。存完之后才进行Sample_Num++，存放由0位置开始	
				
				/*重置为默认值*/
				TT_Sample[i][0]=0xFF;																//写入之后就将数据初始化为FFFF（根据协议4.3条例，无效数据用FFH表示）
				TT_Sample[i][1]=0xFF;	
			}				
		}
		TT_Sample_Manage.Time[TT_Sample_Manage.Sample_Num] = time;									//采集时间填入
		TT_Sample_Manage.Sample_Num++;																//本小时探头采集数+1		
		BSP_WriteDataToFm(Sample_Manage_Addr,(INT8U *)&TT_Sample_Manage.Len,Sample_Manage_Len);		//写管理结构体到铁电
																				
		sprintf(BUFF, "本小时已采集%d次\r\n",TT_Sample_Manage.Sample_Num);
		BspUartWrite(2,(INT8U*)BUFF,strlen(BUFF));
	}
}

/*******************************************************************************
* Function Name : void RF_Fault_Judge(INT8U Index)
* Description   : 判断探头是否出故障了（含数据错误和射频接收不到数据），并记录到Fault_Manage对应的标志位中。并根据南网协议，将无效数据填充为FFFF。
* Input         : Index:探头号索引
*
* Return        : None
*******************************************************************************/
void RF_Fault_Judge(INT8U Index)
{	
	INT16U Temp=0;
	TCHAR BUFF[50] = {0};
	
	/*RF未接收到数据*/
	if( (TT_Sample[Index][0]&TT_Sample[Index][1])==0xFF )											//收到FFFF代表射频未接收到数据，射频错误01H
	{
		if(TT_RF_Fault_Count[Index]<5) TT_RF_Fault_Count[Index]++;									
		if((TT_RF_Fault_Count[Index]==5) && (!Fault_Manage.F_RF[Index]))							//连续未收到数据达5次，则上报错误
			NW_Fault_Manage(Index, FAULT_STA);														//标记故障发生且未恢复。调用此函数主要会标记Need_Report，在Task_RF_Main中被轮询到，从而引发故障上报动作
		
		#ifdef RF_Test				
		sprintf(BUFF, "第 %d 个探头（%X）未接收到数据！\r\n",Index+1,Unit_ID_Code[Index]);			//从1开始计数
		BspUartWrite(2,(INT8U*)BUFF,strlen(BUFF));
		#endif
	}
	
	/*接收到数据*/
	else
	{		
		TT_RF_Fault_Count[Index]=0;																	//收到任意数据即清零
		if(Fault_Manage.F_RF[Index])																//若此时存在射频未接收到数据故障
			NW_Fault_Manage(Index, NOFAULT_STA);													//射频接收故障恢复。	01H
		
		Temp=(TT_Sample[Index][0]<<8)+TT_Sample[Index][1];
		#if RF_Test
		sprintf(BUFF, "第 %d 个探头温度： %d.%d\r\n",Index+1,Temp/10,Temp%10);						//从1开始计数
		BspUartWrite(2,(INT8U*)BUFF,strlen(BUFF));
		#endif
		
		/*此探头温度数据异常*/
		if(((0xf800>Temp)&&(Temp>Tem_Upper)) || (Temp>Tem_Lower))									//若温度异常（超量程或有错误代码）
		{
			/*Temp-3984，例如4000->0x10，表示未接收到数据，详见测温探头错误代码表。（<=-55或>=125度超量程时，报正常数据，不会进这里）*/
			if((4000>Temp) || (Temp>4019)) Temp=3986;												//若接收到其他异常值（除4000~4020外的：1250~F800，FA26~FFFF），认为是温度异常，使用南网“温度异常故障编码”0x02（3986-3984）
			NW_Fault_Manage(Index+55, Temp - 3984);													//标记温度异常故障发生。Index+55传入后代表的是 02H温度异常的错误	原：NW_Fault_Manage(Index+55, FAULT_STA);	

			/*注意！修改内存中的无效温度数据*/
			TT_Sample[Index][0] = 0xFF;																//根据南网协议，无效数据填充为FFFF
			TT_Sample[Index][1] = 0xFF;
		}
		/*此探头温度数据正常*/
		else
		{
			if(Fault_Manage.F_TEM[Index])															//需要温度异常故障解除
				NW_Fault_Manage(Index+55, NOFAULT_STA);												//温度异常故障恢复。直接写入0x80
		}
	}
}


/*******************************************************************************
* Function Name : void History_Data_Store(void)
* Description   : 接收采样命令，将采样数据存储到FLASH文件中。
* Input         : None
*
* Return        : None
*******************************************************************************/
void History_Data_Store(void)
{
	INT8U 				i=0;
	struct NW_TIME		time = {0};												//年月日时分秒
	INT8U 				Store_Buff[60*2+1+2+2]={0};								//最大为每个小时60个数据+1（探头索引）+2(CRC)+2（0d0a）
	INT16U 				crc;																		
	struct SAMP_MANAGE	TT_Store_Manage={0};
	TCHAR 				BUFF[100]={0};
	INT16U				TT_Data_Addr=0;											//探头采样数据再铁电中的存放地址
	INT8U  				Hour_Byte=0;											//指向Unreport_Index文件索引表 小时Byte
	INT8U				Hour_Bit=0;												//指向Unreport_Index文件索引表 小时bit
	TCHAR 				File_Name[20]={0};
	UINT 				bw;
	INT8U				Head[2]={0XFF,0XAA};									//文件头 FF AA
	INT8U   			Tail[2]={0XFF,0XBB};									//文件头 FF BB
		
	if(!TT_Sample_Manage.Sample_Num) return;									//未采集数据，直接退出，不创建文件
	if(!SecondToNwTime(TT_Sample_Manage.Time[0],&time)) 						//返回时间格式错误，不进行存储
	{																								
		BspUartWrite(2,SIZE_OF("时间错误，无法创建文件----------------------------------\r\n"));OSTimeDly(2);
		return;
	}
	
	sprintf(File_Name,"SUB%d/%02d%02d",time.mday,time.mday,time.hour);								//生成文件名
	while(FATFS_Lock)	OSTimeDly(20);
	FATFS_Lock=1;
	BspUartWrite(2,(INT8U*)File_Name,strlen(File_Name));	
	BspUartWrite(2,SIZE_OF("文件开始写入\r\n"));

	/*打开对应日期目录，创建文件，写入文件*/
	for(i=0;i<3;i++)
	{
		if (f_mount(&fs, "", 1))continue;													    	//挂载
		if (f_open(&fil,File_Name, FA_CREATE_ALWAYS | FA_READ | FA_WRITE ))continue;				//创建并覆盖文件（以绝对路径来创建），以当前时间（日时）命名

		
		f_write(&fil, Head,2, &bw);																	//写文件头 FF AA
		/*从铁电中将一个小时的数据读取出来，写入到W25Q256*/
		BSP_ReadDataFromFm(Sample_Manage_Addr,(INT8U *)&TT_Store_Manage,Sample_Manage_Len);
		TT_Store_Manage.Len=TT_Store_Manage.Sample_Num*2+1+2+2;										//每条数据的长度  采样次数*2+1（探头索引）++2(CRC)+2（0D0A）
		TT_Store_Manage.crc=RTU_CRC((INT8U *)&TT_Store_Manage,Sample_Manage_Len-2-2);				//计算CRC，用于读写校验 【校验长度：总长度-2（0D0A）-2（crc】	
		memcpy(TT_Store_Manage.Newline,SIZE_OF("\r\n"));
		f_write(&fil, (INT8U *)&TT_Store_Manage,Sample_Manage_Len, &bw);							//写完指针自动指向下一位				需要读出来检验一下？？
		
		
		for(i=0;i<55;i++)																			//探头数据依次写入W25Q256
		{
			if(TT_Info.HaveTT[i]==0x55)
			{
				Store_Buff[0]=Unit_ID_Code[i];														//填入探头对应的功能单元码
				TT_Data_Addr=Sample_Data_Addr+One_TT_Sample_Data_Len*i;
				BSP_ReadDataFromFm(TT_Data_Addr,&Store_Buff[1],TT_Store_Manage.Sample_Num*2);		//填充该探头一个小时的采样数据
				crc=RTU_CRC(Store_Buff,TT_Store_Manage.Len-2-2);									//-2（0D0A）-2（crc）
				Store_Buff[TT_Store_Manage.Len-2-2]=(crc>>8)&0xff;
				Store_Buff[TT_Store_Manage.Len-1-2]=crc&0xff;
				memcpy(&Store_Buff[TT_Store_Manage.Len-2],SIZE_OF("\r\n"));							//数据尾加0D0A
				f_write(&fil,Store_Buff,TT_Store_Manage.Len, &bw);									//写完指针自动指向下一位
				
				memset(Store_Buff,0,sizeof(Store_Buff));	
			}				
		}
		f_write(&fil, Tail,2, &bw);																	//写文件尾 FF BB	

		Hour_Byte = time.hour/8;																				
		Hour_Bit  = time.hour%8;
		Unreport_Index[time.mday-1][Hour_Byte]&=~(0x80>>Hour_Bit);									//将该文件对应文件索引表中标志位写0，代表该文件未上报
		BSP_WriteDataToFm(Unreport_Index_Addr,(INT8U*)Unreport_Index,Unreport_Index_Len);			//未上报数据索引表写入铁电
		
#ifdef RF_Test	
		BspUartWrite(2,(INT8U*)File_Name,strlen(File_Name));
		BspUartWrite(2,SIZE_OF("文件写入完成\r\n"));OSTimeDly(2);	
		sprintf(BUFF, "共%d个探头，完成采集%d次\r\n",TT_Store_Manage.TT_Count,TT_Store_Manage.Sample_Num);			
		BspUartWrite(2,(INT8U*)BUFF,strlen(BUFF));OSTimeDly(2);
		memset(BUFF,0,sizeof(BUFF));
		/*检验写的效果的部分！(再开大空间进行比较所有内容会内存溢出)只进行CRC校验*/
		memset(&TT_Store_Manage,0,Sample_Manage_Len);
		f_lseek(&fil,2);
		f_read(&fil,&TT_Store_Manage,Sample_Manage_Len,&bw);
		if(TT_Store_Manage.crc!=RTU_CRC((INT8U *)&TT_Store_Manage,Sample_Manage_Len-2-2))			
			BspUartWrite(2,SIZE_OF("文件采集管理数据校验错误\r\n"));OSTimeDly(2);	
		
		for(i=0;i<TT_Store_Manage.TT_Count;i++)
		{
			f_read(&fil,Store_Buff,TT_Store_Manage.Len,&bw);
			crc=(Store_Buff[TT_Store_Manage.Len-2-2]<<8)+Store_Buff[TT_Store_Manage.Len-1-2];
			if(crc!=RTU_CRC(Store_Buff,TT_Store_Manage.Len-2-2))
				sprintf(BUFF, "文件采集温度数据第%d校验错误\r\n", i);	
				BspUartWrite(2,(INT8U*)BUFF,strlen(BUFF));OSTimeDly(2);
				memset(BUFF,0,sizeof(BUFF));			
			memset(Store_Buff,0,sizeof(Store_Buff));			
		}
		
		BspUartWrite(2,SIZE_OF("文件数据校验结束\r\n"));OSTimeDly(5);
#endif			
		f_close(&fil);																				//关文件
		f_mount(0, "", 0);																			//卸载
		break;
	}
	FATFS_Lock=0;
}

/*******************************************************************************
名称：void RF_Receive_Data(u32 rate, u16 timeout)
功能: 读取RF接收板中的温度数据进行解析并填入对应的数据结构体中。
入参：u32 rate，串口通信波特率；u16 timeout，超时等待时长。
出参：无
返回：无
*******************************************************************************/
void RF_Receive_Data(u32 rate, u16 timeout)
{
	INT8U RFCmd[] = RF_READ_CMD;
	INT8U RFCmdLen = sizeof(RFCmd);
	INT8U Err =0;
	struct Str_Msg *pRfMsg = (struct Str_Msg *)0; 
				
	RF_Uart_init(rate);																				//初始化RF串口
	memset(RF_Uart_RxBuff,0x00,RF_BuffLen);															//清空串口接收缓存
	BspUartWrite(1,RFCmd,RFCmdLen);    																//发送获取数据指令
	StopModeLock++;
	pRfMsg = (struct Str_Msg *)OSMboxPend(RFSGIN,timeout,&Err);   									//等待串口消息 timeout个时间片（射频模块串口最多给256字节，1200波特率时约2.13秒）
	if(StopModeLock) StopModeLock--;

	if(Err==OS_NO_ERR && pRfMsg)
	{
		if( pRfMsg->MsgID == BSP_MSGID_RFDataIn )													//判断邮箱中的消息是否为RF串口给的
		{
			 if(pRfMsg->DataLen <= 256)																//且数据不超过长度256字节（由射频模块决定）
			 {
				 if(RF_Data_Judge(pRfMsg))															//且协议校验通过
				 {						
					if(pRfMsg->pData[2]==0x01) 														//若CMD=0X01，表示获取接收板的测温数据
						RF_Received_Data_DealWith(pRfMsg);											//将数据填入到探头数据结构体当中
				 }
			 }
		}						
	}
	BSP_UART_RxClear(pRfMsg->DivNum);																//清消息缓存
	RF_LowPower();																					//与上面RF初始化配对使用	
}


/******************************************************************************
* Function Name: void RF_Power_Init(void)
* Description:   RF 电源引脚配置								 
* Input:  Nothing
* Output: Nothing
******************************************************************************/
void RF_Power_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(PWRF_Port_CK,ENABLE);													//开时钟
	PWRFDIS();																						//设置为关闭	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =PWRF_PIN;
	GPIO_Init(PWRF_Port, &GPIO_InitStructure);	
}

/*******************************************************************************
* Function Name : void RF_Uart_init(unsigned int rate)
* Description   : Rf接收串口初始化
* Input         : rate : 波特率
*
* Return        : None
*******************************************************************************/
void RF_Uart_init(unsigned int rate)
{
	UARTx_Setting_Struct UARTInit = {0};	
	
	if(RFSGIN == NULL) RFSGIN = OSMboxCreate(0);
	else RFSGIN->OSEventPtr= (void *)0;																//清消息邮箱，不清会导致误判 ZE
		
	UARTInit.BaudRate = rate;
	UARTInit.Parity   = BSPUART_PARITY_NO;
	UARTInit.StopBits = BSPUART_STOPBITS_1;
	UARTInit.DataBits = 8;
	UARTInit.RxBuf    = RF_Uart_RxBuff;			   
	UARTInit.RxBufLen = RF_BuffLen;     
	UARTInit.TxBuf    = RF_Uart_TxBuff;
	UARTInit.TxBufLen = RF_BuffLen;     
	UARTInit.Mode     = UART_DEFAULT_MODE;	  														//普通串口
	
	BSP_UART_Init(1,&UARTInit,RFSGIN);
}

/******************************************************************************* 
* Function Name  : void RF_LowPower(void)
* Description    : RF进入低功耗，并对相应IO口作低功耗处理
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/
void RF_LowPower(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1 ,DISABLE);										//关闭U1时钟及复用时钟（注意：ADC也用到这个） | RCC_APB2Periph_AFIO	//不关复用时钟|RCC_APB2Periph_AFIO（各串口和AD都用到这个）
	
	/*电源使能、TX、RX模拟输入*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;													//模拟输入
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;														///PA9/U1_TX
	GPIO_Init(GPIOA, &GPIO_InitStructure);										
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;														//PA10/U1_RX
	GPIO_Init(GPIOA, &GPIO_InitStructure);										
	
	USART_ITConfig( USART1, USART_IT_TC, DISABLE);
	USART_ITConfig( USART1, USART_IT_RXNE, DISABLE);
	USART_Cmd( USART1,DISABLE);		
}

/*******************************************************************************
* Function Name : INT8U RF_Data_Judge( struct Str_Msg * pMsg)
* Description   : 射频头传来的数据有效性判断
* Input         : 
* Return        : 
*******************************************************************************/
INT8U RF_Data_Judge(struct Str_Msg * pMsg)
{
	INT8U i = 0;
	INT8U Length = 0;
	INT8U CS = 0;
	
	if(pMsg->DivNum!=1) return 0;
	if(pMsg->DataLen<11) return 0;
	
	Length = pMsg->pData[3] + 4;																	//数据域长度+AA 2D 01 DataLen
	for(i = 0;i < Length;i++) CS += pMsg->pData[i];													//计算累加和
	if(CS != pMsg->pData[Length]) return 0;															//累加和校验
	
	return 1;
}

/*******************************************************************************
* Function Name : void RF_Data_Extract(struct Str_Msg * pMsg)
* Description   : 射频头上传来的数据的解析，丢弃无效数据，并按绑定的探头查找Index，存入TT_Sample[Index]。
* Input         : pMsg:信息结构体指针
*
* Return        : 无
*******************************************************************************/
INT8U RF_Received_Data_DealWith(struct Str_Msg * pMsg)
{
	INT8U 	i = 0;																					//AA 2B 01 DataLen CNT ……
	INT8U 	Cnt = pMsg->pData[4];																	//数据域的第一个字节，表示有效测温点个数
	INT8U 	*Ptr = &pMsg->pData[5];																	//指向数据域中，测温点数据的指针
	INT8U	Index = 0;   
	
	if(!TT_Info.TT_Count) return 0;																	//没有录入探头
	
	for(i=0; i<Cnt; i++)
	{	
		Index = CMP_TT_ID(Ptr);																		//得到比对成功的探头索引
		if(Index != 0xff)	  																		//0XFF表示匹配失败，丢弃此探头数据
		{
			memcpy(TT_Sample[Index],&Ptr[2],2);														//按照索引将对应探头的温度填入，若未到下次采样命令过来取值，新的数据会覆盖旧的数据			
		}
		Ptr += 4;  
	}
	return 1;
}
																				
/*******************************************************************************
* Function Name : INT8U CMP_TT_ID(INT8U* pIn)                                                     
* Description   : 比对收到的探头数据是否在已录入的探头范围内
* Input         : pIn ：输入数组               
* Return        : 探头索引0~54  （索引可对应到功能单元识别码）
				  0xff:不在索引范围内
*******************************************************************************/
INT8U CMP_TT_ID(INT8U* pIn)
{
	INT8U i=0;
	for(i=0;i<55;i++)
	{
		if(!memcmp(pIn,&TT_Info.TT_ID[i][0],2)) return i;											//如果符合，则返回索引，数据按索引位置存放				
	}
	return 0xff;																					//未匹配上已录入的探头ID
}

/*******************************************************************************
* Function Name : void Read_TT_From_FM(void);	                                                   
* Description   : 将录入的探头信息从铁电中读取出来，用于匹配收到的探头温度,并按顺序填充探头温度信息结构体的探头ID部分,为后续探头填入数据做准备
* Input         :              
* Return        : 		 
*******************************************************************************/
void Read_TT_From_FM(void)
{	
	INT8U i=0;
	
	BSP_ReadDataFromFm(TT_Count_Addr,&TT_Info.TT_Count,sizeof(TT_Info));							//上电读出录入的探头信息到TT_Info结构体
	BSP_ReadDataFromFm(Sample_Manage_Addr,(u8*)&TT_Sample_Manage,Sample_Manage_Len);				//上电读出采集管理结构体
	memcpy(TT_Sample_Manage.Newline,SIZE_OF("\r\n"));												//结构体结尾加上0x0D 0x0A
	TT_Sample_Manage.TT_Count=TT_Info.TT_Count;
	
	for(i=0;i<55;i++)
	{
		if((TT_Info.TT_ID[i][0]==0)&&(TT_Info.TT_ID[i][1]==0))TT_Info.HaveTT[i]=0;					//标记此位置无探头，（当两字节都是0时才标记为空）
		else TT_Info.HaveTT[i]=0x55;
	}	
}	
