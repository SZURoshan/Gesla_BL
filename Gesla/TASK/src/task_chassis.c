/*
*	l_front:	motor_A	input(+)back	input(-)front	O|----LIDAR(HEAD)----|O		r_front:	motor_B	input(+)back	input(-)front
*													 	 |---------^---------|
*													 	 |---------|---------|
*													 	 |---------|---------|
*													 	 |---------|---------|
*													 	 |---------|---------|
*													 	 |---------|---------|
*													 	 |---------|---------|
*													 	 |---------|---------|
*													 	 |---------|---------|
*	l_back:		motor_C	input(+)front	input(-)back 	O|------ANTENNA------|O		r_back:		motor_D	input(+)front	input(-)back
*	
*/
#include "task_chassis.h"
#include <stdio.h>

#include "kinematics.h"

#include "sysinit.h"

#include "uart_nano.h"
#include "motor.h"
#include "motor_encoder.h"

#include "uart_nano.h"

void Chassis_MoveCtl(void);
int16_t Chassis_Motor_A_Speed_PID(int16_t speed_target, int16_t speed_measure);
int16_t Chassis_Motor_B_Speed_PID(int16_t speed_target, int16_t speed_measure);
int16_t Chassis_Motor_C_Speed_PID(int16_t speed_target, int16_t speed_measure);
int16_t Chassis_Motor_D_Speed_PID(int16_t speed_target, int16_t speed_measure);

#define CHASSIS_SPEED_MAX 800
#define CHASSIS_TASK_TIME_INCREMENT 2

/******************************param define******************************/
int16_t motor_encoder_accumulator[4]  = {0};//for NANO
int16_t motor_encoder_delta[4] 		  = {0};
int16_t motor_encoder_delta_target[4] = {0};

int16_t motor_pwm_final_output[4]	  = {0}; //moter final output

int16_t chassis_odometer[6] 		  = {0}; //for NANO,odometer, absolute value & delta value£¬x y z dx dy dz

//pid
float Chassis_motor_kP = 100;
float Chassis_motor_kI = 0;
float Chassis_motor_kD = 0;

/************************************************************************/
//500Hz
void Task_Chassis(void *pvParameters)
{
	portTickType currentTime;	
	
	for(;;)
	{
		currentTime = xTaskGetTickCount();
		
		/* CODE */

//		printf("wheel_velocity	x:%d y:%d z:%d \r\n", (int16_t)wheel_velocity[0], (int16_t)wheel_velocity[1], (int16_t)wheel_velocity[2]);
		Chassis_MoveCtl();
		
		/*  */
		
		vTaskDelayUntil(&currentTime, CHASSIS_TASK_TIME_INCREMENT);
	}
}
/************************************************************************/


void Chassis_MoveCtl(void)
{ 
	motor_encoder_delta[0] = -( Moter_Encoder_AB_GetCounter() - MOTOR_ENCODER_MID_VALUE );
	motor_encoder_delta[1] =  ( Moter_Encoder_CD_GetCounter() - MOTOR_ENCODER_MID_VALUE );
	motor_encoder_delta[2] = -( Moter_Encoder_EF_GetCounter() - MOTOR_ENCODER_MID_VALUE );
	motor_encoder_delta[3] =  ( Moter_Encoder_GH_GetCounter() - MOTOR_ENCODER_MID_VALUE );

	//Sets the encoder median
	Moter_Encoder_AB_SetCounter(MOTOR_ENCODER_MID_VALUE); 
	Moter_Encoder_CD_SetCounter(MOTOR_ENCODER_MID_VALUE); 
	Moter_Encoder_EF_SetCounter(MOTOR_ENCODER_MID_VALUE); 
	Moter_Encoder_GH_SetCounter(MOTOR_ENCODER_MID_VALUE);

	//Calculate the cumulative value of the encoder
	motor_encoder_accumulator[0] += motor_encoder_delta[0];
	motor_encoder_accumulator[1] += motor_encoder_delta[1];
	motor_encoder_accumulator[2] += motor_encoder_delta[2];
	motor_encoder_accumulator[3] += motor_encoder_delta[3];
	
	constrain_int16_t( chassis_speed_target[0], 2000, 2000);
	constrain_int16_t( chassis_speed_target[1], 2000, 2000);
	constrain_int16_t( chassis_speed_target[2], 2000, 2000);
	
	//Kinematics Analysis
	//analysis the encoder data that send to the Jetson NANO
	//according to the number of degrees the encoder has rotated that calculates how many meters the robot has moved forward
//	Kinematics_Analysis(motor_encoder_accumulator,chassis_odometer);
	
	//Inverse Kinematics Analysis
	//analysis the data from Jetson NANO
	Inverse_Kinematics_Analysis( chassis_speed_target, motor_encoder_delta_target );

	//PID calcu
	motor_pwm_final_output[0] = Chassis_Motor_A_Speed_PID(motor_encoder_delta_target[0], motor_encoder_delta[0]);   
	motor_pwm_final_output[1] = Chassis_Motor_B_Speed_PID(motor_encoder_delta_target[1], motor_encoder_delta[1]);   
	motor_pwm_final_output[2] = Chassis_Motor_C_Speed_PID(motor_encoder_delta_target[2], motor_encoder_delta[2]);   
	motor_pwm_final_output[3] = Chassis_Motor_D_Speed_PID(motor_encoder_delta_target[3], motor_encoder_delta[3]);  
	
//	printf("pid out put0: %d\r\n",motor_encoder_delta[0]);
//	printf("pid out put1: %d\r\n",motor_encoder_delta[1]);
//	printf("pid out put2: %d\r\n",motor_encoder_delta[2]);
//	printf("pid out put3: %d\r\n",motor_encoder_delta[3]);
	if(0)//chassis_speed_target[0] != 0 | chassis_speed_target[1] != 0 | chassis_speed_target[0] != 0)
	{
		Motor_A_SetSpeed( motor_pwm_final_output[0] );
		Motor_B_SetSpeed( motor_pwm_final_output[1] );
		Motor_C_SetSpeed( motor_pwm_final_output[2] );
		Motor_D_SetSpeed( motor_pwm_final_output[3] );
	}
	else
	{
		Motor_A_SetSpeed( 0 );
		Motor_B_SetSpeed( 0 );
		Motor_C_SetSpeed( 0 );
		Motor_D_SetSpeed( 0 );
	}
}


/*--------------------PID--------------------*/
int16_t Chassis_Motor_A_Speed_PID(int16_t speed_target, int16_t speed_measure)
{
	static int32_t motor_pwm_output = 0;
	static int32_t speed_error = 0;
	static int32_t speed_error_last = 0;
	static int32_t speed_error_accum = 0;
	
	int32_t pTermChassis, iTermChassis, dTermChassis;
	
	
	//calc error
	speed_error = speed_target - speed_measure;//for P term
	speed_error_accum += speed_error;//for I term
	printf("speed_error A: %d\r\n",speed_error);
	//P term
	pTermChassis = speed_error * Chassis_motor_kP;
	
	//I term
	iTermChassis = speed_error_accum * Chassis_motor_kI * (CHASSIS_TASK_TIME_INCREMENT * 0.001);
	iTermChassis = constrain_int32_t(iTermChassis, -1000, 1000);
	
	//D term
	dTermChassis = (speed_error - speed_error_last) * Chassis_motor_kD;

	//PID term,final output
	motor_pwm_output = pTermChassis + iTermChassis + dTermChassis;
	motor_pwm_output = constrain_int32_t(motor_pwm_output, -CHASSIS_SPEED_MAX, CHASSIS_SPEED_MAX);
	
	//error update
	speed_error_last = speed_error;
	//printf("chassis pid: %d \r\n", motor_pwm_output);
	return motor_pwm_output;
}

int16_t Chassis_Motor_B_Speed_PID(int16_t speed_target, int16_t speed_measure)
{
	static int32_t motor_pwm_output = 0;
	static int32_t speed_error = 0;
	static int32_t speed_error_last = 0;
	static int32_t speed_error_accum = 0;
	
	int32_t pTermChassis, iTermChassis, dTermChassis;
	
	
	//calc error
	speed_error = speed_target - speed_measure;//for P term
	speed_error_accum += speed_error;//for I term
	printf("speed_error B: %d\r\n",speed_error);
	//P term
	pTermChassis = speed_error * Chassis_motor_kP;
	
	//I term
	iTermChassis = speed_error_accum * Chassis_motor_kI * (CHASSIS_TASK_TIME_INCREMENT * 0.001);
	iTermChassis = constrain_int32_t(iTermChassis, -1000, 1000);
	
	//D term
	dTermChassis = (speed_error - speed_error_last) * Chassis_motor_kD;

	//PID term,final output
	motor_pwm_output = pTermChassis + iTermChassis + dTermChassis;
	motor_pwm_output = constrain_int32_t(motor_pwm_output, -CHASSIS_SPEED_MAX, CHASSIS_SPEED_MAX);
	
	//error update
	speed_error_last = speed_error;
	//printf("chassis pid: %d \r\n", motor_pwm_output);
	return motor_pwm_output;
}

int16_t Chassis_Motor_C_Speed_PID(int16_t speed_target, int16_t speed_measure)
{
	static int32_t motor_pwm_output = 0;
	static int32_t speed_error = 0;
	static int32_t speed_error_last = 0;
	static int32_t speed_error_accum = 0;
	
	int32_t pTermChassis, iTermChassis, dTermChassis;
	
	
	//calc error
	speed_error = speed_target - speed_measure;//for P term
	speed_error_accum += speed_error;//for I term
	printf("speed_error C: %d\r\n",speed_error);
	//P term
	pTermChassis = speed_error * Chassis_motor_kP;
	
	//I term
	iTermChassis = speed_error_accum * Chassis_motor_kI * (CHASSIS_TASK_TIME_INCREMENT * 0.001);
	iTermChassis = constrain_int32_t(iTermChassis, -1000, 1000);
	
	//D term
	dTermChassis = (speed_error - speed_error_last) * Chassis_motor_kD;

	//PID term,final output
	motor_pwm_output = pTermChassis + iTermChassis + dTermChassis;
	motor_pwm_output = constrain_int32_t(motor_pwm_output, -CHASSIS_SPEED_MAX, CHASSIS_SPEED_MAX);
	
	//error update
	speed_error_last = speed_error;
	//printf("chassis pid: %d \r\n", motor_pwm_output);
	return motor_pwm_output;
}

int16_t Chassis_Motor_D_Speed_PID(int16_t speed_target, int16_t speed_measure)
{
	static int32_t motor_pwm_output = 0;
	static int32_t speed_error = 0;
	static int32_t speed_error_last = 0;
	static int32_t speed_error_accum = 0;
	
	int32_t pTermChassis, iTermChassis, dTermChassis;
	
	
	//calc error
	speed_error = speed_target - speed_measure;//for P term
	speed_error_accum += speed_error;//for I term
	printf("speed_error D: %d\r\n",speed_error);
	//P term
	pTermChassis = speed_error * Chassis_motor_kP;
	
	//I term
	iTermChassis = speed_error_accum * Chassis_motor_kI * (CHASSIS_TASK_TIME_INCREMENT * 0.001);
	iTermChassis = constrain_int32_t(iTermChassis, -1000, 1000);
	
	//D term
	dTermChassis = (speed_error - speed_error_last) * Chassis_motor_kD;

	//PID term,final output
	motor_pwm_output = pTermChassis + iTermChassis + dTermChassis;
	motor_pwm_output = constrain_int32_t(motor_pwm_output, -CHASSIS_SPEED_MAX, CHASSIS_SPEED_MAX);
	
	//error update
	speed_error_last = speed_error;
	//printf("chassis pid: %d \r\n", motor_pwm_output);
	return motor_pwm_output;
}


