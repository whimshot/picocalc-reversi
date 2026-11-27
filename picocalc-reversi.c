
#include "picocalc-reversi.h"
#include "graphic-game-board.h"

#if PICO_RP2350
#include "lcdshot.h"
#endif

const int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
const int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

char score[128];
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

bool in_bounds(int r, int c)
{
    return r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE;
}

// Returns number of pieces flipped if 'piece' is placed at (r, c).
// If 'apply' is true, modifies the board.
int evaluate_move(int r, int c, char piece, bool apply, int board_type)
{
    if (!in_bounds(r, c) || board[r][c] != EMPTY)
        return 0;

    char opponent = (piece == WHITE) ? BLACK : WHITE;
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
                    if (fr < BOARD_SIZE && fc < BOARD_SIZE) // Ensure we don't go out of bounds
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
        switch (piece)
        {
        case WHITE:
            place_piece(&white_disc, r, c);
            redraw_board(&board, board_type);
            count_pieces(&black_score, &white_score);
            printf("\033[HWhite: %d Black: %d", white_score, black_score);
            break;
        case BLACK:
            place_piece(&black_disc, selector_r, selector_c);
            redraw_board(&board, board_type);
            count_pieces(&black_score, &white_score);
            printf("\033[HWhite: %d Black: %d", white_score, black_score);
            break;
        }
    }
    return total_flipped;
}

bool has_valid_moves(char piece, int board_type)
{
    for (int r = 0; r < BOARD_SIZE; r++)
    {
        for (int c = 0; c < BOARD_SIZE; c++)
        {
            if (evaluate_move(r, c, piece, false, board_type) > 0)
                return true;
        }
    }
    return false;
}

void count_pieces(int *black_score, int *white_score)
{
    // debug_printf("Called: %s\n", __func__);
    *black_score = 0;
    *white_score = 0;
    for (int r = 0; r < BOARD_SIZE; r++)
    {
        for (int c = 0; c < BOARD_SIZE; c++)
        {
            if (board[r][c] == BLACK)
                (*black_score)++;
            else if (board[r][c] == WHITE)
                (*white_score)++;
        }
    }
}

// Simple MACHINE prioritizing corners, then maximum flips
void machine_turn(int board_type)
{
    int best_r = -1, best_c = -1, best_score = -1;

    for (int r = 0; r < BOARD_SIZE; r++)
    {
        for (int c = 0; c < BOARD_SIZE; c++)
        {
            int flips = evaluate_move(r, c, WHITE, false, board_type);
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
        evaluate_move(best_r, best_c, WHITE, true, board_type);
    }
}

void sidebars(void)
{
    int start_row = 3;
    int left_column = 0;
    int right_column = 63;

#if PICO_RP2350
    char left_side[] = "ESC Quit - DEL BOOTSEL";
    char right_side[] = "R restart - F10 Screenshot";
#else
    char left_side[] = "DEL BOOTSEL";
    char right_side[] = "R restart";
#endif

    int row = start_row;
    for (int i = 0; i < strlen(left_side); i++)
    {
        printf("\033[%d;%dH%c", row, left_column, left_side[i]);
        row++;
    }
    row = start_row;
    for (int i = 0; i < strlen(right_side); i++)
    {
        printf("\033[%d;%dH%c", row, right_column, right_side[i]);
        row++;
    }
}

void restart_game(int board_type)
{
    move_selected = false;
    game_started = false;
    init_board(&board, board_type);
    count_pieces(&black_score, &white_score);
    render_board(&board, board_type);
    printf("\033[32H\033[2KSelect a square and press ENTER");
}

int main(void)
{
    picocalc_init(); // Initialize PicoCalc hardware and drivers
    stdio_init_all();
    int board_type = REVERSI;
    init_board(&board, board_type); // Initialize the Reversi board with starting pieces

    lcd_enable_cursor(false); // Turn cursor off
    lcd_set_foreground(COLOR_TEXT);
    lcd_set_bold(true);
    sb_write_lcd_backlight(255);      // Set the LCD backlight to maximum brightness
    sb_write_keyboard_backlight(255); // Set the keyboard backlight to maximum brightness
                                      // Display initial instructions and prompt for user input
    game_started = false;             // the game hasn't started yet
    move_selected = false;
    bool running = true;
    render_board(&board, board_type);
    sidebars();
    while (running)
    {
        bool black_can_move = has_valid_moves(BLACK, board_type);
        bool white_can_move = has_valid_moves(WHITE, board_type);

        // both sides are out of moves the game is over.
        if (!black_can_move && !white_can_move)
        {
            // Check for end of game conditions. Outcome.
            count_pieces(&black_score, &white_score);
            if (black_score > white_score) // Black wins.
            {
                redraw_board(&board, board_type);
                printf("\033[H\033[2KWhite: %d Black: %d. Black Wins!", white_score, black_score);
                printf("\033[32H\033[2KGAME OVER!");
                erase_selector(COLOR_GREEN);
                game_over = true;
            }
            else if (white_score > black_score) // White wins.
            {
                redraw_board(&board, board_type);
                printf("\033[H\033[2KWhite: %d Black: %d. White Wins!", white_score, black_score);
                printf("\033[32H\033[2KGAME OVER!");
                erase_selector(COLOR_GREEN);
                game_over = true;
            }
            else // The game is a tie.
            {
                redraw_board(&board, board_type);
                printf("\033[H\033[2KWhite: %d Black: %d. It's A TIE!", white_score, black_score);
                printf("\033[32H\033[2KGAME OVER!");
                erase_selector(COLOR_GREEN);
                game_over = true;
            }
            uint8_t key = (uint8_t)keyboard_get_key(); // Get next key pressed
            switch (key)
            {
            case 'r':
            case 'R':
                move_selected = false;
                game_started = false;
                init_board(&board, board_type);
                count_pieces(&black_score, &white_score);
                render_board(&board, board_type);
                printf("\033[H\033[2KWhite: %d Black: %d", white_score, black_score);
                printf("\033[32H\033[2KSelect a square and press ENTER");
                break;

            case KEY_ESC:
                running = false;
                shell_exit(); // Exit the shell cleanly
                break;

            case KEY_DEL:
                reset_usb_boot(0, 0);
                break;

#if PICO_RP2350
            case KEY_F10:
                char saved[80];
                lcd_screenshot("reversi", saved, sizeof saved);
                break;
#endif
            }
        }

        if (black_can_move)
        {
            redraw_board(&board, board_type);
            count_pieces(&black_score, &white_score);
            printf("\033[H\033[2KWhite: %d Black: %d", white_score, black_score);
            move_selector(selector_r, selector_c, board_type); // Place cursor at the current position
            move_selected = false;
            while (!move_selected)
            {
                uint8_t key = (uint8_t)keyboard_get_key(); // Get next key pressed
                switch (key)
                {
                case KEY_UP:
                    if (selector_r > 0)
                    {
                        move_selector(selector_r - 1, selector_c, board_type);
                    }
                    break;

                case KEY_DOWN:
                    if (selector_r < BOARD_SIZE - 1)
                    {
                        move_selector(selector_r + 1, selector_c, board_type);
                    }
                    break;

                case KEY_RIGHT:
                    if (selector_c < BOARD_SIZE - 1)
                    {
                        move_selector(selector_r, selector_c + 1, board_type);
                    }
                    break;

                case KEY_LEFT:
                    if (selector_c > 0)
                    {
                        move_selector(selector_r, selector_c - 1, board_type);
                    }
                    break;

                case KEY_RETURN:
                case KEY_ENTER:
                    move_selected = true; // Set move_selected to true to exit the loop
                    if (!game_started)
                        game_started = true;
                    break;

                case KEY_ESC:
                    running = false;
                    move_selected = true; // close the !move_selected while loop
                    lcd_reset();
                    shell_exit(); // Exit the shell cleanly
                    break;

                case KEY_DEL:
                    reset_usb_boot(0, 0);
                    break;

                case 'r':
                case 'R':
                    move_selected = false;
                    game_started = false;
                    init_board(&board, board_type);
                    count_pieces(&black_score, &white_score);
                    render_board(&board, board_type);
                    printf("\033[H\033[2KWhite: %d Black: %d", white_score, black_score);
                    printf("\033[32H\033[2KSelect a square and press ENTER");
                    break;

#if PICO_RP2350
                case KEY_F10:
                    char saved[80];
                    lcd_screenshot("reversi", saved, sizeof saved);
                    break;
#endif

                default:
                    break;
                }
            }
            if (game_started)
            {
                if (evaluate_move(selector_r, selector_c, BLACK, true, board_type) == 0)
                {
                    printf("\033[32H\033[2KInvalid move. Try again:");
                    continue;
                }
            }
        }
        else if (game_started)
        {
            redraw_board(&board, board_type);
            count_pieces(&black_score, &white_score);
            printf("\033[H\033[2KWhite: %d Black: %d", white_score, black_score);
        }

        if (has_valid_moves(WHITE, board_type) && game_started)
        {
            erase_selector(COLOR_GREEN);
            place_piece(&black_disc, selector_r, selector_c);
            redraw_board(&board, board_type);
            count_pieces(&black_score, &white_score);
            printf("\033[H\033[2KWhite: %d Black: %d", white_score, black_score);
            machine_turn(board_type);
            printf("\033[32H\033[2KSelect a square and press ENTER.");
        }
        else if (has_valid_moves(BLACK, board_type) && game_started)
        {
            printf("\033[32H\033[2KWhite had no moves and passed!");
        }
    }
    return 0;
}
