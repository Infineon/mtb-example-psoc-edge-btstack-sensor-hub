/*******************************************************************************
* File Name: bt_app.h
*
* Description: This file is the public interface of bt_app.c source file
*
* Related Document: README.md
*
********************************************************************************
* (c) 2024-2026, Infineon Technologies AG, or an affiliate of Infineon
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
*******************************************************************************/
#ifndef BT_APP_H
#define BT_APP_H


/*******************************************************************************
* Header file includes
*******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "wiced_bt_dev.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_uuid.h"
#include "wiced_result.h"
#include "wiced_bt_stack.h"
#include "wiced_memory.h"
#include "cybt_platform_config.h"
#include "cycfg_gatt_db.h"


/*******************************************************************************
* Global constants
*******************************************************************************/

/* Notification parameters */
enum
{
   NOTIFIY_OFF,
   NOTIFIY_ON,
};

enum
{
   NOTIFY_TIME_100MS = 100,
   NOTIFY_TIME_500MS = 500,
   NOTIFY_TIME_1000MS = 1000,
   NOTIFY_TIME_5000MS = 5000, /* 5sec */
};

/*******************************************************************************
* Extern Variables
*******************************************************************************/
extern const uint16_t notify_interval_value[4];
extern uint8_t notify_index;
extern uint8_t notify_enabled;
extern uint8_t notify_motion_sensor;
extern uint8_t notify_temperature_sensor;

/*******************************************************************************
* Function prototype
*******************************************************************************/
void bt_app_send_notification(uint8_t index);
wiced_result_t bt_app_management_cb(wiced_bt_management_evt_t event,
                                wiced_bt_management_evt_data_t *p_event_data);
#endif /* BT_APP_H */
