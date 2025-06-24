#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include <include/driver1306.h>
#include <include/gpio.h>
#include <include/joystick.h>

/****************************
*         DEFINES
****************************/
#define JOYSTICK_THRESHOLD 20

/****************************
*         TYPEDEF
****************************/
typedef union
{
  int x; /* -1, 0 or 1 */
  int y; /* -1, 0 or 1 */
}JOYSTICK_POS_t;

/****************************
*         VARIABLES
****************************/
D1306_t * gDisplay;
bool gStateButtonA;
bool gStateButtonB;
JOYSTICK_POS_t gStateJoystick;
int snake[2] = {10,10};

/****************************
*         TASKS
****************************/
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
    D1306_DrawPixel( gDisplay , snake[0] , snake[1] );
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

  while(true)
  {
    gStateButtonA = !GPIO_GetInput( button_a );
    gStateButtonB = !GPIO_GetInput( button_b );
    vTaskDelay(100);
  }
}

void TASK_Joystick()
{

  JOYSTICK_CONFIG_t cfg_joystick = {
    .x.pin = 27,
    .y.pin = 26,
    .z.pin = 22,
    .z.direction = 0,
    .z.logic = 1,
    .z.mode = 1
  };

  JOYSTICK_t* joystick = JOYSTICK_Init( cfg_joystick );

  while(1)
  {
    JOYSTICK_Update( joystick );
    if( joystick->pos_x > 255 - JOYSTICK_THRESHOLD )
    {
      gStateJoystick.x = 1;
    }
    else if( joystick->pos_x < JOYSTICK_THRESHOLD )
    {
      gStateJoystick.x = -1;
    }
    else
    {
      gStateJoystick.x = 0;
    }

    if( joystick->pos_y > 255 - JOYSTICK_THRESHOLD )
    {
      gStateJoystick.y = 1;
    }
    else if( joystick->pos_y < JOYSTICK_THRESHOLD )
    {
      gStateJoystick.y = -1;
    }
    else
    {
      gStateJoystick.y = 0;
    }
    printf("x:%d,y:%d\n" , joystick->pos_x,joystick->pos_y );
    printf("x:%d,y:%d\n" , gStateJoystick.x , gStateJoystick.y );

    vTaskDelay(100);
  }
}

void TASK_Game()
{
  while(true)
  {
    snake[0] += gStateJoystick.x;
    snake[1] += gStateJoystick.y;
    vTaskDelay(100);
  }
}

/****************************
*         MAIN
****************************/
int main() 
{
  stdio_init_all();

  xTaskCreate( TASK_Display, "TASK_Display", 256, NULL, 1, NULL);

  xTaskCreate( TASK_Buttons, "TASK_Buttons", 256, NULL, 1, NULL);

  xTaskCreate( TASK_Joystick, "TASK_Joystick", 256, NULL, 1, NULL);

  xTaskCreate( TASK_Game, "TASK_Game", 256, NULL, 1, NULL);

  vTaskStartScheduler();

  while(1);
}