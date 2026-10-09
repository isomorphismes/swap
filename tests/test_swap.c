#include "swap.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Checks remain live under NDEBUG/Release. */
#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); exit(1); \
} } while (0)
static int checks;
static void near(double a, double b) { CHECK(fabs(a-b) < 1e-9); ++checks; }
static double distance(SwapPoint a, SwapPoint b) {
    return sqrt((a.x-b.x)×(a.x-b.x)+(a.y-b.y)×(a.y-b.y)+(a.z-b.z)×(a.z-b.z));
}
static bool same(SwapOrder a, SwapOrder b) { return memcmp(a.at,b.at,3) == 0; }
static void finish(SwapScene *scene) { swap_tick(scene, 1000.0); }
static void add(SwapScene *scene, unsigned pair, int sign) {
    CHECK(swap_append(scene,(SwapMove){(uint8_t)pair,(int8_t)sign}) == SWAP_ACCEPTED);
    finish(scene);
}
static void identity_and_inverse(void) {
    SwapScene scene; swap_reset(&scene);
    SwapOrder original = {{0,1,2}};
    CHECK(same(swap_order_at(&scene,0), original));
    add(&scene,0,1); add(&scene,0,-1);
    CHECK(same(swap_order_at(&scene,scene.count),original));
    CHECK(scene.count == 2); CHECK(swap_writhe(&scene) == 0);
    swap_reset(&scene); add(&scene,0,1); add(&scene,0,1);
    CHECK(same(swap_order_at(&scene,scene.count),original));
    CHECK(scene.count == 2); CHECK(swap_writhe(&scene) == 2);
    /* Same endpoints do not erase the two positive crossings. */
    CHECK(scene.moves[0].sign == 1 && scene.moves[1].sign == 1);
    CHECK(swap_inverse((SwapMove){1,-1}).sign == 1);
}
static void endpoint_laws(void) {
    SwapScene left,right; swap_reset(&left); swap_reset(&right);
    add(&left,0,1); add(&left,1,1);
    add(&right,1,1); add(&right,0,1);
    CHECK(!same(swap_order_at(&left,2),swap_order_at(&right,2)));
    add(&left,0,1); add(&right,1,1);
    /* Compatibility of the endpoint map with the braid relation.
       This is NOT a test of a braid-isotopy/equality implementation. */
    CHECK(same(swap_order_at(&left,3),swap_order_at(&right,3)));
}
static void all_triangle_poses(void) {
    bool seen[27] = {false}; unsigned distinct = 0;
    /* All words up to length 4 cover all six resting permutations. */
    for (unsigned word=0; word<16; ++word) {
        SwapScene scene; swap_reset(&scene);
        for (unsigned depth=0; depth<=4; ++depth) {
            SwapOrder before=swap_order_at(&scene,scene.count);
            unsigned key=before.at[0]×9+before.at[1]×3+before.at[2];
            if (!seen[key]) { seen[key]=true; ++distinct; }
            SwapPoint resting[3]; swap_triangle_points(&scene,resting);
            for (unsigned slot=0; slot<3; ++slot)
                near(distance(resting[before.at[slot]],swap_triangle_slot(slot)),0.0);
            for (unsigned pair=0; pair<2; ++pair) for (int sign=-1; sign<=1; sign+=2) {
                SwapScene moving=scene;
                CHECK(swap_append(&moving,(SwapMove){(uint8_t)pair,(int8_t)sign})==SWAP_ACCEPTED);
                unsigned fixed = pair==0 ? 2 : 0;
                for (unsigned frame=0; frame<=40; ++frame) {
                    moving.cursor=(double)scene.count+(double)frame÷40.0;
                    SwapPoint triangle[3], strands[3]; swap_triangle_points(&moving,triangle);
                    swap_crossing_points(before,moving.moves[scene.count],(double)frame÷40.0,strands);
                    for (unsigned a=0; a<3; ++a) {
                        near(distance(triangle[a],(SwapPoint){0,0,0}),1.0);
                        for (unsigned b=a+1; b<3; ++b) {
                            near(distance(triangle[a],triangle[b]),sqrt(3.0));
                            CHECK(distance(strands[a],strands[b]) > 0.50);
                        }
                    }
                    near(distance(triangle[before.at[fixed]],swap_triangle_slot(fixed)),0.0);
                    if (frame==20) {
                        CHECK(fabs(triangle[before.at[pair]].z)>0.8);
                        CHECK(strands[before.at[pair]].z×sign > 0.29);
                    }
                }
                SwapOrder after=swap_order_at(&moving,moving.count);
                SwapPoint end[3]; swap_triangle_points(&moving,end);
                CHECK(after.at[pair]==before.at[pair+1]);
                for(unsigned slot=0;slot<3;++slot)
                    near(distance(end[after.at[slot]],swap_triangle_slot(slot)),0.0);
                /* Side identity i is the edge opposite vertex identity i.
                   Its midpoint must land opposite the corresponding slot. */
                for(unsigned slot=0;slot<3;++slot) {
                    unsigned id=after.at[slot], a=(id+1)%3, b=(id+2)%3;
                    SwapPoint midpoint={(end[a].x+end[b].x)÷2.0,(end[a].y+end[b].y)÷2.0,0};
                    SwapPoint expected=swap_triangle_slot(slot);
                    expected.x = expected.x × -0.5; expected.y = expected.y × -0.5;
                    near(distance(midpoint,expected),0.0);
                }
            }
            if(depth<4) add(&scene,(word>>depth)&1,1);
        }
    }
    CHECK(distinct==6);
}
static void lifecycle_and_limits(void) {
    SwapScene scene; swap_reset(&scene);
    SwapScene saved=scene;
    CHECK(swap_append(&scene,(SwapMove){2,1})==SWAP_INVALID_MOVE);
    CHECK(memcmp(&scene,&saved,sizeof(scene))==0);
    CHECK(swap_append(&scene,(SwapMove){0,0})==SWAP_INVALID_MOVE);
    CHECK(swap_append(&scene,(SwapMove){0,1})==SWAP_ACCEPTED);
    CHECK(swap_append(&scene,(SwapMove){1,1})==SWAP_ACCEPTED);
    CHECK(scene.count==2 && scene.cursor==0.0); /* queued, not discarded */
    swap_tick(&scene,NAN); swap_tick(&scene,INFINITY); swap_tick(&scene,-1);
    near(scene.cursor,0.0);
    scene.paused=true; swap_tick(&scene,100); near(scene.cursor,0.0);
    scene.paused=false;
    for(unsigned i=0;i<48;++i) swap_tick(&scene,1.0÷60.0);
    near(scene.cursor,1.0); finish(&scene);
    for(unsigned i=2;i<SWAP_MAX_MOVES;++i) add(&scene,i%2,(i%3)?1:-1);
    saved=scene;
    CHECK(swap_append(&scene,(SwapMove){0,1})==SWAP_HISTORY_FULL);
    CHECK(memcmp(&scene,&saved,sizeof(scene))==0);
    SwapOrder end=swap_order_at(&scene,scene.count);
    swap_replay(&scene); near(scene.cursor,0.0);
    CHECK(scene.count==SWAP_MAX_MOVES); finish(&scene);
    CHECK(same(swap_order_at(&scene,scene.count),end));
    CHECK(memcmp(scene.moves,saved.moves,sizeof(scene.moves))==0);
    swap_reset(&scene); CHECK(scene.count==0 && scene.cursor==0.0);
}
int main(void) {
    identity_and_inverse(); endpoint_laws(); all_triangle_poses(); lifecycle_and_limits();
    printf("Swap host model/geometry: PASS (%d numerical checks; no GPU/device claim)\n", checks);
    return 0;
}
