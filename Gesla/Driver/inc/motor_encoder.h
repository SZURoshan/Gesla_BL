#ifndef __MOTOR_ENCODER_H
#define __MOTOR_ENCODER_H

#include "stm32f10x.h"


#define MOTOR_ENCODER_MID_VALUE  30000  

//OpenCRP驱动接口函数
void Moter_Encoder_AB_Init(uint16_t cycle);          //编码器初始化
uint16_t Moter_Encoder_AB_GetCounter(void);          //编码器获取计数器数值
void Moter_Encoder_AB_SetCounter(uint16_t count);    //编码器设置计数器数值

void Moter_Encoder_CD_Init(uint16_t cycle);          //编码器初始化
uint16_t Moter_Encoder_CD_GetCounter(void);          //编码器获取计数器数值
void Moter_Encoder_CD_SetCounter(uint16_t count);    //编码器设置计数器数值

void Moter_Encoder_EF_Init(uint16_t cycle);          //编码器初始化
uint16_t Moter_Encoder_EF_GetCounter(void);          //编码器获取计数器数值
void Moter_Encoder_EF_SetCounter(uint16_t count);    //编码器设置计数器数值

void Moter_Encoder_GH_Init(uint16_t cycle);          //编码器初始化
uint16_t Moter_Encoder_GH_GetCounter(void);          //编码器获取计数器数值
void Moter_Encoder_GH_SetCounter(uint16_t count);    //编码器设置计数器数值

#endif
