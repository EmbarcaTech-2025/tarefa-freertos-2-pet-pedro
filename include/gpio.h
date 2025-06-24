#ifndef _HAL_GPIO_H
#define _HAL_GPIO_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <hardware/gpio.h>

typedef struct
{
    int pin;
    int logic;
}GPIO_t;

typedef struct
{
    int pin;
    int mode;
    int direction;
    int logic;
}GPIO_CONFIG_t;

GPIO_t* GPIO_Init( GPIO_CONFIG_t );

void GPIO_SetOutput( GPIO_t* , bool );

void GPIO_ToogleOutput( GPIO_t* );

bool GPIO_GetInput( GPIO_t* );

#endif