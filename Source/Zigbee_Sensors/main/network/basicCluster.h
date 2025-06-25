#ifndef _BASIC_CLUSTER_H
#define _BASIC_CLUSTER_H

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
typedef enum BASIC_Cluster_Ret_e{
    BASIC_CLUSTER_STATUS_ERROR,
    BASIC_CLUSTER_STATUS_OK,
}BASIC_Cluster_Ret_t;

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
BASIC_Cluster_Ret_t BASIC_IntiCluster(esp_zb_cluster_list_t *pCluster_list);

#endif//_BASIC_CLUSTER_H