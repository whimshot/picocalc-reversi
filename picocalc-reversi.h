
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


/* ─── Constants / Macros ──────────────────────────────────────────────── */

#define SIZE 8
#define EMPTY ' '
#define HUMAN 'H'   // Black
#define MACHINE 'M' // White

const int top_left_row = 5; 
const int top_left_col = 5; 
const int cell_width = 4; 
const int cell_height = 2; 

/* ─── Function Prototypes ─────────────────────────────────────────────── */

void shell_exit(void);
void init_board(void);
bool in_bounds(int r, int c);
int evaluate_move(int r, int c, char piece, bool apply);
bool has_valid_moves(char piece);
void count_pieces(int *human_score, int *machine_score);
void draw_board(const char *status_msg);
void machine_turn(void);
void read_move(char *col_out, char *row_out);
void draw_selector(int row, int col);


#endif /* PICOCALC_REVERSI_H */
