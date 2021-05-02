#include "uart_nano.h"
#include <stdio.h>

#include "string.h"


uint8_t Uart2_DMA_Buffer[ Uart2_NANO_SIZE ] = {0};//dma buff addr

/**
  * @简  述  串口初始化
  * @参  数  baud： 波特率设置
  * @返回值	 无
  */
void UART_NANO_Init(uint32_t baud)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	//**USART配置******
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //打开串口GPIO的时钟

	//将USART Tx的GPIO配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//将USART Rx的GPIO配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);  //打开串口外设的时钟

	//配置USART参数
	USART_InitStructure.USART_BaudRate = baud;		//波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	//配置USART为中断源
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //抢断优先级	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	//子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	//使能中断
	NVIC_Init(&NVIC_InitStructure);//初始化配置NVIC

	//使能串口idle中断
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
	
	//DMA串口请求中断
	USART_DMACmd( USART2, USART_DMAReq_Rx, ENABLE );
//	USART_DMACmd( USART2, USART_DMAReq_Tx, ENABLE );
	
	//DMA configure
	UART_DMA_Init();
	
	//使能 USART， 配置完毕
	USART_Cmd(USART2, ENABLE);
}
void UART_DMA_Init(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd( RCC_AHBPeriph_DMA1, ENABLE );                            /* 使能DMA传输 */
	DMA_DeInit( DMA1_Channel6 );                                                          /* 将DMA的通道1寄存器重设为缺省值 */
	DMA_InitStructure.DMA_PeripheralBaseAddr	= (uint32_t)&(USART2->DR);                         /* DMA外设ADC基地址 */
	DMA_InitStructure.DMA_MemoryBaseAddr		= (uint32_t)Uart2_DMA_Buffer;                         /* DMA内存基地址 */
	DMA_InitStructure.DMA_DIR			= DMA_DIR_PeripheralSRC;        /* 数据传输方向，从内存读取发送到外设 */
	DMA_InitStructure.DMA_BufferSize		= Uart2_NANO_SIZE;                            /* DMA通道的DMA缓存的大小 */
	DMA_InitStructure.DMA_PeripheralInc		= DMA_PeripheralInc_Disable;    /* 外设地址寄存器不变 */
	DMA_InitStructure.DMA_MemoryInc			= DMA_MemoryInc_Enable;         /* 内存地址寄存器递增 */
	DMA_InitStructure.DMA_PeripheralDataSize	= DMA_PeripheralDataSize_Byte;  /* 数据宽度为8位 */
	DMA_InitStructure.DMA_MemoryDataSize		= DMA_MemoryDataSize_Byte;      /* 数据宽度为8位 */
	DMA_InitStructure.DMA_Mode			= DMA_Mode_Circular;              /*  */
	DMA_InitStructure.DMA_Priority			= DMA_Priority_VeryHigh;          /* DMA通道 x拥有中优先级 */
	DMA_InitStructure.DMA_M2M			= DMA_M2M_Disable;              /* DMA通道x没有设置为内存到内存传输 */
	
	DMA_Init( DMA1_Channel6, &DMA_InitStructure );                                        /* 根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Rx_DMA_Channel所标识的寄存器 */
	DMA_Cmd(DMA1_Channel6, ENABLE);
}

uint8_t RecFlag;
uint16_t RecCnt;
int16_t chassis_speed_target[3] = {0};
void USART2_IRQHandler(void)            
{
	uint16_t uartRecCnt=0;

	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		USART_ReceiveData(USART2);
		uartRecCnt = sizeof(Uart2_DMA_Buffer) - DMA_GetCurrDataCounter(DMA1_Channel6); 
		//**********************//
		RecFlag=1;
		RecCnt=uartRecCnt;
		//printf("recv data0 = %d, data1 = %d, data2 = %d, data3 = %d\r\n", Uart2_DMA_Buffer[0], Uart2_DMA_Buffer[1], Uart2_DMA_Buffer[2], Uart2_DMA_Buffer[3]);
		chassis_speed_target[0] = (int16_t)((Uart2_DMA_Buffer[0]<<8) | Uart2_DMA_Buffer[1]);
		chassis_speed_target[1] = (int16_t)((Uart2_DMA_Buffer[2]<<8) | Uart2_DMA_Buffer[3]);
		chassis_speed_target[2] = (int16_t)((Uart2_DMA_Buffer[4]<<8) | Uart2_DMA_Buffer[5]);

//		printf("speed_target x:%f, y:%f, z:%f \r\n", chassis_speed_target[0], chassis_speed_target[1], chassis_speed_target[2]);
		//clean
		DMA_Cmd(DMA1_Channel6, DISABLE ); 
		DMA_SetCurrDataCounter(DMA1_Channel6, sizeof(Uart2_DMA_Buffer));
		DMA_Cmd(DMA1_Channel6, ENABLE);     
		
//		memset(chassis_speed_target, 0, 100);
		
		USART_ClearITPendingBit(USART2,USART_IT_IDLE);    
	}
}

static uint8_t uart2_tx_buf[60] = {0};  //发送缓冲
void UART_NANO_SendPacket(uint8_t *pbuf, uint8_t len)
{
	uint8_t i;	

	for(i=0; i<len; i++)
	{
		uart2_tx_buf[i] = *(pbuf+i);
		USART_SendData(USART2, uart2_tx_buf[i]);
		//printf("send data[%d]: %d  \r\n", i, uart2_tx_buf[i]);
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC) != SET);
	}
}
