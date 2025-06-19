/******************************************************************************
*   Includes
*******************************************************************************/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "userInterface.h"
#include "buttonController.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/
#define UI_QUEUE_RECV_TIMEOUT_MS        (1000)
#define UI_QUEUE_LENGTH                 (8)

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
static void shortpressCallback(void *args, void *user_data);
static void longpressCallback(void *args, void *user_data);

static void tUiTask(void *pvParameters);

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Private Variables
*******************************************************************************/
static TaskHandle_t ui_task_handle = NULL;
static QueueHandle_t ui_event_queue_handle = NULL;

static const char * TAG = "UI";

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/
static void shortpressCallback(void *args, void *user_data){

    UI_PostEvent(UI_EVENT_BTN_SHORTPRESS, 0);
}

static void longpressCallback(void *args, void *user_data){

    UI_PostEvent(UI_EVENT_BTN_LONGPRESS, 0);
}

static void tUiTask(void *pvParameters){

    UI_Event_t ui_event;
    ESP_LOGI(TAG, "Starting UI task");

    for(;;){
        if(pdTRUE == xQueueReceive(ui_event_queue_handle, 
                                   &ui_event, 
                                   (UI_QUEUE_RECV_TIMEOUT_MS/portTICK_PERIOD_MS))){

            switch(ui_event){

                case UI_EVENT_BOOT:
                {

                }
                break;

                case UI_EVENT_FACTORY_RESET:
                {

                }
                break;

                case UI_EVENT_IDENTIFY:
                {

                }
                break;

                case UI_EVENT_BTN_SHORTPRESS:
                {

                }
                break;

                case UI_EVENT_BTN_LONGPRESS:
                {

                }
                break;

                case UI_EVENT_CONNECTED:
                {

                }
                break;

                case UI_EVENT_NO_CONNECTED:
                {

                }
                break;

                case UI_EVENT_NO_COORDO:
                {

                }
                break;

                case UI_EVENT_SCANNING:
                {

                }
                break;

                case UI_EVENT_INVALID:
                default:
                {
                    //Do nothing...
                }
                break;
            }
        }
    }

    vTaskDelete(NULL);
}

/******************************************************************************
*   Public Functions Definitions
*******************************************************************************/
UI_Ret_t UI_Init(void){

    ESP_LOGI(TAG, "Initialization");

    //Create UI queue
    ui_event_queue_handle = xQueueCreate(UI_QUEUE_LENGTH, sizeof(UI_Event_t));
    if(ui_event_queue_handle == NULL){
        ESP_LOGI(TAG, "Failed to create UI queue");
        return UI_STATUS_ERROR;
    }

    //Init button controller
    BUTTON_Config_t btn_cfg = {
        .active_level = BUTTON_ACTIVE_LOW,
        .gpio = 9,
        .shortpress_callback = shortpressCallback,
        .longpress_callback = longpressCallback,
    };
    if(BUTTON_STATUS_OK != BUTTON_InitController(&btn_cfg)){
        ESP_LOGI(TAG, "Failed to init button controller");
        return UI_STATUS_ERROR;
    }

    //Create UI task
    if(pdTRUE != xTaskCreate(tUiTask,
                             "UI task",
                             2048,
                             NULL,
                             5,
                             &ui_task_handle)){

        ESP_LOGI(TAG, "Failed to create UI task");
        return UI_STATUS_ERROR;
    }

    return UI_STATUS_OK;
}

UI_Ret_t UI_PostEvent(UI_Event_t event, uint32_t wait_ms){

    if(ui_event_queue_handle == NULL){
        return UI_STATUS_ERROR;
    }

    if(event >= UI_EVENT_INVALID){
        return UI_STATUS_ERROR;
    }

    if(pdTRUE != xQueueSend(ui_event_queue_handle, &event, wait_ms/portTICK_PERIOD_MS)){
        ESP_LOGI(TAG, "Failed to queue the event.");
        return UI_STATUS_ERROR;
    }

    return UI_STATUS_OK;
}

/******************************************************************************
*   Interrupts
*******************************************************************************/


