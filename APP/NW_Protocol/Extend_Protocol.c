/***************************** (C) COPYRIGHT 2019 方诚电力 *****************************
* File Name          : Extend_Protocol.c
* Author             : 杜晓瞻、杜颖成、等
* Version            : 见历史版本信息
* Date               : 2020/04/13
* Description        : 根据南网协议编写的通信功能函数。
************************************  历史版本信息  ************************************
* 2019/03/28    : V4.1.0
* Description   : 南网测温项目初版。基础功能完成，调试中。
*******************************************************************************/

#include "Extend_Protocol.h"

INT32U				bin_file_adress = 0x08027000;								//缺省值0x08027000
struct	FILE_UPDATA	file_update = {0};
INT8U				subbag_statistics[STA_NUM] = {0}; 							//每一位代表一包，例：{0xE3,0xFF...}即1110 0011 1111 1111...，表示第0、1、2、6、7、8...包接收成功，第3、4、5包接收失败
bool				update_start = false;										//下发升级请求后将该标志位1才会进入后续升级过程
INT8U				device_status = 0;											//设备状态：为FFH时，表示允许升级；00H表示电池电压不足，01H表示存储空间不足，02H表示当前有未上报的历史数据，03H表示装置存在故障。
INT8U				upgrade_timeout = 0;										//用于DevStatCtr()函数轮询，当远程升级超时，退出升级模式
INT32U				sys2_upgrade_time = 0;										//系统2升级时间，世纪秒表示







/*******************************************************************************
名称：INT8U ExtendOnOffComm(u8 *InBuff, u16 Len)
功能：包括密码检验、回复通信、扩展协议使能控制功能执行，控制字：F0H
入参：u8 *InBuff，传入的内容，正常为接收到的帧；u16 Len，长度
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U ExtendOnOffComm(u8 *InBuff, u16 Len)
{
	if(!PassworkCheckAndReport(InBuff));										//判断主站下发报文中密码是否正确，不正确时上报密码出错信息。
	else																		//若相同，密码正确
	{
		if(InBuff[14]==0xFF)													//启用扩展协议
		{
			/*先不做*/
		}
		else																	//禁用扩展协议
		{
			/*先不做*/
		}
		LteCommunication(InBuff,Len,0,0);										//配置成功，按照原命令返回，不接收（LteCommunication返回0）
		return 1;
	}
	BspUartWrite(2,SIZE_OF("ExtendOnOffComm扩展协议使能控制通信失败！\r\n"));
	return 0;
}

/*******************************************************************************
名称：INT8U SetApnComm(u8 *InBuff, u16 Len)
功能：包括密码检验、回复通信、APN参数配置功能执行，控制字：F3H
入参：u8 *InBuff，传入的内容，正常为接收到的帧；u16 Len，长度
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U SetApnComm(u8 *InBuff, u16 Len)
{
	if(!PassworkCheckAndReport(InBuff));										//判断主站下发报文中密码是否正确，不正确时上报密码出错信息。
	else																		//若相同，密码正确
	{
		if(InBuff[14]!='\0')													//APN非空
		{
			strncpy((char*)APN, (char*)InBuff+14, APN_Len);						//写入APN数组
			if(BSP_WriteDataToFm(APN_Addr, APN, APN_Len))						//写入铁电
			{
				LteCommunication(InBuff,Len,0,0);								//配置成功，按照原命令返回，不接收（LteCommunication返回0）
				return 1;	
			}		
		}
	}
	BspUartWrite(2,SIZE_OF("SetApnComm设置APN通信失败！\r\n"));
	return 0;
}

/*******************************************************************************
名称：INT8U UpdataRequestComm(u8 *InBuff)
功能：主站下发升级请求，控制字：F7H
主站在收到扩展心跳5秒后，下发升级请求。装置接收到主站升级请求后，立刻进行升级检测并进行回复。
入参：u8 *InBuff，传入的内容，正常为接收到的帧
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U UpdataRequestComm(u8 *InBuff)
{
	INT16U	len_frame=0;														//帧长度
	
	if(!PassworkCheckAndReport(InBuff));										//判断主站下发报文中密码是否正确，不正确时上报密码出错信息。
	else																		//若相同，密码正确
	{
		BspUartWrite(2,SIZE_OF("升级请求，控制字：F7H\r\n"));
		memcpy(&file_update,InBuff+14,sizeof(file_update));						//将文件基本信息记录在结构体 file_update 中
		GetDeviceStatusBeforeUpgrade();											//获取设备状态
		if(device_status==0xFF) UpgradePreparation( InBuff ); 					//允许升级，标记为开始升级，并准备更新（回复前处理，以免丢第一包）

		len_frame = NW_Framing(EX_UPDATA_REQUEST,LTE_Tx_Buff);					//根据控制字组帧    
		LteCommunication(LTE_Tx_Buff,len_frame,0,0);							//回复升级检测结果
		
		if(device_status==0xFE)	RestartToUpgrade();								//升级条件已具备，装置需切换到稳定版本程序进行接收并升级（按协议要求先回复再重启）
		return 1;																//通信完成返回1，令NW_Comm_Process继续保持接收
	}
	BspUartWrite(2,SIZE_OF("UpdataRequestComm升级请求密码不正确！\r\n"));
	return 0;																	//重试超次数、等待超时都返回0
}

/*******************************************************************************
名称：INT8U UpdataDownloadComm(u8 *InBuff)
功能：主站升级包下发，控制字：F8H
主站下发升级包，升级包数据为二进制码流。数据包下发间隔为1秒。
入参：u8 *InBuff，传入的内容，正常为接收到的帧
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U UpdataDownloadComm(u8 *InBuff)
{
	if(!PassworkCheckAndReport(InBuff));										//判断主站下发报文中密码是否正确，不正确时上报密码出错信息。
	else																		//若相同，密码正确
	{
		if(update_start==true) 													//防止主站意外下发升级包，破坏FLASH。
		{
			BspUartWrite(2,SIZE_OF("升级包下发，控制字：F8H"));
			SubbagUpdateToFlash( InBuff );										//解析文件和保存（写入FLASH）。
		}
		else BspUartWrite(2,SIZE_OF("当前不在升级状态，升级包未写入FLASH！"));
		return 1;																//通信完成或不在升级状态时返回1，令NW_Comm_Process继续保持接
	}
	BspUartWrite(2,SIZE_OF("UpdataDownloadComm升级包下发密码不正确！\r\n"));
	return 0;																	//重试超次数、等待超时都返回0
}

/*******************************************************************************
名称：INT8U UpdataFinishComm(u8 *InBuff)
功能：主站升级包下发结束，控制字：F9H
全部升级包下发结束后2秒，主站发送该指令，装置收到后立即上传文件补包（FAH）。
入参：u8 *InBuff，传入的内容，正常为接收到的帧
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U UpdataFinishComm(u8 *InBuff)
{
	INT16U	len_frame=0;														//帧长度
	
	if(!PassworkCheckAndReport(InBuff));										//判断主站下发报文中密码是否正确，不正确时上报密码出错信息。
	else																		//若相同，密码正确
	{
		if(update_start==true)													//防止主站意外下发升级包，破坏FLASH																				
		{
			BspUartWrite(2,SIZE_OF("升级包下发结束，控制字：F9H\r\n"));
			len_frame=NW_Framing(EX_UPDATA_FILLING,LTE_Tx_Buff);				//将待补包信息与版本号组帧（控制字FA）
			LteCommunication(LTE_Tx_Buff,len_frame,0,0);						//上报，不等待回复
			if( len_frame==18 )													//即补包数为0（12+6），表示升级包已接收完全
				RestartAfterCrcCheckPassed();									//若CRC校验通过，标记备份寄存器，然后重启。
		}
		else BspUartWrite(2,SIZE_OF("当前不在升级状态，收到异常F9H指令！"));
		return 1;																//通信完成或不在升级状态时返回1，令NW_Comm_Process继续保持接收
	}
	BspUartWrite(2,SIZE_OF("UpdataFinishComm升级包下发结束密码不正确！\r\n"));
	return 0;																	//重试超次数、等待超时都返回0
}

/*******************************************************************************
名称：INT8U FormatFlashComm(u8 *InBuff, u16 Len)
功能：包括密码检验、回复通信、格式化FLASH功能执行，控制字：FBH
入参：u8 *InBuff，传入的内容，正常为接收到的帧；u16 Len，长度
出参：无
返回：成功返回1，失败返回0
*******************************************************************************/
INT8U FormatFlashComm(u8 *InBuff, u16 Len)
{
	FRESULT				res;													/* API result code */
	
	if(!PassworkCheckAndReport(InBuff));										//判断主站下发报文中密码是否正确，不正确时上报密码出错信息。
	else																		//若相同，密码正确
	{
		if(InBuff[14]==0xFF)													//强制格式化FLASH。目前只有此命令
		{
			/* Create FAT volume */
			res = f_mkfs(	"", 												// If it has no drive number in it, it means the default drive.
							FM_FAT|FM_SFD, 										// Specifies the format option in combination of FM_FAT, FM_FAT32, FM_EXFAT and bitwise-or of these three, FM_ANY. If two or more types are specified, one out of them will be selected depends on the volume size and au.
							4096, 												// The valid value is n times the sector size.
							work, 												// Pointer to the working buffer used for the format process
							sizeof work);										// It needs to be the sector size of the corresponding physical drive at least.
			if(res==FR_OK) 
			{
				LteCommunication(InBuff,Len,0,0);								//操作成功，按照原命令返回，不接收（LteCommunication返回0）
				BspUartWrite(2,SIZE_OF("f_mkfs() 格式化成功！3秒后重启！\r\n"));
				OSTimeDly(3*20);
				McuSoftReset();													//返回后直接重启					
			}
			else BspUartWrite(2,SIZE_OF("f_mkfs() 格式化错误！\r\n"));
		}
	}
	BspUartWrite(2,SIZE_OF("FormatFlashComm格式化FLASH通信失败！\r\n"));
	return 0;
}

/*******************************************************************************
名称：INT8U GetExtendedHeartbeatData(u8 *OutBuff)
功能：获取扩展心跳信息数据，根据扩展协议格式传出，并返回长度11字节。
入参：无
出参：u8 *OutBuff，扩展心跳信息数据存放地址
返回：11：长度
*******************************************************************************/
INT8U GetExtendedHeartbeatData(u8 *OutBuff)
{
	INT16U	temp=0;	
	float	f_temp=0;
  
	NW_GetTime((struct NW_TIME *)OutBuff);
	if(!Get_DS18B20Temp(&temp))													//返回的是DS18B20的F8格式温度
	{
		BspUartWrite(2,SIZE_OF("DS18B20---->Error!\r\n"));
	}
	f_temp = TempU16toFloat(temp);												//DS18B20得到的温度数据转浮点型
	//Equipment_state结构体已在心跳组帧时更新
	OutBuff[6]=(INT8U)(Equipment_state.FALA_Volt*10);							//获取法拉电容电压（10倍）
	OutBuff[7]=(((INT16U)(f_temp*10+500))>>8)&0xFF;								//取18b20温度高8位
	OutBuff[8]=(((INT16U)(f_temp*10+500)))&0xFF;								//取18b20温度低8位    
	OutBuff[9]=(((INT16U)(Equipment_state.MCU_Temp*10+500))>>8)&0xFF;			//MCU温度高8位
	OutBuff[10]=(INT16U)(Equipment_state.MCU_Temp*10+500)&0xFF;					//MCU温度低8位
	return 11;	
}

/*******************************************************************************
名称：INT8U GetDeviceVersionAndCardNumber(INT8U* pOutBuff)
功能：获取装置版本号以及卡号，根据扩展协议格式传出，并返回数据域长度。
入参：INT8U* pOutBuff
出参：无
返回：33：长度
*******************************************************************************/
INT8U GetDeviceVersionAndCardNumber(INT8U* pOutBuff)
{
	INT8U				i = 0, k = 0, *p;
	
	BspUartWrite(2,SIZE_OF("查询版本及卡号，控制字：F6H\r\n"));
	
	pOutBuff[0]=(HV>>24) & 0xFF;
	pOutBuff[1]=(HV>>16) & 0xFF;
	pOutBuff[2]=(HV>>8) & 0xFF;
	pOutBuff[3]=(HV>>0) & 0xFF;
	pOutBuff[4]=(VERSION>>24) & 0xFF;
	pOutBuff[5]=(VERSION>>16) & 0xFF;
	pOutBuff[6]=(VERSION>>8) & 0xFF;
	pOutBuff[7]=(VERSION>>0) & 0xFF;

	ME909S_Trans_OFF();															//关闭透传
	p=ME909SCommandP(AT_CNUM, "\",\"", 1)+1;									//+CNUM: "","+8615088666002",145
	for(i=0;i<7;i++)
	{
		k = i*2;
		*(p+k)<0x30 ? *(p+k)=0x0F : (*(p+k)-=0x30);
		*(p+k+1)<0x30 ? *(p+k+1)=0x0F : (*(p+k+1)-=0x30);
		pOutBuff[8+i] = (*(p+k)<<4) | *(p+k+1);									//按协议填入数值	pOutBuff[8]
	}
	
	p=ME909SCommandP(AT_ICCID, "OK", 1)-26;										//20位ICCID+0D0A+0D0A+OK=26
	for(i=0;i<10;i++)
	{
		k = i*2;
		*(p+k)<0x30 ? *(p+k)=0x0F : (*(p+k)-=0x30);
		*(p+k+1)<0x30 ? *(p+k+1)=0x0F : (*(p+k+1)-=0x30);
		pOutBuff[15+i] = (*(p+k)<<4) | *(p+k+1);								//按协议填入数值	pOutBuff[15]
	}
	
	p=ME909SCommandP(AT_IMSI, "OK", 1)-22;										//15位ICCID+0D0A+0D0A+OK=21，为了最高位F 20
	for(i=0;i<8;i++)
	{
		k = i*2;
		*(p+k)<0x30 ? *(p+k)=0x0F : (*(p+k)-=0x30);
		*(p+k+1)<0x30 ? *(p+k+1)=0x0F : (*(p+k+1)-=0x30);
		pOutBuff[25+i] = (*(p+k)<<4) | *(p+k+1);								//按协议填入数值	pOutBuff[25]
	}
	ME909S_Trans_ON();															//重新打开透传
	
	return 33;	
}

/*******************************************************************************
名称：INT8U GetUpdataRequestData(u8 *OutBuff)
功能：获取升级请求回复帧内容，根据扩展协议格式传出，并返回数据域长度。
入参：无
出参：u8 *OutBuff，组帧数据域首地址
返回：5：长度
*******************************************************************************/
INT8U GetUpdataRequestData(u8 *OutBuff)
{
	INT32U version = VERSION;													//版本号宏定义，点魔术棒
	version = htonl(version);													//大小端转换
	memcpy(OutBuff,&version,4);													//本地版本号
	OutBuff[4] = device_status;													//已在接收到升级请求时，获取过设备状态														
	return 5;
}

/*******************************************************************************
名称：INT8U GetUpdataFillingData(u8 *OutBuff)
功能：获取补包回复帧内容，根据扩展协议格式传出，并返回数据域长度。
入参：无
出参：u8 *OutBuff，组帧数据域首地址
返回：数据域长度
*******************************************************************************/
INT8U GetUpdataFillingData(u8 *OutBuff)
{
	INT8U				j,buff;
	INT16U				i,sum=0,sum_byte=0,lost_count=0,lost_number=0;
	TCHAR				temp[20]={0};
	
	sum=(file_update.Sub_package_Sum_High<<8)+file_update.Sub_package_Sum_Low;	//子包总数
	sum_byte = (sum>>3)+1;														//占用的字节数
	for(i=0;i<sum_byte;i++)
	{
		buff = subbag_statistics[i];											//取当前字节
		if(buff==0xFF) continue;												//若当前字节无丢包标记，快速跳过
		for(j=0;j<8;j++)
		{
			if(0==(buff & 0x80))												//不为1，表示当前子包丢失			
			{
				lost_number = (i<<3)+j+1;										//计算真实子包号
				if(lost_number>sum) break;										//超过总包数的不算
				OutBuff[6+2*lost_count] = (lost_number<<8)&0xFF;				//子包号高字节
				OutBuff[6+2*lost_count+1] = lost_number&0xFF;					//子包号低字节
				lost_count++;													//丢包包数
			}
			buff <<= 1;															//左移1位
		}
	}
	
	sprintf(temp, "丢失：%d 包\r\n", lost_count);
	BspUartWrite(2,(INT8U*)temp,strlen(temp));
	memcpy(OutBuff,file_update.Version,4);										//升级软件版本号	（这个对的）
	OutBuff[4] = (lost_count>>8)&0xFF;											//补包数高8位
	OutBuff[5] = lost_count&0xFF;												//补包数低8位
	return 6+2*lost_count;														//返回数据域长度
}

/*******************************************************************************
名称：void GetDeviceStatusBeforeUpgrade(void)
功能：升级前检测设备状态。为FFH时，表示允许升级；00H表示电池电压不足，01H表示存储空间
不足，02H表示当前有未上报的历史数据，03H表示装置存在故障。
入参：无
出参：无
返回：设备状态
*******************************************************************************/
void GetDeviceStatusBeforeUpgrade(void)
{
	INT32U	size = 0;

	size = htonl( *(INT32U*)file_update.Program_Size );							//取文件大小，大小端转换
	if(file_update.forced_upgrade==0xFF) 
	{
		if(BKP->DR3==0x02) device_status = 0xFE;								//若当前运行系统一，切换系统升级
		else device_status = 0xFF;												//被迫允许升级
		BspUartWrite(2,SIZE_OF("强制升级\r\n"));
		return;
	}
	else if(Equipment_state.BAT_Volt<BAT_UP && Equipment_state.FALA_Volt<FALA_UP)//保险点，用上限	
	{
		device_status=0x00;														//电源电压不足
		BspUartWrite(2,SIZE_OF("电源欠压\r\n"));
	}
	else if(size > (FLASH_UPDATE_PAGES<<11))									//一页2K，<<11
	{
		device_status=0x01;														//存储空间不足	
		BspUartWrite(2,SIZE_OF("存储空间不足\r\n"));
	}
	else if(IsUploadCompletely())
	{
		device_status=0x02;														//有未上传的历史数据	
		BspUartWrite(2,SIZE_OF("有未上传的历史数据\r\n"));
	}
	else if(Fault_Manage.Need_Report)											
	{
		device_status=0x03;														//有未上报的故障信息	
		BspUartWrite(2,SIZE_OF("有未上报的故障信息\r\n"));
	}
	/*其他升级条件已具备，判断系统*/
	else if(BKP->DR3==0x02) 													//若当前运行系统一
	{
		device_status = 0xFE;													//切换系统升级
		BspUartWrite(2,SIZE_OF("条件具备，切换系统一升级\r\n"));	
	}
	
	else 
	{
		device_status = 0xFF;													//上述条件全部满足，允许升级
		BspUartWrite(2,SIZE_OF("条件具备，允许升级\r\n"));	
	}
}

/*******************************************************************************
名称：void RestartAfterCrcCheckPassed(void)
功能：若CRC校验通过，标记备份寄存器，然后重启。
入参：无
出参：无	
返回：无
*******************************************************************************/
void RestartAfterCrcCheckPassed(void)
{
	INT16U				subbag_sum;
	
	update_start = false;														//标记升级结束
	subbag_sum=(file_update.Sub_package_Sum_High<<8)+file_update.Sub_package_Sum_Low;
	if(FlashCrcCheckFromNeiFlash( bin_file_adress, subbag_sum>>1)) 				//若CRC通过，写入flash的最后一页的页数为1024B一包的（总包数-1）/2+1
	{
		if(BKP->DR3==0x01)
		{
			PWR->CR|=1<<8;														//DBP位：取消后备区域的写保护。1：允许写入RTC和后备寄存器
			BKP->DR2=0x00;														//清SYS1失败计数，下次重启后会进入SYS1
			PWR->CR &= ~(1<<8);													//启用后备区域的写保护
			sys2_upgrade_time = RtcGetTimeSecond();								//世纪秒
			BSP_WriteDataToFm(sys2_upgrade_time_Addr,(INT8U*)&sys2_upgrade_time,sys2_upgrade_time_Len);			//系统2升级时间写入铁电
		}
		BspUartWrite(2,SIZE_OF("\r\n--------当前运行系统 0 ，重启后切换系统-------\r\n\r\n"));
		OSTimeDly(2);
		McuSoftReset();															//软件复位，标记热启动
	}
	else																		//CRC校验失败
	{
		BspUartWrite(2,SIZE_OF("\r\n--------CRC校验不通过，升级失败！-------\r\n\r\n"));
		OSTimeDly(1);
	}
}

/*******************************************************************************
名称：INT8U FlashCrcCheckFromNeiFlash( INT32U Address, INT16U LastBlockNo ) 
功能：对目标Flash地址进行CRC校验
入参：Address 起始地址   LastBlockNo  更新代码写在Flash的最后一页的页数
出参：无
返回：1：CRC通过   0：CRC失败
*******************************************************************************/
INT8U FlashCrcCheckFromNeiFlash( INT32U Address, INT16U LastBlockNo ) 
{
	INT32U CRCR;
	INT32U value_in_memory=0xffffffff;							
	INT32U FlashAddr;
	INT16U Blockindex; 
	INT16U i;
	
	__disable_irq();															//禁用所有中断
	RCC_AHBPeriphClockCmd( RCC_AHBPeriph_CRC, ENABLE );							//开CRC计算单元时钟
	FlashAddr= Address;
	CRC_ResetDR();																//Resets the CRC Data register (DR).DR初值ffffffff
	
	for( Blockindex=0; Blockindex<LastBlockNo; Blockindex++ )
	{
		for( i=0; i<(FLASH_PAGE_LEN>>2); i++ )									//FLASH_PAGE_LEN/4次
		{
			value_in_memory=*( vu32*) FlashAddr;								//取值
			value_in_memory = htonl( value_in_memory );							//bin文件是大端模式，单片机是小端模式，所以算CRC前进行大小端转化（BOOT程序里的没这个，因为上位机软件中已经转了）
			CRCR=CRC_CalcCRC(value_in_memory);									//利用CRC计算单元计算CRC
			FlashAddr+=4;														//下一地址
		}
		OSTimeDly(1);															//每个区块算完喂狗
	} 
	__enable_irq();
	if(CRCR==0)
	{
		BspUartWrite(2,SIZE_OF("Flash CRC通过！\r\n"));OSTimeDly(1);
		return 1;										
	}		
	else 
	{
		BspUartWrite(2,SIZE_OF("Flash CRC失败！\r\n"));OSTimeDly(1);
		return 0;
	}
}

/*******************************************************************************
名称：void SubbagUpdateToFlash(INT8U *Inbuff)
功能：解析更新文件的数据帧和保存写入对应地址Flash
入参：INT8U *Inbuff	接收到的数据帧指针		INT16U leng		数据帧长度
出参：无
返回：无
*******************************************************************************/
void SubbagUpdateToFlash(INT8U *Inbuff)
{
	INT16U				data_len;
	INT32U				*data_32_p;
	INT16U				sub_num;
	TCHAR				char_array[50]={0};
	
	data_32_p = (INT32U*)(Inbuff+16);											//指向升级包数据起始地址
	data_len = ((INT16U)Inbuff[8]<<8) + Inbuff[9] -6;							//计算升级包数据长度（保险起见，进行强制转换），不包含子包包号的两个字节,也不包含装置密码
	sub_num=((INT16U)Inbuff[14]<<8) + Inbuff[15];								//子包包号高位乘以256加上子包包号低位，从1开始计数；
	MarkSubbagStatisticsArray(sub_num);											//标记subbag_statistics数组，接收完成的包对应位置置1	
	
	switch(file_update.Format)
	{
		case 0xFC:																//axf文件，不仅包含代码数据，而且还包含着调试信息的编译文件
				break;
		
		case 0xFD:																//Bin文件，只包含最直接的代码映像，不包含地址信息的编译文件		
				if(data_len==1024)												//一包有效数258个字，1024字节				
				{
					Feed_Dog();													//下发速度快时，喂狗任务由于优先级低不容易进，会导致复位
					Wrtie_NFlashNoErase(bin_file_adress+((sub_num-1)<<10), data_32_p, 256);		//将接收到的数据写入STM32内部FLASH
					Feed_Dog();
					sprintf(char_array, "---------第 %d 包已写入---------\r\n", sub_num);
					BspUartWrite(2,(INT8U*)char_array,strlen(char_array));
//					OSTimeDly(7);												//不加延时，因为会直接回到等待接收。加了容易丢包。按38400波特率计算，需要6.25个时间片打印完成
				}
				break;
		
		case 0xFE:																//Hex文件，包含地址信息的编译文件
				break;		 

		default:
				break;
	}	
}

/*******************************************************************************
名称：void MarkSubbagStatisticsArray( INT16U num )
功能：标记subbag_statistics数组，接收完成的包对应位置置1
入参：接收完成的子包号		
出参：无
返回：无
*******************************************************************************/
void MarkSubbagStatisticsArray( INT16U num )
{
	INT16U				num_byte = 0;
	INT8U				num_bit = 0;
	
	num -= 1;																	//按协议规定，包号从1开始计数，还原到0
	num_byte = num>>3;															//标记在第几个字节
	num_bit = num%8;															//标记在第几位
	
	subbag_statistics[num_byte] |= (0x80>>num_bit);
}

/*******************************************************************************
名称：void UpgradePreparation(INT8U *inbuff)
功能：标记为开始升级，并准备更新。
入参：INT8U *inbuff	数据帧指针
出参：无
返回：无
*******************************************************************************/
void UpgradePreparation(INT8U *inbuff)
{
	BspUartWrite(2,SIZE_OF("\r\n---------->远程升级<----------\r\n"));OSTimeDly(1);
	update_start = true;														//标记为开始升级
	Wrtie_ErasePage(bin_file_adress, FLASH_UPDATE_PAGES);						//擦除 bin_file_adress 开始的40*2K=80K空间（擦除就是全写ff）
	memset(subbag_statistics,0,STA_NUM);										//清空子包记录数组
}

/*******************************************************************************
名称：void RestartToUpgrade(void)
功能：装置切换到稳定版本程序进行接收并升级。
入参：无
出参：无
返回：无
*******************************************************************************/
void RestartToUpgrade(void)
{
	PWR->CR|=1<<8;																
	BKP->DR2=0xff;
	PWR->CR &= ~(1<<8);															//启用后备区域的写保护
	BspUartWrite(2,SIZE_OF("\r\n--------当前运行系统 1 ，切换系统升级-------\r\n\r\n"));
	OSTimeDly(2);		
	McuSoftReset();
}

/*******************************************************************************
名称：void CheckSys2OperatingNormally(struct BSPRTC_TIME *pTime)
功能：成功运行24h后，认为程序正常，SYS1运行次数清零。以传入的北京时间计算判断。
入参：INT8U* Time，北京时间，东8区
出参：无
返回：无
*******************************************************************************/
void CheckSys2OperatingNormally(struct BSPRTC_TIME *pTime)
{
	time_t sceond = 0;															//typedef unsigned int time_t;     /* date/time in unix secs past 1-Jan-70 */
	struct tm TTM = {0};   
	
	if(BKP->DR3==0x02) 															//若当前运行系统一
	{
		TTM.tm_year = BcdToHex(pTime->Year)+100;  								/* 年份，其值等于实际年份减去1900 */
		TTM.tm_mon  = BcdToHex(pTime->Month)-1;   								/* 月份（从一月开始，0代表一月） - 取值区间为[0,11] */
		TTM.tm_mday = BcdToHex(pTime->Day);       								/* 一个月中的日期 - 取值区间为[1,31] */
		TTM.tm_hour = BcdToHex(pTime->Hour);      								/* 时 - 取值区间为[0,23] */
		TTM.tm_min  = BcdToHex(pTime->Minute);    								/* 分 - 取值区间为[0,59] */
		TTM.tm_sec  = BcdToHex(pTime->Second);    								/* 秒 – 取值区间为[0,59] */
		sceond = mktime(&TTM)-8*3600;                  							//时间转换成世纪秒	-8*3600：北京时间转换为0区
		
		if(sceond-sys2_upgrade_time>86400)										//运行超过24小时才清BKP->DR2
		{
			PWR->CR|=1<<8;														//DBP位：取消后备区域的写保护。1：允许写入RTC和后备寄存器
			BKP->DR2=0x00;														//清SYS1失败计数，下次重启后继续进入SYS1
			PWR->CR &= ~(1<<8);													//启用后备区域的写保护
		}
	}
}

/*******************************************************************************
名称：INT8U GetUpgradeTime(void)
功能：从铁电读取系统2升级时间，存入sys2_upgrade_time数组。
入参：无
出参：无
返回：无
*******************************************************************************/
INT8U GetUpgradeTime(void)
{
	if(!BSP_ReadDataFromFm(sys2_upgrade_time_Addr,(INT8U*)&sys2_upgrade_time,sys2_upgrade_time_Len)) return 0;
	return 1;
}

/*******************************************************************************
名称：INT8U IsUploadCompletely(void)
功能：判断是否有未上传的历史数据。
入参：无
出参：无
返回：1：有		0：无
*******************************************************************************/
INT8U IsUploadCompletely(void)
{
	INT8U i,j;
	
	for(i=0;i<31;i++)
	{
		for(j=0;j<3;j++)
		{
			if(Unreport_Index[i][j]^0xff) return 1;								//有未上传的历史记录
		}	
	}
	return 0;																	//无上传的历史记录
}
