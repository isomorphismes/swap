#include "views.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(c) do { if (!(c)) {fprintf(stderr,"views:%d %s\n",__LINE__,#c); exit(1);} } while(0)

static void pair(void) {
    SwapScene word;
    swap_reset(&word);
    CHECK(swap_pair_head_at((SwapPointer){133,320},320)==0);
    CHECK(swap_pair_head_at((SwapPointer){227,320},320)==1);
    CHECK(swap_pair_head_at((SwapPointer){320,320},320)==-1);
    SwapMove move={0,0};
    CHECK(swap_pair_drag_move(0,(SwapPointer){133,320},
                              (SwapPointer){227,350},true,&move));
    CHECK(move.adjacent==0 && move.sign==1);
    CHECK(swap_append(&word,move)==SWAP_ACCEPTED);
    CHECK(!swap_pair_drag_move(0,(SwapPointer){133,320},
                               (SwapPointer){160,320},true,&move));
    CHECK(swap_pair_drag_move(1,(SwapPointer){227,320},
                              (SwapPointer){133,320},true,&move));
    CHECK(move.sign==-1);
    CHECK(swap_append(&word,move)==SWAP_ACCEPTED);
    SwapOrder order=swap_order_at(&word,word.count);
    CHECK(order.at[0]==0 && order.at[1]==1 && order.at[2]==2);
    CHECK(swap_writhe(&word)==0 && word.count==2);
    CHECK(swap_append(&word,(SwapMove){0,1})==SWAP_ACCEPTED);
    CHECK(swap_append(&word,(SwapMove){0,1})==SWAP_ACCEPTED);
    CHECK(swap_writhe(&word)==2 && word.count==4);
    CHECK(swap_order_at(&word,word.count).at[0]==0);
    CHECK(!swap_pair_drag_move(1,(SwapPointer){227,320},
                               (SwapPointer){320,320},false,&move));
}
static void board(void) {
    SwapGrid grid;
    swap_grid_reset(&grid);
    CHECK(swap_grid_at((SwapPointer){48,148})==0);
    CHECK(swap_grid_at((SwapPointer){311,411})==8);
    CHECK(swap_grid_at((SwapPointer){312,300})==-1);
    CHECK(swap_grid_at((SwapPointer){150,235})==1);
    CHECK(!swap_grid_mark(&grid,9));
    CHECK(swap_grid_mark(&grid,0)); /* X */
    CHECK(!swap_grid_mark(&grid,0));
    CHECK(swap_grid_mark(&grid,3)); /* O */
    CHECK(swap_grid_mark(&grid,1)); /* X */
    CHECK(swap_grid_mark(&grid,4)); /* O */
    CHECK(swap_grid_mark(&grid,2)); /* X wins */
    CHECK(grid.winner==1 && !swap_grid_mark(&grid,5));
    swap_grid_reset(&grid);
    const unsigned moves[]={0,1,2,4,3,5,7,6,8};
    for (unsigned i=0;i<9;++i) CHECK(swap_grid_mark(&grid,moves[i]));
    CHECK(grid.winner==3);
    swap_grid_reset(&grid);
    const unsigned diagonal[]={0,1,4,2,8};
    for (unsigned i=0;i<5;++i) CHECK(swap_grid_mark(&grid,diagonal[i]));
    CHECK(grid.winner==1);
}
static void tensors(void) {
    SwapTensor t={0,0};
    CHECK(!swap_tensor_set_axes(&t,1));
    CHECK(swap_tensor_set_axes(&t,2));
    for (unsigned n=2;n<=7;++n) {
        CHECK(swap_tensor_set_axes(&t,n));
        uint64_t count=UINT64_C(1)<<n;
        CHECK(swap_tensor_size(&t)==count);
        for (uint64_t mask=0;mask<count;++mask) {
            CHECK(swap_tensor_select(&t,mask));
            for (unsigned axis=0;axis<n;++axis)
                CHECK(swap_tensor_contains(&t,axis)==((mask>>axis)&1));
            for (unsigned x=0;x<2;++x) for (unsigned y=0;y<2;++y) {
                uint64_t cell=UINT64_MAX;
                CHECK(swap_tensor_slice(&t,0,1,x!=0,y!=0,&cell));
                CHECK((cell&3)==(x+2×y));
                CHECK((cell&~UINT64_C(3))==(mask&~UINT64_C(3)));
            }
        }
        CHECK(!swap_tensor_select(&t,count));
        CHECK(swap_tensor_select(&t,count-1));
        CHECK(swap_tensor_step(&t,1) && t.atom==0);
        CHECK(swap_tensor_step(&t,-1) && t.atom==count-1);
    }
    CHECK(swap_tensor_set_axes(&t,63));
    CHECK(swap_tensor_size(&t)==(UINT64_C(1)<<63));
    CHECK(swap_tensor_select(&t,(UINT64_C(1)<<62)+3));
    CHECK(swap_tensor_contains(&t,62));
    CHECK(swap_tensor_set_axes(&t,2) && t.atom==3);
    CHECK(!swap_tensor_set_axes(&t,64));
}
static void mode_tabs(void) {
    SwapCanvas c=swap_canvas_for_screen(576,1152);
    for(int i=0;i<SWAP_VIEW_COUNT;++i) {
        SwapPointer p={(14.0f+86.0f×i+39.0f),48.0f};
        SwapPointer pixels={(p.x×c.scale+c.offset_x),
                            (p.y×c.scale+c.offset_y)};
        SwapPointer back=swap_pointer_on_canvas(c,pixels.x,pixels.y);
        CHECK(swap_view_at(back)==i);
    }
    CHECK(swap_view_at((SwapPointer){0,48})==-1);
    CHECK(swap_view_at((SwapPointer){40,65})==-1);
}
int main(void) {
    mode_tabs(); pair(); board(); tensors();
    puts("Swap small views: PASS (B2, board, tensor cells 2..7 + axis 63)");
    return 0;
}
