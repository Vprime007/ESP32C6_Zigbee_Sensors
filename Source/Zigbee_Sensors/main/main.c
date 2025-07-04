/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

/******************************************************************************
*   Includes
*******************************************************************************/
#include <stdio.h>
#include <inttypes.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "zigbeeManager.h"
#include "userInterface.h"
#include "sensorController.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/
#define LOG_LOCAL_LEVEL                 (ESP_LOG_INFO)

/******************************************************************************
*   Private Macros
*******************************************************************************/


/******************************************************************************
*   Private Data Types
*******************************************************************************/


/******************************************************************************
*   Private Functions Declaration
*******************************************************************************/
static void networkChangeCallback(ZIGBEE_Nwk_State_t nwk_state);

static void tMainTask(void *pvParameters);

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Private Variables
*******************************************************************************/
static TaskHandle_t main_task_handle = NULL;

static const char * TAG = "MAIN";

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/
/***************************************************************************//*!
*  \brief App Main.
*
*   This function is the programme entry point.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*******************************************************************************/
void app_main(void){

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    //Create main task
    if(pdTRUE != xTaskCreate(tMainTask,
                             "Main task",
                             3*2048,
                             NULL,
                             4,
                             &main_task_handle)){

        ESP_LOGI(TAG, "Failed to create main task");
        while(1);
    }
}

/***************************************************************************//*!
*  \brief Network change callback.
*
*   This function is called at every network state transition with the new
*   state as parameter.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*******************************************************************************/
static void networkChangeCallback(ZIGBEE_Nwk_State_t nwk_state){

    ESP_LOGI(TAG, "New network state: %d", nwk_state);

    switch(nwk_state){
        case ZIGBEE_NWK_NOT_CONNECTED:
        {
            UI_PostEvent(UI_EVENT_NOT_CONNECTED, 0);
        }
        break;

        case ZIGBEE_NWK_CONNECTED:
        {
            UI_PostEvent(UI_EVENT_CONNECTED, 0);
        }
        break;

        case ZIGBEE_NWK_NO_PARENT:
        {
            UI_PostEvent(UI_EVENT_NO_COORDO, 0);
        }
        break;

        case ZIGBEE_NWK_SCANNING:
        {
            UI_PostEvent(UI_EVENT_SCANNING, 0);
        }
        break;

        default:
        {
            //Do nothing...
        }
        break;
    }
}

/***************************************************************************//*!
*  \brief Main task.
*
*   Main task.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  pvParameters
*
*******************************************************************************/
static void tMainTask(void *pvParameters){

    ESP_LOGI(TAG, "Starting Main task");

    //Init User Interface
    if(UI_STATUS_OK != UI_Init()){
        ESP_LOGI(TAG, "Failed to init UI");
    }

    //Init Zigbee stack
    if(ZIGBEE_STATUS_OK != ZIGBEE_InitStack(networkChangeCallback)){
        ESP_LOGI(TAG, "Failed to init Zigbee stack");
    }
    else{
        //Start Zigbee stack
        ZIGBEE_StartStack();
    }

    //Init sensor
    if(SENSOR_STATUS_OK != SENSOR_InitController()){
        ESP_LOGI(TAG, "Failed to init sensor");
    }

    for(;;){

        vTaskDelay(1000/portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

/******************************************************************************
*   Public Functions Definitions
*******************************************************************************/


/******************************************************************************
*   Interrupts
*******************************************************************************/

