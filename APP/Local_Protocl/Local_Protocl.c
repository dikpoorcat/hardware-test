/***************************** (C) COPYRIGHT 2019 ���ϵ��� *****************************
* File Name          : Local_Protocl.c
* Author             : �з���
* Version            : ����ʷ�汾��Ϣ
* Date               : 2019/03/12
* Description        : ����Э�飬���ڻ�վ�豸����λ������485ͨ�ţ����ò�������ԡ�
************************************  ��ʷ�汾��Ϣ  ************************************
* 2019/03/28    : V4.1.0
* Description   : ����������Ŀ���档����������ɣ������С�
*******************************************************************************/
#include "Local_Protocl.h"

/*ȫ�ֱ���*/
INT8U					B485BUF[Buff485_LEN] = { 0 };   						//����B485ͨ��
INT8U  					Reset_Flag = COLDRST;    								//�临λ��Ĭ��ֵ�����ȸ�λ�������и�λ
INT8U					Reset_Count = 0;          								//������������ʶ
struct Str_Msg 			GgdataBox; 
struct Str_Msg 			*GyMess = (struct Str_Msg *)0;      
struct LOCAL_PROTOCAL	Local_Protocal;      									//���ڱ���Э��ͨ��
OS_EVENT  				*GyBOX = (OS_EVENT *)0;

BYTE					work[FF_MAX_SS];										/* Work area (larger is better for processing time) */   //FF_MAX_SS
FIL						fil;            										/* File object */
FATFS					fs;														/* Filesystem object */
FATFS					*fs0;

																														
/*******************************************************************************
���ƣ�void Task_Local_main(void *arg)
���ܣ�����Э�鴦����������������������λ��ͨ�Ų����в������õȡ���װ���Լ�Ӧ��Ҳ������������ZE��
************************************************************************************************************************/
void Task_Local_main(void *arg)
{
    INT8U WaitTime = 6;															//�ϵ��ڼ��485���������ò��� WaitTime*10��
	INT8U state=0;

/*Ӳ����ʼ��*/
	BSP_InitFm(LOC_Num);														//��ʼ������洢
    B485_init(38400);															//485��ʼ��������38400����LOCAL�������ʱ��ǿ��ִ�У��������B485DIS�궨���ж��Ƿ�ִ�У�
	DrawSysLogo();																//SYS0����ӡBTC	logo	SYS1����ӡSYS1
	
/*Ӳ������*/
	HardwareTest();
/*===============================����������������===============================*/	
	
/*�豸�Լ�*/
	WaitTime = Check_Reset_Mod(WaitTime);										//�ж�������ʽ������Ӧ�������������������WaitTime����������λ��ͨ��
	FM_Space_Usage();															//����ռ�ռ����Ϣ
	Check_Getfree();															//����Ƿ�Ҫ��ʽ�����������豸д�����Ĭ�ϲ�����

/*���������ȡ������ڸ�ʽ��֮��*/
	Read_TT_From_FM();															//��ȡ̽ͷ��Ϣ
	LTE_Get_Config();   														//��ȡLTE����
	Print_Config(true);															//��ӡ��ǰ����
	Get_NW_Info();																//��ȡFault_Info������Ϣ�ṹ�����顢δ�ϱ������������
	GetUpgradeTime();															//��ȡϵͳ2����ʱ��
	
/*�ȴ���λ��ͨ�ż�����*/
	BspUartWrite(2,SIZE_OF("---------->>�ȴ���λ��ͨ��<<----------\r\n"));
    while(WaitTime--)															//�ϵ��ڼ��485���������ò��� WaitTime*10��
    {
		state = Wait_Local_Comm(10);											//���ȴ�10s	
        if(state==1) WaitTime = 6;   											//һ������������ͨ��PC���߲�����վ�Ļ�������������ȴ�ʱ��Ϊ1���ӡ� 				
		else if(state==0xFF) break;												//���յ������ȴ�����	68 ff ff ff ff ff 02 00 00 00 65 16
    }
	/*���ò�����ȡ,�����ڱ��������б��ı䣬�ɴ����¶�һ��*/
	Read_TT_From_FM();															//��ȡ̽ͷ��Ϣ			
	LTE_Get_Config();   														//��ȡLTE����
	BspUartWrite(2,SIZE_OF("---------->>��λ��ͨ�Ž���<<----------\r\n"));
	
/*FATFSĿ¼ά����*/
	Dir_Maintenance();															//FATFSĿ¼ά����������λ��ͨ��֮�󣬱�������
//	Dir_Test();																	//Ŀ¼����
//	File_Test();																//�ļ���������

/*�͹��Ĵ���*/
	BSP_UART_Close(2); 															//���ʹ��ڵ����ģ�������B485_LowPower��
	FM_LowPower(LOC_Num);														//����洢�͹���	  

/*�ָ�RF����*/
	OSTaskResume(RF_Task_Prio);													//�ָ�RF����
	
/*�ȴ���ѹ�������ָ�LTE����*/
	while(1)																	//�������while����Ҳ�У�LTE������Ҳ�н��ܹ���ģʽ�ж���
	{
		if(Equipment_state.BAT_Volt>BAT_UP || Equipment_state.FALA_Volt>FALA_UP)//BAT>9.5V����FALA>5.5V
		{
			OSTaskResume(LTE_Task_Prio);										//�ָ�LTE����
			break;
		}
		BspUartWrite(2,SIZE_OF("��ԴǷѹ�����磡\r\n"));
		OSTimeDly(3*60*20);														//�ȴ����3min	
		Get_Voltage_MCUtemp_Data( 3 );											//��ȡ��ص�ѹ���ݺ͵�Ƭ���¶�
		OSTimeDly(10);															//�ȴ���ѹ��ӡ���
		WDTClear(Local_Task_Prio);
	}
	#ifdef B485DIS
	BspUartWrite(2,SIZE_OF("---------->>B485��ӡ�ѹر�<<----------\r\n"));
	#endif
//	OSTimeDly(10);																//�ȴ���ӡ���
	
/*ɾ����������*/
    while(1)
    {	
		WDTClear(Local_Task_Prio);
		TaskActive &= Local_INACT;												//��ɾ���ɹ���������Ѳ������������Ź�
		OSTaskDel(OS_PRIO_SELF);												//ɾ��������ɾ���ɹ�֮������Ķ���������		
		TaskActive |= Local_ACT;												//������������˵��ûɾ���ɹ����������Ѳ���������������Ź�
		OSTimeDly(60*20);														//60s
    }
}







/*******************************************************************************
���ƣ�INT8U Check_Reset_Mod(u8 waittime)
���ܣ��ж�������ʽ������Ӧ�����������������Ҫ�ⲿ�ȶ�485��ʼ����ע��Hot_Reset_Flag����������
�У���ȡ�󶼲�Ӧ�ٶ��丳ֵ�������������κεط�ʹ�ô�ȫ�ֱ�����
��Σ�u8 waittime���ϵ��ڼ��485���������ò��� waittime*10��
���Σ���
���أ�������ʱ�᷵��0�����򷵻ش���ֵ������ֵ���ڸ�ֵ��WaitTime��������ʱ���ȴ�485������
*******************************************************************************/
INT8U Check_Reset_Mod(u8 waittime)
{
	INT8U	LTE_Sending_Flag = DONE;
	
	/*���ڵ��Դ�ӡ*/
    BspUartWrite(2,SIZE_OF("\r\n485 is OK!\r\n")); 									
	
	/*��ȡ�����еı�־λ*/
	BSP_ReadDataFromFm(Reset_Flag_Addr,&Reset_Flag,1);							//��ȡ��λ��־
	BSP_ReadDataFromFm(Reset_Count_Addr,&Reset_Count,1);						//��ȡ��λ����
	BSP_ReadDataFromFm(LTE_Sending_Flag_Addr,&LTE_Sending_Flag,1);   			//���͹��̱�ʶ��ȡ����
	
	/*�ж�������ʽ����Ӧ����*/
	if(LTE_Sending_Flag == UNDONE) Reset_Flag = FAULTRST;						//��UNDONE�������ڷ����������ˣ�дΪFault����													
	switch(Reset_Flag)
	{
		case COLDRST:															//�临λ
				BspUartWrite(2,SIZE_OF("==============================Cold Reset!==============================\r\n"));
				Reset_Count++;													//ϵͳ������λ2�κ�LTE����ǰǿ�ƴ򿪵�أ��������رղ������							
				BSP_WriteDataToFm(Reset_Count_Addr,&Reset_Count,1);				//д������
				break;
		
		case FAULTRST:															//�����и�λ
				BspUartWrite(2,SIZE_OF("------------------------------Fault Reset!------------------------------\r\n"));
				/*����Ҫ��Ҫ�ϵ�����*/
				break;
		
		case HOTRST:															//������ʱ������������־Ĩ��
				BspUartWrite(2,SIZE_OF("Hot Reset!\r\n")); 
				waittime = 0;
				break;
		
		default:																//�������ǣ���洢����
				break;
	}
	Reset_Flag = COLDRST;
	BSP_WriteDataToFm(Reset_Flag_Addr,&Reset_Flag,1); 							//дΪ��������ʶ�����ó�ʼֵ��
	OSTimeDly(2);

	/*������ʱ�᷵��0�����򷵻ش���ֵ*/
	return waittime;															//����WaitTime����������λ��ͨ��
}

/*******************************************************************************
���ƣ�INT8U Wait_Local_Comm(INT8U WaitTime)
���ܣ�ͨ��485�ȴ�����ͨ�ţ���ִ����������Ӧ�Ĺ��ܡ���485һ�����ڴ�ӡ��������յĻ�һ��ֻ
����λ���·�������ָ���˵���Local_Protocol_Process������
��Σ�INT8U WaitTime�����ڵȴ���ʱʱ������λ����
���Σ���
���أ�1���ɹ����ղ�����   0��ʧ��		0xFF���յ������ȴ�ָ��
*******************************************************************************/
INT8U Wait_Local_Comm(INT8U WaitTime)
{
    INT8U Err = 0, state = 0; 
    
    while(WaitTime--)
    {
		StopModeLock++;
        GyMess = (struct Str_Msg *)OSMboxPend(GyBOX,20,&Err);   				//�ȴ�������Ϣ1s
		if(StopModeLock) StopModeLock--;										//��ͣ����
        if (Err == OS_NO_ERR)
    	{
            if( GyMess->MsgID == BSP_MSGID_RS485DataIn)							//���ID���ǲ���485���ڷ�����
    		{
				if(GyMess->DataLen > Buff485_LEN)								//���ȳ������쳣
				{
					BSP_UART_RxClear(GyMess->DivNum);	
					return 0;
				}
				
			/*��˾�ڲ�Э�����������*/
				state = Local_Protocol_Process(GyMess->pData,GyMess->DataLen,B485BUF);
				BSP_UART_RxClear(GyMess->DivNum);								//������ɺ󣬴��ڽ��ջ��������
				if(state==0) continue;											//�����ϱ���˾Э��ʱ�����ȴ�����������
				else return state;
    		}
    	}
    }
    return 0;  																	//�����ݽ���
}

/*******************************************************************************
���ƣ�INT8U Local_Protocol_Process(INT8U *In,INT16U inLen,INT8U *pUseBuff)
���ܣ�����˾���ڲ�Э�����������
��Σ�INT8U *In,����������ָ�룻INT16U inLen,���������ݳ��ȣ�INT8U *pUseBuff������485����ʹ�õĻ��档
���Σ���
���أ�1���ɹ�����������   0��ʧ��		0xFF���յ������ȴ�ָ��
*******************************************************************************/
INT8U Local_Protocol_Process(INT8U *In,INT16U inLen,INT8U *pUseBuff)
{
	INT8U *pRet = NULL;
	INT8U Err = 0;
	INT16U Rlen = 0;  															//һ֡��Ч����

	pRet = Judge_Local_Framing(In,inLen,&Rlen);									//�ж��Ƿ���ϱ���Э�飬������Local_Protocal�ṹ��
	if(pRet)  																	//�ҵ�һ֡��Ч������
	{
		switch(Local_Protocal.DoType)
    	{
            case 0: // ���ò���
					Err = Set_Local_Para(Local_Protocal.CMD,Local_Protocal.Pointer,Local_Protocal.CMDlen);			//���ز�������
					Local_Protocol_Reply(Err,Local_Protocal.CMD,pUseBuff);   										//��֡���ظ�
					break;
    		
            case 1: // ��ȡ����
                    Err = Get_Local_Para(Local_Protocal.CMD,pUseBuff+10,&Rlen);										//��ȡ����������pUseBuff+10
                    if(Err) Local_Protocol_Reply(Err,Local_Protocal.CMD,pUseBuff);									//�����ϱ�
                    else Local_Protocol_Reply_Para(pUseBuff+10,Rlen,Local_Protocal.CMD,pUseBuff);					//�����ϱ�
            		break;
    		
			case 0xFF: //����485�ȴ�
					return 0xFF;												//��ʾ�յ������ȴ�ָ��
			
            case 0x0f: // �ļ�����												//�ɽ��ô˹��ܣ����������ܽ�������BOOT����ʵ��������Ҳ�������ģ�
            		break;
		}
		return 1;
	}
	return 0;
}

/*******************************************************************************
* Function Name : INT8U *Judge_Local_Framing(INT8U *pInBuff,INT16U Len,INT16U *pOutLen)
* Description   : �ж��Ƿ���ϱ���˾�ڲ���ԼЭ��
* Input         : pInBuff : �������ݰ�ͷָ��
*                 Len     : �������ݰ��ĳ���
*                 pOutLen �������ṩ�ģ����ڴ洢������Ч�����ȵĿռ��ָ��
*
* Return        : ��ʽ���� pRet    : ��ЧЭ���ͷָ�룬ָ��CMD�������׵�ַ
*               : �βη��� pOutLen : ��ЧЭ����ĳ���
*               ����ʽ���� Local_Protocal     ���������ȫ�ֻ������������ˡ�
*******************************************************************************/
INT8U *Judge_Local_Framing(INT8U *pInBuff,INT16U Len,INT16U *pOutLen)
{
	INT16U i = 0;
	INT16U Packet_Length = 0;
	INT8U *pRet = NULL;

/*Э�����У��*/
	if (Len < 8) return 0;  													//Э��������Ȳ���С��8
	for(i = 0; i<7; i++)
	{
		if(pInBuff[i] == 0x68)
		{
			pRet = &pInBuff[i]; 												//��¼����λ��
			Len -= i;           												//��������
			break;
		}
	}
	if(i >= 7) return 0;  														//ǰ8�ֽ���δ�ҵ���ͷ������ʧ��
	if(Len < 8) return 0;														//Э��������ȣ�����С��8�������������ٴ��жϣ�
	Packet_Length = pRet[7];													//�������ݳ��ȣ����ֽ�
	Packet_Length <<= 8;
	Packet_Length += pRet[6];													//���������ܳ���
	Packet_Length += 10;														//���峤��
	if (Len < Packet_Length) return 0;  										//Э�鳤�ȴ���
	if (0 == Judge_Device_Addr(pRet+1)) return 0;								//�ж��Ƿ�Ϊ��Ч�ĵ�ַ��ͨ�ŵ�ַ����ѡ��ͬһ�������ϵĲ�ͬ�豸��
	i = RTU_CS(pRet,Packet_Length-2);											//�����ۼӺ�
	if (i != pRet[Packet_Length-2]) return 0;									//У���ۼӺ�
	if (0x16 != pRet[Packet_Length-1]) return 0;								//У���β

/*�����յ����������Local_Protocal�ṹ��*/
	Local_Protocal.DoType = pRet[5];
	switch(Local_Protocal.DoType)
	{
		case 0:	//д
//				if(!memcmp(&Local_Protocal.Password,pRet+8,3)) return 0;		//����У�飨��λ�����빦�ܺ���û���ɣ�
				Local_Protocal.CMD = (pRet[12]<<8) + pRet[11];					//CMD u16
				Local_Protocal.CMDlen = Packet_Length - 15; 					//CMD��Ӧ�����ĳ���
				Local_Protocal.Pointer = &pRet[13];								//ָ��CMD�������׵�ַ
				break;

		case 1:	//��
				Local_Protocal.CMD = (pRet[9]<<8) + pRet[8];					//CMD u16
				Local_Protocal.CMDlen = 0;				 									
				Local_Protocal.Pointer = 0;
				break;

		case 0xff:																//ָ��	68 ff ff ff ff ff 02 00 00 00 65 16
				break;															//�����ȴ�
		
		case 0x0f:	//��������
		case 0x31:	//��װ���ж�ȡ��ʷ����
				Local_Protocal.Pointer = &pRet[11];								//ָ��CMD�������׵�ַ
				Local_Protocal.CMDlen = Packet_Length-14; 						//ID�ĳ���
				break;
		
		default:
				break;
	}
	if(pOutLen) *pOutLen = Packet_Length;										//����һ����ȷ�����ݳ���Packet_Length
	return pRet;																//������ЧЭ���׵�ַ
}

/*******************************************************************************
���ƣ�INT8U Set_Local_Para(INT16U CMD,INT8U *pInBuff,INT8U Len)
���ܣ�ͨ������Э����в������ã���ʱ�䡢IP���˿ںš���վ���š�̽ͷID�ȡ�
��Σ�INT16U CMD,�������INT8U *pInBuff,�������ݵ�ָ�룻INT8U Len���������ݵĳ��ȡ�
���Σ���
���أ�����Э��������
*******************************************************************************/
INT8U Set_Local_Para(INT16U CMD,INT8U *pInBuff,INT8U Len)
{
	INT8U				Temp = 0;
	static INT32U		buff[512] = {0};										//ռ�ÿռ�̫��С������
	
	switch(CMD)
	{
		case 0xffff:	/*���ع���*/ 
				if(!Local_Function(pInBuff)) return Other_Err;                 	//���ع��ܣ���λ��
				break;
		
		case 0x0001:	/*ϵͳʱ��*/
				if(!System_Time_Fun(pInBuff,WRITETYPE))
				{
					Time_Proofread = UNDONE;									//���ΪУʱδ���
					return Other_Err; 	    									//ϵͳʱ�����					
				}
				else
				{
					Time_Proofread = DONE;										//���ΪУʱ�����
				}
				break;
		
		case 0x0004:	/*IP��ַ+�˿ں�+APN*/
				Temp = pInBuff[5];												//�˿ںţ����ڱ��ز��õ���С��д�룬���������ô��ģʽ������˫�ֽڵĲ�����Ҫ�ߵ�һ��
				pInBuff[5] = pInBuff[6];
				pInBuff[6] = Temp;
		
				/*ʹ���ֽ� 1���ֽڣ�0x50��ʾ ��ͨ�������ϱ����ݣ��������ȴ�����	���������û�ã�������*/
				/*IP��˿�*/
				if(!BSP_WriteDataToFm(IP_Config_Addr,pInBuff+1,4)) return Other_Err;						//д�� IP_addr_1&2
				if(!BSP_WriteDataToFm(IP_Config_Addr+6,pInBuff+1,4)) return Other_Err;
				if(!BSP_WriteDataToFm(IP_Config_Addr+4,pInBuff+5,2)) return Other_Err;						//д�� PortNum_1&2 �˿ں�
				if(!BSP_WriteDataToFm(IP_Config_Addr+10,pInBuff+5,2)) return Other_Err;
				/*APN*/
				Temp = pInBuff[7];												//��λ���·�APN��һ���ֽ�Ϊ���ȣ�Ŀǰ����·�1+32�ֽ�
				pInBuff[8+Temp] = '\0';											//��λ���·�APN�޽�����������
				if(!BSP_WriteDataToFm(APN_Addr,pInBuff+8,APN_Len)) return Other_Err;						//д�� APN	
				break;

		case 0x1000:	/*дװ�ñ��*/
				/*д���ڲ�FLASH*/
				Read_NFlash(0x08006000-0x800, buff, 512);						//��ȡд�й����ҳ	ע�⣺������16���ƣ�Device_Number��BCD
				buff[448] = pInBuff[5]-0x30;									//�·���asciiתhex����д������Ӧλ�ã���ʵֵ����С��ģʽд�룩
				buff[448] += (pInBuff[4]-0x30)*10;
				buff[448] += (pInBuff[3]-0x30)*100;
				buff[448] += (pInBuff[2]-0x30)*1000;
				Wrtie_NFlash(0x08006000-0x800, buff, 512);						//�����������޸ĺõ���һҳд��ȥ

				/*д������*/
				if(!BSP_WriteDataToFm(Device_Number_Addr,pInBuff,Device_Number_Len)) return Other_Err;		//���ȱ����ǹ̶���6λ��PC�������Ļ���17λ����6λд�Ų���Ӱ�������洢	
				break;
		
//		case ????:	/*��վ����*/
//				if(!BSP_WriteDataToFm(IP_Config_Addr+12,pInBuff,Len)) return Other_Err;						//д�� CardNum_1&2 ��վ����
//				if(!BSP_WriteDataToFm(IP_Config_Addr+18,pInBuff,Len)) return Other_Err;
//				break;
					 
		case 0x11FF:	/*̽ͷID*/
				switch(pInBuff[0])												//����ָ��ѡ����Ӧ����
				{
					case 0: 
						if(! Add_TT_ID(pInBuff+1))return Other_Err;				//����һ��������
						break;
					case 1:
						if(! Delete_TT_ID(pInBuff+1))return Other_Err; 			//ɾ��һ��������		
						break;
					case 2:	
						if(! Delete_All_TT_ID())return Other_Err;	 			//ɾ��ȫ��������
						break;
				}
				break;
	}
	return No_Err;																//�����������޴���
}

/*******************************************************************************
* Function Name : INT8U Get_Local_Para(INT16U CMD,INT8U *pOutBuff,INT16U *pOutLen)
* Description   : ��ȡָ��CMD�������pOutBuffָ���Ŀռ���ȥ����ȡ���ĳ��ȡ�ת�ַ��ء����в�pOutLenָ���Ŀռ���ȥ��
* Input         : CMD      : ��Э�鸽��A������B
*                 pOutBuff : ������Ŀ�ĵ�ַ
*                 pOutLen  : CMD��Ӧ�ĳ���, �ڽṹ ID_STR��Tn_STR �ṹ���У�ID�ĳ��ȶ�������1�����ڱ���У���ֽ�    

*
* Return        : ����	  ���������
*                 No_Err  ����ȷ
*******************************************************************************/
INT8U Get_Local_Para(INT16U CMD,INT8U *pOutBuff,INT16U *pOutLen)
{
	INT8U Temp = 0, state = No_Err;
	
	switch(CMD)
	{
		case 0x0000:	/*���汾��*/
				pOutBuff[0]=(VERSION>>24) & 0xFF;
				pOutBuff[1]=(VERSION>>16) & 0xFF;
				pOutBuff[2]=(VERSION>>8) & 0xFF;
				pOutBuff[3]=(VERSION>>0) & 0xFF;
				*pOutLen = 4;
				return No_Err;
		
		case 0x0001:	/*�豸ʱ��*/
				 System_Time_Fun(pOutBuff,READTYPE);        					// ��ϵͳʱ��
				 *pOutLen = 7;	
				 return No_Err;	
	
		case 0x0004:	/*IP��ַ+�˿ں�+APN*/									//pOutBuff+7��APN������û�õ�������
				pOutBuff[0] = 0x50;												//ʹ���ֽ� 1���ֽڣ�0x50��ʾ ��ͨ�������ϱ����ݣ��������ȴ�����
				state = BSP_ReadDataFromFm(IP_Config_Addr,pOutBuff+1,4);		//���� IP_addr_1&2
				if(!state) break;

				state = BSP_ReadDataFromFm(IP_Config_Addr+4,pOutBuff+5,2);		//���� PortNum_1&2 �˿ں�
				if(!state) break;
		
				state = BSP_ReadDataFromFm(APN_Addr,pOutBuff+8,APN_Len);		//��APN pOutBuff+7��APN���ȣ�Ŀǰ���32
				if(!state) break;
		
				Temp = pOutBuff[5];												//���ڱ��ز��õ���С��д�룬���������ô��ģʽ������˫�ֽڵĲ�����Ҫ�ߵ�һ��
				pOutBuff[5] = pOutBuff[6];
				pOutBuff[6] = Temp;
				pOutBuff[7] = strlen((TCHAR*)APN);								//APN����
				*pOutLen = 8+32;
				return No_Err;

		case 0x1000:															//��ȡװ�ñ���
				if(BSP_ReadDataFromFm(Device_Number_Addr,pOutBuff,Device_Number_Len)){
					*pOutLen = Device_Number_Len;
					return No_Err;}
				break;
				
//		case ????: 	/*��֪����������վ������ʲô��˼������Э����ʱû����Ӧ��CMD*/
//				if(BSP_ReadDataFromFm(IP_Config_Addr+12,pOutBuff,6))			//���� CardNum_1&2 ��վ����
//				{
//					*pOutLen =6;
//					return No_Err;
//				}
//				break;
						
		default: break;
	}
/*�������ѯ*/
	if((CMD >= 0x1100)&&(CMD <= 0x1137))  										//�������ѯ�������55������
	{	
		*pOutLen = Read_TT_Num_Or_ID(CMD,pOutBuff);
		return No_Err;
	}
	return Other_Err;
}

/*******************************************************************************
* Function Name: INT8U System_Time_Fun(INT8U *pInOutBuff,INT8U Type)             
* Description:   ���û��߶�ȡϵͳʱ�� 
* Input:         pInOutBuff   : ָ��ʵʱʱ��ʱ�䴮��ָ�룬������Ҫ��˳���ǣ��ꡢ�¡��ա�ʱ���֡��롢��
*                Type         ��WRITETYPE������ϵͳʱ��
*                               READTYPE : ��ȡϵͳʱ��
*
* Return:        1���ɹ�   0��ʧ��
*******************************************************************************/
INT8U System_Time_Fun(INT8U *pInOutBuff,INT8U Type)
{
	if (Type == WRITETYPE) 											
	{
		//PC����������˳����������ʱ�����ܵ�BCD�룡��������BSPRTC_TIME��˳�������ʱ���������BCD�룡������Ҫת��һ��
		gSetTime.Year=pInOutBuff[0];
		gSetTime.Month=pInOutBuff[1];
		gSetTime.Day=pInOutBuff[2];
		gSetTime.Hour=pInOutBuff[3];
		gSetTime.Minute=pInOutBuff[4];
		gSetTime.Second=pInOutBuff[5];
		gSetTime.Week=pInOutBuff[6];
		return RtcSetChinaStdTimeStruct(&gSetTime);								//дϵͳʱ��
	}
	else
	{
		return GetSysTime(pInOutBuff);											//��ȡϵͳʱ�� 
	}
}
																				
/*******************************************************************************
* Function Name : INT8U Judge_Device_Addr(INT8U *pAddr)
* Description   : �ж��Ƿ�Ϊ��Ч��װ�õ�ַ
* Input         : pAddr : ָ��Э����װ�õ�ַ�ֽڵ�ָ��
*
* Return        : 0 : ��Ч��ַ
*               : 1 ����Ч��ַ
*******************************************************************************/
INT8U Judge_Device_Addr(INT8U *pAddr)
{
	INT16U len = 0;

	if((pAddr[0]==0xff)&&(pAddr[1]==0xff)&&(pAddr[2]==0xff)&&(pAddr[3]==0xff)) 	//4��0xff��ʾ�㲥��ַ����Ч
		return 1;   														
	Get_Local_Para(0x0002,Local_Protocal.Addr,&len);							//��ȡװ�õ�ַ
	if(0==memcmp(pAddr,&Local_Protocal.Addr,4)) return 1;
	return 0;
}

/*******************************************************************************
* Function Name : INT8U Local_Function(INT8U *pInBuff)                                                               
* Description   : CMD FFFF ��Ӧ�Ĵ�������
* Input         : pInBuff��
*                 
* Return        : 1���ɹ�	0��ʧ�ܣ�ͳһ��׼ZE��
*******************************************************************************/
INT8U Local_Function(INT8U *InBuff)
{
	switch(*InBuff)
	{
		case 0xff: // ϵͳ��λ
				McuSoftReset();													//�ֶ�ϵͳ��λ���ֶ���������ϵ�����Ʒ��������V1.3��-->��������-->ϵͳ��λ ��ť��ͻ���뵽���case��
				break;
					
		/*�������ܶ��������������*/
		default: 
				break;
	}
	return 1;
}

/*******************************************************************************
���ƣ�void Local_Protocol_Reply(INT8U Err,INT16U CMD,INT8U *pUseBuff)
���ܣ�������Э�飬ͨ��485���лظ�ͨ�š�
��Σ�INT8U Err,������룻INT8U DOType,0xC0ָ�����д��󷵻أ�0x80�޴��󷵻أ�INT16U CMD,INT8U *pUseBuff
���Σ���
���أ���
*******************************************************************************/
void Local_Protocol_Reply(INT8U Err,INT16U CMD,INT8U *pUseBuff)
{
	INT8U DOType;
    pUseBuff[0] = 0x68;                            								//Э���ͷ
    memcpy(&pUseBuff[1],Local_Protocal.Addr,4);             					//ͨ��Դ��ַ
	if(Err) DOType = 0xC0;   													//0xC0 ��ʶ�д��󷵻�
	else DOType = 0x80;      													//0x80 ��ʶ�޴��󷵻�	
    pUseBuff[5] = DOType;                          								//��������
    pUseBuff[6] = 3;                               								//�����򳤶ȣ�С��ģʽ
    pUseBuff[7] = 0;															//�̶�3�ֽ�
    pUseBuff[8] = CMD&0xff;                         							//CMD���ֽ�
    pUseBuff[9] = (CMD>>8)&0xff;                    							//CMD���ֽ�
    pUseBuff[10] = Err;                            								//�������
    pUseBuff[11] = RTU_CS(pUseBuff,11);            								//�ۼӺ�У��
    pUseBuff[12] = 0x16;                           								//Э���β

	BspUartWrite(2,pUseBuff,13);												//���͸�PC
}

/*******************************************************************************
���ƣ�void Local_Protocol_Reply_Para(INT8U *In,INT16U len,INT16U CMD,INT8U *pUseBuff)
���ܣ�������Э�飬ͨ��485���лظ�ͨ�ţ��ϱ���ȡ�Ĳ�����
��Σ�INT8U *In,INT16U len,INT16U CMD,INT8U *pUseBuff ̫�ӵ��ˣ����ø��ˣ��Լ�����
���Σ���
���أ���
*******************************************************************************/
void Local_Protocol_Reply_Para(INT8U *In,INT16U len,INT16U CMD,INT8U *pUseBuff)
{
	INT16U	i = 0;
	INT8U	DOType = 0x81;														//0x81����ʾ��װ�ö�ȡCMD����
	
    pUseBuff[0] = 0x68;                           								//Э���ͷ
    memcpy(pUseBuff+1,Local_Protocal.Addr,4);            						//ͨ��Դ��ַ
    pUseBuff[5] = DOType;                          								//��������
    pUseBuff[6] = (len+2) &0xff;
    pUseBuff[7] = ((len+2)>>8) &0xff;
    pUseBuff[8] = CMD&0xff;
    pUseBuff[9] = (CMD>>8)&0xff;
    if(In != pUseBuff+10)														//��ô����Ĳ���ͷһ�μ�  ����������
    {
        for(i=0;i<len;i++)
    	{
            pUseBuff[10+i] = In[i];
    	}
    }
    pUseBuff[10+len] = RTU_CS(pUseBuff,10+len);
    pUseBuff[11+len] = 0x16;

	BspUartWrite(2,pUseBuff,12+len);											//��PC��������ظ�
}

#if 1 /*============================================================̽ͷ¼�����ȡ����============================================================*/
/*******************************************************************************
* Function Name : INT8U Add_TT_ID(INT8U *pInBuff)
* Description   : ����һ��ID,���Ҫ���ӵ�ID���Ѿ����ڣ���ֱ�ӷ��ء�
*
* Input         : pInBuff: ָ��ID���ֽڴ���ָ�롣	�����pInBuffΪ FF FF +���ֽڵ�TT_ID
*
* Return        : 1 :��ʶִ�гɹ�
*******************************************************************************/
INT8U Add_TT_ID(INT8U *pInBuff)	
{
	INT8U INDEX=0;
	INT8U i=0;
	
	INDEX=CMP_TT_ID(pInBuff+2);													//���������в��ң�����������
	if(INDEX==0xFF)																//δ�ҵ�ƥ���ID��˵��δ¼��
	{
		for(i=0;i<55;i++)														//Ѱ�ҵ�һ����λ
		{
			if(TT_Info.TT_ID[i][0]!=0) continue;
			if(TT_Info.TT_ID[i][1]!=0) continue;								//�ѱ�ռ��
			
			memcpy(TT_Info.TT_ID[i],pInBuff+2,2);								//��λ�ã���ID����
			TT_Info.TT_Count+=1;												//¼�������1
			return BSP_WriteDataToFm(TT_Count_Addr,&TT_Info.TT_Count,sizeof(TT_Info));		//д������
		}
	}	
	return 1;																	//����¼���б������ҵ��������ظ�¼��							
}

/*******************************************************************************
* Function Name : INT8U Delete_TT_ID(INT8U *pInBuff)	
* Description   : Ѱ��һ����¼���̽ͷID������ɾ��֮
*
* Input         : pInBuff: ָ��ID���ֽڴ���ָ�롣	�����pInBuffΪ FF FF +���ֽڵ�TT_ID
*
* Return        : 1 :��ʶִ�гɹ�
*******************************************************************************/
INT8U Delete_TT_ID(INT8U *pInBuff)	
{
	INT8U 	INDEX=0;
	INT16U	TT_Data_Addr=0;
	
	INDEX=CMP_TT_ID(&pInBuff[2]);
	if(INDEX==0xFF)		return 1;												//δ�ҵ�ƥ���ID��˵��δ¼�룬����ɾ����ֱ�ӷ��سɹ�
	else{		
		memset(TT_Info.TT_ID[INDEX],0,2);										//ɾ����Ӧ��̽ͷID
		TT_Info.TT_Count-=1;													//���ҵ�������Countһ������0
		TT_Data_Addr=Sample_Data_Addr+One_TT_Sample_Data_Len*INDEX;				//��Ӧ̽ͷ�������д洢�ɼ����ݵ���ʼλ��
		BSP_FM_Erase(TT_Data_Addr,One_TT_Sample_Data_Len);						//ͬʱɾ����Ӧ̽ͷԭ�ɼ����ݣ�������¼���µ�̽ͷռ���˸�λ�ö���ɻ���
		return BSP_WriteDataToFm(TT_Count_Addr,&TT_Info.TT_Count,sizeof(TT_Info));			//д������
	}																									
}

/*******************************************************************************
* Function Name : INT8U Delete_All_TT_ID(void)		
* Description   : ɾ��������¼���̽ͷ
*
* Input         : ��
* Return        : 1 :��ʶִ�гɹ�
*******************************************************************************/
INT8U Delete_All_TT_ID(void)	
{
	memset(&TT_Info.TT_Count,0,sizeof(TT_Info));								//���̽ͷ��Ϣ�ṹ��												
	BSP_FM_Erase(Sample_Manage_Addr,Sample_Manage_Len);							//ͬʱ�����������ṹ�壨̽ͷ������ˣ�������������µ�̽ͷҲ�޷���Ӧ�ϣ�Ӧ����ԭ�������ݣ�
	return BSP_WriteDataToFm(TT_Count_Addr,&TT_Info.TT_Count,sizeof(TT_Info));	//д������																							
}

/*******************************************************************************
* Function Name : INT8U Read_TT_Num_Or_ID(INT16U ID,INT8U *pOutBuff)		
* Description   : ��ȡָ��̽ͷID
*
* Input         : ID��PC���ڷ������Ķ�ȡ̽ͷ������ID�Ĵ���		pOutBuff:����̽ͷID��λ��->������֡�ظ�
* Return        : pOutBuff���ݳ���
*******************************************************************************/
INT8U Read_TT_Num_Or_ID(INT16U ID,INT8U *pOutBuff)	
{	
	INT8U INDEX=0;
	if(ID==0x1100)																//��ȡ̽ͷ����
	{
		pOutBuff[0]=TT_Info.TT_Count;	
		return 1;
	}
		
	INDEX=ID-0x1101;															//ID��Χ0x1101~0x1137��Ӧ��������λ����0~54	
	pOutBuff[0]=0xFF;															//�ϱ����ڵ�IDǰ��λ�����FF
	pOutBuff[1]=0xFF;
	memcpy(pOutBuff+2,&TT_Info.TT_ID[INDEX],2);									//��������װ���Ӧ��̽ͷID
	return 4;																	//IDΪ4���ֽ�		
}
#endif

/*******************************************************************************
���ƣ�void FM_Space_Usage(void)
���ܣ��������ռ�ռ����Ϣ��
��Σ���
���Σ���
���أ���
*******************************************************************************/
void FM_Space_Usage(void)
{
	TCHAR   chars[256]={0};
	
	sprintf(chars, "ϵͳ����������K0��ʹ��%d�ֽڣ�%d%%USED��\r\n",FM_K0_End_Addr,100*FM_K0_End_Addr/0x400);
	BspUartWrite(2,(INT8U*)chars,strlen(chars));OSTimeDly(1);

	sprintf(chars, "�¶Ȳ���������K1-K7��ʹ��%d�ֽڣ�%d%%USED��\r\n",FM_K1_K7_End_Addr-0x400,100*(FM_K1_K7_End_Addr-0x400)/0x1C00);
	BspUartWrite(2,(INT8U*)chars,strlen(chars));OSTimeDly(1);
	
	sprintf(chars, "ͳ�ƣ�������ʹ��%d�ֽڣ�%d%%USED��\r\n",FM_K1_K7_End_Addr-0x400+FM_K0_End_Addr,100*(FM_K1_K7_End_Addr-0x400+FM_K0_End_Addr)/0x2000);
	BspUartWrite(2,(INT8U*)chars,strlen(chars));OSTimeDly(1);
}

/*******************************************************************************
���ƣ�void Print_Config(INT8U cmd)
���ܣ���485��ӡLTE�������������ýṹ��&��վIP��ַ���˿ںźͿ������ýṹ��
��Σ�INT8U cmd��0���رմ�ӡ��1��������ӡ
���Σ���
���أ���
*******************************************************************************/	
void Print_Config(INT8U cmd)
{
	TCHAR	temp[200]={0},version[4]={0};
	
	if(!cmd) return;															//����ӡ�رգ�����
	
	version[3] = VERSION & 0xFF;
	version[2] = (VERSION>>8) & 0xFF;
	version[1] = (VERSION>>16) & 0xFF;
	version[0] = (VERSION>>24) & 0xFF;											//�汾�ź궨�壬��ħ����		
	sprintf(temp, "\r\n---------->>��ӡ�豸��Ϣ<<----------\r\n");
	sprintf(temp+strlen(temp), "ϵͳ�汾��%c%c.%c.%c\r\n\r\n",version[0],version[1],version[2],version[3]);
	
	/*��ӡ����ʱ��*/
	RtcGetChinaStdTimeStruct(&gRtcTime);										//��ʱ��оƬȡ��RTCʱ��
	sprintf(temp+strlen(temp), "����ʱ�䣺20%X��%X��%X�� %02X:%02X:%02X\r\n",gRtcTime.Year,gRtcTime.Month,gRtcTime.Day,gRtcTime.Hour,gRtcTime.Minute,gRtcTime.Second);
	BspUartWrite(2,(INT8U*)temp,strlen(temp));

	/*��ӡװ�ú���Device_Number*/
	sprintf(temp, "\r\n");
	sprintf(temp+strlen(temp), "װ�ú��룺%c%c%c%c%c%c\r\n",Device_Number[0],Device_Number[1],Device_Number[2],Device_Number[3],Device_Number[4],Device_Number[5]);
	BspUartWrite(2,(INT8U*)temp,strlen(temp));

	/*��ӡ��վIP��ַ���˿ںźͿ������ýṹ��IP_Config*/
	sprintf(temp, "\r\n");
	sprintf(temp+strlen(temp), "��վIP��%d.%d.%d.%d	",IP_Config.IP_addr_1[0],IP_Config.IP_addr_1[1],IP_Config.IP_addr_1[2],IP_Config.IP_addr_1[3]);
	sprintf(temp+strlen(temp), "�˿ںţ�%d	",(IP_Config.PortNum_1[0]<<8)+IP_Config.PortNum_1[1]);
	sprintf(temp+strlen(temp), "APN��%s\r\n", APN);
	sprintf(temp+strlen(temp), "��վ���ţ�%X%X%X%X%X%X\r\n",IP_Config.CardNum_1[0],IP_Config.CardNum_1[1],IP_Config.CardNum_1[2],IP_Config.CardNum_1[3],IP_Config.CardNum_1[4],IP_Config.CardNum_1[5]);
	BspUartWrite(2,(INT8U*)temp,strlen(temp));	
	
	/*��ӡ�������ýṹ��Config*/
	sprintf(temp, "\r\n");
	sprintf(temp+strlen(temp), "װ�����룺%c%c%c%c\r\n",Config.Password[0],Config.Password[1],Config.Password[2],Config.Password[3]);
	sprintf(temp+strlen(temp), "���������%d����\r\n",Config.BeatTime[0]);
	sprintf(temp+strlen(temp), "�ɼ������%d����\r\n",(Config.ScanInterval[0]<<8)+Config.ScanInterval[1]);
	sprintf(temp+strlen(temp), "����ʱ����%d����\r\n",(Config.SleepTime[0]<<8)+Config.SleepTime[1]);
	sprintf(temp+strlen(temp), "����ʱ����%d����\r\n",(Config.OnlineTime[0]<<8)+Config.OnlineTime[1]);
	sprintf(temp+strlen(temp), "����ʱ�䣺%d��%dʱ%d��(0�ձ�ʾÿ��)\r\n",Config.ResetTime[0],Config.ResetTime[1],Config.ResetTime[2]);
	sprintf(temp+strlen(temp), "������֤��%c%c%c%c\r\n",Config.SecurityCode[0],Config.SecurityCode[1],Config.SecurityCode[2],Config.SecurityCode[3]);
	BspUartWrite(2,(INT8U*)temp,strlen(temp));
	
	/*��ӡ�������ò���FUN_Config*/
	sprintf(temp, "\r\n");
	sprintf(temp+strlen(temp), "���ù��ܣ�%XH,%XH",FUN_Config[0],FUN_Config[1]);
	BspUartWrite(2,(INT8U*)temp,strlen(temp));
	
	/*��ӡ����ͳ����ϢLocal_FLow_Data*/
	sprintf(temp, "\r\n\r\n");
	sprintf(temp+strlen(temp), "ÿ���ײ�������%d KB\r\n",MONTHLY_FLOW<<10);
	sprintf(temp+strlen(temp), "��������������%d KB\r\n",Local_FLow_Data.Flow_Day_Used_B/1024);
	sprintf(temp+strlen(temp), "��������������%d KB\r\n",Local_FLow_Data.Flow_Month_Used_B/1024);
	sprintf(temp+strlen(temp), "����ʣ��������%d KB\r\n\r\n",(MONTHLY_FLOW<<10)-Local_FLow_Data.Flow_Month_Used_B/1024);
	BspUartWrite(2,(INT8U*)temp,strlen(temp));
	
	/*��ӡ��ѹ��Ϣ*/
	Get_Voltage_MCUtemp_Data( 3 );											//��ȡ��ص�ѹ���ݺ͵�Ƭ���¶�
	OSTimeDly(1);
}

/*******************************************************************************
���ƣ�void DrawSysLogo(void)
���ܣ���485��ӡ BTC logo
��Σ���
���Σ���
���أ���
*******************************************************************************/
void DrawSysLogo(void)
{
	TCHAR	temp[500]={0};
	
	if(SYS==0)	/*���ݺ궨��SYS�ж�*/
	{
		sprintf(temp, 			   "                   / .]]]OOOOOO]]].\r\n");                     
		sprintf(temp+strlen(temp), "                ]OOOOOOOOOOOOOOOOOOOOOO]\r\n");
		sprintf(temp+strlen(temp), "            ,/OOOOOOOOOOOOOOOOOOOOOOOOOOOO\\`\r\n");
		sprintf(temp+strlen(temp), "          /OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\\\r\n");          
		sprintf(temp+strlen(temp), "       ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO`\r\n");       
		sprintf(temp+strlen(temp), "      /OOOOOOOOOOOOOOOOOOO^  [OOOOOOOOOOOOOOOOOOO\\\r\n");      
		sprintf(temp+strlen(temp), "    ,OOOOOOOOOOOOOOOOOOOOO   =OO   =OOOOOOOOOOOOOOO`\r\n");    
		sprintf(temp+strlen(temp), "   =OOOOOOOOOOOOOOO     [`  ,OO^  .OOOOOOOOOOOOOOOOO^\r\n"); 
		BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(2);
		sprintf(temp,			   "  =OOOOOOOOOOOOOOO\\.              =OOOOOOOOOOOOOOOOOO^\r\n");  
		sprintf(temp+strlen(temp), " ,OOOOOOOOOOOOOOOOOOOO.              ,OOOOOOOOOOOOOOOO`\r\n"); 
		sprintf(temp+strlen(temp), " OOOOOOOOOOOOOOOOOOOO/     =OOO\\`      ,OOOOOOOOOOOOOOO\r\n");  
		sprintf(temp+strlen(temp), "=OOOOOOOOOOOOOOOOOOOO`     OOOOOOO`     =OOOOOOOOOOOOOO^\r\n"); 
		sprintf(temp+strlen(temp), "=OOOOOOOOOOOOOOOOOOO/     =OOOOOOO      /OOOOOOOOOOOOOO^\r\n"); 
		sprintf(temp+strlen(temp), "OOOOOOOOOOOOOOOOOOOO`                  =OOOOOOOOOOOOOOOO\r\n"); 
		BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(2);
		sprintf(temp, 			   "OOOOOOOOOOOOOOOOOOO/     ,].         /OOOOOOOOOOOOOOOOOO\r\n"); 
		sprintf(temp+strlen(temp), "OOOOOOOOOOOOOOOOOOO`     OOOOOO\\      ,OOOOOOOOOOOOOOOOO\r\n"); 
		sprintf(temp+strlen(temp), "=OOOOOOOOOOOOOOOOO/     =OOOOOOOO`     ,OOOOOOOOOOOOOOO^\r\n"); 
		sprintf(temp+strlen(temp), ".OOOOOOOOOOOOO^         OOOOOOOOO      =OOOOOOOOOOOOOOO.\r\n"); 
		sprintf(temp+strlen(temp), " =OOOOOOOOOOO\\`                        OOOOOOOOOOOOOOO^\r\n");  
		sprintf(temp+strlen(temp), "  OOOOOOOOOOOOOOOOO^                 ,OOOOOOOOOOOOOOOO\r\n");   
		sprintf(temp+strlen(temp), "   OOOOOOOOOOOOOOOO   =OO   =000000OOOOOOOOOOOOOOOOOO\r\n");
		BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(2);	    
		sprintf(temp,			   "    \\OOOOOOOOOOOOO^   OO^   OOOOOOOOOOOOOOOOOOOOOOO/\r\n");
		sprintf(temp+strlen(temp), "     =OOOOOOOOOOOOOOOOOO]. =OOOOOOOOOOOOOOOOOOOOOO^\r\n");      
		sprintf(temp+strlen(temp), "       \\OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO/\r\n");        
		sprintf(temp+strlen(temp), "        ,\\OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO/`\r\n");         
		sprintf(temp+strlen(temp), "           \\OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO[\r\n");            
		sprintf(temp+strlen(temp), "              [OOOOOOOOOOOOOOOOOOOOOOOOOO[\r\n");               
		sprintf(temp+strlen(temp), "                  ,[OOOOOOOOOOOOOOOO[`\r\n");     
		BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(10);					//��ʱҪ����������쳣
	}
	else if(SYS==1)
	{	
		sprintf(temp, 			   "   SSSSSSSSSSSSSSS YYYYYYY       YYYYYYY   SSSSSSSSSSSSSSS   1111111\r\n");   
		sprintf(temp+strlen(temp), " SS:::::::::::::::SY:::::Y       Y:::::Y SS:::::::::::::::S 1::::::1\r\n");   
		sprintf(temp+strlen(temp), "S:::::SSSSSS::::::SY:::::Y       Y:::::YS:::::SSSSSS::::::S1:::::::1\r\n");   
		sprintf(temp+strlen(temp), "S:::::S     SSSSSSSY::::::Y     Y::::::YS:::::S     SSSSSSS111:::::1\r\n");   
		BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(2);
		sprintf(temp, 			   "S:::::S            YYY:::::Y   Y:::::YYYS:::::S               1::::1\r\n");   
		sprintf(temp+strlen(temp), "S:::::S               Y:::::Y Y:::::Y   S:::::S               1::::1\r\n");   
		sprintf(temp+strlen(temp), " S::::SSSS             Y:::::Y:::::Y     S::::SSSS            1::::1\r\n");   
		sprintf(temp+strlen(temp), "  SS::::::SSSSS         Y:::::::::Y       SS::::::SSSSS       1::::l\r\n");   
		BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(2);
		sprintf(temp, 			   "    SSS::::::::SS        Y:::::::Y          SSS::::::::SS     1::::l\r\n");   
		sprintf(temp+strlen(temp), "       SSSSSS::::S        Y:::::Y              SSSSSS::::S    1::::l\r\n");   
		sprintf(temp+strlen(temp), "            S:::::S       Y:::::Y                   S:::::S   1::::l\r\n");   
		sprintf(temp+strlen(temp), "            S:::::S       Y:::::Y                   S:::::S   1::::l\r\n");
		BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(2);
		sprintf(temp, 			   "SSSSSSS     S:::::S       Y:::::Y       SSSSSSS     S:::::S111::::::111\r\n");
		sprintf(temp+strlen(temp), "S::::::SSSSSS:::::S    YYYY:::::YYYY    S::::::SSSSSS:::::S1::::::::::1\r\n");
		sprintf(temp+strlen(temp), "S:::::::::::::::SS     Y:::::::::::Y    S:::::::::::::::SS 1::::::::::1\r\n");
		sprintf(temp+strlen(temp), " SSSSSSSSSSSSSSS       YYYYYYYYYYYYY     SSSSSSSSSSSSSSS   111111111111\r\n");
		BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(10);					//��ʱҪ����������쳣
	}
	
	sprintf(temp,"\r\n����ʱ�䣺%s %s\r\n",__DATE__,__TIME__);
	BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(1);						
}

/*******************************************************************************
���ƣ�void DrawErrLogo(void)
���ܣ���485��ӡ err logo
��Σ���
���Σ���
���أ���
*******************************************************************************/
void DrawErrLogo(void)
{
	char	temp[500]={0};

	sprintf(temp, 			   "\r\n");
	sprintf(temp+strlen(temp), "                        *,] *                                   *,] *                         ]`* \r\n");                         
	sprintf(temp+strlen(temp), "                        *=@@@@]  *                   ,/@`        @@@@@]`  *          *  ,]@@@@@^* \r\n");                         
	sprintf(temp+strlen(temp), "                          =@@@@@@@@@@]]]]]]]]]]/@@@@@@@@^       *,@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \r\n");                           
	sprintf(temp+strlen(temp), "                          **\\@@@@@@@@@@@@@@@@@@@@@@@@@@/         *,@@@@@@@@@@@@@@@@@@@@@@@@@@` \r\n");                            
	sprintf(temp+strlen(temp), "                            *=@@@@@@@@@@@@@@@@@@@@@@@[ *           */@@@@@@@@@@@@@@@@@@@@/` *\r\n");
	BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(1);
	sprintf(temp,			   "                             @@@@@@@@@@@@@@@@@@[[  *                @@@@@@@@@@@@@@@[[  * \r\n");                                  
	sprintf(temp+strlen(temp), "                             @@@@@@@@@@@@@@@                        @@@@@@@@@@@@@@^\r\n");                                        
	sprintf(temp+strlen(temp), "                             @@@@@@@@@@@@@@`*                      *=@@@@@@@@@@@@@ \r\n");                                        
	sprintf(temp+strlen(temp), "                             *\\@@@@@@@@@@@**                        *,@@@@@@@@@@/\r\n");                                          
	sprintf(temp+strlen(temp), "                                ,\\@@@@/`**                            *,\\@@@@@[ *\r\n"); 
	BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(5);
	sprintf(temp, 			   "\r\n");                                                      
	sprintf(temp+strlen(temp), "\r\n"); 
	sprintf(temp+strlen(temp), "\r\n");
	sprintf(temp+strlen(temp), "\r\n");
	sprintf(temp+strlen(temp), "\r\n");
	sprintf(temp+strlen(temp), "\r\n");
	sprintf(temp+strlen(temp), "\r\n");
	sprintf(temp+strlen(temp), "                                                        * ,]]]]\r\n");                                                            
	sprintf(temp+strlen(temp), "                                                      ,/@@@@@@@@@@`\r\n");                                                        
	sprintf(temp+strlen(temp), "                                                  **/@@@@@@@@@@@@@@@\\\r\n");                                                      
	sprintf(temp+strlen(temp), "                                                  /@@@@@[[[    ,[[@@@@\r\n");                                                     
	sprintf(temp+strlen(temp), "                                                 ,[[ *               * \r\n");
	sprintf(temp+strlen(temp), "\r\n");
	BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(10);					//��ʱҪ����������쳣
	
//	sprintf(temp,"\r\n����ʱ�䣺%s %s\r\n",__DATE__,__TIME__);
//	BspUartWrite(2,(INT8U*)temp,strlen(temp));OSTimeDly(1);						
}

/*******************************************************************************
���ƣ�void HardwareTest(void)
���ܣ����ڲ��»�վ��΢�����վ�ȵ�����Ӳ�����ԡ�
��Σ���
���Σ���
���أ���
*******************************************************************************/
void HardwareTest(void)
{
	INT8U		i;
	INT16U		DS;//DS18B20
	INT8U		Temp[10];
	INT8U*		pointer;
	struct HARDWARE_TEST	hardware = {0};										//��Ÿ�Ӳ���������1��ʾͨ����0Ϊ�쳣
	
	Led_Init();
 
	while(1)
	{
	/*485���ԣ���B485_init�з��ؽ��*/
		hardware.b485 = 1;														//Ĭ��Ϊ1��������Ļ���Ȼ�򲻳�������
		BspUartWrite(2,SIZE_OF("\r\n485_Test    ------------------->485_OK\r\n"));OSTimeDly(1);

#if 1  //ͨ�ò��ԣ������磬WQ256��ʱ�ӣ���Դ��ѹ����Ƭ���¶Ȳ��ԣ�
	/*�������*/
		BspUartWrite(2,SIZE_OF("FM_Test     "));OSTimeDly(1);
		if(FM_test()==1)
		{
			hardware.ferroelectric_ram = 1;
			BspUartWrite(2,SIZE_OF("------------------->FM_OK\r\n"));
		}
		else 
			BspUartWrite(2,SIZE_OF("------------------->FM_ERR\r\n"));
		OSTimeDly(1);	
		
	/* WQ256����*/
		BspUartWrite(2,SIZE_OF("WQ256_Test  "));OSTimeDly(1);
		if( WQ256_Test(3)==1)
		{
			hardware.flash_memory = 1;
			BspUartWrite(2,SIZE_OF("------------------->WQ256_OK\r\n"));
		}
		else
			BspUartWrite(2,SIZE_OF("------------------->WQ256_ERR\r\n"));		
		OSTimeDly(1);

	/*ʱ�Ӳ���*/	
		BspUartWrite(2,SIZE_OF("RTC_Test    -------------------\r\n"));OSTimeDly(1);			
		hardware.rtc = RTCTaskTest();
		BspUartWrite(2,SIZE_OF("\r\n"));

	/*��ص�ѹ����Ƭ���¶Ȳ���*/
		BspUartWrite(2,SIZE_OF("VCC_Test    -------------------\r\n"));OSTimeDly(1);
		Get_Voltage_MCUtemp_Data(3);
		BspUartWrite(2,SIZE_OF("\r\n"));
		if(Equipment_state.BAT_Volt>BAT_UP || Equipment_state.FALA_Volt>FALA_UP)//���յ㣬������
		{
			hardware.power_supply = 1;
		}
#endif
#if 1 //1����վ���²��ԣ��������¶ȣ���Ƶ����ģ����ԣ�   0����վ΢������ԣ������ٷ�����ʪ�ȣ�����ѹ���ԣ�    //PS���������18B20����AM2302����������΢��������У��ᵼ��ϵͳ������
	/*DS18B20����*/
		BspUartWrite(2,SIZE_OF("DS18B20_Test"));								//DS18B20
		OSTimeDly(1);
		if(Get_DS18B20Temp(&DS))
		{
			BspUartWrite(2,SIZE_OF("------------------->DS18B20_OK\r\n"));
			hardware.ds18b20 = 1;
			Temp[0]=(DS/100+0x30);//ȡ��λ
			Temp[1]=((DS/10%10)+0x30);//ȡ��λ
			Temp[2]='.';
			Temp[3]=((DS%10)+0x30);//С��
			BspUartWrite(2,SIZE_OF("Temperature = "));
			BspUartWrite(2,Temp,4);
			BspUartWrite(2,SIZE_OF("��\r\n"));
		}
		else
		{
			BspUartWrite(2,SIZE_OF("------------------->DS18B20_Err\r\n"));
		}
		OSTimeDly(1);

	/*��Ƶ����*/
		RF_Power_Init();       													//RFģ���Դ��ʼ��
		PWRFEN();				 												//�򿪵�Դ��������
		RF_Uart_init(1200);														//��ʼ��RF����
		BspUartWrite(2,SIZE_OF("\r\n"));
		BspUartWrite(2,SIZE_OF("RF_Test     -------------------\r\n"));OSTimeDly(1);
		if(RfModuleTest())
		{	
			hardware.lora = 1;
			BspUartWrite(2,SIZE_OF("------------------------------->RF_OK\r\n"));
		}
		else
		{
			BspUartWrite(2,SIZE_OF("------------------------------->RF_Err\r\n"));
		}
		OSTimeDly(1);
#else
	/*΢�������*/
		PowerMETPin_Init();														//��ʪ�ȴ�������ʼ��
		PWMETEN();																//����ʪ�ȴ�������Դ 
		hardware.meteorology = !Test_Meteorology_Data(1);    					//����0��ʾ�޴���
#endif		

	/*��Ϊģ�����*/
		BspUartWrite(2,SIZE_OF("LTE_Test    "));
		OSTimeDly(1);
		if(ME909S_TEST())
		{
			hardware.lte = 1;
			BspUartWrite(2,SIZE_OF("LTE_Test    ------------------->LTE_OK\r\n"));
			OSTimeDly(1);	
		}
		else
		{
			BspUartWrite(2,SIZE_OF("LTE_Test    ------------------->LTE_ERR\r\n"));
			OSTimeDly(1);	
		}
		
#if 0		
	/*���ܲ���*/
		BspUartWrite(2,SIZE_OF("JM_Test     "));OSTimeDly(1);
		
		if(NAREC300_Test()==1)
		{
			hardware.encryption_chip = 1;
			BspUartWrite(2,SIZE_OF("------------------->JM_Test_OK\r\n"));
		}
		else
		{
			BspUartWrite(2,SIZE_OF("------------------->JM_Test_ERR\r\n"));
		}
		OSTimeDly(1);
#endif

	/*����Ӳ��ȫ������ͨ��ʱ���Ž��п��Ź�����*/
		if(hardware.b485 & hardware.ferroelectric_ram & hardware.flash_memory & hardware.rtc & hardware.power_supply & hardware.ds18b20 & hardware.lora & hardware.lte)
		{
		/*���Ź�����*/
			BspUartWrite(2,SIZE_OF("WDG_Test\r\n"));
			BspUartWrite(2,SIZE_OF("if Systerm restart,Test is OK\r\n"));		//������������ͨ������������

			PWR->CR |= 1<<8;													//DBPλ��ȡ���������д������1������д��RTC�ͺ󱸼Ĵ���
			BKP->DR2 = 0xCC;													//���ֵΪ0xCC��10����Ź�����δ��λ����Ϊ0���ٴ��ϵ绹��ϵͳ2���������Գ���
			PWR->CR &= ~(1<<8);													//���ú������д����
			WDG_En = false;														//��ֹinit_task_core()���ⲿ���Ź�ι��
			BSP_WDGDeInit();													//�����ⲿ���Ź�ι�����������Ǹ����У�ԭ��δ֪��
			
			while(1)
			{
				OSTimeDly(10*20);												//��ʱ10�롾���������޸ġ�
				BspUartWrite(2,SIZE_OF("������λ�����ⲿ���Ź��쳣\r\n"));
				
				PWR->CR |= 1<<8;												//DBPλ��ȡ���������д������1������д��RTC�ͺ󱸼Ĵ���			
				BKP->DR2 = 0;													//��Ϊ0���´μ�������Ӳ������ϵͳ����10���ڸ�λ������ͨ�����Ժ󶼽�ϵͳ1��
				PWR->CR &= ~(1<<8);												//���ú������д���������Ź���λ���Զ�������
			}			
		}
		
	/*����Ӳ�����Բ�ͨ������ӡ�����*/
		DrawErrLogo();
		
		pointer = (INT8U*)&hardware;
		for(i=0;i<10;i++)														//Ŀǰ�����8��ģ��
		{
			if(*(pointer+i)==0)													//Ϊ0����ʾ����δͨ��
			{
				switch(i)
				{
					case 0:
						BspUartWrite(2,SIZE_OF("���ϸ�485����\r\n\r\n"));
						break;
					case 1:
//						BspUartWrite(2,SIZE_OF("���ϸ񣺼��ܹ���\r\n\r\n"));
						break;
					case 2:
						BspUartWrite(2,SIZE_OF("���ϸ�����洢����\r\n\r\n"));
						break;
					case 3:
						BspUartWrite(2,SIZE_OF("���ϸ��ⲿFLASH����\r\n\r\n"));
						break;
					case 4:
						BspUartWrite(2,SIZE_OF("���ϸ�RTC����\r\n\r\n"));
						break;
					case 5:
						BspUartWrite(2,SIZE_OF("���ϸ񣺵�Դ��ѹ����\r\n\r\n"));
						break;
					case 6:
						BspUartWrite(2,SIZE_OF("���ϸ�DS18B20����\r\n\r\n"));
						break;
					case 7:
						BspUartWrite(2,SIZE_OF("���ϸ�LORA��Ƶ���չ���\r\n\r\n"));
						break;
					case 8:
//						BspUartWrite(2,SIZE_OF("���ϸ�΢������\r\n\r\n"));
						break;
					case 9:
						BspUartWrite(2,SIZE_OF("���ϸ�LTE����\r\n\r\n"));
						break;
					default:	
						BspUartWrite(2,SIZE_OF("�����쳣��\r\n"));
						break;
				}
				OSTimeDly(1);
			}				
		}
		while(1) OSTimeDly(3*20);
	}
}



#if 1 /*============================================================FATFSӦ�ú���============================================================*/
/*******************************************************************************
���ƣ�FRESULT scan_files (char* path)
���ܣ�ɨ���ļ��У���ӡ�����ļ��С����ļ��С��ļ�����
��Σ�char* path��Ҫɨ���·��
���Σ���
���أ�FRESULT
*******************************************************************************/
FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;


    res = f_opendir(&dir, path);                       							/* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                  						/* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  						/* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    						/* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
				/*��ӡĿ¼��*/
				BspUartWrite(2,(INT8U*)path,strlen(path));
//				BspUartWrite(2,(INT8U*)fno.fname,strlen(fno.fname));
				BspUartWrite(2,SIZE_OF("\r\n"));
                res = scan_files(path);                    						/* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       						/* It is a file. */
				/*��ӡ�ļ���*/
//				printf("%s/%s\n", path, fno.fname);
				BspUartWrite(2,(INT8U*)path,strlen(path));
				BspUartWrite(2,SIZE_OF("/"));
				BspUartWrite(2,(INT8U*)fno.fname,strlen(fno.fname));
				BspUartWrite(2,SIZE_OF("\r\n"));
            }
			OSTimeDly(1);
        }
        f_closedir(&dir);
    }

    return res;
}

/*******************************************************************************
���ƣ�FRESULT Dir_Maintenance (void)
���ܣ�Ŀ¼ά�����̶�����SUB1~SUB31��31���ļ��С�
��Σ���
���Σ���
���أ�FRESULT
*******************************************************************************/
FRESULT Dir_Maintenance (void)
{
	static FILINFO fno;
	static FRESULT res;															/* API result code */
	UINT i,fnlen=0;
	DIR dir;
	char path[256];
	
	BspUartWrite(2,SIZE_OF("\r\n->��ʼĿ¼ά��<-\r\n"));OSTimeDly(1);
	res = f_mount(&fs, "", 1);	/* Mode option 0:Do not mount (delayed mount), 1:Mount immediately */
	if (res == FR_OK) res = f_opendir(&dir, "/");                       		//�����Ŀ¼
	else if(!Fault_Manage.F_STORAGE)											//����ʧ��
	{
		NW_Fault_Manage(STORAGE_F, FAULT_STA);									//�洢���Ϸ���ʱ��������
		BspUartWrite(2,SIZE_OF("����ʧ�ܣ�<-\r\n"));OSTimeDly(1);
		return res;
	}
	if(Fault_Manage.F_STORAGE) NW_Fault_Manage(STORAGE_F, NOFAULT_STA);			//�������洢���ϣ��ϱ����ϻָ�
    if (res == FR_OK) 
	{
	/*Ŀ¼ά��*/
		for(i=1;i<32;i++)														//����31��Ŀ¼
		{	
		/*����Ŀ¼��*/
			sprintf(path, "/SUB%d", i);											//����Ŀ¼����*path
			fnlen = strlen(path)-1;												//Ŀ¼���ĳ��ȣ�-1ָ��/��
			
		/*�ж�Ŀ¼�Ƿ����*/
			res = f_readdir(&dir, &fno);                   						/* Read a directory item */
			if (res != FR_OK ) 				  									/* Break on error */
			{
				BspUartWrite(2,SIZE_OF("f_readdir()��ȡĿ¼ʧ�ܣ�ά����ֹ\r\n"));OSTimeDly(1);
				return res;
			}
			else if (fno.fname[0] == 0) 				  						/* End of dir */
			{
			/*Ŀ¼�����ڣ�������Ŀ¼*/
				res = f_mkdir(path);											//����Ŀ¼
				BspUartWrite(2,SIZE_OF("������Ŀ¼��"));
				BspUartWrite(2,(INT8U*)path+1,fnlen);							/*��ӡĿ¼��*/				
				if (res != FR_OK) BspUartWrite(2,SIZE_OF("ʧ��\r\n"));
				else BspUartWrite(2,SIZE_OF("�ɹ�\r\n"));
				OSTimeDly(1);
			}
			else if (fno.fattrib & AM_DIR)                   					/* It is a directory */
			{  
			/*�ҵ�һ��Ŀ¼���ж�Ŀ¼�Ƿ�Ϊ���貢����*/
				if(memcmp(&fno.fname,path+1,fnlen)) 							//��Ŀ¼��ΪSUBi
				{
				/*Ŀ¼���ԣ�ɾ�����½�Ŀ¼*/
					f_unlink(fno.fname);										//�Ƴ���ǰĿ¼
					BspUartWrite(2,SIZE_OF("������Ŀ¼��"));
					BspUartWrite(2,(INT8U*)path+1,fnlen);						/*��ӡĿ¼��*/				
					res = f_mkdir(path);										//����Ŀ¼
					if(res==FR_EXIST) BspUartWrite(2,SIZE_OF("�Ѵ��ڣ�������Ŀ¼����ˣ�\r\n"));
					else if(res != FR_OK) BspUartWrite(2,SIZE_OF("ʧ��\r\n"));
					else  BspUartWrite(2,SIZE_OF("�ɹ�\r\n"));
				} else 															
				{	
				/*Ŀ¼�Ѵ���*/
					BspUartWrite(2,SIZE_OF("��Ŀ¼��"));
					BspUartWrite(2,(INT8U*)fno.fname,strlen(fno.fname));		/*��ӡĿ¼��*/
					BspUartWrite(2,SIZE_OF("�Ѵ���\r\n"));
				}
				OSTimeDly(1);
			} else i--;															//���ļ�������
		}
		f_closedir(&dir);
	}
	/* Unregister work area */
    f_mount(0, "", 0);
	BspUartWrite(2,SIZE_OF("->Ŀ¼ά������<-\r\n\r\n"));OSTimeDly(1);
	return res;
}

/*******************************************************************************
���ƣ�void Dir_Test(void)
���ܣ�Ŀ¼���ԡ�
��Σ���
���Σ���
���أ���
*******************************************************************************/
void Dir_Test(void)
{
	TCHAR	chars[256];
	FRESULT res;																/* API result code */
	
    res = f_mount(&fs, "", 1);	/* Mode option 0:Do not mount (delayed mount), 1:Mount immediately */
//    if (res == FR_OK) {
//		/*����Ŀ¼*/
//		res = f_mkdir("sub1");
//		if (res) {BspUartWrite(2,SIZE_OF("����sub1��Ŀ¼ʧ��\r\n"));OSTimeDly(1);}
//		res = f_mkdir("sub1/sub2");
//		if (res) {BspUartWrite(2,SIZE_OF("����sub1/sub2��Ŀ¼ʧ��\r\n"));OSTimeDly(1);}
//		res = f_mkdir("sub1/sub2/sub3");
//		if (res) {BspUartWrite(2,SIZE_OF("����sub1/sub2/sub3��Ŀ¼ʧ��\r\n"));OSTimeDly(1);}
//		if (res == FR_OK) {
//			/*��"/sub1/sub2/sub3"Ŀ¼�²����ļ�*/
//			strcpy(chars, "/sub1/sub2/sub3");
//			res = scan_files(chars);
//		 }
//	 }

    if (res == FR_OK) {		
		/*��"/"Ŀ¼�²����ļ�*/
        strcpy(chars, "");
        res = scan_files(chars);
    }
	
	/* Unregister work area */
    f_mount(0, "", 0);
	
	BspUartWrite(2,SIZE_OF("Dir_Test()���Խ���\r\n"));OSTimeDly(1);
}

/*******************************************************************************
���ƣ�void Check_Getfree(void)
���ܣ�����Ƿ���Ҫ��ʽ����ִ�С��������豸д��Config��Unreport_Index����Ĭ�ϲ�����
���ⲿFLASHδ��ʽ�����豸�϶�Ϊ���豸��
��Σ���
���Σ���
���أ���
*******************************************************************************/
void Check_Getfree(void)
{
	FRESULT				res;																/* API result code */
	TCHAR				chars[256];
	static DWORD		fre_clust, fre_sect, tot_sect;
	INT32U				id = 0;
	
    /* Register work area */
    res = f_mount(&fs, "", 0);
	if (res) BspUartWrite(2,SIZE_OF("f_open() ����ʧ�ܣ�\r\n"));
	
    /* Get volume information and free clusters of drive 1 */
    res = f_getfree("", &fre_clust, &fs0);

    if (res)	//res
	{
		BspUartWrite(2,SIZE_OF("f_getfree() �������ڸ�ʽ���ⲿFLASH����\r\n"));
		/* Create FAT volume */
		res = f_mkfs(	"", 													// If it has no drive number in it, it means the default drive.
						FM_FAT|FM_SFD, 											// Specifies the format option in combination of FM_FAT, FM_FAT32, FM_EXFAT and bitwise-or of these three, FM_ANY. If two or more types are specified, one out of them will be selected depends on the volume size and au.
						4096, 													// The valid value is n times the sector size.
						work, 													// Pointer to the working buffer used for the format process
						sizeof work);											// It needs to be the sector size of the corresponding physical drive at least.
		if (res) BspUartWrite(2,SIZE_OF("f_mkfs() ��ʽ������\r\n"));
		else BspUartWrite(2,SIZE_OF("f_mkfs() ��ʽ���ɹ���\r\n"));
		
//		/*����ԭ��ϵͳ�������á������粻�ܲ�����Ҳ���������*/
//		BSP_FM_Erase(FM_Start_Addr,0x400*8);									//�����������索��
//		BspUartWrite(2,SIZE_OF("��������ɹ���\r\n"));
		
		/*��ȡ������Ϊװ�ú���*/
		Read_NFlash(Device_Number_Flash_Addr, &id, 1);							//��ȡ���룬��Ϊ��ʼװ�ú���
		NW_DeviceNumberToAscii(id, &Device_Number[2]);							//��16����תΪASCII������Device_Number[2]
		BSP_WriteDataToFm(Device_Number_Addr,Device_Number,Device_Number_Len);	//װ�ú���д������
		
		/*���豸д��Config��Unreport_Index����Ĭ�ϲ���*/
		BSP_WriteDataToFm(Config_Addr,(u8*)&Config,Config_Len);					//дһ��Ĭ�����ã�������λ�������޸�д������֮��ÿ���ϵ�������Ĳ�����ȷ�ģ���Ȼ�״ζ���ȫ��0
		BSP_WriteDataToFm(IP_Config_Addr,(u8*)&IP_Config,IP_Config_Len);		//дһ���Զ����IPĬ������
		BSP_WriteDataToFm(APN_Addr, APN, APN_Len);								//дһ���Զ����APNĬ������
		BSP_WriteDataToFm(FUN_Config_Addr,FUN_Config,FUN_Config_Len);			//дĬ�Ϲ������ò���
		memset(Unreport_Index,0xFF,Unreport_Index_Len);							//дFF
		BSP_WriteDataToFm(Unreport_Index_Addr,(u8*)Unreport_Index,Unreport_Index_Len);	//δ�ϱ�����������ȫд�����ϱ�������λд1��
		BspUartWrite(2,SIZE_OF("����д�������óɹ���\r\n"));
	}
	
	/* Get total sectors and free sectors */
	tot_sect = (fs0->n_fatent - 2) * fs0->csize;
	fre_sect = fre_clust * fs0->csize;
	
	/* Print the free space (assuming 4K bytes/sector) */
    sprintf(chars,"�ⲿFLASH��%10lu KiB total drive space.\n%10lu KiB available.  ��%d%%USED��\r\n", tot_sect*4, fre_sect*4, 100*(tot_sect-fre_sect)/tot_sect);	
	BspUartWrite(2,(INT8U*)chars,strlen(chars));								//��ӡ
	
	/* Unregister work area */
    f_mount(0, "", 0);
}

/*******************************************************************************
���ƣ�void Dir_Test(void)
���ܣ��򿪹ر��ļ�����д�ļ����ԡ�
��Σ���
���Σ���
���أ���
*******************************************************************************/
void File_Test(void)
{
	FRESULT res;																/* API result code */
	TCHAR	read_buff[256];
	UINT bw;																	/* Bytes written */	

	res = f_mount(&fs, "", 0);
	
    /* Create a file as new */
    res = f_open(&fil, "hello.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE );
    if (res) BspUartWrite(2,SIZE_OF("f_open() ���ļ�����\r\n"));
	
    /* Write a message */
    f_write(&fil, "Hello, World!\r\n", 15, &bw);
    if (bw != 15) BspUartWrite(2,SIZE_OF("f_write() д�ļ�����\r\n"));
	
	/* File read/write pointer */
	res = f_lseek(&fil, 0);
	
    /* Read a message */
    f_read(&fil, read_buff, 100, &bw);
    if (bw != 15) BspUartWrite(2,SIZE_OF("f_read() ���ļ�����\r\n"));	

    /* Close the file */
    f_close(&fil);

    /* Unregister work area */
    f_mount(0, "", 0);
	
	BspUartWrite(2,SIZE_OF("File_Test()���Խ���\r\n"));OSTimeDly(1);
}

#endif
/*============================================================FATFSӦ�ú��� end============================================================*/
