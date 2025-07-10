#ifndef _HUMIDITY_MEAS_CLUSTER_H
#define _HUMIDITY_MEAS_CLUSTER_H

#include "esp_zigbee_core.h"

/******************************************************************************
*   Public Definitions
*******************************************************************************/
#define MAX_RELATIVE_HUMIDITY_LEVEL             (ESP_ZB_ZCL_REL_HUMIDITY_MEASUREMENT_MAX_MEASURED_VALUE_MAXIMUM)
#define MIN_RELATIVE_HUMIDITY_LEVEL             (ESP_ZB_ZCL_REL_HUMIDITY_MEASUREMENT_MIN_MEASURED_VALUE_MINIMUM)
#define INVALID_RELATIVE_HUMIDITY_LEVEL         (ESP_ZB_ZCL_REL_HUMIDITY_MEASUREMENT_MEASURED_VALUE_UNKNOWN)

/******************************************************************************
*   Public Macros
*******************************************************************************/


/******************************************************************************
*   Public Data Types
*******************************************************************************/
typedef enum HUMIDITY_Cluster_Ret_e{
    HUMIDITY_CLUSTER_STATUS_ERROR,
    HUMIDITY_CLUSTER_STATUS_OK,
}HUMIDITY_Cluster_Ret_t;

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
HUMIDITY_Cluster_Ret_t HUMIDITY_InitCluster(esp_zb_cluster_list_t *pCluster_list);

/***************************************************************************//*!
*  \brief Humidity cluster reporting setup.
*
*   Setup Humidity cluster attributes reporting parameters.  
*   
*   Preconditions: This function must be called AFTER esp_zb_device_register().
*
*   Side Effects: None. 
*
*   \return     Operation status
*
*******************************************************************************/
HUMIDITY_Cluster_Ret_t HUMIDITY_SetupReporting(void);

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
HUMIDITY_Cluster_Ret_t HUMIDITY_SetRelHumidity(uint16_t rel_humidity);

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
HUMIDITY_Cluster_Ret_t HUMIDITY_GetRelHumidity(uint16_t *pRel_humidity);

#endif//_HUMIDITY_MEAS_CLUSTER_H