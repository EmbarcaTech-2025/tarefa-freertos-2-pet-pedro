#ifndef _HAL_JOYSTICK_H
#define _HAL_JOYSTICK_H

#include "adc.h"
#include "gpio.h"

typedef struct
{
    ADC_t* adc_x;
    ADC_t* adc_y;
    GPIO_t* gpio_z;
    uint8_t pos_x;
    uint8_t pos_y;
    bool state_z;
}JOYSTICK_t;

typedef struct
{
    ADC_CONFIG_t x;
    ADC_CONFIG_t y;
    GPIO_CONFIG_t z;
}JOYSTICK_CONFIG_t;

JOYSTICK_t* JOYSTICK_Init( JOYSTICK_CONFIG_t );

void JOYSTICK_Update( JOYSTICK_t* );

uint8_t JOYSTICK_GetX( JOYSTICK_t* );

uint8_t JOYSTICK_GetY( JOYSTICK_t* );

uint8_t JOYSTICK_GetZ( JOYSTICK_t* );

#endif