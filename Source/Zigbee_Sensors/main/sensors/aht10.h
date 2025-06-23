#ifndef _AHT10_H
#define _AHT10_H

#include <stdint.h>

/******************************************************************************
*   Public Definitions
*******************************************************************************/
#define AHT10_INVALID_TEMPERATURE               (0x8000)
#define AHT10_INVALID_HUMIDITY                  (0xFFFF)

/******************************************************************************
*   Public Macros
*******************************************************************************/


/******************************************************************************
*   Public Data Types
*******************************************************************************/
typedef void(*WaitMsFunction_t)(uint32_t wait_ms);

typedef enum AHT10_Ret_e{
    AHT10_STATUS_ERROR,
    AHT10_STATUS_OK,
}AHT10_Ret_t;

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
*  \brief AHT10 initialization.
*
*   Initialize the AHT10 interface and peripheral. A wait function must 
*   be passed as parameter for the AHT10 to measure properly.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  scl_gpio            I2C SCL gpio.
*   \param[in]  sda_gpio            I2C SDA gpio.
*   \param[in]  wait_function       Wait function.
*
*   \return     Operation status
*
*******************************************************************************/
AHT10_Ret_t AHT10_Init(uint8_t scl_gpio, 
                       uint8_t sda_gpio, 
                       WaitMsFunction_t wait_function);

/***************************************************************************//*!
*  \brief Start AHT10 measurement.
*
*   Start measurement process and read back the temperature/humidity values
*   from the AHT10 sensor.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \return     Operation status
*
*******************************************************************************/
AHT10_Ret_t AHT10_StartMeasurement(void);

/***************************************************************************//*!
*  \brief Get the last temperature measurement.
*
*   Return the last valid temperature measurement result for the AHT10.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  pTemperature         Pointer to store the temperature value.
*
*   \return     Operation status
*
*******************************************************************************/
AHT10_Ret_t AHT10_GetLastTemperature(int16_t *pTemperature);

/***************************************************************************//*!
*  \brief Get the last humidity measurement.
*
*   Return the last valid humidity measurement result for the AHT10.
*   
*   Preconditions: None.
*
*   Side Effects: None.
*
*   \param[in]  pTemperature         Pointer to store the humidity value.
*
*   \return     Operation status
*
*******************************************************************************/
AHT10_Ret_t AHT10_GetLastHumidity(uint16_t *pHumidity);


#endif//_AHT10_H