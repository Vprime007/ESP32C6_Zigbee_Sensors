#ifndef _LED_CONTROLLER_H
#define _LED_CONTROLLER_H

#include <stdint.h>
#include "ledDriver_cfg.h"

/******************************************************************************
*   Public Definitions
*******************************************************************************/


/******************************************************************************
*   Public Macros
*******************************************************************************/


/******************************************************************************
*   Public Data Types
*******************************************************************************/
typedef enum LED_Pattern_e{
    LED_PATTERN_BOOT,
    LED_PATTERN_FACTORY_RESET,
    LED_PATTERN_IDENTIFY,

    LED_PATTERN_CONNECTED,
    LED_PATTERN_NOT_CONNECTED,
    LED_PATTERN_NO_COORDO,
    LED_PATTERN_SCANNING,

    LED_PATTERN_INVALID,
}LED_Pattern_t;

typedef enum LED_Ret_e{
    LED_STATUS_ERROR,
    LED_STATUS_OK,
}LED_Ret_t;

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Error Check
*******************************************************************************/


/******************************************************************************
*   Public Functions
*******************************************************************************/
LED_Ret_t LED_InitController(uint8_t led_strip_gpio);

LED_Ret_t LED_StartPattern(LED_Pattern_t pattern);

LED_Ret_t LED_StopPattern(LED_Pattern_t pattern);

#endif//_LED_CONTROLLER_H