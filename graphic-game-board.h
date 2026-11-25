#pragma once
#ifndef GRAPHIC_GAME_BOARD_H
#define GRAPHIC_GAME_BOARD_H

// --- Includes ---

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "pico/stdlib.h"

#if PICO_RP2350
#include "third_party/picocalc-shell/picocalc_shell.h"
#endif

#include "drivers/display.h"
#include "drivers/font.h"
#include "drivers/keyboard.h"
#include "drivers/lcd.h"
#include "drivers/onboard_led.h"
#include "drivers/picocalc.h"
#include "drivers/southbridge.h"

// --- Constants / Macros ---

// Board layout constants (8x8 grid of 36x36 px cells = 288x288 px)
// The board is square so we don't have to store separate x and y
// constants.
#define BOARD_SIZE 8 // 8x8 board = 64 squares
#define CELL_SIZE 36
#define SCREEN_PX 320
#define BOARD_PX (BOARD_SIZE * CELL_SIZE)      // 288x288 pixel board
#define OFFSET_PX ((SCREEN_PX - BOARD_PX) / 2) // 16px margin left, right, top, and bottom
#define SPRITE_SIZE 34

// RGB565 Color Helper
#define RGB565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

#define COLOR_BG RGB565(20, 24, 28)
#define COLOR_BOARD RGB565(2, 128, 2)
#define COLOR_GRID RGB565(2, 64, 64)
#define COLOR_BLACK_DISC RGB565(10, 10, 10)
#define COLOR_WHITE_DISC RGB565(224, 224, 224)
#define COLOR_SELECTION_BOX RGB565(255, 215, 0)

// --- typedefs and declarations ---

extern const uint16_t white_disc[SPRITE_SIZE * SPRITE_SIZE];
extern const uint16_t black_disc[SPRITE_SIZE * SPRITE_SIZE];
// grid square states
typedef enum
{
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
} GridSquareState;

extern int selector_r;
extern int selector_c;

extern uint8_t board[BOARD_SIZE][BOARD_SIZE];

// --- Function Prototypes ---

void init_board(uint8_t (*b)[BOARD_SIZE][BOARD_SIZE]);
void place_piece(const uint16_t (*piece)[SPRITE_SIZE * SPRITE_SIZE], int c, int r);
void draw_selector(int new_c, int new_r);
void render_board(uint8_t (*b)[BOARD_SIZE][BOARD_SIZE]);
void redraw_board(uint8_t (*b)[BOARD_SIZE][BOARD_SIZE]);

#endif