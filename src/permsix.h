#ifndef SWAP_PERMSIX_H
#define SWAP_PERMSIX_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/* Degree-six permutation ACTION, not a braid or a braid equivalence test.
   image[old_slot] is the new slot of the same permanently coloured object. */
enum { SWAP_SIX_N = 6, SWAP_SIX_QUEUE_CAP = 64 };
enum { SWAP_SIX_CYCLIC = 0, SWAP_SIX_DIHEDRAL = 1,
       SWAP_SIX_SYMMETRIC = 2, SWAP_SIX_GROUP_COUNT = 3 };

typedef struct { uint8_t image[SWAP_SIX_N]; } SwapSixPermutation;
typedef struct { uint8_t identity_at[SWAP_SIX_N]; } SwapSixPlacement;
typedef struct { float x, y; } SwapSixPoint;
/* One shared set of button rectangles for raster drawing AND touch picking.
   All coordinates are in the original 360-wide canvas; six occupies y>720. */
typedef struct { float x, y, width, height; } SwapSixBox;
SwapSixBox swap_six_control_box(unsigned index); /* 0..5 */
SwapSixBox swap_six_open_box(void);
SwapSixBox swap_six_back_box(void);
bool swap_six_box_contains(SwapSixBox box,float x,float y);
int swap_six_control_hit(float x,float y);

typedef struct {
    unsigned group;
    SwapSixPlacement current; /* fully completed motion */
    SwapSixPlacement start, goal; /* current interpolation endpoints */
    SwapSixPermutation pending[SWAP_SIX_QUEUE_CAP];
    unsigned first, queued;
    unsigned completed;
    float phase;           /* [0,1] while moving */
    bool moving;
    uint32_t random_state; /* reproducible, NON-cryptographic test/demo PRNG */
} SwapSix;

void swap_six_init(SwapSix *state);
bool swap_six_select_group(SwapSix *state, unsigned group);
void swap_six_reset(SwapSix *state); /* same selected group */
bool swap_six_permutation_valid(SwapSixPermutation permutation);
bool swap_six_placement_valid(SwapSixPlacement order);
SwapSixPlacement swap_six_apply(SwapSixPlacement before, SwapSixPermutation map);
SwapSixPermutation swap_six_generator(unsigned group, unsigned generator);
const char *swap_six_group_name(unsigned group);
const char *swap_six_generator_name(unsigned group, unsigned generator);
unsigned swap_six_group_order(unsigned group);
bool swap_six_enqueue(SwapSix *state, SwapSixPermutation permutation);
bool swap_six_request_generator(SwapSix *state, unsigned generator);
bool swap_six_request_random_nonidentity(SwapSix *state);
void swap_six_tick(SwapSix *state, double seconds);
bool swap_six_position(const SwapSix *state, unsigned identity, SwapSixPoint *point);
/* Cycle form of the completed action on ORIGINAL positions, fixed points omitted. */
void swap_six_cycles(SwapSixPlacement order, char *buffer, size_t capacity);

#endif
