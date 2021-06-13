#include "uart_db.h"
#include <stdio.h>

#include "OTA.h"

#include "led.h"


uint8_t OTA_FLASH_FLAG = 0;     

/**
  * @��  ��  UART DB���Դ��ڳ�ʼ��
  * @��  ��  baud�� ����������
  * @����ֵ	 ��
  */
void UART_DB_Init(uint32_t baud)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//**���Դ���USART����******
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //�򿪴���GPIO��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);  //�򿪴��������ʱ��
	
	//��USART Tx��GPIO����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//��USART Rx��GPIO����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//����USART����
	USART_InitStructure.USART_BaudRate = baud; //������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	
	//����USARTΪ�ж�Դ
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //�������ȼ�	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	//ʹ���ж�
	NVIC_Init(&NVIC_InitStructure);//��ʼ������NVIC
	
	//ʹ�ܴ��ڽ����ж�
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	//ʹ�� USART�� �������
	USART_Cmd(USART1, ENABLE);
}

/**
  * @��  ��  DBUART �����жϷ�����
  * @��  ��  �� 
  * @����ֵ  ��
  */
u16 USART_RX_STA=0;       //����״̬���	 
u16 USART_RX_CNT=0;			//���յ��ֽ���	 
u8 USART_RX_BUF[OTA_USART_REC_LEN] = {0}; //OTA���ջ���
//u8 USART_RX_BUF[OTA_USART_REC_LEN] __attribute__ ((at(0X20001000)));//���ջ���,���USART_REC_LEN���ֽ�,��ʼ��ַΪ0X20001000. 
void USART1_IRQHandler(void)
{
	uint8_t Res = 0;
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�
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

//��BL��
void UART1_Reset(void)
{
	USART_DeInit(USART1);
	DMA_DeInit( DMA1_Channel5 );
}

void UART1_Buff_Reset()
{
	
}

/**************************���ڴ�ӡ��غ����ض���********************************/
/**
  * @��  ��  �ض���putc������USART1��	
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
  * @��  ��  �ض���getc������USART1��	
  */
int fgetc(FILE *f)
{
	/* �ȴ�����1�������� */
	while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
	{}

	return (int)USART_ReceiveData(USART1);
}
