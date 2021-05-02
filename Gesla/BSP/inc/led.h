#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"

void LED_Init(void);

#define LED_Red_Off()		GPIO_SetBits(GPIOD, GPIO_Pin_2)      
#define LED_Red_On()		GPIO_ResetBits(GPIOD, GPIO_Pin_2)    
#define LED_Red_Toggle()	GPIO_WriteBit(GPIOD, GPIO_Pin_2, (BitAction) (1 - GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_2)))	//LEDGÂÌÉ«×´Ì¬·­×ª

#define LED_Green_Off()		GPIO_SetBits(GPIOA, GPIO_Pin_8)      
#define LED_Green_On()		GPIO_ResetBits(GPIOA, GPIO_Pin_8)    
#define LED_Green_Toggle()	GPIO_WriteBit(GPIOA, GPIO_Pin_8, (BitAction) (1 - GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8)))	//LEDGÂÌÉ«×´Ì¬·­×ª

#endif 
