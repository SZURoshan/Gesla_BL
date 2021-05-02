#ifndef __MOTOR_H
#define __MOTOR_H


#include "stm32f10x.h"

void Motor_Init(uint8_t freq_khz);
void Motor_A_SetSpeed(int16_t speed);
void Motor_B_SetSpeed(int16_t speed);
void Motor_C_SetSpeed(int16_t speed);
void Motor_D_SetSpeed(int16_t speed);

#endif
