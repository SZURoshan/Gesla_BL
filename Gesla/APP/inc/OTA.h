#ifndef __OTA_H
#define __OTA_H

#include "stm32f10x.h"
#include <stdio.h>

#define OTA_USART_REC_LEN  			32*1024 	//定义最大接收字节数 32	K

typedef  void (*iapfun)(void);				//定义一个函数类型的参数. 
void OTA_Begin(u32 WriteAddr_begin, u8 *pBuffer, u32 ota_bin_length);
void iap_load_app(u32 appxaddr);			//跳转到APP程序执行


#endif
