#include "flash_control.h"


/* BSP */
#include "uart_db.h"


uint16_t NEED_OTA_FLAG = 0;//1��Ҫ����FW  0����Ҫ����FW
uint16_t NEED_BL_RESET_FLAG = 0;//1��Ҫ����BL  0����Ҫ����BL

extern uint8_t OTA_FLASH_FLAG;
//Ҫд�뵽STM32 FLASH���ַ�������
const u8 TEXT_Buffer[]={"STM32 FLASH TEST"};

const u8 ZERO_Buffer[]={"Sanger"};

//��ȡָ����ַ�İ���(16λ����) 
//faddr:����ַ 
//����ֵ:��Ӧ����.
u32 Flash_ReadWord(u32 faddr)
{
	return *(vu32*)faddr; 
} 

u16 Flash_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void Flash_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)
{
	u16 i;
	printf("[flash][OTA] Flash_Read...\r\n");
	for(i=0; i<NumToRead; i++)
	{
		pBuffer[i] = Flash_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr += 2;//ƫ��2���ֽ�.	
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
	    WriteAddr += 2;//��ַ����2.
	}  
} 

//���� FLASH_ERROR_PG ʱ����������д��
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
#define FLASH_MAX_SIZE 8 //��д�������ֽ���(8 bits),���ܳ���F1��256*1024
u16 STMFLASH_BUF[FLASH_MAX_SIZE/2]; //flash�ǰ��ֲ��� ����Ϊ16 bits
uint8_t Flash_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{  
 	uint16_t remain_times = 0;//��ʣ���ٰ���Ҫ�޸�
	
	if(WriteAddr<FLASH_SAVE_ADDR 
		|| (WriteAddr >= (FLASH_SAVE_ADDR+FLASH_MAX_SIZE)) )
			//|| NumToWrite > FLASH_MAX_SIZE )
	{
		FLASH_Lock();//����
		printf("[flash][OTA] illegal address or operation\r\n");
		return 0;
	}
	else
	{
		FLASH_Unlock();//����
		
		uint32_t offaddr   = WriteAddr - FLASH_SAVE_ADDR;		//ʵ��ƫ�Ƶ�ַ.ȥ��0X08000000��ĵ�ַ
		uint32_t secpos    = offaddr / FLASH_MAX_SIZE;			//������ַ  0~127 for STM32F103RBT6
		uint16_t secoff    = (offaddr%FLASH_MAX_SIZE) / 2;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)������ƫ�Ƶ�ַ(16λ�ּ���)
		uint16_t secremain = FLASH_MAX_SIZE/2 - secoff;		//����ʣ��ռ��С  (16λ�ּ���)	 
		
		printf("[flash][OTA] halfword remain: %d \r\n", secremain);
		
		if(NumToWrite <= secremain)
		{	
			secremain = NumToWrite;//�����ڸ�������Χ
		}
		
		while(1) 
		{	
			Flash_Read(secpos*FLASH_MAX_SIZE+FLASH_SAVE_ADDR, STMFLASH_BUF, FLASH_MAX_SIZE/2);//������������������
			
			//flash�ǰ��ֲ���,һ�β���16 bits,�����ֽ���Ҫ����2
			for( remain_times=0; remain_times<secremain; remain_times++)//У������
			{
				if(STMFLASH_BUF[secoff+remain_times] != 0XFFFF)//�������ݲ���ff
				{
					break;//��Ҫ����
				}					
			}
			
			if(remain_times < secremain)//��Ҫ����
			{
				FLASH_ErasePage(secpos*FLASH_MAX_SIZE+FLASH_SAVE_ADDR);//�����������
				for(remain_times=0; remain_times<secremain; remain_times++)//����
				{
					STMFLASH_BUF[remain_times+secoff] = pBuffer[remain_times];	  
				}
				Flash_Write_NoCheck(secpos*FLASH_MAX_SIZE+FLASH_SAVE_ADDR, STMFLASH_BUF, FLASH_MAX_SIZE/2);//д����������  
			}
			else 
			{
				Flash_Write_NoCheck(WriteAddr, pBuffer, secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 	
			}
			
			if(NumToWrite == secremain)
			{
				break;//д�������
			}
			else//д��δ����
			{
				secpos++;				//������ַ��1
				secoff=0;				//ƫ��λ��Ϊ0 	 
				pBuffer+=secremain;  	//ָ��ƫ��
				WriteAddr+=secremain;	//д��ַƫ��	   
				NumToWrite-=secremain;	//�ֽ�(16λ)���ݼ�
				if( NumToWrite > (FLASH_MAX_SIZE/2) )
				{
					secremain=FLASH_MAX_SIZE/2;//��һ����������д����
				}
				else 
				{
					secremain=NumToWrite;//��һ����������д����
				}
			}	 
		}
		
		FLASH_Lock();//����
		
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
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else 
#define STM_SECTOR_SIZE	2048   //1k������Ϊһ������, ������2048 bytesΪһ�����㵥λ
#endif	
u16 FLASH_OTA_BUF[STM_SECTOR_SIZE/2];//�����2K�ֽ�
void FLASH_OTA_Bin_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   //������ַ
	u16 secoff;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
	u16 secremain; //������ʣ���ַ(16λ�ּ���)	   
 	u16 i;    
	u32 offaddr;   //ȥ��0X08000000��ĵ�ַ
	
	if( WriteAddr < FLASH_FW_ADDR
			|| (WriteAddr >= (STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)) )
	{
		FLASH_Lock();//����
		printf("[flash][OTA] illegal address or operation\r\n");
		return ;//�Ƿ���ַ
	}
	
	FLASH_Unlock();						//����
	
	offaddr   = WriteAddr - STM32_FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
	secpos    = offaddr / STM_SECTOR_SIZE;			//������ַ, 1k������Ϊһ������
	secoff    = (offaddr%STM_SECTOR_SIZE) / 2;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	secremain = STM_SECTOR_SIZE/2 - secoff;		//����ʣ��ռ��С   
	
	printf("[flash][OTA] start to write ota bin...\r\n");
	printf("[flash][OTA] WriteAddr: 0x%08X, offaddr: 0x%08X, secpos: %d, secoff: %d, secremain: %d \r\n", WriteAddr, offaddr, secpos, secoff, secremain); 
	if(NumToWrite<=secremain)
	{
		secremain=NumToWrite;//�����ڸ�������Χ
	}
	
	while(1) 
	{	
		Flash_Read( secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE, FLASH_OTA_BUF, STM_SECTOR_SIZE/2 );//������������������

		FLASH_ErasePage(secpos*STM_SECTOR_SIZE + STM32_FLASH_BASE);//�����������
		
		for(i=0; i<secremain; i++)//����
		{
			FLASH_OTA_BUF[i+secoff]=pBuffer[i];	  
		}
		printf("[flash][OTA] need erase...\r\n");
		Flash_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE, FLASH_OTA_BUF, STM_SECTOR_SIZE/2);//д���������� 			

		
		if(NumToWrite==secremain)
		{
			printf("[flash][OTA] FLASH_OTA_Bin_Write finish...\r\n");
			break;//д�������
		}
		else//д��δ����
		{
			secpos++;				//������ַ��1
			secoff = 0;				//ƫ��λ��Ϊ0 	 
		   	pBuffer    += secremain;  	//ָ��ƫ��
			WriteAddr  += secremain;	//д��ַƫ��	   
		   	NumToWrite -= secremain;	//�ֽ�(16λ)���ݼ�
			
			if( NumToWrite > (STM_SECTOR_SIZE/2) )
			{
				secremain = STM_SECTOR_SIZE/2;//��һ����������д����
			}
			else
			{
				secremain = NumToWrite;//��һ����������д����
			}
			printf("[flash][OTA] next sector to write ...\r\n");
		}	 
	};	
	FLASH_Lock();//����
}
