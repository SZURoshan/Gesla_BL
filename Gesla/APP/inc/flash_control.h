#ifndef __FLASH_CONTROL_H
#define __FLASH_CONTROL_H

#include "stm32f10x.h"
#include <stdio.h>

//用户根据自己的需要设置
#define STM32_FLASH_SIZE 256 	 		//所选STM32的FLASH容量大小(单位为K)
//#define STM32_FLASH_WREN 1              //使能FLASH写入(0，不是能;1，使能)
//////////////////////////////////////////////////////////////////////////////////////////////////////

//FLASH起始地址
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH的起始地址
#define FLASH_SAVE_ADDR  ((uint32_t)0x08003000) //16k bootload code
#define FLASH_FW_ADDR     ((uint32_t)0x08004000) //16k BootLoader remain

#define NEED_OTA_FLAG_ADDR ((uint32_t)0x08003002)
extern uint16_t NEED_OTA_FLAG;//1需要更新FW  0不需要更新FW


u32 STMFLASH_ReadWord(u32 faddr);
u16 STMFLASH_ReadHalfWord(u32 faddr);

u16 Flash_ReadHalfWord(u32 faddr);
u32 Flash_ReadWord(u32 faddr);
void Flash_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);

void Writeflash(u32 addr,u16 data);

void Flash_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);
uint8_t Flash_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);

void Flash_Test(void);

void FLASH_OTA_Bin_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);

#endif
