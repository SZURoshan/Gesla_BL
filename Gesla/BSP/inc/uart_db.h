#ifndef __UART_DB_H
#define __UART_DB_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"


void UART_DB_Init(uint32_t baud);  //UART 调试串口初始化
uint8_t UART_DB_GetData(uint8_t *pbuf);  //UART 获取接收的数据

#endif 
