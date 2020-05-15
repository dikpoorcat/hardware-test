#include "ME909S.h"


/*全局变量*/
OS_EVENT	*GPRSBOX = (OS_EVENT *)0;
struct Str_Msg 	*pGprsMess=(struct Str_Msg *)0;//GPRS 统一消息
INT8U UART3_Tx_Buff[LTE_BUFF_LEN] = {0};										//用于串口3发送
INT8U UART3_Rx_Buff[LTE_BUFF_LEN] = {0};										//用于串口3接收
INT8U ME909SCOMMANDID=0;														//0x40：接收到返回信息并存储； 0x10：断网了
GPRS_STRUCT GPRS = {0};
A58Control A58Cont;																// zzs add this for 在传送图片的时候，标识一下GPRS模块处在什么状态，其实完全就可以用本方案原始的 A8500COMMANDID，但图片发送协议完全不同于本工程的其他的上传过程，所以就借用图片发送的很多东西和模式。
																				// 主要目的是不想对图片发送过程的程序结构做太大的改动。就让它单独用一套过程控制状态机好了。=============================到时候清理掉






/******************************************************************************* 
* Function Name  : void ME909S_LowPower(void)
* Description    : ME909S全网通模块电路低功耗。关串口时钟，配置控制IO口。
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/ 
void ME909S_LowPower(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;

//	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, DISABLE );						//不关复用时钟|RCC_APB2Periph_AFIO（各串口和AD都用到这个）
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3, DISABLE );					//关闭U3时钟

	/*GPRSEN、TX、RX模拟输入*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;								//模拟输入
	
	GPIO_InitStructure.GPIO_Pin = ME909S_PW_PIN;								//GPRSEN（PC10）
	GPIO_Init(ME909S_PW_Port, &GPIO_InitStructure);								//
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;									//USART3 Tx (PB10)
	GPIO_Init(GPIOB, &GPIO_InitStructure);										//
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;									//USART3 Rx (PB11)
	GPIO_Init(GPIOB, &GPIO_InitStructure);										//
	
	/*G1RESET、G1ONOFF推挽输出*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;							//推挽输出
	
	GPIO_ResetBits(ME909S_REST_PORT, ME909S_REST_PIN);							//拉低
	GPIO_InitStructure.GPIO_Pin = ME909S_REST_PIN;								//G1RESET（PB12）
	GPIO_Init(ME909S_REST_PORT, &GPIO_InitStructure);							//
	
	GPIO_ResetBits(ME909S_ONOFF_PORT, ME909S_ONOFF_PIN);						//拉低				
	GPIO_InitStructure.GPIO_Pin   = ME909S_ONOFF_PIN;							//G1ONOFF（PB13）
	GPIO_Init(ME909S_ONOFF_PORT, &GPIO_InitStructure);            				//
}

/*******************************************************************************
* Function Name : void ME909S_PinInit(void)                                   
* Description   : MES909S 串口配置，电源及控制引脚初始化
* Input         : 
*                 
* Return        :
*******************************************************************************/
void ME909S_PinInit(void)
{
    GPIO_InitTypeDef 	GPIO_InitStructure = {0};	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC,ENABLE);
		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	
	GPIO_InitStructure.GPIO_Pin   = ME909S_PW_PIN;
	GPIO_Init(ME909S_PW_Port, &GPIO_InitStructure);								//PWEN 推挽
	
	GPIO_InitStructure.GPIO_Pin   = ME909S_ONOFF_PIN;
	GPIO_Init(ME909S_ONOFF_PORT, &GPIO_InitStructure);            				//ONOFF 推挽
	
	GPIO_InitStructure.GPIO_Pin = ME909S_REST_PIN;
	GPIO_Init(ME909S_REST_PORT, &GPIO_InitStructure);							//RESET 推挽

	ME909S_REST_H();
	ME909S_ONOFF_H();     														//这俩实际是拉低
	ME909S_PW_OFF();     														//电源关闭
}


/*******************************************************************************
* Function Name : void ME909SInit(INT32U rate)                                        
* Description   : MES909S 串口配置，电源及控制引脚初始化
* Input         : rate    ：串口波特率设置
*                 
* Return        :
*******************************************************************************/

void ME909SInit(INT32U rate)
{
	UARTx_Setting_Struct UARTInit = {0};
	
	if(GPRSBOX == NULL) GPRSBOX = OSMboxCreate(0);								//GPRS加入邮箱配置
	else GPRSBOX->OSEventPtr= (void *)0;										//清消息邮箱，不清会导致误判 ZE		
	
	UARTInit.BaudRate = rate;
	UARTInit.Parity   = BSPUART_PARITY_NO;
	UARTInit.StopBits = BSPUART_STOPBITS_1;
	UARTInit.DataBits = 8;
	UARTInit.RxBuf    = UART3_Rx_Buff;	   
	UARTInit.RxBufLen = LTE_BUFF_LEN ;
	UARTInit.TxBuf    = UART3_Tx_Buff;   
	UARTInit.TxBufLen = LTE_BUFF_LEN ;
	UARTInit.Mode     = UART_DEFAULT_MODE;										//UART_HALFDUP_MODE
	BSP_UART_Init(ME909SPort,&UARTInit,GPRSBOX);								//配置4G模块串口，串口号ME909SPort=3
	
    ME909S_PinInit();            												//GPRS电源引脚初始化
}

/*******************************************************************************
* Function Name : INT8U ME909S_ON(void)                                       
* Description   : MES909S 模块的开机   打开电源后需延迟300MS以上，将POWERONOFF拉低1s以上再拉高
* Input         :
*                 
* Return        : 
*******************************************************************************/
INT8U ME909S_ON(void)
{
	INT8U				*pRet = NULL, ST=0;
		
	ME909S_PW_OFF();     
	ME909S_REST_H(); 
    ME909S_ONOFF_H();
	ME909S_PW_ON() 	
	OSTimeDly(20);																//等待300ms以上

	ME909S_ONOFF_L();
	OSTimeDly(30);																//开机 拉低1s以上
	ME909S_ONOFF_H();
	OSTimeDly(20);												
	
	pRet=ME909SCommandP(AT_AT, "SYSSTART", 2);     								//使用发送AT激活MAX3373，同时等待开机成功返回（>7S）,(在模块波特率没有设定的时候，发送AT可进行波特率自适应，支持9600自适应)
	if(pRet) 	ST= 1;
	else ST=0;
	
	if(!ST){
		pRet=ME909SCommandP(AT_AT, "OK", 2);             						//存在已返回SYSSTART但未收到的情况时，能回复OK也可继续发送
		if(pRet) 	ST= 1;
		else ST=0;}
	
	if(ST){
		pRet=ME909SCommandP(AT_ATE, "OK", 2);           	    				//关闭回显。这个耗时较长
		if(pRet) return 1;}
			
	BspUartWrite(2,SIZE_OF("开机失败----------------------------------\r\n"));OSTimeDly(1);
	return 0;																	//开机失败	
}

/*******************************************************************************
* Function Name : void ME909S_OFF(void)                                  
* Description   : MES909S 模块的关机  且关闭模块电源
* Input         :
*                 
* Return        : 
*******************************************************************************/
void ME909S_OFF(void)      														//反正都要关电源，关不关机有啥区别？？？
{
#if 1
    ME909SCommandP(AT_MSO, "0", 2); 	
	OSTimeDly(10);																//预留保存用户数据时间
#else						 							
    ME909S_REST_H(); 
    ME909S_ONOFF_H();
	OSTimeDly(5);
	
	ME909S_ONOFF_L();
	OSTimeDly(100);	 															//关机 拉低4s以上
	ME909S_ONOFF_H();
	OSTimeDly(5);	
#endif	
	ME909S_PW_OFF();    														//关闭模块电源
}

/*******************************************************************************
* Function Name :void DeviceRstOnCMD(void)                            
* Description   :接到装置重启命令后，MES909S 模块的关机  且关闭模块电源 再进行单片机复位
* Input         :
*                 
* Return        : 
*******************************************************************************/
void DeviceRstOnCMD(void)
{
	ME909S_Trans_OFF();			//退出透传模式进入命令模式
	ME909S_OFF();				//模块关机关电源
	McuSoftReset();				//立即复位单片机
}

/*******************************************************************************
* Function Name :INT8U ME909S_RateChange(void)                                
* Description   :修改波特率为9600 暂时先不搞波特率参数了，固定修改为某值，不然还有32位hex转ascii的问题，这个函数应该很少需要用到
* Input         :
*                 
* Return        : 
*******************************************************************************/
INT8U ME909S_RateChange(void)
{
	INT8U				*pRet=NULL, ST=0;
	
	pRet=ME909SCommandP(AT_AT, "OK", 2);      									//尝试沟通
	if(pRet) 	ST=1;
	else ST=0;
	
	if(ST)
	{ pRet=ME909SCommandP(AT_IPR9600, "OK", 2);    							//设置固定9600波特率
	if(pRet) 	ST=1;
	else ST=0;}
	
	
	if(ST)   
	OSTimeDly(300);		                                    					//放电
	ME909SInit(9600);
	ST=ME909S_ON();                                  							//重新开机
	
	
	if(ST)
	{ pRet=ME909SCommandP(AT_AT, "OK", 2);      								//尝试沟通
	if(pRet) 	ST=1;
	else ST=0;}
				
					
	if(ST)
	{ pRet=ME909SCommandP(AT_IPRCHECK, "+IPR", 2);    						//查询波特率
	if(pRet) 	ST=1;
	else ST=0;}

	if(ST) return 1;
	return 0;
}

/*******************************************************************************
* Function Name :INT8U ME909S_Get_Signal_Strength(void)                            
* Description   :获得模块的信号强度，用于填充心跳包 一个字节
* Input         :无
*                 
* Return        : 信号强度百分比		0:无信号或信号强度未知或不可测	
*******************************************************************************/
INT8U ME909S_Get_Signal_Strength(void)
{
	
	INT8U				*pRet=NULL;
	INT8U				Signal_Strength=0, i=0;

	for(i=0;i<3;i++)
	{
		pRet=ME909SCommandP(AT_HCSQ, "^HCSQ:", 2);      						//查询信号强度   模块硬件（天线）能搜索到的网络质量
		if(pRet) 
		{
			pRet = ScanAsicc(pRet,15,(INT8U *)"\",",2); 						//定位RSSI数据的指针
			if(pRet)  
			{
				if((pRet[0]==0x32)&&(pRet[1]==0x35)&&(pRet[2]==0x35)) continue;	//当测得RSSI=255时，信号强度为未知或不可测,重新测
				Signal_Strength=100*((pRet[0]-0x30)*10+(pRet[1]-0x30))/96;		//华为模块测得的RSSI_MAX=96（-25dbm），依此来算百分比
				return Signal_Strength;
			}					
		}
	}
	return 0;
}

/*******************************************************************************
* Function Name :INT8U HB_Get_Signal_Strength(void)                           
* Description   :为心跳组帧获取一个无线信号强度百分比(需关闭透传，输入查询指令，再打开透传)
* Input         :
*                 
* Return        : 0-100:返回信号强度（0表示无信号或信号不可测） 0xFF：打开透传失败，需要重新尝试
*******************************************************************************/
INT8U HB_Get_Signal_Strength(void)
{
	INT8U Signal_Strength=0;
	
	ME909S_Trans_OFF();										//退出透传模式  
	Signal_Strength=ME909S_Get_Signal_Strength();			//
	if(ME909S_Trans_ON()) return Signal_Strength;			//重回透传模式
	return 0XFF;											//重回透传模式失败
}

/*******************************************************************************
* Function Name :INT8U ME909S_CONFIG(void)                               
* Description   :模块开机后进行本机配置，本机配置指令等待时间都缩短一些
* Input         :
*                 
* Return        : 
*******************************************************************************/
INT8U ME909S_CONFIG(void)
{
	INT8U				*pRet=NULL, ST=0;
	
	{ pRet=ME909SCommandP(AT_CURC, "OK", 2);       		 					//关闭自动上报模块状态
	if(pRet) 		ST= 1;
	else ST=0;}
	
#if 0		//设置天线模式
	if(ST)																									
	{ pRet=ME909SCommandP(AT_ANT, "OK", 2);           	 					//设置天线模式       //此设置会导致设备断网重新寻网，并且导致寻网之后关闭回显和关闭自动上报设置会失效，需要重新设置
	if(pRet) 	ST=1;												 			//默认所有制式使用双天线，非必要不进行设置
	else ST=0;}

	if(ST)
	{ pRet=ME909SCommandP(AT_ANTCHECK, "OK", 2);      	 					//查询天线模式
	if(pRet) 	ST=1;
	else ST=0;}
	
	//设置LED闪灯模式 
	if(ST)  //可省略
	{ pRet=ME909SCommandP(AT_LED, "OK", 2);          	 						//设置LED闪灯模式，默认，该设置掉电保存，只需设置一次
	if(pRet) 	ST=1;
	else ST=0;}

	if(ST)  //可省略
	{ pRet=ME909SCommandP(AT_LEDCHECK, "OK", 2);     	 						//查询LED模式  
	if(pRet) 	ST=1;
	else ST=0;}
#endif	
	
#if 0	//设置网络连接优先级
	if(ST)
	{ pRet=ME909SCommandP(AT_FREQLOCKCHECK, "OK", 2);     					//锁频查询
	if(pRet) 	ST=1;
	else ST=0;}
	
	if(ST)
	{ pRet=ME909SCommandP(AT_SYSCFGEXCHECK, "OK", 2);    						//查询系统扩展配置 
	if(pRet) 	ST=1;
	else ST=0;}	
	
	if(ST)
	{ pRet=ME909SCommandP(AT_SYSCFGEX, "OK", 2);         						//配置系统扩展   这个配置是掉电保存的，设置一次即可
	if(pRet) 	ST=1;
	else ST=0;}
	
//	if(ST)  //可省略
//	{ pRet=ME909SCommandP(AT_SYSCFGEXCHECK, "OK", 2);     					//查询系统扩展配置  
//	if(pRet) 	ST=1;
//	else ST=0;}
#endif	
	
	
	/*透传相关参数设置*/		
	if(ST)
	{pRet=ME909SCommandP(AT_IPCFL5, "OK", 2);       							//透传超时时间设置
	if(pRet) 	ST= 1;	
	else ST=0;}
	
	if(ST)
	{pRet=ME909SCommandP(AT_IPCFL10, "OK", 2);     							//透传buffer设置   
	if(pRet) 	ST= 1;
	else ST=0;}
	
	if(ST)
	{pRet=ME909SCommandP(AT_IPCFL12, "OK", 2);     							//透传模式设置
	if(pRet) 	ST= 1;
	else ST=0;}
	
	if(ST) return 1;
	return 0;
}

/*******************************************************************************
* Function Name :INT8U ME909S_REG(void)                             
* Description   :模块进行网络注册
* Input         :
*                 
* Return        : 
*******************************************************************************/
INT8U ME909S_REG(void)
{
	INT8U *pRet=NULL;
	INT8U ST=0;
	INT8U i=0;
	INT8U j=0;
	INT8U retry=2;
	
	for(i=0;i<retry;i++)
	{
//		pRet=ME909SCommandP(AT_CFUN, "OK", 2);          						//设置模块为online mode，且不重启模块
//		if(pRet) 	ST=1;
//		else ST=0;

//		pRet=ME909SCommandP(AT_CFUNCHECK, "OK", 2);    						//模块模式查询
//		if(pRet) 	ST=1;
//		else ST=0;
		
//		if(ST)
		{ pRet=ME909SCommandP(AT_CPIN, "OK", 2);       						//查询SIM卡是否正常		#define SZOF(a)   a,sizeof(a)-1  
		if(pRet) 	ST=1;
		else ST=0;}
		
		if(ST)
		{ pRet=ME909SCommandP(AT_COPS, "OK", 2);      	 	   				//设置运营商为自动选择
		if(pRet) 	ST=1;
		else ST=0;}		
		
		j=6;          //这里采用多次短时间的查询，目的是在SIM卡只支持2G/3G的时候搜索运营商会较慢，多次短时间是能最快连上的方式 这里最多给108s
		while(j--)
		{
			if(ST)
			{ pRet=ME909SCommandP(AT_COPSCHECK, "0,0,", 1);     				//确认运营商已连接上     这里如果是只回复COPS:0说明还未连上运营商
				if(pRet) 	
				{
					ST=1;
					break;
				}													   		   //正确返回应该是+COPS: 0,0,"CMCC",7 如果是4G会很快找到，其他的可能需要更长延时，这里适当增加尝试次数， 7：4G；0：GSM；2：UTRAN （3G）			
			}				
			if(j==0) ST=0;
	    }

		if(ST)
		{ pRet=ME909SCommandP(AT_CREG, "OK", 2);    			   				//设置网络注册状态自动上报	GSM
		if(pRet) 	ST=1;
		else ST=0;}	

		if(ST)
		{ pRet=ME909SCommandP(AT_CGREG, "OK", 2);    		   					//设置网络注册状态自动上报	GPRS
		if(pRet) 	ST=1;
		else ST=0;}
		
		j=3;
		while(j--)
		{
			if(ST)  //有一个注册上就OK了，优先GPRS
			{ 
				pRet=ME909SCommandP(AT_CGREGCHECK, "+CGREG:", 1);    			//查询当前GPRS网络注册状态  正确回复+CGREG: 1,1    //GPRS
				if(pRet) 	
				{
					pRet = ScanAsicc(pRet,5,(INT8U *)"1,1",3); 					//找","
		            if(pRet)  
					{
						ST=1;         //注册成功跳出	
						break;
					}					
				}	
				
				pRet=ME909SCommandP(AT_CREGCHECK, "+CREG:", 1);    			//查询当前GSM网络注册状态  正确回复+CREG: 1,1		/*当使用GSM网络时（2G）*/
				if(pRet) 	
				{
					pRet = ScanAsicc(pRet,5,(INT8U *)"1,1",3); 					//找","
		            if(pRet)  
					{
						ST=1;         											//注册成功跳出	
						break;
					}					
				}	
				
				OSTimeDly(5*20);       											//不管是没返回还是返回的不是有效内容，都等5s再询问
			}	
		    if(j==0) ST=0;           											//三次机会都用光了	
		}

#if 0		
		if(ST) //可省略
		{ pRet=ME909SCommandP(AT_SYSINFO, "OK", 2);    						//查询扩展系统信息   注册上之后可查询注册上的是哪个网络
		if(pRet) ST=1;
		else ST=0;}			 													//^SYSINFOEX: 2,3,0,1,,1,"GSM",3,"EDGE"：服务有效，PS+CS服务，非漫游状态，有效SIM卡，，GSM制式代码，GSM制式，制式子类EDGE代码，制式子类EDGE
#endif

		if(ST) return 1;
		OSTimeDly(100);
    }

	if(!Net_Fault_Time) Net_Fault_Time=RtcGetTimeSecond();						//网络注册失败 获取当前的时间	单位：秒 
	return 0;
}

/*******************************************************************************
* Function Name :INT8U ME909S_IPINIT(void)                         
* Description   :模块进行TCP/IP初始化
* Input         :无
*                
* Return        : 1：设置成功
*******************************************************************************/
INT8U ME909S_IPINIT(void)
{
	INT8U				*pRet=NULL;
	TCHAR				AT_IPINIT[120] = {0};									//当前公司使用的物联卡用CMMTM或CMIOT(支持4G)会连接更快，若使用CMNET则较慢

	snprintf(AT_IPINIT, APN_Len+12, "AT^IPINIT=\"%s\"\r\n", APN);				//写入到数组，用于发送AT指令
	pRet=ME909SCommandP(AT_IPINIT, "OK", 2);    								//初始化TCP/IP     //这个时间可能较长
	//第一次没等到OK的话（可能等待时间不够长）后面再发可能会返回ERR:1013：The network has been opened already ,后续再查状态若已打开则直接继续。
	if(pRet) 	return 1;

	else{
	pRet=ME909SCommandP(AT_IPINITCHECK, "^IPINIT:", 3);   						//查询TCP初始化结果 
	if(pRet){
		pRet = ScanAsicc(pRet,5,(INT8U *)"1,",2); 								//找","
		if(pRet) return 1;                                              		//The network has been opened already 	
		}
	}	
	return 0;
}

/*******************************************************************************
* Function Name :INT8U ME909S_IPOPEN(INT8U SocketID,INT8U *IP,INT8U *Remote_Port,INT16U Local_Port)                        
* Description   :模块使用一个socket打开一个IP链接
* Input         :SocketID:链接号（1-5）
				 IP：写入flash用于连接的IP地址    remote Port   :远程端口，由设置决定  
				 local Port（当有多个链接要打开时，最好指定避免冲突，这里指定为1000，否者可以省略该参数由模块自动分配）               
* Return        : 1:设置成功
*******************************************************************************/
INT8U ME909S_IPOPEN(INT8U SocketID,INT8U *IP,INT8U *Remote_Port,INT16U Local_Port)  
{
	INT8U				*pRet=NULL, *P=NULL;
	INT8U				Num=0, MaxNum=0;
	INT16U				Re_Port=(Remote_Port[0]<<8)+Remote_Port[1];
	INT8U				ATIPOPEN[50] = {0};   //用于打开指定IP
	
	if((SocketID<1)||(SocketID>5)) return 0;
	
	/*填充ATIPOPEN[]*/
	memcpy(ATIPOPEN,(INT8U *)"AT^IPOPEN=",10);                          	
	ATIPOPEN[10]=SocketID+0X30;
	memcpy(&ATIPOPEN[11],(INT8U *)",\"UDP\",\"",8);                    			//到这里写到了AT^IPOPEN=1,"TCP","   共19个字节			//如果是UDP连接则这里修改为UDP
	
	/*有空改写成snprintf*/
	MaxNum=19;
	P=&ATIPOPEN[19];
	Num=INT16UHexToAscii(IP[0],P);
	P=P+Num;
	MaxNum+=Num;
	*P++='.';
	MaxNum++;
	Num=INT16UHexToAscii(IP[1],P);
	P=P+Num;
	MaxNum+=Num;
	*P++='.';
	MaxNum++;
	Num=INT16UHexToAscii(IP[2],P);
	P=P+Num;
	MaxNum+=Num;
	*P++='.';
	MaxNum++;
	Num=INT16UHexToAscii(IP[3],P);
	P=P+Num;
	MaxNum+=Num;
	*P++='\"';														  
	MaxNum++;
	*P++=',';
	MaxNum++;
	Num=INT16UHexToAscii(Re_Port,P);
	P=P+Num;
	MaxNum+=Num;
	*P++=',';
	MaxNum++;
	Num=INT16UHexToAscii(Local_Port,P);               
	P=P+Num;
	MaxNum+=Num;                                                 				//到这里写到了AT^IPOPEN=1,"TCP","121.199.13.107",8301,1000
	*P++='\r';
	*P='\n';
	MaxNum+=2;                           

	pRet=ME909SCommandP((TCHAR*)ATIPOPEN, "OK", 2);         					//初始化TCP/IP
	if(pRet) 	return 1;

	else{
	pRet=ME909SCommandP(AT_OPENCHECK, "^IPOPEN", 3);      						//查询IP连接情况确认IP连接成功       
	if(pRet) 	return 1;														//回复是：^IPOPEN: 1,"TCP",1000,"121.199.13.107",8301,1,1400  这里1指的是端口UART，最大字长1400
	}
	return 0;
}

/*******************************************************************************
* Function Name :INT8U ME909S_IPCLOSE(INT8U SocketID)             
* Description   :模块关闭所有IP链接
* Input         : Port:模块串口号
*                 SOCKID:1-5：5个链接；
						 6：关闭服务器侦听功能, 并关闭与服务器的连接
						 7：关闭无线网络连接。如果存在链接或服务器监听，也会被关闭。
* Return        : 
*******************************************************************************/
INT8U ME909S_IPCLOSE(INT8U SocketID)
{
	INT8U				*pRet=NULL;
	TCHAR				ATIPCLOSE[14]={0};
	
	if((SocketID<1)||(SocketID>7)) return 0;
	
	memcpy(ATIPCLOSE,(INT8U *)"AT^IPCLOSE=",11);   
	ATIPCLOSE[11]=SocketID+0x30;
	ATIPCLOSE[12]=0x0d;
	ATIPCLOSE[13]=0x0a;
	
	pRet=ME909SCommandP(ATIPCLOSE, "OK", 5);    								//较久 返回为：^IPCLOSE: 0,0,0,0,0 代表5个链接都关闭了，如果sockid=7，就是这个结果
	if(pRet) 	return 1;
	else return 0;	
}

/*******************************************************************************
* Function Name :INT8U ME909S_Trans_ON(void)                         
* Description   :模块设置并打开透明输出通道
* Input         :
*                 
* Return        : 1：打开透传模式	0：打开透传失败
*******************************************************************************/
INT8U ME909S_Trans_ON(void)
{
	INT8U *pRet=NULL;
	
	/*将透传模式的参数设置移到CFG函数里，只要不断电，设置一次即可*/
//	pRet=ME909SCommandP(AT_IPCFLCHECK, "OK", 2);								//查询模式参数设置结果	可省略
	pRet=ME909SCommandP(AT_IPENTRANS, "OK", 2);									//打开透传模式
	if(pRet) return 1;
	return 0;
}

/*******************************************************************************
* Function Name :INT8U ME909S_Trans_OFF(void)                     
* Description   :模块关闭透传
* Input         :
*                 
* Return        : 
*******************************************************************************/
INT8U ME909S_Trans_OFF(void)
{
	INT8U				*pRet=NULL;

	OSTimeDly(20);
	pRet=ME909SCommandP(AT_IPDISTRANS, "OK", 2);								//前后都需要延时900ms
	if(pRet) 	return 1;
	else return 0;
}

/*******************************************************************************
* Function Name :INT8U ME909S_Link(INT8U SocketID)                             
* Description   :模块进行网络注册
* Input         :
*                 
* Return        : 
*******************************************************************************/
INT8U ME909S_Link(INT8U SocketID)
{
	INT8U ST=0;
	INT8U retry=3;
	INT8U i=0;

	for(i=0;i<retry;i++)
	{	
//		ST=ME909S_IPCLOSE(1);
//		ST=ME909S_IPCLOSE(6);													//6：关闭服务器侦听功能, 并关闭与服务器的连接
		ST=ME909S_IPCLOSE(7);       											//7：关闭无线网络连接。如果存在链接或服务器监听，也会被关闭
		if(ST==0) continue ;
		ST=ME909S_IPINIT();														//模块进行TCP/IP初始化
		if(ST) ST=ME909S_IPOPEN(SocketID,IP_Config.IP_addr_1,IP_Config.PortNum_1,1000);
		if(ST) ST=ME909S_Trans_ON();
		if(ST)																	//成功
		{
			if(Fault_Manage.F_NETWORK || Net_Fault_Time)						//已过30分钟产生故障上报或未产生故障上报但曾经联不上网（这里不能单以Net_Fault_Time否有值为判定条件，当故障产生后系统复位，此值一定为0，会导致无法产生联网故障恢复）
			{
				Net_Fault_Time=0;												//清零	
				if(Fault_Manage.F_NETWORK)										//若产生过故障
					NW_Fault_Manage(NETWORK_F, NOFAULT_STA);					//联网故障恢复上报
			}
			return 1;	    												
		}
	}	
	if(!Net_Fault_Time) Net_Fault_Time=RtcGetTimeSecond();						//网络连接失败 获取当前的时间	单位：秒 
	return 0;                  													//三次尝试都失败了	
}

/*******************************************************************************
* Function Name : INT8U *ME909SCommandP(TCHAR *pAtCmd,TCHAR *pExpResponse,INT16U WaitTime)                                                    
* Description   : 以AT命令的形式操控A8500执行相应的动作，等待返回的结果是否是希望的值
* Input         : Port          : A8500模块所接的串口号
*                 pAtCmd   		: 指向一条AT命令的Buffer地址
*                 AtCmdLen      : AT命令的长度
*                 pExpResponse	: 期望中的返回内容，主调指定的返回内容，这个就是根据A8500模块的操作手册来指定的。
*                 ExpBkLen      : 期望返回内容的长度,主调指定的，而且主调也知道这个期望返回长度是多少
*                 WaitTime      : 等待返回内容的最大忍耐时间，单位：秒
*                 
* Return        : 形参返回：pExpResponse	: A8500模块返回来的内容
*                 显式返回：NULL			: 重发了三次都失败，返回空指针
*                          pRet			: 发送后，有收到GPRS模块的回复，返回携带有内容的指针。
*******************************************************************************/
INT8U *ME909SCommandP(TCHAR *pAtCmd,TCHAR *pExpResponse,INT16U WaitTime)
{
	INT8U				*pRet = NULL, Retry_Cnt = 0, Num = 0, ST = 0, WaitCnt = 0;         
	INT16U				i, RBLen = 0, AtCmdLen = 0, ExpBkLen = 0;

	AtCmdLen = strlen(pAtCmd);
	ExpBkLen = strlen(pExpResponse);
	for(Retry_Cnt=0;Retry_Cnt<3;Retry_Cnt++)  
	{		
		StopModeLock++;
//		Feed_Dog();
		OSTimeDly(2);                         						
		if(pAtCmd && AtCmdLen)													//发送内容非空判定
		{	
			ST=60;																//3秒超时
			while( BSP_UART_TxState(2) && (ST--) ) OSTimeDly(1);				//等待串口空闲
			BspUartWrite(2,(INT8U*)pAtCmd,AtCmdLen); 							//调试打印
			ST=60;																//3秒超时
			while(BSP_UART_TxState(2)&&(ST--))OSTimeDly(1); 					//等待打印完成

			BSP_UART_RxClear(ME909SPort);  										//清空
			ST=60;
			BspUartWrite(ME909SPort,(INT8U*)pAtCmd,AtCmdLen);  					//这个才是真正的发送AT指令	
			while(BSP_UART_TxState(ME909SPort)&&(ST--))OSTimeDly(1);			//等待串口3发送完成
		}

		for(WaitCnt=0;WaitCnt<WaitTime;WaitCnt++)  
		{
			pRet = GPRS_WaitSign(20,&Num,&RBLen); 								//每次等待1秒，最多WaitTime次
			if(StopModeLock) StopModeLock--;
			if(pRet && pExpResponse && ExpBkLen )
			{		
				if((RBLen >= ExpBkLen))
				{
					for(i=0;i <= (RBLen - ExpBkLen);i++)
					{
						if (!memcmp(pExpResponse,pRet,ExpBkLen))				//去回复的内容里，找期待的回复
						{
							pRet += ExpBkLen;								
							return pRet;   										//成功，这里返回的是比对成功之后的后一位的地址
						}
						pRet++;
					}
					continue;  													//没有找到期望值，继续等待
				}
			}
			if( (ExpBkLen == 0) && (pRet) )
			{
				return pRet;
			}
		}	
		if(Retry_Cnt<2)
			OSTimeDly(20);
	}
	return NULL;  
}






/////以下整理自A8500.C==========================================================================================================================
/*******************************************************************************
* Function Name : INT8U* GPRS_WaitSign(INT16U WaitTime,INT8U *pDivNum,INT16U *pRxLen)                                                   
* Description   : 4G模块等待返回结果
* Input         : Port          : 4G模块所接的串口号
*                 WaitTime      : 循环等待扫描的次数，这个形参传进来之后，主要结合本函数内部的 OSTimeDly(1)使用，如果一直没有等到数据，
*                                 最终就是执行WaitTime次循环等待后就直接退出函数了。总等待时长 = 行WaitTime * OSTimeDly(1)
*                 pDivNum       : 主调开辟空间后，将地址传入本函数，由本函数填充一个值，看字面意思好像是一个设备号，什么的。
*                 pRxLen        : 主调开辟空间后，将地址传入本函数，由本函数填充一个GPRS模块接收到的数据的长度值。
*                 
* Return        : 形参返回：pDivNum ：设备号,“就只有一个主调函数 GPRS_RecvDataWait()中有使用了这个返回的设备号,其他的主调函数中并未使用，真相待查明”
*                           pRxLen ：GPRS模块接收到的数据长度 
*                 显式返回: pRet    : GPRS模块返回数据的首地址 
*                           NULL   : 形参WaitTime输入有误，或者等待超时无数据
*******************************************************************************/
INT8U* GPRS_WaitSign(INT16U WaitTime,INT8U *pDivNum,INT16U *pRxLen)
{
	INT8U Err=0;
	INT8U *pRet = NULL;   														//用于指向GPRS模块返回数据的首地址 
	INT16U RBLen = 0;
	INT8U ST=0;

	if( WaitTime == 0) return NULL; 											// 参数合法性检查
		
		StopModeLock++;
		pGprsMess = (struct Str_Msg *)OSMboxPend(GPRSBOX,WaitTime,&Err);		// 等待串口消息  
		if(StopModeLock) StopModeLock--;
		if((Err==OS_NO_ERR) && pGprsMess)										//有消息传入
		{
			if( pGprsMess->MsgID == BSP_MSGID_UART_RXOVER )						//是串口3的消息
			{
				RBLen= pGprsMess->DataLen;										//读取消息长度
				pRet=pGprsMess->pData;											//消息指针传入
				BSP_UART_RxClear(pGprsMess->DivNum);							//清除串口（清除指针，内容还在已传给pRet）
			}
			
		}
		else return NULL;
		
		if( pRet && RBLen )      												// 检查是否有东西
		{		
			if( RBLen > LTE_BUFF_LEN) BSP_UART_RxClear(ME909SPort);				// 接收到的内容超长度了。
			
			else
			{
				BSP_UART_RxClear(ME909SPort);
				*pRxLen=RBLen;
				*pDivNum = ME909SPort;  		
//				ST=ME9009S_RecvDataDealWith(pRet,RBLen); 						//透传模式下的数据接收处理用一个新的函数     2018.08.02  CW修改
		
				ST=60;
				while( BSP_UART_TxState(2) && (ST--) ) OSTimeDly(1);
				BspUartWrite(2,pRet,RBLen);    									// 将GPRS模块接收到的内容，向2号串口，调试打印。        //不管收到什么都打印出来
				ST=60;
				while(BSP_UART_TxState(2)&&(ST--))OSTimeDly(1); 				//等待打印完成
				
				return pRet;              
			}
		}
	return NULL;
}

/*******************************************************************************
名称：INT8U LTE_Get_Config(void)
功能：读取LTE参数：参数配置结构体&主站IP地址、端口号和卡号配置结构体
入参：无
出参：无
返回：0：失败；1：成功
*******************************************************************************/	
INT8U LTE_Get_Config(void)
{
	if(!BSP_ReadDataFromFm(Config_Addr,(u8*)&Config,Config_Len)) return 0;							//参数配置结构体Config
	if(!BSP_ReadDataFromFm(IP_Config_Addr,(u8*)&IP_Config,IP_Config_Len)) return 0;					//主站IP地址、端口号和卡号配置结构体IP_Config
	if(!BSP_ReadDataFromFm(APN_Addr, APN, APN_Len)) return 0;										//APN信息
	if(!BSP_ReadDataFromFm(Device_Number_Addr,Device_Number,Device_Number_Len)) return 0;			//装置号码Device_Number
	if(!BSP_ReadDataFromFm(FUN_Config_Addr,FUN_Config,FUN_Config_Len)) return 0;					//功能配置参数FUN_Config
	if(!BSP_ReadDataFromFm(Flow_Data_Addr,(u8*)&Local_FLow_Data,Flow_Data_Len)) return 0;			//开机读取流量统计信息
	return 1;		// 参数有效设置
}

/*========================================================以下为新写的驱动程序========================================================*/
/*******************************************************************************
* Function Name :INT16U LTE_WaitData(INT8U *OutBuff,INT16U timeout)       
* Description   :模块等待上位机返回数据，并将数据传出
* Input         :pOutBuff：指向输出数据地址  timeout：等到超时时间 （传入时间片 1=20ms）
*                 
* Return        :RBlen:返回数据长度    0:等待无返回  0XFFFF:断网标记
*******************************************************************************/
INT16U LTE_WaitData(INT8U *OutBuff,INT16U timeout)
{
    INT8U i=0,Err=0;
	INT16U RBLen =0;
	INT8U *pRB=NULL;
	INT8U ST=0;

	if( OutBuff == 0) return 0; 												// 参数合法性检查
	if( timeout == 0) return 0; 												// 参数合法性检查
		
	StopModeLock++;
	pGprsMess = (struct Str_Msg *)OSMboxPend(GPRSBOX,timeout,&Err);				//本质就是等待串口
	if(StopModeLock) StopModeLock--;
	
	if((Err==OS_NO_ERR) && pGprsMess)											//有消息传入
	{
		if( pGprsMess->MsgID == BSP_MSGID_UART_RXOVER )							//是串口3的消息
		{
			RBLen= pGprsMess->DataLen;											//读取消息长度
			pRB=pGprsMess->pData;												//消息指针传入			
			BSP_UART_RxClear(pGprsMess->DivNum);								//清除串口（清除指针，内容还在已传pRB）
		}		
	}	
	
	if( pRB && RBLen )      													//检查是否有东西，
	{	
		if(RBLen>LTE_BUFF_LEN)RBLen=LTE_BUFF_LEN;
		memcpy(OutBuff,pRB,RBLen);
		BspUartWrite(2,SIZE_OF("接收<<："));		
		ST=60;
		while( BSP_UART_TxState(2) && (ST--) ) OSTimeDly(1);
		if(update_start==false)													//进入升级过程时，不打印接收到的内容
		{
			BspUartWrite(2,pRB,RBLen);    										//将GPRS模块接收到的内容，向2号串口，调试打印。
			BspUartWrite(2,SIZE_OF("\r\n"));
		}
		ST=60;
		while(BSP_UART_TxState(2)&&(ST--))OSTimeDly(1); 						//等待打印完成
		
		/*进行断网判断*/
		for(i=0;i<30;i++)                 										//检查是否断网   ^IPSTATE主动上报网络状态，上报的时候就是断网了，只有网络站台改变时候才会收到此上报
		{
			if (!memcmp(pRB,SIZE_OF("^IPSTATE")))  
			{
				if(!Net_Fault_Time) Net_Fault_Time=RtcGetTimeSecond();			//网络连接失败 获取当前的时间	单位：秒 
				return 0xFFFF;	   
			}
			pRB++;
		}

		Update_Flow_DayAndMonth(RBLen);											//将下行流量统计进去
		BSP_UART_RxClear(3); 		
		return RBLen;															//获得有效数据	   		
	}
	return 0;													
}

/*******************************************************************************
* Function Name :INT16U LteCommunication(INT8U *pIn,INT16U InLen,INT8U *pOut,INT16U timeout)        
* Description   :模块发送数据并传出返回数据
* Input         :pIn：传入数据 InLen：传入数据长度      pOut：返回数据   timeout:等待回复的时间（传入时间片 1=20ms）
*                 
* Return        : 返回接收到的数据长度。不进行接收或没接收到时返回0
*******************************************************************************/
INT16U LteCommunication(INT8U *InBuff,INT16U InLen,INT8U *OutBuff,INT16U timeout)
{		
	INT16U RBLen = 0;      														//返回的数据长度  
	INT8U Count = 0;
	
	if((!InBuff)||(InLen==0))return 0;
	if(InLen>1472) return 0; 													//透传最大输入长度
	
	/*发送部分*/	
	if ((InBuff)&&(InLen))
	{	
		StopModeLock++;  
		Count=60;
		while( BSP_UART_TxState(2) && (Count--) ) OSTimeDly(1);
		BspUartWrite(2,SIZE_OF("发送>>："));
		BspUartWrite(2,InBuff,InLen); 
		BspUartWrite(2,SIZE_OF("\r\n"));
		Count=60;
		while(BSP_UART_TxState(2)&&(Count--)) OSTimeDly(1);

			
		BSP_UART_RxClear(3);  													// 清空 接收缓存，准备接收返回的数据
		Count=60;		
		BspUartWrite(3,InBuff,InLen);   										// 向4G模块写入AT指令或者输入数据	
		while(BSP_UART_TxState(3)&&(Count--)) OSTimeDly(1);
		if(StopModeLock) StopModeLock--;	
	}
	Update_Flow_DayAndMonth(InLen);												//将上行流量统计进去
	
	/*接收部分*/
	if( OutBuff == 0) return 0; 												//不进行接收
	RBLen=LTE_WaitData(OutBuff,timeout);										//里面已将下行流量统计进去
	
	/*上位机未回复故障判断部分*/
	if(!RBLen)																	//RBLen=0
	{
		if(!Host_No_Reply_Time)
			Host_No_Reply_Time=	RtcGetTimeSecond();								//主站无回复，记录未回复的时间,仅记录第一次
	}
	else if( RBLen!=0xFFFF && (Fault_Manage.F_REPLY ||Host_No_Reply_Time))		//主站正确回复且发生过联网故障  （这里不能单以Host_No_Reply_Time是否有值为判定条件，当故障已产生后系统复位，此值一定为0，会导致无法产生主站未回复故障恢复）
	{
		Host_No_Reply_Time=0;													//主站回复，清除主站未回复计时
		if(Fault_Manage.F_REPLY)												//故障已产生（30分钟未能联网成功）
			NW_Fault_Manage(REPLY_F, NOFAULT_STA);								//已产生主站未回复故障，上报故障恢复
		
	}
	return RBLen;
}

/*========================================================		短信驱动		========================================================*/
/*******************************************************************************
* Function Name :INT8U ME909S_SMS_CFG(void) 	                             
* Description   :模块进行短信功能配置 
* Input         :pIn:指向传入的短信内容，InLen：短信内容的长度
*                 
* Return        : 1：配置成功 0：配置出错
*******************************************************************************/
INT8U ME909S_SMS_CFG(void)
{
	INT8U				*pRet=NULL;
	INT8U				ST=0;

	pRet=ME909SCommandP(AT_CPMS_CHECK, "OK", 2);      	 	    				//查询短信存储设置  默认为SM,SM,SM
	if(pRet) 	ST=1;
	else ST=0;	
	
	if(ST)
	{
		pRet=ME909SCommandP(AT_CMGF_TEXT, "OK", 2);     						//设置短信模式为TEXT模式  掉电不保存 发送TEXT模式短信需每次都进行设置
		if(pRet) 	ST= 1;
		else ST=0;}		
	
	if(ST)
	{
		pRet=ME909SCommandP(AT_CSMP_GSM, "OK", 2);     							//设置文本短信的编码方式 GSM  
		if(pRet) 	ST= 1;
		else ST=0;}		
		
	if(ST)
	{
		pRet=ME909SCommandP(AT_CSCS_GSM, "OK", 2);     							//设置TE 字符集
		if(pRet) 	ST= 1;
		else ST=0;}	
	
	if(ST)
	{
		pRet=ME909SCommandP(AT_CNMI, "OK", 2);     								//设置短信模式为TEXT模式  掉电不保存 发送TEXT模式短信需每次都进行设置
		if(pRet) 	ST= 1;
		else ST=0;}		
		
//	if(ST)
//	{
//		pRet=ME909SCommandP(AT_CMGL, "OK", 2);        							// 读取短信   
//		if(pRet) 	ST=1;
//		else ST=0;
//	}
	
	/*
	这里可以加入短信读取出来并抛出去解析（上电读取，如果有需要的话）
	*/
	
	if(ST) 
	{
		pRet=ME909SCommandP(AT_CMGD_ALL, "OK", 2);        						// 删除全部短信	(开机清除杂乱的内容)
		if(pRet) 	return 1;}
	
	return 0;	
}
/*******************************************************************************
* Function Name :INT8U ME909S_SMS_Delete(INT8U Index,INT8U opt)	                             
* Description   :删除短信
* Input         :	Index:短信位置索引
					opt：传入删除短信选项  
						0 删除有 <index> 指定位置的短信
						1 删除首选存储器上所有的已读短信
						2 删除首选存储器上所有的已读短信和已发送短信
						3 删除首选存储器上所有的已读短信、已发送短信和未发送短信
						4 删除首选存储器上所有短信
*                 
* Return        : 1：成功 0：出错
*******************************************************************************/
INT8U ME909S_SMS_Delete(INT8U Index,INT8U opt)
{
	TCHAR  Buff[20]={0};
	
	sprintf(Buff, "AT+CMGD=%d,%d\r\n",Index,opt);
	
	if(ME909SCommandP(Buff, "OK", 5)) return 1;
	return 0;
}

/*******************************************************************************
* Function Name :INT8U ME909S_SMS_Send(INT8U *pIn, INT8U InLen,INT8U *DstPhoneNum)	                             
* Description   :模块发送短信（以TEXT模式）  
* Input         :pIn:指向传入的短信内容，InLen：短信内容的长度，DstPhoneNum：目标电话号码
*                 
* Return        : 1:发送成功 0：发送失败
*******************************************************************************/
INT8U ME909S_SMS_Send(INT8U *pIn, INT8U InLen,INT8U *DstPhoneNum)
{
	INT8U *pRet=NULL;
	INT8U ST=0;
	TCHAR ATCMGS[]="AT+CMGS=\"00000000000\"\r\n";
	TCHAR TEXT[200]={0};
	
	memcpy(&ATCMGS[9],DstPhoneNum,11);											//输入目标电话号码
	memcpy(TEXT,pIn,InLen); 													//输入内容
	TEXT[InLen]=0x1A;															//添加发送标识符
	InLen+=1;

	pRet=ME909SCommandP(AT_CMGF_TEXT, "OK", 2);     							//设置短信模式为TEXT模式  掉电不保存 发送TEXT模式短信需每次都进行设置
		if(pRet) 	ST= 1;
		else ST=0;
	
	if(ST)
	{
		pRet=ME909SCommandP(AT_CSMP_GSM, "OK", 2);     							//设置文本短信的编码方式 GSM  
		if(pRet) 	ST= 1;
		else ST=0;}	
	
	if(ST)
	{
	pRet=ME909SCommandP(ATCMGS, ">", 2);     			        				//发送TEXT短信
		if(pRet) 	ST= 1;
		else ST=0;}
	
	if(ST)
	{
		pRet=ME909SCommandP(TEXT, "OK", 2);     			           			//发送TEXT短信
		if(pRet) 	ST= 1;
		else ST=0;}		
		
	if(ST)	return 1;
		return 0;
}

/*******************************************************************************
* Function Name :INT16U ME909S_Read_SMS(INT8U *pOut)                         
* Description   :模块读取未读短信
* Input         :pOut:指向返回的短信存储位置
*                 
* Return        :短信内容长度   可能不止一条	0：
*******************************************************************************/
INT16U ME909S_Read_SMS(INT8U *pOut)
{
	INT8U 	ST=0;
	INT8U 	*pRet=NULL;
	INT16U 	SMSLen=0;
	
	pRet=ME909SCommandP(AT_CMGF_TEXT, "OK", 2);     							//设置短信模式为TEXT模式  掉电不保存 发送TEXT模式短信需每次都进行设置
		if(pRet) 	ST= 1;
		else ST=0;
	
	if(ST)
	{
		pRet=ME909SCommandP(AT_CSMP_GSM, "OK", 2);     							//设置文本短信的编码方式 GSM  
		if(pRet) 	ST= 1;
		else ST=0;
	}	
	
	if(ST) SMSLen=LteCommunication((INT8U*)AT_CMGL_UNREAD,sizeof(AT_CMGL_UNREAD)-1,pOut,100);		// 读取未读短信 
		
	if(SMSLen>10)																//如果无未读消息则会返回\r\nOK\r\n不超过10
	{
		pRet=ME909SCommandP(AT_CMGD_ALL, "OK", 2); 				
		if(pRet) 
		{
			if(SMSLen>200) SMSLen=200;
			return SMSLen;
		}
	}																			//如果有消息返回长度
	return 0;
																				
}
/*******************************************************************************
* Function Name :INT16U ME909S_Waitfor_SMS(u8 *pOut,u8 timeout_s)                         
* Description   :等待模块的短信到来通知，受到通知后读取短信，返回含模块固有信息的短信（带包装）
* Input         :pOut:指向返回的短信存储位置；u8 timeout，超时时间，单位为秒。
*                 
* Return        :0：超时；SMSLen：短信内容长度
*******************************************************************************/
INT16U ME909S_Waitfor_SMS(u8 *pOut,u8 timeout_s)
{
	INT8U 	i=0,Err=0;
	INT8U 	*pRet=NULL;
	INT16U 	SMSLen=0;
	
	/*LTE模块无法彻底关闭回复的特性导致不可用timeout进行长时间的延时，用循环分解下*/
	while(timeout_s--)															//timeout_s单位为秒
	{
		StopModeLock++;
		/*装置休眠状态，等待短信*/
		pGprsMess = (struct Str_Msg *)OSMboxPend(GPRSBOX,20,&Err);   			//等待串口消息1秒
		if(StopModeLock) StopModeLock--;
		if(Err==OS_NO_ERR && pGprsMess)											//如果收到短信通知，则读取短信
		{	
			if((pGprsMess->DivNum==3)&&(pGprsMess->DataLen>10))					//稍微过滤下错误串口错误讯息
			{	
				pRet=pGprsMess->pData;
				for(i=0;i<pGprsMess->DataLen;i++)
				{
					if(!memcmp(pRet,(INT8U*)"+CMTI:",6))						//如果是短信达到自动上报
					{	
						SMSLen=ME909S_Read_SMS(pOut);							//则读取未读短信，传出，并删除已读短信	
						BSP_UART_RxClear(pGprsMess->DivNum);					//清除串口指针位置
						return SMSLen;					
					}
					pRet++;
				}
			}
			BSP_UART_RxClear(pGprsMess->DivNum);								//清除串口指针位置
		}
	}
	return 0;
}
																				

/*******************************************************************************
* Function Name :INT8U *ME909S_SMS_Extract(u8 *pIn,INT8U InLen,INT8U *OutLen)                        
* Description   :从带模块格式的返回中，提取短信有效内容
* Input         :pIn:输入短信指针（含模块格式）  InLen：输入长度
*                OutLen：返回短信有效内容的长度
* Return        :短信内容指针
*******************************************************************************/
INT8U *ME909S_SMS_Extract(u8 *pIn,INT8U InLen,INT8U *OutLen)
{
	INT8U i=0;
	
	if(!InLen||!pIn) return NULL;
	
	for(i=0;i<InLen;i++)
	{
		if(!memcmp(pIn,(INT8U*)"\"\r\n",3))
		{
			InLen-=3;
			pIn+=3;
			OutLen[0]=InLen;
			return pIn;	
		}
		pIn++;
		InLen--;
	}	
	return NULL;
}



/*========================================================流量统计相关部分驱动========================================================*/

/*******************************************************************************
* Function Name :INT8U Update_Flow_DayAndMonth(INT32U In)   
* Description   :更新位于铁电中的日流量和月流量统计
* Input         :In：新增的的流量 byte
*                 
* Return        :U32数据（0：读取失败）
*******************************************************************************/
INT8U Update_Flow_DayAndMonth(INT32U In)
{
	Local_FLow_Data.Flow_Day_Used_B		+=	In;									//日流量
	Local_FLow_Data.Flow_Month_Used_B	+=	In;									//月流量

	if(BSP_WriteDataToFm(Flow_Data_Addr,(u8*)&Local_FLow_Data,Flow_Data_Len))	//写入铁电
		return 1;
	return 0;
}

/*******************************************************************************
* Function Name :void ClearFlowDataDailyAndMonthly(struct BSPRTC_TIME *pTime)	 
* Description   :每天零点清除日流量统计，每月初清除月流量统计。
* Input         :*ptime，年月日时分秒            
* Return        :无
*******************************************************************************/
void ClearFlowDataDailyAndMonthly(struct BSPRTC_TIME *pTime)										
{	
	if(Local_FLow_Data.Date != pTime->Day)										//日期变更时
	{
		Local_FLow_Data.Flow_Day_Used_B=0;										//日流量清零
		Local_FLow_Data.Date = pTime->Day;
		if(Local_FLow_Data.Month != pTime->Month)								//月份变更时
		{
			Local_FLow_Data.Flow_Month_Used_B=0;								//月流量清零
			Local_FLow_Data.Month = pTime->Month;
		}
	}	
	else return ;																//月、日未发生改变						
	BSP_WriteDataToFm(Flow_Data_Addr,(u8*)&Local_FLow_Data,Flow_Data_Len);		//清零后写入铁电
}

/*******************************************************************
函数名：INT8U *ScanAsicc(INT8U *InR,INT8U *Asicc,INT8U Asicclen)
功能：  搜索指定的字符串，并指向字符串后一个
*********************************************************************/
INT8U *ScanAsicc(INT8U *InR,INT16U inRlen,INT8U *Asicc,INT8U Asicclen)
{
	INT16U i = 0;	
	INT8U *pRet = InR;
	
	for(i = 0; i < inRlen-Asicclen; i++)
	{
		if (!(memcmp(pRet,Asicc,Asicclen)))
		{			 	 
			return pRet + Asicclen;//开机成功
		}
		pRet++;
	}
	return 0;
}

/*******************************************************************************
* Function Name : INT8U INT8UBCDToAscii(INT8U InData,INT8U *pOut)
* Description   : INT8U的BCD码格式数据转换到Ascii码（这个函数只输出A~F,大写格式的Ascii字符）
*
* Input         : InData : 被转换的BCD格式的数据
*
* Return        : 形参返回 ：pOut : 转换后的Ascii码格式数据
*                 显式返回 ：2 ：2位宽度
*******************************************************************************/
INT8U INT8UBCDToAscii(INT8U InData,INT8U *pOut)
{
	INT8U Temp = 0; 
	INT8U Temp1 = InData; 
	
	Temp = (Temp1>>4&0x0f);
	if(Temp <= 9) pOut[0] = Temp + 0x30;
	else
	{	
		pOut[0] = (Temp - 10) + 'A';
	}
	Temp = (Temp1&0x0f);
	if(Temp <= 9) pOut[1] = Temp + 0x30;
	else
	{	
		pOut[1] = (Temp -10) + 'A';
	}
	return 2;
}

/*******************************************************************************
* Function Name : INT8U INT16UHexToAscii(INT16U InData,INT8U *pOut)
* Description   : INT16U的Hex格式数据转换到Ascii
*
* Input         : InData : 被转换的字符(ASCII字符)
*
* Return        : 形参返回 ：pOut : 转换后的Ascii码串
*                 显式返回 ：5 ：5位宽度
*                           4 ：4位宽度
*                           3 ：3位宽度
*                           2 ：2位宽度
*                           1 ：1位宽度
*******************************************************************************/
INT8U INT16UHexToAscii(INT16U InData,INT8U *pOut)
{
	INT8U Temp[5] = {0};
	INT8U gTemp = 0;
	INT8U i = 0;
	
	for(i = 4; i != 0xff; i--)
	{
		gTemp = InData%10;     //个位
		Temp[i] =	gTemp;
		InData -= gTemp;
		InData = InData/10;        //去掉个位
	}
	
	if(Temp[0])  
	{
		pOut[4] = Temp[4] + 0x30;
		pOut[3] = Temp[3] + 0x30;
		pOut[2] = Temp[2] + 0x30;
		pOut[1] = Temp[1] + 0x30;
		pOut[0] = Temp[0] + 0x30;
		return 5;
	}
	
	if(Temp[1])
	{				 
		pOut[3] = Temp[4] + 0x30;
		pOut[2] = Temp[3] + 0x30;
		pOut[1] = Temp[2] + 0x30;
		pOut[0] = Temp[1] + 0x30;
		return 4;
	}
	
	if(Temp[2])
	{				 
		pOut[2] = Temp[4] + 0x30;
		pOut[1] = Temp[3] + 0x30;
		pOut[0] = Temp[2] + 0x30;
		return 3;
	}
	if(Temp[3])
	{				 
		pOut[1] = Temp[4] + 0x30;
		pOut[0] = Temp[3] + 0x30;
		return 2;
	}
			 
	pOut[0] = Temp[4] + 0x30;

	return 1;
}

/*******************************************************************************
* Function Name : void INT8UHexToAscii(INT8U InData,TCHAR *pOut)
* Description   : INT8U的hex数字转换到Ascii码
*
* Input         : InData : 被转化的数字
*
* Return        : 形参返回 ：pOut : 转换后的Ascii码格式数据
*******************************************************************************/
void INT8UHexToAscii(INT8U InData,TCHAR *pOut)
{
	pOut[0]=InData/10+0x30;
	pOut[1]=InData%10+0x30;
}
