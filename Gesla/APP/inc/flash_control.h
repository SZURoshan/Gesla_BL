#ifndef __FLASH_CONTROL_H
#define __FLASH_CONTROL_H

#include "stm32f10x.h"
#include <stdio.h>


#define FLASH_SAVE_ADDR  0x08004000 //16k bootload code

u32 STMFLASH_ReadWord(u32 faddr);
u16 STMFLASH_ReadHalfWord(u32 faddr);
void Flash_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);
void Flash_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);
uint8_t Flash_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);
void Flash_Test(void);

#endif
