#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include <stdlib.h>
#include <string.h>
#include <include/driver1306.h>
#include <include/gpio.h>
#include <include/joystick.h>

/****************************
* DEFINES
****************************/
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define PLAYER_WIDTH 4
#define PLAYER_HEIGHT 4
#define PLAYER_GRAVITY 1

#define PLAYER_SPEED_DEFAULT PLAYER_WIDTH
#define PLAYER_SPEED_MIN 1
#define PLAYER_SPEED_MAX 8

#define MOBILE_PLATFORM_WIDTH 20
#define MOBILE_PLATFORM_HEIGHT 2 // Espessura das plataformas móveis

#define STATIC_PLATFORM_DEFAULT_WIDTH 25
#define STATIC_PLATFORM_HEIGHT 4 // Altura das plataformas fixas

#define NUM_PLATFORMS 10
#define NUM_MOVING_PLATFORMS 8

#define MIN_MOVING_PLATFORM_INTERVAL 1 // Plataforma mais rápida
#define MAX_MOVING_PLATFORM_INTERVAL 5 // Plataforma mais lenta

#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6

#define HORIZONTAL_SPACING 1

#define TOP_SCREEN_BOUNDARY 0
#define BOTTOM_SCREEN_BOUNDARY (SCREEN_HEIGHT - MOBILE_PLATFORM_HEIGHT)

#define PLATFORM_RESET_OFFSET 10

#define MIN_STATIC_PLATFORM_Y (SCREEN_HEIGHT - 40)
#define MAX_STATIC_PLATFORM_Y (SCREEN_HEIGHT - STATIC_PLATFORM_HEIGHT)

#define INITIAL_PAIR_BOTTOM_Y (SCREEN_HEIGHT - MOBILE_PLATFORM_HEIGHT - 5)
#define INITIAL_PAIR_TOP_Y_OFFSET (SCREEN_HEIGHT / 2 - MOBILE_PLATFORM_HEIGHT / 2)

#define VERTICAL_COLUMN_CONSTANT_GAP (INITIAL_PAIR_BOTTOM_Y - INITIAL_PAIR_TOP_Y_OFFSET)

#define LONG_PRESS_TIME_MS 1000
#define STATE_TRANSITION_DEBOUNCE_MS 200
#define SHORT_CLICK_MAX_TIME_MS 200

/****************************
* TYPEDEF
****************************/

typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_PLAY,
    GAME_STATE_CONFIG,
    GAME_STATE_GAME_OVER,
    GAME_STATE_LEVEL_COMPLETE
} GAME_STATE_t;

typedef enum {
    DIR_UP,
    DIR_DOWN
} PLATFORM_DIRECTION_t;

typedef struct {
    int x;
    int y;
    int width;
    int height;
    bool is_moving;
    PLATFORM_DIRECTION_t direction;
    int speed_interval;
    int move_counter;
} PLATFORM_t;

/****************************
* VARIABLES
****************************/
D1306_t * gDisplay;
bool gStateButtonA;
bool gStateButtonB;

GAME_STATE_t gCurrentGameState = GAME_STATE_MENU;

int gPlayerSpeed = PLAYER_SPEED_DEFAULT;

static uint64_t b_press_start_time_us = 0;
static bool b_long_pressed_triggered = false;

static int gLastLevelFinalPlatformY = -1; // -1: Primeira partida

int gPlayerPos[2];
PLATFORM_t gPlatforms[NUM_PLATFORMS];


/****************************
* FUNÇÕES DE INICIALIZAÇÃO DO JOGO
****************************/

void InitGameElements() {
    int num_unique_moving_columns = 4;
    int total_moving_columns_width = num_unique_moving_columns * MOBILE_PLATFORM_WIDTH;
    int total_spacing_width = (num_unique_moving_columns + 1) * HORIZONTAL_SPACING;

    int remaining_width = SCREEN_WIDTH - total_moving_columns_width - total_spacing_width;
    int static_platform_width = remaining_width / 2;

    // Plataforma inicial (gPlatforms[0])
    gPlatforms[0] = (PLATFORM_t){
        .x = 0,
        .y = (gLastLevelFinalPlatformY != -1) ? gLastLevelFinalPlatformY :
             (rand() % (MAX_STATIC_PLATFORM_Y - MIN_STATIC_PLATFORM_Y + 1)) + MIN_STATIC_PLATFORM_Y,
        .width = static_platform_width,
        .height = STATIC_PLATFORM_HEIGHT,
        .is_moving = false
    };

    // Colunas de plataformas móveis (gPlatforms[1] a gPlatforms[8])
    // Coluna 1 (gPlatforms[1] e gPlatforms[2])
    gPlatforms[1] = (PLATFORM_t){
        .x = gPlatforms[0].x + gPlatforms[0].width + HORIZONTAL_SPACING,
        .y = INITIAL_PAIR_BOTTOM_Y,
        .width = MOBILE_PLATFORM_WIDTH, .height = MOBILE_PLATFORM_HEIGHT, .is_moving = true,
        .direction = (rand() % 2 == 0) ? DIR_UP : DIR_DOWN,
        .speed_interval = (rand() % (MAX_MOVING_PLATFORM_INTERVAL - MIN_MOVING_PLATFORM_INTERVAL + 1)) + MIN_MOVING_PLATFORM_INTERVAL,
        .move_counter = 0
    };
    gPlatforms[2] = (PLATFORM_t){
        .x = gPlatforms[1].x, .y = INITIAL_PAIR_TOP_Y_OFFSET,
        .width = MOBILE_PLATFORM_WIDTH, .height = MOBILE_PLATFORM_HEIGHT, .is_moving = true,
        .direction = gPlatforms[1].direction, .speed_interval = gPlatforms[1].speed_interval, .move_counter = 0
    };

    // Coluna 2 (gPlatforms[3] e gPlatforms[4])
    gPlatforms[3] = (PLATFORM_t){
        .x = gPlatforms[1].x + MOBILE_PLATFORM_WIDTH + HORIZONTAL_SPACING,
        .y = INITIAL_PAIR_BOTTOM_Y,
        .width = MOBILE_PLATFORM_WIDTH, .height = MOBILE_PLATFORM_HEIGHT, .is_moving = true,
        .direction = (rand() % 2 == 0) ? DIR_UP : DIR_DOWN,
        .speed_interval = (rand() % (MAX_MOVING_PLATFORM_INTERVAL - MIN_MOVING_PLATFORM_INTERVAL + 1)) + MIN_MOVING_PLATFORM_INTERVAL,
        .move_counter = 0
    };
    gPlatforms[4] = (PLATFORM_t){
        .x = gPlatforms[3].x, .y = INITIAL_PAIR_TOP_Y_OFFSET,
        .width = MOBILE_PLATFORM_WIDTH, .height = MOBILE_PLATFORM_HEIGHT, .is_moving = true,
        .direction = gPlatforms[3].direction, .speed_interval = gPlatforms[3].speed_interval, .move_counter = 0
    };

    // Coluna 3 (gPlatforms[5] e gPlatforms[6])
    gPlatforms[5] = (PLATFORM_t){
        .x = gPlatforms[3].x + MOBILE_PLATFORM_WIDTH + HORIZONTAL_SPACING,
        .y = INITIAL_PAIR_BOTTOM_Y,
        .width = MOBILE_PLATFORM_WIDTH, .height = MOBILE_PLATFORM_HEIGHT, .is_moving = true,
        .direction = (rand() % 2 == 0) ? DIR_UP : DIR_DOWN,
        .speed_interval = (rand() % (MAX_MOVING_PLATFORM_INTERVAL - MIN_MOVING_PLATFORM_INTERVAL + 1)) + MIN_MOVING_PLATFORM_INTERVAL,
        .move_counter = 0
    };
    gPlatforms[6] = (PLATFORM_t){
        .x = gPlatforms[5].x, .y = INITIAL_PAIR_TOP_Y_OFFSET,
        .width = MOBILE_PLATFORM_WIDTH, .height = MOBILE_PLATFORM_HEIGHT, .is_moving = true,
        .direction = gPlatforms[5].direction, .speed_interval = gPlatforms[5].speed_interval, .move_counter = 0
    };

    // Coluna 4 (gPlatforms[7] e gPlatforms[8])
    gPlatforms[7] = (PLATFORM_t){
        .x = gPlatforms[5].x + MOBILE_PLATFORM_WIDTH + HORIZONTAL_SPACING,
        .y = INITIAL_PAIR_BOTTOM_Y,
        .width = MOBILE_PLATFORM_WIDTH, .height = MOBILE_PLATFORM_HEIGHT, .is_moving = true,
        .direction = (rand() % 2 == 0) ? DIR_UP : DIR_DOWN,
        .speed_interval = (rand() % (MAX_MOVING_PLATFORM_INTERVAL - MIN_MOVING_PLATFORM_INTERVAL + 1)) + MIN_MOVING_PLATFORM_INTERVAL,
        .move_counter = 0
    };
    gPlatforms[8] = (PLATFORM_t){
        .x = gPlatforms[7].x, .y = INITIAL_PAIR_TOP_Y_OFFSET,
        .width = MOBILE_PLATFORM_WIDTH, .height = MOBILE_PLATFORM_HEIGHT, .is_moving = true,
        .direction = gPlatforms[7].direction, .speed_interval = gPlatforms[7].speed_interval, .move_counter = 0
    };

    // Plataforma final (gPlatforms[9])
    gPlatforms[9] = (PLATFORM_t){
        .x = gPlatforms[7].x + MOBILE_PLATFORM_WIDTH + HORIZONTAL_SPACING,
        .y = (rand() % (MAX_STATIC_PLATFORM_Y - MIN_STATIC_PLATFORM_Y + 1)) + MIN_STATIC_PLATFORM_Y,
        .width = static_platform_width, .height = STATIC_PLATFORM_HEIGHT, .is_moving = false
    };

    if (gPlatforms[9].x + gPlatforms[9].width < SCREEN_WIDTH) {
        gPlatforms[9].width = SCREEN_WIDTH - gPlatforms[9].x;
    }

    // Posição inicial do personagem na primeira plataforma fixa
    gPlayerPos[0] = gPlatforms[0].x + (gPlatforms[0].width / 2) - (PLAYER_WIDTH / 2);
    gPlayerPos[1] = gPlatforms[0].y - PLAYER_HEIGHT;
}

/****************************
* TASKS
****************************/

void TASK_Display() {
    D1306_CONFIG_t cfg = {
        .external_vcc = false, .width = SCREEN_WIDTH, .height = SCREEN_HEIGHT,
        .i2c_cfg.address = 0x3C, .i2c_cfg.frequency = 400 * 1000,
        .i2c_cfg.i2c_id = 1, .i2c_cfg.pin_sda = 14, .i2c_cfg.pin_sdl = 15
    };
    gDisplay = D1306_Init(cfg);
    char str_buffer[20];

    while (true) {
        D1306_Clear(gDisplay);

        if (gCurrentGameState == GAME_STATE_MENU) {
            int mountain_base_y = 40; int peak_height = 20;
            for (int x = 0; x < SCREEN_WIDTH / 2; x += 3) {
                int y_offset = rand() % (peak_height / 2) - (peak_height / 4);
                int current_height = (int)((float)peak_height * (1.0f - (float)x / (SCREEN_WIDTH / 2))) + y_offset;
                if (current_height < 1) current_height = 1;
                D1306_DrawSquare(gDisplay, x, mountain_base_y - current_height, 2, current_height);
                if (rand() % 10 < 3) D1306_DrawPixel(gDisplay, x + rand()%2, mountain_base_y - current_height - (rand()%5 + 1));
            }
            for (int x = SCREEN_WIDTH / 2; x < SCREEN_WIDTH; x += 3) {
                int y_offset = rand() % (peak_height / 2) - (peak_height / 4);
                int current_height = (int)((float)peak_height * ((float)(x - SCREEN_WIDTH / 2) / (SCREEN_WIDTH / 2))) + y_offset;
                if (current_height < 1) current_height = 1;
                D1306_DrawSquare(gDisplay, x, mountain_base_y - current_height, 2, current_height);
                if (rand() % 10 < 3) D1306_DrawPixel(gDisplay, x + rand()%2, mountain_base_y - current_height - (rand()%5 + 1));
            }

            int char_width = 5; const char* title_chaos = "CHAOS"; const char* title_climb = "CLIMB";
            D1306_DrawString(gDisplay, (SCREEN_WIDTH - (strlen(title_chaos) * char_width)) / 2, 8, 1, title_chaos);
            D1306_DrawString(gDisplay, (SCREEN_WIDTH - (strlen(title_climb) * char_width)) / 2, 16, 1, title_climb);
            D1306_DrawString(gDisplay, (SCREEN_WIDTH - (strlen("A: INICIAR") * char_width)) / 2, 45, 1, "A: INICIAR");
            D1306_DrawString(gDisplay, (SCREEN_WIDTH - (strlen("B: CONFIG") * char_width)) / 2, 55, 1, "B: CONFIG");

        } else if (gCurrentGameState == GAME_STATE_PLAY) {
            D1306_DrawSquare(gDisplay, gPlayerPos[0], gPlayerPos[1], PLAYER_WIDTH, PLAYER_HEIGHT);
            for (int i = 0; i < NUM_PLATFORMS; i++) {
                D1306_DrawSquare(gDisplay, gPlatforms[i].x, gPlatforms[i].y,
                                 gPlatforms[i].width, gPlatforms[i].height);
                if (!gPlatforms[i].is_moving) {
                    for (int y_fill = gPlatforms[i].y + gPlatforms[i].height; y_fill < SCREEN_HEIGHT; y_fill += 2) {
                        for (int x_fill = gPlatforms[i].x; x_fill < gPlatforms[i].x + gPlatforms[i].width; x_fill +=2) {
                            D1306_DrawPixel(gDisplay, x_fill, y_fill);
                        }
                    }
                }
            }
        } else if (gCurrentGameState == GAME_STATE_CONFIG) {
            int char_width = 5;
            D1306_DrawString(gDisplay, (SCREEN_WIDTH - (strlen("CONFIG") * char_width)) / 2, 10, 1, "CONFIG");
            sprintf(str_buffer, "VELOC: %d", gPlayerSpeed);
            D1306_DrawString(gDisplay, 10, 30, 1, str_buffer);
            D1306_DrawString(gDisplay, 10, 40, 1, "A: + B: -");
            D1306_DrawString(gDisplay, 10, 50, 1, "SEGURE B P/ SAIR");

            if (b_press_start_time_us != 0 && !b_long_pressed_triggered) {
                uint64_t elapsed_time_us = time_us_64() - b_press_start_time_us;
                int progress_width = (int)((float)elapsed_time_us / (LONG_PRESS_TIME_MS * 1000.0f) * SCREEN_WIDTH);
                if (progress_width > SCREEN_WIDTH) progress_width = SCREEN_WIDTH;
                D1306_DrawSquare(gDisplay, 0, 60, progress_width, 2);
            }
        } else if (gCurrentGameState == GAME_STATE_GAME_OVER) {
            int gameover_char_width = 5; int gameover_scale = 2;
            const char* game_over_text = "GAME OVER";
            int actual_text_width_pixels = strlen(game_over_text) * gameover_char_width * gameover_scale;
            int actual_text_height_pixels = 8 * gameover_scale;
            D1306_DrawString(gDisplay, (SCREEN_WIDTH - actual_text_width_pixels) / 2,
                             (SCREEN_HEIGHT - actual_text_height_pixels) / 2, gameover_scale, game_over_text);
        } else if (gCurrentGameState == GAME_STATE_LEVEL_COMPLETE) {
            int char_width = 5; int scale = 2;
            const char* msg1 = "NIVEL";
            const char* msg2 = "COMPLETO!";
            int actual_text_width_pixels1 = strlen(msg1) * char_width * scale;
            int actual_text_width_pixels2 = strlen(msg2) * char_width * scale;
            int actual_text_height_pixels = 8 * scale; // Altura de uma linha de texto na escala 2

            // Centraliza "NIVEL COMPLETO!" em duas linhas
            D1306_DrawString(gDisplay, (SCREEN_WIDTH - actual_text_width_pixels1) / 2,
                             (SCREEN_HEIGHT / 2) - actual_text_height_pixels + 5, scale, msg1); // Ajuste Y
            D1306_DrawString(gDisplay, (SCREEN_WIDTH - actual_text_width_pixels2) / 2,
                             (SCREEN_HEIGHT / 2) + 5, scale, msg2); // Ajuste Y
        }
        D1306_Show(gDisplay);
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

void TASK_ButtonControl() {
    GPIO_CONFIG_t cfg_button_a = { .pin = BUTTON_A_PIN, .direction = 0, .logic = 1, .mode = 1 };
    GPIO_CONFIG_t cfg_button_b = { .pin = BUTTON_B_PIN, .direction = 0, .logic = 1, .mode = 1 };
    GPIO_t* button_a = GPIO_Init(cfg_button_a);
    GPIO_t* button_b = GPIO_Init(cfg_button_b);
    static bool lastStateButtonA = false; static bool lastStateButtonB = false;
    bool currentStateA; bool currentStateB; static uint64_t b_click_start_time_us = 0;

    while (true) {
        currentStateA = !GPIO_GetInput(button_a);
        currentStateB = !GPIO_GetInput(button_b);

        if (gCurrentGameState == GAME_STATE_MENU) {
            if (currentStateA && !lastStateButtonA) {
                gCurrentGameState = GAME_STATE_PLAY;
                gLastLevelFinalPlatformY = -1; // Reset para início de nova partida
                InitGameElements();
                vTaskDelay(pdMS_TO_TICKS(STATE_TRANSITION_DEBOUNCE_MS));
                currentStateA = !GPIO_GetInput(button_a); currentStateB = !GPIO_GetInput(button_b);
            } else if (currentStateB && !lastStateButtonB) {
                gCurrentGameState = GAME_STATE_CONFIG;
                vTaskDelay(pdMS_TO_TICKS(STATE_TRANSITION_DEBOUNCE_MS));
                currentStateA = !GPIO_GetInput(button_a); currentStateB = !GPIO_GetInput(button_b);
                b_press_start_time_us = 0; b_long_pressed_triggered = false; b_click_start_time_us = 0;
            }
        } else if (gCurrentGameState == GAME_STATE_PLAY) {
            if (currentStateA) { gPlayerPos[0] -= gPlayerSpeed; if (gPlayerPos[0] < 0) gPlayerPos[0] = 0; }
            if (currentStateB) { gPlayerPos[0] += gPlayerSpeed; if (gPlayerPos[0] >= SCREEN_WIDTH - PLAYER_WIDTH) gPlayerPos[0] = SCREEN_WIDTH - PLAYER_WIDTH - 1; }
        } else if (gCurrentGameState == GAME_STATE_CONFIG) {
            if (currentStateA && !lastStateButtonA) { if (gPlayerSpeed < PLAYER_SPEED_MAX) gPlayerSpeed++; }
            if (currentStateB && !lastStateButtonB) {
                b_press_start_time_us = time_us_64(); b_click_start_time_us = time_us_64(); b_long_pressed_triggered = false;
            } else if (currentStateB && lastStateButtonB) {
                if (!b_long_pressed_triggered) {
                    uint64_t elapsed_time_us = time_us_64() - b_press_start_time_us;
                    if (elapsed_time_us >= (uint64_t)LONG_PRESS_TIME_MS * 1000) {
                        gCurrentGameState = GAME_STATE_MENU; b_long_pressed_triggered = true; b_press_start_time_us = 0;
                        vTaskDelay(pdMS_TO_TICKS(STATE_TRANSITION_DEBOUNCE_MS));
                        currentStateA = !GPIO_GetInput(button_a); currentStateB = !GPIO_GetInput(button_b);
                    }
                }
            } else if (!currentStateB && lastStateButtonB) {
                uint64_t click_duration_us = time_us_64() - b_click_start_time_us;
                if (!b_long_pressed_triggered && click_duration_us < (uint64_t)SHORT_CLICK_MAX_TIME_MS * 1000 && b_click_start_time_us != 0) {
                    if (gPlayerSpeed > PLAYER_SPEED_MIN) gPlayerSpeed--;
                }
                b_press_start_time_us = 0; b_long_pressed_triggered = false; b_click_start_time_us = 0;
            }
        } else if (gCurrentGameState == GAME_STATE_GAME_OVER) {
            if ((currentStateA && !lastStateButtonA) || (currentStateB && !lastStateButtonB)) {
                gCurrentGameState = GAME_STATE_MENU;
                gLastLevelFinalPlatformY = -1; // Reset para início de nova partida
                vTaskDelay(pdMS_TO_TICKS(STATE_TRANSITION_DEBOUNCE_MS));
                currentStateA = !GPIO_GetInput(button_a); currentStateB = !GPIO_GetInput(button_b);
            }
        } else if (gCurrentGameState == GAME_STATE_LEVEL_COMPLETE) {
            if ((currentStateA && !lastStateButtonA) || (currentStateB && !lastStateButtonB)) {
                gCurrentGameState = GAME_STATE_PLAY;
                gLastLevelFinalPlatformY = gPlatforms[9].y; // Salva a altura da plataforma final
                InitGameElements(); // Inicia o próximo nível
                vTaskDelay(pdMS_TO_TICKS(STATE_TRANSITION_DEBOUNCE_MS));
                currentStateA = !GPIO_GetInput(button_a); currentStateB = !GPIO_GetInput(button_b);
            }
        }
        lastStateButtonA = currentStateA; lastStateButtonB = currentStateB;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void TASK_PlatformMovement() {
    while(true) {
        if (gCurrentGameState == GAME_STATE_PLAY) {
            for (int i = 0; i < NUM_PLATFORMS; i++) {
                if (gPlatforms[i].is_moving) {
                    gPlatforms[i].move_counter++;
                    if (gPlatforms[i].move_counter >= gPlatforms[i].speed_interval) {
                        if (gPlatforms[i].direction == DIR_UP) {
                            gPlatforms[i].y--;
                            if (gPlatforms[i].y + gPlatforms[i].height < TOP_SCREEN_BOUNDARY) {
                                gPlatforms[i].y = SCREEN_HEIGHT + PLATFORM_RESET_OFFSET + (rand() % 5);
                            }
                        } else {
                            gPlatforms[i].y++;
                            if (gPlatforms[i].y > SCREEN_HEIGHT) {
                                gPlatforms[i].y = TOP_SCREEN_BOUNDARY - MOBILE_PLATFORM_HEIGHT - PLATFORM_RESET_OFFSET - (rand() % 5);
                            }
                        }
                        gPlatforms[i].move_counter = 0;
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void TASK_GameLogic() {
    while(true) {
        if (gCurrentGameState == GAME_STATE_PLAY) {
            bool on_platform = false;
            for (int i = 0; i < NUM_PLATFORMS; i++) {
                PLATFORM_t *p = &gPlatforms[i];
                if (gPlayerPos[0] < p->x + p->width && gPlayerPos[0] + PLAYER_WIDTH > p->x &&
                    gPlayerPos[1] + PLAYER_HEIGHT >= p->y && gPlayerPos[1] + PLAYER_HEIGHT <= p->y + p->height &&
                    gPlayerPos[1] + PLAYER_HEIGHT + PLAYER_GRAVITY > p->y) {
                    gPlayerPos[1] = p->y - p->height; on_platform = true;
                    if (p->is_moving) {
                        if (p->move_counter == (p->speed_interval -1)) {
                            if (p->direction == DIR_UP) { gPlayerPos[1]--; } else { gPlayerPos[1]++; }
                        }
                    }
                    // Detecção de nível completo: Chegou na plataforma final
                    if (i == 9) {
                        printf("NIVEL COMPLETO!\n");
                        gCurrentGameState = GAME_STATE_LEVEL_COMPLETE;
                        break;
                    }
                    break;
                }
            }
            if (!on_platform) { gPlayerPos[1] += PLAYER_GRAVITY; }
            if (gPlayerPos[1] >= SCREEN_HEIGHT - PLAYER_HEIGHT && !on_platform) {
                printf("!!! GAME OVER !!!\n");
                gCurrentGameState = GAME_STATE_GAME_OVER;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/****************************
* MAIN
****************************/
int main() {
    stdio_init_all(); srand(time_us_64());
    InitGameElements();
    xTaskCreate(TASK_Display, "Display", 256, NULL, 1, NULL);
    xTaskCreate(TASK_ButtonControl, "ButtonControl", 256, NULL, 1, NULL);
    xTaskCreate(TASK_PlatformMovement, "PlatformMovement", 256, NULL, 1, NULL);
    xTaskCreate(TASK_GameLogic, "GameLogic", 256, NULL, 1, NULL);
    vTaskStartScheduler();
    while (1);
}