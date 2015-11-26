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

#include "compass.hpp"
#include <stdint.h>
#include "io.hpp"
#include "periodic_callback.h"
#include "can.h"
#include "gps.hpp"
#include "CompassGPS_calculation.hpp"
#include "can_gpsCompass.hpp"
#include "hashDefine.hpp"
#include "tlm/c_tlm_comp.h"
#include "tlm/c_tlm_var.h"
#include "receive_Canmsg.hpp"

/// This is the stack size used for each of the period tasks
const uint32_t PERIOD_TASKS_STACK_SIZE_BYTES = (512 * 4);

gpsData_t  gpsCurrentData;
float_t    distToDest, distToChkPnt, currentHeading;
double_t   chkPntLat, chkPntLon, desiredHeading;
uint8_t    presentChkPnt, compassMode = 0;

/// Called once before the RTOS is started, this is a good place to initialize things once
bool period_init(void)
{
    CAN_init(can1,100,2,2,NULL,NULL);
    bool canSetup = CAN_fullcan_add_entry(can1,CAN_gen_sid(can1,MASTER_GPSDATA_ID), CAN_gen_sid(can1,MASTER_RESET_ID));
    CAN_reset_bus(can1);

    return true; // Must return true upon success
}

/// Register any telemetry variables
bool period_reg_tlm(void)
{
    // Make sure "SYS_CFG_ENABLE_TLM" is enabled at sys_config.h to use Telemetry
    tlm_component *gpsCompass_cmp = tlm_component_add("GPS_Compass");

    TLM_REG_VAR(gpsCompass_cmp, presentChkPnt, tlm_uint);
    TLM_REG_VAR(gpsCompass_cmp, gpsCurrentData.latitude, tlm_float);
    TLM_REG_VAR(gpsCompass_cmp, gpsCurrentData.longitude, tlm_float);
    TLM_REG_VAR(gpsCompass_cmp, chkPntLat, tlm_double);
    TLM_REG_VAR(gpsCompass_cmp, chkPntLon, tlm_double);
    TLM_REG_VAR(gpsCompass_cmp, distToChkPnt, tlm_float);
    TLM_REG_VAR(gpsCompass_cmp, distToDest, tlm_float);
    TLM_REG_VAR(gpsCompass_cmp, compassMode, tlm_uint);
    TLM_REG_VAR(gpsCompass_cmp, desiredHeading, tlm_double);
    TLM_REG_VAR(gpsCompass_cmp, currentHeading, tlm_float);

    return true; // Must return true upon success
}


void period_1Hz(void)
{
    //heartbeat();
    //can_receive();
}

void period_10Hz(void)
{
    static QueueHandle_t gpsCurrData_q = scheduler_task::getSharedObject("gps_queue");
    double_t presentLat, presentLon;
    static bool finalChkPnt_b = false;
    bool chkPntRchd_b = false;

    if(NULL == gpsCurrData_q)
    {
        LE.on(1);
        LE.on(2);
    }
    else if(xQueueReceive(gpsCurrData_q, &gpsCurrentData, 0))
    {
        LE.off(1);
        LE.off(2);

        presentChkPnt = getPresentChkPnt();

        // present car latitude and longitude
        presentLat = gpsCurrentData.latitude;
        presentLon = gpsCurrentData.longitude;

        //Sending GPS data to master.
        sendGPS_data(&presentChkPnt,&presentLat,&presentLon);

        // latitude and longitude of checkpoint
        chkPntLat = getLatitude(presentChkPnt);
        chkPntLon = getLongitude(presentChkPnt);

        // heading degree of car
        desiredHeading = headingdir(presentLat, presentLon, chkPntLat, chkPntLon);
        if(BEARINGMODE == compassMode)
            currentHeading = compassBearing_inDeg();

        // Distance of checkpoint and final distance
        distToChkPnt = calcDistToNxtChkPnt(presentLat, presentLon, chkPntLat, chkPntLon);
        distToDest = calcDistToFinalDest(distToChkPnt);

        //Sending compass data to master.
        sendCompass_data(currentHeading, desiredHeading, presentChkPnt, distToChkPnt, distToDest);

        // check if the car has reached the checkpoint
        finalChkPnt_b = checkPntReached(distToChkPnt);

        if(finalChkPnt_b)
            destReached();

    }
    else
    {
        LE.toggle(1);
        LE.toggle(2);
    }

    if(CALIBRATIONMODE == compassMode)
        compassMode = compass_calibrationMode(compassMode); //calibration mode

    else if(HEADINGMODE == compassMode)
        compassMode = compass_headingMode();   //To get back to bearing compassMode

    else
        LD.setNumber(13);

    if(SW.getSwitch(2))
    {
        compassMode = 2;//0
        LD.setNumber(10);
    }
    if(SW.getSwitch(1))
        compassMode = 1;
}


void period_100Hz(void)
{
    if(CAN_is_bus_off(can1))
        CAN_reset_bus(can1);
}

void period_1000Hz(void)
{

}
