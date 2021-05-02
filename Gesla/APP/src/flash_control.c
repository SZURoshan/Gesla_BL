#include "flash_control.h"


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
	for(i=0; i<NumToRead; i++)
	{
		pBuffer[i] = Flash_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr += 2;//ƫ��2���ֽ�.	
	}
}



void Flash_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{ 			 		 
	u16 i;
	for(i=0; i<NumToWrite; i++)
	{
		FLASH_ProgramHalfWord(WriteAddr, pBuffer[i]);
	    WriteAddr += 2;//��ַ����2.
	}  
} 

#define FLASH_MAX_SIZE 8 //��д�������ֽ���(8 bits),���ܳ���F1��256*1024
u16 STMFLASH_BUF[FLASH_MAX_SIZE/2]; //flash�ǰ��ֲ��� 16 bits
uint8_t Flash_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{  
 	uint16_t remain_times = 0;//��ʣ���ٰ���Ҫ�޸�
	
	if(WriteAddr<FLASH_SAVE_ADDR 
		|| (WriteAddr >= (FLASH_SAVE_ADDR+FLASH_MAX_SIZE)) 
			|| NumToWrite > FLASH_MAX_SIZE )
	{
		FLASH_Lock();//����
		printf("illegal address or operation\r\n");
		return 0;
	}
	else
	{
		FLASH_Unlock();//����
		
		uint32_t offaddr   = WriteAddr - FLASH_SAVE_ADDR;		//ʵ��ƫ�Ƶ�ַ.ȥ��0X08000000��ĵ�ַ
		uint32_t secpos    = offaddr / FLASH_MAX_SIZE;			//������ַ  0~127 for STM32F103RBT6
		uint16_t secoff    = (offaddr%FLASH_MAX_SIZE) / 2;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)������ƫ�Ƶ�ַ(16λ�ּ���)
		uint16_t secremain = FLASH_MAX_SIZE/2 - secoff;		//����ʣ��ռ��С  (16λ�ּ���)	 
		
		printf("halfword remain: %d \r\n", secremain);
		
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
		printf("read flash: %s \r\n", datatemp);
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

