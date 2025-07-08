/******************************************************************************
*   Includes
*******************************************************************************/
#include "sequencer_cfg.h"
#include "ledController.h"
#include "ledDriver.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/


/******************************************************************************
*   Private Macros
*******************************************************************************/


/******************************************************************************
*   Private Data Types
*******************************************************************************/


/******************************************************************************
*   Private Functions Declaration
*******************************************************************************/


/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Private Variables
*******************************************************************************/


/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/


/******************************************************************************
*   Public Functions Definitions
*******************************************************************************/
/***************************************************************************/ /*!
*  \brief Sequencer turn output ON.
*
*   This function is used to interface the sequencer module with the outputs.
*   It turn an Output ON.
*
*   Preconditions: None.
*
*   Side Effects: None.
*   
*   \param[in]  seq_id                  Sequence Output ID.
*
*   \return     Operation status
*
*******************************************************************************/
SEQUENCER_CFG_Ret_t SEQUENCER_CFG_TurnOn(SEQUENCE_OutputId_t seq_id){

    if(seq_id >= SEQUENCE_ID_NB){
        return SEQUENCER_CFG_STATUS_ERROR;
    }

    LED_Handle_t led_handle;

    switch(seq_id){
        case SEQUENCE_ID_RED_LED:
        {
            //Turn red led ON
            if(LED_STATUS_OK != LED_GetLedHandle(LED_CTRL_ID_RED, &led_handle)){
                return SEQUENCER_CFG_STATUS_ERROR;
            }
            else{
                LDRV_SetLedSinglePwmDuty(LDRV_CFG_MAX_PWM_DUTY, led_handle);
            }
        }
        break;

        case SEQUENCE_ID_GREEN_LED:
        {
            //Turn green led ON
            if(LED_STATUS_OK != LED_GetLedHandle(LED_CTRL_ID_GREEN, &led_handle)){
                return SEQUENCER_CFG_STATUS_ERROR;
            }
            else{
                LDRV_SetLedSinglePwmDuty(LDRV_CFG_MAX_PWM_DUTY, led_handle);
            }
        }
        break;

        default:
        {
            //Do nothing...
        }
        break;
    }

    return SEQUENCER_CFG_STATUS_OK;
}

/***************************************************************************/ /*!
*  \brief Sequencer turn output OFF.
*
*   This function is used to interface the sequencer module with the outputs.
*   It turn an Output OFF.
*
*   Preconditions: None.
*
*   Side Effects: None.
*   
*   \param[in]  seq_id                  Sequence Output ID.
*
*   \return     Operation status
*
*******************************************************************************/
SEQUENCER_CFG_Ret_t SEQUENCER_CFG_TurnOff(SEQUENCE_OutputId_t seq_id){

    if(seq_id >= SEQUENCE_ID_NB){
        return SEQUENCER_CFG_STATUS_ERROR;
    }

    LED_Handle_t led_handle;

    switch(seq_id){

        case SEQUENCE_ID_RED_LED:
        {
            //Turn red led OFF
            if(LED_STATUS_OK != LED_GetLedHandle(LED_CTRL_ID_RED, &led_handle)){
                return SEQUENCER_CFG_STATUS_ERROR;
            }
            else{
                LDRV_SetLedSinglePwmDuty(LDRV_CFG_MIN_PWM_DUTY, led_handle);
            }
        }
        break;

        case SEQUENCE_ID_GREEN_LED:
        {
            //Turn green led OFF
            //Turn green led ON
            if(LED_STATUS_OK != LED_GetLedHandle(LED_CTRL_ID_GREEN, &led_handle)){
                return SEQUENCER_CFG_STATUS_ERROR;
            }
            else{
                LDRV_SetLedSinglePwmDuty(LDRV_CFG_MIN_PWM_DUTY, led_handle);
            }
        }
        break;

        default:
        {
            //Do nothing....
        }
        break;
    }

    return SEQUENCER_CFG_STATUS_OK;
}

/******************************************************************************
*   Interrupts
*******************************************************************************/