/*******************************************************************************
* File Name: bt_utils.c
*
* Description: This file contains NVM memory access and helper functions.
*
* Related Document: See README.md
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
 * Header file includes
 ******************************************************************************/
#include "wiced_bt_trace.h"
#include "wiced_memory.h"
#include "bt_utils.h"
#include "mtb_kvstore.h"
#include "stdlib.h"
#include "retarget_io_init.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define NVM_CONFIG_MAX_LEN                 (256U)
#define NUM_KEYS                           (8U)
#define MIN_PROGRAM_SIZE                   (0x200U)
#define NVM_REGION_ZERO                    (0U)
#define MULTIPLICATION_FACTOR              (2U)
#define LENGTH_INVALID                     (0U)
#define NVM_ERROR                          (0U)
#define RRAM_NVM_DATA_NS_OFFSET            (CYMEM_CM33_0_user_nvm_OFFSET)

/*******************************************************************************
* Global Variables
*******************************************************************************/
uint8_t erase_buf[CY_RRAM_BLOCK_SIZE_BYTES] = {0};
mtb_kvstore_t kvstore_obj;
static mtb_block_storage_t* bsd;
static mtb_block_storage_t nvm_bsd;
static mtb_hal_nvm_t nvm_obj;
static mtb_hal_nvm_region_info_t region_info;

/*******************************************************************************
* Function Name: get_kvstore_init_params
********************************************************************************
* Summary:
* This function is used to define the bond data storage
*         (in this case NVM).
*
* Parameters:
* uint32_t *start_addr: Start Address to erase the data
*                       from the device.
* uint32_t *length    : Length of the data that needs to be erased
*
* Return:
*  void
*
*******************************************************************************/
static void get_kvstore_init_params(uint32_t *length, uint32_t *start_addr)
 {
     uint32_t program_size = bsd->get_program_size(bsd->context,
             region_info.start_address);
     program_size = (program_size > MIN_PROGRAM_SIZE) ? program_size :
             MIN_PROGRAM_SIZE;
     uint32_t area_size = (NUM_KEYS * program_size * MULTIPLICATION_FACTOR);
     *start_addr = region_info.start_address + RRAM_NVM_DATA_NS_OFFSET;
     *length = area_size;
 }

/*******************************************************************************
* Function Name: app_kvstore_bd_init
********************************************************************************
* Summary:
*  This function initializes the underlying block device.
*******************************************************************************/
static void app_kvstore_bd_init(void)
{
    mtb_hal_nvm_info_t nvm_info;
    mtb_hal_nvm_setup(&nvm_obj,NULL);
    mtb_hal_nvm_get_info(&nvm_obj, &nvm_info);
    mtb_block_storage_create_hal_nvm(&nvm_bsd, &nvm_obj);
    bsd = &nvm_bsd;
    region_info = nvm_info.regions[NVM_REGION_ZERO];
}

/*******************************************************************************
* Function Name: bt_app_nvm_init
********************************************************************************
* Summary:
* This function Initialize the NVM.
*
* Parameters:
*  None
*
* Return:
*  cy_rslt_t : returns the result status.
*
*******************************************************************************/
cy_rslt_t bt_app_nvm_init(void)
{
       cy_rslt_t rslt;
       uint32_t  start_addr, length;
       app_kvstore_bd_init();
       get_kvstore_init_params(&length, &start_addr);

       /*Initialize kv-store library*/
       rslt = mtb_kvstore_init(&kvstore_obj, start_addr, length, bsd,NULL,NULL);

       /*Check if the kv-store initialization was successful*/
       if (CY_RSLT_SUCCESS != rslt)
       {
           printf("failed to initialize kv-store \n");
           handle_app_error();
       }

       return rslt;
}

/*******************************************************************************
* Function Name: bt_app_nvm_read
********************************************************************************
* Summary:
*  This function reads data from the NVM.
*
* Parameters:
*  key : pointer to key value
*  len : data length
*  buf : data buffer
*
* Return:
*  uint32_t : returns the length of data size.
*
*******************************************************************************/
uint32_t bt_app_nvm_read(uint8_t* key, uint32_t len, uint8_t* buf)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if (len >= NVM_CONFIG_MAX_LEN)
    {
        printf("NVM read failed reached maximum length %d!", (uint16_t)len);
        len = LENGTH_INVALID;
    }
    else
    {
        result = mtb_kvstore_read(&kvstore_obj, (const char*)key,
                (uint8_t*)buf, &len);
        if(CY_RSLT_SUCCESS != result)
        {
            printf("Notify interval not stored in NVM!\n");
            len = LENGTH_INVALID;
        }
    }
    return (len);
}

/*******************************************************************************
* Function Name: bt_app_nvm_write
********************************************************************************
* Summary:
* This function write data to NVM.
*
* Parameters:
*  key : pointer to key value
*  len : data length
*  buf : data buffer
*
* Return:
*  uint32_t : returns the length of data size.
*
*******************************************************************************/
uint32_t bt_app_nvm_write(uint8_t* key, uint32_t len, uint8_t* buf)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if (len >= NVM_CONFIG_MAX_LEN)
    {
        printf("NVM write failed reached maximum length %d!", (uint16_t)len);
        len = LENGTH_INVALID;
    }
    else
    {
        result = mtb_kvstore_write(&kvstore_obj, (const char*)key,
                (uint8_t*)buf, len);
        if(CY_RSLT_SUCCESS != result)
        {
            printf("NVM write failed with error code: 0x%x\r\n", (int)result);
            len = LENGTH_INVALID;
        }
    }
    return (len);
}

/*******************************************************************************
* Function Name: bt_app_nvm_delete
********************************************************************************
* Summary:
* This function delete data from the NVM.
*
* Parameters:
*  uint8_t* key
*
* Return:
*  uint32_t : returns the status.
*
*******************************************************************************/
uint32_t bt_app_nvm_delete(uint8_t* key)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    result = mtb_kvstore_delete(&kvstore_obj, (const char*)key);

    if(CY_RSLT_SUCCESS != result)
    {
        printf("NVM delete failed with error code: 0x%x\r\n", (int)result);
        result = NVM_ERROR;
    }
    return result;
}

/*******************************************************************************
* Function Name: bt_app_nvm_reset
********************************************************************************
* Summary:
* This function reset data from the NVM.
*
* Parameters:
*  None
*
* Return:
*  uint32_t : returns the status.
*
*******************************************************************************/
uint32_t bt_app_nvm_reset(void)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    result = mtb_kvstore_reset(&kvstore_obj);

    if(CY_RSLT_SUCCESS != result)
    {
        printf("NVM reset failed with error code: 0x%x\r\n", (int)result);
        result = NVM_ERROR;
    }
    return result;
}
