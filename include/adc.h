#ifndef _HAL_ADC_H
#define _HAL_ADC_H

#include <stdint.h>
#include "pico/stdlib.h"
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <hardware/adc.h>

typedef struct
{
    uint8_t channel;
}ADC_t;

typedef struct
{
    uint8_t pin;
}ADC_CONFIG_t;

ADC_t* ADC_Init( ADC_CONFIG_t );

uint16_t ADC_ReadValue( ADC_t* );

#endif