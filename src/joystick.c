#include <include/joystick.h>

JOYSTICK_t* JOYSTICK_Init( JOYSTICK_CONFIG_t cfg )
{
    JOYSTICK_t* joystick;

    joystick = (JOYSTICK_t*)malloc( sizeof ( JOYSTICK_t ) );
    
    assert( joystick != NULL );

    joystick->adc_x = ADC_Init( cfg.x );
    joystick->adc_y = ADC_Init( cfg.y );
    joystick->gpio_z = GPIO_Init( cfg.z );

    return joystick;
}

void JOYSTICK_Update( JOYSTICK_t* joystick )
{
    joystick->pos_x = ADC_ReadValue( joystick->adc_x );
    joystick->pos_y = ADC_ReadValue( joystick->adc_y );
    joystick->state_z = GPIO_GetInput( joystick->gpio_z );
}

uint8_t JOYSTICK_GetX( JOYSTICK_t* joystick )
{
    return joystick->pos_x;
}

uint8_t JOYSTICK_GetY( JOYSTICK_t* joystick )
{
    return joystick->pos_y;
}

uint8_t JOYSTICK_GetZ( JOYSTICK_t* joystick )
{
    return joystick->state_z;
}