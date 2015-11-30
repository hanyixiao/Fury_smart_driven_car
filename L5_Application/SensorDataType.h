/*
 * SensorDataType.h
 *
 *  Created on: Oct 21, 2015
 *      Author: Christopher
 */

#ifndef L5_APPLICATION_SENSORDATATYPE_H_
#define L5_APPLICATION_SENSORDATATYPE_H_


typedef struct {
    uint16_t LIDAR;
    uint16_t SonicSensor1;
    uint16_t SonicSensor2;
    uint16_t SonicSensor3;
} SonicSensors_t;

typedef struct {
    uint8_t MotorBattery;
    uint8_t BoardBattery;
    uint8_t BatteryFlag;

} BatterySensors_t;


#endif /* L5_APPLICATION_SENSORDATATYPE_H_ */
