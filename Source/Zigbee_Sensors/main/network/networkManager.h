#ifndef _NETWORK_MANAGER_H
#define _NETWORK_MANAGER_H

/******************************************************************************
*   Public Definitions
*******************************************************************************/


/******************************************************************************
*   Public Macros
*******************************************************************************/


/******************************************************************************
*   Public Data Types
*******************************************************************************/
typedef enum NETWORK_State_e{
    NETWORK_STATE_NOT_CONNECTED,
    NETWORK_STATE_CONNECTED,
    NETWORK_STATE_NO_COORDO,
    NETWORK_STATE_SCANNING,

    NETWORK_STATE_INVALID,
}NETWORK_State_t;

typedef enum NETWORK_Ret_e{
    NETWORK_STATUS_ERROR,
    NETWORK_STATUS_OK,
}NETWORK_Ret_t;

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Error Check
*******************************************************************************/


/******************************************************************************
*   Public Functions
*******************************************************************************/
NETWORK_Ret_t NETWORK_Init(void);

NETWORK_Ret_t NETWORK_StartScanning(void);

NETWORK_State_t NETWORK_GetState(void);

void NETWORK_StartZigbee(void);

#endif//_NETORK_MANAGER_H