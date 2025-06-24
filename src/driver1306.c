#include "driver1306.h"
#include <string.h>
#include <stdio.h>

extern const uint8_t font_8x5[];

inline static void D1306_Write( D1306_t* D1306 , uint8_t val )
{
    uint8_t buffer[2] = { 0x00 , val };
    I2C_WriteByteArray( D1306->i2c , buffer , 2 );
}

D1306_t * D1306_Init( D1306_CONFIG_t cfg )
{
    D1306_t* D1306;

    D1306 = (D1306_t*)malloc( sizeof ( D1306_t ) );
    
    assert( D1306 != NULL );
    sleep_ms(1000);
    D1306->i2c = I2C_Init( cfg.i2c_cfg );

    D1306->width = cfg.width;
    D1306->height = cfg.height;
    D1306->pages = cfg.height/8;

    D1306->bufsize = (D1306->pages)*(D1306->width);
    D1306->buffer = malloc( D1306->bufsize );
    D1306->font = font_8x5;

    assert( D1306->buffer != NULL );

    ++(D1306->buffer);

    uint8_t cmds[]= {
        SET_DISP,
        // timing and driving scheme
        SET_DISP_CLK_DIV,
        0x80,
        SET_MUX_RATIO,
        D1306->height - 1,
        SET_DISP_OFFSET,
        0x00,
        // resolution and layout
        SET_DISP_START_LINE,
        // charge pump
        SET_CHARGE_PUMP,
        cfg.external_vcc ? 0x10 : 0x14,
        SET_SEG_REMAP | 0x01,           // column addr 127 mapped to SEG0
        SET_COM_OUT_DIR | 0x08,         // scan from COM[N] to COM0
        SET_COM_PIN_CFG,
        D1306->width >2*D1306->height ? 0x02 : 0x12,
        // display
        SET_CONTRAST,
        0xff,
        SET_PRECHARGE,
        cfg.external_vcc ? 0x22 : 0xF1,
        SET_VCOM_DESEL,
        0x30,                           // or 0x40?
        SET_ENTIRE_ON,                  // output follows RAM contents
        SET_NORM_INV,                   // not inverted
        SET_DISP | 0x01,
        // address setting
        SET_MEM_ADDR,
        0x00,  // horizontal
    };

    for( size_t i = 0 ; i < sizeof(cmds) ; ++i )
    {
        D1306_Write( D1306 , cmds[i] );
    }
    printf("Entrei");

    return D1306;
}

void D1306_Invert( D1306_t* D1306 , uint8_t inv )
{
    D1306_Write( D1306 , SET_NORM_INV | (inv & 1));
}

void D1306_Show( D1306_t* D1306 )
{
    uint8_t payload[] = { SET_COL_ADDR , 0 , D1306->width - 1 , SET_PAGE_ADDR , 0 , D1306->pages - 1 };
    
    if( D1306->width == 64 ) 
    {
        payload[1] += 32;
        payload[2] += 32;
    }

    for( size_t i = 0 ; i < sizeof(payload) ; ++i )
    {
        D1306_Write( D1306 , payload[i]);
    }

    *( D1306->buffer - 1 ) = 0x40;

    I2C_WriteByteArray( D1306->i2c , D1306->buffer - 1 , D1306->bufsize + 1);
}

void D1306_Clear( D1306_t* D1306 )
{
    memset( D1306->buffer , 0 , D1306->bufsize );
}

void D1306_DrawPixel( D1306_t* D1306 , uint32_t x, uint32_t y )
{
    if( x >= D1306->width || y >= D1306->height ) return;
    D1306->buffer[x + D1306->width * ( y >> 3 ) ] |= (0x1 << ( y & 0x7));
}

void D1306_DrawChar( D1306_t* D1306 , uint32_t x, uint32_t y, uint32_t scale , char c ) 
{
    if( c < D1306->font[3] || c > D1306->font[4] )
        return;

    uint32_t parts_per_line = ( D1306->font[0] >> 3 ) + (( D1306->font[0] & 7 ) > 0 );
    for( uint8_t w = 0; w < D1306->font[1]; ++w ) 
    {
        uint32_t pp = ( c - D1306->font[3] ) * D1306->font[1] * parts_per_line + w * parts_per_line + 5;
        for( uint32_t lp = 0; lp < parts_per_line ; ++lp )
        {
            uint8_t line = D1306->font[pp];

            for( int8_t j = 0 ; j < 8 ; ++j , line >>= 1 ) 
            {
                if(line & 1)
                    D1306_DrawSquare( D1306 , x + w * scale , y + ((lp << 3) + j) * scale , scale , scale );
            }

            ++pp;
        }
    }
}

void D1306_DrawString( D1306_t* D1306 , uint32_t x, uint32_t y, uint32_t scale, const char *s) 
{
    for( int32_t x_n = x ; *s ; x_n += ( D1306->font[1] + D1306->font[2] ) * scale ) 
    {
        D1306_DrawChar( D1306 , x_n , y , scale , *(s++) );
    }
}

void D1306_DrawSquare( D1306_t* D1306 , uint32_t x, uint32_t y, uint32_t width, uint32_t height) 
{
    for( uint32_t i = 0; i < width ; ++i )
        for( uint32_t j = 0 ; j < height ; ++j )
            D1306_DrawPixel( D1306 , x + i, y + j );
}