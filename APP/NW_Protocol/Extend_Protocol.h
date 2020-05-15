#ifndef	__EXtend_Protocol_H
#define	__EXtend_Protocol_H
#include "main.h"


/*�궨��*/
#define	FLASH_PAGE_LEN			2048
#define	FLASH_UPDATE_PAGES		64												//�ݶ������ļ���С���2*64=128K
#define	UPDATA_MAX_PACKAGES		1024											//�ݶ����ְ�1024��
#define STA_NUM					(UPDATA_MAX_PACKAGES/8)							//subbag_statistics���鳤��


/*�ļ��ṹ��*/
#pragma pack(1)						//�趨Ϊ1�ֽڶ���
struct FILE_UPDATA
{
	INT8U					Version[4];											//�ļ��汾
	INT8U					forced_upgrade;										//ǿ������
	INT32U					Check_CRC;											//�ļ�CRC
	INT8U					Format;												//�ļ���ʽ
	INT8U					Program_Size[4];									//�ļ���С�����ģʽ
	INT8U					Sub_package_Sum_High;  								//�ļ��ְ�����λ
	INT8U					Sub_package_Sum_Low;   								//�ļ��ְ�����λ
};
#pragma pack()						//ȡ��1�ֽڶ���



/*ȫ�ֱ�������*/
extern INT32U				bin_file_adress;
extern INT8U				subbag_statistics[STA_NUM];
extern struct FILE_UPDATA	file_update;
extern bool					update_start;
extern INT8U				device_status;
extern INT8U				upgrade_timeout;
extern INT32U				sys2_upgrade_time;





/*��������*/
INT8U ExtendOnOffComm(u8 *InBuff, u16 Len);
INT8U SetApnComm(u8 *InBuff, u16 Len);
INT8U FormatFlashComm(u8 *InBuff, u16 Len);
INT8U UpdataRequestComm(u8 *InBuff);
INT8U UpdataDownloadComm(u8 *InBuff);
INT8U UpdataFinishComm(u8 *InBuff);
INT8U GetExtendedHeartbeatData(u8 *OutBuff);
INT8U GetDeviceVersionAndCardNumber(INT8U* pOutBuff);
INT8U GetUpdataRequestData(u8 *OutBuff);
INT8U GetUpdataFillingData(u8 *OutBuff);
void GetDeviceStatusBeforeUpgrade(void);
void RestartAfterCrcCheckPassed(void);
INT8U FlashCrcCheckFromNeiFlash( INT32U Address, INT16U LastBlockNo );
void SubbagUpdateToFlash(INT8U *Inbuff);
void MarkSubbagStatisticsArray( INT16U num );
void UpgradePreparation(INT8U *inbuff);
void RestartToUpgrade(void);
void BigEndToSmallEnd(INT32U *inbuff,INT16U word_num);
void CheckSys2OperatingNormally(struct BSPRTC_TIME *pTime);
INT8U GetUpgradeTime(void);
INT8U IsUploadCompletely(void);

#endif
