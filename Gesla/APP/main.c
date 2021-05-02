/*
** A single spark can start a prairie fire.
*/

#include "stm32f10x.h"
#include <stdio.h>

/* APP */
#include "sysinit.h"
#include "flash_control.h"


int main(void)
{	
	System_Init();
	
	while(1)
	{
		Flash_Test();
	}
}

