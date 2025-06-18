/******************************************************************************
*   Includes
*******************************************************************************/
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "led_strip.h"
#include "esp_log.h"

#include "ledController.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/
#define LED_STRIP_RMT_RES_HZ            (10 * 1000 * 1000)
#define LED_STRIP_MEMORY_BLOCK_WORDS    (0)

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
static const char * TAG = "LED";

static led_strip_handle_t led_handle;
static LED_Pattern_t current_pattern;

static SemaphoreHandle_t led_mutex_handle = NULL;

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/


/******************************************************************************
*   Public Functions Definitions
*******************************************************************************/
LED_Ret_t LED_InitController(uint8_t led_strip_gpio){
    
    ESP_LOGI(TAG, "LED controller initialization");

    //Create led mutex
    led_mutex_handle = xSemaphoreCreateMutex();
    if(led_mutex_handle == NULL){
        ESP_LOGI(TAG, "Failed to create led mutex");
        return LED_STATUS_ERROR;
    }

    //Led strip initialization
    led_strip_config_t strip_config = {
        .strip_gpio_num = led_strip_gpio,
        .max_leds = 1,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags = {
            .invert_out = false,
        },
    };

    //Led strip backend initialization
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = LED_STRIP_RMT_RES_HZ,
        .mem_block_symbols = LED_STRIP_MEMORY_BLOCK_WORDS,
    };

    if(ESP_OK != led_strip_new_rmt_device(&strip_config, &rmt_config, &led_handle)){
        
        ESP_LOGI(TAG, "Failed to config led strip");
        return LED_STATUS_ERROR;
    }

    return LED_STATUS_OK;
}

LED_Ret_t LED_StartPattern(LED_Pattern_t pattern){

    if(pattern >= LED_PATTERN_INVALID){
        return LED_STATUS_ERROR;
    }

    if(current_pattern == pattern){
        ESP_LOGI(TAG, "Pattern already applied");
        return LED_STATUS_OK;
    }

    switch(pattern){

        case LED_PATTERN_BOOT:
        {
            ESP_LOGI(TAG, "Starting Boot pattern");
        }
        break;

        case LED_PATTERN_FACTORY_RESET:
        {
            ESP_LOGI(TAG, "Starting Factory Reset pattern");
        }
        break;

        case LED_PATTERN_IDENTIFY:
        {
            ESP_LOGI(TAG, "Starting Identify pattern");
        }
        break;

        case LED_PATTERN_CONNECTED:
        {
            ESP_LOGI(TAG, "Starting Connected pattern");
        }
        break;

        case LED_PATTERN_NOT_CONNECTED:
        {
            ESP_LOGI(TAG, "Starting Not-Connected pattern");
        }
        break;

        case LED_PATTERN_NO_COORDO:
        {
            ESP_LOGI(TAG, "Starting No-Coordo pattern");
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

LED_Ret_t LED_StopPattern(LED_Pattern_t pattern){

    if(pattern >= LED_PATTERN_INVALID){
        return LED_STATUS_ERROR;
    }

    switch(pattern){

        case LED_PATTERN_BOOT:
        {
            ESP_LOGI(TAG, "Stopping Boot pattern");
        }
        break;

        case LED_PATTERN_FACTORY_RESET:
        {
            ESP_LOGI(TAG, "Stopping Factory Reset pattern");
        }
        break;

        case LED_PATTERN_IDENTIFY:
        {
            ESP_LOGI(TAG, "Stopping Identify pattern");
        }
        break;

        case LED_PATTERN_CONNECTED:
        {
            ESP_LOGI(TAG, "Stopping Connected pattern");
        }
        break;

        case LED_PATTERN_NOT_CONNECTED:
        {
            ESP_LOGI(TAG, "Stopping Not-Connected pattern");
        }
        break;

        case LED_PATTERN_NO_COORDO:
        {
            ESP_LOGI(TAG, "Stopping No-Coordo pattern");
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

/******************************************************************************
*   Interrupts
*******************************************************************************/