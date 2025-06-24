#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include <include/driver1306.h>
#include <include/gpio.h>

D1306_t * gDisplay;
bool gStateButtonA;
bool gStateButtonB;
int snake = 10;

void TASK_Display() 
{
  D1306_CONFIG_t cfg = {
    .external_vcc = false,
    .width = 128,
    .height = 64,
    .i2c_cfg.address = 0x3C,
    .i2c_cfg.frequency = 400*1000,
    .i2c_cfg.i2c_id = 1,
    .i2c_cfg.pin_sda = 14,
    .i2c_cfg.pin_sdl = 15
  };

  gDisplay = D1306_Init( cfg );

  D1306_Clear( gDisplay );

  while (true) 
  {
    D1306_DrawPixel( gDisplay , snake , 10 );
    D1306_Show( gDisplay );
    vTaskDelay(100);
    D1306_Clear( gDisplay );
  }
}

void TASK_Buttons()
{
  GPIO_CONFIG_t cfg_button_a = {
    .pin = 5,
    .direction = 0,
    .logic = 1,
    .mode = 1
  };

  GPIO_CONFIG_t cfg_button_b = {
    .pin = 6,
    .direction = 0,
    .logic = 1,
    .mode = 1
  };

  GPIO_t* button_a = GPIO_Init( cfg_button_a );
  GPIO_t* button_b = GPIO_Init( cfg_button_b );

  while(1)
  {
    gStateButtonA = !GPIO_GetInput( button_a );
    gStateButtonB = !GPIO_GetInput( button_b );
    if( gStateButtonA )
    {
      snake++;
    }else if( gStateButtonB )
    {
      snake--;
    }
    vTaskDelay(100);
  }
}



int main() 
{
  stdio_init_all();

  xTaskCreate( TASK_Display, "TASK_Display", 256, NULL, 1, NULL);

  xTaskCreate( TASK_Buttons, "TASK_Buttons", 256, NULL, 1, NULL);

  vTaskStartScheduler();

  while(1);
}