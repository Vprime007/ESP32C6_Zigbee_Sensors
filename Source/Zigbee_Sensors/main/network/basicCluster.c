/******************************************************************************
*   Includes
*******************************************************************************/
#include "esp_log.h"
#include "esp_zigbee_cluster.h"

#include "basicCluster.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/
#define MANUFACTURER_NAME               ("V_PRIME_TECH")
#define MODEL_IDENTIFIER                ("AHT10_ZIGBEE")

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
static const char * TAG = "BASIC";

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/


/******************************************************************************
*   Public Functions Definitions
*******************************************************************************/
/***************************************************************************//*!
*  \brief Basic cluster initialization.
*
*   Initialize Basic cluster and attributes with default value. It 
*   also add the Basic cluster to the cluster list passed to this
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
BASIC_Cluster_Ret_t BASIC_IntiCluster(esp_zb_cluster_list_t *pCluster_list){

    ESP_LOGI(TAG, "Cluster Initialization");

    esp_zb_basic_cluster_cfg_t basic_cfg = {
        .power_source = ESP_ZB_ZCL_BASIC_POWER_SOURCE_DC_SOURCE,
        .zcl_version = ESP_ZB_ZCL_BASIC_ZCL_VERSION_DEFAULT_VALUE,
    };
    esp_zb_attribute_list_t *pBasicCluster = esp_zb_basic_cluster_create(&basic_cfg);

    esp_zb_basic_cluster_add_attr(pBasicCluster, ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, MANUFACTURER_NAME);
    esp_zb_basic_cluster_add_attr(pBasicCluster, ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, MODEL_IDENTIFIER);

    if(ESP_OK != esp_zb_cluster_list_add_basic_cluster(pCluster_list, 
                                                       pBasicCluster, 
                                                       ESP_ZB_ZCL_CLUSTER_SERVER_ROLE)){
        
        ESP_LOGI(TAG, "Failed to add basic cluster");
        return BASIC_CLUSTER_STATUS_ERROR;
    }

    return BASIC_CLUSTER_STATUS_OK;
}

/******************************************************************************
*   Interrupts
*******************************************************************************/


