#ifndef _key_
#define _key_
#include "sys.h"

#define KEY0 GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)
#define KEY1 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15)
#define KEY2 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//WK_UP

#define KEY0_VALUE 1//不能设置为0
#define KEY1_VALUE 2
#define KEY2_VALUE 3

void KEY_INT(void);
u8 KEY_CHOOSE(u8 mode);
#endif

