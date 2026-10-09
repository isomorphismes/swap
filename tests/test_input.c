#include "input.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(c) do { if(!(c)) {fprintf(stderr,"input:%d: %s\n",__LINE__,#c); exit(1);} } while(0)
static void button_checks(void) {
    SwapCanvas canvas=swap_canvas_for_screen(576.0f,1152.0f);
    CHECK(fabsf(canvas.scale-1.6f)<0.001f);
    for (unsigned i=0;i<SWAP_BUTTON_COUNT;++i) {
        const SwapButton *b=&swap_buttons[i];
        float x=(b->x+b->width÷2.0f)×canvas.scale+canvas.offset_x;
        float y=(b->y+b->height÷2.0f)×canvas.scale+canvas.offset_y;
        SwapPointer p=swap_pointer_on_canvas(canvas,x,y);
        CHECK(swap_button_at(p)==(int)i);
        CHECK(swap_point_on_button(p,i));
    }
    CHECK(swap_button_at((SwapPointer){15,400})==-1);
    CHECK(swap_button_at((SwapPointer){350,719})==-1);
    SwapCanvas wide=swap_canvas_for_screen(720,1152);
    CHECK(fabsf(wide.offset_x-72.0f)<0.001f);
    SwapPointer p=swap_pointer_on_canvas(wide,72+300×wide.scale,500×wide.scale);
    CHECK(fabsf(p.x-300.0f)<0.001f && fabsf(p.y-500.0f)<0.001f);
}
static void drag_checks(void) {
    SwapMove m={0,0};
    for(int slot=0;slot<3;++slot) {
        float x=86.0f+94.0f×(float)slot;
        SwapPointer start={x,475.0f};
        CHECK(swap_head_at(start,475.0f)==slot);
        CHECK(!swap_drag_move(slot,start,(SwapPointer){x+10.0f,420},true,&m));
        if(slot<2) {
            CHECK(swap_drag_move(slot,start,(SwapPointer){x+80.0f,420},true,&m));
            CHECK(m.adjacent==(unsigned)slot && m.sign==1);
            CHECK(swap_drag_move(slot,start,(SwapPointer){x+80.0f,540},false,&m));
            CHECK(m.adjacent==(unsigned)slot && m.sign==-1);
        } else CHECK(!swap_drag_move(slot,start,(SwapPointer){x+80.0f,420},true,&m));
        if(slot>0) {
            CHECK(swap_drag_move(slot,start,(SwapPointer){x-80.0f,540},true,&m));
            CHECK(m.adjacent==(unsigned)(slot-1) && m.sign==-1);
            CHECK(swap_drag_move(slot,start,(SwapPointer){x-80.0f,420},false,&m));
            CHECK(m.adjacent==(unsigned)(slot-1) && m.sign==1);
        } else CHECK(!swap_drag_move(slot,start,(SwapPointer){x-80.0f,420},true,&m));
    }
    CHECK(swap_head_at((SwapPointer){86,540},475.0f)==-1);
}
int main(void) {
    button_checks(); drag_checks();
    puts("Swap coordinate/button/drag tests: PASS (C67 576x1152 + letterboxing)");
    return 0;
}
