/******************************************************************************
*   Includes
*******************************************************************************/
#include "esp_log.h"

#include "zigbeeManager.h"
#include "tempMeasCluster.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/
#define TEMP_MEAS_REPORT_MIN_INTERVAL_S     (10)
#define TEMP_MEAS_REPORT_MAX_INTERVAL_S     (1 * 3600)
#define TEMP_MEAS_REPORT_DELTA              (100)

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


/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Private Variables
*******************************************************************************/
static const char * TAG = "TEMP_MEAS";

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/


/******************************************************************************
*   Public Functions Definitions
*******************************************************************************/
/***************************************************************************//*!
*  \brief Temperate cluster initialization.
*
*   Initialize Temperatue cluster and attributes with default value. It 
*   also add the Temperature cluster to the cluster list passed to this
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
TEMP_Cluster_Ret_t TEMP_InitCluster(esp_zb_cluster_list_t *pCluster_list){

    ESP_LOGI(TAG, "Cluster Initialization");

    esp_zb_temperature_meas_cluster_cfg_t temp_cfg = {
        .measured_value = ESP_ZB_ZCL_TEMP_MEASUREMENT_MEASURED_VALUE_UNKNOWN,
        .max_value = (MAX_TEMPERATURE_VALUE),
        .min_value = (MIN_TEMPERATURE_VALUE),
    };
    esp_zb_attribute_list_t *pTempCluster = esp_zb_temperature_meas_cluster_create(&temp_cfg);

    if(ESP_OK != esp_zb_cluster_list_add_temperature_meas_cluster(pCluster_list,
                                                                  pTempCluster,
                                                                  ESP_ZB_ZCL_CLUSTER_SERVER_ROLE)){

        ESP_LOGI(TAG, "Failed to add Temperature Meas Cluster");
        return TEMP_CLUSTER_STATUS_ERROR;

    }

    return TEMP_CLUSTER_STATUS_OK;
}

/***************************************************************************//*!
*  \brief Temperate cluster reporting setup.
*
*   Setup Temperature cluster attributes reporting parameters.  
*   
*   Preconditions: This function must be called AFTER esp_zb_device_register().
*
*   Side Effects: None. 
*
*   \return     Operation status
*
*******************************************************************************/
TEMP_Cluster_Ret_t TEMP_SetupReporting(void){

    //Setup cluter attrib reporting
    esp_zb_zcl_reporting_info_t reporting_info = {
        .direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV,
        .ep = ZIGBEE_ENDPOINT_1,
        .cluster_id = ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,
        .cluster_role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        .attr_id = ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID,
        .manuf_code = ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC,
        .u.send_info.min_interval = TEMP_MEAS_REPORT_MIN_INTERVAL_S,
        .u.send_info.max_interval = TEMP_MEAS_REPORT_MAX_INTERVAL_S,
        .u.send_info.def_min_interval = TEMP_MEAS_REPORT_MIN_INTERVAL_S,
        .u.send_info.def_max_interval = TEMP_MEAS_REPORT_MAX_INTERVAL_S,
        .u.send_info.delta.s16 = TEMP_MEAS_REPORT_DELTA,
    };
    if(ESP_OK != esp_zb_zcl_update_reporting_info(&reporting_info)){

        ESP_LOGI(TAG, "Failed to setup attrib reporting");
        return TEMP_CLUSTER_STATUS_ERROR;
    }

    return TEMP_CLUSTER_STATUS_OK;
}

/***************************************************************************//*!
*  \brief Set Temperature measurement value.
*
*   Set the Temperature Measurmeent attirbute value.
*   
*   Preconditions: Temperature cluster is initialized.
*
*   Side Effects: None. 
*
*   \param[in]  temperature             Temperature value.
*
*   \return     Operation status
*
*******************************************************************************/
TEMP_Cluster_Ret_t TEMP_SetTemperature(int16_t temperature){

    //Clip temperature measurement
    if(temperature >= MAX_TEMPERATURE_VALUE)    temperature = MAX_TEMPERATURE_VALUE;
    if(temperature <= MIN_TEMPERATURE_VALUE)    temperature = MIN_TEMPERATURE_VALUE;

    //Update attribute value
    esp_zb_zcl_status_t ret;

    esp_zb_lock_acquire(portMAX_DELAY);

    ret = esp_zb_zcl_set_attribute_val(ZIGBEE_ENDPOINT_1, 
                                       ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,
                                       ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
                                       ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID,
                                       &temperature,
                                       false);

    esp_zb_lock_release();

    if(ret != ESP_ZB_ZCL_STATUS_SUCCESS){
        ESP_LOGI(TAG, "Failed to set attrib: 0x%02x", ret);
        return TEMP_CLUSTER_STATUS_ERROR;
    }

    return TEMP_CLUSTER_STATUS_OK;
}

/***************************************************************************//*!
*  \brief Get Temperature measurement value.
*
*   Get the current Temperature Measurmeent attirbute value.
*   
*   Preconditions: Temperature cluster is initialized.
*
*   Side Effects: None. 
*
*   \param[in]  pTemperature           Pointer to store the temperature.
*
*   \return     Operation status
*
*******************************************************************************/
TEMP_Cluster_Ret_t TEMP_GetTemperature(int16_t *pTemperature){

    if(pTemperature == NULL){
        ESP_LOGI(TAG, "Failed to get temperature: Invalid param.");
        return TEMP_CLUSTER_STATUS_ERROR;
    }

    esp_zb_zcl_attr_t *pAttrib;

    esp_zb_lock_acquire(portMAX_DELAY);

    pAttrib = esp_zb_zcl_get_attribute(ZIGBEE_ENDPOINT_1, 
                                       ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT, 
                                       ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, 
                                       ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID);

    esp_zb_lock_release();

    *pTemperature = *(int16_t*)(pAttrib->data_p);

    return TEMP_CLUSTER_STATUS_OK;
}

/******************************************************************************
*   Interrupts
*******************************************************************************/

