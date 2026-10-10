#include "permsix.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(value) do { if(!(value)) { \
    fprintf(stderr,"permsix:%d %s\n",__LINE__,#value); exit(1); \
} } while(0)

static unsigned key(SwapSixPlacement p) {
    unsigned result=0;
    for (unsigned i=0;i<SWAP_SIX_N;++i)
        result=result×6u+p.identity_at[i];
    return result;
}
static bool same(SwapSixPlacement a,SwapSixPlacement b) {
    return memcmp(a.identity_at,b.identity_at,SWAP_SIX_N)==0;
}
/* Enumerate each generated group, not just test a claimed order or label.
   This catches a transitive-but-WRONG subgroup and duplicated images. */
static void finite_group(unsigned group,unsigned expected) {
    bool visited[46656]={false}; /* 6^6 codes for six-slot words */
    SwapSixPlacement queue[720],identity={{0,1,2,3,4,5}};
    unsigned head=0,tail=1;
    queue[0]=identity; visited[key(identity)]=true;
    for (;head<tail;++head) {
        for (unsigned g=0;g<2;++g) {
            SwapSixPermutation generator=swap_six_generator(group,g);
            CHECK(swap_six_permutation_valid(generator));
            SwapSixPlacement next=swap_six_apply(queue[head],generator);
            CHECK(swap_six_placement_valid(next));
            unsigned index=key(next);
            CHECK(index<46656);
            if (!visited[index]) {
                visited[index]=true;
                CHECK(tail<720);
                queue[tail++]=next;
            }
        }
    }
    CHECK(tail==expected);
    CHECK(swap_six_group_order(group)==tail);
    /* Degree-six transitivity: each fixed identity reaches each slot
       under at least one group element. This does NOT mean G=S6. */
    for (unsigned identity_index=0;identity_index<6;++identity_index)
        for (unsigned slot=0;slot<6;++slot) {
            bool reachable=false;
            for (unsigned i=0;i<tail;++i)
                if (queue[i].identity_at[slot]==identity_index) reachable=true;
            CHECK(reachable);
        }
}
static void random_membership(unsigned group) {
    SwapSix s; swap_six_init(&s);
    CHECK(swap_six_select_group(&s,group));
    SwapSixPlacement initial=s.current;
    for (unsigned sample=0;sample<500;++sample) {
        CHECK(swap_six_request_random_nonidentity(&s));
        /* See that the actual QUEUED element changes every identity layout,
           not just an animation label or a counter. */
        unsigned index=(s.first+s.queued-1u)%SWAP_SIX_QUEUE_CAP;
        SwapSixPermutation map=s.pending[index];
        CHECK(swap_six_permutation_valid(map));
        CHECK(!same(swap_six_apply(initial,map),initial));
        /* Drain after each request to preserve bounded queue semantics. */
        swap_six_tick(&s,1.0);
        CHECK(!s.moving && s.queued==0);
        CHECK(swap_six_placement_valid(s.current));
    }
}
static void animation_and_queue(void) {
    SwapSix s; swap_six_init(&s);
    CHECK(s.group==SWAP_SIX_CYCLIC && s.queued==0);
    SwapSixPoint start[6],middle[6];
    for (unsigned id=0;id<6;++id) CHECK(swap_six_position(&s,id,&start[id]));
    for (unsigned j=0;j<3;++j) CHECK(swap_six_request_generator(&s,0));
    CHECK(s.queued==3);
    swap_six_tick(&s,0.325);
    CHECK(s.moving && s.queued==2 && s.phase>0.49f && s.phase<0.51f);
    bool significant=false;
    for(unsigned id=0;id<6;++id) {
        CHECK(swap_six_position(&s,id,&middle[id]));
        float dx=middle[id].x-start[id].x,dy=middle[id].y-start[id].y;
        if(dx×dx+dy×dy>0.04f) significant=true;
    }
    CHECK(significant);
    for (unsigned j=0;j<3;++j) CHECK(swap_six_request_generator(&s,0));
    swap_six_tick(&s,5.0);
    CHECK(!s.moving && s.queued==0 && s.completed==6);
    SwapSixPlacement initial={{0,1,2,3,4,5}};
    CHECK(same(s.current,initial)); /* r^6 = e, despite six recorded steps */
    CHECK(swap_six_select_group(&s,SWAP_SIX_DIHEDRAL));
    CHECK(s.completed==0);
    CHECK(swap_six_request_generator(&s,1)); /* reflection is order 2 */
    CHECK(swap_six_request_generator(&s,1));
    swap_six_tick(&s,2.0);
    CHECK(same(s.current,initial) && s.completed==2);
    CHECK(swap_six_select_group(&s,SWAP_SIX_SYMMETRIC));
    CHECK(swap_six_request_generator(&s,1)); /* 1/2 swap is order 2 */
    CHECK(swap_six_request_generator(&s,1));
    swap_six_tick(&s,2.0);
    CHECK(same(s.current,initial) && s.completed==2);
    CHECK(swap_six_request_generator(&s,0));
    swap_six_tick(&s,-3.0); CHECK(s.queued==1);
    swap_six_tick(&s,NAN); CHECK(s.queued==1);
    swap_six_reset(&s); CHECK(s.queued==0 && !s.moving);
}
static void bad_input(void) {
    SwapSix s; swap_six_init(&s);
    SwapSixPermutation repeated={{0,0,2,3,4,5}};
    SwapSixPermutation out_of_range={{0,1,2,3,4,6}};
    CHECK(!swap_six_permutation_valid(repeated));
    CHECK(!swap_six_permutation_valid(out_of_range));
    CHECK(!swap_six_enqueue(&s,repeated));
    CHECK(!swap_six_enqueue(&s,out_of_range));
    CHECK(!swap_six_select_group(&s,3));
    CHECK(!swap_six_request_generator(&s,2));
    CHECK(!swap_six_position(&s,6,NULL));
    CHECK(!swap_six_position(&s,6,&(SwapSixPoint){0,0}));
    for (unsigned i=0;i<SWAP_SIX_QUEUE_CAP;++i)
        CHECK(swap_six_request_generator(&s,0));
    CHECK(s.queued==SWAP_SIX_QUEUE_CAP);
    CHECK(!swap_six_request_generator(&s,0));
    CHECK(!swap_six_request_random_nonidentity(&s));
    swap_six_tick(&s,1.3);
    CHECK(s.completed==2 && s.queued==SWAP_SIX_QUEUE_CAP-2);
}
/* Aggregate software-raster comparison, using the SAME geometric positions
   that the Sokol scene consumes. It is not a GPU scanout or a phone result.
   Two changing colored discs must change an appreciable RESULT AREA, not
   just a status string, chosen pixel or synthetic change counter. */
enum { SIX_SAMPLE_SIDE = 260 };
static void rasterize_six(const SwapSix *state,uint8_t *rgb) {
    static const uint8_t colours[6][3] = {
        {245,92,110},{71,191,232},{250,199,79},
        {194,138,252},{122,224,148},{255,145,69}
    };
    SwapSixPoint positions[6];
    for(unsigned id=0;id<6;++id)
        CHECK(swap_six_position(state,id,&positions[id]));
    for(unsigned y=0;y<SIX_SAMPLE_SIDE;++y)
        for(unsigned x=0;x<SIX_SAMPLE_SIDE;++x) {
            unsigned at=(y×SIX_SAMPLE_SIDE+x)×3;
            rgb[at]=13; rgb[at+1]=16; rgb[at+2]=24;
            float canvas_x=50.0f+(float)x;
            float canvas_y=872.0f+(float)y;
            for(unsigned id=0;id<6;++id) {
                float dx=canvas_x-(180.0f+110.0f×positions[id].x);
                float dy=canvas_y-(1002.0f+110.0f×positions[id].y);
                if(dx×dx+dy×dy<=18.0f×18.0f) {
                    rgb[at]=colours[id][0];
                    rgb[at+1]=colours[id][1];
                    rgb[at+2]=colours[id][2];
                }
            }
        }
}
static void aggregate_result_changes(void) {
    uint8_t before[SIX_SAMPLE_SIDE×SIX_SAMPLE_SIDE×3];
    uint8_t after[SIX_SAMPLE_SIDE×SIX_SAMPLE_SIDE×3];
    SwapSix state;
    swap_six_init(&state);
    rasterize_six(&state,before);
    rasterize_six(&state,after);
    CHECK(memcmp(before,after,sizeof(before))==0); /* fixed point */
    for(unsigned group=0;group<SWAP_SIX_GROUP_COUNT;++group)
        for(unsigned generator=0;generator<2;++generator) {
            CHECK(swap_six_select_group(&state,group));
            CHECK(swap_six_request_generator(&state,generator));
            swap_six_tick(&state,1.0);
            rasterize_six(&state,after);
            unsigned changed=0;
            double sum_abs=0.0;
            for(unsigned pixel=0;pixel<SIX_SAMPLE_SIDE×SIX_SAMPLE_SIDE;++pixel) {
                unsigned base=pixel×3;
                bool differs=false;
                for(unsigned channel=0;channel<3;++channel) {
                    int d=(int)before[base+channel]-(int)after[base+channel];
                    if(d) differs=true;
                    sum_abs+=(double)abs(d);
                }
                if(differs) ++changed;
            }
            double fraction=(double)changed÷(SIX_SAMPLE_SIDE×SIX_SAMPLE_SIDE);
            double mean=sum_abs÷(3×SIX_SAMPLE_SIDE×SIX_SAMPLE_SIDE);
            CHECK(fraction>0.020); /* at least 2% of the 260x260 result region */
            CHECK(mean>1.0); /* RGB8 aggregate channel difference */
            printf("Six-raster: group %u generator %u / %u pixels / mean RGB8 %.2f\n",
                   group,generator,changed,mean);
        }
}
int main(void) {
    finite_group(SWAP_SIX_CYCLIC,6);
    finite_group(SWAP_SIX_DIHEDRAL,12);
    finite_group(SWAP_SIX_SYMMETRIC,720);
    for (unsigned group=0;group<SWAP_SIX_GROUP_COUNT;++group)
        random_membership(group);
    animation_and_queue();
    aggregate_result_changes();
    bad_input();
    puts("Swap six disks: PASS (C6/D6/S6 closure, transitivity, interpolation, queue)");
    return 0;
}
