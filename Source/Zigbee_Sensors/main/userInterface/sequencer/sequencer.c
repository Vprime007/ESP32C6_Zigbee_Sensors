/******************************************************************************
*   Includes
*******************************************************************************/
#include <stdlib.h>
#include "sequencer.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/


/******************************************************************************
*   Private Macros
*******************************************************************************/


/******************************************************************************
*   Private Data Types
*******************************************************************************/
typedef enum SEQUENCE_State_e{
    SEQUENCE_STATE_IDLE,
    SEQUENCE_STATE_START,
    SEQUENCE_STATE_OUTPUT_ON,
    SEQUENCE_STATE_OUTPUT_OFF,

    SEQUENCE_STATE_INVALID,
}SEQUENCE_State_t;

typedef struct SEQUENCE_Info_s{
    SEQUENCE_OutputId_t output_id;
    SEQUENCE_State_t state;
    SEQUENCE_t const *pSequence;
    uint32_t time_cptr;
    uint32_t repeat_cptr;
}SEQUENCE_Info_t;

/******************************************************************************
*   Private Functions Declaration
*******************************************************************************/
static void applySequenceOn(SEQUENCE_Info_t *pSequence);
static void applySequenceOff(SEQUENCE_Info_t *pSequence);
static void stopSequence(SEQUENCE_Info_t *pSequence);

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Private Variables
*******************************************************************************/
static SEQUENCE_Info_t sequence_info_table[SEQUENCE_ID_NB] = {0};

/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/
/***************************************************************************/ /*!
*  \brief Apply sequence ON.
*
*   Apply sequence ON to output.
*
*   Preconditions: None.
*
*   Side Effects: None.
*   
*   \param[in]  pSequence           Pointer to sequence info.
*
*******************************************************************************/
static void applySequenceOn(SEQUENCE_Info_t *pSequence){
    
    if(pSequence != NULL){
        SEQUENCER_CFG_TurnOn(pSequence->output_id);
        pSequence->time_cptr = 0;
        pSequence->state = SEQUENCE_STATE_OUTPUT_ON;
    }
}

/***************************************************************************/ /*!
*  \brief Apply sequence OFF.
*
*   Apply sequence OFF to output.
*
*   Preconditions: None.
*
*   Side Effects: None.
*   
*   \param[in]  pSequence           Pointer to sequence info.
*
*******************************************************************************/
static void applySequenceOff(SEQUENCE_Info_t *pSequence){

    if(pSequence != NULL){
        SEQUENCER_CFG_TurnOff(pSequence->output_id);
        pSequence->time_cptr = 0;
        pSequence->state = SEQUENCE_STATE_OUTPUT_OFF;
    }
}

/***************************************************************************/ /*!
*  \brief Stop sequence.
*
*   Stop the sequence.
*
*   Preconditions: None.
*
*   Side Effects: None.
*   
*   \param[in]  pSequence           Pointer to sequence info.
*
*******************************************************************************/
static void stopSequence(SEQUENCE_Info_t *pSequence){

    if(pSequence != NULL){
        SEQUENCER_CFG_TurnOff(pSequence->output_id);
        pSequence->state = SEQUENCE_STATE_IDLE;
    }
}

/******************************************************************************
*   Public Functions Definitions
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
SEQUENCER_Ret_t SEQUENCER_DoSequence(SEQUENCE_OutputId_t output_id, SEQUENCE_t *pSequence){

    if(output_id >= SEQUENCE_ID_NB){
        return SEQUENCER_STATUS_ERROR;
    }

    if(pSequence == NULL){
        return SEQUENCER_STATUS_ERROR;
    }

    //Store sequence infos in table
    sequence_info_table[output_id].output_id = output_id;
    sequence_info_table[output_id].pSequence = pSequence;
    sequence_info_table[output_id].time_cptr = 0;
    sequence_info_table[output_id].repeat_cptr = 0;
    sequence_info_table[output_id].state = SEQUENCE_STATE_INVALID;

    //Set output initial state
    if((sequence_info_table[output_id].pSequence->init_time_off == 0) && 
       (sequence_info_table[output_id].pSequence->time_on)){

        applySequenceOn(&sequence_info_table[output_id]);
    }
    else{
        sequence_info_table[output_id].state = SEQUENCE_STATE_START;
        SEQUENCER_CFG_TurnOff(output_id);
    }

    return SEQUENCER_STATUS_OK;
}

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
void SEQUENCER_Tic(void){

    SEQUENCE_Info_t *pSequence_info = NULL;

    for(uint8_t i=0; i<SEQUENCE_ID_NB; i++){

        pSequence_info = &sequence_info_table[i];

        switch(pSequence_info->state){

            case SEQUENCE_STATE_START:
            {
                pSequence_info->time_cptr++;
                if(pSequence_info->time_cptr >= pSequence_info->pSequence->init_time_off){

                    //check if the sequence has a ON time
                    if(pSequence_info->pSequence->time_on != 0){
                        applySequenceOn(pSequence_info);
                    }
                    else if(pSequence_info->pSequence->time_off != 0){
                        //Check if th sequence has OFF time
                        applySequenceOff(pSequence_info);
                    }
                    else{
                        //No ON/OFF time in the sequence... STOP it!
                        stopSequence(pSequence_info);
                    }
                }
            }
            break;

            case SEQUENCE_STATE_OUTPUT_ON:
            {
                if(pSequence_info->pSequence->time_on != SEQUENCE_ACTIVE_FOREVER){
                    //Check if ON time is reached
                    if(pSequence_info->time_cptr >= pSequence_info->pSequence->time_on){

                        //check if the seqeunce need to go OFF
                        if(pSequence_info->pSequence->time_off != 0){
                            applySequenceOff(pSequence_info);
                        }
                        else{
                            //The sequence does not go OFF -> Stop it!
                            stopSequence(pSequence_info);
                        }
                    }
                    else{
                        pSequence_info->time_cptr++;//Increment time cptr
                    }
                }
                else{
                    //Remain ON forever... Go IDLE
                    pSequence_info->state = SEQUENCE_STATE_IDLE;
                }
            }
            break;

            case SEQUENCE_STATE_OUTPUT_OFF:
            {
                if(pSequence_info->pSequence->time_off != SEQUENCE_ACTIVE_FOREVER){
                    //Check if OFF time is reached
                    if(pSequence_info->time_cptr >= pSequence_info->pSequence->time_off){

                        //check if we need to repeat the sequence forever
                        if(pSequence_info->repeat_cptr == SEQUENCE_REPEAT_FOREVER){
                            applySequenceOn(pSequence_info);
                        }
                        else{
                            pSequence_info->repeat_cptr++;
                            if(pSequence_info->repeat_cptr >= pSequence_info->pSequence->nb_repeat){
                                //Number of repeat reached... Stop it!
                                stopSequence(pSequence_info);
                            }
                            else{
                                applySequenceOn(pSequence_info);
                            }
                        }
                    }
                    else{
                        pSequence_info->time_cptr++;//Increment time cptr
                    }
                }
                else{
                    //Remain OFF forever... Go IDLE
                    pSequence_info->state = SEQUENCE_STATE_IDLE;
                }
            }
            break;

            case SEQUENCE_STATE_IDLE:
            case SEQUENCE_STATE_INVALID:
            default:
            {
                pSequence_info->state = SEQUENCE_STATE_IDLE;
            }
            break;
        }
    }
}

/******************************************************************************
*   Interrupts
*******************************************************************************/

