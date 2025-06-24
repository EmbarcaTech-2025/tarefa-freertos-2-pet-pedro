#include "gpio.h"

GPIO_t* GPIO_Init( GPIO_CONFIG_t cfg )
{
    GPIO_t* gpio;

    gpio = (GPIO_t*)malloc( sizeof ( GPIO_t ) );
    
    assert( gpio != NULL );
    
    gpio->pin = cfg.pin;

    gpio->logic = cfg.logic;

    gpio_init( gpio->pin );

    gpio_set_dir( gpio->pin , cfg.direction );
    
    if( cfg.mode )
    {
        gpio_pull_up( gpio->pin );
    }else
    {
        gpio_pull_down( gpio->pin );
    }

    return gpio;
}

void GPIO_SetOutput( GPIO_t* gpio , bool value )
{
    if( gpio->logic )
    {
        gpio_put( gpio->pin , value );
    }else
    {
        gpio_put( gpio->pin , !value );
    }
}

void GPIO_ToogleOutput( GPIO_t* gpio )
{
    uint state = gpio_get( gpio->pin );
    gpio_put( gpio->pin , !state );
}

bool GPIO_GetInput( GPIO_t* gpio )
{
    if( gpio->logic )
    {
        return gpio_get( gpio->pin );
    }else
    {
        return !gpio_get( gpio->pin );
    }
}