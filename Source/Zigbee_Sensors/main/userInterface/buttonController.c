/******************************************************************************
*   Includes
*******************************************************************************/
#include "iot_button.h"
#include "button_gpio.h"
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


/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Private Variables
*******************************************************************************/
static const char * TAG = "BUTTON";

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/


/******************************************************************************
*   Public Functions Definitions
*******************************************************************************/
/***************************************************************************//*!
*  \brief Button controller initialization.
*
*   This function perform the Button Controller module initialization.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  pConfig             Pointer to button configuration.
*
*   \return     Operation status
*
*******************************************************************************/
BUTTON_Ret_t BUTTON_InitController(BUTTON_Config_t const *pConfig){

    ESP_LOGI(TAG, "Controller initialization");

    if(pConfig == NULL){
        ESP_LOGI(TAG, "Failed to init controller: invalid params");
        return BUTTON_STATUS_ERROR;
    }

    button_handle_t btn_handle;
    button_config_t btn_cfg = {0};
    button_gpio_config_t gpio_cfg = {
        .gpio_num = pConfig->gpio,
        .active_level = ((pConfig->active_level == BUTTON_ACTIVE_HIGH) ? 1 : 0),
    };

    //Init new button
    if(ESP_OK != iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &btn_handle)){
        ESP_LOGI(TAG, "Failed to init button");
        return BUTTON_STATUS_ERROR;
    }

    //Config callback
    if(pConfig->shortpress_callback != NULL){
        if(ESP_OK != iot_button_register_cb(btn_handle, 
                                            BUTTON_SINGLE_CLICK, 
                                            NULL, 
                                            pConfig->shortpress_callback, 
                                            NULL)){

            ESP_LOGI(TAG, "Failed to register shortpress callback");
            return BUTTON_STATUS_ERROR;
        }
    }

    if(pConfig->longpress_callback != NULL){
        button_event_args_t args;
        args.long_press.press_time = 5 * 1000;
        if(ESP_OK != iot_button_register_cb(btn_handle, 
                                            BUTTON_LONG_PRESS_START, 
                                            &args, 
                                            pConfig->longpress_callback, 
                                            NULL)){

            ESP_LOGI(TAG, "Failed to register longpress callback");
            return BUTTON_STATUS_ERROR;
        }
    }

    return BUTTON_STATUS_OK;
}

/******************************************************************************
*   Interrupts
*******************************************************************************/