#include "sys.h"
#include "adc.h"
#include "lcd.h"
#include "delay.h"
#include "key.h"
#include "exti.h"
#include "led.h"
#include "math.h"
#include "usart.h"

//RCT6������DACͨ�����ֱ���DACͨ��1����ӦPA4,DACͨ��������ӦPA5��Ϊ�˱���������ţ�����������Ӧ������Ϊģ������

#define much 256       //DACֵ�ĸ���

u16 value[160];        //ADC��������

//n�ð���ת��ADC�������ڵ�λ�ı�־
//time_get  ADC����������  �������  �����жϺ�����Ҳ����Ӧ����  ����ᷢ��ʵ����������ʾ������
//us_ms   ʵ�ʸñ�ADC�������ڵ��ж�
//time_show  lcd��ʾ�������ڵı�־
int n,time_show,time_get=20,us_ms;

#define DAC_DHR12R2    (u32)&(DAC->DHR12R1)  //DACͨ������ַ  12λ�����Ҷ���

//DACֵ,ע�����ݴ洢ʱ u16��u32��ȫ��ͬ  ����DACֻ��16λ��ʵ��12λ�� ���Բ�����u32 �������ֵ����  ���Բο�ȡֵ����
//������ֵ�ο���ֵ��
u16 dac_out[much]={1985,2016,2047,2078,2109,2140,2171,2202,2233,2264,2295,
	2326,2357,2388,2419,2450,2481,2512,2543,2574,2605,2636,2667,2698,2730,
	2761,2792,2823,2854,2885,2916,2947,2978,3009,3040,3071,3102,3133,3164,
	3195,3226,3257,3288,3319,3350,3381,3412,3443,3474,3505,3536,3567,3598,
	3629,3660,3691,3722,3753,3784,3815,3846,3877,3908,3939,3970,3939,3908,
	3877,3846,3815,3784,3753,3722,3691,3660,3629,3598,3567,3536,3505,3474,
	3443,3412,3381,3350,3319,3288,3257,3226,3195,3164,3133,3102,3071,3040,
	3009,2978,2947,2916,2885,2854,2823,2792,2761,2730,2698,2667,2636,2605,
	2574,2543,2512,2481,2450,2419,2388,2357,2326,2295,2264,2233,2202,2171,
	2140,2109,2078,2047,2016,1985,1954,1923,1892,1861,1830,1799,1768,1737,
	1706,1675,1644,1613,1582,1551,1520,1489,1458,1427,1396,1365,1333,1302,
	1271,1240,1209,1178,1147,1116,1085,1054,1023,992,961,930,899,868,837,
	806,775,744,713,682,651,620,589,558,527,496,465,434,403,372,341,310,
	279,248,217,186,155,124,93,62,31,31,62,93,124,155,186,217,248,279,310,
	341,372,403,434,465,496,527,558,589,620,651,682,713,744,775,806,837,
	868,899,930,961,992,1023,1054,1085,1116,1147,1178,1209,1240,1271,1302,
	1333,1365,1396,1427,1458,1489,1520,1551,1582,1613,1644,1675,1706,1737,
  1768,1799,1830,1861,1892,1923,1954,1985};  
  
//����������������Ϊ���β��������ã�����DMA+DAC+TIM2
//��ʼ����ʱ��2
void timer_init(u32 f)   
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	
	f=(u16)(72000000*2/sizeof(dac_out)/f);             //����ת���ɶ�ʱ����װ��ֵ
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);   //��ʼ���ṹ���Ա  һ��Ҫ��  �������ΪһЩ��Ա��Ĭ��ֵ���³�ʼ��ʧ��
	TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInitStruct.TIM_Period=f;               //ʵ������װ��ֵ  �ı�����¼��Ĵ���Ƶ��   ֵԽС  Ƶ��Խ��  ֵԽ��  Ƶ��ԽС
	TIM_TimeBaseInitStruct.TIM_Prescaler=0x5;         //Tout= ((arr+1)*(psc+1))/Tclk��10000
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
	
	TIM_SelectOutputTrigger(TIM2,TIM_TRGOSource_Update);//��ʱ�������������������������һ����ʱ��������ADת����
}

//��ʼ��DAC2,ͨ��1����-ģת��DAC  PA4
void dac_init(void) 
{
	GPIO_InitTypeDef GPIO_InitStruct;
	DAC_InitTypeDef DAC_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC,ENABLE);
	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AIN;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	DAC_StructInit(&DAC_InitStruct);                       //��ʼ���ṹ���Ա  һ��Ҫ��  �������ΪһЩ��Ա��Ĭ��ֵ���³�ʼ��ʧ��
	//DAC����������������棬������������������迹�������ⲿ�˷žͿ�ֱ������������ʹ�ܵĻ���������޷��ﵽ0
	DAC_InitStruct.DAC_OutputBuffer=DAC_OutputBuffer_Enable;//enable�����⣡����������
	DAC_InitStruct.DAC_Trigger=DAC_Trigger_T2_TRGO;
	DAC_InitStruct.DAC_WaveGeneration=DAC_WaveGeneration_None;
	DAC_Init(DAC_Channel_1, &DAC_InitStruct);
	
	DAC_Cmd(DAC_Channel_1,ENABLE);
	
	DAC_DMACmd(DAC_Channel_1,ENABLE);
}

//��ʼ��DMA
void dma_init(void)
{
	DMA_InitTypeDef DMA_InitStruct;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2,ENABLE);
	
	DMA_StructInit( &DMA_InitStruct);           //��ʼ���ṹ���Ա  һ��Ҫ��  �������ΪһЩ��Ա��Ĭ��ֵ���³�ʼ��ʧ��
	DMA_InitStruct.DMA_BufferSize=much;
	DMA_InitStruct.DMA_DIR=DMA_DIR_PeripheralDST;
	DMA_InitStruct.DMA_M2M=DMA_M2M_Disable;
	DMA_InitStruct.DMA_MemoryDataSize=DMA_MemoryDataSize_HalfWord;
	DMA_InitStruct.DMA_MemoryInc=DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_Mode=DMA_Mode_Circular;
	DMA_InitStruct.DMA_PeripheralDataSize=DMA_PeripheralDataSize_HalfWord;
	DMA_InitStruct.DMA_PeripheralInc=DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_Priority=DMA_Priority_VeryHigh;
	
	DMA_InitStruct.DMA_MemoryBaseAddr=(uint32_t)dac_out;//�ڴ�Ļ���ַ
	DMA_InitStruct.DMA_PeripheralBaseAddr=DAC_DHR12R2;//�������ַ��һ��Ҫע���Ƿ���ȷ
	
	DMA_Init(DMA2_Channel3, &DMA_InitStruct);//����DAC��ͨ��һ����DMA2�ĵ�3ͨ������
	
	DMA_Cmd(DMA2_Channel3,ENABLE);//����DAC��ͨ��������DMA2�ĵ�4ͨ�����ƣ�ע������
}

//������������ʼ��
void wave_init(u32 f)
{
	timer_init(f);
	dac_init();
	dma_init();
	TIM_Cmd(TIM2,ENABLE);
}


//������
int main()
{
	int i=0;              //
	delay_init();         //��ʱ����ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);     //�жϷּ�
	uart_init(9600);      //��ʼ������   ����lcd�޷�ʹ��
	wave_init(10000);      //��ʼ�����β���������Ƶ��
	EXTIX_Init();         //��ʼ���ⲿ�ж�  �а�������
	LED_Init();           //��ʼ��LED  ��ʾ��������
	KEY_INT();            //��ʼ������
	LCD_Init();           //��ʼ��lcd
	LCD_Display_Dir(1);   //����lcd������ʾ
	adc_init();           //��ʼ��adc
	POINT_COLOR=RED;      //������ʾΪ��ɫ
	LED0=0;				  //����LED
	LED1=1;               //������LED
	while(1)
	{
		
		//��ʾ��ѹ�ߺͲ�������
		LCD_DrawLine(0,120,320,120);
		LCD_ShowString(5,102,50,12,12,"0.5---");   //0.5/0.0275=18.1�����أ�ÿ�����ص�0.0275V
		LCD_ShowString(5,84,50,12,12,"1.0---");
		LCD_ShowString(5,66,50,12,12,"1.5---");
		LCD_ShowString(5,48,50,12,12,"2.0---");
		LCD_ShowString(5,30,50,12,12,"2.5---");
		LCD_ShowString(5,12,50,12,12,"3.0---");
		LCD_ShowString(10,140,40,30,16,"unit:");  //��λ
		LCD_ShowString(10,160,50,30,16,"cycle:"); //����
		
		//����������
		
		
		//��������ѡ��   �ڰ����ж��У�ÿ��һ��PC5 - KEY0 �ı�һ��n�����ı�һ������ѡ��
		if(n%2) {
			LCD_ShowString(50,140,12,12,16,"ms");
			us_ms=1;}//ms
		else  {  
			LCD_ShowString(50,140,12,12,16,"us");
			us_ms=0;}//us
			switch(time_show)  //��ʾ�������ڱ�־
		{
			case 0:LCD_ShowString(60,160,20,20,16,"20");break;//0
		  case 1:LCD_ShowString(60,160,20,20,16,"40");break;//1
		  case 2:LCD_ShowString(60,160,20,20,16,"60");break;//2
		  case 3:LCD_ShowString(60,160,20,20,16,"80");break;//3
		  case 4:LCD_ShowString(60,160,20,20,16,"100");break;//4
		  case 5:LCD_ShowString(60,160,20,20,16,"120");break;//5
		}
		
		//ADC����             
		while(i<160)
		{
			value[i]=adc_get();   //��adc�ɼ��������ݸ�value
			value[i]=(int)value[i]/27;   //����lcd��ʾ��ѹ����Ϊ120��131�У���ʾ�Ǹ��ߣ�������ÿ�����ص�Ϊ0.0275v��3.3V/120���أ�����adc.c������value����ֵ��ֵ3300��
			                                                                                                                    //�����õ�ѹֵ����27���Ա���ʾ
			if(us_ms==1) delay_ms(time_get); //ms��  delay_ms��20/40/60/80/100/120�� // ����ÿ��������  
			else delay_us(time_get-16);   //us��  delay_us��4/24/44/64/84/104��     //�ı�����֮��ļ��delay����������ͼ�����ڵ�Ӱ��
			i++;                                                                 //
		}
		i=0;
		
		//LCD��ʾ����
		while(i<159)
		{
			POINT_COLOR=RED;  //320�����أ�160��ȡ��������i*2��ÿ��������һ��ȡ���㡣
			LCD_DrawLine(i*2,120-value[i],(i+1)*2,120-value[i+1]);  //ͼ��--�� ������i��i+1֮���ʱ����
			delay_ms(5);
			i++;
		}
		i=0;
		
		//�ӳ���ʾʱ��
		delay_ms(3000);
		delay_ms(3000);
		
		//����
		LCD_Clear(WHITE);
	}
}

