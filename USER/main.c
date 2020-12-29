#include "sys.h"
#include "adc.h"
#include "lcd.h"
#include "delay.h"
#include "key.h"
#include "exti.h"
#include "led.h"
#include "math.h"
#include "usart.h"

//RCT6有两个DAC通道，分别是DAC通道1，对应PA4,DAC通道二，对应PA5，为了避免寄生干扰，这两个引脚应该设置为模拟输入

#define much 256       //DAC值的个数

u16 value[160];        //ADC采样个数

//n用按键转换ADC测量周期单位的标志
//time_get  ADC采样的周期  如果更改  请在中断函数中也做相应更改  否则会发生实际周期与显示不符和
//us_ms   实际该变ADC采样周期的判断
//time_show  lcd显示采样周期的标志
int n,time_show,time_get=20,us_ms;

#define DAC_DHR12R2    (u32)&(DAC->DHR12R1)  //DAC通道二地址  12位数据右对齐

//DAC值,注意数据存储时 u16与u32完全不同  由于DAC只存16位（实际12位） 所以不能用u32 会造成数值减半  可以参看取值函数
//具体数值参看数值表
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
  
//接下来到主函数均为波形产生的配置，包括DMA+DAC+TIM2
//初始化定时器2
void timer_init(u32 f)   
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	
	f=(u16)(72000000*2/sizeof(dac_out)/f);             //这里转换成定时器重装载值
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);   //初始化结构体成员  一定要加  否则会因为一些成员的默认值导致初始化失败
	TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInitStruct.TIM_Period=f;               //实际是重装载值  改变更新事件的触发频率   值越小  频率越大  值越大  频率越小
	TIM_TimeBaseInitStruct.TIM_Prescaler=0x5;         //Tout= ((arr+1)*(psc+1))/Tclk；10000
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
	
	TIM_SelectOutputTrigger(TIM2,TIM_TRGOSource_Update);//定时器触发输出，比如用作触发另一个定时器，触发AD转换等
}

//初始化DAC2,通道1，数-模转换DAC  PA4
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
	
	DAC_StructInit(&DAC_InitStruct);                       //初始化结构体成员  一定要加  否则会因为一些成员的默认值导致初始化失败
	//DAC集成了两个输出缓存，可以用来减少输出的阻抗，无需外部运放就可直接驱动，但是使能的话会让输出无法达到0
	DAC_InitStruct.DAC_OutputBuffer=DAC_OutputBuffer_Enable;//enable的问题！！！！！！
	DAC_InitStruct.DAC_Trigger=DAC_Trigger_T2_TRGO;
	DAC_InitStruct.DAC_WaveGeneration=DAC_WaveGeneration_None;
	DAC_Init(DAC_Channel_1, &DAC_InitStruct);
	
	DAC_Cmd(DAC_Channel_1,ENABLE);
	
	DAC_DMACmd(DAC_Channel_1,ENABLE);
}

//初始化DMA
void dma_init(void)
{
	DMA_InitTypeDef DMA_InitStruct;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2,ENABLE);
	
	DMA_StructInit( &DMA_InitStruct);           //初始化结构体成员  一定要加  否则会因为一些成员的默认值导致初始化失败
	DMA_InitStruct.DMA_BufferSize=much;
	DMA_InitStruct.DMA_DIR=DMA_DIR_PeripheralDST;
	DMA_InitStruct.DMA_M2M=DMA_M2M_Disable;
	DMA_InitStruct.DMA_MemoryDataSize=DMA_MemoryDataSize_HalfWord;
	DMA_InitStruct.DMA_MemoryInc=DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_Mode=DMA_Mode_Circular;
	DMA_InitStruct.DMA_PeripheralDataSize=DMA_PeripheralDataSize_HalfWord;
	DMA_InitStruct.DMA_PeripheralInc=DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_Priority=DMA_Priority_VeryHigh;
	
	DMA_InitStruct.DMA_MemoryBaseAddr=(uint32_t)dac_out;//内存的基地址
	DMA_InitStruct.DMA_PeripheralBaseAddr=DAC_DHR12R2;//外设基地址，一定要注意是否正确
	
	DMA_Init(DMA2_Channel3, &DMA_InitStruct);//对于DAC的通道一，是DMA2的第3通道控制
	
	DMA_Cmd(DMA2_Channel3,ENABLE);//对于DAC的通道二，是DMA2的第4通道控制，注意区分
}

//函数发生器初始化
void wave_init(u32 f)
{
	timer_init(f);
	dac_init();
	dma_init();
	TIM_Cmd(TIM2,ENABLE);
}


//主函数
int main()
{
	int i=0;              //
	delay_init();         //定时器初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);     //中断分级
	uart_init(9600);      //初始化串口   否则lcd无法使用
	wave_init(10000);      //初始化波形产生并输入频率
	EXTIX_Init();         //初始化外部中断  有按键产生
	LED_Init();           //初始化LED  显示程序运行
	KEY_INT();            //初始化按键
	LCD_Init();           //初始化lcd
	LCD_Display_Dir(1);   //设置lcd竖向显示
	adc_init();           //初始化adc
	POINT_COLOR=RED;      //设置显示为红色
	LED0=0;				  //点亮LED
	LED1=1;               //不点亮LED
	while(1)
	{
		
		//显示电压线和采样周期
		LCD_DrawLine(0,120,320,120);
		LCD_ShowString(5,102,50,12,12,"0.5---");   //0.5/0.0275=18.1个像素，每个像素点0.0275V
		LCD_ShowString(5,84,50,12,12,"1.0---");
		LCD_ShowString(5,66,50,12,12,"1.5---");
		LCD_ShowString(5,48,50,12,12,"2.0---");
		LCD_ShowString(5,30,50,12,12,"2.5---");
		LCD_ShowString(5,12,50,12,12,"3.0---");
		LCD_ShowString(10,140,40,30,16,"unit:");  //单位
		LCD_ShowString(10,160,50,30,16,"cycle:"); //周期
		
		//横坐标增加
		
		
		//更改周期选择   在按键中断中，每按一次PC5 - KEY0 改变一次n，即改变一次周期选择
		if(n%2) {
			LCD_ShowString(50,140,12,12,16,"ms");
			us_ms=1;}//ms
		else  {  
			LCD_ShowString(50,140,12,12,16,"us");
			us_ms=0;}//us
			switch(time_show)  //显示采样周期标志
		{
			case 0:LCD_ShowString(60,160,20,20,16,"20");break;//0
		  case 1:LCD_ShowString(60,160,20,20,16,"40");break;//1
		  case 2:LCD_ShowString(60,160,20,20,16,"60");break;//2
		  case 3:LCD_ShowString(60,160,20,20,16,"80");break;//3
		  case 4:LCD_ShowString(60,160,20,20,16,"100");break;//4
		  case 5:LCD_ShowString(60,160,20,20,16,"120");break;//5
		}
		
		//ADC采样             
		while(i<160)
		{
			value[i]=adc_get();   //将adc采集到的数据给value
			value[i]=(int)value[i]/27;   //由于lcd显示电压部分为120（131行，显示那根线），所以每个像素点为0.0275v（3.3V/120像素），（adc.c过来的value函数值满值3300）
			                                                                                                                    //所以用电压值除以27，以便显示
			if(us_ms==1) delay_ms(time_get); //ms级  delay_ms（20/40/60/80/100/120） // 对于每个抽样点  
			else delay_us(time_get-16);   //us级  delay_us（4/24/44/64/84/104）     //改变点与点之间的间隔delay可以做到对图像周期的影响
			i++;                                                                 //
		}
		i=0;
		
		//LCD显示波形
		while(i<159)
		{
			POINT_COLOR=RED;  //320个像素，160个取样，所以i*2，每两个像素一个取样点。
			LCD_DrawLine(i*2,120-value[i],(i+1)*2,120-value[i+1]);  //图像--线 ，利用i与i+1之间的时间间隔
			delay_ms(5);
			i++;
		}
		i=0;
		
		//延长显示时间
		delay_ms(3000);
		delay_ms(3000);
		
		//清屏
		LCD_Clear(WHITE);
	}
}

