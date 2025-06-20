/******************************************************************************
*   Includes
*******************************************************************************/
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#include "zigbeeManager.h"
#include "basicCluster.h"
#include "tempMeasCluster.h"
#include "humidityMeasCluster.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/
#define NETWORK_NVS_NAMESPACE           ("Network")
#define NETWORK_STATE_NVS               ("NetworkState")

#define NETWORK_STEERING_ATTEMPTS       (10)
#define NETWORK_LEAVE_DELAY_MS          (4000)

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
static void bdbStartTopLevelCommissioningCallback(uint8_t mode_mask);
static esp_err_t zigbeeActionHandler(esp_zb_core_action_callback_id_t callback_id, 
                                     const void *message);

static void leaveNetworkCallback(uint8_t param);
static void updateNetworkState(ZIGBEE_Nwk_State_t state);

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Private Variables
*******************************************************************************/
static esp_zb_ep_list_t *device_endpoint_list = NULL;
static esp_zb_cluster_list_t *device_cluster_list = NULL;

static ZIGBEE_Nwk_State_t network_state = ZIGBEE_NWK_INVALID;
static uint8_t network_steering_attempt = 0;

static SemaphoreHandle_t zigbee_mutex_handle = NULL;

static const char * TAG = "ZIGBEE";

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/
static void bdbStartTopLevelCommissioningCallback(uint8_t mode_mask){

}

static esp_err_t zigbeeActionHandler(esp_zb_core_action_callback_id_t callback_id, 
                                     const void *message){  

    esp_err_t ret = ESP_OK;

    return ret;
}

static void leaveNetworkCallback(uint8_t param){
    esp_zb_factory_reset();
}

static void updateNetworkState(ZIGBEE_Nwk_State_t state){

    ZIGBEE_Nwk_State_t tmp_nwk_state = ZIGBEE_NWK_INVALID;

    xSemaphoreTake(zigbee_mutex_handle, portMAX_DELAY);
    tmp_nwk_state = network_state;
    xSemaphoreGive(zigbee_mutex_handle);

    if(state == tmp_nwk_state){
        ESP_LOGI(TAG, "Zigbee Network state already at %d", state);
    }
    else{

        nvs_handle_t nvs_handle;
        esp_err_t nvs_err = nvs_open(NETWORK_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
        if(nvs_err != ESP_OK){
            ESP_LOGI(TAG, "Failed to open NVS");
        }
        else{
            //Store new value in NVS
            nvs_set_u8(nvs_handle, NETWORK_STATE_NVS, state);
            nvs_commit(nvs_handle);
            nvs_close(nvs_handle);

            //Update global value
            xSemaphoreTake(zigbee_mutex_handle, portMAX_DELAY);
            network_state = state;
            xSemaphoreGive(zigbee_mutex_handle);

            ESP_LOGI(TAG, "Updated Network state to %d", state);
        }
    }
}

void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct){

}

/******************************************************************************
*   Public Functions Definitions
*******************************************************************************/
/***************************************************************************//*!
*  \brief Init Zigbee stack
*
*   This function perform the zigbee stack initialization.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \return     Status of operation.   
*
*******************************************************************************/
ZIGBEE_Ret_t ZIGBEE_InitStack(void){

    //Create zigbee mutex
    zigbee_mutex_handle = xSemaphoreCreateMutex();
    if(zigbee_mutex_handle == NULL){
        ESP_LOGI(TAG, "Failed to create zigbee mutex");
        return ZIGBEE_STATUS_ERROR;
    }

    //Restore network state from NVS
    ZIGBEE_Nwk_State_t tmp_nwk_state = ZIGBEE_NWK_INVALID;
    nvs_handle_t nvs_handle;
    esp_err_t nvs_err = nvs_open(NETWORK_NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if(nvs_err != ESP_OK){
        ESP_LOGI(TAG, "Failed to open nvs");
        return ZIGBEE_STATUS_ERROR;
    }

    nvs_err = nvs_get_u8(nvs_handle, NETWORK_STATE_NVS, (uint8_t*)&tmp_nwk_state);
    if(nvs_err == ESP_ERR_NVS_NOT_FOUND){
        ESP_LOGI(TAG, "Zigbee Nwk state mst be init in NVS");
        nvs_set_u8(nvs_handle, NETWORK_STATE_NVS, ZIGBEE_NWK_NOT_CONNECTED);
        nvs_commit(nvs_handle);
    }

    nvs_get_u8(nvs_handle, NETWORK_STATE_NVS, (uint8_t*)&tmp_nwk_state);

    //If networ kstate is in an invalid boot state -> set to NOT_CONNECTED
    if((tmp_nwk_state != ZIGBEE_NWK_CONNECTED) && (tmp_nwk_state != ZIGBEE_NWK_NOT_CONNECTED)){
        tmp_nwk_state = ZIGBEE_NWK_NOT_CONNECTED;

        nvs_set_u8(nvs_handle, NETWORK_STATE_NVS, tmp_nwk_state);
        nvs_commit(nvs_handle);
    }

    nvs_close(nvs_handle);

    //Update network state with value stored in NVS
    xSemaphoreTake(zigbee_mutex_handle, portMAX_DELAY);
    network_state = tmp_nwk_state;
    xSemaphoreGive(zigbee_mutex_handle);

    return ZIGBEE_STATUS_OK;
}

/***************************************************************************//*!
*  \brief Zigbee start stack
*
*   This function is used to start the zigbee stack
*   
*   Preconditions: None.
*
*   Side Effects: None. 
*
*******************************************************************************/
void ZIGBEE_StartStack(void){

    esp_zb_main_loop_iteration();
}

/***************************************************************************//*!
*  \brief Zigbee start network scanning.
*
*   This function is use to start a zigbee network scan.
*   
*   Preconditions: None.
*
*   Side Effects: None. 
*
*   \return     Operation status
*
*******************************************************************************/
ZIGBEE_Ret_t ZIGBEE_StartScanning(void){

    return ZIGBEE_STATUS_OK;
}

/***************************************************************************//*!
*  \brief Zigbee leave network.
*
*   This function is use to leave a zigbee network
*   
*   Preconditions: None.
*
*   Side Effects: None. 
*
*******************************************************************************/
void ZIGBEE_LeaveNetwork(void){

    updateNetworkState(ZIGBEE_NWK_NOT_CONNECTED);

    esp_zb_scheduler_alarm((esp_zb_callback_t)leaveNetworkCallback, 
                           0, 
                           NETWORK_LEAVE_DELAY_MS);
}

/***************************************************************************//*!
*  \brief Get Zigbee network state
*
*   This function return the current zigbee network state.
*   
*   Preconditions: None.
*
*   Side Effects: None. 
*
*   \return     zigbee network state
*
*******************************************************************************/
ZIGBEE_Nwk_State_t ZIGBEE_GetNwkState(void){

    ZIGBEE_Nwk_State_t tmp_state = ZIGBEE_NWK_INVALID;

    //Check if mutex was created
    if(zigbee_mutex_handle != NULL){
        xSemaphoreTake(zigbee_mutex_handle, portMAX_DELAY);
        tmp_state = network_state;//Update to current value
        xSemaphoreGive(zigbee_mutex_handle);
    }

    return tmp_state;
}

/******************************************************************************
*   Interrupts
*******************************************************************************/

