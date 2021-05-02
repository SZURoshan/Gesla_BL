#include "Task_Start.h"
#include <stdio.h>

/* FreeRtos includes */
#include "FreeRTOS.h"
#include "task.h"

/* BSP */
#include "uart_db.h"
#include "uart_nano.h"

/* Driver */
#include "battery.h"

/* TASK */
#include "task_chassis.h"

#define STK_SIZE_128 	128 
#define STK_SIZE_256	256
#define STK_SIZE_512 	512

/* task create */
#define START_TASK_PRIO  6  						
TaskHandle_t StartTask_Handler; 			
void Start_Task(void *pvParameters); 		

#define TASK_1000ms_PRIO 2  					
TaskHandle_t  Task_1000ms_Handler; 			
void Task_1000ms(void *pvParameters); 	

#define TASK_Chassis_PRIO 4  					
TaskHandle_t  Task_Chassis_Handler; 			
void Task_Chassis(void *pvParameters); 


void test_task(void* param);

/***********************************************************/
void App_Task_Create(void)
{
	xTaskCreate((TaskFunction_t )Start_Task, 			
				(const char* )"Start_Task", 			
				(uint16_t )STK_SIZE_128, 				
				(void* )NULL, 							
				(UBaseType_t )START_TASK_PRIO, 			
				(TaskHandle_t* )&StartTask_Handler); 	

}


void Start_Task(void *param)
{
	taskENTER_CRITICAL();
	
	/*------------------------------------------------*/
	
	xTaskCreate((TaskFunction_t )Task_1000ms,
				(const char* )"Task_20ms",
				(uint16_t )STK_SIZE_128,
				(void* )NULL,
				(UBaseType_t )TASK_1000ms_PRIO,
				(TaskHandle_t* )&Task_1000ms_Handler);	
				
	//chassis task
	xTaskCreate((TaskFunction_t )Task_Chassis,
				(const char* )"Task_Chassis",
				(uint16_t )STK_SIZE_256,
				(void* )NULL,
				(UBaseType_t )TASK_Chassis_PRIO,
				(TaskHandle_t* )&Task_Chassis_Handler);	
	
	xTaskCreate(test_task, "test_task", 100, NULL, 6, NULL);
	xTaskCreate(test_task, "test_task", 100, NULL, 6, NULL);
	
	/*------------------------------------------------*/
	
	vTaskDelay(500);
	vTaskDelete(StartTask_Handler);
	
	taskEXIT_CRITICAL();
}

void Task_1000ms(void *pvParameters)
{
	for(;;)
	{	
		vTaskDelay(1000);
		
		/* CODE */
		Battery_Management();
	}
}

uint8_t send_test[4] = {0};
void test_task(void* param)
{
	for(;;)
	{
		vTaskDelay(1000);
//		printf("task running \r\n");
		
		//send test
		send_test[0] = send_test[3]+2;
		send_test[1] = send_test[0]+2;
		send_test[2] = send_test[1]+2;
		send_test[3] = send_test[2]+2;
		UART_NANO_SendPacket(send_test, 4);
	}
}
