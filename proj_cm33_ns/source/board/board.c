/*******************************************************************************
* File Name: board.c
*
* Description: This file contains board supported API's.
*
* Related Document: See README.md
*
********************************************************************************
* Copyright 2024-2025, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

/*******************************************************************************
* Header file includes
*******************************************************************************/
#include <FreeRTOS.h>
#include <task.h>
#include "board.h"
#include "sensor.h"
#include "bt_utils.h"
#include "bt_app.h"
#include "retarget_io_init.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define PWM_DUTY_CYCLE_0                (0U)
#define PWM_DUTY_CYCLE_50               (1000U)
#define PWM_DUTY_CYCLE_100              (2000U)
#define GPIO_INTERRUPT_PRIORITY         (7U)
#define BOARD_TASK_PRIORITY             (0U)
#define BOARD_TASK_STACK_SIZE           (512U)
#define ULBITS_TO_CLEAR_ON_ENTRY        (0U)
#define ULBITS_TO_CLEAR_ON_EXIT         (0U)
#define NOTIFY_INDEX_MAX                (4U)
#define NOTIFY_INDEX_RESET              (0U)
#define NVM_WRITE_DATA_LENGTH           (1U)
#define NOTIFICAITON_VALUE              (1U)
#define INTERRUPT_MASKED                (1UL)
#define USER_LED_ON                     (1U)
#define USER_LED_OFF                    (0U)
#define SHORT_PRESS_DELAY_MSEC          (10U)
#define LONG_PRESS_DELAY_MSEC           (3000U)
#define BUTTON_SCAN_DELAY_MSEC          (10U)

#if (CY_CFG_PWR_SYS_IDLE_MODE == CY_CFG_PWR_MODE_DEEPSLEEP)

/* PWM Context for SysPm Callback */
mtb_syspm_tcpwm_deepsleep_context_t PWMdscontext =
{
    .channelNum = CYBSP_PWM_LED_CTRL_NUM,
};

/* SysPm callback parameter structure for PWM */
static cy_stc_syspm_callback_params_t PWMDSParams =
{
        .context   = &PWMdscontext,
        .base      = CYBSP_PWM_LED_CTRL_HW
};

/* SysPm callback structure for PWM */
static cy_stc_syspm_callback_t PWMDeepSleepCallbackHandler =
{
    .callback           = &mtb_syspm_tcpwm_deepsleep_callback,
    .skipMode           = CY_SYSPM_SKIP_CHECK_FAIL | CY_SYSPM_SKIP_CHECK_READY,
    .type               = CY_SYSPM_DEEPSLEEP,
    .callbackParams     = &PWMDSParams,
    .prevItm            = NULL,
    .nextItm            = NULL,
    .order              = SYSPM_CALLBACK_ORDER
};

#endif /*CY_CFG_PWR_SYS_IDLE_MODE == CY_CFG_PWR_MODE_DEEPSLEEP) */

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* FreeRTOS task handle for board task. Button task is used to handle button
 * events */
TaskHandle_t  board_task_handle;

/* Variables for button debouncing */
volatile bool button_debouncing = false;
volatile uint32_t button_debounce_timestamp = 0;

/*******************************************************************************
* Function Definition
*******************************************************************************/

/*******************************************************************************
* Function Name: button_interrupt_callback
********************************************************************************
* Summary:
*  GPIO interrupt handler.
*******************************************************************************/
static void button_interrupt_callback(void)
{
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;

    /* Check if the interrupt is from User Button 1 */
    if(INTERRUPT_MASKED == Cy_GPIO_GetInterruptStatusMasked(CYBSP_USER_BTN_PORT,
                CYBSP_USER_BTN_PIN))
    {
        /* Clear the interrupt and pending IRQ for User Button 1*/
        Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN_PORT, CYBSP_USER_BTN_PIN);
        NVIC_ClearPendingIRQ(CYBSP_USER_BTN1_IRQ);
        xTaskNotifyFromISR(board_task_handle,(uint32_t) NOTIFICAITON_VALUE,
               eSetValueWithoutOverwrite, &xHigherPriorityTaskWoken);
    }

    /* CYBSP_USER_BTN1 (SW2) and CYBSP_USER_BTN2 (SW4) share the same port and
     * hence they share the same NVIC IRQ line. Since both the buttons are
     * configured for falling edge interrupt in the BSP, pressing any button
     * will trigger the execution of this ISR. Therefore, we must clear the
     * interrupt flag of the user button (CYBSP_USER_BTN2) to avoid issues in
     * case if user presses BTN2 by mistake.
     */
    Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN2_PORT, CYBSP_USER_BTN2_PIN);
    NVIC_ClearPendingIRQ(CYBSP_USER_BTN2_IRQ);
}

/*******************************************************************************
* Function Name: board_button_init
********************************************************************************
* Summary:
*  Initialize the button with Interrupt.
*******************************************************************************/
static void board_button_init(void)
{
    /* Interrupt config structure */
    cy_stc_sysint_t intrCfg =
    {
        .intrSrc = CYBSP_USER_BTN_IRQ,
        .intrPriority = GPIO_INTERRUPT_PRIORITY
    };

    /* CYBSP_USER_BTN1 (SW2) and CYBSP_USER_BTN2 (SW4) share the same port and
     * hence they share the same NVIC IRQ line. Since both are configured in the
     * BSP via the Device Configurator, the interrupt flags for both the buttons
     * are set right after they get initialized through the call to cybsp_init().
     * The flags must be cleared before initializing the interrupt, otherwise
     * the interrupt line will be constantly asserted. */
    Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN1_PORT, CYBSP_USER_BTN1_PIN);
    Cy_GPIO_ClearInterrupt(CYBSP_USER_BTN2_PORT, CYBSP_USER_BTN2_PIN);
    NVIC_ClearPendingIRQ(CYBSP_USER_BTN1_IRQ);
    NVIC_ClearPendingIRQ(CYBSP_USER_BTN2_IRQ);

    /* Initialize the interrupt and register interrupt callback */
    Cy_SysInt_Init(&intrCfg, &button_interrupt_callback);

    /* Enable the interrupt in the NVIC */
    NVIC_EnableIRQ(intrCfg.intrSrc);
}

/*******************************************************************************
* Function Name: board_led_init
********************************************************************************
* Summary:
*   Initialize the User Led 3 with PWM.
*******************************************************************************/
static void board_led_init(void)
{
    cy_rslt_t result = CY_TCPWM_SUCCESS;

    /* Initialize the TCPWM block for User LED 3*/
    result = Cy_TCPWM_PWM_Init(CYBSP_PWM_LED_CTRL_HW,
            CYBSP_PWM_LED_CTRL_NUM,
            &CYBSP_PWM_LED_CTRL_config);

    /* PWM init failed. Stop program execution */
    if(CY_TCPWM_SUCCESS != result)
    {
        printf("Failed to initialize PWM for LED2.\r\n");
        handle_app_error();
    }

#if (CY_CFG_PWR_SYS_IDLE_MODE == CY_CFG_PWR_MODE_DEEPSLEEP)

    /* SysPm callback registration for PWM */
    Cy_SysPm_RegisterCallback(&PWMDeepSleepCallbackHandler);

#endif

    /* Enable the TCPWM block */
    Cy_TCPWM_PWM_Enable(CYBSP_PWM_LED_CTRL_HW,
            CYBSP_PWM_LED_CTRL_NUM);

    /* Start the PWM */
    Cy_TCPWM_TriggerStart_Single(CYBSP_PWM_LED_CTRL_HW,
            CYBSP_PWM_LED_CTRL_NUM);

    /* Turn LED off during initialization */
    Cy_TCPWM_PWM_SetCompare0(CYBSP_PWM_LED_CTRL_HW,
              CYBSP_PWM_LED_CTRL_NUM,PWM_DUTY_CYCLE_0);

}

/*******************************************************************************
* Function Name: board_task
********************************************************************************
* Summary:
*  This task initialize the board and button events.
*
* Parameters:
*  void *pvParameters: Not used
*
* Return:
*  None
*
*******************************************************************************/
static void board_task(void *pvParameters)
{
    uint32_t notify_value;
    TickType_t last_button_press_time = 0;
    TickType_t long_press_start_time = 0;
    BaseType_t long_press_detected = pdFALSE;

    for(;;)
     {
        /* Block till a notification is received. */
         xTaskNotifyWait(ULBITS_TO_CLEAR_ON_ENTRY,ULBITS_TO_CLEAR_ON_EXIT,
                 &notify_value, portMAX_DELAY);
         last_button_press_time = xTaskGetTickCount();
         long_press_start_time = xTaskGetTickCount();
         long_press_detected = pdFALSE;

         /* Condition for short press and long press of User Button 1*/
         while (Cy_GPIO_Read(CYBSP_SW2_PORT, CYBSP_SW2_PIN) == CYBSP_BTN_PRESSED)
         {
             if(!long_press_detected &&
                   ((xTaskGetTickCount() - long_press_start_time) >=
                   pdMS_TO_TICKS(LONG_PRESS_DELAY_MSEC)))
             {
                 long_press_detected = pdTRUE;
                 printf("USER BUTTON 1 long press is detected!\r\n");
                 bt_app_nvm_reset();
                 printf("\x1b[2J\x1b[;H");
                 printf("Successfully erased notify interval from NVM "
                         "(Non-Volatile-Memory), Please reset the device\r\n");
                 handle_app_error();
             }
             vTaskDelay(pdMS_TO_TICKS(BUTTON_SCAN_DELAY_MSEC));
         }

         if (!long_press_detected &&
            ((xTaskGetTickCount() - last_button_press_time) >=
              pdMS_TO_TICKS(SHORT_PRESS_DELAY_MSEC)))
         {
             printf("User button 1 (SW2) pressed \r\n");
             if(++notify_index >= NOTIFY_INDEX_MAX) notify_index =
                     NOTIFY_INDEX_RESET;
             printf("Notify interval values is set to %d ms \r\n",
                     notify_interval_value[notify_index]);

             /* Write notification interval time to NVM*/
             bt_app_nvm_write((uint8_t*)"NOTIFY_INX", NVM_WRITE_DATA_LENGTH,
                     (uint8_t*)&notify_index);
         }
     }
}

/*******************************************************************************
* Function Name: board_init
********************************************************************************
* Summary:
*  Initialize the NVM and board with LED's and Buttons.
*******************************************************************************/
void board_init(void)
{
    BaseType_t rtos_result;

    /* Initilize the NVM */
    if(CY_RSLT_SUCCESS == bt_app_nvm_init())
    {
        printf("kv-store initialized! \r\n");
    }

    /* Initialize the USER LED 3 with PWM*/
    board_led_init();

    /* Initialize the USER BUTTON */
    board_button_init();

    /* Create Button Task for processing board events */
    rtos_result = xTaskCreate(board_task,"Board Task", BOARD_TASK_STACK_SIZE,
                            NULL, BOARD_TASK_PRIORITY, &board_task_handle);

    if( pdPASS != rtos_result )
    {
        printf("Failed to create board task.\r\n");
        handle_app_error();
    }
}

/*******************************************************************************
* Function Name: board_led_set_state
********************************************************************************
* Summary:
*  Set the led state
*
* Parameters:
*  index: index of LED
*  value: ON/OFF state
*
* Return:
*  None
*
*******************************************************************************/
void board_led_set_state(uint8_t index, bool value)
{
    if(USER_LED1 == index)
    {
        Cy_GPIO_Write(CYBSP_USER_LED1_PORT, CYBSP_USER_LED_PIN,
                value? USER_LED_OFF : USER_LED_ON);
    }
    else
    {
        Cy_TCPWM_PWM_SetCompare0(CYBSP_PWM_LED_CTRL_HW,
          CYBSP_PWM_LED_CTRL_NUM,value?PWM_DUTY_CYCLE_0:
          PWM_DUTY_CYCLE_100);
    }
}

/*******************************************************************************
* Function Name: board_led_set_blink
********************************************************************************
* Summary:
*  Set the User led 3 duty cycle to 50%.
*******************************************************************************/
void board_led_set_blink(void)
{
    Cy_TCPWM_PWM_SetCompare0(CYBSP_PWM_LED_CTRL_HW,
            CYBSP_PWM_LED_CTRL_NUM,PWM_DUTY_CYCLE_50);
}


/* [] END OF FILE */
