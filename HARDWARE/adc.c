#include "adc.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"

void adc_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;   //定义结构体
	ADC_InitTypeDef ADC_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_ADC1,ENABLE);  //开启ADC和IO口时钟
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);       //对adc时钟分频，此处使用6，即时钟为12M   72/6=12MHZ
	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AIN;  //模拟输入
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0;    //PC0 作采集端口
	GPIO_Init(GPIOC, &GPIO_InitStruct);     //初始化IO口
	
	ADC_DeInit(ADC1);   //ADC时钟复位，之前分频的
	
	ADC_InitStruct.ADC_ContinuousConvMode=DISABLE;  //AD 单次转换模式
	ADC_InitStruct.ADC_DataAlign=ADC_DataAlign_Right;//ADC 数据右对齐
	ADC_InitStruct.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;//转换由软件而不是外部触发启动
	ADC_InitStruct.ADC_Mode=ADC_Mode_Independent;//ADC 工作模式:独立模式
	ADC_InitStruct.ADC_NbrOfChannel=1;    ////顺序进行规则转换的 ADC 通道的数目 1
	ADC_InitStruct.ADC_ScanConvMode=DISABLE;//AD 单通道模式
	ADC_Init(ADC1,&ADC_InitStruct);        //初始化ADC
	
	ADC_Cmd(ADC1,ENABLE);              //使能ADC
	
	ADC_ResetCalibration(ADC1);        //使能复位校准
	
	while(ADC_GetResetCalibrationStatus(ADC1))   //等待复位校准完成
		ADC_StartCalibration(ADC1);                //开启ADC校准
	while(ADC_GetCalibrationStatus(ADC1));       //等待校准完成
}

u16 adc_get(void)  //adc采集函数
{
	u16 value=0;
	                       //配置ADC1通道10为71.5个采样周期，序列为1
	ADC_RegularChannelConfig(ADC1,ADC_Channel_10,1,ADC_SampleTime_71Cycles5); //一次转换时间7us  T转换=T采样+12.5个时钟周期 
	                                                                         //  一次完整的采样时间=（71.5+12.5）/（12*10^6）=7us
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);   //使能指定的ADC1的软件转换启动功能
	
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));  //等待转换结束
	
	value=ADC_GetConversionValue(ADC1);     //获得ADC值
	
	value=(int)value*3.3*1000/4096;         //得到整数的电压值，满值为3300，方便数据处理
	
	return value;   //返回取样值
}

