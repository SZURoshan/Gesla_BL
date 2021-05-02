#ifndef __VIN_H
#define __VIN_H

#include "stm32f10x.h"

void Vin_Init(void); //input vol init
uint16_t Vin_GetVol_X100(void);  //get input vol

#endif
