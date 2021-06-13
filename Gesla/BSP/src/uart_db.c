#include "uart_db.h"
#include <stdio.h>

#include "OTA.h"

#include "led.h"


uint8_t OTA_FLASH_FLAG = 0;     

/**
  * @简  述  UART DB调试串口初始化
  * @参  数  baud： 波特率设置
  * @返回值	 无
  */
void UART_DB_Init(uint32_t baud)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//**调试串口USART配置******
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //打开串口GPIO的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);  //打开串口外设的时钟
	
	//将USART Tx的GPIO配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//将USART Rx的GPIO配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//配置USART参数
	USART_InitStructure.USART_BaudRate = baud; //波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	
	//配置USART为中断源
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //抢断优先级	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	//使能中断
	NVIC_Init(&NVIC_InitStructure);//初始化配置NVIC
	
	//使能串口接收中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	//使能 USART， 配置完毕
	USART_Cmd(USART1, ENABLE);
}

/**
  * @简  述  DBUART 串口中断服务函数
  * @参  数  无 
  * @返回值  无
  */
u16 USART_RX_STA=0;       //接收状态标记	 
u16 USART_RX_CNT=0;			//接收的字节数	 
u8 USART_RX_BUF[OTA_USART_REC_LEN] = {0}; //OTA接收缓存
//u8 USART_RX_BUF[OTA_USART_REC_LEN] __attribute__ ((at(0X20001000)));//接收缓冲,最大USART_REC_LEN个字节,起始地址为0X20001000. 
void USART1_IRQHandler(void)
{
	uint8_t Res = 0;
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断
	{
		LED_Green_Off();
		LED_Red_Off();
		Res =USART_ReceiveData(USART1);	
		if(USART_RX_CNT<OTA_USART_REC_LEN)
		{
			USART_RX_BUF[USART_RX_CNT]=Res;
			USART_RX_CNT++;			 									     
		} 
		
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	} 
}

//进BL用
void UART1_Reset(void)
{
	USART_DeInit(USART1);
	DMA_DeInit( DMA1_Channel5 );
}

void UART1_Buff_Reset()
{
	
}

/**************************串口打印相关函数重定义********************************/
/**
  * @简  述  重定义putc函数（USART1）	
  */
int fputc(int ch, FILE *f)
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	USART_SendData(USART1, (uint8_t) ch);

	/* Loop until the end of transmission */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
	{}

	return ch;
}

/**
  * @简  述  重定义getc函数（USART1）	
  */
int fgetc(FILE *f)
{
	/* 等待串口1输入数据 */
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
	{}

	return (int)USART_ReceiveData(USART1);
}
