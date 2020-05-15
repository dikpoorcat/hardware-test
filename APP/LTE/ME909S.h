#ifndef	__ME909S_H
#define __ME909S_H
#include "main.h"
/*控制引脚宏定义*/
////-----------------------------------短信及电话检测引脚------------------------------------------
//#define SMS_PIN         	GPIO_Pin_1
//#define SMS_Port       	GPIOB

//-----------------------------------电源电池控制引脚------------------------------------------				//2018.08.10 CW修改 增加一个电源电压控制引脚
#define BAT_CTL_PIN  		GPIO_Pin_11 													
#define BAT_CTL_Port 		GPIOC 
#define BAT_CTL_PIN_H()		GPIO_SetBits(BAT_CTL_Port,BAT_CTL_PIN);			//强制打开电池
#define BAT_CTL_PIN_L()		GPIO_ResetBits(BAT_CTL_Port,BAT_CTL_PIN);

//-----------------------------------GPRS电源控制引脚------------------------------------------
#define PWGPRS_PIN			GPIO_Pin_10										//全网通基站主板
#define PWGPRS_Port			GPIOC 
#define PWGPRSEN()			GPIO_SetBits(PWGPRS_Port,PWGPRS_PIN);
#define PWGPRSDIS()			GPIO_ResetBits(PWGPRS_Port,PWGPRS_PIN);


#define ME909S_REST_PIN		GPIO_Pin_12
#define ME909S_REST_PORT 	GPIOB
#define ME909S_REST_H()   	GPIO_ResetBits(ME909S_REST_PORT, ME909S_REST_PIN)				//这里也要反着来  2018.7.18 CW 修改
#define ME909S_REST_L()   	GPIO_SetBits(ME909S_REST_PORT, ME909S_REST_PIN)

#define ME909S_ONOFF_PIN    GPIO_Pin_13  
#define ME909S_ONOFF_PORT   GPIOB        
#define ME909S_ONOFF_H()   	GPIO_ResetBits(ME909S_ONOFF_PORT, ME909S_ONOFF_PIN)             //这里是反着来的
#define ME909S_ONOFF_L()   	GPIO_SetBits(ME909S_ONOFF_PORT, ME909S_ONOFF_PIN)

#define ME909S_PW_PIN       GPIO_Pin_10       
#define ME909S_PW_Port      GPIOC      
#define ME909S_PW_ON()      GPIO_SetBits(ME909S_PW_Port,ME909S_PW_PIN);
#define ME909S_PW_OFF()     GPIO_ResetBits(ME909S_PW_Port,ME909S_PW_PIN);




/*常量宏定义*/
#define ME909SBaudrate		9600
#define ME909SPort			3 //4G模块使用串口3

#define GPRSRXDATA			4 //接收到数据状态 GPRS 接收到数据
#define GPRSTXDATA   		5 //GPRS 发送数据

#define GPRSDATA     		6 //数据处理中
#define GPRSWAITDATA     	7 //




/*AT指令宏定义*/
#define AT_CNUM				"AT+CNUM\r\n"
#define AT_ICCID			"AT^ICCID?\r\n"
#define AT_IMSI				"AT+CIMI\r\n"
#define AT_AT				"AT\r\n"          			   						//4   尝试通讯
#define AT_ATE				"ATE0\r\n"		    								//6   关闭指令回显
#define AT_MSO				"AT^MSO\r\n"	          			    			//8   关机(不等待)，关闭模块电源，此时就算29302的电源还开着，但模块的1.8V没有输出了，说明电源不再输入到模块
#define AT_IPRCHECK			"AT+IPR?\r\n"   									//9   查询波特率
#define AT_IPR9600			"AT+IPR=9600\r\n"    								//13  固定9600波特率
#define AT_IPR115200		"AT+IPR=115200\r\n"    								//15  固定115200波特率
#define AT_HCSQ				"AT^HCSQ?\r\n"         								//10  查询信号强度
#define AT_CURC				"AT^CURC=0\r\n"    									//11  关闭自动上报^RSSI,^MODE,等一些乱七八糟的东西
#define AT_IPCFL5			"AT^IPCFL=5,2\r\n"        							//14  设置透传超时发送定时器长度0.1s*2
#define AT_IPCFL10			"AT^IPCFL=10,1472\r\n"  							//18  设置透传满buffer长度，最大长度1472
#define AT_IPCFL12			"AT^IPCFL=12,0\r\n"     							//15  设置透传模式0：即普通的满buffer模式。满buffer自动发送，未满等待超时发送，关闭透传直接发送
#define AT_ANT				"AT^ANTMODE=0,0\r\n"								//16  设置天线模式 参数10:全开，1：主集，2：分集 参数2：0：全制式，1：保留，2：WCDMA，3：LTE  掉电保存
#define AT_ANTCHECK			"AT^ANTMODE?\r\n"									//13  查询天线模式
#define AT_LED				"AT^LEDCTRL=1\r\n"	        						//14  设置LED闪灯模式，默认，未注册网络2s内快闪两次，已注册网络2s内快闪1次，已拨号连接常亮  此设置掉电保存
#define AT_LEDCHECK			"AT^LEDCTRL?\r\n"	    							//13  查询LED模式
#define AT_CMEE				"AT+CMEE=2\r\n"    									//11  设置报错直接输出字符串代替默认的输出错误代码，谁有空天天去查手册
/*以下3行用于设置网络制式选择，默认自动模式*/
#define AT_FREQLOCKCHECK	"AT^FREQLOCK?\r\n"									//锁频查询
#define AT_SYSCFGEX			"AT^SYSCFGEX=\"0102\",3FFFFFFF,1,2,7FFFFFFFFFFFFFFF,,\r\n"	//14  配置扩展系统  010203:GSM->UMTS->LTE   00:自动模式  掉电保存
#define AT_SYSCFGEXCHECK	"AT^SYSCFGEX?\r\n"									//14  扩展系统配置查询
#define AT_CPIN				"AT+CPIN?\r\n"		          						//10   查询SIM卡是否插入
#define AT_COPS				"AT+COPS=0\r\n"		          						//11   设置运营商选择模式为自动选择	
#define AT_COPSCHECK		"AT+COPS?\r\n"		          						//10   查询运营商	
#define AT_CREG				"AT+CREG=1\r\n"										//11   注册GSM状态自动上报	
#define AT_CGREG			"AT+CGREG=1\r\n"		         				 	//12   注册GPRS状态自动上报	
#define AT_CREGCHECK		"AT+CREG?\r\n"		          						//10   查询当前注册GSM状态	
#define AT_CGREGCHECK		"AT+CGREG?\r\n"		          						//11   查询当前注册GPRS状态
#define AT_SYSINFO			"AT^SYSINFOEX\r\n"									//14   查询扩展系统信息
#define AT_CSQ				"AT+CSQ\r\n"		          			 			//8    查询信号强度
#define AT_CFUN				"AT+CFUN=1,0\r\n"		          					//13   设置模块为online mode，且不重启模块
#define AT_CFUNCHECK		"AT+CFUN?\r\n"		          						//10   查询模块模式
#define AT_IPCFLCHECK		"AT^IPCFL?\r\n"			    						//11   查询TCP/UDP 静态参数设置结果
#define AT_IPINITCHECK		"AT^IPINIT?\r\n"									//12   查询TCP初始化结果
#define AT_OPENCHECK		"AT^IPOPEN?\r\n"									//12   查询已打开的IP
#define AT_IPENTRANS		"AT^IPENTRANS=1\r\n"								//16 打开透明传输，仅有一个链接的时候方可使用，多个不行
#define AT_IPENTRANSCHEC	"AT^IPENTRANS?\r\n"									//15 查询透传是否打开
#define AT_IPDISTRANS		"+++"												//3  退出透传指令，命令用于从透传模式切换到命令模式  这个指令不能加\r\n
#define AT_CPMS_CHECK		"AT+CPMS=?\r\n"										//查询短消息存储位置
#define AT_CMGD_ALL			"AT+CMGD=1,4\r\n"									//删除首选存储器上所有短信
#define AT_CMGF_TEXT		"AT+CMGF=1\r\n"										//短信TEXT模式
#define AT_CMGL				"AT+CMGL=\"ALL\"\r\n"								//读取所有接收到的短信
#define AT_CNMI				"AT+CNMI=2,1,0,0,0\r\n"								//设置短信接收模式Stores the message on the SIM card or ME, and presents the new message indication.
#define AT_CSCS_GSM			"AT+CSCS=\"GSM\"\r\n"								//设置TE字符集为GSM 7bit	
#define AT_CSMP_GSM			"AT+CSMP=,,,0\r\n"									//设置TEXT模式下的参数,这里设置数据编码格式为GSM 7bit
#define AT_CPMS				"AT+CPMS=\"SM\",\"SM\",\"SM\" \r\n"		    		//设置短消息读取和删除介质/写入和发送介质/接受介质		仅支持（U）SIM卡
#define AT_CMGF_PD			"AT+CMGF=0\r\n"										//设置短消息格式为PDU，TEXT格式则改=1   掉电不保存，缺省为PDU
#define AT_CMGF_TEXT		"AT+CMGF=1\r\n"										//设置短消息格式为PDU，TEXT格式则改=1   掉电不保存，缺省为PDU
#define AT_CSMP_GSM			"AT+CSMP=,,,0\r\n"									//设置TEXT模式下的参数,这里设置数据编码格式为GSM 7bit
#define AT_CMGF_TEXT		"AT+CMGF=1\r\n"										//短信TEXT模式
#define AT_CMGL_UNREAD		"AT+CMGL=\"REC UNREAD\"\r\n"						//读取收到的未读短信
#define AT_CMGD_ALL			"AT+CMGD=1,4\r\n"									//删除首选存储器上所有短信
#define AT_CSMP_GSM			"AT+CSMP=,,,0\r\n"									//设置TEXT模式下的参数,这里设置数据编码格式为GSM 7bit




/*结构体定义*/
typedef struct GPRS_CanHX_
{
	INT8U  YanShiOFFTime;  // 延时关闭IP连接时间
	INT8U  StartHour;
	INT8U  StartMin;
	INT8U  EndHour;
	INT8U  EndMin;	  	
	INT16U WakeUp_cycle;   // 苏醒周期   单位S
	INT16U Wakeing_Time;  // 苏醒的时间 单位S
}GPHX_;

typedef struct SMS_Struct
{
	INT8U SaveIndex;    // 保存的位置
	INT8U Stat;         // 0：未读，1：已读 ，2：存储未发，3： 存储已发
	
	INT8U DataLen;      // 短消息的长度 不包含 中心号码
	INT8U CSALen;       // 中心号码的长度
	INT8U DSCType;      // 编码类型

	INT8U OaLen   ;     // 信息来源地址的长度
	
	INT8U CSTSTime[7];  // 短信接收时间，如果超过2天，短信将无效
	
	INT8U OaAddr[32];   // 短信来源地址
	
	INT8U SMSDATALen;   // 短信息长度
	
	INT8U SMSDATA[140]; // 短信息内容
}SMS_S;

typedef struct gprs_str
{
	INT8U  SOCKID;         // 套接字
	
	INT16U RecvLen;        // 就收到的长度
	INT16U Packet_length;  // 报文的长度
	
	INT8U  Frame_type;     // 帧类型 
	INT8U  Packet_Type;    // 报文类型
	
	INT8U *RecvDataP;      // 数据存放指针
}GPRS_STRUCT;

typedef struct A58_Struct														//结构体名字改一下
{
		INT8U A58stats;    //5800  工作状态
		INT8U A58BKPstats; //5800  备份
		
}A58Control;



/*全局变量声明*/





/*函数声明*/
void ME909S_LowPower(void);
void ME909S_PinInit(void);
void ME909SInit(INT32U rate);																	//MES909S 串口配置，电源及控制引脚初始化
INT8U ME909S_ON(void);																			//MES909S 模块的开机  
void ME909S_OFF(void)  ;    																	//MES909S 模块的关机  且关闭模块电源
void DeviceRstOnCMD(void);																		//接到装置重启命令后，MES909S 模块的关机  且关闭模块电源 再进行单片机复位
INT8U ME909S_RateChange(void);																	//修改波特率为9600 暂时先不搞波特率参数了，固定修改为某值，不然还有32位hex转ascii的问题，这个函数应该很少需要用到
INT8U ME909S_CONFIG(void);																		//模块开机后进行本机配置
INT8U ME909S_REG(void);																			//模块进行网络注册
INT8U ME909S_IPINIT(void);																		//模块进行TCP/IP初始化		UDP/TCP连接,可增加参数进行选择
INT8U ME909S_IPOPEN(INT8U SocketID,INT8U *IP,INT8U *Remote_Port,INT16U Local_Port)  ;			//模块使用一个socket打开一个IP链接
INT8U ME909S_IPCLOSE(INT8U SocketID);															//模块关闭IP链接
INT8U ME909S_Trans_ON(void)	;																	//模块设置并打开透明输出通道
INT8U ME909S_Trans_OFF(void);																	//模块关闭透传
INT8U ME909S_Link(INT8U SocketID)	;															//模块进行IP连接
INT8U ME9009S_RecvDataDealWith(INT8U *pIn,INT16U InLen)     ;									//模块在透传模式下处理 模块返回的数据 
INT8U ME909S_TransSendData(INT8U *pIn,INT16U InLen,INT8U SendPacketType);						//模块通过透传模式传输数据
INT8U ME909S_RecvDataWait(INT16U Time)	;														//GPRS模块处理接收到的数据
INT8U ME909S_XTSend(void);																		//模块发送心跳
INT8U ME909S_WDSend(INT8U *pIn,INT16U InLen);													//模块发送温度数据，每条尝试3次
INT8U ME909S_Get_Signal_Strength(void);															//获得模块的信号强度百分比
INT8U HB_Get_Signal_Strength(void);																//为心跳组帧获取一个无线信号强度百分比(需关闭透传，输入查询指令，再打开透传)
INT8U *ME909SCommandP(TCHAR *pAtCmd,TCHAR *pExpResponse,INT16U WaitTime);						//向模块发送AT指令，等待返回的结果是否是希望的值

/*部分整理自A8500.C======================================================================================================================*/
INT8U* GPRS_WaitSign(INT16U WaitTime,INT8U *pDivNum,INT16U *pRxLen);							//GPRS模块等待返回结果 
INT8U Read_GPRS_RxDataFromGPFM(void)  ;															// 处理接收到的数据	
INT8U LTE_Get_Config(void);																		//获取GPRS参数

/*========================================================以下为新写的驱动程序	========================================================*/
INT16U LTE_WaitData(INT8U *OutBuff,INT16U timeout);												//模块等待上位机返回数据，并将数据传出
INT16U LteCommunication(INT8U *InBuff,INT16U InLen,INT8U *OutBuff,INT16U timeout);				//模块发送数据并传出返回数据

/*========================================================		短信驱动		========================================================*/
INT8U ME909S_SMS_CFG(void);																		//模块进行短信功能配置
INT8U ME909S_SMS_Send(INT8U *pIn, INT8U InLen,INT8U *DstPhoneNum);								//模块发送短信（以TEXT模式） 
INT16U ME909S_Read_SMS(INT8U *pOut);															//模块读取未读短信
INT16U ME909S_Waitfor_SMS(u8 *pOut,u8 timeout_s);												//模块等待短信
INT8U ME909S_SMS_Delete(INT8U Index,INT8U opt);													//模块删除短信
INT8U *ME909S_SMS_Extract(u8 *pIn,INT8U InLen,INT8U *OutLen);									//模块短信提取

/*========================================================流量统计相关部分驱动	========================================================*/
INT8U Update_Flow_DayAndMonth(INT32U In);														//更新位于铁电中的日流量和月流量统计
void ClearFlowDataDailyAndMonthly(struct BSPRTC_TIME *pTime);									//每天零点清除日流量统计，每月初清除月流量统计	


INT8U *ScanAsicc(INT8U *InR,INT16U inRlen,INT8U *Asicc,INT8U Asicclen);
INT8U INT8UBCDToAscii(INT8U InData,INT8U *pOut);
INT8U INT16UHexToAscii(INT16U InData,INT8U *pOut);
void INT8UHexToAscii(INT8U InData,TCHAR *pOut);
#endif 
