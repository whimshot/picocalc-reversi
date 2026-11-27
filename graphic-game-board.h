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
#define BOARD_PX (BOARD_SIZE * CELL_SIZE) + 2      // 288x288 pixel board
#define OFFSET_PX ((SCREEN_PX - BOARD_PX) / 2) - 1 // 16px margin left, right, top, and bottom
#define SPRITE_SIZE 34

// RGB565 Color Helper
#define RGB565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

// Reversi (Othello) board and piece colors
#define COLOR_GREEN RGB565(0, 128, 0)       // felt color
#define COLOR_BLACK RGB565(10, 10, 10)      // black pieces
#define COLOR_WHITE RGB565(224, 224, 224)   // white pieces
#define COLOR_SELECTOR RGB565(251, 236, 93) // move selection box

// Draughts or Checkers square and piece colors
#define COLOR_FOREST_GREEN RGB565(4, 34, 4) // dark squares are forest green
#define COLOR_CREAM RGB565(31, 63, 26)      // light squares are cream
#define COLOR_CHARCOAL RGB565(4, 8, 4)      // dark pieces are charcoal
#define COLOR_RED RGB565(25, 4, 5)          // light pieces are classic red

#define START_POSITION_ROW 2
#define START_POSITION_COLUMN 3

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
void erase_selector(uint16_t color);
void draw_selector(int new_r, int new_c, uint16_t color);
void move_selector(int new_c, int new_r);
void render_board(uint8_t (*b)[BOARD_SIZE][BOARD_SIZE]);
void redraw_board(uint8_t (*b)[BOARD_SIZE][BOARD_SIZE]);

#endif