/******************************************************************************
*   Includes
*******************************************************************************/
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

#include "esp_log.h"

#include "ledController.h"
#include "sequencer.h"
#include "main.h"

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
typedef enum LED_Pattern_Flag_e{
    LED_FLAG_SCANNING = 0x0001,
    LED_FLAG_CONNECTED = 0x0002,
    LED_FLAG_NO_COORDO = 0x0004,
    LED_FLAG_IDENTIFY = 0x0008,
}LED_Pattern_Flag_t;

/******************************************************************************
*   Private Functions Declaration
*******************************************************************************/
static void tSequencerTask(void *pvParameters);
static void tLedTask(void *pvParameters);

static void redLedTimerCallback(TimerHandle_t xTimer);
static void greenLedTimerCallback(TimerHandle_t xTimer);

static void processRedLedEvent(void);
static void processGreenLedEvent(void);

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Private Variables
*******************************************************************************/
//Boot led sequence
const SEQUENCE_t seq_boot = {
    .init_time_off = SEQUENCER_MS_TO_TIC(500),
    .time_on = SEQUENCER_MS_TO_TIC(5000),
    .time_off = SEQUENCER_MS_TO_TIC(100),
    .nb_repeat = 0,
};

//Identify led sequence
const SEQUENCE_t seq_identify = {
    .init_time_off = SEQUENCER_MS_TO_TIC(250),
    .time_on = SEQUENCER_MS_TO_TIC(500),
    .time_off = SEQUENCER_MS_TO_TIC(500),
    .nb_repeat = SEQUENCE_REPEAT_FOREVER,
};

//Factory reset led sequence
const SEQUENCE_t seq_factory_reset = {
    .init_time_off = SEQUENCER_MS_TO_TIC(250),
    .time_on = SEQUENCER_MS_TO_TIC(500),
    .time_off = SEQUENCER_MS_TO_TIC(500),
    .nb_repeat = 5,
};

//Scanning led sequence
const SEQUENCE_t seq_scanning = {
    .init_time_off = SEQUENCER_MS_TO_TIC(250),
    .time_on = SEQUENCER_MS_TO_TIC(500),
    .time_off = SEQUENCER_MS_TO_TIC(500),
    .nb_repeat = SEQUENCE_REPEAT_FOREVER,
};

//Always ON led sequence
const SEQUENCE_t seq_always_on = {
    .init_time_off = SEQUENCER_MS_TO_TIC(250),
    .time_on = SEQUENCE_ACTIVE_FOREVER,
    .time_off = 0,
};

//Always OFF led sequence
const SEQUENCE_t seq_always_off = {
    .init_time_off = SEQUENCER_MS_TO_TIC(250),
    .time_on = 0,
    .time_off = SEQUENCE_ACTIVE_FOREVER,
};

static SemaphoreHandle_t led_mutex_handle = NULL;
static TaskHandle_t seq_task_handle = NULL;
static TaskHandle_t led_task_handle = NULL;

//Red led related variables
static LED_Handle_t red_led_handle = LED_DRIVER_HANDLE_INVALID;
static LED_Pattern_Flag_t red_led_flag = 0;
static LED_Pattern_t red_led_current_pattern = LED_PATTERN_INVALID;
static LED_Pattern_t red_led_buffered_pattern = LED_PATTERN_INVALID;
static TimerHandle_t red_led_timer_handle = NULL;
static SemaphoreHandle_t red_led_semph_handle = NULL;

//Green led related variables
static LED_Handle_t green_led_handle = LED_DRIVER_HANDLE_INVALID;
static LED_Pattern_Flag_t green_led_flag = 0;
static LED_Pattern_t green_led_current_pattern = LED_PATTERN_INVALID;
static LED_Pattern_t green_led_buffered_pattern = LED_PATTERN_INVALID;
static TimerHandle_t green_led_timer_handle = NULL;
static SemaphoreHandle_t green_led_semph_handle = NULL;

static const char * TAG = "LED";

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/
/***************************************************************************//*!
*  \brief Red led timer callback
*
*   This function is called after the red led timeout occur.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  xTimer          timer handle
*
*******************************************************************************/
static void redLedTimerCallback(TimerHandle_t xTimer){
    xSemaphoreGive(red_led_semph_handle);
}

/***************************************************************************//*!
*  \brief Green led timer callback
*
*   This function is called after the green led timeout occur.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  xTimer          timer handle
*
*******************************************************************************/
static void greenLedTimerCallback(TimerHandle_t xTimer){
    xSemaphoreGive(green_led_semph_handle);
}

/***************************************************************************//*!
*  \brief Sequencer task
*
*   This function is the sequencer task.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  pvParameters            task parameters
*
*******************************************************************************/
static void tSequencerTask(void *pvParameters){

    ESP_LOGI(TAG, "Starting Sequencer task");

    for(;;){
        vTaskDelay(SEQUENCER_TIC_PERIOD_MS/portTICK_PERIOD_MS);
        SEQUENCER_Tic();
    }
    vTaskDelete(NULL);
}

/***************************************************************************//*!
*  \brief Led task
*
*   This function is the Led task. It managed all led patterns and transitions.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  pvParameters            task parameters
*
*******************************************************************************/
static void tLedTask(void *pvParameters){

    ESP_LOGI(TAG, "Starting Leds task");

    for(;;){
        
        //Check if there is red led event to process
        if(pdPASS == xSemaphoreTake(red_led_semph_handle, 0)){
            processRedLedEvent();
        }

        //Check if there is green led event to process
        if(pdPASS == xSemaphoreTake(green_led_semph_handle, 0)){
            processGreenLedEvent();
        }

        vTaskDelay(10/portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

/***************************************************************************//*!
*  \brief Process red led event
*
*   This function is used to process red led events.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*******************************************************************************/
static void processRedLedEvent(void){

    LED_Pattern_t pattern = LED_PATTERN_INVALID;
    static LED_Pattern_Flag_t prev_flag = 0;

    xSemaphoreTake(led_mutex_handle, portMAX_DELAY);
    pattern = red_led_current_pattern;
    xSemaphoreGive(led_mutex_handle);

    switch(pattern){
        case LED_PATTERN_BOOT:
        {
            xSemaphoreTake(led_mutex_handle, portMAX_DELAY);
            red_led_current_pattern = LED_PATTERN_INVALID;
            xSemaphoreGive(led_mutex_handle);

            //Start sequence
            prev_flag = 0;
            SEQUENCER_DoSequence(SEQUENCE_ID_RED_LED, &seq_boot);

            //Schedule led update after the sequence
            xTimerStop(red_led_timer_handle, 10/portTICK_PERIOD_MS);
            xTimerChangePeriod(red_led_timer_handle, (5000+500)/portTICK_PERIOD_MS, 10/portTICK_PERIOD_MS);
            xTimerStart(red_led_timer_handle, 10/portTICK_PERIOD_MS);
        }
        break;

        case LED_PATTERN_FACTORY_RESET:
        {
            xSemaphoreTake(led_mutex_handle, portMAX_DELAY);
            red_led_current_pattern = LED_PATTERN_INVALID;
            xSemaphoreGive(led_mutex_handle);

            //Start sequence
            prev_flag = 0;
            SEQUENCER_DoSequence(SEQUENCE_ID_RED_LED, &seq_factory_reset);

            //Schedule led update after the sequence
            xTimerStop(red_led_timer_handle, 10/portTICK_PERIOD_MS);
            xTimerChangePeriod(red_led_timer_handle, (5000+500)/portTICK_PERIOD_MS, 10/portTICK_PERIOD_MS);
            xTimerStart(red_led_timer_handle, 10/portTICK_PERIOD_MS);
        }
        break;

        case LED_PATTERN_INVALID:
        default:
        {
            LED_Pattern_Flag_t led_flag = 0;

            xSemaphoreTake(led_mutex_handle, portMAX_DELAY);
            red_led_buffered_pattern = LED_PATTERN_INVALID;
            led_flag = red_led_flag;
            xSemaphoreGive(led_mutex_handle);

            if((led_flag & LED_FLAG_IDENTIFY) == LED_FLAG_IDENTIFY){

                if((prev_flag & LED_FLAG_IDENTIFY) != LED_FLAG_IDENTIFY){
                    prev_flag = LED_FLAG_IDENTIFY;
                    SEQUENCER_DoSequence(SEQUENCE_ID_RED_LED, &seq_identify);
                }
            }
            else if((led_flag & LED_FLAG_NO_COORDO) == LED_FLAG_NO_COORDO){

                if((prev_flag & LED_FLAG_NO_COORDO) != LED_FLAG_NO_COORDO){
                    prev_flag = LED_FLAG_NO_COORDO;
                    SEQUENCER_DoSequence(SEQUENCE_ID_RED_LED, &seq_always_on);
                }
            }
            else{
                prev_flag = 0;
                SEQUENCER_DoSequence(SEQUENCE_ID_RED_LED, &seq_always_off);
            }
        }
        break;
    }
}

/***************************************************************************//*!
*  \brief Process green led event
*
*   This function is used to process green led events.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*******************************************************************************/
static void processGreenLedEvent(void){

    LED_Pattern_t pattern = LED_PATTERN_INVALID;
    static LED_Pattern_Flag_t prev_flag = 0;

    xSemaphoreTake(led_mutex_handle, portMAX_DELAY);
    pattern = green_led_current_pattern;
    xSemaphoreGive(led_mutex_handle);

    switch(pattern){
        case LED_PATTERN_BOOT:
        {
            xSemaphoreTake(led_mutex_handle, portMAX_DELAY);
            green_led_current_pattern = LED_PATTERN_INVALID;
            xSemaphoreGive(led_mutex_handle);

            //Start sequence
            prev_flag = 0;
            SEQUENCER_DoSequence(SEQUENCE_ID_GREEN_LED, &seq_boot);

            //Schedule led update after the sequence
            xTimerStop(green_led_timer_handle, 10/portTICK_PERIOD_MS);
            xTimerChangePeriod(green_led_timer_handle, (5000+500)/portTICK_PERIOD_MS, 10/portTICK_PERIOD_MS);
            xTimerStart(green_led_timer_handle, 10/portTICK_PERIOD_MS);
        }
        break;

        case LED_PATTERN_INVALID:
        default:
        {
            LED_Pattern_Flag_t led_flag = 0;

            xSemaphoreTake(led_mutex_handle, portMAX_DELAY);
            green_led_buffered_pattern = LED_PATTERN_INVALID;
            led_flag = green_led_flag;
            xSemaphoreGive(led_mutex_handle);

            if((led_flag & LED_FLAG_IDENTIFY) == LED_FLAG_IDENTIFY){

                if((prev_flag & LED_FLAG_IDENTIFY) != LED_FLAG_IDENTIFY){
                    prev_flag = LED_FLAG_IDENTIFY;
                    SEQUENCER_DoSequence(SEQUENCE_ID_GREEN_LED, &seq_identify);
                }
            }
            else if((led_flag & LED_FLAG_SCANNING) == LED_FLAG_SCANNING){

                if((prev_flag & LED_FLAG_SCANNING) != LED_FLAG_SCANNING){
                    prev_flag = LED_FLAG_SCANNING;
                    SEQUENCER_DoSequence(SEQUENCE_ID_GREEN_LED, &seq_scanning);
                }
            }
            else if((led_flag & LED_FLAG_CONNECTED) == LED_FLAG_CONNECTED){

                if((prev_flag & LED_FLAG_CONNECTED) != LED_FLAG_CONNECTED){
                    prev_flag = LED_FLAG_CONNECTED;
                    SEQUENCER_DoSequence(SEQUENCE_ID_GREEN_LED, &seq_always_on);
                }
            }
            else{
                prev_flag = 0;
                SEQUENCER_DoSequence(SEQUENCE_ID_GREEN_LED, &seq_always_off);
            }
        }
        break;
    }
}

/******************************************************************************
*   Public Functions Definitions
*******************************************************************************/
/***************************************************************************/ /*!
*  \brief LED controller initialization.
*
*   Function to initialized the LED controller module.
*
*   Preconditions: None.
*
*   Side Effects: None.
*   
*   \return operation status
*
*******************************************************************************/
LED_Ret_t LED_InitController(void){
    
    ESP_LOGI(TAG, "LED controller initialization");

    //Create led mutex
    led_mutex_handle = xSemaphoreCreateMutex();
    if(led_mutex_handle == NULL){
        ESP_LOGI(TAG, "Failed to create led mutex");
        return LED_STATUS_ERROR;
    }

    //create leds binary semaphores
    red_led_semph_handle = xSemaphoreCreateBinary();
    if(red_led_semph_handle == NULL){
        ESP_LOGI(TAG, "Failed to create red led semaphore");
        return LED_STATUS_ERROR;
    }

    green_led_semph_handle = xSemaphoreCreateBinary();
    if(green_led_semph_handle == NULL){
        ESP_LOGI(TAG, "Failed to create green led semaphore");
        return LED_STATUS_ERROR;
    }

    //create leds timers
    red_led_timer_handle = xTimerCreate("Red Led timer",
                                        1000/portTICK_PERIOD_MS,
                                        pdFALSE,
                                        (void*)0,
                                        redLedTimerCallback);

    if(red_led_timer_handle == NULL){
        ESP_LOGI(TAG, "Failed to create red led timer");
        return LED_STATUS_ERROR;
    }

    green_led_timer_handle = xTimerCreate("Green Led timer",
                                          1000/portTICK_PERIOD_MS,
                                          pdFALSE,
                                          (void*)0,
                                          greenLedTimerCallback);

    if(green_led_timer_handle == NULL){
        ESP_LOGI(TAG, "Failed to create green led timer");
        return LED_STATUS_ERROR;
    }

    //init led driver and add leds
    if(LDRV_STATUS_OK != LDRV_InitDriver()){
        ESP_LOGI(TAG, "Failed to init led driver");
        return LED_STATUS_ERROR;
    }

    LDRV_CFG_Single_Pwm_Config_t red_led_cfg = {
        .active_level = LDRV_CFG_ACTIVE_HIGH,
        .gpio_num = HWI_RED_LED_GPIO,
        .led_channel = LEDC_CHANNEL_0,
        .led_timer = LEDC_TIMER_0,
    };
    if(LDRV_STATUS_OK != LDRV_AddLedSinglePwm(red_led_cfg, &red_led_handle)){
        ESP_LOGI(TAG, "Failed to add red led");
        return LED_STATUS_ERROR;
    }

    LDRV_CFG_Single_Pwm_Config_t green_led_cfg = {
        .active_level = LDRV_CFG_ACTIVE_HIGH,
        .gpio_num = HWI_GREEN_LED_GPIO,
        .led_channel = LEDC_CHANNEL_1,
        .led_timer = LEDC_TIMER_1,
    };
    if(LDRV_STATUS_OK != LDRV_AddLedSinglePwm(green_led_cfg, &green_led_handle)){
        ESP_LOGI(TAG, "Failed to add green led");
        return LED_STATUS_ERROR;
    }

    red_led_current_pattern = LED_PATTERN_INVALID;
    red_led_buffered_pattern = LED_PATTERN_INVALID;
    red_led_flag = 0;

    green_led_current_pattern = LED_PATTERN_INVALID;
    green_led_buffered_pattern = LED_PATTERN_INVALID;
    green_led_flag = 0;

    //create sequencer task
    if(pdTRUE != xTaskCreate(tSequencerTask,
                             "Seq Task",
                             2048,
                             NULL,
                             5,
                             &seq_task_handle)){
        
        ESP_LOGI(TAG, "Failed to create sequencer task");
        return LED_STATUS_ERROR;
    }

    //create leds task
    if(pdTRUE != xTaskCreate(tLedTask,
                             "Led task",
                             2048,
                             NULL,
                             6,
                             &led_task_handle)){

        ESP_LOGI(TAG, "Failed to create leds task");
        return LED_STATUS_ERROR;
    }

    return LED_STATUS_OK;
}

/***************************************************************************/ /*!
*  \brief Start Led pattern
*
*   Function used to start a Led pattern.
*
*   Preconditions: None.
*
*   Side Effects: None.
*   
*   \param[in]  pattern         Led pattern to start   
*
*   \return operation status
*
*******************************************************************************/
LED_Ret_t LED_StartPattern(LED_Pattern_t pattern){

    if(pattern >= LED_PATTERN_INVALID){
        return LED_STATUS_ERROR;
    }

    switch(pattern){

        case LED_PATTERN_BOOT:
        {
            ESP_LOGI(TAG, "Starting Boot pattern");

            xSemaphoreTake(led_mutex_handle, portMAX_DELAY);
            red_led_current_pattern = LED_PATTERN_BOOT;
            red_led_buffered_pattern = LED_PATTERN_BOOT;

            green_led_current_pattern = LED_PATTERN_BOOT;
            green_led_buffered_pattern = LED_PATTERN_BOOT;
            xSemaphoreGive(led_mutex_handle);

            //Stop any running timer
            xTimerStop(red_led_timer_handle, 10/portTICK_PERIOD_MS);
            xTimerStop(green_led_timer_handle, 10/portTICK_PERIOD_MS);

            //Notify the task to apply new pattern
            xSemaphoreGive(red_led_semph_handle);
            xSemaphoreGive(green_led_semph_handle);
        }
        break;

        case LED_PATTERN_FACTORY_RESET:
        {
            ESP_LOGI(TAG, "Starting Factory Reset pattern");

            xSemaphoreTake(led_mutex_handle, portMAX_DELAY);
            red_led_current_pattern = LED_PATTERN_FACTORY_RESET;
            red_led_buffered_pattern = LED_PATTERN_FACTORY_RESET;
            xSemaphoreGive(led_mutex_handle);

            //Stop any running timer
            xTimerStop(red_led_timer_handle, 10/portTICK_PERIOD_MS);

            //Notify the task to apply new pattern
            xSemaphoreGive(red_led_semph_handle);
        }
        break;

        case LED_PATTERN_IDENTIFY:
        {
            ESP_LOGI(TAG, "Starting Identify pattern");

            LED_Pattern_t red_buffered_pattern = LED_PATTERN_INVALID;
            LED_Pattern_t green_buffered_pattern = LED_PATTERN_INVALID;

            xSemaphoreTake(led_mutex_handle, portTICK_PERIOD_MS);
            red_led_flag |= LED_FLAG_IDENTIFY;
            red_buffered_pattern = red_led_buffered_pattern;

            green_led_flag |= LED_FLAG_IDENTIFY;
            green_buffered_pattern = green_led_buffered_pattern;
            xSemaphoreGive(led_mutex_handle);

            if(red_buffered_pattern == LED_PATTERN_INVALID){
                //Re-apply invalid pattern
                xSemaphoreGive(red_led_semph_handle);
            }

            if(green_buffered_pattern == LED_PATTERN_INVALID){
                //Re-apply invalid pattern
                xSemaphoreGive(green_led_semph_handle);
            }
        }
        break;

        case LED_PATTERN_CONNECTED:
        {
            ESP_LOGI(TAG, "Starting Connected pattern");

            LED_Pattern_t buffered_pattern = LED_PATTERN_INVALID;

            xSemaphoreTake(led_mutex_handle, portMAX_DELAY);
            green_led_flag |= LED_FLAG_CONNECTED;
            buffered_pattern = green_led_buffered_pattern;
            xSemaphoreGive(led_mutex_handle);

            if(buffered_pattern == LED_PATTERN_INVALID){
                //Re-apply invalid pattern
                xSemaphoreGive(green_led_semph_handle);
            }
        }
        break;

        case LED_PATTERN_NO_COORDO:
        {
            ESP_LOGI(TAG, "Starting No-Coordo pattern");

            LED_Pattern_t buffered_pattern = LED_PATTERN_INVALID;

            xSemaphoreTake(led_mutex_handle, portMAX_DELAY);
            red_led_flag |= LED_FLAG_NO_COORDO;
            buffered_pattern = red_led_buffered_pattern;
            xSemaphoreGive(led_mutex_handle);
        
            if(buffered_pattern == LED_PATTERN_INVALID){
                //Re-apply invalid pattern
                xSemaphoreGive(red_led_semph_handle);
            }
        }
        break;

        case LED_PATTERN_SCANNING:
        {
            ESP_LOGI(TAG, "Starting Scanning pattern");

            LED_Pattern_t buffered_pattern = LED_PATTERN_INVALID;

            xSemaphoreTake(led_mutex_handle, portMAX_DELAY);
            green_led_flag |= LED_FLAG_SCANNING;
            buffered_pattern = green_led_buffered_pattern;
            xSemaphoreGive(led_mutex_handle);

            if(buffered_pattern == LED_PATTERN_INVALID){
                //Re-apply invalid pattern
                xSemaphoreGive(green_led_semph_handle);
            }
        }
        break;

        case LED_PATTERN_INVALID:
        default:
        {
            ESP_LOGI(TAG, "Starting Invalid pattern");
        }
        break;
    }

    return LED_STATUS_OK;
}

/***************************************************************************/ /*!
*  \brief Stop Led pattern
*
*   Function used to stop an active Led pattern.
*
*   Preconditions: None.
*
*   Side Effects: None.
*   
*   \param[in]  pattern         Led pattern to stop  
*
*   \return operation status
*
*******************************************************************************/
LED_Ret_t LED_StopPattern(LED_Pattern_t pattern){

    if(pattern >= LED_PATTERN_INVALID){
        return LED_STATUS_ERROR;
    }

    switch(pattern){

        case LED_PATTERN_IDENTIFY:
        {
            ESP_LOGI(TAG, "Stopping Identify pattern");

            LED_Pattern_t red_buffered_pattern = LED_PATTERN_INVALID;
            LED_Pattern_t green_buffered_pattern = LED_PATTERN_INVALID;

            xSemaphoreTake(led_mutex_handle, portMAX_DELAY);
            red_led_flag &= ~LED_FLAG_IDENTIFY;
            red_buffered_pattern = red_led_buffered_pattern;

            green_led_flag &= ~LED_FLAG_IDENTIFY;
            green_buffered_pattern = green_led_buffered_pattern;
            xSemaphoreGive(led_mutex_handle);

            if(red_buffered_pattern == LED_PATTERN_INVALID){
                //Re-apply invalid pattern
                xSemaphoreGive(red_led_semph_handle);
            }

            if(green_buffered_pattern == LED_PATTERN_INVALID){
                //Re-apply invalid pattern
                xSemaphoreGive(green_led_semph_handle);
            }
        }
        break;

        case LED_PATTERN_CONNECTED:
        {
            ESP_LOGI(TAG, "Stopping Connected pattern");

            LED_Pattern_t buffered_pattern = LED_PATTERN_INVALID;

            xSemaphoreTake(led_mutex_handle, portMAX_DELAY);
            green_led_flag &= ~LED_FLAG_CONNECTED;
            buffered_pattern = green_led_buffered_pattern;
            xSemaphoreGive(led_mutex_handle);

            if(buffered_pattern == LED_PATTERN_INVALID){
                //Re-apply invalid pattern
                xSemaphoreGive(green_led_semph_handle);
            }
        }
        break;

        case LED_PATTERN_NO_COORDO:
        {
            ESP_LOGI(TAG, "Stopping No-Coordo pattern");

            LED_Pattern_t buffered_pattern = LED_PATTERN_INVALID;

            xSemaphoreTake(led_mutex_handle, portMAX_DELAY);
            red_led_flag &= ~LED_FLAG_NO_COORDO;
            buffered_pattern = red_led_buffered_pattern;
            xSemaphoreGive(led_mutex_handle);

            if(buffered_pattern == LED_PATTERN_INVALID){
                //Re-apply invalid pattern
                xSemaphoreGive(red_led_semph_handle);
            }
        }
        break;

        case LED_PATTERN_SCANNING:
        {
            ESP_LOGI(TAG, "Stopping Scanning pattern");

            LED_Pattern_t buffered_pattern = LED_PATTERN_INVALID;

            xSemaphoreTake(led_mutex_handle, portMAX_DELAY);
            green_led_flag &= ~LED_FLAG_SCANNING;
            buffered_pattern = green_led_buffered_pattern;
            xSemaphoreGive(led_mutex_handle);

            if(buffered_pattern == LED_PATTERN_INVALID){
                //Re-apply invalid pattern
                xSemaphoreGive(green_led_semph_handle);
            }
        }
        break;

        case LED_PATTERN_INVALID:
        default:
        {
            ESP_LOGI(TAG, "Stopping Invalid pattern");
        }
        break;
    }

    return LED_STATUS_OK;
}

/***************************************************************************/ /*!
*  \brief Get led handle
*
*   This function is used to get the led driver handle of a led entity.
*
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  led_id              Led ID
*   \param[out] pHandle             Pointer to store the correspondig led handle.
*
*   \return     Operation status
*
*******************************************************************************/
LED_Ret_t LED_GetLedHandle(LED_Ctrl_Id_t led_id, LED_Handle_t *pHandle){

    if((led_id >= LED_CTRL_ID_INVALID) || (pHandle == NULL)){

        return LED_STATUS_ERROR;
    }

    xSemaphoreTake(led_mutex_handle, portMAX_DELAY);
    
    switch(led_id){
        case LED_CTRL_ID_RED:
        {
            *pHandle = red_led_handle;
        }
        break;

        case LED_CTRL_ID_GREEN:
        {
            *pHandle = green_led_handle;
        }
        break;

        default:
        {
            //Do nothing...
        }
        break;
    }
    
    xSemaphoreGive(led_mutex_handle);

    return LED_STATUS_OK;
}

/******************************************************************************
*   Interrupts
*******************************************************************************/