/******************************************************************************
*   Includes
*******************************************************************************/
#include "esp_log.h"
#include "esp_zigbee_cluster.h"

#include "identifyCluster.h"
#include "zigbeeManager.h"
#include "userInterface.h"

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


/******************************************************************************
*   Private Functions Declaration
*******************************************************************************/
static void identifyCallback(uint8_t identify_on);

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Private Variables
*******************************************************************************/
static const char * TAG = "IDENTIFY";

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/
static void identifyCallback(uint8_t identify_on){

    if(identify_on >= 1){
        //Start identifying
        ESP_LOGI(TAG, "Start identifying");
        UI_PostEvent(UI_EVENT_START_IDENTIFY, 0);
    }
    else{
        //Stop identifying
        ESP_LOGI(TAG, "Stop identifying");
        UI_PostEvent(UI_EVENT_STOP_IDENTIFY, 0);
    }
}

/******************************************************************************
*   Public Functions Definitions
*******************************************************************************/
/***************************************************************************//*!
*  \brief Identify cluster initialization.
*
*   Initialize Identify cluster and attributes with default value. It 
*   also add the Identify cluster to the cluster list passed to this
*   function.
*   
*   Preconditions: None.
*
*   Side Effects: None. 
*
*   \param[in]  pCluster_list           Zigbee cluster list
*
*   \return     Operation status
*
*******************************************************************************/
IDENTIFY_Cluster_Ret_t IDENTIFY_InitCluster(esp_zb_cluster_list_t *pCluster_list){

    ESP_LOGI(TAG, "Cluster Initialization");

    esp_zb_identify_cluster_cfg_t identify_cfg = {
        .identify_time = ESP_ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE,
    };
    esp_zb_attribute_list_t *pIdentifyCluster = esp_zb_identify_cluster_create(&identify_cfg);

    if(esp_zb_cluster_list_add_identify_cluster(pCluster_list,
                                                pIdentifyCluster,
                                                ESP_ZB_ZCL_CLUSTER_SERVER_ROLE)){

        ESP_LOGI(TAG, "Failed to add identify cluster");
        return IDENTIFY_CLUSTER_STATUS_ERROR;
    }

    return IDENTIFY_CLUSTER_STATUS_OK;
}

/***************************************************************************//*!
*  \brief Setup Identify cluster cmd handler.
*
*   Setup and register Identify cluster cmd handler.
*   
*   Preconditions: This function must be called AFTER esp_zb_device_register().
*
*   Side Effects: None. 
*
*   \return     Operation status
*
*******************************************************************************/
IDENTIFY_Cluster_Ret_t IDENTIFY_SetupCmdHandler(void){

    //Register Identify callback
    esp_zb_identify_notify_handler_register(ZIGBEE_ENDPOINT_1,
                                            (esp_zb_identify_notify_callback_t)identifyCallback);

    return IDENTIFY_CLUSTER_STATUS_OK;
}

/******************************************************************************
*   Interrupts
*******************************************************************************/

