#include "adc.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"

void adc_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;   //����ṹ��
	ADC_InitTypeDef ADC_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_ADC1,ENABLE);  //����ADC��IO��ʱ��
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);       //��adcʱ�ӷ�Ƶ���˴�ʹ��6����ʱ��Ϊ12M   72/6=12MHZ
	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AIN;  //ģ������
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0;    //PC0 ���ɼ��˿�
	GPIO_Init(GPIOC, &GPIO_InitStruct);     //��ʼ��IO��
	
	ADC_DeInit(ADC1);   //ADCʱ�Ӹ�λ��֮ǰ��Ƶ��
	
	ADC_InitStruct.ADC_ContinuousConvMode=DISABLE;  //AD ����ת��ģʽ
	ADC_InitStruct.ADC_DataAlign=ADC_DataAlign_Right;//ADC �����Ҷ���
	ADC_InitStruct.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;//ת��������������ⲿ��������
	ADC_InitStruct.ADC_Mode=ADC_Mode_Independent;//ADC ����ģʽ:����ģʽ
	ADC_InitStruct.ADC_NbrOfChannel=1;    ////˳����й���ת���� ADC ͨ������Ŀ 1
	ADC_InitStruct.ADC_ScanConvMode=DISABLE;//AD ��ͨ��ģʽ
	ADC_Init(ADC1,&ADC_InitStruct);        //��ʼ��ADC
	
	ADC_Cmd(ADC1,ENABLE);              //ʹ��ADC
	
	ADC_ResetCalibration(ADC1);        //ʹ�ܸ�λУ׼
	
	while(ADC_GetResetCalibrationStatus(ADC1))   //�ȴ���λУ׼���
		ADC_StartCalibration(ADC1);                //����ADCУ׼
	while(ADC_GetCalibrationStatus(ADC1));       //�ȴ�У׼���
}

u16 adc_get(void)  //adc�ɼ�����
{
	u16 value=0;
	                       //����ADC1ͨ��10Ϊ71.5���������ڣ�����Ϊ1
	ADC_RegularChannelConfig(ADC1,ADC_Channel_10,1,ADC_SampleTime_71Cycles5); //һ��ת��ʱ��7us  Tת��=T����+12.5��ʱ������ 
	                                                                         //  һ�������Ĳ���ʱ��=��71.5+12.5��/��12*10^6��=7us
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);   //ʹ��ָ����ADC1�����ת����������
	
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));  //�ȴ�ת������
	
	value=ADC_GetConversionValue(ADC1);     //���ADCֵ
	
	value=(int)value*3.3*1000/4096;         //�õ������ĵ�ѹֵ����ֵΪ3300���������ݴ���
	
	return value;   //����ȡ��ֵ
}

