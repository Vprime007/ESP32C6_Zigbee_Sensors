/******************************************************************************
*   Includes
*******************************************************************************/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_log.h"

#include "buttonController.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/
#define BUTTON_TASK_PERIOD_MS           (10)
#define BUTTON_LONGPRESS_DELAY_MS       (5 * 1000)

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
static void tButtonTask(void *pvParameters);

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Private Variables
*******************************************************************************/
static BUTTON_Config_t button_config = {0};
static TaskHandle_t button_task_handle = NULL;

static uint16_t button_cptr = 0;

static const char * TAG = "BUTTON";

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/
static void tButtonTask(void *pvParameters){

    ESP_LOGI(TAG, "Starting button task");

    for(;;){

        vTaskDelay(BUTTON_TASK_PERIOD_MS/portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

/******************************************************************************
*   Public Functions Definitions
*******************************************************************************/
BUTTON_Ret_t BUTTON_InitController(BUTTON_Config_t *pConfig){

    ESP_LOGI(TAG, "Controller initialization");

    if(pConfig == NULL){
        ESP_LOGI(TAG, "Failed to init controller: invalid params");
        return BUTTON_STATUS_ERROR;
    }

    //Config button
    gpio_config_t gpio_cfg = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pin_bit_mask = pConfig->gpio,
    };
    if(ESP_OK != gpio_config(&gpio_cfg)){
        ESP_LOGI(TAG, "Failed to config gpio");
        return BUTTON_STATUS_ERROR;
    }

    //Store new button config
    button_config.active_level = pConfig->active_level;
    button_config.gpio = pConfig->gpio;

    if(pConfig->shortpress_callback != NULL){
        button_config.shortpress_callback = pConfig->shortpress_callback;
    }
    else{
        button_config.shortpress_callback = NULL;
    }

    if(pConfig->longpress_callback != NULL){
        button_config.longpress_callback = pConfig->longpress_callback;
    }
    else{
        button_config.longpress_callback = NULL;
    }

    //Create button task
    if(pdPASS != xTaskCreate(tButtonTask,
                             "BTN task",
                             2048,
                             NULL,
                             4,
                             &button_task_handle)){

        ESP_LOGI(TAG, "Failed to create button task");
        return BUTTON_STATUS_ERROR;
    }

    return BUTTON_STATUS_OK;
}

/******************************************************************************
*   Interrupts
*******************************************************************************/