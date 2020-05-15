//Z.E. 20180525
#include "delay.h"

#if Time_test//������
#include "Bsp_AM2302.h"
//���������������ʱֻ���޸�WS2812_init()�����¼���
#define LED0_H 	(GPIOC->BSRR = 0x00000100) 			//PC8��1	���ٵض�GPIOA��λ6���з�ת	LED0��������	ռ��һ���������� Machine Cycle Time 0.03125us
#define LED0_L 	(GPIOC->BSRR = 0x01000000)			//PC8��0	���ٵض�GPIOA��λ6���з�ת	LED0��������	ռ��һ���������� Machine Cycle Time
//#define LED1_H 	(GPIOC->BSRR = 0x00000080) 		//PC7��1	���ٵض�GPIOA��λ7���з�ת	LED1��������
//#define LED1_L 	(GPIOC->BSRR = 0x00800000)		//PC7��0	���ٵض�GPIOA��λ7���з�ת	LED1��������
void WS2812_init(void)
{
	RCC->APB2ENR|=1<<4;  		//PORTCʱ��ʹ�� 	2~8�ֱ�ΪA~G  
	GPIOC->CRH&=0XFFFFFFF0; 
	GPIOC->CRH|=0X00000007;		//PC8(3���죬7��©�����
//	GPIOA->CRL&=0XFFFF00FF; 
//	GPIOA->CRL|=0X00003300;		//PA3/2(3���죬7��©�����		
}
void time_test(void)
{
	PowerMETPin_Init();			//��Դ�������ų�ʼ��
	PWMETEN();					//���������ĵ�Դ
	WS2812_init();				//�۲�����PC8�ĳ�ʼ��
	
#if	0	//����systick��ʱ��100usʵ������104us��100msʵ��100ms������׼��
	
	while(1)
	{
		LED0_H;				//ռ��һ���������� Machine Cycle Time��Լ0.03us
		delay_SYSus(50000);
		LED0_L;				//ռ��һ���������� Machine Cycle Time
		delay_SYSus(50000);
	}	

#elif	0	//����TIM3������ʱ����׼��
	
	delay_init();			//��ʼ����ʱ��TIM3
	while(1)
	{
		LED0_H;				//ռ��һ���������� Machine Cycle Time��Լ0.03us
		delay_50us(1);
		LED0_L;				//ռ��һ���������� Machine Cycle Time
		delay_50us(1);
	}
	
#elif 0		//����nopѭ��������ʱ
	while(1)
	{	
		#if 1
		LED0_H;				//ռ��һ���������� Machine Cycle Time��Լ0.03us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		//100us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		//100us
		//������200us��ʵ��300us
		
		LED0_L;				//ռ��һ���������� Machine Cycle Time
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us	
		//100us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);delay_1us_32M(1);	//10us
		//100us
		//������200us��ʵ��280us�������Ϊ1�ĵ��ε���Լ1.4us������1us
		#else
		LED0_H;				//ռ��һ���������� Machine Cycle Time��Լ0.03us
		delay_1us_32M(10);
		
		LED0_L;				//ռ��һ���������� Machine Cycle Time
		delay_1us_32M(10);	//ʵ������21.4us��7%ƫ��
		#endif
	}
#elif 0
	while(1)				//��ʾ�����۲첢��¼
	{	
		LED0_H;				//ռ��һ���������� Machine Cycle Time��Լ0.03us
		delay_1us_32M(1000);//1000us
		
		LED0_L;				//ռ��һ���������� Machine Cycle Time
		delay_1us_32M(1000);	
		
		LED0_H;				//ռ��һ���������� Machine Cycle Time��Լ0.03us
		delay_1us_32M(100);	//100.8us
		
		LED0_L;				//ռ��һ���������� Machine Cycle Time
		delay_1us_32M(100);	
		
		LED0_H;				//ռ��һ���������� Machine Cycle Time��Լ0.03us
		delay_1us_32M(50);	//50.6us
		
		LED0_L;				//ռ��һ���������� Machine Cycle Time
		delay_1us_32M(50);
		
		LED0_H;				//ռ��һ���������� Machine Cycle Time��Լ0.03us
		delay_1us_32M(10);	//10.74us
		
		LED0_L;				//ռ��һ���������� Machine Cycle Time
		delay_1us_32M(10);	
		
		LED0_H;				//ռ��һ���������� Machine Cycle Time��Լ0.03us
		delay_1us_32M(1);	//1.74us
		
		LED0_L;				//ռ��һ���������� Machine Cycle Time
		delay_1us_32M(1);	
	}
#elif 0
	while(1)				//��ʾ�����۲죬�պ�2us���� ��ÿ���������ڣ���25��nop+1���Ĵ�������ָ���26���������ڣ�
	{						//while(1)���ж�+ִ�й�2��ָ�����ڣ���2*6=12���������ڡ�26+12=32���������ڣ���Ӧ32Mʱ��Ƶ�ʸպ�1us��
		LED0_H;				//ռ��һ���������� Machine Cycle Time
		MachineCycle_25;
		LED0_L;				//ռ��һ���������� Machine Cycle Time
		MachineCycle_25;
	}
#elif 1
	while(1)
	{	
		#if 0
		LED0_H;				//ռ��һ���������� Machine Cycle Time��Լ0.03us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		//200us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		//200us
		//����400us��ʵ��1080us�������Ϊ1�ĵ��ε���Լ5.4us������2us
		
		LED0_L;				//ռ��һ���������� Machine Cycle Time
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		//200us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);delay_2us_4M(1);	//20us
		//200us
		//����400us��ʵ��1080us�������Ϊ1�ĵ��ε���Լ5.4us������2us
		#else
		LED0_H;				//ռ��һ���������� Machine Cycle Time
		delay_2us_4M(50);
		
		LED0_L;				//ռ��һ���������� Machine Cycle Time
		delay_2us_4M(50);	//(10)ʵ������51.6us��29%ƫ�(20)ʵ������91us��13.75%ƫ�(30)ʵ������132us��10%ƫ�(40)ʵ������172us��7.5%ƫ�(50)ʵ������212us��6%ƫ�
		#endif
	}
	#elif 0
	while(1)	//4M����nop����
	{			//while(1)���ж�+ִ�й�2��ָ�����ڣ���2*6=12���������ڣ�����LED0_H��LED0_L2���������ڣ���14����14/4=3.5us����ʵ�����ӽ�
		LED0_H;				//ռ��һ���������� Machine Cycle Time
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		//50us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		//50us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		//50us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		//50us
		//50*4=200us
		
		LED0_L;				//ռ��һ���������� Machine Cycle Time
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		//50us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		//50us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		//50us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;DELAY1US_4M;	//10us
		//50us
		//50*4=200us
	}
#endif
}
#endif


//------------------------------------------------------------����nop��ʱ�����и� DELAY1US_32M ���ã���ͷ�ļ���------------------------------------------------------------//
/**********************************************************************
������void delay_1us_32M(u32 us)
���ܣ��ɹ����õ�1us��ʱ��ֻ����1us�Ļ�����΢��׼����Ϊ�г���ջʱ�䣬����һ�κ�������1.49-1=0.49us
��100�ε���1us��ռ��150us��ÿ��0.5us�����ε����0.74��0.6��0.8����Լ15.68����������(15.68/32=0.49us)
���ڿɾ�ȷ��1us��ʱ���ʲ���ֹOS���ȣ���Ҫ�ڵ���ʱע�⣬�������ⲿ��ֹOS����
	2018/08/30���ԣ����Ϊ1�ĵ��ε���Լ1.4us�����Ϊ10���ε���Լ21.4us
��Σ���ʱ��΢����������( us > 10 )��7%ƫ��
���Σ���
***********************************************************************/
void delay_1us_32M(u32 us)
{
	while(us--)				//ʵ��ÿ��ѭ��(us*5)����������
	{
		MachineCycle_27;	//5+27=32���������ڣ�32Mʱ��Ƶ����Ϊ1us
	}
}

/**********************************************************************
������void delay_2us_4M(u32 us)------4M�޷�����1us��һ��whileѭ����1.25us��
���ܣ��ɹ����õ�2us��ʱ��
���ڿɾ�ȷ��1us��ʱ���ʲ���ֹOS���ȣ���Ҫ�ڵ���ʱע�⣬�������ⲿ��ֹOS����
	2018/08/30���ԣ����Ϊ1�ĵ��ε���Լ5.4us������2us��
��Σ���ʱ��΢����������( us > 50 )��6%ƫ��
���Σ���
***********************************************************************/
void delay_2us_4M(u32 us)
{
//	u32	i=0;
//	for(;i<us;i++)			//ʵ��ÿ��ѭ��(us*6)����������
//	{
//		MachineCycle_2;		//6+2���������ڣ�4Mʱ��Ƶ����Ϊ2us
//	}
	while(us--)				//ʵ��ÿ��ѭ��(us*5)����������
	{
		MachineCycle_3;		//5+3���������ڣ�4Mʱ��Ƶ����Ϊ2us
	}
}


//------------------------------------------------------------����TIM3��ʱ����STM32F407�ƹ�������û�޸ģ���׼��134us��------------------------------------------------------------//
/**********************************************************************
������void delay_init(void)
���ܣ�TIM3���г�ʼ����������ʱ
��Σ���
���Σ���
***********************************************************************/
void delay_init(void)
{
	TIM3_Int_Init(49,83);//10Khz����,50usһ���ж�
}

/**********************************************************************
������void delay_50us(u32 us)
���ܣ�����TIM3����50us��ʱ
��Σ���ʱ�ģ�50΢�룩��
���Σ���
***********************************************************************/
void delay_50us(u32 us)
{		
	TIM_Cmd(TIM3,ENABLE);  //ʹ�ܶ�ʱ��3
	while(us--)
	{
		while(TIM_GetFlagStatus(TIM3,TIM_IT_Update)!=SET); //�ȴ����
		TIM_ClearFlag(TIM3,TIM_IT_Update);                 //�����־λ
	}
	TIM_Cmd(TIM3,DISABLE); //ֹͣ��ʱ��3
}

/**********************************************************************
������void delay_ms(u32 ms)
���ܣ�����TIM3����1ms��ʱ
��Σ���ʱ�ĺ�����
���Σ���
***********************************************************************/
void delay_ms(u32 ms)
{	 	 
	while(ms--)
	{
		delay_50us(20);
	}
} 

/**********************************************************************
������void TIM3_Int_Init(u16 arr,u16 psc)
���ܣ�ͨ�ö�ʱ��3�жϳ�ʼ��
��Σ�u16 arr���Զ���װ��ֵ��u16 psc����ʱ����Ƶ
���Σ���
***********************************************************************/
void TIM3_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);           //ʹ��TIM3ʱ��
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;                  //��ʱ����Ƶ
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseInitStructure.TIM_Period=arr;                     //�Զ���װ��ֵ
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
}



//------------------------------------------------------------����systick��ʱ------------------------------------------------------------//
/*===========================================================================
������void delay_SYSus(u32 nus)
˵����us��ʱ������systickһֱ�ڵݼ�����ͣ��ȡֵ�Ƚϣ����Ƚ�ֵ�������ʱ���˳�               �����us�������ʱ��ָ��ִ��ʱ��ᳬ��10us�������޷�����10us���ڵ���ʱ������ʹ�ã���ϵͳʱ��Ƶ�ʹ��ߵ�ʱ��ſ�ʹ�á�
     ��Ҫע���ֹOS���ȴ����ʱ����ռ��CPU��OS�����ȴ���
��Σ�
���Σ�
============================================================================*/
void delay_SYSus(u32 nus)
{		
	u32 ticks=0;
	u32 told=0;
	u32 tnow=0;
	u32 tcnt=0;
	u32 reload=SysTick->LOAD;					//��ȡLOAD��ֵ	    	 
	ticks=nus*4;								//��Ҫ�Ľ�������nus��Ҫ��ticks����	  ϵͳ��ʱ��ʱ��systick=HCLK/8=4M��ÿus��Ҫ�Ľ�����Ϊ4	HCLK����AHB���ߡ��ںˡ��ڴ��DMAʹ�õ�HCLKʱ�ӡ���������ʱ�ӣ�һ�㲻��Ƶ������ϵͳʱ�ӣ����Ǹ��ⲿ�豸�ģ������ڴ棬flash��DMA	
	tcnt=0;
	OSSchedLock();								//��ֹOS���ȣ���ֹ���us��ʱ
	told=SysTick->VAL;							//�ս���ʱ�ļ�����ֵ
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told) tcnt+=told-tnow;		//���δ�ݼ���0
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;				//ʱ�䳬�������Ҫ�ӳٵ�ʱ��,���˳�.
		}  
	};
	OSSchedUnlock();							//�ָ�OS����									    
}
