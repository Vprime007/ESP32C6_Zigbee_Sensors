#ifndef _BUTTON_CONTROLLER_H
#define _BUTTON_CONTROLLER_H

#include <stdint.h>

/******************************************************************************
*   Public Definitions
*******************************************************************************/


/******************************************************************************
*   Public Macros
*******************************************************************************/


/******************************************************************************
*   Public Data Types
*******************************************************************************/
typedef void(*ShortpressCallback_t)(void *args, void *user_data);
typedef void(*LongpressCallback_t)(void *args, void *user_data);

typedef enum BUTTON_Active_Level_e{
    BUTTON_ACTIVE_LOW,
    BUTTON_ACTIVE_HIGH,
}BUTTON_Active_Level_t;

typedef struct BUTTON_Config_s{
    uint8_t gpio;
    BUTTON_Active_Level_t active_level;
    ShortpressCallback_t shortpress_callback;
    LongpressCallback_t longpress_callback;
}BUTTON_Config_t;

typedef enum BUTTON_Ret_e{
    BUTTON_STATUS_ERROR,
    BUTTON_STATUS_OK,
}BUTTON_Ret_t;

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Error Check
*******************************************************************************/


/******************************************************************************
*   Public Functions
*******************************************************************************/
BUTTON_Ret_t BUTTON_InitController(BUTTON_Config_t const *pConfig);


#endif//_BUTTON_CONTROLLER_H