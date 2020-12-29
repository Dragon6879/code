#include "stm32f10x.h"
#include "key.h"
#include "delay.h"
#include "usart.h"

void KEY_INT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_5;     // PC5--KEY0
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPU; //由于开发板按键接地，故拉高   输入上拉
	GPIO_Init(GPIOC,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_15;    //PA15--KEY1
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPU; //由于开发板按键接地，故拉高   输入上拉
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0;    //PA0--WK_UP
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPD; //由于开发板按键接高，故拉低   输入下拉
	GPIO_Init(GPIOA,&GPIO_InitStruct);
}

u8 KEY_CHOOSE(u8 mode)
{
	static u8 KEY_UP=1;   //静态  按键按松开标志
	if(mode)//1支持连按
	{
		KEY_UP=1;
	}
	if(KEY_UP&&(KEY0==0||KEY1==0||KEY2==1))  //若有按键被按下
 {
	delay_ms(10);//去抖动
	KEY_UP=0;
	if(KEY0==0)
	{
		return KEY0_VALUE;//1   宏定义过，方便用
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
 else if(KEY0==1&&KEY1==1&&KEY2==0)  //没有按键按下
	{
		KEY_UP=1;  //按键松开标志
	}
	return 0;
}
