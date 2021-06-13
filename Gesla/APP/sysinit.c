#include "sysinit.h"
#include <stdio.h>

/* APP */
#include "delay.h"

/* BSP */
#include "uart_db.h"
#include "led.h"

void System_Init(void)
{
	UART1_Reset();
	UART_DB_Init(115200);

	LED_Init();
	delay_init();
	
	printf(" BootLoader init... \r\n");
}

int16_t constrain_int16_t(int16_t amt, int16_t low, int16_t high)
{
	if (amt < low)
			return low;
	else if (amt > high)
			return high;
	else
			return amt;
}

float constrain_float(float amt, float low, float high)
{
	if (amt < low)
			return low;
	else if (amt > high)
			return high;
	else
			return amt;
}

int32_t constrain_int32_t(int32_t amt, int32_t low, int32_t high)
{
	if (amt < low)
			return low;
	else if (amt > high)
			return high;
	else
			return amt;
}
