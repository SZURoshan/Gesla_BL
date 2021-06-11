#include "OTA.h"
#include "flash_control.h"

/* BSP */
#include "uart_db.h"

//����ջ����ַ
//addr:ջ����ַ
#if 1
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}
#endif

iapfun jump2app; 
u16 iapbuf[1024];//��startup�ļ���֪Stack_Size      EQU     0x00000400  ,1k bytesһ��
void OTA_Begin(u32 WriteAddr_begin, u8 *pBuffer, u32 ota_bin_length)
{  
	
	u16 t = 0;
	u16 i = 0;
	u16 temp;
	u32 fwaddr = WriteAddr_begin;//��ǰд��ĵ�ַ
	u8 *dfu = pBuffer;
	
	for(t=0; t<ota_bin_length; t+=2)
	{						    
		temp  = (u16)dfu[1]<<8;
		temp += (u16)dfu[0];	  
		dfu  += 2;//ƫ��2���ֽ�
		iapbuf[i++] = temp;	    
		if( i == 1024 )//һ��д1k
		{
			printf("OTA bin in %dk half-word\r\n", t/1024);
			i = 0;
			FLASH_OTA_Bin_Write(fwaddr, iapbuf, 1024);	
			fwaddr += 2048;//ƫ��2048  16=2*8.����Ҫ����2.
		}
	}
	
	if(i)
	{
		printf("OTA bin remain %d half-word\r\n", i);
		FLASH_OTA_Bin_Write(fwaddr, iapbuf, i);//������һЩ�����ֽ�д��ȥ.
	}
}

//��ת��Ӧ�ó����
//appxaddr:�û�������ʼ��ַ.
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//���ջ����ַ�Ƿ�Ϸ�.
	{ 
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
		MSR_MSP(*(vu32*)appxaddr);					//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
		jump2app();									//��ת��APP.
	}
	else
	{
		printf("illegal ota bin MSP: %x \r\n", ((*(vu32*)appxaddr)&0x2FFE0000) );
	}
}		 
