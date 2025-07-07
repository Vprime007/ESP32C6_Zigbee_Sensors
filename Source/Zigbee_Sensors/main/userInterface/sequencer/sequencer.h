#ifndef _SEQUENCER_H
#define _SEQUENCER_H

#include <stdint.h>
#include "sequencer_cfg.h"

/******************************************************************************
*   Public Definitions
*******************************************************************************/
#define SEQUENCE_ACTIVE_FOREVER                 (0xFFFFFFFF)
#define SEQUENCE_REPEAT_FOREVER                 (0xFFFFFFFF)

/******************************************************************************
*   Public Macros
*******************************************************************************/


/******************************************************************************
*   Public Data Types
*******************************************************************************/
typedef struct SEQUENCE_s{
    uint32_t init_time_off;
    uint32_t time_on;
    uint32_t time_off;
    uint32_t nb_repeat;
}SEQUENCE_t;

typedef enum SEQUENCER_Ret_e{
    SEQUENCER_STATUS_ERROR,
    SEQUENCER_STATUS_OK,
}SEQUENCER_Ret_t;

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Error Check
*******************************************************************************/


/******************************************************************************
*   Public Functions
*******************************************************************************/
/***************************************************************************/ /*!
*  \brief Start a sequence.
*
*   This function is used to start a sequence for a specific output.
*
*   Preconditions: None.
*
*   Side Effects: None.
*   
*   \param[in]  output_id           Sequence Output ID.
*   \param[in]  pSequence           Pointer to sequence to apply.
*
*   \return     Operation status
*
*******************************************************************************/
SEQUENCER_Ret_t SEQUENCER_DoSequence(SEQUENCE_OutputId_t output_id, SEQUENCE_t *pSequence);

/***************************************************************************/ /*!
*  \brief Sequencer Tic.
*
*   This function must be called perdiodically at SEQUENCER_TIC_PERIOD_MS for 
*   the sequences timmings to be applied correctly. 
*
*   Preconditions: None.
*
*   Side Effects: None.
*
*******************************************************************************/
void SEQUENCER_Tic(void);

#endif//_SEQUENCER_H