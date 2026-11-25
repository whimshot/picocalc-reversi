# Graphic Game Board

A lightweight C graphics template and interactive grid renderer for board games (initialized with a standard Reversi/Othello setup), designed for the **PicoCalc** platform using the Raspberry Pi Pico SDK.

## Features

* **Grid & Felt Board Rendering:** Draws a customizable square board with grid lines and configurable pixel offsets (`OFFSET_PX`, `CELL_SIZE`, `BOARD_PX`).
* **Sprite Blitting:** Renders game pieces (`BLACK` and `WHITE` 34×34 pixel discs) directly to the LCD buffer via `lcd_blit`.
* **Interactive Cursor / Selection Box:** Smoothly moves a highlight border across cells using keyboard arrow keys without redrawing the entire screen.
* **Hardware Integration:** Initializes PicoCalc display and keyboard subsystems, disables the text cursor, and sets both LCD and keyboard backlights to maximum brightness.

## Controls

| Key | Action |
| :--- | :--- |
| **Up / Down / Left / Right** | Move the selection box across the board |
| **Enter / Return** | Select current cell *(placeholder for game move logic)* |
| **Esc** / **Q** / **q** | Exit back to the PicoCalc shell |

## Project Structure

* `graphic-game-board.c` — Main application loop, board initialization, rendering routines (`render_board`, `draw_selector`), and keyboard input handling.
* `graphic-game-board.h` — Header containing board dimensions (`BOARD_SIZE`, `CELL_SIZE`, `BOARD_PX`, `OFFSET_PX`), color definitions (`COLOR_BOARD`, `COLOR_GRID`, `COLOR_SELECTION_BOX`), piece enums (`EMPTY`, `BLACK`, `WHITE`), sprite arrays (`black_disc`, `white_disc`), and PicoCalc hardware includes.

## Prerequisites & Building

1. **Raspberry Pi Pico SDK:** Ensure `PICO_SDK_PATH` is configured in your environment.
2. **PicoCalc SDK / Libraries:** Link against the PicoCalc display (`lcd_*`), backlight (`sb_write_*`), and keyboard (`keyboard_get_key`) libraries referenced in `graphic-game-board.h`.
3. **Compile with CMake:**

   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

4. Flash the resulting `.uf2` or binary to your PicoCalc device.

## Customization

* **Change Game Type:** Update `init_board()` to set up starting positions for Checkers, Chess, Tic-Tac-Toe, or custom grid games, and adjust `BOARD_SIZE` in `graphic-game-board.h`.
* **Add Move Logic:** Implement piece placement, valid-move checks, and turn switching inside the `KEY_RETURN` / `KEY_ENTER` case in `main()`.
