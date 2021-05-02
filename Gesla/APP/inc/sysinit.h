#ifndef __SYSINIT_H
#define __SYSINIT_H

#include "stm32f10x.h"

void System_Init(void);

int16_t constrain_int16_t(int16_t amt, int16_t low, int16_t high);
float constrain_float(float amt, float low, float high);
int32_t constrain_int32_t(int32_t amt, int32_t low, int32_t high);

#endif
