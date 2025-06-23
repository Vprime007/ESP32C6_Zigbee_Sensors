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

static void leaveNetworkCallback(uint8_t param);
static void updateNetworkState(ZIGBEE_Nwk_State_t state);

static void tZigbeeTask(void *pvParameters);

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

static TaskHandle_t zigbee_task_handle = NULL;
static SemaphoreHandle_t zigbee_mutex_handle = NULL;

static const char * TAG = "ZIGBEE";

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/
/***************************************************************************//*!
*  \brief BDB top level commissioning callback.
*   
*   Preconditions: Zigbee stack is initialized.
*
*   Side Effects: None.
*
*   \param[in]  mode_mask 
*
*******************************************************************************/
static void bdbStartTopLevelCommissioningCallback(uint8_t mode_mask){

    esp_err_t ret = esp_zb_bdb_start_top_level_commissioning(mode_mask);
    if(ret != ESP_OK){
        ESP_LOGI(TAG, "Failed to start Zigbee bdb commissioning");
    }
}

/***************************************************************************//*!
*  \brief leave network callback.
*
*   Leave network callback need perform network leave and factory reset
*   of the device. The function do not use param for the moment.
*   
*   Preconditions: Zigbee stack is initialized.
*
*   Side Effects: None.
*
*   \param[in]  param          optional parameter.
*
*******************************************************************************/
static void leaveNetworkCallback(uint8_t param){
    esp_zb_factory_reset();
}

/***************************************************************************//*!
*  \brief Update network state.
*
*   Update network state to new value and managed value stored in NVS.
*   
*   Preconditions: Zigbee stack is initialized.
*
*   Side Effects: None.
*
*   \param[in]  state           Network state.
*
*******************************************************************************/
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

/**
 * @brief Zigbee stack application signal handler.
 * @anchor esp_zb_app_signal_handler
 *
 * @param[in] signal_s   pointer of Zigbee zdo app signal struct @ref esp_zb_app_signal_s.
 * @note After esp_zb_start, user shall based on the corresponding signal type refer to esp_zdo_app_signal_type 
 *       from struct pointer signal_s to do certain actions.
 * 
 * User could also use refer to esp_zb_bdb_start_top_level_commissioning to change BDB mode.
 * 
 * @warning This function has to be defined by user in each example.
 */
void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct){

    uint32_t *p_sg_p = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig_type = *p_sg_p;

    switch(sig_type){

        case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
        {
            ESP_LOGI(TAG, "Initialize Zigbee stack");
            esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
        }
        break;

        case ESP_ZB_BDB_SIGNAL_STEERING:
        {
            if(err_status == ESP_OK){

            }
            else{

                ESP_LOGI(TAG, "Network steering was not successful (status: %s)", esp_err_to_name(err_status));

                if(network_steering_attempt < NETWORK_STEERING_ATTEMPTS-1){
                    network_steering_attempt++;//Increment attempts cptr
                    ESP_LOGI(TAG, "Network Steering attempt: %d", network_steering_attempt);
                    //Schedule a new network steering attempt
                    esp_zb_scheduler_alarm((esp_zb_callback_t)bdbStartTopLevelCommissioningCallback, 
                                           ESP_ZB_BDB_MODE_NETWORK_STEERING, 
                                           1000);
                }
            }
        }
        break;

        default:
        {
            ESP_LOGI(TAG, "ZDO signal: %s (0x%x), status: %s", 
                          esp_zb_zdo_signal_to_string(sig_type), 
                          sig_type, 
                          esp_err_to_name(err_status));
        }
        break;
    }
}

/***************************************************************************//*!
*  \brief Zigbee task
*
*   Zigbee task. This is where the stack main loop is called.
*   
*   Preconditions: Zigbee stack is initialized.
*
*   Side Effects: None.
*
*******************************************************************************/
static void tZigbeeTask(void *pvParameters){

    ESP_LOGI(TAG, "Starting Zigbee task");

    for(;;){
        //Zigbee stack loop
        esp_zb_stack_main_loop();
    }
    vTaskDelete(NULL);
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
*   Preconditions: Zigbee stack is initialized.
*
*   Side Effects: None. 
*
*******************************************************************************/
void ZIGBEE_StartStack(void){

    if(pdTRUE != xTaskCreate(tZigbeeTask,
                             "Zigbee Task",
                             4096,
                             NULL,
                             8,
                             &zigbee_task_handle)){

        ESP_LOGI(TAG, "Failed to create Zigbee stask");
        while(1);//Stall here
    }
}

/***************************************************************************//*!
*  \brief Zigbee start network scanning.
*
*   This function is use to start a zigbee network scan.
*   
*   Preconditions: Zigbee stack is started and running.
*
*   Side Effects: None. 
*
*   \return     Operation status
*
*******************************************************************************/
ZIGBEE_Ret_t ZIGBEE_StartScanning(void){

    //Check current network state
    ZIGBEE_Nwk_State_t tmp_nwk_state = ZIGBEE_NWK_INVALID;

    xSemaphoreTake(zigbee_mutex_handle, portMAX_DELAY);
    tmp_nwk_state = network_state;
    xSemaphoreGive(zigbee_mutex_handle);

    if(tmp_nwk_state != ZIGBEE_NWK_NOT_CONNECTED){
        return ZIGBEE_STATUS_ERROR;
    }

    //Start network steering
    esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);

    return ZIGBEE_STATUS_OK;
}

/***************************************************************************//*!
*  \brief Zigbee leave network.
*
*   This function is use to leave a zigbee network
*   
*   Preconditions: Zigbee stack is started and running.
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
*   Preconditions: Zigbee stack is started and running.
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

