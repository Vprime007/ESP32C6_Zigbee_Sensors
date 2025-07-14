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
#include "zigbeeManager.h"
#include "tempMeasCluster.h"
#include "humidityMeasCluster.h"
#include "main.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/
#define INTER_STEP_DELAY_MS             (25)
#define INITIAL_DELAY_MS                (10 * 1000)
#define SENSOR_LOOP_PERIOD_MS           (1 * 1000)
#define NB_TEMPERATURE_SAMPLE           (8)
#define NB_HUMIDITY_SAMPLE              (8)

#define LOG_LOCAL_LEVEL                 (ESP_LOG_INFO)

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

    static uint8_t temp_avg_cptr = 0;
    static uint8_t temp_invalid_cptr = 0;
    static int32_t temp_cumul = 0;

    static uint8_t rh_avg_cptr = 0;
    static uint8_t rh_invalid_cptr = 0;
    static uint32_t rh_cumul = 0;

    vTaskDelay(INITIAL_DELAY_MS/portTICK_PERIOD_MS);

    for(;;){

        switch(sensor_step){

            case SENSOR_STEP_IDLE:
            {
                //Set next step
                sensor_step = SENSOR_STEP_MEAS;
                vTaskDelay(INTER_STEP_DELAY_MS/portTICK_PERIOD_MS);
            }
            break;

            case SENSOR_STEP_MEAS:
            {
                AHT10_StartMeasurement();

                //Set next step
                sensor_step = SENSOR_STEP_PROCESS_TEMP;
                vTaskDelay(INTER_STEP_DELAY_MS/portTICK_PERIOD_MS);
            }
            break;

            case SENSOR_STEP_PROCESS_TEMP:
            {
                int16_t temperature = AHT10_INVALID_TEMPERATURE;
                if(AHT10_STATUS_OK != AHT10_GetLastTemperature(&temperature)){
                    ESP_LOGI(TAG, "Failed to get last temperature");
                    temperature = AHT10_INVALID_TEMPERATURE;
                }

                if(temperature != (int16_t)AHT10_INVALID_TEMPERATURE){
                    //Reset invalid temp cptr
                    temp_invalid_cptr = 0;

                    //Check if we need to update zigbee attrib
                    if(temp_avg_cptr >= NB_TEMPERATURE_SAMPLE){
                        //Calculate the average
                        temp_cumul /= NB_TEMPERATURE_SAMPLE;
                        temperature = (int16_t)temp_cumul;

                        //Reset average temperature
                        temp_cumul = 0;
                        temp_avg_cptr = 0;

                        ESP_LOGI(TAG, "Temperature: %d *C", temperature);

                        //Update zigbee attrib with new value
                        if(TEMP_CLUSTER_STATUS_OK != TEMP_SetTemperature(temperature)){
                            ESP_LOGI(TAG, "Failed to update zigbee attrib");
                        }
                    }
                    else{
                        //Increment avg temp cptr
                        temp_avg_cptr++;
                        temp_cumul += temperature;
                    }
                }
                else{
                    //Reset avg temp cptr
                    temp_avg_cptr = 0;
                    //Reset avg temp value
                    temp_cumul = 0;

                    //Check if we need to update zigbee attrib
                    if(temp_invalid_cptr >= NB_TEMPERATURE_SAMPLE){
                        //Reset invalid temp cptr
                        temp_invalid_cptr = 0;

                        ESP_LOGI(TAG, "Temperature: %d *C", temperature);

                        //Update zigbee attrib with invalid value
                        if(TEMP_CLUSTER_STATUS_OK != TEMP_SetTemperature(temperature)){
                            ESP_LOGI(TAG, "Failed to update zigbee attrib");
                        }                    
                        else{
                        //Increment invalid temp cptr
                        temp_invalid_cptr++;
                        }
                    }
                }

                //Set next step
                sensor_step = SENSOR_STEP_PROCESS_HUMIDITY;
                vTaskDelay(INTER_STEP_DELAY_MS/portTICK_PERIOD_MS);
            }
            break;

            case SENSOR_STEP_PROCESS_HUMIDITY:
            {
                uint16_t humidity = AHT10_INVALID_HUMIDITY;
                if(AHT10_STATUS_OK != AHT10_GetLastHumidity(&humidity)){
                    ESP_LOGI(TAG, "Failed to get last humidity");
                    humidity = AHT10_INVALID_HUMIDITY;
                }

                if(humidity != AHT10_INVALID_HUMIDITY){
                    //Reset invalid humidity cptr
                    rh_invalid_cptr = 0;

                    //Check if we need to update zigbee attrib
                    if(rh_avg_cptr >= NB_HUMIDITY_SAMPLE){
                        //Calculate te average
                        rh_cumul /= NB_HUMIDITY_SAMPLE;
                        humidity = (uint16_t)rh_cumul;

                        //Reset average rh
                        rh_cumul = 0;
                        rh_avg_cptr = 0;

                        ESP_LOGI(TAG, "Humidity: %d", humidity);

                        //Update zigbee attrib with new value
                        if(HUMIDITY_CLUSTER_STATUS_OK != HUMIDITY_SetRelHumidity(humidity)){
                            ESP_LOGI(TAG, "Failed to update zigbee attrib");
                        }
                    }
                    else{
                        //Increment avg cptr
                        rh_avg_cptr++;
                        rh_cumul += humidity;
                    }
                }
                else{
                    //Reset avg rh cptr
                    rh_avg_cptr = 0;
                    //Reset cumul value
                    rh_avg_cptr = 0;

                    //Check if we need to update zigbee attrib
                    if(rh_invalid_cptr >= NB_HUMIDITY_SAMPLE){
                        //Reset invalid rh cptr
                        rh_invalid_cptr = 0;

                        //Update zigbee attrib with invalid value
                        ESP_LOGI(TAG, "Humidity: %d", humidity);

                        //Update zigbee attrib with new value
                        if(HUMIDITY_CLUSTER_STATUS_OK != HUMIDITY_SetRelHumidity(humidity)){
                            ESP_LOGI(TAG, "Failed to update zigbee attrib");
                        }
                    }
                    else{
                        //Increment invalid rh cptr
                        rh_invalid_cptr++;
                    }
                }

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

        vTaskDelay(SENSOR_LOOP_PERIOD_MS/portTICK_PERIOD_MS);
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
    if(AHT10_STATUS_OK != AHT10_Init(HWI_AHT10_SCL_GPIO,
                                     HWI_AHT10_SDA_GPIO,
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