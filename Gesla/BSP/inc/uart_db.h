#ifndef __UART_DB_H
#define __UART_DB_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#include "OTA.h"


extern u8  USART_RX_BUF[OTA_USART_REC_LEN]; //���ջ���,���OTA_USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_CNT;				//���յ��ֽ���	

void UART_DB_Init(uint32_t baud);  //UART ���Դ��ڳ�ʼ��
uint8_t UART_DB_GetData(uint8_t *pbuf);  //UART ��ȡ���յ�����
void UART1_Reset(void);

#endif 
