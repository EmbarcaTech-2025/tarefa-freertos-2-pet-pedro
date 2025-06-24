#include <include/adc.h>

ADC_t* ADC_Init( ADC_CONFIG_t cfg )
{
    ADC_t* adc;

    adc = (ADC_t*)malloc( sizeof ( ADC_t ) );
    
    assert( adc != NULL );
    assert( cfg.pin >= 26 && cfg.pin <= 29 ); // RP2040 usa 0 para 26,1 para 27, 2 para 28 e 3 para 29 

    adc->channel = cfg.pin - 26;

    adc_init();
    adc_gpio_init( cfg.pin );

    return adc;
}

uint16_t ADC_ReadValue( ADC_t* adc )
{
    adc_select_input( adc->channel );
    return adc_read();
}
