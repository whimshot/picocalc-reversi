#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <sys/stat.h>
#include <errno.h>

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

#include "picocalc-reversi.h"

#define SIZE 8
#define EMPTY ' '
#define HUMAN 'H'   // Black
#define MACHINE 'M' // White

volatile bool user_interrupt = false;
extern void readline(char *buffer, size_t size);
uint8_t columns = 64;

char board[SIZE][SIZE];
int cursor_row = 0; // Current cursor row position (0-7)
int cursor_col = 0; // Current cursor column position (0-7)
char status[128] = "Select a square and press ENTER to place piece\nor press ESC to quit:";

const int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
const int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

void shell_exit()
{
#if PICO_RP2350
    // Files must be closed first; the REPL keeps none open between commands
    printf("Returning to the shell...\n");
    if (!picocalc_shell_exit())
    {
        // It only returns when there is no shell to go back to
        printf("Cannot return to the shell.\n");
        printf("The shell needs UF2 Loader to have\nstarted this program, and the\nshell on the SD card.\n");
    }
#else
    printf("The PicoCalc shell needs an RP2350.\n");
#endif
}

void init_board(void)
{
    for (int r = 0; r < SIZE; r++)
    {
        for (int c = 0; c < SIZE; c++)
        {
            board[r][c] = EMPTY;
        }
    }
    board[3][3] = MACHINE;
    board[3][4] = HUMAN;
    board[4][3] = HUMAN;
    board[4][4] = MACHINE;
    cursor_row = 0; // Reset cursor position to top-most row
    cursor_col = 0; // Reset cursor position to top-most column
}

bool in_bounds(int r, int c)
{
    return r >= 0 && r < SIZE && c >= 0 && c < SIZE;
}

// Returns number of pieces flipped if 'piece' is placed at (r, c).
// If 'apply' is true, modifies the board.
int evaluate_move(int r, int c, char piece, bool apply)
{
    if (!in_bounds(r, c) || board[r][c] != EMPTY)
        return 0;

    char opponent = (piece == HUMAN) ? MACHINE : HUMAN;
    int total_flipped = 0;

    for (int d = 0; d < 8; d++)
    {
        int nr = r + dr[d];
        int nc = c + dc[d];
        int count = 0;

        while (in_bounds(nr, nc) && board[nr][nc] == opponent)
        {
            nr += dr[d];
            nc += dc[d];
            count++;
        }

        if (count > 0 && in_bounds(nr, nc) && board[nr][nc] == piece)
        {
            total_flipped += count;
            if (apply)
            {
                int fr = r + dr[d];
                int fc = c + dc[d];
                for (int i = 0; i < count; i++)
                {
                    if (fr < SIZE && fc < SIZE) // Ensure we don't go out of bounds
                    {
                        board[fr][fc] = piece;
                        fr += dr[d];
                        fc += dc[d];
                    }
                }
            }
        }
    }

    if (apply && total_flipped > 0)
    {
        board[r][c] = piece;
    }
    return total_flipped;
}

bool has_valid_moves(char piece)
{
    for (int r = 0; r < SIZE; r++)
    {
        for (int c = 0; c < SIZE; c++)
        {
            if (evaluate_move(r, c, piece, false) > 0)
                return true;
        }
    }
    return false;
}

void count_pieces(int *human_score, int *machine_score)
{
    *human_score = 0;
    *machine_score = 0;
    for (int r = 0; r < SIZE; r++)
    {
        for (int c = 0; c < SIZE; c++)
        {
            if (board[r][c] == HUMAN)
                (*human_score)++;
            else if (board[r][c] == MACHINE)
                (*machine_score)++;
        }
    }
}

void draw_board(const char *status_msg)
{
    int human_score, machine_score;
    count_pieces(&human_score, &machine_score);

    // Clear screen and move cursor to top-left
    // printf("\033[2J\033[H");

    // printf("\033[?25l"); // Hide cursor

    // Switch to DEC Special Character Set and draw a box
    // Visual representation of what will be drawn:
    //   ┌───┬───┐
    //   │   │   │
    //   ├───┼───┤
    //   │   │   │
    //   └───┴───┘
    //
    // DEC Special Character mappings:
    // l = ┌ (top-left)     q = ─ (horizontal)   k = ┐ (top-right)
    // x = │ (vertical)     w = ┬ (top-tee)
    // t = ├ (left-tee)     n = ┼ (cross)        u = ┤ (right-tee)
    // m = └ (bottom-left)  v = ┴ (bottom-tee)   j = ┘ (bottom-right)

    printf("\033[2J"); // Clear screen
    printf("\033[H\033[?25h"); // Move cursor to top-left and show cursor
    printf("Human (\033[91;5;208m%c\033[37;5;208m): %d\t", HUMAN, human_score);
    printf("Machine (\033[94;5;208m%c\033[37;5;208m): %d\n\n\n", MACHINE, machine_score);

    printf("\033(0"); // Select DEC Special Character Set for G0
    printf("\033[32;5;208m"); // Set foreground colour to green
    printf("  lqqqwqqqwqqqwqqqwqqqwqqqwqqqwqqqk\n"); // ┌──────┬──────┐

    for (int r = 0; r < SIZE; r++)
    {
        printf("  x"); // ├──────┼──────┤
        for (int c = 0; c < SIZE; c++)
        {
            if (board[r][c] == HUMAN)           // Set color for HUMAN pieces
            {
                printf("\033[91;5;208m");       // Red color for HUMAN
            }
            else if (board[r][c] == MACHINE)    // Set color for MACHINE pieces
            {
                printf("\033[94;5;208m");       // Blue color for MACHINE
            }
            printf(" %c ", board[r][c]);        // Print the piece (HUMAN, MACHINE, or EMPTY)
            printf("\033[32;5;208m");           // Reset color to green for the board lines
            printf("x");
        }
        printf("\n");
        if (r < SIZE - 1)
        {
            printf("  tqqqnqqqnqqqnqqqnqqqnqqqnqqqnqqqu\n"); // ├──────┼──────┤
        }
    }
    printf("  mqqqvqqqvqqqvqqqvqqqvqqqvqqqvqqqj\n"); // └──────┴──────┘
    printf("\033(B\033[?25h");                       // Select ASCII for G0 and show cursor
    printf("\033[0m");                               // Reset colors

    // Display scores and status message
    printf("%s\n", status_msg, top_left_row);
}

// Simple MACHINE prioritizing corners, then maximum flips
void machine_turn(void)
{
    int best_r = -1, best_c = -1, best_score = -1;

    for (int r = 0; r < SIZE; r++)
    {
        for (int c = 0; c < SIZE; c++)
        {
            int flips = evaluate_move(r, c, MACHINE, false);
            if (flips > 0)
            {
                int weight = flips;
                // Bonus weight for capturing corners
                if ((r == 0 || r == 7) && (c == 0 || c == 7))
                    weight += 20;
                if (weight > best_score)
                {
                    best_score = weight;
                    best_r = r;
                    best_c = c;
                }
            }
        }
    }

    if (best_r != -1)
    {
        evaluate_move(best_r, best_c, MACHINE, true);
    }
}

void place_cursor(int row, int col)
{
    printf("\033[H");
    if (row < 0 || row >= SIZE || col < 0 || col >= SIZE)
        return;
    int new_row = top_left_row + row * cell_height; // Calculate the cursor row based on the cell height and offset
    int new_col = top_left_col + col * cell_width; // Calculate the cursor column based on the cell width and offset
    printf("\033[%d;%dH", new_row, new_col);
}

int main(void)
{
    picocalc_init();            // Initialize PicoCalc hardware and drivers
    init_board();               // Initialize the Reversi board with starting pieces
    lcd_set_font(&font_5x10);   // Set the LCD font to 5x10 pixel font for better readability
    sb_write_lcd_backlight(255); // Set the LCD backlight to maximum brightness
    sb_write_keyboard_backlight(255);  // Set the keyboard backlight to maximum brightness
    // Display initial instructions and prompt for user input
    bool selected = false;
    bool running = true;
    draw_board(status);
    while (running)
    {
        bool human_can_move = has_valid_moves(HUMAN);
        bool machine_can_move = has_valid_moves(MACHINE);

        if (!human_can_move && !machine_can_move)
        {
            int human_score, machine_score;
            count_pieces(&human_score, &machine_score);
            if (human_score > machine_score)
                draw_board("GAME OVER: You Win!");
            else if (machine_score > human_score)
                draw_board("GAME OVER: MACHINE Wins!");
            else
                draw_board("GAME OVER: It's a Tie!");

            printf("\nPress any key to restart...");
            getchar();
            init_board();
            snprintf(status, sizeof(status), "Select a square and press ENTER to place piece\nor press ESC to quit:");
            continue;
        }

        if (human_can_move)
        {
            draw_board(status);
            place_cursor(cursor_row, cursor_col); // Place cursor at the current position
            bool selected = false;
            while (!selected)
            {
                uint8_t key = (uint8_t)keyboard_get_key(); // Get next key pressed
                switch (key)
                {
                    case KEY_UP:
                        cursor_row--; // Update cursor row position
                        if (cursor_row < 0) 
                        {
                            cursor_row = SIZE - 1; // Wrap around to bottom if needed
                            printf("\033[14B"); // Return cursor to bottom of the board
                        }
                        else {
                            printf("\033[2A"); // Move cursor up two lines
                        }
                        break;
                    case KEY_DOWN:
                        cursor_row++; // Update cursor row position
                        if (cursor_row >= SIZE) 
                        {
                            cursor_row = 0; // Wrap around to top if needed
                            printf("\033[14A"); // Reset cursor to top of the board
                        }
                        else {
                            printf("\033[2B"); // Move cursor up two lines
                        }
                        break;
                    case KEY_RIGHT:
                        cursor_col++; // Update cursor column position
                        if (cursor_col >= SIZE)
                        {
                            cursor_col = 0; // Wrap around to left if needed
                            printf("\033[28D"); // Move cursor right four columns
                        }
                        else {
                            printf("\033[4C"); // Move cursor right four columns
                        }
                        break;
                    case KEY_LEFT:
                        cursor_col--;// Update cursor column position
                        if (cursor_col < 0)
                        {
                            cursor_col = SIZE - 1; // Wrap around to right if needed
                            printf("\033[28C"); // Move cursor left four columns
                        }
                        else {
                            printf("\033[4D"); // Move cursor left four columns
                        }
                        break;
                    case KEY_RETURN:
                    case '\n':
                        selected = true; // Set selected to true to exit the loop
                        break;

                    case KEY_ESC:
                        running = false;
                        shell_exit(); // Exit the shell cleanly
                        break;
                }
            }

            if (evaluate_move(cursor_row, cursor_col, HUMAN, true) == 0)
            {
                snprintf(status, sizeof(status), "Invalid move. Try again:");
                continue;
            }
        }
        else
        {
            draw_board("No legal moves for you! Passing...");
            // place_cursor(cursor_row, cursor_col); // Place cursor at the current position
            sleep_ms(1500);
        }

        if (has_valid_moves(MACHINE))
        {
            draw_board("MACHINE is thinking...");
            // place_cursor(cursor_row, cursor_col); // Place cursor at the current position
            sleep_ms(600);
            machine_turn();
            snprintf(status, sizeof(status), "Your turn. Select a move and press ENTER or ESC to quit:");
        }
        else if (has_valid_moves(HUMAN))
        {
            snprintf(status, sizeof(status), "MACHINE had no moves and passed!");
        }
    }
    return 0;
}
