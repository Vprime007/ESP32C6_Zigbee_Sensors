/******************************************************************************
*   Includes
*******************************************************************************/
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#include "sensorController.h"
#include "aht10.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/
#define LOG_LOCAL_LEVEL                 (ESP_LOG_INFO)

#define INTER_STEP_DELAY_MS             (25)

/******************************************************************************
*   Private Macros
*******************************************************************************/


/******************************************************************************
*   Private Data Types
*******************************************************************************/
typedef enum SENSOR_Step_e{
    SENSOR_STEP_IDLE,
    SENSOR_STEP_MEAS,
    SENSOR_STEP_PROCESS_TEMP,
    SENSOR_STEP_PROCESS_HUMIDITY,
    
    SENSOR_STEP_INVALID,
}SENSOR_Step_t;

/******************************************************************************
*   Private Functions Declaration
*******************************************************************************/
static void tSensorTask(void *pvParameters);

static void wait_ms(uint32_t wait_time_ms);

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Private Variables
*******************************************************************************/
static TaskHandle_t sensor_task_handle = NULL;
static SemaphoreHandle_t sensor_mutex_handle = NULL;

static SENSOR_Step_t sensor_step = SENSOR_STEP_IDLE;

static const char * TAG = "SENSOR";

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/
/***************************************************************************//*!
*  \brief Sensor Task.
*
*   Sensor controller task. It perform sensor management and sampling sequentially.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*******************************************************************************/
static void tSensorTask(void *pvParameters){

    ESP_LOGI(TAG, "Starting Sensor task");

    for(;;){

        switch(sensor_step){

            case SENSOR_STEP_IDLE:
            {
                ESP_LOGI(TAG, "STEP_IDLE");

                //Set next step
                sensor_step = SENSOR_STEP_MEAS;
                vTaskDelay(INTER_STEP_DELAY_MS/portTICK_PERIOD_MS);
            }
            break;

            case SENSOR_STEP_MEAS:
            {
                ESP_LOGI(TAG, "STEP_MEAS");

                AHT10_StartMeasurement();

                //Set next step
                sensor_step = SENSOR_STEP_PROCESS_TEMP;
                vTaskDelay(INTER_STEP_DELAY_MS/portTICK_PERIOD_MS);
            }
            break;

            case SENSOR_STEP_PROCESS_TEMP:
            {
                ESP_LOGI(TAG, "STEP_PROCESS_TEMP");

                int16_t temperature = AHT10_INVALID_TEMPERATURE;
                if(AHT10_STATUS_OK != AHT10_GetLastTemperature(&temperature)){
                    ESP_LOGI(TAG, "Failed to get last temperature");
                    temperature = AHT10_INVALID_TEMPERATURE;
                }

                //Store new value in zigbee cluster

                //Set next step
                sensor_step = SENSOR_STEP_PROCESS_HUMIDITY;
                vTaskDelay(INTER_STEP_DELAY_MS/portTICK_PERIOD_MS);
            }
            break;

            case SENSOR_STEP_PROCESS_HUMIDITY:
            {
                ESP_LOGI(TAG, "STEP_PROCESS_HUMIDITY");

                uint16_t humidity = AHT10_INVALID_HUMIDITY;
                if(AHT10_STATUS_OK != AHT10_GetLastHumidity(&humidity)){
                    ESP_LOGI(TAG, "Failed to get last humidity");
                    humidity = AHT10_INVALID_HUMIDITY;
                }

                //Store new value in zigbee cluster

                //Set next step
                sensor_step = SENSOR_STEP_IDLE;
                vTaskDelay(INTER_STEP_DELAY_MS/portTICK_PERIOD_MS);
            }
            break;

            case SENSOR_STEP_INVALID:
            default:
            {
                //Do nothing...
            }
            break;
        }

        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

/***************************************************************************//*!
*  \brief Wait milli-seconds
*
*   This function is used as a callback for the aht10 sensor.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  wait_time_ms            Time to wait in milli-seconds.«s
*
*******************************************************************************/
static void wait_ms(uint32_t wait_time_ms){

    if(wait_time_ms > 0)    vTaskDelay(wait_time_ms/portTICK_PERIOD_MS);
}

/******************************************************************************
*   Public Functions Definitions
*******************************************************************************/
/***************************************************************************//*!
*  \brief Sensor controller initialization.
*
*   This function perform the initialization of the sensor module.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \return     Operation status
*
*******************************************************************************/
SENSOR_Ret_t SENSOR_InitController(void){

    //Create sensor mutex
    sensor_mutex_handle = xSemaphoreCreateMutex();
    if(sensor_mutex_handle == NULL){
        ESP_LOGI(TAG, "Failed to create sensor mutex");
        return SENSOR_STATUS_ERROR;
    }

    //Init AHT10
    if(AHT10_STATUS_OK != AHT10_Init(7,
                                     6,
                                     wait_ms)){

        ESP_LOGI(TAG, "Failed to init AHT10");
        return SENSOR_STATUS_ERROR;
    }

    //Create sensor task
    if(pdTRUE != xTaskCreate(tSensorTask,
                             "Sensor Task",
                             2048,
                             NULL,
                             6,
                             &sensor_task_handle)){

        ESP_LOGI(TAG, "Failed to create sensor task");
        return SENSOR_STATUS_ERROR;
    }

    return SENSOR_STATUS_OK;
}

/******************************************************************************
*   Interrupts
*******************************************************************************/