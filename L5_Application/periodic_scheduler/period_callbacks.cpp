/*
 *     SocialLedge.com - Copyright (C) 2013
 *
 *     This file is part of free software framework for embedded processors.
 *     You can use it and/or distribute it as long as this copyright header
 *     remains unmodified.  The code is free for personal use and requires
 *     permission to use in a commercial product.
 *
 *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *     You can reach the author of this software at :
 *          p r e e t . w i k i @ g m a i l . c o m
 */

/**
 * @file
 * This contains the period callback functions for the periodic scheduler
 *
 * @warning
 * These callbacks should be used for hard real-time system, and the priority of these
 * tasks are above everything else in the system (above the PRIORITY_CRITICAL).
 * The period functions SHOULD NEVER block and SHOULD NEVER run over their time slot.
 * For example, the 1000Hz take slot runs periodically every 1ms, and whatever you
 * do must be completed within 1ms.  Running over the time slot will reset the system.
 */

#include <stdint.h>
#include "io.hpp"
#include "periodic_callback.h"
#include "stdio.h"
#include "can_custom_header.hpp"
#include "utilities.h"
#include "file_logger.h"
#include "queue.h"
#include "can.h"
#define rx
#define lidar_threshold 100
#define sonic_threshold 50

#define forward 0xF0
#define reverse 0x00
enum direction {
	straight, far_right, right, left, far_left
};

typedef struct {
	uint16_t SonicSensor1;
	uint16_t SonicSensor2;
	uint16_t SonicSensor3;
} SonicSensor_t;

can_msg_t motor_throttle;
can_msg_t motor_steer;
can_msg_t heart_beat;

int correctDirection = straight;
int previousDirection = straight;

int correctSpeed = forward;
int previousSpeed = reverse;

SonicSensor_t SonicData;
uint16_t lidar = 0;
/// This is the stack size used for each of the period tasks
const uint32_t PERIOD_TASKS_STACK_SIZE_BYTES = (512 * 4);
/*
 *
 *
 *
 can_std_id_t can_test1;
 can_std_id_t can_test2;
 can_fullcan_msg_t *can_ptr;
 can_fullcan_msg_t *can_test1_ptr;
 */
can_fullcan_msg_t can_ptr;

QueueHandle_t can_queue = xQueueCreate(10, sizeof(can_msg_t));



/// Called once before the RTOS is started, this is a good place to initialize things once
bool period_init(void)
{
	motor_throttle.msg_id = 0x022;
	motor_throttle.frame_fields.is_29bit = 0;
	motor_throttle.frame_fields.data_len = 1;       // Send 8 bytes

	motor_steer.msg_id = 0x021;
	motor_steer.frame_fields.is_29bit = 0;
	motor_steer.frame_fields.data_len = 1;       // Send 8 bytes

	heart_beat.msg_id = 0x221;
	heart_beat.frame_fields.is_29bit = 0;
	heart_beat.frame_fields.data_len = 1;       // Send 8 bytes
    return true; // Must return true upon success
}

//Register any telemetry variables
bool period_reg_tlm(void)
{
    // Make sure "SYS_CFG_ENABLE_TLM" is enabled at sys_config.h to use Telemetry
    return true; // Must return true upon success
}



void period_1Hz(void) {
	/*
		 #ifdef rx
		 can_test1_ptr = CAN_fullcan_get_entry_ptr(can_test1);
		 if(CAN_fullcan_read_msg_copy(&can_ptr,can_test1_ptr) == true)
		 {
		 printf("new message : %d \n", can_ptr.data.bytes);

	 else
	 printf("no message at this time \n");
	 #endif

	 #ifdef tx
	 static can_msg_t mess;

	 mess.msg_id = 0x100;
	 mess.frame_fields.is_29bit = 1;
	 mess.frame_fields.data_len = 8;
	 mess.data.qword = 10;
	 if(CAN_tx(can1, &mess, portMAX_DELAY) == true)
	 printf("sent \n");

	 else
	 printf("not sent\n");
	 #endif
	 */

}

void get_sensor_value(void){
	can_msg_t temp = { 0 };
		CAN_rx(can1, &temp, 0);

		switch (temp.msg_id) {
		case 0x142:
			uint16_t temp3, temp2;

			temp2 = (temp.data.words[1] >> 8);
			temp3 = (temp.data.words[1] << 8);
			SonicData.SonicSensor1 = 0;
			SonicData.SonicSensor1 = temp2 | temp3;

			temp2 = (temp.data.words[2] >> 8);
			temp3 = (temp.data.words[2] << 8);
			SonicData.SonicSensor2 = 0;
			SonicData.SonicSensor2 = temp2 | temp3;

			temp2 = (temp.data.words[3] >> 8);
			temp3 = (temp.data.words[3] << 8);
			SonicData.SonicSensor3 = 0;
			SonicData.SonicSensor3 = temp2 | temp3;

			temp2 = (temp.data.words[0] >> 8);
			temp3 = (temp.data.words[0] << 8);
			lidar = 0;
			lidar = temp2 | temp3;
			break;

		case 0x140:
			CAN_tx(can1, &heart_beat, 0);
			break;
			}

}

void Obstruction_avoidance_algorithm(void)
{
	if (lidar < lidar_threshold)
		{
			 if (SonicData.SonicSensor1 < sonic_threshold)
			 {
				correctDirection = right;
			 }
			 else
			 {
				correctDirection = left;
			 }
		}
		else
		{
			if (SonicData.SonicSensor1 < sonic_threshold)
			{
				correctDirection = right;
			}
			else if (SonicData.SonicSensor2 < sonic_threshold)
			{
				correctDirection = left;
			}
			else
			{
				correctDirection = straight;
			}
		}
		motor_steer.data.bytes[0] = correctDirection;
		CAN_tx(can1, &motor_steer, 0);
}

void period_10Hz(void)
{
	get_sensor_value();
	//Obstruction avoidance algorithm
	Obstruction_avoidance_algorithm();
}

void period_100Hz(void) {
	/*can_msg_t msg;
	 if(CAN_rx(can1, &msg,0))
	 {
	 if(!xQueueSend(can_queue, &msg, 0))
	 {
	 printf("couldnt send to queue\n");
	 }
	 }
	 */
}

void period_1000Hz(void) {
	//LE.toggle(4);
}
