#ifndef _HAL_I2C_H
#define _HAL_I2C_H

#include <stdint.h>
#include "pico/stdlib.h"
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <hardware/i2c.h>

typedef struct
{
    uint8_t address;
    i2c_inst_t* i2c_hw;
}I2C_t;

typedef struct
{
    uint8_t i2c_id;
    uint8_t address;
    uint32_t frequency;
    uint8_t pin_sda;
    uint8_t pin_sdl;
}I2C_CONFIG_t;

I2C_t* I2C_Init( I2C_CONFIG_t );

size_t I2C_WriteByte( I2C_t* , char );

size_t I2C_ReadByte( I2C_t* , char* );

size_t I2C_WriteByteArray( I2C_t* , char* , size_t );

size_t I2C_ReadByteArray( I2C_t* , char* , size_t );

#endif