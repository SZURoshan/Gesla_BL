#include "OTA.h"
#include "flash_control.h"

/* BSP */
#include "uart_db.h"

//设置栈顶地址
//addr:栈顶地址
#if 1
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}
#endif

iapfun jump2app; 
u16 iapbuf[1024];//从startup文件得知Stack_Size      EQU     0x00000400  ,1k bytes一组
void OTA_Begin(u32 WriteAddr_begin, u8 *pBuffer, u32 ota_bin_length)
{  
	
	u16 t = 0;
	u16 i = 0;
	u16 temp;
	u32 fwaddr = WriteAddr_begin;//当前写入的地址
	u8 *dfu = pBuffer;
	
	for(t=0; t<ota_bin_length; t+=2)
	{						    
		temp  = (u16)dfu[1]<<8;
		temp += (u16)dfu[0];	  
		dfu  += 2;//偏移2个字节
		iapbuf[i++] = temp;	    
		if( i == 1024 )//一次写1k
		{
			printf("OTA bin in %dk half-word\r\n", t/1024);
			i = 0;
			FLASH_OTA_Bin_Write(fwaddr, iapbuf, 1024);	
			fwaddr += 2048;//偏移2048  16=2*8.所以要乘以2.
		}
	}
	
	if(i)
	{
		printf("OTA bin remain %d half-word\r\n", i);
		FLASH_OTA_Bin_Write(fwaddr, iapbuf, i);//将最后的一些内容字节写进去.
	}
}

//跳转到应用程序段
//appxaddr:用户代码起始地址.
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//检查栈顶地址是否合法.
	{ 
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//用户代码区第二个字为程序开始地址(复位地址)		
		MSR_MSP(*(vu32*)appxaddr);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		jump2app();									//跳转到APP.
	}
	else
	{
		printf("illegal ota bin MSP: %x \r\n", ((*(vu32*)appxaddr)&0x2FFE0000) );
	}
}		 
