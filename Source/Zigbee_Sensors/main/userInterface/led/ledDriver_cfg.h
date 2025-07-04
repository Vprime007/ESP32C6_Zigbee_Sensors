#ifndef _LED_DRIVER_CFG_H
#define _LED_DRIVER_CFG_H

#include <stdint.h>
#include "driver/ledc.h"

/******************************************************************************
*   Public Definitions
*******************************************************************************/
#define LDRV_CFG_MAX_NB_LED                 (2)
#define LDRV_CFG_MAX_PWM_DUTY               (255)
#define LDRV_CFG_MIN_PWM_DUTY               (0)

/******************************************************************************
*   Public Macros
*******************************************************************************/


/******************************************************************************
*   Public Data Types
*******************************************************************************/
typedef enum LDRV_CFG_Active_Level_e{
    LDRV_CFG_ACTIVE_LOW,
    LDRV_CFG_ACTIVE_HIGH,

    LDRV_CFG_ACTIVE_INVALID,
}LDRV_CFG_Active_Level_t;

typedef struct LDRV_CFG_Single_Config_s{
    uint8_t gpio_num;
    LDRV_CFG_Active_Level_t active_level;
}LDRV_CFG_Single_Config_t;

typedef struct LDRV_CFG_Single_Pwm_Config_s{
    uint8_t gpio_num;
    LDRV_CFG_Active_Level_t active_level;
    ledc_timer_t led_timer;
    ledc_channel_t led_channel;
}LDRV_CFG_Single_Pwm_Config_t;

typedef struct LDRV_CFG_Rgb_Config_s{
    uint8_t red_gpio_num;
    uint8_t green_gpio_num;
    uint8_t blue_gpio_num;
    LDRV_CFG_Active_Level_t active_level;

    ledc_timer_t led_timer;
    ledc_channel_t red_channel;
    ledc_channel_t green_channel;
    ledc_channel_t blue_channel;
}LDRV_CFG_Rgb_Config_t;

typedef enum LDRV_CFG_Ret_e{
    LDRV_CFG_STATUS_ERROR,
    LDRV_CFG_STATUS_OK,
}LDRV_CFG_Ret_t;


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
*  \brief Init Led driver.
*
*   Init the related peripherals for the led driver. 
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \return     Operation status
*
*******************************************************************************/
LDRV_CFG_Ret_t LDRV_CFG_InitDriver(void);

/***************************************************************************//*!
*  \brief LED driver start fade service
*
*   This function is used to enable the fade service. Must be called after all
*   leds initialization to prevent deadlock.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \return     Operation status
*
*******************************************************************************/
LDRV_CFG_Ret_t LDRV_CFG_StartFadeService(void);

/***************************************************************************//*!
*  \brief Setup single led.
*
*   Setup the peripherals to drive a single led with ON/OFF control type.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  pConfig             Pointer to led configuration
*
*   \return     Operation status
*
*******************************************************************************/
LDRV_CFG_Ret_t LDRV_CFG_SetuptLedSingle(LDRV_CFG_Single_Config_t *pConfig);

/***************************************************************************//*!
*  \brief Setup single pwm led.
*
*   Setup the peripherals to drive a single led with PWM control type.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  pConfig             Pointer to led configuration
*
*   \return     Operation status
*
*******************************************************************************/
LDRV_CFG_Ret_t LDRV_CFG_SetupLedSinglePwm(LDRV_CFG_Single_Pwm_Config_t *pConfig);

/***************************************************************************//*!
*  \brief Setup RGB led.
*
*   Setup the peripherals to drive a RGB led (3x PWM control).
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  pConfig             Pointer to led configuration
*
*   \return     Operation status
*
*******************************************************************************/
LDRV_CFG_Ret_t LDRV_CFG_SetupLedRgb(LDRV_CFG_Rgb_Config_t *pConfig);

/***************************************************************************//*!
*  \brief Set single led state.
*
*   Set the state (ON/OFF) of a single led.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  state               (1->ON / 0->OFF)
*   \param[in]  pConfig             Pointer to led configuration
*
*   \return     Operation status
*
*******************************************************************************/
LDRV_CFG_Ret_t LDRV_CFG_SetLedSingleState(uint8_t state, 
                                          LDRV_CFG_Single_Config_t *pConfig);

/***************************************************************************//*!
*  \brief Set single led pwm duty-cycle.
*
*   Set the duty-cycle of a single led.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  duty                Led duty-cycle
*   \param[in]  pConfig             Pointer to led configuration
*
*   \return     Operation status
*
*******************************************************************************/
LDRV_CFG_Ret_t LDRV_CFG_SetLedSinglePwmDuty(uint32_t duty, 
                                            LDRV_CFG_Single_Pwm_Config_t *pConfig);

/***************************************************************************//*!
*  \brief Fade the duty-cycle of a single led.
*
*   Fade the duty-cycle to a target value over an amount of time (in ms).
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  target_duty         Final duty-cycle
*   \param[in]  fade_time_ms        Fade time in ms
*   \param[in]  pConfig             Pointer to led configuration
*
*   \return     Operation status
*
*******************************************************************************/
LDRV_CFG_Ret_t LDRV_CFG_FadeLedSinglePwmDuty(uint32_t target_duty, 
                                             uint32_t fade_time_ms, 
                                             LDRV_CFG_Single_Pwm_Config_t *pConfig);

/***************************************************************************//*!
*  \brief Set RGB led pwm duty-cycles.
*
*   Set the duty-cycles of a RGB led.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  red_duty            RGB red duty-cycle
*   \param[in]  green_duty          RGB green duty-cycle
*   \param[in]  blue_duty           RGB blue duty-cycle
*   \param[in]  pConfig             Pointer to led configuration
*
*   \return     Operation status
*
*******************************************************************************/
LDRV_CFG_Ret_t LDRV_CFG_SetLedRgbColor(uint32_t red_duty, 
                                       uint32_t green_duty, 
                                       uint32_t blue_duty, 
                                       LDRV_CFG_Rgb_Config_t *pConfig);

/***************************************************************************//*!
*  \brief Fade the duty-cycles of a RGB led.
*
*   Fade the duty-cycles to target values over an amount of time (in ms).
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  target_red_duty         Final RGB red duty-cycle
*   \param[in]  target_green_duty       Final RGB green duty-cycle 
*   \param[in]  target_blue_duty        Final RGB blue duty-cycle
*   \param[in]  fade_time_ms            Fade time in ms
*   \param[in]  pConfig                 Pointer to led configuration
*
*   \return     Operation status
*
*******************************************************************************/
LDRV_CFG_Ret_t LDRV_CFG_FadeLedRgbColor(uint32_t target_red_duty,
                                        uint32_t target_green_duty,
                                        uint32_t target_blue_duty,
                                        uint32_t fade_time_ms,
                                        LDRV_CFG_Rgb_Config_t *pConfig);

/***************************************************************************//*!
*  \brief Set single led pwm frequency
*
*   Set PWM frequency of a single led.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  freq_hz             PWM frequency (in Hz)
*   \param[in]  pConfig             Pointer to led configuration
*
*   \return     Operation status
*
*******************************************************************************/
LDRV_CFG_Ret_t LDRV_CFG_SetLedSinglePwmFreq(uint32_t freq_hz, 
                                            LDRV_CFG_Single_Pwm_Config_t *pConfig);

/***************************************************************************//*!
*  \brief Set RGB led pwms frequency
*
*   Set PWMs frequency of a RGB led.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  freq_hz             PWM frequency (in Hz)
*   \param[in]  pConfig             Pointer to led configuration
*
*   \return     Operation status
*
*******************************************************************************/
LDRV_CFG_Ret_t LDRV_CFG_SetLedRgbFreq(uint32_t freq_hz, 
                                      LDRV_CFG_Rgb_Config_t *pConfig);

/***************************************************************************//*!
*  \brief Get single led pwm frequency
*
*   Get PWM frequency of a single led.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[out]  pFreq_hz            Pointer to store the frequency (in Hz)
*   \param[in]   pConfig             Pointer to led configuration
*
*   \return     Operation status
*
*******************************************************************************/
LDRV_CFG_Ret_t LDRV_CFG_GetLedSinglePwmFreq(uint32_t *pFreq_hz,
                                            LDRV_CFG_Single_Pwm_Config_t *pConfig);

/***************************************************************************//*!
*  \brief Get RGB led pwms frequency
*
*   Get PWMs frequency of a RGB led.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[out]  pFreq_hz            Pointer to store the frequency (in Hz)
*   \param[in]   pConfig             Pointer to led configuration
*
*   \return     Operation status
*
*******************************************************************************/
LDRV_CFG_Ret_t LDRV_CFG_GetLedRgbFreq(uint32_t *pFreq_hz,
                                      LDRV_CFG_Rgb_Config_t *pConfig);

/***************************************************************************//*!
*  \brief Take led driver mutex
*
*   Take the led driver mutex. (If no mutex needed -> left empty).
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*******************************************************************************/
void LDRV_CFG_TakeMutex(void);

/***************************************************************************//*!
*  \brief Give led driver mutex
*
*   Give back the led driver mutex. (If no mutex needed -> left empty).
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*******************************************************************************/
void LDRV_CFG_GiveMutex(void);

#endif//_LED_DRIVER_CFG_H