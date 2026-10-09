#ifndef SWAP_INPUT_H
#define SWAP_INPUT_H
#include <stdbool.h>
#include "swap.h"

/* The Android adapter and the native tests share one coordinate/hit model.
   Coordinates below are in a 360 by 720 logical canvas, never raw pixels. */
typedef struct { float x, y; } SwapPointer;
typedef struct { float scale, offset_x, offset_y; } SwapCanvas;
typedef struct {
    float x, y, width, height;
    const char *title;
    unsigned command;
} SwapButton;

enum {
    SWAP_LEFT_OVER = 0, SWAP_RIGHT_OVER = 1,
    SWAP_LEFT_UNDER = 2, SWAP_RIGHT_UNDER = 3,
    SWAP_RESET = 4, SWAP_REPLAY = 5, SWAP_PAUSE = 6,
    SWAP_DRAG_OVER = 7, SWAP_DRAG_UNDER = 8,
    SWAP_BUTTON_COUNT = 9
};
extern const SwapButton swap_buttons[SWAP_BUTTON_COUNT];
SwapCanvas swap_canvas_for_screen(float width, float height);
SwapPointer swap_pointer_on_canvas(SwapCanvas canvas, float screen_x, float screen_y);
int swap_button_at(SwapPointer pointer);
bool swap_point_on_button(SwapPointer pointer, unsigned button);
int swap_head_at(SwapPointer pointer, float row_y);
/* True only for a horizontal gesture towards an immediate neighbour.
   "dragged_over" belongs to the grabbed identity, NOT to the left slot. */
bool swap_drag_move(int slot, SwapPointer from, SwapPointer to,
                    bool dragged_over, SwapMove *move);
#endif
