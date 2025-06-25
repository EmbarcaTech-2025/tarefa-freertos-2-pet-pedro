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
#define JOYSTICK_THRESHOLD 30

/****************************
*         TYPEDEF
****************************/
typedef struct
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
int spaceship_x = 0;
int spaceship_y = 50;
extern int map[][2];
extern int contour[][2];
extern int spaceship[][2];

/****************************
*         FUNCTIONS
****************************/
void _DRAW_Map()
{
  for(int i = 0; i < 3178; i++ )
  {
      D1306_DrawPixel( gDisplay , map[i][0] , map[i][1] );
  }
}

void _DRAW_SpaceShip()
{
  for(int i = 0; i < 43; i++ )
  {
      D1306_DrawPixel( gDisplay , spaceship[i][0] + spaceship_x , spaceship[i][1] + spaceship_y );
  }
}

void _CLEAR_SpaceShip()
{
  for(int i = 0; i < 43; i++ )
  {
      D1306_ClearPixel( gDisplay , spaceship[i][0] + spaceship_x , spaceship[i][1] + spaceship_y );
  }
}

bool _CHECK_Colision()
{
  for(int i = 0; i < 43; i++ )
  {
    for(int j = 0; i < 138; i++ )
    {
      if( spaceship[i][0] == contour[j][0] && spaceship[i][1] == contour[j][1])
      {
        return true;
      }
    }
  }
  return false;
}

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
    _DRAW_Map();
    _DRAW_SpaceShip();
    if( _CHECK_Colision() )
    {
        //DO SOMETHING
    }
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
    if( joystick->pos_x > (255 - JOYSTICK_THRESHOLD) )
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

    if( joystick->pos_y > (255 - JOYSTICK_THRESHOLD) )
    {
      gStateJoystick.y = -1;
    }
    else if( joystick->pos_y < JOYSTICK_THRESHOLD )
    {
      gStateJoystick.y = 1;
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
    spaceship_x += gStateJoystick.x;
    spaceship_y += gStateJoystick.y;
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