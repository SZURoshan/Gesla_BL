#ifndef __UART_NANO_H
#define __UART_NANO_H

#define    Uart2_NANO_SIZE           20   //ÉÔÎ¢¸ø´óµã

/* Includes ------------------------------------------------------------------*/	 
#include "stm32f10x.h"

extern uint8_t Uart2_DMA_Buffer[ Uart2_NANO_SIZE ];
extern int16_t chassis_speed_target[3]; // x y z

void UART_NANO_Init(uint32_t baud);
void UART_DMA_Init(void);
void UART_NANO_SendPacket(uint8_t *pbuf, uint8_t len);

#endif 
