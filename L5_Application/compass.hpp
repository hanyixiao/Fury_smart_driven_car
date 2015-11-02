/*
 * compass.h
 *
 *  Created on: Oct 24, 2015
 *      Author: Tejeshwar
 */

#ifndef L5_APPLICATION_COMPASS_HPP_
#define L5_APPLICATION_COMPASS_HPP_

#include "scheduler_task.hpp"
#include "i2c2.hpp"
#include "i2c2_device.hpp"
#include "i2c_base.hpp"
#include "stdio.h"
#include "io.hpp"
#include "utilities.h"
#include "math.h"

/*Calculates heading direction for a full circle*/
void compassbearing_reading();

/*Calculates heading direction*/
void compassbearingreading_highlowbytes();

/*Calculates pitch angle*/
void pitchangle();

/*Calculates roll angle*/
void rollangle();

/*Calculates temperature*/
void temperature();

/*Compasss enters into calibration when this function is called*/
uint8_t calibrate_compass(uint8_t mode);

/*Compass enters into heading mode when this function is called*/
uint8_t headingmode_compass();


#endif /* L5_APPLICATION_COMPASS_HPP_ */
