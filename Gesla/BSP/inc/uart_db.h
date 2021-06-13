#ifndef __UART_DB_H
#define __UART_DB_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#include "OTA.h"


extern u8  USART_RX_BUF[OTA_USART_REC_LEN]; //接收缓冲,最大OTA_USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_CNT;				//接收的字节数	

void UART_DB_Init(uint32_t baud);  //UART 调试串口初始化
uint8_t UART_DB_GetData(uint8_t *pbuf);  //UART 获取接收的数据
void UART1_Reset(void);

#endif 
