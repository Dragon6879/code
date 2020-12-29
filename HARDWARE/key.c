#include "stm32f10x.h"
#include "key.h"
#include "delay.h"
#include "usart.h"

void KEY_INT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_5;     // PC5--KEY0
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPU; //���ڿ����尴���ӵأ�������   ��������
	GPIO_Init(GPIOC,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_15;    //PA15--KEY1
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPU; //���ڿ����尴���ӵأ�������   ��������
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0;    //PA0--WK_UP
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPD; //���ڿ����尴���Ӹߣ�������   ��������
	GPIO_Init(GPIOA,&GPIO_InitStruct);
}

u8 KEY_CHOOSE(u8 mode)
{
	static u8 KEY_UP=1;   //��̬  �������ɿ���־
	if(mode)//1֧������
	{
		KEY_UP=1;
	}
	if(KEY_UP&&(KEY0==0||KEY1==0||KEY2==1))  //���а���������
 {
	delay_ms(10);//ȥ����
	KEY_UP=0;
	if(KEY0==0)
	{
		return KEY0_VALUE;//1   �궨�����������
	}
	else if(KEY1==0)
	{
		return KEY1_VALUE;//2
	}
	else if(KEY2==1)
	{
		return KEY2_VALUE;//3
	}
 }
 else if(KEY0==1&&KEY1==1&&KEY2==0)  //û�а�������
	{
		KEY_UP=1;  //�����ɿ���־
	}
	return 0;
}
