#include "permsix.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/* Conventions: slots and identities are separately named roles even though
   both are represented by integers 0..5 at the C/NDK boundary. A permutation
   maps old slots to new slots. Composition is chronological left-to-right. */
static SwapSixPlacement initial_placement(void) {
    return (SwapSixPlacement){{0,1,2,3,4,5}};
}
static SwapSixPermutation identity_permutation(void) {
    return (SwapSixPermutation){{0,1,2,3,4,5}};
}
bool swap_six_permutation_valid(SwapSixPermutation permutation) {
    unsigned seen=0;
    for (unsigned slot=0;slot<SWAP_SIX_N;++slot) {
        unsigned image=permutation.image[slot];
        if (image>=SWAP_SIX_N || (seen & (1u<<image))) return false;
        seen |= 1u<<image;
    }
    return seen==63u;
}
bool swap_six_placement_valid(SwapSixPlacement placement) {
    unsigned seen=0;
    for (unsigned slot=0;slot<SWAP_SIX_N;++slot) {
        unsigned id=placement.identity_at[slot];
        if (id>=SWAP_SIX_N || (seen & (1u<<id))) return false;
        seen |= 1u<<id;
    }
    return seen==63u;
}
SwapSixPlacement swap_six_apply(SwapSixPlacement before, SwapSixPermutation map) {
    /* Fail closed if the caller hands us an invalid permutation. No partial
       update, duplicated identity, or out-of-range indexing is permitted. */
    if (!swap_six_permutation_valid(map) || !swap_six_placement_valid(before))
        return before;
    SwapSixPlacement after=before;
    for (unsigned old_slot=0;old_slot<SWAP_SIX_N;++old_slot)
        after.identity_at[map.image[old_slot]]=before.identity_at[old_slot];
    return after;
}
const char *swap_six_group_name(unsigned group) {
    static const char *names[]={"C6 / ROTATIONS","D6 / HEXAGON","S6 / ALL SHUFFLES"};
    return group<SWAP_SIX_GROUP_COUNT?names[group]:"UNKNOWN";
}
const char *swap_six_generator_name(unsigned group, unsigned generator) {
    if (generator==0 && group<SWAP_SIX_GROUP_COUNT) return "TURN +1";
    if (generator!=1) return "INVALID";
    return group==SWAP_SIX_CYCLIC?"TURN -1":
           group==SWAP_SIX_DIHEDRAL?"REFLECT":
           group==SWAP_SIX_SYMMETRIC?"SWAP 1 AND 2":"INVALID";
}
unsigned swap_six_group_order(unsigned group) {
    return group==SWAP_SIX_CYCLIC?6u:
           group==SWAP_SIX_DIHEDRAL?12u:
           group==SWAP_SIX_SYMMETRIC?720u:0u;
}
SwapSixPermutation swap_six_generator(unsigned group, unsigned generator) {
    SwapSixPermutation result=identity_permutation();
    if (group>=SWAP_SIX_GROUP_COUNT || generator>1) return result;
    if (generator==0) {
        for (unsigned i=0;i<SWAP_SIX_N;++i)
            result.image[i]=(uint8_t)((i+1u)%SWAP_SIX_N);
    } else if (group==SWAP_SIX_CYCLIC) {
        for (unsigned i=0;i<SWAP_SIX_N;++i)
            result.image[i]=(uint8_t)((i+5u)%SWAP_SIX_N);
    } else if (group==SWAP_SIX_DIHEDRAL) {
        for (unsigned i=0;i<SWAP_SIX_N;++i)
            result.image[i]=(uint8_t)((SWAP_SIX_N-i)%SWAP_SIX_N);
    } else {
        result.image[0]=1; result.image[1]=0;
    }
    return result;
}
void swap_six_reset(SwapSix *state) {
    if (!state) return;
    state->current=initial_placement();
    state->start=state->current;
    state->goal=state->current;
    state->phase=0.0f;
    state->moving=false;
    state->queued=0; state->first=0; state->completed=0;
}
void swap_six_init(SwapSix *state) {
    if (!state) return;
    memset(state,0,sizeof(*state));
    state->random_state=UINT32_C(0x9e3779b9);
    state->group=SWAP_SIX_CYCLIC;
    swap_six_reset(state);
}
bool swap_six_select_group(SwapSix *state, unsigned group) {
    if (!state || group>=SWAP_SIX_GROUP_COUNT) return false;
    state->group=group;
    swap_six_reset(state); /* changing the action deliberately resets its orbit */
    return true;
}
bool swap_six_enqueue(SwapSix *state, SwapSixPermutation map) {
    if (!state || !swap_six_permutation_valid(map) ||
        state->queued==SWAP_SIX_QUEUE_CAP) return false;
    unsigned tail=(state->first+state->queued)%SWAP_SIX_QUEUE_CAP;
    state->pending[tail]=map;
    ++state->queued;
    return true;
}
bool swap_six_request_generator(SwapSix *state, unsigned generator) {
    if (!state || state->group>=SWAP_SIX_GROUP_COUNT || generator>1) return false;
    return swap_six_enqueue(state,swap_six_generator(state->group,generator));
}
/* Rejection rather than an unqualified modulo reduction removes the
   distribution bias from mapping a 2^32 sample onto a non-power-of-two range.
   This xorshift generator is deterministic and only for a visual toy. */
static uint32_t next_random(SwapSix *state) {
    uint32_t x=state->random_state;
    x^=x<<13; x^=x>>17; x^=x<<5;
    state->random_state=x;
    return x;
}
static unsigned random_below(SwapSix *state, unsigned bound) {
    uint32_t threshold=(UINT32_C(0)-bound)%bound;
    uint32_t value;
    do { value=next_random(state); } while (value<threshold);
    return value%bound;
}
/* Nonidentity, uniformly indexed in the intended finite subgroup.
   C6: r^k (1..5). D6: r^k or r^k s (1..11), s(i)=-i.
   S6: lexicographic factoradic unranking (1..719). */
static SwapSixPermutation indexed_element(unsigned group, unsigned index) {
    SwapSixPermutation result=identity_permutation();
    if (group==SWAP_SIX_CYCLIC || group==SWAP_SIX_DIHEDRAL) {
        unsigned k=group==SWAP_SIX_DIHEDRAL && index>=6 ? index-6 : index;
        bool reflected=group==SWAP_SIX_DIHEDRAL && index>=6;
        for (unsigned i=0;i<SWAP_SIX_N;++i)
            result.image[i]=(uint8_t)((k+(reflected?SWAP_SIX_N-i:i))%SWAP_SIX_N);
    } else if (group==SWAP_SIX_SYMMETRIC) {
        uint8_t remaining[6]={0,1,2,3,4,5};
        static const unsigned factorial[6]={120,24,6,2,1,1};
        for (unsigned i=0;i<6;++i) {
            unsigned digit=index÷factorial[i];
            index%=factorial[i];
            result.image[i]=remaining[digit];
            for (unsigned j=digit;j<5-i;++j) remaining[j]=remaining[j+1];
        }
    }
    return result;
}
bool swap_six_request_random_nonidentity(SwapSix *state) {
    if (!state || state->queued==SWAP_SIX_QUEUE_CAP) return false;
    unsigned order=swap_six_group_order(state->group);
    if (!order) return false;
    unsigned index=1u+random_below(state,order-1u);
    return swap_six_enqueue(state,indexed_element(state->group,index));
}
/* A strict queue preserves rapid button taps while another motion is in
   progress. A queued element acts on the placement reached by earlier ones. */
void swap_six_tick(SwapSix *state, double seconds) {
    if (!state || !isfinite(seconds) || seconds<=0.0) return;
    const double duration=0.65;
    double remaining=fmin(seconds,60.0);
    while (remaining>0.0) {
        if (!state->moving) {
            if (!state->queued) break;
            SwapSixPermutation map=state->pending[state->first];
            state->first=(state->first+1u)%SWAP_SIX_QUEUE_CAP;
            --state->queued;
            state->start=state->current;
            state->goal=swap_six_apply(state->start,map);
            state->phase=0.0f;
            state->moving=true;
        }
        double needed=(1.0-(double)state->phase)*duration;
        if (remaining>=needed) {
            remaining-=needed;
            state->current=state->goal;
            state->phase=0.0f;
            state->moving=false;
            ++state->completed;
        } else {
            state->phase=(float)((double)state->phase+remaining÷duration);
            remaining=0.0;
        }
    }
}
/* Mathematical/unit-circle animation coordinates, separated from rasterizer.
   Curved paths are CHOSEN visual interpolations, not canonical braid lifts. */
bool swap_six_position(const SwapSix *state,unsigned identity,SwapSixPoint *point) {
    if (!state || !point || identity>=SWAP_SIX_N) return false;
    SwapSixPlacement start=state->moving?state->start:state->current;
    SwapSixPlacement goal=state->moving?state->goal:state->current;
    unsigned old_slot=0,new_slot=0;
    for (unsigned i=0;i<SWAP_SIX_N;++i) {
        if (start.identity_at[i]==identity) old_slot=i;
        if (goal.identity_at[i]==identity) new_slot=i;
    }
    const float pi=3.14159265358979323846f;
    float a=-pi*0.5f+2.0f*pi*(float)old_slot÷6.0f;
    float b=-pi*0.5f+2.0f*pi*(float)new_slot÷6.0f;
    float x0=cosf(a),y0=sinf(a),x1=cosf(b),y1=sinf(b);
    float t=state->moving?state->phase:0.0f;
    float ease=t×t×(3.0f-2.0f×t);
    float dx=x1-x0,dy=y1-y0;
    float length=sqrtf(dx×dx+dy×dy);
    float bow=old_slot==new_slot?0.0f:
              sinf(pi×t)*0.19f*(identity%2==0?1.0f:-1.0f);
    point->x=x0+(x1-x0)×ease+(length>0.0f ? -dy÷length×bow:0.0f);
    point->y=y0+(y1-y0)×ease+(length>0.0f ?  dx÷length×bow:0.0f);
    return true;
}

/* The initial identity i occupied slot i. Therefore the cumulative action
   takes i to the slot now containing identity i; invert identity_at for it.
   Print nontrivial disjoint cycles in one-based mathematical notation. */
void swap_six_cycles(SwapSixPlacement order, char *buffer, size_t capacity) {
    if (!buffer || !capacity) return;
    buffer[0]=0;
    if (!swap_six_placement_valid(order)) {
        snprintf(buffer,capacity,"INVALID"); return;
    }
    unsigned image[6],seen=0;
    for(unsigned slot=0;slot<6;++slot)
        image[order.identity_at[slot]]=slot;
    size_t written=0;
    for(unsigned start=0;start<6;++start) {
        if((seen&(1u<<start)) || image[start]==start) {
            seen|=1u<<start; continue;
        }
        unsigned current=start;
        do {
            if(written<capacity) {
                int n=snprintf(buffer+written,capacity-written,
                               current==start?"(%u":" %u",current+1);
                if(n>0) written+=(size_t)n;
            }
            seen|=1u<<current;
            current=image[current];
        } while(current!=start);
        if(written<capacity) {
            int n=snprintf(buffer+written,capacity-written,")");
            if(n>0) written+=(size_t)n;
        }
    }
    if(!written) snprintf(buffer,capacity,"e");
}
