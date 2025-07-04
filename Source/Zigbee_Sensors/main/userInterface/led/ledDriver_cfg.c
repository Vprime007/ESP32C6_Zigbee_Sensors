/******************************************************************************
*   Includes
*******************************************************************************/
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"
#include "ledDriver_config.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/
#define LDRV_TIMER_MODE                     (LEDC_LOW_SPEED_MODE)
#define LDRV_DEFAULT_FREQ_HZ                (1000)
#define LDRV_DEFAULT_DUTY_RES               (LEDC_TIMER_8_BIT)

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
static SemaphoreHandle_t ldrv_mutex_handle = NULL;

/******************************************************************************
*   Error Check
*******************************************************************************/


/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/


/******************************************************************************
*   CallBack Functions implementation
*******************************************************************************/


/******************************************************************************
*   Public Functions Definitions
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
LDRV_CFG_Ret_t LDRV_CFG_InitDriver(void){

    ldrv_mutex_handle = xSemaphoreCreateMutex();
    if(ldrv_mutex_handle == NULL){
        return LDRV_CFG_STATUS_ERROR;
    }

    return LDRV_CFG_STATUS_OK;
}

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
LDRV_CFG_Ret_t LDRV_CFG_StartFadeService(void){

    //Initialize the led fade service
    if(ESP_OK != ledc_fade_func_install(0)){
        return LDRV_CFG_STATUS_ERROR;
    }

    return LDRV_CFG_STATUS_OK;
}

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
LDRV_CFG_Ret_t LDRV_CFG_SetuptLedSingle(LDRV_CFG_Single_Config_t *pConfig){

    gpio_config_t cfg = {
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pin_bit_mask = (1ULL<<pConfig->gpio_num),
    };
    gpio_config(&cfg);

    //Set led OFF after setup
    gpio_set_level(pConfig->gpio_num, !(pConfig->active_level == LDRV_CFG_ACTIVE_HIGH));

    return LDRV_CFG_STATUS_OK;
}

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
LDRV_CFG_Ret_t LDRV_CFG_SetupLedSinglePwm(LDRV_CFG_Single_Pwm_Config_t *pConfig){

    //Config ledc 
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LDRV_TIMER_MODE,
        .duty_resolution = LDRV_DEFAULT_DUTY_RES,
        .timer_num = pConfig->led_timer,
        .freq_hz = LDRV_DEFAULT_FREQ_HZ,
        .clk_cfg = LEDC_USE_XTAL_CLK,
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LDRV_TIMER_MODE,
        .channel = pConfig->led_channel,
        .timer_sel = pConfig->led_timer,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = pConfig->gpio_num,
        .duty = LDRV_CFG_MIN_PWM_DUTY,
        .hpoint = 0,
    };
    ///Inverse duty cycle min value if active low
    if(pConfig->active_level == LDRV_CFG_ACTIVE_LOW)   ledc_channel.duty = LDRV_CFG_MAX_PWM_DUTY;

    ledc_channel_config(&ledc_channel);

    return LDRV_CFG_STATUS_OK;
}

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
LDRV_CFG_Ret_t LDRV_CFG_SetupLedRgb(LDRV_CFG_Rgb_Config_t *pConfig){

    //config ledc timer 
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LDRV_TIMER_MODE,
        .duty_resolution = LDRV_DEFAULT_DUTY_RES,
        .timer_num = pConfig->led_timer,
        .freq_hz = LDRV_DEFAULT_FREQ_HZ,
        .clk_cfg = LEDC_USE_XTAL_CLK,
    };
    ledc_timer_config(&ledc_timer);

    //config ledc channels
    ledc_channel_config_t rgb_channel = {
        .speed_mode = LDRV_TIMER_MODE,
        .channel = pConfig->red_channel,
        .timer_sel = pConfig->led_timer,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = pConfig->red_gpio_num,
        .duty = LDRV_CFG_MIN_PWM_DUTY,
        .hpoint = 0,
    };
    //Inverse duty cycle min value if active low
    if(pConfig->active_level == LDRV_CFG_ACTIVE_LOW)  rgb_channel.duty = LDRV_CFG_MAX_PWM_DUTY;
    ledc_channel_config(&rgb_channel);

    rgb_channel.channel = pConfig->green_channel;
    rgb_channel.gpio_num = pConfig->green_gpio_num;
    ledc_channel_config(&rgb_channel);

    rgb_channel.channel = pConfig->blue_channel;
    rgb_channel.gpio_num = pConfig->blue_gpio_num;
    ledc_channel_config(&rgb_channel);

    return LDRV_CFG_STATUS_OK;
}

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
                                          LDRV_CFG_Single_Config_t *pConfig){

    if(((state >= 1) && (pConfig->active_level == LDRV_CFG_ACTIVE_HIGH)) || 
      ((state == 0) && (pConfig->active_level == LDRV_CFG_ACTIVE_LOW))){

        //Set led active
        gpio_set_level(pConfig->gpio_num, (pConfig->active_level == LDRV_CFG_ACTIVE_HIGH));
    }
    else{
        //Set led inactive
        gpio_set_level(pConfig->gpio_num, !(pConfig->active_level == LDRV_CFG_ACTIVE_HIGH));
    }

    return LDRV_CFG_STATUS_OK;
}

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
                                            LDRV_CFG_Single_Pwm_Config_t *pConfig){

    if(ESP_OK != ledc_set_duty(LDRV_TIMER_MODE, pConfig->led_channel, duty)){
        return LDRV_CFG_STATUS_ERROR;
    }
    else{

        if(ESP_OK != ledc_update_duty(LDRV_TIMER_MODE, pConfig->led_channel)){
            return LDRV_CFG_STATUS_ERROR;
        }
    }

    return LDRV_CFG_STATUS_OK;
}

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
                                             LDRV_CFG_Single_Pwm_Config_t *pConfig){

    if(ESP_OK != ledc_set_fade_time_and_start(LDRV_TIMER_MODE, 
                                              pConfig->led_channel, 
                                              target_duty, 
                                              fade_time_ms, 
                                              LEDC_FADE_NO_WAIT)){

        return LDRV_CFG_STATUS_ERROR;
    }

    return LDRV_CFG_STATUS_OK;
}

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
                                       LDRV_CFG_Rgb_Config_t *pConfig){

    //set and update red rgb duty-cycle
    if(ESP_OK != ledc_set_duty(LDRV_TIMER_MODE, pConfig->red_channel, red_duty)){
        return LDRV_CFG_STATUS_ERROR;
    }
    else{
        if(ESP_OK != ledc_update_duty(LDRV_TIMER_MODE, pConfig->red_channel)){
            return LDRV_CFG_STATUS_ERROR;
        }
    }

    //set and update green duty-cycle
    if(ESP_OK != ledc_set_duty(LDRV_TIMER_MODE, pConfig->green_channel, green_duty)){
        return LDRV_CFG_STATUS_ERROR;
    }   
    else{
        if(ESP_OK != ledc_update_duty(LDRV_TIMER_MODE, pConfig->green_channel)){
            return LDRV_CFG_STATUS_ERROR;
        }
    } 

    //set and update blue duty-cycle
    if(ESP_OK != ledc_set_duty(LDRV_TIMER_MODE, pConfig->blue_channel, blue_duty)){
        return LDRV_CFG_STATUS_ERROR;
    }
    else{
        if(ESP_OK != ledc_update_duty(LDRV_TIMER_MODE, pConfig->blue_channel)){
            return LDRV_CFG_STATUS_ERROR;
        }
    }

    return LDRV_CFG_STATUS_OK;
}

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
                                        LDRV_CFG_Rgb_Config_t *pConfig){

    //Start red rgb fade
    if(ESP_OK != ledc_set_fade_time_and_start(LDRV_TIMER_MODE, 
                                              pConfig->red_channel, 
                                              target_red_duty, 
                                              fade_time_ms, 
                                              LEDC_FADE_NO_WAIT)){

        return LDRV_CFG_STATUS_ERROR;
    }

    //Start green rgb fade
    if(ESP_OK != ledc_set_fade_time_and_start(LDRV_TIMER_MODE, 
                                              pConfig->green_channel, 
                                              target_green_duty, 
                                              fade_time_ms, 
                                              LEDC_FADE_NO_WAIT)){

        return LDRV_CFG_STATUS_ERROR;
    }

    //Start blue rgb fade
    if(ESP_OK != ledc_set_fade_time_and_start(LDRV_TIMER_MODE, 
                                              pConfig->blue_channel, 
                                              target_blue_duty, 
                                              fade_time_ms, 
                                              LEDC_FADE_NO_WAIT)){

        return LDRV_CFG_STATUS_ERROR;
    }

    return LDRV_CFG_STATUS_OK;
}

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
                                            LDRV_CFG_Single_Pwm_Config_t *pConfig){

    if(ESP_OK != ledc_set_freq(LDRV_TIMER_MODE, pConfig->led_timer, freq_hz)){
        return LDRV_CFG_STATUS_ERROR;
    }

    return LDRV_CFG_STATUS_OK;
}

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
                                      LDRV_CFG_Rgb_Config_t *pConfig){

    if(ESP_OK != ledc_set_freq(LDRV_TIMER_MODE, pConfig->led_timer, freq_hz)){
        return LDRV_CFG_STATUS_ERROR;
    }

    return LDRV_CFG_STATUS_OK;
}

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
                                            LDRV_CFG_Single_Pwm_Config_t *pConfig){

    *pFreq_hz = ledc_get_freq(LDRV_TIMER_MODE, pConfig->led_timer);

    return LDRV_CFG_STATUS_OK;
}

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
                                      LDRV_CFG_Rgb_Config_t *pConfig){

    *pFreq_hz = ledc_get_freq(LDRV_TIMER_MODE, pConfig->led_timer);

    return LDRV_CFG_STATUS_OK;
}

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
void LDRV_CFG_TakeMutex(void){
    xSemaphoreTake(ldrv_mutex_handle, portMAX_DELAY);
}

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
void LDRV_CFG_GiveMutex(void){
    xSemaphoreGive(ldrv_mutex_handle);
}
