#ifndef SWAP_VIEWS_H
#define SWAP_VIEWS_H
#include "input.h"
#include <stdbool.h>
#include <stdint.h>

/* Independent visual experiments; the existing three-strand model stays intact. */
typedef enum {
    SWAP_VIEW_THREE=0, SWAP_VIEW_TWO=1,
    SWAP_VIEW_GRID=2, SWAP_VIEW_SETS=3,
    SWAP_VIEW_COUNT=4
} SwapView;

int swap_view_at(SwapPointer point);

/* B_2 has one signed generator, and projects onto the two permutations.
   Reuse SwapScene with adjacent=0; never discard signed history. */
int swap_pair_head_at(SwapPointer point, float head_y);
bool swap_pair_drag_move(int slot, SwapPointer from, SwapPointer to,
                         bool grabbed_over, SwapMove *move);

typedef struct {
    uint8_t cell[9];     /* 0=empty, 1=X, 2=O */
    uint8_t next;        /* player 1 or 2 */
    uint8_t winner;      /* 0=playing, 1=X, 2=O, 3=draw */
} SwapGrid;
void swap_grid_reset(SwapGrid *grid);
int swap_grid_at(SwapPointer point);
bool swap_grid_mark(SwapGrid *grid, unsigned index);

/* A tensor of binary axes is indexed by membership words, not by
   intersecting drawn curves. This model supports 2..63 axes without
   allocating 2^n cells. Rendering can use a two-axis slice. */
typedef struct {
    unsigned axes;
    uint64_t atom;
} SwapTensor;
bool swap_tensor_set_axes(SwapTensor *tensor, unsigned axes);
uint64_t swap_tensor_size(const SwapTensor *tensor);
bool swap_tensor_select(SwapTensor *tensor, uint64_t atom);
bool swap_tensor_step(SwapTensor *tensor, int direction);
bool swap_tensor_contains(const SwapTensor *tensor, unsigned axis);
bool swap_tensor_slice(const SwapTensor *tensor, unsigned x_axis,
                       unsigned y_axis, bool x, bool y, uint64_t *atom);
#endif
