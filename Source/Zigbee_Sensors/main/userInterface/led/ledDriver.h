#ifndef _LED_DRIVER_H
#define _LED_DRIVER_H

#include <stdint.h>
#include "ledDriver_cfg.h"

/******************************************************************************
*   Public Definitions
*******************************************************************************/
#define LED_DRIVER_HANDLE_INVALID           (0xFF);

#define LDRV_RGB_RED            (LDRV_Color_t){.red_duty = LDRV_CFG_MAX_PWM_DUTY, .green_duty = LDRV_CFG_MIN_PWM_DUTY, .blue_duty = LDRV_CFG_MIN_PWM_DUTY}
#define LDRV_RGB_GREEN          (LDRV_Color_t){.red_duty = LDRV_CFG_MIN_PWM_DUTY, .green_duty = LDRV_CFG_MAX_PWM_DUTY, .blue_duty = LDRV_CFG_MIN_PWM_DUTY}
#define LDRV_RGB_BLUE           (LDRV_Color_t){.red_duty = LDRV_CFG_MIN_PWM_DUTY, .green_duty = LDRV_CFG_MIN_PWM_DUTY, .blue_duty = LDRV_CFG_MAX_PWM_DUTY}
#define LDRV_RGB_WHITE          (LDRV_Color_t){.red_duty = LDRV_CFG_MAX_PWM_DUTY, .green_duty = LDRV_CFG_MAX_PWM_DUTY, .blue_duty = LDRV_CFG_MAX_PWM_DUTY}
#define LDRV_RGB_AQUA           (LDRV_Color_t){.red_duty = LDRV_CFG_MIN_PWM_DUTY, .green_duty = LDRV_CFG_MAX_PWM_DUTY, .blue_duty = LDRV_CFG_MAX_PWM_DUTY}
#define LDRV_RGB_PURPLE         (LDRV_Color_t){.red_duty = LDRV_CFG_MAX_PWM_DUTY, .green_duty = LDRV_CFG_MIN_PWM_DUTY, .blue_duty = LDRV_CFG_MAX_PWM_DUTY}
#define LDRV_RGB_YELLOW         (LDRV_Color_t){.red_duty = LDRV_CFG_MAX_PWM_DUTY, .green_duty = LDRV_CFG_MAX_PWM_DUTY, .blue_duty = LDRV_CFG_MIN_PWM_DUTY}
#define LDRV_RGB_ORANGE         (LDRV_Color_t){.red_duty = LDRV_CFG_MAX_PWM_DUTY, .green_duty = LDRV_CFG_MAX_PWM_DUTY/2, .blue_duty = LDRV_CFG_MIN_PWM_DUTY}
#define LDRV_RGB_BLACK          (LDRV_Color_t){.red_duty = LDRV_CFG_MIN_PWM_DUTY, .green_duty = LDRV_CFG_MIN_PWM_DUTY, .blue_duty = LDRV_CFG_MIN_PWM_DUTY}

/******************************************************************************
*   Public Macros
*******************************************************************************/
typedef uint8_t (LED_Handle_t);

/******************************************************************************
*   Public Data Types
*******************************************************************************/
typedef enum LDRV_Led_Type_e{
    LDRV_LED_TYPE_SINGLE,
    LDRV_LED_TYPE_SINGLE_PWM,
    LDRV_LED_TYPE_RGB,

    LDRV_LED_TYPE_INVALID,
}LDRV_Led_Type_t;

typedef struct LDRV_Color_s{
    uint32_t red_duty;
    uint32_t green_duty;
    uint32_t blue_duty;
}LDRV_Color_t;

typedef enum LDRV_Ret_e{
    LDRV_STATUS_ERROR,
    LDRV_STATUS_OK,
}LDRV_Ret_t;

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
*  \brief LED driver initialization
*
*   This function perform the LED Driver module initialization.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \return     Operation status
*
*******************************************************************************/
LDRV_Ret_t LDRV_InitDriver(void);

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
LDRV_Ret_t LDRV_StartFadeService(void);

/***************************************************************************//*!
*  \brief Add single led
*
*   Add a single type led to the driver. Single led is drive by a gpio and
*   it has two state (ON/OFF) based on the state of the digital IO and it's
*   active level. The function return a handle for the new led. The handle is 
*   necessary to change the level of the led.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]      single_config           Led configuration
*   \param[out]     pLed_handle             Pointer to led handle
*
*   \return         Operation status
*
*******************************************************************************/
LDRV_Ret_t LDRV_AddLedSingle(LDRV_CFG_Single_Config_t single_config, 
                             LED_Handle_t *pLed_handle);

/***************************************************************************//*!
*  \brief Add single pwm led 
*
*   Add a single pwm type led to the driver. Single pwm led is drive by the ledc
*   module and use a PWM signal to modulate it's brightness. The function return
*   a handle for the new led. The handle is used to change the brightness level
*   of the led.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]      single_pwm_config       Led configuration
*   \param[out]     pLed_handle             Pointer to led handle
*
*   \return         Operation status
*
*******************************************************************************/
LDRV_Ret_t LDRV_AddLedSinglePwm(LDRV_CFG_Single_Pwm_Config_t single_pwm_config, 
                                LED_Handle_t *pLed_handle);

/***************************************************************************//*!
*  \brief Add RGB led 
*
*   Add a RGB type led to the driver. RGB led are driven by three ledc PWM 
*   signals (one for each color). Each PWM signals can be used to change
*   the led brightness and/or color. The function return a hanlde for the
*   new led. The handle is used to change le color/brigthness level of the 
*   led.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]      rgb_config              Led configuration
*   \param[out]     pLed_handle             Pointer to led handle
*
*   \return         Operation status
*
*******************************************************************************/
LDRV_Ret_t LDRV_AddLedRgb(LDRV_CFG_Rgb_Config_t rgb_config, 
                          LED_Handle_t *pLed_handle);

/***************************************************************************//*!
*  \brief Set single led state.
*
*   This function is used to change the state of a single type led. 
*   State >= 1 -> ON / state == 0 -> OFF.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]      state                   state (ON/OFF)
*   \param[in]      led_handle              led handle
*
*   \return         Operation status
*
*******************************************************************************/
LDRV_Ret_t LDRV_SetLedSingleState(uint8_t state, LED_Handle_t led_handle);

/***************************************************************************//*!
*  \brief Set single pwm led duty-cycle.
*
*   This function is used to change the duty-cycle of a single pwm led.
*   The duty-cyle must be equal or smaller than LED_DRIVER_MAX_DUTY_CYCLE 
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]      duty_cycle              led duty-cycle
*   \param[in]      led_handle              led handle
*
*   \return         Operation status
*
*******************************************************************************/
LDRV_Ret_t LDRV_SetLedSinglePwmDuty(uint32_t duty_cycle, LED_Handle_t led_handle);

/***************************************************************************//*!
*  \brief Fade single pwm led.
*
*   This function is used to fade the single led pwm duty cyle to a specified
*   value within a certain amount of time (in ms).
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]      target_duty             Final fade duty-cycle
*   \param[in]      fade_timer_ms           Fade time (in ms)
*   \param[in]      led_handle              led handle
*
*   \return         Operation status
*
*******************************************************************************/
LDRV_Ret_t LDRV_FadeLedSinglePwmDuty(uint32_t target_duty, 
                                     uint32_t fade_time_ms, 
                                     LED_Handle_t led_handle);

/***************************************************************************//*!
*  \brief Set RGB led color
*
*   This function is used to change the color/brigthness of a RGB type led.
*   Each color (red/green/blue) pwm duty-cyle must be equal or smaller than
*   LED_DRIVER_MAX_DUTY_CYCLE.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]      color                   RGB color
*   \param[in]      led_handle              led handle
*
*   \return         Operation status
*
*******************************************************************************/
LDRV_Ret_t LDRV_SetLedRgbColor(LDRV_Color_t color, LED_Handle_t led_handle);

/***************************************************************************//*!
*  \brief Fade RGB led.
*
*   This function is used to fade RGB led to a specified color within 
*   a certain amount of time (in ms).
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]      target_color            Final fade color
*   \param[in]      fade_timer_ms           Fade time (in ms)
*   \param[in]      led_handle              led handle
*
*   \return         Operation status
*
*******************************************************************************/
LDRV_Ret_t LDRV_FadeLedRgbColor(LDRV_Color_t target_color, 
                                uint32_t fade_time_ms, 
                                LED_Handle_t led_handle);

/***************************************************************************//*!
*  \brief Set led frequency.
*
*   This function is used to change the frequency of the PWM signal(s).
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]      freq_hz                 PWM frequency in Hz
*   \param[in]      led_handle              led handle
*
*   \return         Operation status
*
*******************************************************************************/
LDRV_Ret_t LDRV_SetLedPwmFreq(uint32_t freq_hz, LED_Handle_t led_handle);

/***************************************************************************//*!
*  \brief Get led frequency.
*
*   This function is used to get the frequency of the PWM signal(s).
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]      led_handle              led handle
*   \param[in]      pFreq_hz                Freq return pointer
*
*   \return         Operation status
*
*******************************************************************************/
LDRV_Ret_t LDRV_GetLedPwmFreq(LED_Handle_t led_handle, uint32_t *pFreq_hz);

#endif//_LED_DRIVER_H