/*
 * sensors.hpp
 *
 *  Created on: Nov 7, 2015
 *      Author: Bhargava
 */

#ifndef L5_APPLICATION_SENSORS_HPP_
#define L5_APPLICATION_SENSORS_HPP_
#include "CAN_base_class.hpp"

class sensor_class:public CAN_base_class
{
public:
	uint16_t heart_beat = 0x140;
	uint16_t sensors = 0x142;
	uint16_t battery = 0x144;
	uint16_t left = 0;
	uint16_t right = 0;
	uint16_t lidar = 0;
	uint16_t back = 0;
	bool get_sensor_reading();
	bool sensor_class_init();
};




#endif /* L5_APPLICATION_SENSORS_HPP_ */
