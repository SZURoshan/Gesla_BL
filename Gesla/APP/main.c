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
	
	u16 oldcount = 0;	//老的串口接收数据值
	u32 applenth = 0;	//接收到的app代码长度
	u8  recive_ota_bin_flag  = 1;//1正在收数据, 0接收完毕
	
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
					if(oldcount == USART_RX_CNT)//10ms内,没有收到任何数据,认为本次数据接收完成.
					{
						applenth     = USART_RX_CNT;
						oldcount     = 0;
						USART_RX_CNT = 0;
						
						printf("用户程序接收完成!\r\n");
						printf("代码长度:%dBytes\r\n", applenth);
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
				if(reset_time > 1000)//从FW 回来要记得reset，不然进不了uart interrupt
				{
					reset_time = 0;
					// 关闭所有中断
					__set_FAULTMASK(1); 
					// 复位
					NVIC_SystemReset(); 				
				}
			}
			else
			{
				if(((*(vu32*)(USART_RX_BUF+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
				{
					printf("开始更新固件...\r\n");
					OTA_Begin(FLASH_FW_ADDR, USART_RX_BUF, applenth);
					printf("固件更新完成!\r\n");	
					
					#if 0
					/* FW Running */
					printf("FLASH FW running: 0x%02X \r\n", FLASH_FW_ADDR);

					delay_ms(1000);
					if(((*(vu32*)(FLASH_FW_ADDR+4))&0xFF000000)==0x08000000)//判断OTA里面主程序开始地址是否合法，是否为0X08XXXXXX.
					{
						iap_load_app(FLASH_FW_ADDR);//执行FLASH APP代码
					}
					else
					{
						printf("illegal ota bin FLASH_FW_ADDR: %x \r\n", ((*(vu32*)(FLASH_FW_ADDR+4))&0xFF000000));
					}
					#else
					FLASH_Unlock();//解锁
					Writeflash(NEED_OTA_FLAG_ADDR, 0);//flash 重置
					FLASH_Lock();//上锁
					
					
					NEED_OTA_FLAG = Flash_ReadHalfWord(NEED_OTA_FLAG_ADDR);
					printf("BL system reset, NEED_OTA_FLAG: %d \r\n", NEED_OTA_FLAG);
					delay_ms(1000);//防止快速运行到unlock
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
				
				FLASH_Unlock();//解锁
				Writeflash(NEED_OTA_FLAG_ADDR, 1);//flash 重置
				FLASH_Lock();//上锁
				delay_ms(1000);//防止快速运行到unlock
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
			if(((*(vu32*)(FLASH_FW_ADDR+4))&0xFF000000)==0x08000000)//判断OTA里面主程序开始地址是否合法，是否为0X08XXXXXX.
			{
				iap_load_app(FLASH_FW_ADDR);//执行FLASH APP代码
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
			
			FLASH_Unlock();//解锁
			Writeflash(NEED_OTA_FLAG_ADDR, 1);//flash 重置
			FLASH_Lock();//上锁
			delay_ms(1000);//防止快速运行到unlock
			reset_time = 0;
			
			NEED_OTA_FLAG = Flash_ReadHalfWord(NEED_OTA_FLAG_ADDR);
			printf("BL_2 get NEED_OTA_FLAG: %d \r\n", NEED_OTA_FLAG);
		}
	}
}

