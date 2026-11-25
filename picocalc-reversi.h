
#ifndef PICOCALC_REVERSI_H
#define PICOCALC_REVERSI_H

/* ─── Includes ────────────────────────────────────────────────────────── */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/bootrom.h"
#include "pico/float.h"
#include "pico/util/datetime.h"

#if PICO_RP2350
#include "picocalc_shell.h" // Only the RP2350 runs the shell
#endif

#include "drivers/display.h"
#include "drivers/keyboard.h"
#include "drivers/lcd.h"
#include "drivers/picocalc.h"
#include "drivers/sdcard.h"
#include "drivers/southbridge.h"

#include "graphic-game-board.h"

/* ─── Constants / Macros ──────────────────────────────────────────────── */

const int top_left_row = 5;
const int top_left_col = 5;
const int cell_width = 4;
const int cell_height = 2;

/* ─── Function Prototypes ─────────────────────────────────────────────── */

#define COLOR_TEXT RGB565(224, 224, 224)

int black_score, white_score;
bool game_started;
bool selected;

void shell_exit(void);
bool in_bounds(int r, int c);
int evaluate_move(int r, int c, char piece, bool apply);
bool has_valid_moves(char piece);
void count_pieces(int *human_score, int *machine_score);
void machine_turn(void);

#endif /* PICOCALC_REVERSI_H */
