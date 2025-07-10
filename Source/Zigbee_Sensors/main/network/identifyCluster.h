#ifndef _IDENTIFY_CLUSTER_H
#define _IDENTIFY_CLUSTER_H

#include "esp_zigbee_core.h"

/******************************************************************************
*   Public Definitions
*******************************************************************************/


/******************************************************************************
*   Public Macros
*******************************************************************************/


/******************************************************************************
*   Public Data Types
*******************************************************************************/
typedef enum IDENTIFY_Cluster_Ret_e{
    IDENTIFY_CLUSTER_STATUS_ERROR,
    IDENTIFY_CLUSTER_STATUS_OK,
}IDENTIFY_Cluster_Ret_t;

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
IDENTIFY_Cluster_Ret_t IDENTIFY_InitCluster(esp_zb_cluster_list_t *pCluster_list);

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
IDENTIFY_Cluster_Ret_t IDENTIFY_SetupCmdHandler(void);

#endif//_IDENTIFY_CLUSTER_H