#include "i2c.h"

I2C_t* I2C_Init( I2C_CONFIG_t cfg )
{
    I2C_t* i2c;

    i2c = (I2C_t*)malloc( sizeof ( I2C_t ) );
    
    assert( i2c != NULL );

    i2c->address = cfg.address;
    if( cfg.i2c_id ){ i2c->i2c_hw = i2c1; } else { i2c->i2c_hw = i2c0; }

    i2c_init( i2c->i2c_hw , cfg.frequency );
    gpio_set_function( cfg.pin_sda , GPIO_FUNC_I2C );
    gpio_set_function( cfg.pin_sdl , GPIO_FUNC_I2C );
    gpio_pull_up( cfg.pin_sda );
    gpio_pull_up( cfg.pin_sdl );

    return i2c;
}

size_t I2C_WriteByte( I2C_t* i2c , char buffer )
{
    size_t success;
    success = i2c_write_blocking( i2c->i2c_hw , i2c->address , &buffer , 1 , false );
    if( success == 1 ) { return true; } else { return false; }
}

size_t I2C_ReadByte( I2C_t* i2c , char* buffer )
{
    size_t success;
    success = i2c_read_blocking( i2c->i2c_hw , i2c->address , buffer , 1 , true );
    if( success == 1 ) { return true; } else { return false; }
}

size_t I2C_WriteByteArray( I2C_t* i2c , char* buffer , size_t length )
{
    size_t success;
    success = i2c_write_blocking( i2c->i2c_hw , i2c->address , buffer , length , true );
    if( success == length ) { return true; } else { return false; }
}

size_t I2C_ReadByteArray( I2C_t* i2c , char* buffer , size_t length)
{
    size_t success;
    success = i2c_read_blocking( i2c->i2c_hw , i2c->address , buffer , length , false );
    if( success == length ) { return true; } else { return false; }
}