#ifndef __LTE_H
#define __LTE_H

#include "main.h"



/*�궨��*/
#define LTE_Task_Prio 			LTE_PRIO    									//LTE�������ȼ�
#define LTE_Send_Event_Bit 		0x0001
#define LTE_Send_Holding_Bit	0x0002

#define RETRY				3													//3��
#define TIMEOUT				10*20												//10*20��ʱ��Ƭ����10��
#define LTE_BUFF_LEN		1500												//������4��ȫ�ֱ����Ŀռ�
#define DES_LEN				1024												//�ļ��ϱ�ʱ��֡���ȣ��ɰ���Ҫ����
#define MONTHLY_FLOW		30													//ÿ����������λ��M���ɰ�ʵ���ײ͸���
#define MF_0				(MONTHLY_FLOW>>24)&0xFF								//���ֽ�
#define MF_1				(MONTHLY_FLOW>>16)&0xFF	
#define MF_2				(MONTHLY_FLOW>>8)&0xFF	
#define MF_3				MONTHLY_FLOW&0xFF									//���ֽ�
																				


/*ȫ�ֱ�������*/
extern INT8U 				LTE_Buff[LTE_BUFF_LEN];								//������֡
extern INT8U 				LTE_Tx_Buff[LTE_BUFF_LEN];							//����LTE����
extern INT8U 				LTE_Rx_Buff[LTE_BUFF_LEN];							//����LTE����
extern INT32U				Net_Fault_Time;										//������������ʧ�ܼ�ʱ������ʱ��30����ʱ����һ���������Ӵ�������ϱ�����������ʱ��������������ӻָ��ϱ�
extern INT32U 				Host_No_Reply_Time;									//������վ��Ӧ���ʱ������ʱ��30����ʱ����һ����վ��Ӧ���������ϱ����ٴη���ѶϢ�õ��ظ�ʱ�������ϻָ��ϱ�

/*��������*/
void Task_LTE_Main(void *arg);
void Sleep_Or_Reset(u8 CMD);
#endif
