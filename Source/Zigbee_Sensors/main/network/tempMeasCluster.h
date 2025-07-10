#ifndef _TEMP_MEAS_CLUSTER_H
#define _TEMP_MEAS_CLUSTER_H

#include "esp_zigbee_core.h"

/******************************************************************************
*   Public Definitions
*******************************************************************************/
#define MAX_TEMPERATURE_VALUE               (100 * 100)//0.01*C
#define MIN_TEMPERATURE_VALUE               (-20 * 100)//0.01*C
#define INVALID_TEMPERATURE_VALUE           (ESP_ZB_ZCL_TEMP_MEASUREMENT_MEASURED_VALUE_UNKNOWN)

/******************************************************************************
*   Public Macros
*******************************************************************************/


/******************************************************************************
*   Public Data Types
*******************************************************************************/
typedef enum TEMP_Cluster_Ret_e{
    TEMP_CLUSTER_STATUS_ERROR,
    TEMP_CLUSTER_STATUS_OK,
}TEMP_Cluster_Ret_t;

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Error Check
*******************************************************************************/


/******************************************************************************
*   Public Functions
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
TEMP_Cluster_Ret_t TEMP_InitCluster(esp_zb_cluster_list_t *pCluster_list);

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
TEMP_Cluster_Ret_t TEMP_SetupReporting(void);

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
TEMP_Cluster_Ret_t TEMP_SetTemperature(int16_t temperature);

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
TEMP_Cluster_Ret_t TEMP_GetTemperature(int16_t *pTemperature);

#endif//_TEMP_MEAS_CLUSTER_H