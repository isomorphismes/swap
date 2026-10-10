#ifndef SWAP_MODEL_H
#define SWAP_MODEL_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SWAP_STRANDS 3
#define SWAP_MAX_MOVES 128
#define SWAP_SECONDS_PER_MOVE 0.80

typedef struct { uint8_t adjacent; int8_t sign; } SwapMove;
typedef struct { double x, y, z; } SwapPoint;
/* order[slot] is an identity, never a colour or a geometric coordinate. */
typedef struct { uint8_t at[SWAP_STRANDS]; } SwapOrder;
typedef struct {
    SwapMove moves[SWAP_MAX_MOVES];
    size_t count;
    double cursor;                 /* continuous time in crossing units */
    bool paused;
} SwapScene;

typedef enum {
    SWAP_ACCEPTED, SWAP_INVALID_MOVE, SWAP_BUSY, SWAP_HISTORY_FULL
} SwapResult;

bool swap_move_valid(SwapMove move);
SwapMove swap_inverse(SwapMove move);
void swap_reset(SwapScene *scene);
SwapResult swap_append(SwapScene *scene, SwapMove move);
void swap_tick(SwapScene *scene, double seconds);
void swap_replay(SwapScene *scene);
/* Query functions require a scene built through this API. */
SwapOrder swap_order_at(const SwapScene *scene, size_t completed);
size_t swap_completed(const SwapScene *scene);
double swap_phase(const SwapScene *scene);
int swap_writhe(const SwapScene *scene); /* useful invariant, NOT an equality test */
/* Arrays below are indexed by strand/vertex identity, not by slot. */
void swap_crossing_points(SwapOrder before, SwapMove move, double phase,
                          SwapPoint points[SWAP_STRANDS]);
void swap_triangle_points(const SwapScene *scene, SwapPoint points[SWAP_STRANDS]);
SwapPoint swap_triangle_slot(unsigned slot);
#endif
