/***************************** (C) COPYRIGHT 2019 ���ϵ��� *****************************
* File Name          : Extend_Protocol.c
* Author             : ����հ����ӱ�ɡ���
* Version            : ����ʷ�汾��Ϣ
* Date               : 2020/04/13
* Description        : ��������Э���д��ͨ�Ź��ܺ�����
************************************  ��ʷ�汾��Ϣ  ************************************
* 2019/03/28    : V4.1.0
* Description   : ����������Ŀ���档����������ɣ������С�
*******************************************************************************/

#include "Extend_Protocol.h"

INT32U				bin_file_adress = 0x08027000;								//ȱʡֵ0x08027000
struct	FILE_UPDATA	file_update = {0};
INT8U				subbag_statistics[STA_NUM] = {0}; 							//ÿһλ����һ��������{0xE3,0xFF...}��1110 0011 1111 1111...����ʾ��0��1��2��6��7��8...�����ճɹ�����3��4��5������ʧ��
bool				update_start = false;										//�·���������󽫸ñ�־λ1�Ż���������������
INT8U				device_status = 0;											//�豸״̬��ΪFFHʱ����ʾ����������00H��ʾ��ص�ѹ���㣬01H��ʾ�洢�ռ䲻�㣬02H��ʾ��ǰ��δ�ϱ�����ʷ���ݣ�03H��ʾװ�ô��ڹ��ϡ�
INT8U				upgrade_timeout = 0;										//����DevStatCtr()������ѯ����Զ��������ʱ���˳�����ģʽ
INT32U				sys2_upgrade_time = 0;										//ϵͳ2����ʱ�䣬�������ʾ







/*******************************************************************************
���ƣ�INT8U ExtendOnOffComm(u8 *InBuff, u16 Len)
���ܣ�����������顢�ظ�ͨ�š���չЭ��ʹ�ܿ��ƹ���ִ�У������֣�F0H
��Σ�u8 *InBuff����������ݣ�����Ϊ���յ���֡��u16 Len������
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U ExtendOnOffComm(u8 *InBuff, u16 Len)
{
	if(!PassworkCheckAndReport(InBuff));										//�ж���վ�·������������Ƿ���ȷ������ȷʱ�ϱ����������Ϣ��
	else																		//����ͬ��������ȷ
	{
		if(InBuff[14]==0xFF)													//������չЭ��
		{
			/*�Ȳ���*/
		}
		else																	//������չЭ��
		{
			/*�Ȳ���*/
		}
		LteCommunication(InBuff,Len,0,0);										//���óɹ�������ԭ����أ������գ�LteCommunication����0��
		return 1;
	}
	BspUartWrite(2,SIZE_OF("ExtendOnOffComm��չЭ��ʹ�ܿ���ͨ��ʧ�ܣ�\r\n"));
	return 0;
}

/*******************************************************************************
���ƣ�INT8U SetApnComm(u8 *InBuff, u16 Len)
���ܣ�����������顢�ظ�ͨ�š�APN�������ù���ִ�У������֣�F3H
��Σ�u8 *InBuff����������ݣ�����Ϊ���յ���֡��u16 Len������
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U SetApnComm(u8 *InBuff, u16 Len)
{
	if(!PassworkCheckAndReport(InBuff));										//�ж���վ�·������������Ƿ���ȷ������ȷʱ�ϱ����������Ϣ��
	else																		//����ͬ��������ȷ
	{
		if(InBuff[14]!='\0')													//APN�ǿ�
		{
			strncpy((char*)APN, (char*)InBuff+14, APN_Len);						//д��APN����
			if(BSP_WriteDataToFm(APN_Addr, APN, APN_Len))						//д������
			{
				LteCommunication(InBuff,Len,0,0);								//���óɹ�������ԭ����أ������գ�LteCommunication����0��
				return 1;	
			}		
		}
	}
	BspUartWrite(2,SIZE_OF("SetApnComm����APNͨ��ʧ�ܣ�\r\n"));
	return 0;
}

/*******************************************************************************
���ƣ�INT8U UpdataRequestComm(u8 *InBuff)
���ܣ���վ�·��������󣬿����֣�F7H
��վ���յ���չ����5����·���������װ�ý��յ���վ������������̽���������Ⲣ���лظ���
��Σ�u8 *InBuff����������ݣ�����Ϊ���յ���֡
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U UpdataRequestComm(u8 *InBuff)
{
	INT16U	len_frame=0;														//֡����
	
	if(!PassworkCheckAndReport(InBuff));										//�ж���վ�·������������Ƿ���ȷ������ȷʱ�ϱ����������Ϣ��
	else																		//����ͬ��������ȷ
	{
		BspUartWrite(2,SIZE_OF("�������󣬿����֣�F7H\r\n"));
		memcpy(&file_update,InBuff+14,sizeof(file_update));						//���ļ�������Ϣ��¼�ڽṹ�� file_update ��
		GetDeviceStatusBeforeUpgrade();											//��ȡ�豸״̬
		if(device_status==0xFF) UpgradePreparation( InBuff ); 					//�������������Ϊ��ʼ��������׼�����£��ظ�ǰ�������ⶪ��һ����

		len_frame = NW_Framing(EX_UPDATA_REQUEST,LTE_Tx_Buff);					//���ݿ�������֡    
		LteCommunication(LTE_Tx_Buff,len_frame,0,0);							//�ظ����������
		
		if(device_status==0xFE)	RestartToUpgrade();								//���������Ѿ߱���װ�����л����ȶ��汾������н��ղ���������Э��Ҫ���Ȼظ���������
		return 1;																//ͨ����ɷ���1����NW_Comm_Process�������ֽ���
	}
	BspUartWrite(2,SIZE_OF("UpdataRequestComm�����������벻��ȷ��\r\n"));
	return 0;																	//���Գ��������ȴ���ʱ������0
}

/*******************************************************************************
���ƣ�INT8U UpdataDownloadComm(u8 *InBuff)
���ܣ���վ�������·��������֣�F8H
��վ�·�������������������Ϊ���������������ݰ��·����Ϊ1�롣
��Σ�u8 *InBuff����������ݣ�����Ϊ���յ���֡
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U UpdataDownloadComm(u8 *InBuff)
{
	if(!PassworkCheckAndReport(InBuff));										//�ж���վ�·������������Ƿ���ȷ������ȷʱ�ϱ����������Ϣ��
	else																		//����ͬ��������ȷ
	{
		if(update_start==true) 													//��ֹ��վ�����·����������ƻ�FLASH��
		{
			BspUartWrite(2,SIZE_OF("�������·��������֣�F8H"));
			SubbagUpdateToFlash( InBuff );										//�����ļ��ͱ��棨д��FLASH����
		}
		else BspUartWrite(2,SIZE_OF("��ǰ��������״̬��������δд��FLASH��"));
		return 1;																//ͨ����ɻ�������״̬ʱ����1����NW_Comm_Process�������ֽ�
	}
	BspUartWrite(2,SIZE_OF("UpdataDownloadComm�������·����벻��ȷ��\r\n"));
	return 0;																	//���Գ��������ȴ���ʱ������0
}

/*******************************************************************************
���ƣ�INT8U UpdataFinishComm(u8 *InBuff)
���ܣ���վ�������·������������֣�F9H
ȫ���������·�������2�룬��վ���͸�ָ�װ���յ��������ϴ��ļ�������FAH����
��Σ�u8 *InBuff����������ݣ�����Ϊ���յ���֡
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U UpdataFinishComm(u8 *InBuff)
{
	INT16U	len_frame=0;														//֡����
	
	if(!PassworkCheckAndReport(InBuff));										//�ж���վ�·������������Ƿ���ȷ������ȷʱ�ϱ����������Ϣ��
	else																		//����ͬ��������ȷ
	{
		if(update_start==true)													//��ֹ��վ�����·����������ƻ�FLASH																				
		{
			BspUartWrite(2,SIZE_OF("�������·������������֣�F9H\r\n"));
			len_frame=NW_Framing(EX_UPDATA_FILLING,LTE_Tx_Buff);				//����������Ϣ��汾����֡��������FA��
			LteCommunication(LTE_Tx_Buff,len_frame,0,0);						//�ϱ������ȴ��ظ�
			if( len_frame==18 )													//��������Ϊ0��12+6������ʾ�������ѽ�����ȫ
				RestartAfterCrcCheckPassed();									//��CRCУ��ͨ������Ǳ��ݼĴ�����Ȼ��������
		}
		else BspUartWrite(2,SIZE_OF("��ǰ��������״̬���յ��쳣F9Hָ�"));
		return 1;																//ͨ����ɻ�������״̬ʱ����1����NW_Comm_Process�������ֽ���
	}
	BspUartWrite(2,SIZE_OF("UpdataFinishComm�������·��������벻��ȷ��\r\n"));
	return 0;																	//���Գ��������ȴ���ʱ������0
}

/*******************************************************************************
���ƣ�INT8U FormatFlashComm(u8 *InBuff, u16 Len)
���ܣ�����������顢�ظ�ͨ�š���ʽ��FLASH����ִ�У������֣�FBH
��Σ�u8 *InBuff����������ݣ�����Ϊ���յ���֡��u16 Len������
���Σ���
���أ��ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U FormatFlashComm(u8 *InBuff, u16 Len)
{
	FRESULT				res;													/* API result code */
	
	if(!PassworkCheckAndReport(InBuff));										//�ж���վ�·������������Ƿ���ȷ������ȷʱ�ϱ����������Ϣ��
	else																		//����ͬ��������ȷ
	{
		if(InBuff[14]==0xFF)													//ǿ�Ƹ�ʽ��FLASH��Ŀǰֻ�д�����
		{
			/* Create FAT volume */
			res = f_mkfs(	"", 												// If it has no drive number in it, it means the default drive.
							FM_FAT|FM_SFD, 										// Specifies the format option in combination of FM_FAT, FM_FAT32, FM_EXFAT and bitwise-or of these three, FM_ANY. If two or more types are specified, one out of them will be selected depends on the volume size and au.
							4096, 												// The valid value is n times the sector size.
							work, 												// Pointer to the working buffer used for the format process
							sizeof work);										// It needs to be the sector size of the corresponding physical drive at least.
			if(res==FR_OK) 
			{
				LteCommunication(InBuff,Len,0,0);								//�����ɹ�������ԭ����أ������գ�LteCommunication����0��
				BspUartWrite(2,SIZE_OF("f_mkfs() ��ʽ���ɹ���3���������\r\n"));
				OSTimeDly(3*20);
				McuSoftReset();													//���غ�ֱ������					
			}
			else BspUartWrite(2,SIZE_OF("f_mkfs() ��ʽ������\r\n"));
		}
	}
	BspUartWrite(2,SIZE_OF("FormatFlashComm��ʽ��FLASHͨ��ʧ�ܣ�\r\n"));
	return 0;
}

/*******************************************************************************
���ƣ�INT8U GetExtendedHeartbeatData(u8 *OutBuff)
���ܣ���ȡ��չ������Ϣ���ݣ�������չЭ���ʽ�����������س���11�ֽڡ�
��Σ���
���Σ�u8 *OutBuff����չ������Ϣ���ݴ�ŵ�ַ
���أ�11������
*******************************************************************************/
INT8U GetExtendedHeartbeatData(u8 *OutBuff)
{
	INT16U	temp=0;	
	float	f_temp=0;
  
	NW_GetTime((struct NW_TIME *)OutBuff);
	if(!Get_DS18B20Temp(&temp))													//���ص���DS18B20��F8��ʽ�¶�
	{
		BspUartWrite(2,SIZE_OF("DS18B20---->Error!\r\n"));
	}
	f_temp = TempU16toFloat(temp);												//DS18B20�õ����¶�����ת������
	//Equipment_state�ṹ������������֡ʱ����
	OutBuff[6]=(INT8U)(Equipment_state.FALA_Volt*10);							//��ȡ�������ݵ�ѹ��10����
	OutBuff[7]=(((INT16U)(f_temp*10+500))>>8)&0xFF;								//ȡ18b20�¶ȸ�8λ
	OutBuff[8]=(((INT16U)(f_temp*10+500)))&0xFF;								//ȡ18b20�¶ȵ�8λ    
	OutBuff[9]=(((INT16U)(Equipment_state.MCU_Temp*10+500))>>8)&0xFF;			//MCU�¶ȸ�8λ
	OutBuff[10]=(INT16U)(Equipment_state.MCU_Temp*10+500)&0xFF;					//MCU�¶ȵ�8λ
	return 11;	
}

/*******************************************************************************
���ƣ�INT8U GetDeviceVersionAndCardNumber(INT8U* pOutBuff)
���ܣ���ȡװ�ð汾���Լ����ţ�������չЭ���ʽ�����������������򳤶ȡ�
��Σ�INT8U* pOutBuff
���Σ���
���أ�33������
*******************************************************************************/
INT8U GetDeviceVersionAndCardNumber(INT8U* pOutBuff)
{
	INT8U				i = 0, k = 0, *p;
	
	BspUartWrite(2,SIZE_OF("��ѯ�汾�����ţ������֣�F6H\r\n"));
	
	pOutBuff[0]=(HV>>24) & 0xFF;
	pOutBuff[1]=(HV>>16) & 0xFF;
	pOutBuff[2]=(HV>>8) & 0xFF;
	pOutBuff[3]=(HV>>0) & 0xFF;
	pOutBuff[4]=(VERSION>>24) & 0xFF;
	pOutBuff[5]=(VERSION>>16) & 0xFF;
	pOutBuff[6]=(VERSION>>8) & 0xFF;
	pOutBuff[7]=(VERSION>>0) & 0xFF;

	ME909S_Trans_OFF();															//�ر�͸��
	p=ME909SCommandP(AT_CNUM, "\",\"", 1)+1;									//+CNUM: "","+8615088666002",145
	for(i=0;i<7;i++)
	{
		k = i*2;
		*(p+k)<0x30 ? *(p+k)=0x0F : (*(p+k)-=0x30);
		*(p+k+1)<0x30 ? *(p+k+1)=0x0F : (*(p+k+1)-=0x30);
		pOutBuff[8+i] = (*(p+k)<<4) | *(p+k+1);									//��Э��������ֵ	pOutBuff[8]
	}
	
	p=ME909SCommandP(AT_ICCID, "OK", 1)-26;										//20λICCID+0D0A+0D0A+OK=26
	for(i=0;i<10;i++)
	{
		k = i*2;
		*(p+k)<0x30 ? *(p+k)=0x0F : (*(p+k)-=0x30);
		*(p+k+1)<0x30 ? *(p+k+1)=0x0F : (*(p+k+1)-=0x30);
		pOutBuff[15+i] = (*(p+k)<<4) | *(p+k+1);								//��Э��������ֵ	pOutBuff[15]
	}
	
	p=ME909SCommandP(AT_IMSI, "OK", 1)-22;										//15λICCID+0D0A+0D0A+OK=21��Ϊ�����λF 20
	for(i=0;i<8;i++)
	{
		k = i*2;
		*(p+k)<0x30 ? *(p+k)=0x0F : (*(p+k)-=0x30);
		*(p+k+1)<0x30 ? *(p+k+1)=0x0F : (*(p+k+1)-=0x30);
		pOutBuff[25+i] = (*(p+k)<<4) | *(p+k+1);								//��Э��������ֵ	pOutBuff[25]
	}
	ME909S_Trans_ON();															//���´�͸��
	
	return 33;	
}

/*******************************************************************************
���ƣ�INT8U GetUpdataRequestData(u8 *OutBuff)
���ܣ���ȡ��������ظ�֡���ݣ�������չЭ���ʽ�����������������򳤶ȡ�
��Σ���
���Σ�u8 *OutBuff����֡�������׵�ַ
���أ�5������
*******************************************************************************/
INT8U GetUpdataRequestData(u8 *OutBuff)
{
	INT32U version = VERSION;													//�汾�ź궨�壬��ħ����
	version = htonl(version);													//��С��ת��
	memcpy(OutBuff,&version,4);													//���ذ汾��
	OutBuff[4] = device_status;													//���ڽ��յ���������ʱ����ȡ���豸״̬														
	return 5;
}

/*******************************************************************************
���ƣ�INT8U GetUpdataFillingData(u8 *OutBuff)
���ܣ���ȡ�����ظ�֡���ݣ�������չЭ���ʽ�����������������򳤶ȡ�
��Σ���
���Σ�u8 *OutBuff����֡�������׵�ַ
���أ������򳤶�
*******************************************************************************/
INT8U GetUpdataFillingData(u8 *OutBuff)
{
	INT8U				j,buff;
	INT16U				i,sum=0,sum_byte=0,lost_count=0,lost_number=0;
	TCHAR				temp[20]={0};
	
	sum=(file_update.Sub_package_Sum_High<<8)+file_update.Sub_package_Sum_Low;	//�Ӱ�����
	sum_byte = (sum>>3)+1;														//ռ�õ��ֽ���
	for(i=0;i<sum_byte;i++)
	{
		buff = subbag_statistics[i];											//ȡ��ǰ�ֽ�
		if(buff==0xFF) continue;												//����ǰ�ֽ��޶�����ǣ���������
		for(j=0;j<8;j++)
		{
			if(0==(buff & 0x80))												//��Ϊ1����ʾ��ǰ�Ӱ���ʧ			
			{
				lost_number = (i<<3)+j+1;										//������ʵ�Ӱ���
				if(lost_number>sum) break;										//�����ܰ����Ĳ���
				OutBuff[6+2*lost_count] = (lost_number<<8)&0xFF;				//�Ӱ��Ÿ��ֽ�
				OutBuff[6+2*lost_count+1] = lost_number&0xFF;					//�Ӱ��ŵ��ֽ�
				lost_count++;													//��������
			}
			buff <<= 1;															//����1λ
		}
	}
	
	sprintf(temp, "��ʧ��%d ��\r\n", lost_count);
	BspUartWrite(2,(INT8U*)temp,strlen(temp));
	memcpy(OutBuff,file_update.Version,4);										//��������汾��	������Եģ�
	OutBuff[4] = (lost_count>>8)&0xFF;											//��������8λ
	OutBuff[5] = lost_count&0xFF;												//��������8λ
	return 6+2*lost_count;														//���������򳤶�
}

/*******************************************************************************
���ƣ�void GetDeviceStatusBeforeUpgrade(void)
���ܣ�����ǰ����豸״̬��ΪFFHʱ����ʾ����������00H��ʾ��ص�ѹ���㣬01H��ʾ�洢�ռ�
���㣬02H��ʾ��ǰ��δ�ϱ�����ʷ���ݣ�03H��ʾװ�ô��ڹ��ϡ�
��Σ���
���Σ���
���أ��豸״̬
*******************************************************************************/
void GetDeviceStatusBeforeUpgrade(void)
{
	INT32U	size = 0;

	size = htonl( *(INT32U*)file_update.Program_Size );							//ȡ�ļ���С����С��ת��
	if(file_update.forced_upgrade==0xFF) 
	{
		if(BKP->DR3==0x02) device_status = 0xFE;								//����ǰ����ϵͳһ���л�ϵͳ����
		else device_status = 0xFF;												//������������
		BspUartWrite(2,SIZE_OF("ǿ������\r\n"));
		return;
	}
	else if(Equipment_state.BAT_Volt<BAT_UP && Equipment_state.FALA_Volt<FALA_UP)//���յ㣬������	
	{
		device_status=0x00;														//��Դ��ѹ����
		BspUartWrite(2,SIZE_OF("��ԴǷѹ\r\n"));
	}
	else if(size > (FLASH_UPDATE_PAGES<<11))									//һҳ2K��<<11
	{
		device_status=0x01;														//�洢�ռ䲻��	
		BspUartWrite(2,SIZE_OF("�洢�ռ䲻��\r\n"));
	}
	else if(IsUploadCompletely())
	{
		device_status=0x02;														//��δ�ϴ�����ʷ����	
		BspUartWrite(2,SIZE_OF("��δ�ϴ�����ʷ����\r\n"));
	}
	else if(Fault_Manage.Need_Report)											
	{
		device_status=0x03;														//��δ�ϱ��Ĺ�����Ϣ	
		BspUartWrite(2,SIZE_OF("��δ�ϱ��Ĺ�����Ϣ\r\n"));
	}
	/*�������������Ѿ߱����ж�ϵͳ*/
	else if(BKP->DR3==0x02) 													//����ǰ����ϵͳһ
	{
		device_status = 0xFE;													//�л�ϵͳ����
		BspUartWrite(2,SIZE_OF("�����߱����л�ϵͳһ����\r\n"));	
	}
	
	else 
	{
		device_status = 0xFF;													//��������ȫ�����㣬��������
		BspUartWrite(2,SIZE_OF("�����߱�����������\r\n"));	
	}
}

/*******************************************************************************
���ƣ�void RestartAfterCrcCheckPassed(void)
���ܣ���CRCУ��ͨ������Ǳ��ݼĴ�����Ȼ��������
��Σ���
���Σ���	
���أ���
*******************************************************************************/
void RestartAfterCrcCheckPassed(void)
{
	INT16U				subbag_sum;
	
	update_start = false;														//�����������
	subbag_sum=(file_update.Sub_package_Sum_High<<8)+file_update.Sub_package_Sum_Low;
	if(FlashCrcCheckFromNeiFlash( bin_file_adress, subbag_sum>>1)) 				//��CRCͨ����д��flash�����һҳ��ҳ��Ϊ1024Bһ���ģ��ܰ���-1��/2+1
	{
		if(BKP->DR3==0x01)
		{
			PWR->CR|=1<<8;														//DBPλ��ȡ���������д������1������д��RTC�ͺ󱸼Ĵ���
			BKP->DR2=0x00;														//��SYS1ʧ�ܼ������´�����������SYS1
			PWR->CR &= ~(1<<8);													//���ú������д����
			sys2_upgrade_time = RtcGetTimeSecond();								//������
			BSP_WriteDataToFm(sys2_upgrade_time_Addr,(INT8U*)&sys2_upgrade_time,sys2_upgrade_time_Len);			//ϵͳ2����ʱ��д������
		}
		BspUartWrite(2,SIZE_OF("\r\n--------��ǰ����ϵͳ 0 ���������л�ϵͳ-------\r\n\r\n"));
		OSTimeDly(2);
		McuSoftReset();															//�����λ�����������
	}
	else																		//CRCУ��ʧ��
	{
		BspUartWrite(2,SIZE_OF("\r\n--------CRCУ�鲻ͨ��������ʧ�ܣ�-------\r\n\r\n"));
		OSTimeDly(1);
	}
}

/*******************************************************************************
���ƣ�INT8U FlashCrcCheckFromNeiFlash( INT32U Address, INT16U LastBlockNo ) 
���ܣ���Ŀ��Flash��ַ����CRCУ��
��Σ�Address ��ʼ��ַ   LastBlockNo  ���´���д��Flash�����һҳ��ҳ��
���Σ���
���أ�1��CRCͨ��   0��CRCʧ��
*******************************************************************************/
INT8U FlashCrcCheckFromNeiFlash( INT32U Address, INT16U LastBlockNo ) 
{
	INT32U CRCR;
	INT32U value_in_memory=0xffffffff;							
	INT32U FlashAddr;
	INT16U Blockindex; 
	INT16U i;
	
	__disable_irq();															//���������ж�
	RCC_AHBPeriphClockCmd( RCC_AHBPeriph_CRC, ENABLE );							//��CRC���㵥Ԫʱ��
	FlashAddr= Address;
	CRC_ResetDR();																//Resets the CRC Data register (DR).DR��ֵffffffff
	
	for( Blockindex=0; Blockindex<LastBlockNo; Blockindex++ )
	{
		for( i=0; i<(FLASH_PAGE_LEN>>2); i++ )									//FLASH_PAGE_LEN/4��
		{
			value_in_memory=*( vu32*) FlashAddr;								//ȡֵ
			value_in_memory = htonl( value_in_memory );							//bin�ļ��Ǵ��ģʽ����Ƭ����С��ģʽ��������CRCǰ���д�С��ת����BOOT�������û�������Ϊ��λ��������Ѿ�ת�ˣ�
			CRCR=CRC_CalcCRC(value_in_memory);									//����CRC���㵥Ԫ����CRC
			FlashAddr+=4;														//��һ��ַ
		}
		OSTimeDly(1);															//ÿ����������ι��
	} 
	__enable_irq();
	if(CRCR==0)
	{
		BspUartWrite(2,SIZE_OF("Flash CRCͨ����\r\n"));OSTimeDly(1);
		return 1;										
	}		
	else 
	{
		BspUartWrite(2,SIZE_OF("Flash CRCʧ�ܣ�\r\n"));OSTimeDly(1);
		return 0;
	}
}

/*******************************************************************************
���ƣ�void SubbagUpdateToFlash(INT8U *Inbuff)
���ܣ����������ļ�������֡�ͱ���д���Ӧ��ַFlash
��Σ�INT8U *Inbuff	���յ�������ָ֡��		INT16U leng		����֡����
���Σ���
���أ���
*******************************************************************************/
void SubbagUpdateToFlash(INT8U *Inbuff)
{
	INT16U				data_len;
	INT32U				*data_32_p;
	INT16U				sub_num;
	TCHAR				char_array[50]={0};
	
	data_32_p = (INT32U*)(Inbuff+16);											//ָ��������������ʼ��ַ
	data_len = ((INT16U)Inbuff[8]<<8) + Inbuff[9] -6;							//�������������ݳ��ȣ��������������ǿ��ת�������������Ӱ����ŵ������ֽ�,Ҳ������װ������
	sub_num=((INT16U)Inbuff[14]<<8) + Inbuff[15];								//�Ӱ����Ÿ�λ����256�����Ӱ����ŵ�λ����1��ʼ������
	MarkSubbagStatisticsArray(sub_num);											//���subbag_statistics���飬������ɵİ���Ӧλ����1	
	
	switch(file_update.Format)
	{
		case 0xFC:																//axf�ļ������������������ݣ����һ������ŵ�����Ϣ�ı����ļ�
				break;
		
		case 0xFD:																//Bin�ļ���ֻ������ֱ�ӵĴ���ӳ�񣬲�������ַ��Ϣ�ı����ļ�		
				if(data_len==1024)												//һ����Ч��258���֣�1024�ֽ�				
				{
					Feed_Dog();													//�·��ٶȿ�ʱ��ι�������������ȼ��Ͳ����׽����ᵼ�¸�λ
					Wrtie_NFlashNoErase(bin_file_adress+((sub_num-1)<<10), data_32_p, 256);		//�����յ�������д��STM32�ڲ�FLASH
					Feed_Dog();
					sprintf(char_array, "---------�� %d ����д��---------\r\n", sub_num);
					BspUartWrite(2,(INT8U*)char_array,strlen(char_array));
//					OSTimeDly(7);												//������ʱ����Ϊ��ֱ�ӻص��ȴ����ա��������׶�������38400�����ʼ��㣬��Ҫ6.25��ʱ��Ƭ��ӡ���
				}
				break;
		
		case 0xFE:																//Hex�ļ���������ַ��Ϣ�ı����ļ�
				break;		 

		default:
				break;
	}	
}

/*******************************************************************************
���ƣ�void MarkSubbagStatisticsArray( INT16U num )
���ܣ����subbag_statistics���飬������ɵİ���Ӧλ����1
��Σ�������ɵ��Ӱ���		
���Σ���
���أ���
*******************************************************************************/
void MarkSubbagStatisticsArray( INT16U num )
{
	INT16U				num_byte = 0;
	INT8U				num_bit = 0;
	
	num -= 1;																	//��Э��涨�����Ŵ�1��ʼ��������ԭ��0
	num_byte = num>>3;															//����ڵڼ����ֽ�
	num_bit = num%8;															//����ڵڼ�λ
	
	subbag_statistics[num_byte] |= (0x80>>num_bit);
}

/*******************************************************************************
���ƣ�void UpgradePreparation(INT8U *inbuff)
���ܣ����Ϊ��ʼ��������׼�����¡�
��Σ�INT8U *inbuff	����ָ֡��
���Σ���
���أ���
*******************************************************************************/
void UpgradePreparation(INT8U *inbuff)
{
	BspUartWrite(2,SIZE_OF("\r\n---------->Զ������<----------\r\n"));OSTimeDly(1);
	update_start = true;														//���Ϊ��ʼ����
	Wrtie_ErasePage(bin_file_adress, FLASH_UPDATE_PAGES);						//���� bin_file_adress ��ʼ��40*2K=80K�ռ䣨��������ȫдff��
	memset(subbag_statistics,0,STA_NUM);										//����Ӱ���¼����
}

/*******************************************************************************
���ƣ�void RestartToUpgrade(void)
���ܣ�װ���л����ȶ��汾������н��ղ�������
��Σ���
���Σ���
���أ���
*******************************************************************************/
void RestartToUpgrade(void)
{
	PWR->CR|=1<<8;																
	BKP->DR2=0xff;
	PWR->CR &= ~(1<<8);															//���ú������д����
	BspUartWrite(2,SIZE_OF("\r\n--------��ǰ����ϵͳ 1 ���л�ϵͳ����-------\r\n\r\n"));
	OSTimeDly(2);		
	McuSoftReset();
}

/*******************************************************************************
���ƣ�void CheckSys2OperatingNormally(struct BSPRTC_TIME *pTime)
���ܣ��ɹ�����24h����Ϊ����������SYS1���д������㡣�Դ���ı���ʱ������жϡ�
��Σ�INT8U* Time������ʱ�䣬��8��
���Σ���
���أ���
*******************************************************************************/
void CheckSys2OperatingNormally(struct BSPRTC_TIME *pTime)
{
	time_t sceond = 0;															//typedef unsigned int time_t;     /* date/time in unix secs past 1-Jan-70 */
	struct tm TTM = {0};   
	
	if(BKP->DR3==0x02) 															//����ǰ����ϵͳһ
	{
		TTM.tm_year = BcdToHex(pTime->Year)+100;  								/* ��ݣ���ֵ����ʵ����ݼ�ȥ1900 */
		TTM.tm_mon  = BcdToHex(pTime->Month)-1;   								/* �·ݣ���һ�¿�ʼ��0����һ�£� - ȡֵ����Ϊ[0,11] */
		TTM.tm_mday = BcdToHex(pTime->Day);       								/* һ�����е����� - ȡֵ����Ϊ[1,31] */
		TTM.tm_hour = BcdToHex(pTime->Hour);      								/* ʱ - ȡֵ����Ϊ[0,23] */
		TTM.tm_min  = BcdToHex(pTime->Minute);    								/* �� - ȡֵ����Ϊ[0,59] */
		TTM.tm_sec  = BcdToHex(pTime->Second);    								/* �� �C ȡֵ����Ϊ[0,59] */
		sceond = mktime(&TTM)-8*3600;                  							//ʱ��ת����������	-8*3600������ʱ��ת��Ϊ0��
		
		if(sceond-sys2_upgrade_time>86400)										//���г���24Сʱ����BKP->DR2
		{
			PWR->CR|=1<<8;														//DBPλ��ȡ���������д������1������д��RTC�ͺ󱸼Ĵ���
			BKP->DR2=0x00;														//��SYS1ʧ�ܼ������´��������������SYS1
			PWR->CR &= ~(1<<8);													//���ú������д����
		}
	}
}

/*******************************************************************************
���ƣ�INT8U GetUpgradeTime(void)
���ܣ��������ȡϵͳ2����ʱ�䣬����sys2_upgrade_time���顣
��Σ���
���Σ���
���أ���
*******************************************************************************/
INT8U GetUpgradeTime(void)
{
	if(!BSP_ReadDataFromFm(sys2_upgrade_time_Addr,(INT8U*)&sys2_upgrade_time,sys2_upgrade_time_Len)) return 0;
	return 1;
}

/*******************************************************************************
���ƣ�INT8U IsUploadCompletely(void)
���ܣ��ж��Ƿ���δ�ϴ�����ʷ���ݡ�
��Σ���
���Σ���
���أ�1����		0����
*******************************************************************************/
INT8U IsUploadCompletely(void)
{
	INT8U i,j;
	
	for(i=0;i<31;i++)
	{
		for(j=0;j<3;j++)
		{
			if(Unreport_Index[i][j]^0xff) return 1;								//��δ�ϴ�����ʷ��¼
		}	
	}
	return 0;																	//���ϴ�����ʷ��¼
}
