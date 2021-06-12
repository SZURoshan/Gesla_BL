/*
** A single spark can start a prairie fire.
*/

#include "stm32f10x.h"
#include <stdio.h>

/* APP */
#include "sysinit.h"
#include "flash_control.h"
#include "OTA.h"
#include "delay.h"

/* BSP */
#include "uart_db.h"
const u8 OTA_TEST[]={"Sanger"};

int main(void)
{	
	u16 oldcount = 0;	//�ϵĴ��ڽ�������ֵ
	u32 applenth = 0;	//���յ���app���볤��
	u8  recive_ota_bin_flag  = 1;//1����������, 0�������
	
	System_Init();
	
	while(1)
	{
		if(recive_ota_bin_flag)
		{
			if(USART_RX_CNT)
			{
				if(oldcount == USART_RX_CNT)//10ms��,û���յ��κ�����,��Ϊ�������ݽ������.
				{
					applenth     = USART_RX_CNT;
					oldcount     = 0;
					USART_RX_CNT = 0;
					
					printf("�û�����������!\r\n");
					printf("���볤��:%dBytes\r\n", applenth);
					recive_ota_bin_flag = 0;
					
					delay_ms(1000);
				}
				else 
				{
					oldcount = USART_RX_CNT;		
				}				
			}
			delay_ms(10);
		}
		else
		{
			if(((*(vu32*)(0X20001000+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
			{
				printf("��ʼ���¹̼�...\r\n");
				OTA_Begin(FLASH_FW_ADDR, USART_RX_BUF, applenth);
				printf("�̼��������!\r\n");	
				
				/* FW Running */
				printf("FLASH FW running: 0x%02X \r\n", FLASH_FW_ADDR);

				delay_ms(1000);
				if(((*(vu32*)(FLASH_FW_ADDR+4))&0xFF000000)==0x08000000)//�ж�OTA����������ʼ��ַ�Ƿ�Ϸ����Ƿ�Ϊ0X08XXXXXX.
				{
					iap_load_app(FLASH_FW_ADDR);//ִ��FLASH APP����
				}
				else
				{
					printf("illegal ota bin FLASH_FW_ADDR: %x \r\n", ((*(vu32*)(FLASH_FW_ADDR+4))&0xFF000000));
				}

			}
			else
			{
				printf("it is not FW flash bin...\r\n");
			}
			
			printf("re-start recieve...\r\n");
			recive_ota_bin_flag = 1;
			oldcount = USART_RX_CNT;
		}
	}
}

