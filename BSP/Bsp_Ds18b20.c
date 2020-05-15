#include "Bsp_DS18B20.h"



/******************************************************************************* 
* Function Name  : void DS18B20_LowPower(void)
* Description    : DS18B20��·�͹��ġ�
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/ 
void DS18B20_LowPower(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	/*��Դʹ�ܡ�DQ(WSD_DA)����������*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;							//ģ������
	
	GPIO_InitStructure.GPIO_Pin = DS18B20_PW_PIN;							//��Դʹ�ܿڣ��ص�Դ PA11
	GPIO_Init(DS18B20_PW_PORT, &GPIO_InitStructure);						//

	GPIO_InitStructure.GPIO_Pin = DS18B20_IO_PIN;							//DQ(WSD_DA) PC8
	GPIO_Init(DS18B20_IO_PORT, &GPIO_InitStructure);						//
}

/*===========================================================================
������void delay_SYSus(u32 nus)
˵����us��ʱ������systickһֱ�ڵݼ�����ͣ��ȡֵ�Ƚϣ����Ƚ�ֵ�������ʱ���˳�              
     ��Ҫע���ֹOS���ȴ����ʱ
	 
	 systickƵ��̫�͵���us�����޷�������׼
	 delay_SYS_2us��15��=60us�� 10=40us, 1=30us,
��Σ�
���Σ�
============================================================================*/
void delay_SYS_2us(u32 n2us)
{		
	u32 ticks = 0;
	u32 told = 0;
	u32 tnow = 0;
	u32 tcnt = 0;
	u32 reload = SysTick->LOAD;					//LOAD��ֵ	    	 
	
	ticks=n2us; 						      	  //��Ҫ�Ľ�����HLCK=32M	  systick=HCLK/8=4M��ÿus��Ҫ�Ľ�����Ϊ4		 //HCLK=4M systick=500K,��С��λΪ2us��
	tcnt=0;
	OSSchedLock();						       //��ֹOS���ȣ���ֹ���us��ʱ
	told=SysTick->VAL;        			//�ս���ʱ�ļ�����ֵ
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;		//���δ�ݼ���0
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;				//ʱ�䳬�������Ҫ�ӳٵ�ʱ��,���˳�.
		}  
	};
	OSSchedUnlock();						//�ָ�OS����									    
}




/*===========================================================================
������void DS18B20_RST(void)
˵������λDS18B20���ȴ�Ӧ��
��Σ�
���Σ�
============================================================================*/
void DS18B20_RST(void)
{
	DS18B20_IO_OUT();
	DS18B20_L();
	DS_delay600us;               
	DS18B20_H();                     //�����>480us���ͷ�����
	DS_delay40us ;                    //�ӳ�15-60us�ȴ�Ӧ��
}

/*===========================================================================
������u8 DS18B20_CHECK(void)
˵����DS18B20��λӦ��
��Σ�
���Σ�0:DS18B20 READY
============================================================================*/
u8 DS18B20_CHECK(void)
{
	int i = 0;
	
	DS18B20_IO_IN();         // ����ģʽ
	while(DS18B20_IN&&(i<100))  // 200us�ڵȴ�DS18B20���͵�ƽ
	{
	  i++;
	  DS_delay2us ;	//2us
	}
	if(i>=100)
	{ 
		return 1; 
	}
    
	i=0;                
	while(!DS18B20_IN&&(i<120))  //�͵�ƽ����������240us�϶���Ч
	{
	  i++;
	 DS_delay2us ;	//2us
	}
	
	DS_delay600us ;	//��λӦ��slot>480us
	
	if(i>=120)
	{
		return 1;
	}
	
	return 0;
}

/*===========================================================================
������u8 DS18B20_Init(void)  
˵������ʼ������,ȷ��DS18B20�ܹ�Ӧ��
��Σ�
���Σ�0:DS18B20 READY
============================================================================*/
u8 DS18B20_Init(void)                                              
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(DS18B20_PW_Port_CK|DS18B20_IO_Port_CK,ENABLE);
 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =DS18B20_PW_PIN;
	GPIO_Init(DS18B20_PW_PORT, &GPIO_InitStructure);           //DS18B20VCC �������
	
	GPIO_InitStructure.GPIO_Pin =DS18B20_IO_PIN;
	GPIO_Init(DS18B20_IO_PORT, &GPIO_InitStructure);	         //DS18B20 DQ �������
	
	
	DS18B20_PW_ON();                                           
	DS_delay10us ;	                                              //�ȴ���Դ�ȶ��������ޱ�Ҫ��


	DS18B20_H();                                               //����
                    	
	DS18B20_RST();
		
	return DS18B20_CHECK();
}

/*===========================================================================
������void DS18B20_Wakeup(void)  
˵��������DS18B20���ϵ������и�λ
��Σ�
���Σ�
============================================================================*/

void DS18B20_Wakeup(void)  
{
    DS18B20_PW_ON();                                           
	DS_delay10us ;                                            //�ȴ���Դ�ȶ��������ޱ�Ҫ��


	DS18B20_H();                                               //����
                    	
	DS18B20_RST();
		
	DS18B20_CHECK(); 
}
/*===========================================================================
������void DS18B20_Sleep(void)  
˵�������͹��ģ��رյ�Դ��DQ��
��Σ�
���Σ�
============================================================================*/

void DS18B20_Sleep(void)  
{
   DS18B20_PW_OFF();
   DS18B20_L(); 
}

/*===========================================================================
������void DS18B20_Write_Byte(u8 dat)
˵������DS18B20д��һ���ֽڣ�����ָ������
��Σ�u8
���Σ�
============================================================================*/
void DS18B20_Write_Byte(u8 dat)
{
	int temp,i=0;
    DS18B20_IO_OUT();    //���ģʽ
	for(i=0;i<8;i++)
	{
			temp=dat&0x01;
			dat=dat>>1;
			if(temp)          //д1    a minimum of 60��s
			{
				DS18B20_L();                     //��������д
				DS_delay2us ;     //2us
				DS18B20_H();  		
				DS_delay60us ;            //60us
			}
			else              //д0
			{
				DS18B20_L();                    //��������д
				DS_delay60us ;               //60us
				DS18B20_H();  		
				DS_delay2us ;	     //2us
			}
  }
}

/*===========================================================================
������u8 DS18B20_Read_Bit(void)
˵������DS18B20һλ
��Σ�
���Σ�1λ
============================================================================*/
u8 DS18B20_Read_Bit(void)
{
	int temp;
    DS18B20_IO_OUT();     //���ģʽ
	DS18B20_L();          //����������
	DS_delay2us ;	      //2us
	DS18B20_H();          //�ͷ�����
	DS18B20_IO_IN();      //����ģʽ
	DS_delay10us ;     //��15us֮�ڲ���     2+10=12us
	if(DS18B20_IN)
		temp=1;
	else temp=0;

	DS_delay60us ;         //a minimum of 60��s

	DS18B20_IO_OUT();
	DS18B20_H();          //�ͷ�����
	
	return temp;
}

/*===========================================================================
������u8 DS18B20_Read_Byte(void)
˵������DS18B20һ���ֽ�
��Σ�
���Σ�1���ֽ�
============================================================================*/
u8 DS18B20_Read_Byte(void)
{
    int i,dat,temp=0;
	for(i=0;i<8;i++)
	{
		temp=DS18B20_Read_Bit();
		dat=(temp<<7)|(dat>>1);
//		dat=dat>>1;
//	    temp=DS18B20_Read_Bit();
//		if(temp)dat|=0x80;	
	}
	return dat;
}
/*===========================================================================
������u8 DS18B20_Read_ROM(void)
˵������ROM��LSB������д�Ƿ�ɹ�
��Σ�
���Σ�1���ֽ�
============================================================================*/
u8 DS18B20_Read_ROM(void)
{ 
    u8 temp;
	DS18B20_RST();
	DS18B20_CHECK();
	DS18B20_Write_Byte(0x33);
	temp=DS18B20_Read_Byte();
	return temp;
}


/*===========================================================================
������u8 DS18B20_Get_Temperature(u16 *TempBuff)
˵������DS18B20�����ֽڣ��ֱ�ΪTL��TH��TH��5λΪ��־λ
��Σ�
���Σ�����1��ʾ�¶�ת���ɹ�,TempBuff[0]����ת������¶�
============================================================================*/
u8 DS18B20_Get_Temperature(u16 *TempBuff)
{
    u16 TL,TH,temp=0;
	u8 over=0;
	

	if( DS18B20_Init()) return 0;        //DS18B20 not ready
  
	DS18B20_Write_Byte(0xcc);             //skip rom
	DS18B20_Write_Byte(0x44);             //convert  start
    OSTimeDly(15); 	                      //��ʱ750ms�ȴ�ת�����             
	if(DS18B20_Read_Bit()) over=1;        //ת�����
	
	DS18B20_RST();	
    if(DS18B20_CHECK()) return 0;         //DS18B20 not ready
	DS18B20_Write_Byte(0xcc);             //skip rom
	DS18B20_Write_Byte(0xbe);             //read   
    TL = DS18B20_Read_Byte();              //LSB
	TH = DS18B20_Read_Byte();              //MSB

	
	if(TH>7)                              //����
	{
		temp=~((TH<<8)+TL)+1;            //DS18B20�Բ�����ʽ�������
		temp=(temp*5)>>3;                //*0.625=5/8  �����¶ȵ�10��  ����0.0625��C
		if(temp>550) temp = 550;
		// TempBuff[0]=temp|=0xf800;
		temp|=0xf800;
		TempBuff[0]=temp;
	}
	else 
	{
		temp=((TH<<8)+TL);                 //����
		temp=(temp*5)>>3;                  //*0.625=5/8  �����¶ȵ�10��  ����0.0625��C 
		if(temp>1250)temp=1250;
		TempBuff[0]=temp;
	}
	
	
    DS18B20_Sleep(); 
	return over;
}

/*************************************************************
������u8 Get_DS18B20Temp(u16 *temp)
˵�������¶�3�β���������С˳�����У�ȡ�м�ֵ
��Σ�Buff  �ɼ����¶�ֵ
���Σ���ȷ����1 ���󷵻�0
**********************************************************************/
u8 Get_DS18B20Temp(u16 *temp)
{
	u16 DStemp,TEMP;
	u16 DSbuff[3]={0};
	u8 count,i,j,k=0;
	u8 over=0;

	DS18B20_Init();
//  ROM_lsb=DS18B20_Read_ROM();            		//���Է��������ܷ�������������
	for(count=0;count<3;count++)             	//����3��
	{
		for(i=0;i<3;i++)
		{
			over = DS18B20_Get_Temperature(&DStemp);
			if(!over) break;                    //����ȡ�¶�ʧ��
			else DSbuff[i]=DStemp;
		}
		
		/*���ɼ��ɹ�*/
		if(over)
		{
			for(j=0;j<2;j++)
			{
				for(k=j+1;k<3;k++)
				{
					if(DSbuff[j]<DSbuff[k])
					{
						TEMP=DSbuff[j];
						DSbuff[j]=DSbuff[k];
						DSbuff[k]=TEMP;
					}
				}	 
			}
			temp[0]=DSbuff[1];                	//ȡ�м�ֵ
		}
	}
	DS18B20_Sleep(); 
	return over;
}


/*************************************************************
������float TempU16toFloat(INT16U *pIn)
˵����DS18B20�õ����¶�����ת������
��Σ�U16����
���Σ����ظ������¶�
**********************************************************************/
float TempU16toFloat(INT16U In)
{
	float Temp=0;
	
	if(In>=0xf800)  					// ��5λΪ����λ,����ֵ˵��Ϊ���£���=0xf800��ʱ����Ϊ-0��Ҫ���ɸ���������F8ȥ��������ʵ���Ǹ�0��
	{
		In = (In&0x07ff);        		// ȥ������λ��������ֵ   
		Temp = (float )In;
		Temp = -Temp;	
		Temp = Temp /10.0 ;      		// 1��С��λ		
	}
	else{
		Temp = (float )In;
		Temp =  In /10.0 ;           	// 1��С��λ
	}

return Temp;							//���ظ�����
}

/************************************************************************************************************************
* Function Name : INT16U FloatToTempU16(float pIn)                                                                     
* Description   : ��������������˾�Ĺ���ת����INT16U�������
* Input         : None                                                
* Return        : None                        
*
* History :                               
* First Issued  : ��־˴��2018��11��26�մ���������             E-Mail:11207656@qq.com
* Version       : V1.0
* Modify By     : 
*************************************************************************************************************************/
INT16U FloatToTempU16(float In)
{
	INT16U Temp = 0;
	
	if(In<0)
	{
		In=-In;
		In*=10;
		Temp=(INT16U)In;
		Temp+=0xF800;
	}else{
	    In*=10;
		Temp=(INT16U)In;
	}
return Temp;					 //������
}

