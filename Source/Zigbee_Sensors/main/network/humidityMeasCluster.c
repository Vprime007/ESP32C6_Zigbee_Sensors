/******************************************************************************
*   Includes
*******************************************************************************/
#include "esp_log.h"

#include "zigbeeManager.h"
#include "humidityMeasCluster.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/
#define HUMIDITY_MEAS_REPORT_MIN_INTERVAL_S     (10)
#define HUMIDITY_MEAS_REPORT_MAX_INTERVAL_S     (3600)
#define HUMIDITY_MEAS_REPORT_DELTA              (100)

#define LOG_LOCAL_LEVEL                         (ESP_LOG_INFO)

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
static const char * TAG = "HUMIDITY_MEAS";

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/


/******************************************************************************
*   Public Functions Definitions
*******************************************************************************/
/***************************************************************************//*!
*  \brief Rel. Humidity cluster initialization.
*
*   Initialize Rel. Humidity cluster and attributes with default value. It 
*   also add the Rel. Humidity cluster to the cluster list passed to this
*   function.
*   
*   Preconditions: None.
*
*   Side Effects: None. 
*
*   \param[in]  pCluster_list           Zigbee cluster list.
*
*   \return     Operation status
*
*******************************************************************************/
HUMIDITY_Cluster_Ret_t HUMIDITY_InitCluster(esp_zb_cluster_list_t *pCluster_list){

    ESP_LOGI(TAG, "Cluster Initialization");

    esp_zb_humidity_meas_cluster_cfg_t humidity_cfg = {
        .measured_value = ESP_ZB_ZCL_REL_HUMIDITY_MEASUREMENT_MEASURED_VALUE_UNKNOWN,
        .min_value = ESP_ZB_ZCL_REL_HUMIDITY_MEASUREMENT_MIN_MEASURED_VALUE_MINIMUM,
        .max_value = ESP_ZB_ZCL_REL_HUMIDITY_MEASUREMENT_MAX_MEASURED_VALUE_MAXIMUM,
    };
    esp_zb_attribute_list_t *pHumidityCluster = esp_zb_humidity_meas_cluster_create(&humidity_cfg);

    if(ESP_OK != esp_zb_cluster_list_add_humidity_meas_cluster(pCluster_list, 
                                                               pHumidityCluster, 
                                                               ESP_ZB_ZCL_CLUSTER_SERVER_ROLE)){

        ESP_LOGI(TAG, "Failed to add Humidity Meas cluster");
        return HUMIDITY_CLUSTER_STATUS_ERROR;
    }

    //setup reporting
    /*esp_zb_zcl_reporting_info_t reporting_info = {
        .direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV,
        .ep = ZIGBEE_ENDPOINT_1,
        .cluster_id = ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
        .cluster_role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        .attr_id = ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID,
        .manuf_code = ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC,
        .u.send_info.min_interval = HUMIDITY_MEAS_REPORT_MIN_INTERVAL_S,
        .u.send_info.max_interval = HUMIDITY_MEAS_REPORT_MAX_INTERVAL_S,
        .u.send_info.def_min_interval = HUMIDITY_MEAS_REPORT_MIN_INTERVAL_S,
        .u.send_info.def_max_interval = HUMIDITY_MEAS_REPORT_MAX_INTERVAL_S,
        .u.send_info.delta.u16 = HUMIDITY_MEAS_REPORT_DELTA,
    };
    if(ESP_OK != esp_zb_zcl_update_reporting_info(&reporting_info)){

        ESP_LOGI(TAG, "Failed to setup attrib reporting");
        return HUMIDITY_CLUSTER_STATUS_ERROR;
    }*/

    return HUMIDITY_CLUSTER_STATUS_OK;
}

/***************************************************************************//*!
*  \brief Set Humdity measurement value.
*
*   Set the Humidity Measurmeent attirbute value.
*   
*   Preconditions: Humdidity cluster is initialized.
*
*   Side Effects: None. 
*
*   \param[in]  rel_humidity            Relative humidity value.
*
*   \return     Operation status
*
*******************************************************************************/
HUMIDITY_Cluster_Ret_t HUMIDITY_SetRelHumidity(uint16_t rel_humidity){

    //Clip humidity value
    if(rel_humidity >= MAX_RELATIVE_HUMIDITY_LEVEL)     rel_humidity = MAX_RELATIVE_HUMIDITY_LEVEL;
    if(rel_humidity <= MIN_RELATIVE_HUMIDITY_LEVEL)     rel_humidity = MIN_RELATIVE_HUMIDITY_LEVEL;     

    //Update attribute value
    esp_zb_zcl_status_t ret;

    esp_zb_lock_acquire(portMAX_DELAY);

    ret = esp_zb_zcl_set_attribute_val(ZIGBEE_ENDPOINT_1, 
                                       ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT, 
                                       ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, 
                                       ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID,
                                       &rel_humidity,
                                       false);

    esp_zb_lock_release();

    if(ret != ESP_ZB_ZCL_STATUS_SUCCESS){
        ESP_LOGI(TAG, "Failed to set attrib: 0x%02x", ret);
        return HUMIDITY_CLUSTER_STATUS_ERROR;
    }

    return HUMIDITY_CLUSTER_STATUS_OK;
}

/***************************************************************************//*!
*  \brief Get Humdity measurement value.
*
*   Get the current Humidity Measurmeent attirbute value.
*   
*   Preconditions: Humdidity cluster is initialized.
*
*   Side Effects: None. 
*
*   \param[in]  pRel_humidity           Pointer to store the rel. humidity.
*
*   \return     Operation status
*
*******************************************************************************/
HUMIDITY_Cluster_Ret_t HUMIDITY_GetRelHumidity(uint16_t *pRel_humidity){

    if(pRel_humidity){
        ESP_LOGI(TAG, "Failed to get rel. humidity: Invalid param.");
        return HUMIDITY_CLUSTER_STATUS_ERROR;
    }

    esp_zb_zcl_attr_t *pAttrib;

    esp_zb_lock_acquire(portMAX_DELAY);

    pAttrib = esp_zb_zcl_get_attribute(ZIGBEE_ENDPOINT_1, 
                                       ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT, 
                                       ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, 
                                       ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID);

    esp_zb_lock_release();

    *pRel_humidity = *(uint16_t*)(pAttrib->data_p);

    return HUMIDITY_CLUSTER_STATUS_OK;
}

/******************************************************************************
*   Interrupts
*******************************************************************************/

