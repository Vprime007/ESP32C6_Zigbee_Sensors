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
#define ZIGBEE_INSTALLCODE_POLICY       (false)
#define ZIGBEE_ED_AGING_TIMEOUT         (ESP_ZB_ED_AGING_TIMEOUT_64MIN)
#define ZIGBEE_ED_KEEP_ALIVE_MS         (7 * 1000)
#define ZIGBEE_PRIMARY_CHANNEL_MASK     (ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK)

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

typedef void(*networkStateChangeCallback_t)(ZIGBEE_Nwk_State_t nwk_state);

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
*   If notification on network state change are desired, set param
*   nwk_change_callback with a non-NULL value.   
*
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  nwk_change_callback     Network state change callback.
*
*   \return     Status of operation.   
*
*******************************************************************************/
ZIGBEE_Ret_t ZIGBEE_InitStack(networkStateChangeCallback_t nwk_change_callback);

/***************************************************************************//*!
*  \brief Zigbee start stack
*
*   This function is used to start the zigbee stack
*   
*   Preconditions: Zigbee stack is initialized.
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
*   Preconditions: Zigbee stack is started and running.
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
*   Preconditions: Zigbee stack is started and running.
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