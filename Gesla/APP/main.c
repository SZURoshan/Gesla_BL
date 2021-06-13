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
uint32_t reset_time = 0;
int main(void)
{	
	SCB->VTOR = FLASH_BASE;
	
	u16 oldcount = 0;	//�ϵĴ��ڽ�������ֵ
	u32 applenth = 0;	//���յ���app���볤��
	u8  recive_ota_bin_flag  = 1;//1����������, 0�������
	
	System_Init();
	
	NEED_OTA_FLAG = Flash_ReadHalfWord(NEED_OTA_FLAG_ADDR);
	printf("BL_1 get NEED_OTA_FLAG: %d \r\n", NEED_OTA_FLAG);
	printf("FW USART_RX_CNT: %d \r\n", USART_RX_CNT);
	
	while(1)
	{
		NEED_OTA_FLAG = Flash_ReadHalfWord(NEED_OTA_FLAG_ADDR);
		if(NEED_OTA_FLAG == 1)
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
				
				reset_time++;
				if(reset_time > 1000)//��FW ����Ҫ�ǵ�reset����Ȼ������uart interrupt
				{
					reset_time = 0;
					// �ر������ж�
					__set_FAULTMASK(1); 
					// ��λ
					NVIC_SystemReset(); 				
				}
			}
			else
			{
				if(((*(vu32*)(USART_RX_BUF+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
				{
					printf("��ʼ���¹̼�...\r\n");
					OTA_Begin(FLASH_FW_ADDR, USART_RX_BUF, applenth);
					printf("�̼��������!\r\n");	
					
					#if 0
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
					#else
					FLASH_Unlock();//����
					Writeflash(NEED_OTA_FLAG_ADDR, 0);//flash ����
					FLASH_Lock();//����
					
					
					NEED_OTA_FLAG = Flash_ReadHalfWord(NEED_OTA_FLAG_ADDR);
					printf("BL system reset, NEED_OTA_FLAG: %d \r\n", NEED_OTA_FLAG);
					delay_ms(1000);//��ֹ�������е�unlock
					NVIC_SystemReset();
					#endif
				}
				else
				{
					printf("it is not FW flash bin...\r\n");
				}
				
				/* OTA re-start recieve */
				printf("re-start recieve and not need to set NEED_OTA_FLAG...\r\n");
				recive_ota_bin_flag = 1;
				oldcount     = 0;
				USART_RX_CNT = 0;
				
				FLASH_Unlock();//����
				Writeflash(NEED_OTA_FLAG_ADDR, 1);//flash ����
				FLASH_Lock();//����
				delay_ms(1000);//��ֹ�������е�unlock
				reset_time = 0;
				
				NEED_OTA_FLAG = Flash_ReadHalfWord(NEED_OTA_FLAG_ADDR);
				printf("BL_3 get NEED_OTA_FLAG: %d \r\n", NEED_OTA_FLAG);
			}
		}
		else //do not recieve OTA bin, FW start immediately
		{			
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

			/* OTA re-start recieve */
			printf("sanger test re-start recieve...\r\n");
			recive_ota_bin_flag = 1;
			oldcount     = 0;
			USART_RX_CNT = 0;
			
			FLASH_Unlock();//����
			Writeflash(NEED_OTA_FLAG_ADDR, 1);//flash ����
			FLASH_Lock();//����
			delay_ms(1000);//��ֹ�������е�unlock
			reset_time = 0;
			
			NEED_OTA_FLAG = Flash_ReadHalfWord(NEED_OTA_FLAG_ADDR);
			printf("BL_2 get NEED_OTA_FLAG: %d \r\n", NEED_OTA_FLAG);
		}
	}
}

