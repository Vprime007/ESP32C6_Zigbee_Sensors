#ifndef _SEQUENCER_CFG_H
#define _SEQUENCER_CFG_H

/******************************************************************************
*   Public Definitions
*******************************************************************************/
#define SEQUENCER_TIC_PERIOD_MS             (10)
#define SEQUENCER_MS_TO_TIC(ms)             (ms/SEQUENCER_TIC_PERIOD_MS)

/******************************************************************************
*   Public Macros
*******************************************************************************/


/******************************************************************************
*   Public Data Types
*******************************************************************************/
typedef enum SEQUENCE_OutputId_e{
    SEQUENCE_ID_RED_LED,
    SEQUENCE_ID_GREEN_LED,

    SEQUENCE_ID_NB,
}SEQUENCE_OutputId_t;

typedef enum SEQUENCER_CFG_Ret_e{
    SEQUENCER_CFG_STATUS_ERROR,
    SEQUENCER_CFG_STATUS_OK,
}SEQUENCER_CFG_Ret_t;

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Error Check
*******************************************************************************/


/******************************************************************************
*   Public Functions
*******************************************************************************/
SEQUENCER_CFG_Ret_t SEQUENCER_CFG_TurnOn(SEQUENCE_OutputId_t seq_id);

SEQUENCER_CFG_Ret_t SEQUENCER_CFG_TurnOff(SEQUENCE_OutputId_t seq_id);

#endif//_SEQUENCER_CFG_H