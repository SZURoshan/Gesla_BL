#include "stm32f10x.h"
#include <stdio.h>

#include "battery.h"
#include "led.h"
#include "vin.h"
#include "motor.h"

#define VBAT_VOL_CHG    1050 
#define VBAT_VOL_OFF    990  

uint16_t bat_vol;


void Battery_Management(void)
{
	bat_vol = Vin_GetVol_X100();		
	
	printf("battery vol: %d \r\n", bat_vol);
	
	if(bat_vol < VBAT_VOL_CHG)  //1050
	{
		LED_Red_Toggle();
		
		if(bat_vol < VBAT_VOL_OFF) //990
		{
			LED_Red_On();
			LED_Green_Off();	
			Motor_A_SetSpeed(0);
			Motor_B_SetSpeed(0);
			Motor_C_SetSpeed(0);
			Motor_D_SetSpeed(0);			
		}
	}
	else
	{
		LED_Red_Off();
		LED_Green_On();
	}
}
