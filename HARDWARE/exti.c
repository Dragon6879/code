#include "exti.h"
#include "led.h"
#include "key.h"
#include "delay.h"
#include "usart.h"
 
extern int time_show,time_get,n,us_ms;
//n用按键转换ADC测量周期单位的标志
//time_get  ADC采样的周期  如果更改  请在中断函数中也做相应更改  否则会发生实际周期与显示不符和
//us_ms   实际改变ADC采样周期的判断
//time_show  lcd显示采样周期的标志
void EXTIX_Init(void)
{
 
 	  EXTI_InitTypeDef EXTI_InitStructure;
 	  NVIC_InitTypeDef NVIC_InitStructure;

  	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);      

	  KEY_INT();                                               //初始化按键

    
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource5);//key0

  	EXTI_InitStructure.EXTI_Line=EXTI_Line5;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;   //下降沿触发
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 

   
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource15);//key1

  	EXTI_InitStructure.EXTI_Line=EXTI_Line15;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;    //下降沿触发
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	  	

    
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);//wk_up

   	EXTI_InitStructure.EXTI_Line=EXTI_Line0;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;     //上升沿触发
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	


 
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			//使能按键所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					//子优先级1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);  	  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
		
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//使能按键所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					//子优先级1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init( &NVIC_InitStructure); 
 
 
   	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;			//使能按键所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					//子优先级1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init( &NVIC_InitStructure); 
 
}

 
void EXTI0_IRQHandler(void)
{
  delay_ms(10);    //消抖
	if(KEY2==1)
	{	  
		LED0=!LED0;
		LED1=!LED1;	
	}
	EXTI_ClearITPendingBit(EXTI_Line0);  //清除EXTI0线路挂起位
}

 void EXTI9_5_IRQHandler(void)
{			
	delay_ms(10);    //消抖			 
  if(KEY0==0)	n++; 
 	 EXTI_ClearITPendingBit(EXTI_Line5);    //清除LINE5上的中断标志位  
}


void EXTI15_10_IRQHandler(void)
{
  delay_ms(10);    //消抖			 
  if(KEY1==0)	
	{
		if(time_show==5)   //20,40,60,80,100,120 循环按键选择
		{
			time_show=-1;
			time_get=0;
		}
		time_show++;
		time_get+=20;
	}
	 EXTI_ClearITPendingBit(EXTI_Line15);  //清除LINE15线路挂起位
}
