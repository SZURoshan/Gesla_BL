#include "stdio.h"
#include "FreeRTOSConfig.h"

#include "FreeRTOS.h"
#include "task.h"

uint32_t traceTickCount;

void vApplicationMallocFailedHook( void )
{
	portDISABLE_INTERRUPTS();
	
	while(1);
}

#if (configCHECK_FOR_STACK_OVERFLOW == 1)
void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed portCHAR *pcTaskName)
{
	portDISABLE_INTERRUPTS();
	
	while(1);
}
#endif

void assertFail(char *exp, char *file, int line)
{
	while (1);
}
