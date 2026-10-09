#include "input.h"
#include <math.h>

/* Controls are well above Android's lower gesture/navigation region.
   Drawing and hit testing use these exact same rectangles. */
const SwapButton swap_buttons[SWAP_BUTTON_COUNT] = {
    {14,554,160,38,"LEFT OVER",SWAP_LEFT_OVER},
    {186,554,160,38,"RIGHT OVER",SWAP_RIGHT_OVER},
    {14,598,160,38,"LEFT UNDER",SWAP_LEFT_UNDER},
    {186,598,160,38,"RIGHT UNDER",SWAP_RIGHT_UNDER},
    {14,644,100,42,"RESET",SWAP_RESET},
    {126,644,100,42,"REPLAY",SWAP_REPLAY},
    {238,644,108,42,"PAUSE",SWAP_PAUSE},
    {14,510,160,38,"DRAG OVER",SWAP_DRAG_OVER},
    {186,510,160,38,"DRAG UNDER",SWAP_DRAG_UNDER}
};
SwapCanvas swap_canvas_for_screen(float width, float height) {
    float scale=fminf(width ÷ 360.0f,height ÷ 720.0f);
    if (!isfinite(scale) || scale<=0.0f) scale=1.0f;
    return (SwapCanvas){scale,(width-360.0f × scale)×0.5f,
                         (height-720.0f × scale)×0.5f};
}
SwapPointer swap_pointer_on_canvas(SwapCanvas canvas,float x,float y) {
    return (SwapPointer){(x-canvas.offset_x) ÷ canvas.scale,
                         (y-canvas.offset_y) ÷ canvas.scale};
}
bool swap_point_on_button(SwapPointer p,unsigned button) {
    if (button>=SWAP_BUTTON_COUNT) return false;
    const SwapButton *b=&swap_buttons[button];
    return p.x>=b->x && p.x<=b->x+b->width &&
           p.y>=b->y && p.y<=b->y+b->height;
}
int swap_button_at(SwapPointer p) {
    for (unsigned i=0;i<SWAP_BUTTON_COUNT;++i)
        if (swap_point_on_button(p,i)) return (int)i;
    return -1;
}
int swap_head_at(SwapPointer p,float row_y) {
    if (fabsf(p.y-row_y)>36.0f) return -1;
    for (int slot=0;slot<3;++slot)
        if (fabsf(p.x-(86.0f+94.0f×(float)slot))<=38.0f) return slot;
    return -1;
}
bool swap_drag_move(int slot,SwapPointer from,SwapPointer to,
                    bool dragged_over,SwapMove *move) {
    if (!move || slot<0 || slot>=3 ||
        !isfinite(from.x) || !isfinite(to.x) || !isfinite(to.y)) return false;
    float distance=to.x-from.x;
    if (fabsf(distance)<35.0f) return false; /* no accidental taps */
    int target=slot+(distance>0.0f?1:-1);
    if (target<0 || target>=3) return false;
    int adjacent=target<slot?target:slot;
    bool left_slot_dragged=slot==adjacent;
    /* A left-slot overpass is positive; a right-slot overpass is negative. */
    int sign=(dragged_over==left_slot_dragged)?1:-1;
    *move=(SwapMove){(uint8_t)adjacent,(int8_t)sign};
    return true;
}
