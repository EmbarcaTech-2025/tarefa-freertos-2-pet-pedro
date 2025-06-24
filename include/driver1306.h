#ifndef _DRIVER1306_H
#define _DRIVER1306_H

#include <pico/stdlib.h>
#include "i2c.h"

//COMMANDS 1306
typedef enum {
    SET_CONTRAST = 0x81,
    SET_ENTIRE_ON = 0xA4,
    SET_NORM_INV = 0xA6,
    SET_DISP = 0xAE,
    SET_MEM_ADDR = 0x20,
    SET_COL_ADDR = 0x21,
    SET_PAGE_ADDR = 0x22,
    SET_DISP_START_LINE = 0x40,
    SET_SEG_REMAP = 0xA0,
    SET_MUX_RATIO = 0xA8,
    SET_COM_OUT_DIR = 0xC0,
    SET_DISP_OFFSET = 0xD3,
    SET_COM_PIN_CFG = 0xDA,
    SET_DISP_CLK_DIV = 0xD5,
    SET_PRECHARGE = 0xD9,
    SET_VCOM_DESEL = 0xDB,
    SET_CHARGE_PUMP = 0x8D
}COMMANDS_t;

typedef struct {
    I2C_t* i2c;
    uint8_t width;
    uint8_t height;
    uint8_t pages;
    uint8_t *buffer;
    size_t bufsize;
    const uint8_t* font;
}D1306_t;

typedef struct {
    uint8_t width;
    uint8_t height;
    I2C_CONFIG_t i2c_cfg;
    bool external_vcc;
}D1306_CONFIG_t;

D1306_t * D1306_Init( D1306_CONFIG_t );

void D1306_Invert( D1306_t * , uint8_t inv);

void D1306_Show( D1306_t * );

void D1306_Clear( D1306_t * );

void D1306_DrawPixel( D1306_t* , uint32_t, uint32_t );

void D1306_DrawChar( D1306_t* , uint32_t , uint32_t , uint32_t , char );

void D1306_DrawString( D1306_t* , uint32_t , uint32_t , uint32_t , const char *);

void D1306_DrawSquare( D1306_t* , uint32_t , uint32_t , uint32_t , uint32_t );

#endif