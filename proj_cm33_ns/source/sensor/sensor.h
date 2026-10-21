/******************************************************************************
* File Name: sensor.h
*
* Description: This file is the public interface of sensor.c
*
* Related Document: See README.md
*
********************************************************************************
* (c) 2023-2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/
/*******************************************************************************
 * Include guard
 ******************************************************************************/
#ifndef SENSOR_H_
#define SENSOR_H_


/*******************************************************************************
 * Header file includes
 ******************************************************************************/
#include "mtb_bmi270.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cybsp.h"
#include <stdlib.h>
#include "cy_result.h"


/*******************************************************************************
 * Data structure and enumeration
 ******************************************************************************/
/* Structure used for storing APP data */
typedef struct
{
    struct bmi2_sens_data data;
    uint8_t orientation;
    uint8_t temperature;
} sensor_data_t;

/* Sensor index values */
enum sensor_type
{
    MOTION_SENSOR,
    TEMPERATURE_SENSOR,
};

/*******************************************************************************
* Variable Definitions
*******************************************************************************/
extern sensor_data_t app_data;

/******************************************************************************
 * Function Definitions
 ******************************************************************************/
void sensor_task(void* pvParameters);
void get_motion_data(void);
void get_temperature(void);
void get_orientation(uint8_t *orientation);

#endif /* SENSOR_H_ */
