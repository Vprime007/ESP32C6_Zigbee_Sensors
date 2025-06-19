#ifndef _USER_INTERFACE_H
#define _USER_INTERFACE_H

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
typedef enum UI_Event_e{
    UI_EVENT_BOOT,
    UI_EVENT_FACTORY_RESET,
    UI_EVENT_IDENTIFY,

    UI_EVENT_SCANNING,
    UI_EVENT_CONNECTED,
    UI_EVENT_NOT_CONNECTED,
    UI_EVENT_NO_COORDO,

    UI_EVENT_BTN_SHORTPRESS,
    UI_EVENT_BTN_LONGPRESS,

    UI_EVENT_INVALID,
}UI_Event_t;

typedef enum UI_Ret_e{
    UI_STATUS_ERROR,
    UI_STATUS_OK,
}UI_Ret_t;

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
*  \brief User Interface initialization.
*
*   This function perform the User Interface module initialization.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \return     Operation status
*
*******************************************************************************/
UI_Ret_t UI_Init(void);

/***************************************************************************//*!
*  \brief Post user interface event.
*
*   This function post a user interface event. You can specify a timeout to
*   post the event. If no timeout is needed, set wait_ms to zero.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  event               UI event.
*   \param[in]  wait_ms             Post timeout in ms.
*
*   \return     Operation status
*
*******************************************************************************/
UI_Ret_t UI_PostEvent(UI_Event_t event, uint32_t wait_ms);

#endif//_USER_INTERFACE_H