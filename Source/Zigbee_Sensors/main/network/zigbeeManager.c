/******************************************************************************
*   Includes
*******************************************************************************/
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#include "zigbeeManager.h"
#include "basicCluster.h"
#include "tempMeasCluster.h"
#include "humidityMeasCluster.h"
#include "identifyCluster.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/
#define NETWORK_NVS_NAMESPACE               ("Network")
#define NETWORK_STATE_NVS                   ("NetworkState")

#define NETWORK_STEERING_ATTEMPTS           (10)
#define NETWORK_LEAVE_DELAY_MS              (4000)
#define NETWORK_COORDO_DETECT_PERIOD_MS     (30 * 1000)
#define NETWORK_COORD_DETECT_TIMEOUT_MS     (5 * 1000)

#define LOG_LOCAL_LEVEL                     (ESP_LOG_INFO)

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
static void sendIeeeAddrReqCallback(uint8_t param);
static void ieeeAddrResponseTimeout(TimerHandle_t xTimer);
static void ieeeAddrResponseCallback(esp_zb_zdp_status_t zdo_status, 
                                     esp_zb_zdo_ieee_addr_rsp_t *resp, 
                                     void *user_ctx);

static void updateNetworkState(ZIGBEE_Nwk_State_t state);

static void tZigbeeTask(void *pvParameters);

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Private Variables
*******************************************************************************/
static ZIGBEE_Nwk_State_t network_state = ZIGBEE_NWK_INVALID;
static uint8_t network_steering_attempt = 0;
static networkStateChangeCallback_t nwk_state_change_callback;

static TaskHandle_t zigbee_task_handle = NULL;
static SemaphoreHandle_t zigbee_mutex_handle = NULL;
static TimerHandle_t ieee_req_timer_handle = NULL;

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
*  \brief Send IEEE address request callback
*
*   Send IEEE address request to the network coordo (addr 0x0000).
*   
*   Preconditions: Zigbee stack is initialized.
*
*   Side Effects: None.
*
*   \param[in]  param          optional parameter.
*
*******************************************************************************/
static void sendIeeeAddrReqCallback(uint8_t param){

    ESP_LOGI(TAG, "Send IEEE Address request");

    esp_zb_zdo_ieee_addr_req_param_t req_param = {
        .dst_nwk_addr = 0x0000,
        .request_type = 0x00,
    };

    esp_zb_zdo_ieee_addr_req(&req_param, ieeeAddrResponseCallback, NULL);
    xTimerStart(ieee_req_timer_handle, 10/portTICK_PERIOD_MS);
}

static void ieeeAddrResponseTimeout(TimerHandle_t xTimer){

    ESP_LOGI(TAG, "IEEE address request timeout");

    if(network_state != ZIGBEE_NWK_NO_PARENT){
        xSemaphoreTake(zigbee_mutex_handle, portMAX_DELAY);
        updateNetworkState(ZIGBEE_NWK_NO_PARENT);
        xSemaphoreGive(zigbee_mutex_handle); 

        //Nofity of network state change
        if(nwk_state_change_callback != NULL){
            nwk_state_change_callback(network_state);
        }
    }
}

/***************************************************************************//*!
*  \brief IEEE address request response callback
*
*   This function is called with the response of the IEEE address request
*   response.
*   
*   Preconditions: Zigbee stack is initialized.
*
*   Side Effects: None.
*
*   \param[in]  zdo_status          ZDO command status.
*   \param[in]  resp                Pointer to request response.
*   \param[in]  user_ctx            Pointer to user context (optional).
*
*******************************************************************************/
static void ieeeAddrResponseCallback(esp_zb_zdp_status_t zdo_status, 
                             esp_zb_zdo_ieee_addr_rsp_t *resp, 
                             void *user_ctx){
    
    //Stop response timeout timer
    xTimerStop(ieee_req_timer_handle, 10/portTICK_PERIOD_MS);

    if(zdo_status != ESP_ZB_ZDP_STATUS_SUCCESS){
        //Failed to reach the network coordo
        ESP_LOGI(TAG, "Failed to detect coordo");

        if(network_state != ZIGBEE_NWK_NO_PARENT){
            xSemaphoreTake(zigbee_mutex_handle, portMAX_DELAY);
            updateNetworkState(ZIGBEE_NWK_NO_PARENT);
            xSemaphoreGive(zigbee_mutex_handle); 

            //Nofity of network state change
            if(nwk_state_change_callback != NULL){
                nwk_state_change_callback(network_state);
            }
        }
    }
    else{
        //Coordo detected
        ESP_LOGI(TAG, "Coordo detected");
        if(network_state != ZIGBEE_NWK_CONNECTED){
            xSemaphoreTake(zigbee_mutex_handle, portMAX_DELAY);
            updateNetworkState(ZIGBEE_NWK_CONNECTED);
            xSemaphoreGive(zigbee_mutex_handle);

            //Nofity of network state change
            if(nwk_state_change_callback != NULL){
                nwk_state_change_callback(network_state);
            }
        }
    }

    //re-schedule a IEEE addr request
    esp_zb_scheduler_alarm((esp_zb_callback_t)sendIeeeAddrReqCallback, 
                           0, 
                           NETWORK_COORDO_DETECT_PERIOD_MS);
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

    if(state == network_state){
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
            network_state = state;

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
                //Update network state
                xSemaphoreTake(zigbee_mutex_handle, portMAX_DELAY);
                updateNetworkState(ZIGBEE_NWK_CONNECTED);
                xSemaphoreGive(zigbee_mutex_handle);

                if(nwk_state_change_callback != NULL){
                    nwk_state_change_callback(network_state);
                }

                //Schedule a IEEE addr request
                esp_zb_scheduler_alarm((esp_zb_callback_t)sendIeeeAddrReqCallback, 
                                       0, 
                                       NETWORK_COORDO_DETECT_PERIOD_MS);
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
                else{
                    ESP_LOGI(TAG, "Failed to connect");
                    xSemaphoreTake(zigbee_mutex_handle, portMAX_DELAY);
                    updateNetworkState(ZIGBEE_NWK_NOT_CONNECTED);
                    xSemaphoreGive(zigbee_mutex_handle);

                    if(nwk_state_change_callback != NULL){
                        nwk_state_change_callback(network_state);
                    }
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

    //Start zigbee stack
    esp_zb_start(false);

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
*   If notification on network state change are desired, set param
*   nwk_change_callback with a non-NULL value.   
*
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  nwk_change_callback     Network state change callback.
*
*   \return     Status of operation.   
*
*******************************************************************************/
ZIGBEE_Ret_t ZIGBEE_InitStack(networkStateChangeCallback_t nwk_change_callback){

    //Create zigbee mutex
    zigbee_mutex_handle = xSemaphoreCreateMutex();
    if(zigbee_mutex_handle == NULL){
        ESP_LOGI(TAG, "Failed to create zigbee mutex");
        return ZIGBEE_STATUS_ERROR;
    }

    //Create ieee request timer
    ieee_req_timer_handle = xTimerCreate("IEEE_TIMER",
                                         NETWORK_COORD_DETECT_TIMEOUT_MS/portTICK_PERIOD_MS,
                                         pdFALSE,
                                         NULL,
                                         ieeeAddrResponseTimeout);
    
    if(ieee_req_timer_handle == NULL){
        ESP_LOGI(TAG, "Failed to create ieee request timer");
        return ZIGBEE_STATUS_ERROR;
    }

    nvs_flash_init();

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
        ESP_LOGI(TAG, "Zigbee Nwk state must be init in NVS");
        nvs_set_u8(nvs_handle, NETWORK_STATE_NVS, ZIGBEE_NWK_NOT_CONNECTED);
        nvs_commit(nvs_handle);
    }

    nvs_get_u8(nvs_handle, NETWORK_STATE_NVS, (uint8_t*)&tmp_nwk_state);

    //If network state is in an invalid boot state -> set to NOT_CONNECTED
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

    //Register network state change callback
    if(nwk_change_callback != NULL)     nwk_state_change_callback = nwk_change_callback;

    //Platform config
    esp_zb_platform_config_t config = {
        .host_config.host_connection_mode = ZB_HOST_CONNECTION_MODE_NONE,
        .radio_config.radio_mode = ZB_RADIO_MODE_NATIVE, 
    };
    if(ESP_OK != esp_zb_platform_config(&config)){
        ESP_LOGI(TAG, "Failed to config zigbee platform");
        return ZIGBEE_STATUS_ERROR;
    }

    //Zigbee device config
    esp_zb_cfg_t zb_nwk_config = {
        .esp_zb_role = ZIGBEE_DEVICE_TYPE,
        .install_code_policy = ZIGBEE_INSTALLCODE_POLICY,
        .nwk_cfg.zed_cfg = {
            .ed_timeout = ZIGBEE_ED_AGING_TIMEOUT,
            .keep_alive = ZIGBEE_ED_KEEP_ALIVE_MS,
        },
    };
    esp_zb_init(&zb_nwk_config);

    //Create cluster list
    esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();
    if(TEMP_CLUSTER_STATUS_OK != TEMP_InitCluster(cluster_list)){
        ESP_LOGI(TAG, "Failed to init Temperature cluster");
        return ZIGBEE_STATUS_ERROR;
    }

    if(HUMIDITY_CLUSTER_STATUS_OK != HUMIDITY_InitCluster(cluster_list)){
        ESP_LOGI(TAG, "Failed to init humidity cluster");
        return ZIGBEE_STATUS_ERROR;
    }

    if(BASIC_CLUSTER_STATUS_OK != BASIC_IntiCluster(cluster_list)){
        ESP_LOGI(TAG, "Failed to init Basic cluster");
        return ZIGBEE_STATUS_ERROR;
    }

    /*if(IDENTIFY_CLUSTER_STATUS_OK != IDENTIFY_InitCluster(cluster_list)){
        ESP_LOGI(TAG, "Failed to init Identify cluster");
        return ZIGBEE_STATUS_ERROR;
    }*/

    //Create device enpoint
    esp_zb_ep_list_t *ep_list = esp_zb_ep_list_create();
    esp_zb_endpoint_config_t endpoint_config = {
        .endpoint = ZIGBEE_ENDPOINT_1,
        .app_device_version = 0,
        .app_device_id = ESP_ZB_HA_TEMPERATURE_SENSOR_DEVICE_ID,
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
    };
    if(ESP_OK != esp_zb_ep_list_add_ep(ep_list, cluster_list, endpoint_config)){
        ESP_LOGI(TAG, "Failed to add endpoint");
        return ZIGBEE_STATUS_ERROR;
    }

    if(ESP_OK != esp_zb_device_register(ep_list)){
        ESP_LOGI(TAG, "Failed to register device");
        return ZIGBEE_STATUS_ERROR;
    }

    esp_zb_set_primary_network_channel_set(ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK);

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
    if(ESP_OK != esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING)){
        ESP_LOGI(TAG, "Failed to start network steering");
        return ZIGBEE_STATUS_ERROR;
    }
    else{
        //Update network status
        xSemaphoreTake(zigbee_mutex_handle, portMAX_DELAY);
        updateNetworkState(ZIGBEE_NWK_SCANNING);
        xSemaphoreGive(zigbee_mutex_handle);

        if(nwk_state_change_callback != NULL){
            nwk_state_change_callback(network_state);
        }
    }

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

