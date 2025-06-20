#ifndef _ZIGBEE_MANAGER_H
#define _ZIGBEE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#include "esp_zigbee_core.h"

/******************************************************************************
*   Public Definitions
*******************************************************************************/
#define ZIGBEE_DEVICE_TYPE              (ESP_ZB_DEVICE_TYPE_ED)
#define ZIGBEE_ENDPOINT_1               (1)

/******************************************************************************
*   Public Macros
*******************************************************************************/


/******************************************************************************
*   Public Data Types
*******************************************************************************/
typedef enum ZIGBEE_Nwk_State_e{
    ZIGBEE_NWK_NOT_CONNECTED,
    ZIGBEE_NWK_CONNECTED,
    ZIGBEE_NWK_NO_PARENT,

    ZIGBEE_NWK_SCANNING,
    ZIGBEE_NWK_LEAVING,

    ZIGBEE_NWK_INVALID,
}ZIGBEE_Nwk_State_t;

typedef enum ZIGBEE_Ret_e{
    ZIGBEE_STATUS_ERROR,
    ZIGBEE_STATUS_OK,
}ZIGBEE_Ret_t;

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
*  \brief Init Zigbee stack
*
*   This function perform the zigbee stack initialization.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \return     Status of operation.   
*
*******************************************************************************/
ZIGBEE_Ret_t ZIGBEE_InitStack(void);

/***************************************************************************//*!
*  \brief Zigbee start stack
*
*   This function is used to start the zigbee stack
*   
*   Preconditions: None.
*
*   Side Effects: None. 
*
*******************************************************************************/
void ZIGBEE_StartStack(void);

/***************************************************************************//*!
*  \brief Zigbee start network scanning.
*
*   This function is use to start a zigbee network scan.
*   
*   Preconditions: None.
*
*   Side Effects: None. 
*
*   \return     Operation status
*
*******************************************************************************/
ZIGBEE_Ret_t ZIGBEE_StartScanning(void);

/***************************************************************************//*!
*  \brief Zigbee leave network.
*
*   This function is use to leave a zigbee network
*   
*   Preconditions: None.
*
*   Side Effects: None. 
*
*******************************************************************************/
void ZIGBEE_LeaveNetwork(void);

/***************************************************************************//*!
*  \brief Get Zigbee network state
*
*   This function return the current zigbee network state.
*   
*   Preconditions: None.
*
*   Side Effects: None. 
*
*   \return     zigbee network state
*
*******************************************************************************/
ZIGBEE_Nwk_State_t ZIGBEE_GetNwkState(void);

#endif//_NETORK_MANAGER_H