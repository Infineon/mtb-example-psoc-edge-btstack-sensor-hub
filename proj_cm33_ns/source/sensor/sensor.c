/*******************************************************************************
* File Name:   sensor.c
*
* Description: This file contains the sensor tasks and drivers
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
 * Header file includes
*******************************************************************************/
#include "retarget_io_init.h"
#include <FreeRTOS.h>
#include <task.h>
#include "inttypes.h"
#include "bt_app.h"
#include "bt_utils.h"
#include "board.h"
#include "sensor.h"
#include "mtb_bmi270.h"


/*******************************************************************************
* Macros
*******************************************************************************/
#define MEMCPY_SIZE_MOTION_SENSOR                (6U)
#define MEMCPY_SIZE_TEMP_SENSOR                  (1U)
#define SCALING_FACTOR                           (512.0F)
#define OFFSET                                   (23.0F)
#define MOTION_SENSOR_ACC_INDEX                  (0U)
#define MOTION_SENSOR_GYRO_INDEX                 (6U)
#define MOTION_SENSOR_ORIENTAION_INDEX           (12U)
#define TEMPERATURE_SENSOR_INDEX                 (0U)
#define ZERO                                     (0U)


/*******************************************************************************
 * Structures
 ******************************************************************************/
static mtb_hal_i2c_t CYBSP_I2C_CONTROLLER_hal_obj;
cy_stc_scb_i2c_context_t CYBSP_I2C_CONTROLLER_context;
cy_en_scb_i2c_status_t initStatus;

/*******************************************************************************
* Global constants
*******************************************************************************/

/* Orientation types:
 * Indicates which edge of the board is pointing towards the ceiling/sky */
typedef enum
{
    ORIENTATION_NULL            = 0,    /* Default orientation state used for
                                         * initialization purposes */
    ORIENTATION_TOP_EDGE        = 1,    /* Top edge of the board points towards
                                         * the ceiling */
    ORIENTATION_BOTTOM_EDGE     = 2,    /* Bottom edge of the board points
                                         * towards the ceiling */
    ORIENTATION_LEFT_EDGE       = 3,    /* Left edge of the board
                                         * (USB connector side) points towards
                                         * the ceiling */
    ORIENTATION_RIGHT_EDGE      = 4,    /* Right edge of the board points
                                         * towards the ceiling */
    ORIENTATION_DISP_UP         = 5,    /* Display faces up (towards the
                                         * sky/ceiling) */
    ORIENTATION_DISP_DOWN       = 6     /* Display faces down (towards the
                                         * ground) */
} orientation_t;

const char* orientation[] = {"UNKNOWN", "TOP_EDGE", "BOTTON_EDGE", "LEFT_EDGE",
                                        "RIGHT_EDGE", "DISP_UP", "DISP_DOWN"};

/*******************************************************************************
* Global Variables
*******************************************************************************/

/* Application data objects */
sensor_data_t app_data;

/* Instance of structure to define BMI2 sensor configurations */
struct bmi2_dev dev;
float temp_data_float ;
cy_rslt_t result;
mtb_bmi270_data_t bmi270_data;
mtb_bmi270_t bmi270;

/*******************************************************************************
* Function Definitions
*******************************************************************************/
/*******************************************************************************
* Function Name: sensor_task
********************************************************************************
* Summary:
*  The function initialize the sensor IO's and interface.
*
* Parameters:
*  void *param : Task parameter defined during task creation (unused)
*
* Return
*  None
*
*******************************************************************************/
void sensor_task(void* param)
{
    /* Status variable to indicate the result of various operations */
    cy_rslt_t result;

    /* To avoid compiler warnings. */
    CY_UNUSED_PARAMETER(result);

    /*Initialize I2C */
    result = Cy_SCB_I2C_Init(CYBSP_I2C_CONTROLLER_HW,
                                    &CYBSP_I2C_CONTROLLER_config,
                                    &CYBSP_I2C_CONTROLLER_context);

    if (CY_SCB_I2C_SUCCESS != result)
    {
        printf("I2C Init Failed\n");
        handle_app_error();
    }

    /* Enable I2C */
    Cy_SCB_I2C_Enable(CYBSP_I2C_CONTROLLER_HW);
    mtb_hal_i2c_setup(&CYBSP_I2C_CONTROLLER_hal_obj,
                                    &CYBSP_I2C_CONTROLLER_hal_config,
                                    &CYBSP_I2C_CONTROLLER_context,
                                    NULL);

    /* Initialize the BMI270 sensor */
    result = mtb_bmi270_init_i2c(&bmi270,
            &CYBSP_I2C_CONTROLLER_hal_obj,
            MTB_BMI270_ADDRESS_DEFAULT);

    if(CY_RSLT_SUCCESS != result)
    {
        printf("Failed to init the BMI270 sensor!\r\n");
        handle_app_error();
    }
    printf("BMI270 Sensor initialization done!\r\n");
    
    /* Configure the BMI270 sensor */
    result = mtb_bmi270_config_default(&bmi270);

    if(CY_RSLT_SUCCESS != result)
    {
        printf("Sensor:failed to configure the BMI270 sensor!\r\n");
        handle_app_error();
    }

    for(;;)
    {
        /* Check for a notification status. */

        if( NOTIFIY_ON == notify_motion_sensor)
        {
            get_motion_data();

            /* Motion data to be sent */
            memcpy(&app_sensor_hub_motion_sensor[MOTION_SENSOR_ACC_INDEX],
                    &app_data.data.acc, MEMCPY_SIZE_MOTION_SENSOR);
            memcpy(&app_sensor_hub_motion_sensor[MOTION_SENSOR_GYRO_INDEX],
                    &app_data.data.gyr, MEMCPY_SIZE_MOTION_SENSOR);
            app_sensor_hub_motion_sensor[MOTION_SENSOR_ORIENTAION_INDEX]
                                         = app_data.orientation;
            bt_app_send_notification(MOTION_SENSOR);
        }

        if(NOTIFIY_ON == notify_temperature_sensor)
        {
            get_temperature();

            /* Temperature data to be sent */
            memcpy(&app_sensor_hub_temperature_sensor[TEMPERATURE_SENSOR_INDEX],
                    &app_data.temperature, MEMCPY_SIZE_TEMP_SENSOR);
            bt_app_send_notification(TEMPERATURE_SENSOR);
        }

        /* Notification Delay */
        vTaskDelay(notify_interval_value[notify_index]);
    }
}

/*******************************************************************************
* Function Name: get_motion_data
********************************************************************************
* Summary:
*  The function reads the motion sensor data and copy to the structure.
*******************************************************************************/
void get_motion_data(void)
{
    /* Read x, y, z components of acceration */
    mtb_bmi270_read(&bmi270, &bmi270_data);
    printf("Motion Sensor:acc: X:%6d Y:%6d Z:%6d\r\n", bmi270_data.sensor_data.
            acc.x,bmi270_data.sensor_data.acc.y,bmi270_data.sensor_data.acc.z);
    printf("Motion Sensor:gyr : X:%6d Y:%6d Z:%6d\r\n", bmi270_data.sensor_data.
            gyr.x,bmi270_data.sensor_data.gyr.y,bmi270_data.sensor_data.gyr.z);
    memcpy(&app_data.data,&(bmi270_data.sensor_data),
            sizeof(bmi270_data.sensor_data));
    get_orientation(&app_data.orientation);
    printf("Motion Sensor:orientation = %s\r\n",
            orientation[app_data.orientation]);
}

/*******************************************************************************
* Function Name: get_temperature
********************************************************************************
* Summary:
*  The function reads the thermistor values and copies to the structure.
*******************************************************************************/
void get_temperature(void)
{
    uint16_t temp_data = ZERO;

    /* Read temperature sensor value */
    result = mtb_bmi270_read_temp(&bmi270,&temp_data);

    if (CY_RSLT_SUCCESS != result)
    {
    printf("read data failed\r\n");
    }
    temp_data_float  = (float)((((float)((int16_t)temp_data)) /
            SCALING_FACTOR) + OFFSET);
    app_data.temperature = (uint32_t)temp_data_float;
    printf("temperature = %f degree celcius \r\n",(double)temp_data_float);
}

/*******************************************************************************
* Function Name: get_orientation
********************************************************************************
* Summary:
*  The function reads the orientation values.
*
* Parameters:
*  uint8_t *orientation : pointer to get orientation values 
*
* Return
*  None
*
*******************************************************************************/
void get_orientation(uint8_t *orientation)
{
    int comp_val = 0;
    
    /* Variables used to store absolute values of the accerometer data */
    uint16_t abs_x, abs_y, abs_z;

    /* Clear the previous orientation data */
    *orientation = ORIENTATION_NULL;

    /* Get the absolute values of the accerations along each axes */
    abs_x = abs(bmi270_data.sensor_data.acc.x);
    abs_y = abs(bmi270_data.sensor_data.acc.y);
    abs_z = abs(bmi270_data.sensor_data.acc.z);

    /* Z axis (perpendicular to face of the display) is most aligned
     * with gravity. */
    if ((abs_z > abs_x) && (abs_z > abs_y))
    {
        if (comp_val > bmi270_data.sensor_data.acc.z)
        {
            /* Display faces down (towards the ground) */
            *orientation = ORIENTATION_DISP_DOWN;
        }
        else
        {
            /* Display faces up (towards the sky/ceiling) */
            *orientation = ORIENTATION_DISP_UP;
        }
    }

    /* Y axis (parallel with shorter edge of board) is most aligned
     * with gravity. */
    else if ((abs_y > abs_x) && (abs_y > abs_z))
    {
        if (comp_val < bmi270_data.sensor_data.acc.y)
        {
            /* Display has an inverted landscape orientation */
            *orientation = ORIENTATION_BOTTOM_EDGE;
        }
        else
        {
            /* Display has landscape orientation */
            *orientation = ORIENTATION_TOP_EDGE;
        }
    }
    /* X axis (parallel with longer edge of board) is most aligned
     * with gravity. */
    else
    {
        if (comp_val > bmi270_data.sensor_data.acc.x)
        {
            /* Display has an inverted portrait orientation */
            *orientation = ORIENTATION_RIGHT_EDGE;
        }
        else
        {
            /* Display has portrait orientation */
            *orientation = ORIENTATION_LEFT_EDGE;
        }
    }
}

/* END OF FILE [] */
