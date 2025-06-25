#ifndef _HUMIDITY_MEAS_CLUSTER_H
#define _HUMIDITY_MEAS_CLUSTER_H

#include "esp_zigbee_core.h"

/******************************************************************************
*   Public Definitions
*******************************************************************************/
#define MAX_RELATIVE_HUMIDITY_LEVEL             (100 * 100)
#define MIN_RELATIVE_HUMIDITY_LEVEL             (0)

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