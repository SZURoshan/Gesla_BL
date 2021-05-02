#ifndef __KINEMATICS_H
#define __KINEMATICS_H


#include "stm32f10x.h"


#define PI 3.1415926

void Kinematics_Analysis(int16_t* input, int16_t* output);
void Inverse_Kinematics_Analysis(int16_t* input, int16_t* output);


#endif
