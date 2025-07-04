/******************************************************************************
*   Includes
*******************************************************************************/
#include <stdbool.h>
#include "ledDriver.h"

/******************************************************************************
*   Private Definitions
*******************************************************************************/
#define NO_AVAILABLE_INDEX                  (0xFF)

/******************************************************************************
*   Private Macros
*******************************************************************************/


/******************************************************************************
*   Private Data Types
*******************************************************************************/
typedef struct LDRV_LED_Info_s{
    LDRV_Led_Type_t led_type;
    bool active_flag;
    union {
        LDRV_CFG_Single_Config_t single_config;
        LDRV_CFG_Single_Pwm_Config_t single_pwm_config;
        LDRV_CFG_Rgb_Config_t rgb_config;
    }config;
}LDRV_LED_Info_t;

/******************************************************************************
*   Private Functions Declaration
*******************************************************************************/
static bool isTableFull(void);
static uint8_t getFirstAvailableIndex(void);

/******************************************************************************
*   Public Variables
*******************************************************************************/


/******************************************************************************
*   Private Variables
*******************************************************************************/
static LDRV_LED_Info_t led_table[LDRV_CFG_MAX_NB_LED];

/******************************************************************************
*   Error Check
*******************************************************************************/


/******************************************************************************
*   Private Functions Definitions
*******************************************************************************/
/***************************************************************************//*!
*  \brief Is led table full
*
*   This function return True if the led table is full. Else it return False.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \return     (True -> Table full / False -> Table not full)
*
*******************************************************************************/
static bool isTableFull(void){

    uint8_t index = 0;
    for(index = 0; index<LDRV_CFG_MAX_NB_LED; index++){
        if(led_table[index].active_flag == false)   break;
    }

    return (index >= LDRV_CFG_MAX_NB_LED);
}

/***************************************************************************//*!
*  \brief Get first available index
*
*   This function return the first available index in the led table.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \return     Available index
*
*******************************************************************************/
static uint8_t getFirstAvailableIndex(void){

    uint8_t index = 0;
    for(index = 0; index<LDRV_CFG_MAX_NB_LED; index++){
        if(led_table[index].active_flag == false)   break;
    }

    if(index >= LDRV_CFG_MAX_NB_LED)    index = NO_AVAILABLE_INDEX;

    return index;
}

/******************************************************************************
*   CallBack Functions implementation
*******************************************************************************/


/******************************************************************************
*   Public Functions Definitions
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
LDRV_Ret_t LDRV_InitDriver(void){

    if(LDRV_CFG_STATUS_OK != LDRV_CFG_InitDriver()){
        return LDRV_STATUS_ERROR;
    }

    LDRV_CFG_TakeMutex();

    for(uint8_t i=0; i<LDRV_CFG_MAX_NB_LED; i++){
        led_table[i].active_flag = false;
    }

    LDRV_CFG_GiveMutex();

    return LDRV_STATUS_OK;
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
LDRV_Ret_t LDRV_StartFadeService(void){
    
    if(LDRV_CFG_STATUS_OK != LDRV_CFG_StartFadeService()){
        return LDRV_STATUS_ERROR;
    }
    
    return LDRV_STATUS_OK;
}

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
                             LED_Handle_t *pLed_handle){

    LDRV_CFG_TakeMutex();

    //Check if led table is full
    if(isTableFull()){
        LDRV_CFG_GiveMutex();
    return LDRV_STATUS_ERROR;
    }

    //Get first available table index
    uint8_t index = getFirstAvailableIndex();
    if(index == NO_AVAILABLE_INDEX){
        LDRV_CFG_GiveMutex();
        return LDRV_STATUS_ERROR;
    }

    //Setup peripherals for the led
    if(LDRV_CFG_STATUS_OK != LDRV_CFG_SetuptLedSingle(&single_config)){
        LDRV_CFG_GiveMutex();
        return LDRV_STATUS_ERROR;
    }

    //Store led infos in table
    led_table[index].led_type = LDRV_LED_TYPE_SINGLE;
    led_table[index].config.single_config = single_config;
    led_table[index].active_flag = true;
    *pLed_handle = index;

    LDRV_CFG_GiveMutex();

    return LDRV_STATUS_OK;
}

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
                                LED_Handle_t *pLed_handle){

    LDRV_CFG_TakeMutex();

    //Check if led table is full
    if(isTableFull()){
        LDRV_CFG_GiveMutex();
        return LDRV_STATUS_ERROR;
    }

    //Get first available table index
    uint8_t index = getFirstAvailableIndex();
    if(index == NO_AVAILABLE_INDEX){
        LDRV_CFG_GiveMutex();
        return LDRV_STATUS_ERROR;
    }

    //Setup peripherals for the led
    if(LDRV_CFG_STATUS_OK != LDRV_CFG_SetupLedSinglePwm(&single_pwm_config)){
        LDRV_CFG_GiveMutex();
    return LDRV_STATUS_ERROR;
    }

    //Store led infos in table
    led_table[index].led_type = LDRV_LED_TYPE_SINGLE_PWM;
    led_table[index].config.single_pwm_config = single_pwm_config;
    led_table[index].active_flag = true;
    *pLed_handle = index;

    LDRV_CFG_GiveMutex();

    return LDRV_STATUS_OK;
}

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
LDRV_Ret_t LDRV_AddLedRgb(LDRV_CFG_Rgb_Config_t rgb_config, LED_Handle_t *pLed_handle){

    LDRV_CFG_TakeMutex();

    //Check if led table is full
    if(isTableFull()){
        LDRV_CFG_GiveMutex();
        return LDRV_STATUS_ERROR;
    }

    //Get first available table index
    uint8_t index = getFirstAvailableIndex();
    if(index == NO_AVAILABLE_INDEX){
        LDRV_CFG_GiveMutex();
        return LDRV_STATUS_ERROR;
    }

    //Setup peripherals for the led
    if(LDRV_CFG_STATUS_OK != LDRV_CFG_SetupLedRgb(&rgb_config)){
        LDRV_CFG_GiveMutex();
        return LDRV_STATUS_ERROR;
    }

    //Store led infos in table
    led_table[index].led_type = LDRV_LED_TYPE_RGB;
    led_table[index].config.rgb_config = rgb_config;
    led_table[index].active_flag = true;
    *pLed_handle = index;

    LDRV_CFG_GiveMutex();

    return LDRV_STATUS_OK;
}

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
LDRV_Ret_t LDRV_SetLedSingleState(uint8_t state, LED_Handle_t led_handle){

    if(led_handle >= LDRV_CFG_MAX_NB_LED){
        return LDRV_STATUS_ERROR;
    }

    LDRV_CFG_TakeMutex();
    LDRV_LED_Info_t led_info = led_table[led_handle]; 
    LDRV_CFG_GiveMutex();

    if(led_info.active_flag == false){
        return LDRV_STATUS_ERROR;
    }

    if(led_info.led_type != LDRV_LED_TYPE_SINGLE){    
        return LDRV_STATUS_ERROR;
    }

    if(LDRV_CFG_STATUS_OK != LDRV_CFG_SetLedSingleState(state, 
                                                        &led_info.config.single_config)){
        return LDRV_STATUS_ERROR;
    }

    return LDRV_STATUS_OK;
}

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
LDRV_Ret_t LDRV_SetLedSinglePwmDuty(uint32_t duty_cycle, LED_Handle_t led_handle){

    if(led_handle >= LDRV_CFG_MAX_NB_LED){
        return LDRV_STATUS_ERROR;
    }

    LDRV_CFG_TakeMutex();
    LDRV_LED_Info_t led_info = led_table[led_handle]; 
    LDRV_CFG_GiveMutex();

    if(led_info.active_flag == false){
        return LDRV_STATUS_ERROR;
    }

    if(led_info.led_type != LDRV_LED_TYPE_SINGLE_PWM){  
        return LDRV_STATUS_ERROR;
    }

    //Clip duty-cycle
    if(duty_cycle >= LDRV_CFG_MAX_PWM_DUTY)     duty_cycle = LDRV_CFG_MAX_PWM_DUTY;
    if(duty_cycle <= LDRV_CFG_MIN_PWM_DUTY)     duty_cycle = LDRV_CFG_MIN_PWM_DUTY;

    //Invserse duty-cycle if active low
    if(led_info.config.single_pwm_config.active_level == LDRV_CFG_ACTIVE_LOW){
        if(duty_cycle == LDRV_CFG_MAX_PWM_DUTY){
            duty_cycle = LDRV_CFG_MIN_PWM_DUTY;
        }
        else if(duty_cycle == LDRV_CFG_MIN_PWM_DUTY){
            duty_cycle = LDRV_CFG_MAX_PWM_DUTY + 1;
        }
        else{
            duty_cycle = LDRV_CFG_MAX_PWM_DUTY - duty_cycle;
        }
    }

    if(LDRV_CFG_STATUS_OK != LDRV_CFG_SetLedSinglePwmDuty(duty_cycle, 
                                                          &led_info.config.single_pwm_config)){
        return LDRV_STATUS_ERROR;
    }

    return LDRV_STATUS_OK;
}

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
                                     LED_Handle_t led_handle){

    if(led_handle >= LDRV_CFG_MAX_NB_LED){
        return LDRV_STATUS_ERROR;
    }

    LDRV_CFG_TakeMutex();
    LDRV_LED_Info_t led_info = led_table[led_handle]; 
    LDRV_CFG_GiveMutex();

    if(led_info.active_flag == false){
        return LDRV_STATUS_ERROR;
    }

    if(led_info.led_type != LDRV_LED_TYPE_SINGLE_PWM){
        return LDRV_STATUS_ERROR;
    }

    //clip duty-cycle
    if(target_duty >= LDRV_CFG_MAX_PWM_DUTY)     target_duty = LDRV_CFG_MAX_PWM_DUTY;
    if(target_duty <= LDRV_CFG_MIN_PWM_DUTY)     target_duty = LDRV_CFG_MIN_PWM_DUTY;

    //Invserse duty-cycle if active low
    if(led_info.config.single_pwm_config.active_level == LDRV_CFG_ACTIVE_LOW){
        if(target_duty == LDRV_CFG_MAX_PWM_DUTY){
            target_duty = LDRV_CFG_MIN_PWM_DUTY;
        }
        else if(target_duty == LDRV_CFG_MIN_PWM_DUTY){
            target_duty = LDRV_CFG_MAX_PWM_DUTY + 1;
        }
        else{
            target_duty = LDRV_CFG_MAX_PWM_DUTY - target_duty;
        }
    }

    if(LDRV_CFG_STATUS_OK != LDRV_CFG_FadeLedSinglePwmDuty(target_duty, 
                                                           fade_time_ms, 
                                                           &led_info.config.single_pwm_config)){
        return LDRV_STATUS_ERROR;
    }

    return LDRV_STATUS_OK;
}

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
LDRV_Ret_t LDRV_SetLedRgbColor(LDRV_Color_t color, LED_Handle_t led_handle){

    if(led_handle >= LDRV_CFG_MAX_NB_LED){
        return LDRV_STATUS_ERROR;
    }

    LDRV_CFG_TakeMutex();
    LDRV_LED_Info_t led_info = led_table[led_handle]; 
    LDRV_CFG_GiveMutex();

    if(led_info.active_flag == false){
        return LDRV_STATUS_ERROR;
    }

    if(led_info.led_type != LDRV_LED_TYPE_RGB){
        return LDRV_STATUS_ERROR;
    }

    //clip red duty-cycle
    if(color.red_duty >= LDRV_CFG_MAX_PWM_DUTY)     color.red_duty = LDRV_CFG_MAX_PWM_DUTY;
    if(color.red_duty <= LDRV_CFG_MIN_PWM_DUTY)     color.red_duty = LDRV_CFG_MIN_PWM_DUTY;

    //clip green duty-cycle
    if(color.green_duty >= LDRV_CFG_MAX_PWM_DUTY)     color.green_duty = LDRV_CFG_MAX_PWM_DUTY;
    if(color.green_duty <= LDRV_CFG_MIN_PWM_DUTY)     color.green_duty = LDRV_CFG_MIN_PWM_DUTY;

    //clip blue duty-cycle
    if(color.blue_duty >= LDRV_CFG_MAX_PWM_DUTY)     color.blue_duty = LDRV_CFG_MAX_PWM_DUTY;
    if(color.blue_duty <= LDRV_CFG_MIN_PWM_DUTY)     color.blue_duty = LDRV_CFG_MIN_PWM_DUTY;

    //Inverse duty-cycle if active low
    if(led_info.config.rgb_config.active_level == LDRV_CFG_ACTIVE_LOW){
        //Inverse red duty-cycle
        if(color.red_duty == LDRV_CFG_MAX_PWM_DUTY){
            color.red_duty = LDRV_CFG_MIN_PWM_DUTY;
        }
        else if(color.red_duty == LDRV_CFG_MIN_PWM_DUTY){
            color.red_duty = LDRV_CFG_MAX_PWM_DUTY + 1;
        }   
        else{
            color.red_duty = LDRV_CFG_MAX_PWM_DUTY - color.red_duty;
        }

        //Inverse green duty-cycle
        if(color.green_duty == LDRV_CFG_MAX_PWM_DUTY){
            color.green_duty = LDRV_CFG_MIN_PWM_DUTY;
        }
        else if(color.green_duty == LDRV_CFG_MIN_PWM_DUTY){
            color.green_duty = LDRV_CFG_MAX_PWM_DUTY + 1;
        }
        else{
            color.green_duty = LDRV_CFG_MAX_PWM_DUTY - color.green_duty;
        }

        //Inverse blue duty-cycle
        if(color.blue_duty == LDRV_CFG_MAX_PWM_DUTY){
            color.blue_duty = LDRV_CFG_MIN_PWM_DUTY;
        }
        else if(color.blue_duty == LDRV_CFG_MIN_PWM_DUTY){
            color.blue_duty = LDRV_CFG_MAX_PWM_DUTY + 1;
        }
        else{
            color.blue_duty = LDRV_CFG_MAX_PWM_DUTY - color.blue_duty;
        }
    }

    if(LDRV_CFG_STATUS_OK != LDRV_CFG_SetLedRgbColor(color.red_duty, 
                                                     color.green_duty, 
                                                     color.blue_duty, 
                                                     &led_info.config.rgb_config)){
        return LDRV_STATUS_ERROR;
    }

    return LDRV_STATUS_OK;
}

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
                                LED_Handle_t led_handle){

    if(led_handle >= LDRV_CFG_MAX_NB_LED){
        return LDRV_STATUS_ERROR;
    }

    LDRV_CFG_TakeMutex();
    LDRV_LED_Info_t led_info = led_table[led_handle]; 
    LDRV_CFG_GiveMutex();

    if(led_info.active_flag == false){
        return LDRV_STATUS_ERROR;
    }

    if(led_info.led_type != LDRV_LED_TYPE_RGB){
        return LDRV_STATUS_ERROR;
    }

    //Inverse duty-cycle if active low
    if(led_info.config.rgb_config.active_level == LDRV_CFG_ACTIVE_LOW){
        //Inverse red duty-cycle
        if(target_color.red_duty == LDRV_CFG_MAX_PWM_DUTY){
            target_color.red_duty = LDRV_CFG_MIN_PWM_DUTY;
        }
        else if(target_color.red_duty == LDRV_CFG_MIN_PWM_DUTY){
            target_color.red_duty = LDRV_CFG_MAX_PWM_DUTY + 1;
        }   
        else{
            target_color.red_duty = LDRV_CFG_MAX_PWM_DUTY - target_color.red_duty;
        }

        //Inverse green duty-cycle
        if(target_color.green_duty == LDRV_CFG_MAX_PWM_DUTY){
            target_color.green_duty = LDRV_CFG_MIN_PWM_DUTY;
        }
        else if(target_color.green_duty == LDRV_CFG_MIN_PWM_DUTY){
            target_color.green_duty = LDRV_CFG_MAX_PWM_DUTY + 1;
        }
        else{
            target_color.green_duty = LDRV_CFG_MAX_PWM_DUTY - target_color.green_duty;
        }

        //Inverse blue duty-cycle
        if(target_color.blue_duty == LDRV_CFG_MAX_PWM_DUTY){
            target_color.blue_duty = LDRV_CFG_MIN_PWM_DUTY;
        }
        else if(target_color.blue_duty == LDRV_CFG_MIN_PWM_DUTY){
            target_color.blue_duty = LDRV_CFG_MAX_PWM_DUTY + 1;
        }
        else{
            target_color.blue_duty = LDRV_CFG_MAX_PWM_DUTY - target_color.blue_duty;
        }
    }

    if(LDRV_CFG_STATUS_OK != LDRV_CFG_FadeLedRgbColor(target_color.red_duty, 
                                                      target_color.green_duty, 
                                                      target_color.blue_duty, 
                                                      fade_time_ms, 
                                                      &led_info.config.rgb_config)){
        return LDRV_STATUS_ERROR;
    }

    return LDRV_STATUS_OK;
}

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
LDRV_Ret_t LDRV_SetLedPwmFreq(uint32_t freq_hz, LED_Handle_t led_handle){

    if(led_handle >= LDRV_CFG_MAX_NB_LED){
        return LDRV_STATUS_ERROR;
    }

    LDRV_CFG_TakeMutex();
    LDRV_LED_Info_t led_info = led_table[led_handle]; 
    LDRV_CFG_GiveMutex();

    if(led_info.active_flag == false){
        return LDRV_STATUS_ERROR;
    }

    if(led_info.led_type == LDRV_LED_TYPE_SINGLE_PWM){
        if(LDRV_CFG_STATUS_OK != LDRV_CFG_SetLedSinglePwmFreq(freq_hz, &led_info.config.single_pwm_config)){
            return LDRV_STATUS_ERROR;
        }
    }
    else if(led_info.led_type == LDRV_LED_TYPE_RGB){
        if(LDRV_CFG_STATUS_OK != LDRV_CFG_SetLedRgbFreq(freq_hz, &led_info.config.rgb_config)){
            return LDRV_STATUS_ERROR;
        }
    }
    else{
        return LDRV_STATUS_ERROR;
    }

    return LDRV_STATUS_OK;
}

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
LDRV_Ret_t LDRV_GetLedPwmFreq(LED_Handle_t led_handle, uint32_t *pFreq_hz){

    if(led_handle >= LDRV_CFG_MAX_NB_LED){
        return LDRV_STATUS_ERROR;
    }

    LDRV_CFG_TakeMutex();
    LDRV_LED_Info_t led_info = led_table[led_handle]; 
    LDRV_CFG_GiveMutex();

    if(led_info.active_flag == false){
        return LDRV_STATUS_ERROR;
    }

    if(led_info.led_type == LDRV_LED_TYPE_SINGLE_PWM){
        if(LDRV_CFG_STATUS_OK != LDRV_CFG_GetLedSinglePwmFreq(pFreq_hz, &led_info.config.single_pwm_config)){
            return LDRV_STATUS_ERROR;
        }
    }
    else if(led_info.led_type == LDRV_LED_TYPE_RGB){
        if(LDRV_CFG_STATUS_OK != LDRV_CFG_GetLedRgbFreq(pFreq_hz, &led_info.config.rgb_config)){
            return LDRV_STATUS_ERROR;
        }
    }
    else{
        return LDRV_STATUS_ERROR;
    }

    return LDRV_STATUS_OK;
}

/******************************************************************************
*   Interrupts
*******************************************************************************/

