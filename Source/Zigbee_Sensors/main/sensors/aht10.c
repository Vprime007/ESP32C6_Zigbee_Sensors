/******************************************************************************
*   Includes
*******************************************************************************/
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "driver/i2c_master.h"
#include "esp_log.h"

#include "aht10.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/
#define I2C_MASTER_NUM                  (I2C_NUM_0)
#define I2C_MASTER_FREQ_HZ              (100000)
#define I2C_COM_TIMEOUT_MS              (1000)

#define AHT10_I2C_ADDR                  (0x38)
#define AHT10_CMD_INIT                  (0xE1)
#define AHT10_CMD_MEAS                  (0xAC)

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
static SemaphoreHandle_t aht10_mutex_handle = NULL;

static int16_t current_temp = AHT10_INVALID_TEMPERATURE;
static uint16_t current_humidity = AHT10_INVALID_HUMIDITY;

static i2c_master_bus_handle_t i2c_bus_handle = NULL;
static i2c_master_dev_handle_t i2c_dev_handle = NULL;

static WaitMsFunction_t wait_ms_function = NULL;

static const char * TAG = "AHT10";

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/


/******************************************************************************
*   Public Functions Definitions
*******************************************************************************/
AHT10_Ret_t AHT10_Init(uint8_t scl_gpio, 
                       uint8_t sda_gpio, 
                       WaitMsFunction_t wait_function){

    //Create mutex
    aht10_mutex_handle = xSemaphoreCreateMutex();
    if(aht10_mutex_handle == NULL){

        ESP_LOGI(TAG, "Failed to create aht10 mutex");
        return AHT10_STATUS_ERROR;
    }

    if(wait_function == NULL){
        ESP_LOGI(TAG, "Failed to init: Invalid params");
        return AHT10_STATUS_ERROR;
    }

    //Init I2C bus
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_MASTER_NUM,
        .scl_io_num = scl_gpio,
        .sda_io_num = sda_gpio,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    if(ESP_OK != i2c_new_master_bus(&bus_cfg, &i2c_bus_handle)){
        ESP_LOGI(TAG, "Failed to config I2C bus");
        return AHT10_STATUS_ERROR;
    }

    //Init AHT10 I2C driver
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = AHT10_I2C_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    if(ESP_OK != i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &i2c_dev_handle)){
        ESP_LOGI(TAG, "Failed to config AHT10 I2C device");
        return AHT10_STATUS_ERROR;
    }

    //Register wait function
    wait_ms_function = wait_function;

    uint8_t cmd_to_send[] = {AHT10_CMD_INIT, 0x08, 0x00};
    if(ESP_OK != i2c_master_transmit(i2c_dev_handle, 
                                     cmd_to_send, 
                                     sizeof(cmd_to_send), 
                                     I2C_COM_TIMEOUT_MS)){

        ESP_LOGI(TAG, "Failed to send init cmd");
        return AHT10_STATUS_ERROR;
    }

    return AHT10_STATUS_OK;
}

AHT10_Ret_t AHT10_StartMeasurement(void){

    uint8_t cmd_to_send[] = {AHT10_CMD_MEAS, 0x33, 0x00};
    uint8_t recv_buffer[6] = {0};

    //Send measurement cmd
    if(ESP_OK != i2c_master_transmit(i2c_dev_handle, 
                                     cmd_to_send, 
                                     sizeof(cmd_to_send), 
                                     I2C_COM_TIMEOUT_MS)){
        
        ESP_LOGI(TAG, "Failed to send measurement cmd");
        return AHT10_STATUS_ERROR;
    }

    //Wait for sensor to sample temp/humidity
    if(wait_ms_function != NULL){
        wait_ms_function(100);//wait 100ms
    }
    else{
        return AHT10_STATUS_ERROR;
    }

    //Read result
    if(ESP_OK != i2c_master_receive(i2c_dev_handle, 
                                    recv_buffer, 
                                    sizeof(recv_buffer), 
                                    I2C_COM_TIMEOUT_MS)){

        ESP_LOGI(TAG, "Failed to read sensor");
        return AHT10_STATUS_ERROR;
    }

    //Process recv buffer
    xSemaphoreTake(aht10_mutex_handle, portMAX_DELAY);

    uint32_t raw_temperature = ((recv_buffer[3] & 0x0F) << 16) | (recv_buffer[4] << 8) | (recv_buffer[5]);
    raw_temperature *= 200;
    raw_temperature /= 0x100000;
    current_temp = (int16_t)(raw_temperature - 50);

    uint32_t raw_humidity = (recv_buffer[1] << 12) | (recv_buffer[2] << 4) | (recv_buffer[3] >> 4);
    raw_humidity *= 100;
    raw_humidity /= 0x100000;
    current_humidity = (uint16_t)(raw_humidity);
    
    xSemaphoreGive(aht10_mutex_handle);

    return AHT10_STATUS_OK;
}

AHT10_Ret_t AHT10_GetLastTemperature(int16_t *pTemperature){

    //Check if param is valid 
    if(pTemperature == NULL){
        ESP_LOGI(TAG, "Failed to return Temperature: Invalid param");
        return AHT10_STATUS_ERROR;
    }

    int16_t temperature = AHT10_INVALID_TEMPERATURE;
    xSemaphoreTake(aht10_mutex_handle, portMAX_DELAY);
    temperature = current_temp;
    xSemaphoreGive(aht10_mutex_handle);

    *pTemperature = temperature;

    return AHT10_STATUS_OK;
}

AHT10_Ret_t AHT10_GetLastHumidity(uint16_t *pHumidity){

    //Check if param is valid
    if(pHumidity == NULL){
        ESP_LOGI(TAG, "Failed to return Humidity: Invalid param");
        return AHT10_STATUS_ERROR;
    }

    uint16_t humidity = AHT10_INVALID_HUMIDITY;
    xSemaphoreTake(aht10_mutex_handle, portMAX_DELAY);
    humidity = current_humidity;
    xSemaphoreGive(aht10_mutex_handle);

    *pHumidity = humidity;

    return AHT10_STATUS_OK;
}

/******************************************************************************
*   Interrupts
*******************************************************************************/


