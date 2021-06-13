#include "flash_control.h"


/* BSP */
#include "uart_db.h"


uint16_t NEED_OTA_FLAG = 0;//1需要更新FW  0不需要更新FW
uint16_t NEED_BL_RESET_FLAG = 0;//1需要重启BL  0不需要重启BL

extern uint8_t OTA_FLASH_FLAG;
//要写入到STM32 FLASH的字符串数组
const u8 TEXT_Buffer[]={"STM32 FLASH TEST"};

const u8 ZERO_Buffer[]={"Sanger"};

//读取指定地址的半字(16位数据) 
//faddr:读地址 
//返回值:对应数据.
u32 Flash_ReadWord(u32 faddr)
{
	return *(vu32*)faddr; 
} 

u16 Flash_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}

//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数
void Flash_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)
{
	u16 i;
	printf("[flash][OTA] Flash_Read...\r\n");
	for(i=0; i<NumToRead; i++)
	{
		pBuffer[i] = Flash_ReadHalfWord(ReadAddr);//读取2个字节.
		ReadAddr += 2;//偏移2个字节.	
	}
}

void Flash_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{ 			 		 
	u16 i;
	printf("[flash][OTA] Flash_Write_NoCheck...\r\n");
	for(i=0; i<NumToWrite; i++)
	{
		//printf("[flash][OTA] NoCheck WriteAddr: 0x%08X, data: %04x \r\n", WriteAddr, pBuffer[i]);
		FLASH_ProgramHalfWord(WriteAddr, pBuffer[i]);
	    WriteAddr += 2;//地址增加2.
	}  
} 

//出现 FLASH_ERROR_PG 时，换用这种写法
void Writeflash(u32 addr,u16 data)
{
	volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;

	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASHStatus = FLASH_ErasePage(addr);

	if(FLASHStatus == FLASH_COMPLETE)
	{
		FLASH_ProgramHalfWord(addr,data);
	}
	FLASH_Lock();
}

/************************************flash test*********************************************/
#define FLASH_MAX_SIZE 8 //可写入的最大字节数(8 bits),不能超过F1的256*1024
u16 STMFLASH_BUF[FLASH_MAX_SIZE/2]; //flash是半字操作 半字为16 bits
uint8_t Flash_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{  
 	uint16_t remain_times = 0;//还剩多少半字要修改
	
	if(WriteAddr<FLASH_SAVE_ADDR 
		|| (WriteAddr >= (FLASH_SAVE_ADDR+FLASH_MAX_SIZE)) )
			//|| NumToWrite > FLASH_MAX_SIZE )
	{
		FLASH_Lock();//上锁
		printf("[flash][OTA] illegal address or operation\r\n");
		return 0;
	}
	else
	{
		FLASH_Unlock();//解锁
		
		uint32_t offaddr   = WriteAddr - FLASH_SAVE_ADDR;		//实际偏移地址.去掉0X08000000后的地址
		uint32_t secpos    = offaddr / FLASH_MAX_SIZE;			//扇区地址  0~127 for STM32F103RBT6
		uint16_t secoff    = (offaddr%FLASH_MAX_SIZE) / 2;		//在扇区内的偏移(2个字节为基本单位.)扇区内偏移地址(16位字计算)
		uint16_t secremain = FLASH_MAX_SIZE/2 - secoff;		//扇区剩余空间大小  (16位字计算)	 
		
		printf("[flash][OTA] halfword remain: %d \r\n", secremain);
		
		if(NumToWrite <= secremain)
		{	
			secremain = NumToWrite;//不大于该扇区范围
		}
		
		while(1) 
		{	
			Flash_Read(secpos*FLASH_MAX_SIZE+FLASH_SAVE_ADDR, STMFLASH_BUF, FLASH_MAX_SIZE/2);//读出整个扇区的内容
			
			//flash是半字操作,一次操作16 bits,所以字节数要除以2
			for( remain_times=0; remain_times<secremain; remain_times++)//校验数据
			{
				if(STMFLASH_BUF[secoff+remain_times] != 0XFFFF)//半字内容不是ff
				{
					break;//需要擦除
				}					
			}
			
			if(remain_times < secremain)//需要擦除
			{
				FLASH_ErasePage(secpos*FLASH_MAX_SIZE+FLASH_SAVE_ADDR);//擦除这个扇区
				for(remain_times=0; remain_times<secremain; remain_times++)//复制
				{
					STMFLASH_BUF[remain_times+secoff] = pBuffer[remain_times];	  
				}
				Flash_Write_NoCheck(secpos*FLASH_MAX_SIZE+FLASH_SAVE_ADDR, STMFLASH_BUF, FLASH_MAX_SIZE/2);//写入整个扇区  
			}
			else 
			{
				Flash_Write_NoCheck(WriteAddr, pBuffer, secremain);//写已经擦除了的,直接写入扇区剩余区间. 	
			}
			
			if(NumToWrite == secremain)
			{
				break;//写入结束了
			}
			else//写入未结束
			{
				secpos++;				//扇区地址增1
				secoff=0;				//偏移位置为0 	 
				pBuffer+=secremain;  	//指针偏移
				WriteAddr+=secremain;	//写地址偏移	   
				NumToWrite-=secremain;	//字节(16位)数递减
				if( NumToWrite > (FLASH_MAX_SIZE/2) )
				{
					secremain=FLASH_MAX_SIZE/2;//下一个扇区还是写不完
				}
				else 
				{
					secremain=NumToWrite;//下一个扇区可以写完了
				}
			}	 
		}
		
		FLASH_Lock();//上锁
		
		return 1;
	}
}

void Flash_Test(void)
{
	uint8_t datatemp[FLASH_MAX_SIZE] = {0};
	
	if(OTA_FLASH_FLAG == 1)
	{
		//clean OTA FLAG
		OTA_FLASH_FLAG = 0;		
		Flash_Read(FLASH_SAVE_ADDR, (u16*)datatemp, sizeof(FLASH_MAX_SIZE));
		printf("[flash][OTA] read flash: %s \r\n", datatemp);
	}
	else if(OTA_FLASH_FLAG == 2)
	{
		Flash_Write(FLASH_SAVE_ADDR,(u16*)ZERO_Buffer, sizeof(ZERO_Buffer));
		OTA_FLASH_FLAG = 0;	
	}
	else if(OTA_FLASH_FLAG == 3)
	{
		Flash_Write(FLASH_SAVE_ADDR,(u16*)TEXT_Buffer, sizeof(TEXT_Buffer));
		OTA_FLASH_FLAG = 0;	
	}
}
/*********************************************************************************/


#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //字节
#else 
#define STM_SECTOR_SIZE	2048   //1k个半字为一个扇区, 所以是2048 bytes为一个计算单位
#endif	
u16 FLASH_OTA_BUF[STM_SECTOR_SIZE/2];//最多是2K字节
void FLASH_OTA_Bin_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   //扇区地址
	u16 secoff;	   //扇区内偏移地址(16位字计算)
	u16 secremain; //扇区内剩余地址(16位字计算)	   
 	u16 i;    
	u32 offaddr;   //去掉0X08000000后的地址
	
	if( WriteAddr < FLASH_FW_ADDR
			|| (WriteAddr >= (STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)) )
	{
		FLASH_Lock();//上锁
		printf("[flash][OTA] illegal address or operation\r\n");
		return ;//非法地址
	}
	
	FLASH_Unlock();						//解锁
	
	offaddr   = WriteAddr - STM32_FLASH_BASE;		//实际偏移地址.
	secpos    = offaddr / STM_SECTOR_SIZE;			//扇区地址, 1k个半字为一个扇区
	secoff    = (offaddr%STM_SECTOR_SIZE) / 2;		//在扇区内的偏移(2个字节为基本单位.)
	secremain = STM_SECTOR_SIZE/2 - secoff;		//扇区剩余空间大小   
	
	printf("[flash][OTA] start to write ota bin...\r\n");
	printf("[flash][OTA] WriteAddr: 0x%08X, offaddr: 0x%08X, secpos: %d, secoff: %d, secremain: %d \r\n", WriteAddr, offaddr, secpos, secoff, secremain); 
	if(NumToWrite<=secremain)
	{
		secremain=NumToWrite;//不大于该扇区范围
	}
	
	while(1) 
	{	
		Flash_Read( secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE, FLASH_OTA_BUF, STM_SECTOR_SIZE/2 );//读出整个扇区的内容

		FLASH_ErasePage(secpos*STM_SECTOR_SIZE + STM32_FLASH_BASE);//擦除这个扇区
		
		for(i=0; i<secremain; i++)//复制
		{
			FLASH_OTA_BUF[i+secoff]=pBuffer[i];	  
		}
		printf("[flash][OTA] need erase...\r\n");
		Flash_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE, FLASH_OTA_BUF, STM_SECTOR_SIZE/2);//写入整个扇区 			

		
		if(NumToWrite==secremain)
		{
			printf("[flash][OTA] FLASH_OTA_Bin_Write finish...\r\n");
			break;//写入结束了
		}
		else//写入未结束
		{
			secpos++;				//扇区地址增1
			secoff = 0;				//偏移位置为0 	 
		   	pBuffer    += secremain;  	//指针偏移
			WriteAddr  += secremain;	//写地址偏移	   
		   	NumToWrite -= secremain;	//字节(16位)数递减
			
			if( NumToWrite > (STM_SECTOR_SIZE/2) )
			{
				secremain = STM_SECTOR_SIZE/2;//下一个扇区还是写不完
			}
			else
			{
				secremain = NumToWrite;//下一个扇区可以写完了
			}
			printf("[flash][OTA] next sector to write ...\r\n");
		}	 
	};	
	FLASH_Lock();//上锁
}
