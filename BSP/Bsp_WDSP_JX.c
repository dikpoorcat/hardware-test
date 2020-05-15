#include "Bsp_WDSP_JX.h"

//变量定义
static INT8U	FS_CMD[8]={0x01,0x03,0x00,0x00,0x00,0x01,0x84,0x0A};//风速协议
static INT8U	FX_CMD[8]={0x02,0x03,0x00,0x00,0x00,0x02,0xC4,0x38};//风向协议





/******************************************************************************* 
* Function Name  : void WDSP_LowPower(void)
* Description    : WDSP进入低功耗，并对相应IO口作低功耗处理。关闭U2时钟及复用时钟。
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/
void WDSP_LowPower(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	B485_LowPower();														//485低功耗

	/*WDSP电源使能推挽输出*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;						//推挽输出
	PWWDSPDIS();															//关电源
	GPIO_InitStructure.GPIO_Pin = PWWDSP_PIN;								//WDSP电源使能口
	GPIO_Init(PWWDSP_Port, &GPIO_InitStructure);							//
}

/******************************************************************************
* Function Name: void PowerWDSPPin_Init(void)
* Description:   风速风向电源控制 引脚配置 
* Input:  Nothing
* Output: Nothing
* Contributor: GuoWei Dong
* Date First Issued: 2010-01-26
******************************************************************************/
void PowerWDSPPin_Init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(PWWDSP_Port_CK,ENABLE);			//开时钟
	PWWDSPDIS();											//设置为关闭
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =PWWDSP_PIN;
	GPIO_Init(PWWDSP_Port, &GPIO_InitStructure);
}

/***************************************************************************************
名称：INT8U Get_WDSP_Data( INT16U OutBuff_WindSpeed, INT16U OutBuff_WindDirection )
功能：风速风向数据采集
入参：无
出参：INT16U OutBuff_WindSpeed, 风速值；INT8U OutBuff_WindDirection，风向值
返回：bit3风速传感器错误，bit4风向传感器错误，返回0采集正确
****************************************************************************************/
INT8U Get_WDSP_Data( INT16U *OutBuff_WindSpeed, INT16U *OutBuff_WindDirection )
{
	INT8U 	err=0;
	
	WindSpeed=0;									//清0全局变量
	WindDirection=0;								//清0全局变量
	
	Power485Pin_Init();								//重新配置485电源控制引脚
	PWWDSPEN();										//开启风速风向传感器电源	
	PWDC485EN();									//DCDC隔离485打开	
	OSTimeDly(60);									//延时3S读取数据（实测2.5秒以上才能读出风速，给裕量到3秒）
	B485_init(4800);								//重新初始化，防止其他任务关串口导致失效。传感器默认波特率为4800

	BSP_UART_Write(2,FS_CMD,8);						//发送风速命令
	if(B485WaitData(1)==1)							//等邮箱1s，并对485接收到的数据进行判断并处理，成功返回1
	{
		*OutBuff_WindSpeed=WindSpeed;				//当前风速（UINT16，10倍风速）
	}else err= 0x04;								//标志位bit3，记录风速传感器错误
	
	BSP_UART_Write(2,FX_CMD,8);						//发送风向命令
	if(B485WaitData(1)==1)							//等邮箱1s，并对485接收到的数据进行判断并处理，成功返回1
	{
		*OutBuff_WindDirection=WindDirection;		//记录当前风向（INT8U，0~7或0~360度，已为后台要求的数据类型）
	}else err= 0x08;								//标志位bit4，记录风向传感器错误
	
	if( err&0x04 )									//风速传感器错误
	{
		BSP_UART_Write(2,(INT8U *)("\r\n——————————本次采集风速失败！——————————\r\n"),18+44);		//485信息打印
		OSTimeDly(10);
	}else if( err&0x08 )							//风向传感器错误
	{
		BSP_UART_Write(2,(INT8U *)("\r\n——————————本次采集风向失败！——————————\r\n"),18+44);		//485信息打印
		OSTimeDly(10);
	}
	WDSP_LowPower();								//进入低功耗
	return err;										//正确时返回0
}

/*********************************************************************************************
函数：INT8U IsProtocol_WDSP( INT8U *In, INT8U Len, INT16U Out_WindSpeed, INT16U Out_WindDirection )
说明：机械式风速方向传感器接收数据协议处理
入参：In：输入风速风向数据    Len：数据长度  	Out_WindSpeed：存放风速值	Out_WindDirection：存放风向值
出参：1:符合协议   0：不符合协议
*******************************************************************************************/
INT8U IsProtocol_WDSP( INT8U *In, INT8U Len, INT16U *Out_WindSpeed, INT16U *Out_WindDirection )
{
	INT8U CRCH=0,CRCL=0;
	if(In[0]==0x01)											//风速数据
	{
		if(In[1]!=0x03)return 0;
		if(In[2]!=0x02)return 0;
		CRC16_Modbus( In, 5, &CRCL, &CRCH );				//计算CRC校验
		if( (In[5]!=CRCL) && (In[6]!=CRCH) )return 0;		//注意此处高低字节位置，小端模式低字节在低地址
		*Out_WindSpeed=(In[3]<<8)+In[4];					//CRC校验通过，计算风速值（10倍）
		return 1;
	}
	if(In[0]==0x02)											//风向数据
	{
		if(In[1]!=0x03)return 0;
		if(In[2]!=0x04)return 0;
		CRC16_Modbus( In, 7, &CRCL, &CRCH );				//计算CRC校验
		if( (In[7]!=CRCL) && (In[8]!=CRCH) )return 0;		//注意此处高低字节位置
		*Out_WindDirection=(In[3]<<8)+In[4];				//CRC校验通过，计算风向值（0~7）
//		*Out_WindDirection=(In[5]<<8)+In[6];				//CRC校验通过，计算风向值（0~360度）
		return 1;
	}
	return 0;
}

/*********************************************************************************************
函数名：void CRC16_Modbus(INT8U *P,INT16U len,INT8U *LByte,INT8U *HByte)
说明：机械式风速方向传感器数据校验	（CRC-16 (Modbus)）
入参：P：校验数据    len：校验数据的长度   
出参： LByte：校验结果低字节    HByte：校验结果高字节
*******************************************************************************************/
void CRC16_Modbus(INT8U *P,INT16U len,INT8U *LByte,INT8U *HByte)
{
	INT8U CRC16Lo,CRC16Hi;
	INT8U CL,CH;
	INT8U SaveHi,SaveLo;
	INT16U i,j;
	CRC16Lo=0xff;
	CRC16Hi=0xff;
	CL=0X01;
	CH=0xA0;//多项式码0xA001
	for(i=0;i<len;i++)
	{
		CRC16Lo=CRC16Lo^(P[i]);//每一个数据与CRC寄存器进行异或
		for(j=0;j<8;j++)
		{
			SaveHi=CRC16Hi;
			SaveLo=CRC16Lo;
			CRC16Hi=(CRC16Hi>>1);//高位右移一位
			CRC16Lo=(CRC16Lo>>1);//低位右移一位
			if((SaveHi&0x01)==0x01)//如果高位字节最后一位为1
			{
					CRC16Lo=CRC16Lo | 0x80;//则低位字节右移后前面补1，否则自动补0
			}
			if((SaveLo&0x01)==0x01)//如果LSB为1，则与多项式码进行异或
			{
					CRC16Hi=CRC16Hi^CH;
				  CRC16Lo=CRC16Lo^CL;		
		  }
	  }
	}
	*HByte = CRC16Hi;
	*LByte = CRC16Lo;
}
