/*****************************************Copyright(C)******************************************
**-----------------------------------------�ļ���Ϣ---------------------------------------------
**��    ��    ��: bsp_RTC.c
**��    ��    ��: andydriver
**��  ��  ��  ��: 081210
**��  ��  ��  ��: V1.0
**��          ��: RS8025����
**---------------------------------------��ʷ�汾��Ϣ-------------------------------------------
**��    ��    ��: ��ӱ��
**��          ��: 2019.03.06
**��          ��: V1.0
**��          ��: �����޶�������������Լ���ֺ�����
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
#include "bsp_RTC.h"



/*ȫ�ֱ���*/
struct BSPRTC_TIME gRtcTime;
struct BSPRTC_TIME gSetTime;




/* --------------------Private functions begin-------------------------------------------------------------------------*/
#if 1
/*******************************************************************************
* Function Name: static void iic_delay(INT32U time)                                        
* Description:   ��IICģ����ʱ����
* Input:         time����ʱֵ��1 or 2 instruction clock,32M�Ļ���Tclk=31.25ns
* Return:        None
*******************************************************************************/
static void iic_delay(INT32U time)
{
	while(time--);
}

/*******************************************************************************
* Function Name: static void iic_clk_high(void)                                         
* Description:   IIC_CLK����ߵ�ƽ
* Input:         None
* Return:        None
*******************************************************************************/
static void iic_clk_high(void)
{
	GPIO_WriteBit(SIIC_GPIO,SIIC_GPIO_SCL,Bit_SET);   
	iic_delay(IIC_DELAY_TIME);
}

/*******************************************************************************
* Function Name: static void iic_clk_low(void)                                          
* Description:   IIC_CLK����͵�ƽ
* Input:         None
* Return:        None
*******************************************************************************/
static void iic_clk_low(void)
{
	GPIO_WriteBit(SIIC_GPIO,SIIC_GPIO_SCL,Bit_RESET);  
	iic_delay(IIC_DELAY_TIME);
}

/*******************************************************************************
* Function Name: static void iic_data_set_in(void)                                                  
* Description:   ����IIC_SDA����Ϊ����ģʽ
* Input:         None
* Return:        None
*******************************************************************************/
static void iic_data_set_in(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	// ����SDA
	GPIO_InitStructure.GPIO_Pin = SIIC_GPIO_SDA;
 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;      					//��������
	
	GPIO_Init(SIIC_GPIO, &GPIO_InitStructure);	
}
																				
/*******************************************************************************
* Function Name: static void iic_data_set_out(void)                                           
* Description:   ����IIC_SDA����Ϊ���ģʽ
* Input:         None
* Return:        None
*******************************************************************************/
static void iic_data_set_out(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	// ����SDA
	GPIO_InitStructure.GPIO_Pin = SIIC_GPIO_SDA;
 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       						//�������
	GPIO_Init(SIIC_GPIO, &GPIO_InitStructure);	
}

/*******************************************************************************
* Function Name: static void iic_data_high(void)                                        
* Description:   IIC_SDA��������ߵ�ƽ(����1)
* Input:         None
* Return:        None
*******************************************************************************/
static void iic_data_high(void)
{
	GPIO_WriteBit(SIIC_GPIO,SIIC_GPIO_SDA,Bit_SET);  
	iic_delay(IIC_DELAY_TIME);
}

/*******************************************************************************
* Function Name: static void iic_data_low(void)                                          
* Description:   IIC_SDA��������͵�ƽ(����0)
* Input:         None
* Return:        None
*******************************************************************************/
static void iic_data_low(void)
{
	GPIO_WriteBit(SIIC_GPIO,SIIC_GPIO_SDA,Bit_RESET);  
	iic_delay(IIC_DELAY_TIME);
}

/*******************************************************************************
* Function Name: static INT8U iic_data_read(void)                                       
* Description:   ��IICģ����ʱ����
* Input:         None
* Return:        I/O���ϵ�һ��λ�ĵ�ǰ�����ƽ״̬��1����  0����
*******************************************************************************/
static INT8U iic_data_read(void)
{
	iic_delay(IIC_DELAY_TIME);
	return GPIO_ReadInputDataBit(SIIC_GPIO,SIIC_GPIO_SDA);
}

/*******************************************************************************
* Function Name: void iic_init(void)                                    
* Description:   �ⲿRTC(Rx8025)оƬ�ӿ�I2C��ʼ�����õ�Ƭ����IO��ģ���I2C
* Input:         None
* Return:        None
*******************************************************************************/
void iic_init(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	// ����SCLΪ�����	  
	RCC_APB2PeriphClockCmd(RCCRTCEN ,ENABLE);
	GPIO_InitStructure.GPIO_Pin = SIIC_GPIO_SCL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SIIC_GPIO, &GPIO_InitStructure);
	
	iic_data_high();
	iic_data_set_out();
	iic_clk_high();
}

/*******************************************************************************
* Function Name: void iic_start(void)                                   
* Description:   I2C��ʼ
* Input:         None
* Return:        None
*******************************************************************************/
void iic_start(void)
{ 
	iic_delay(IIC_DELAY_TIME_LONG);
	iic_delay(IIC_DELAY_TIME_LONG);
	iic_delay(IIC_DELAY_TIME_LONG);
	iic_delay(IIC_DELAY_TIME_LONG);
	iic_data_high();
	iic_data_set_out();
	iic_clk_high();
	iic_data_low();
	iic_clk_low();
}

/*******************************************************************************
* Function Name: void iic_start(void)                                   
* Description:   I2Cֹͣ
* Input:         None
* Return:        None
*******************************************************************************/
void iic_stop(void)
{
	iic_clk_low();
	iic_data_low();
	iic_data_set_out();
	iic_clk_high();
	iic_data_high();
}

/*******************************************************************************
* Function Name: void iic_ack(void)                               
* Description:    Ӧ��:���ݽ��ճɹ�
* Input:         None
* Return:        None
*******************************************************************************/
void iic_ack(void)
{
	iic_data_low();
	iic_data_set_out();
	iic_clk_high();
}

/*******************************************************************************
* Function Name: void iic_noack(void)                          
* Description:   Ӧ��:���ݽ��ղ��ɹ�
* Input:         None
* Return:        None
*******************************************************************************/
void iic_noack(void)
{
	iic_data_high();
	iic_data_set_out();
	iic_clk_high();
}

/*******************************************************************************
* Function Name: INT8U iic_send_byte(INT8U val)                                        
* Description:   I/O��ģ���IIC����һ���ֽڵ�����
* Input:         val:Ҫ���͵�ֵ��1Byte��
* Return:        None
*******************************************************************************/
INT8U iic_send_byte(INT8U val)
{
	INT8U i=0;
	
	iic_clk_low();           // ������ʱ��
	iic_data_set_out();      // ����SDA��Ϊ���ģʽ
	for(i=0;i<8;i++)
	{
		if(val&0x80)
		{
			iic_data_high();
		}
		else
		{
			iic_data_low();
		}
		iic_clk_high();
		                  //zzs note??? CNM���źŶ���û�������������õͣ�����Ҳ�У�����
		iic_clk_low();
		val = val<<1;
	}
	
	/*�ȴ�Ӧ��*/
	iic_data_set_in();
	iic_data_read();  //zzs??? read�˸����أ�Ҳ���ø���������һ�£�
	iic_clk_high();
	i=0;
	while(iic_data_read())
	{
		if(++i > 12)
		{
			iic_clk_low();
			return 0;
		}
	}
	iic_clk_low();
	return 1;
}

/*******************************************************************************
* Function Name: INT8U iic_rec_byte(void)                                 
* Description:   I/O��ģ���IIC����һ���ֽڵ�����
* Input:         None
* Return:        val: ���յ���ֵ(1Byte)
*******************************************************************************/
INT8U iic_rec_byte(void)
{
	INT8U val=0;
	INT8U i=0;
	
	val = 0;
	iic_clk_low();
	iic_data_set_in();														// SDA����,����Ҫ
	for(i=0;i<8;i++)
	{
		iic_delay(IIC_DELAY_TIME_LONG);
		iic_clk_high();
		val = val<<1;
		val |= iic_data_read();
		iic_clk_low();
	}
	return val;
}

/*******************************************************************************
* Function Name: static void RX8025Write(INT8U addr,INT8U *pData,INT8U len)                                 
* Description:   I/O��ģ���IIC����RX8025ָ����ַд��һ������
* Input:         addr  :  RX8025�ڲ��Ĵ����ĵ�ַ
                 pData : ����������ָ����ָ��Ҫ�����������͵����ݴ����׵�ַ��ָ��
                 len   : ���͵����ݴ����� 
*
* Return:        None
*******************************************************************************/
static void RX8025Write(INT8U addr,INT8U *pData,INT8U len)
{
	INT8U i=0;
	
	iic_start();
	if(iic_send_byte(RX8025_ADDR_WRITE)==0)
	{
		iic_stop();
		return;
	}
	if(iic_send_byte(addr)==0)
	{
		iic_stop();
		return;
	}
	for(i=0;i<len;i++)
	{
		if(iic_send_byte(pData[i])==0)
		{
			iic_stop();
			return;
		}
	}
	iic_stop();
}

/*******************************************************************************
* Function Name: static void RX8025Read(INT8U addr,INT8U *pData,INT8U len)                        
* Description:   I/O��ģ���IIC����RX8025ָ����ַ��ȡһ�����ݣ�������Ӧ�þ���8421BCD�� ZE��
* Input:         addr  :  RX8025�ڲ��Ĵ����ĵ�ַ
                 pData : ����������ָ�������ڽ�������RX8025������ �Ľ��տռ���׵�ַָ��
                 len   : ���յ����ݴ����� 
*
*
* Return:        ͨ���β�pData����
*******************************************************************************/
static void RX8025Read(INT8U addr,INT8U *pData,INT8U len)
{
	INT8U i=0;
	
	iic_start();
	if(iic_send_byte(RX8025_ADDR_WRITE)==0)
	{
		iic_stop();
		return;
	}
	
	if(iic_send_byte(addr)==0)
	{
		iic_stop();
		return;
	}
	
	iic_start();
	if(iic_send_byte(RX8025_ADDR_READ)==0)
	{
		iic_stop();
		return;
	}
	
	for(i=0;i<len-1;i++)
	{
		pData[i] = iic_rec_byte();
		iic_ack();
	}
	pData[i] = iic_rec_byte();
	iic_noack();
	iic_stop();
}

/*******************************************************************************
* Function Name: void BSP_RX8025Write(INT8U *pData,INT8U len)                                     
* Description:   ��ʱ��оƬRX8025д������
* Input:         pData : Ҫд�������
                 len   : д�볤��
* Return:        None
*******************************************************************************/
void BSP_RX8025Write(INT8U *pData,INT8U len)
{
	RX8025Write((RX8025_ADDR_SECONDS&RX8025_WRITE_MODE),pData,len);
}

/*******************************************************************************
* Function Name: void BSP_RX8025Read(INT8U *pData,INT8U len)                              
* Description:   ��ʱ��оƬRX8025��ȡ����
* Input:         pData : �������ݵĴ�ŵص�
                 len   : ��ȡ����
* Return:        None
*******************************************************************************/
void BSP_RX8025Read(INT8U *pData,INT8U len)
{
	RX8025Read((RX8025_ADDR_SECONDS&RX8025_READ_MODE),pData,len);
}

#if 0
/*******************************************************************************
* Function Name: void BSP_RX8025ControlINTA(_BSPRX8025_INTAOUT state)                            
* Description:   ����оƬ��INTA��״̬
* Input:         state:״̬(_BSPRX8025_INTAOUT)
								 BSPRX8025_INTAOUT_HIZ:����
								 BSPRX8025_INTAOUT_LOW:�����
								 BSPRX8025_INTAOUT_2HZ:���2Hz(50%)������
								 BSPRX8025_INTAOUT_1HZ:���1Hz(50%)������
								 BSPRX8025_INTAOUT_SEC:ÿ��ĵ�0�뷭ת?��������?
								 BSPRX8025_INTAOUT_MIN:ÿ�ֵĵ�0�뷭ת?��������?
								 BSPRX8025_INTAOUT_HOUR:ÿʱ�ĵ�0�뷭ת?��������?
								 BSPRX8025_INTAOUT_MONTH:ÿ�µĵ�0�뷭ת?��������?
* Return:        None
*
* Author:                               
* Date First Issued: ��־˴��2018��1��18�մ���������             E-Mail:11207656@qq.com
* Version:  V1.0
*******************************************************************************/
void BSP_RX8025ControlINTA(_BSPRX8025_INTAOUT state)
{
	union RX8025_REG_CONTROL1	data={0};
	
	RX8025Read((RX8025_ADDR_CONTROL1&RX8025_READ_MODE),(INT8U *)(&data),1);
	data.bits.CT = state;							// INTA���1Hz����
	RX8025Write((RX8025_ADDR_CONTROL1&RX8025_WRITE_MODE),(INT8U *)(&data),1);
}
#endif

/*******************************************************************************
* Function Name: void BSP_RX8025Init(void)                                
* Description:   ʱ��оƬRX8025�ĳ�ʼ��
* Input:         None
* Return:        None
*******************************************************************************/
void BSP_RX8025Init(void)
{
	INT8U buf[16] = {0};
	
	OSSchedLock();    															
	
	iic_init();
	RX8025Read((RX8025_ADDR_SECONDS&RX8025_READ_MODE),buf,8);
	buf[0] = 0x20;
	RX8025Write((RX8025_ADDR_CONTROL1&RX8025_WRITE_MODE),buf,1);				//24Сʱ��
	RX8025Read((RX8025_ADDR_SECONDS&RX8025_READ_MODE),buf,16);
	iic_delay(50000);
	
	OSSchedUnlock();
}

#endif




/*===========================================================����ΪRX8025�ײ��������===========================================================*/


/*===========================================================����ΪRTCͨ��API�ӿں���===========================================================*/






/*******************************************************************************
* Function Name: static void BSP_RTCWrite(const struct BSPRTC_TIME *pTime)                          
* Description:   ��RTCд��ṹ���е�ʱ��
* Input:         pTime : ����ָ���ģ�ָ��RTCʱ��ṹ����׵�ַ {�룻�֣�ʱ���ܣ��գ��£���}��һ����У��ʱ��ʱ�ı�׼ʱ�丱����
* Return:        None
*******************************************************************************/ 
static void BSP_RTCWrite(const struct BSPRTC_TIME *pTime)
{
	OSSchedLock();
	BSP_RX8025Write((INT8U *)pTime,7);									
	OSSchedUnlock();
}

/*******************************************************************************
���ƣ�static void BSP_RTCRead(struct BSPRTC_TIME *pTime)
���ܣ���ȡRTCʱ���API��
��Σ���
���Σ�struct BSPRTC_TIME *pTime��RTCʱ��ṹ����׵�ַ {�룻�֣�ʱ���ܣ��գ��£���}����������
��ȥ2000�ģ���0x19��8421BCD������2019��
���أ���
*******************************************************************************/
static void BSP_RTCRead(struct BSPRTC_TIME *pTime)
{
	OSSchedLock();
	BSP_RX8025Read((INT8U *)pTime,7);									
	OSSchedUnlock();
}

/*******************************************************************************
* Function Name: INT8U RtcSetChinaStdTimeStruct(const struct BSPRTC_TIME *pTime)                           
* Description:   ��ʱ��оƬRX8025����RTCʱ�䣨UTC +8 �й���׼ʱ�� ��CST�����������������д�����ݺϷ���У����̣��Լ�ʧ�ܶೢ��д��2�εĴ���
                 д��֮��������ȡ����������Ϸ��ԣ��Լ��ԱȽ���ǰ��������Ƿ�һ�¡�
* Input:         pTime : CSTʱ��ṹ����׵�ַ {�룻�֣�ʱ���ܣ��գ��£���}��һ����У��ʱ��ʱ�ı�׼ʱ�丱����
* Return: 		 1���ɹ�   0��ʧ��
*******************************************************************************/
INT8U RtcSetChinaStdTimeStruct(const struct BSPRTC_TIME *pTime)
{   
	INT8U i=3;
	struct BSPRTC_TIME time={0};
	
	if(RtcCheckTimeStruct(pTime)==0) return 0;									//д��ǰ���ݺϷ��Բ���
	while(i--)
	{
		BSP_RTCWrite(pTime);    												//��ʱ��д��RTCоƬ
		if(RtcGetChinaStdTimeStruct(&time)==1) return 1;  						//���ز�У��Ϸ��ԣ���ͨ��
		BSP_RX8025Init();														//ʧ�������³�ʼ��
	}
	return 0;
}

/*******************************************************************************
* Function Name: INT8U RtcGetChinaStdTimeStruct(struct BSPRTC_TIME *pTime)                              
* Description:   ��ʱ��оƬRX8025ȡ��RTCʱ�䣨UTC +8 �й���׼ʱ�� ��CST��������������������ݺϷ���У����̣��Լ�ʧ�ܶೢ�Զ�ȡ2�εĴ���
* Input:         pTime : ����ָ���ģ����ڴ�Ŵ�ʱ��оƬRX8025��ȡ����RTCʱ��ģ�RTCʱ��ṹ����׵�ַ {�룻�֣�ʱ���ܣ��գ��£���}
                         ����׼��������ռ䡣
* Return:       1����ʶȡ�ú�����ȷ��ʱ�䣬�����������β�pTime���շ���
                0����ȡRTCʱ��ʧ��
*******************************************************************************/
INT8U RtcGetChinaStdTimeStruct(struct BSPRTC_TIME *pTime)
{
	INT8U 			i=3;
	
	while(i--)																	//�������3��
	{
		BSP_RTCRead(pTime);														//���ö�ȡRTCʱ���API
		if(RtcCheckTimeStruct(pTime)==1) return 1;								//���ݺϷ�
		BSP_RX8025Init();														//ʧ�������³�ʼ��
	}
	return 0;
}

/*******************************************************************************
* Function Name:  INT8U RtcSetTimeSecond(INT32U time)                   
* Description  :  ������ת������Ҫ����ʱ������RTCд�������룬RTCоƬAPI�ӿں��������������룬ת�����й���׼ʱ�䣨CST��8421BCD��д��RTCоƬ
* Input        :  time : ������
*
* Return       :  1 :��ʶдRTC�ɹ�      
*******************************************************************************/
INT8U RtcSetTimeSecond(const INT32U time)
{
	struct tm 			*TTM = 0;												//���������Զ�ʶ��Ϊ��ָ��
	struct BSPRTC_TIME 	TM = {0}; 
	INT32U time_e8 = time+8*3600;												//��������0��ʱ�䣬ת��Ϊ������ ( UTC +8 )
	
	TTM =localtime(&time_e8);													//������ת��Ϊ����ʱ�䣨û�о���ʱ���任��
	TM.Year   = HexToBCD(TTM->tm_year-100);										//��1900 ��ʼ����
	TM.Month  = HexToBCD(TTM->tm_mon+1);										//�·ݼ���1(localtime�ļ�������0��ʼ)
	TM.Day    = HexToBCD(TTM->tm_mday);
	TM.Hour   = HexToBCD(TTM->tm_hour);
	TM.Minute = HexToBCD(TTM->tm_min);
	TM.Second = HexToBCD(TTM->tm_sec);
	TM.Week   =	HexToBCD(TTM->tm_wday);											//��
	
	return RtcSetChinaStdTimeStruct(&TM);										// 1���ɹ�   0��ʧ��
}

/*******************************************************************************
���ƣ�INT32U RtcGetTimeSecond(void)
���ܣ���RTC��ȡ�й���׼ʱ�䣨CST����¼��ʱ�䣬ת��Ϊ��1970 1.1.0ʱ��ʼ����������������롢ʱ��������ء�
��Σ���
���Σ���
���أ���ȡ�ɹ����������룬��ȡʧ�ܷ���0
*******************************************************************************/
INT32U RtcGetTimeSecond(void)
{
	time_t time = 0;															//typedef unsigned int time_t;     /* date/time in unix secs past 1-Jan-70 */
	struct tm TTM = {0};      								

	RtcGetChinaStdTimeStruct(&gRtcTime);										//��ʱ��оƬRX8025ȡ��RTCʱ��
	TTM.tm_year = BcdToHex(gRtcTime.Year)+100;  								// ��
	TTM.tm_mon  = BcdToHex(gRtcTime.Month)-1;   								// ��
	TTM.tm_mday = BcdToHex(gRtcTime.Day);       								// ��
	TTM.tm_hour = BcdToHex(gRtcTime.Hour);      								// ʱ
	TTM.tm_min  = BcdToHex(gRtcTime.Minute);    								// ��
	TTM.tm_sec  = BcdToHex(gRtcTime.Second);    								// ��
	time = mktime(&TTM)-8*3600;                  								//������ ( UTC +8 )ʱ��ת����������
	
	if (time==0xffffffff) return 0;												//�쳣
	return time;
}

/*******************************************************************************
* Function Name: INT8U RtcCheckTimeStruct(struct BSPRTC_TIME *pTime)                         
* Description:   ������ݺ�����
* Input:         pTime : ����ָ����,ָ��RTCʱ��ṹ����׵�ַ {�룻�֣�ʱ���ܣ��գ��£���}
                         
* Return:        0: RTCʱ���в����������    
                 1��RTCʱ������Բ�����ȷ����
*******************************************************************************/
INT8U RtcCheckTimeStruct(const struct BSPRTC_TIME *pTime)
{ 
	if((pTime->Year > 0x99) || ((pTime->Year&0x0f) > 0x09))   					//����2099�� �� &0x0f���ִ���9���������쳣��8421BCD�������9��
		return 0;
	
	if(((pTime->Month) == 0) || ((pTime->Month) > 0x12) || ((pTime->Month&0x0f) > 0x09))	//�·��쳣
		return 0;
	
	if((pTime->Day == 0) || (pTime->Day > 0x31) || ((pTime->Day&0x0f) > 0x09))	//���쳣
		return 0;
	
	if(pTime->Week>6) return 0;             									//����~������0~6
		
	if((pTime->Hour > 0x23) || ((pTime->Hour&0x0f) > 0x09))						//ʱ�쳣
		return 0;
	
	if((pTime->Minute > 0x59) || ((pTime->Minute&0x0f) > 0x09))					//���쳣
		return 0;
	
	if((pTime->Second > 0x59) || ((pTime->Second&0x0f) > 0x09))					//���쳣
		return 0;
	
	return 1;
}

/*******************************************************************************
���ƣ�INT8U GetSysTime(INT8U *pOutBuff)
���ܣ�ȡ��ϵͳʱ��:ȡ��ʱ����������ʱ�����ܵ�˳Ѱ����һ������Ľṹ�壬�ײ������˳��Ϊ��������ʱ����
��Σ���
���Σ�pOutBuff : �������շ������ݵ�ָ��
���أ��̶�Ϊ1
*******************************************************************************/
INT8U GetSysTime(INT8U *pOutBuff)
{
	RtcGetChinaStdTimeStruct(&gRtcTime);										//��ʱ��оƬȡ��RTCʱ��
	
	pOutBuff[0] = gRtcTime.Year;												// ��
	pOutBuff[1] = gRtcTime.Month;												// ��
	pOutBuff[2] = gRtcTime.Day;													// ��
	pOutBuff[3] = gRtcTime.Hour;												// ʱ
	pOutBuff[4] = gRtcTime.Minute;												// ��
	pOutBuff[5] = gRtcTime.Second;												// ��
	pOutBuff[6] = gRtcTime.Week; 												// ��	========ע������λ�ñ��ˣ�ԭ�ṹ����ʱ�ǵ�4�ֽ�
	
	return 1;
}

/*******************************************************************************
���ƣ�INT8U BcdToHex(INT8U InData)
���ܣ�BCD�뵽Hex��ת��
��Σ�INT8U InData,Ҫת��������(BCD��ʽ 1Byte)
���Σ���
���أ�ת���������(Hex��ʽ 1Byte)
*******************************************************************************/
INT8U BcdToHex(INT8U InData)
{
	INT8U Bits = 0;
	Bits = InData&0x0F;															//ȡBCD���λ����4λ��
	InData = InData&0xF0;														//ȡBCD��ʮλ����4λ��
	InData >>= 4;        														//��ʮλ�����λ
	InData *= 10;       														//�����ʮλ��ֵ
	InData += Bits;																//�ӻظ�λ��8421BCD�룬ֱ�ӼӾ����ˣ�
	return InData;
}
																				
/*******************************************************************************
���ƣ�INT8U HexToBCD(INT8U InData)
���ܣ�Hex��BCD���ת��
��Σ�InData: Ҫת��������(Hex��ʽ 1Byte)
���Σ���
���أ�ת���������(BCD��ʽ 1Byte)�������볬��99�򷵻�0xff����ʾ����
*******************************************************************************/
INT8U HexToBCD(INT8U InData)
{
	INT8U Temp = 0;
	if (InData>99) return 0xff;													//�����쳣
	Temp = InData%10;           												//��λ
	InData = InData-Temp;
	InData /= 10;             													//ʮλ
	Temp += ((InData<<4)&0xf0);
	return Temp;
}

/******************************************************************************* 
* Function Name  : void RTC_LowPower(void)
* Description    : RTC����͹��ģ�������ӦIO�����͹��Ĵ���
* Input          : None 
* Output         : None 
* Return         : None 
*******************************************************************************/
void RTC_LowPower(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = SIIC_GPIO_SCL|SIIC_GPIO_SDA;					//PA0��PA1
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;								//ģ������
	GPIO_Init(SIIC_GPIO, &GPIO_InitStructure);									//PA��
}

/******************************************************************************* 
* Function Name  : INT8U RTCTaskTest(void)
* Description    : RTCӲ�����Ժ���
* Input          : None 
* Output         : None 
* Return         : �ɹ�����1��ʧ�ܷ���0
*******************************************************************************/
INT8U RTCTaskTest(void)
{
	INT8U				times = 5;
	TCHAR				temp[100] = {0};
	
	BSP_RX8025Init();
	if(!RtcSetTimeSecond(1602331994)) return 0;									//2020.10.10 20:13:14
	while(times--)
	{
		if(!RtcGetChinaStdTimeStruct(&gRtcTime)) return 0;						//��ʱ��оƬȡ��RTCʱ��
		sprintf(temp+strlen(temp), "����ʱ�䣺20%X��%X��%X�� %02X:%02X:%02X\r\n",gRtcTime.Year,gRtcTime.Month,gRtcTime.Day,gRtcTime.Hour,gRtcTime.Minute,gRtcTime.Second);
		BspUartWrite(2,(INT8U*)temp,strlen(temp));
		OSTimeDly(20);
	}
	return 1;
}

/************************(C)COPYRIGHT 2018 ���ϵ���*****END OF FILE****************************/
