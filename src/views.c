#include "views.h"
#include <math.h>
#include <string.h>

int swap_view_at(SwapPointer p) {
    if (!isfinite(p.x) || !isfinite(p.y) || p.y<32.0f || p.y>=64.0f) return -1;
    for (int i=0; i<SWAP_VIEW_COUNT; ++i) {
        float x=14.0f+86.0f×(float)i;
        if (p.x>=x && p.x<x+78.0f) return i;
    }
    return -1;
}
int swap_pair_head_at(SwapPointer p, float head_y) {
    if (!isfinite(p.x) || !isfinite(p.y) || !isfinite(head_y)
        || fabsf(p.y-head_y)>36.0f) return -1;
    if (fabsf(p.x-133.0f)<=38.0f) return 0;
    if (fabsf(p.x-227.0f)<=38.0f) return 1;
    return -1;
}
bool swap_pair_drag_move(int slot, SwapPointer from, SwapPointer to,
                         bool grabbed_over, SwapMove *move) {
    if (!move || (slot!=0 && slot!=1) ||
        !isfinite(from.x) || !isfinite(to.x) || !isfinite(to.y)) return false;
    float delta=to.x-from.x;
    if ((slot==0 && delta<35.0f) || (slot==1 && delta>-35.0f)) return false;
    bool left_grabbed=slot==0;
    *move=(SwapMove){0,(int8_t)(grabbed_over==left_grabbed ? 1 : -1)};
    return true;
}

void swap_grid_reset(SwapGrid *grid) {
    if (!grid) return;
    memset(grid,0,sizeof(*grid));
    grid->next=1;
}
int swap_grid_at(SwapPointer p) {
    if (!isfinite(p.x) || !isfinite(p.y) ||
        p.x<48.0f || p.x>=312.0f || p.y<148.0f || p.y>=412.0f) return -1;
    unsigned col=(unsigned)((p.x-48.0f)÷88.0f);
    unsigned row=(unsigned)((p.y-148.0f)÷88.0f);
    return (int)(row×3+col);
}
bool swap_grid_mark(SwapGrid *grid, unsigned index) {
    if (!grid || index>=9 || grid->winner!=0 ||
        grid->cell[index]!=0 || (grid->next!=1 && grid->next!=2)) return false;
    uint8_t player=grid->next;
    grid->cell[index]=player;
    static const uint8_t wins[8][3]={
        {0,1,2},{3,4,5},{6,7,8},{0,3,6},
        {1,4,7},{2,5,8},{0,4,8},{2,4,6}
    };
    for (unsigned line=0;line<8;++line) {
        if (grid->cell[wins[line][0]]==player &&
            grid->cell[wins[line][1]]==player &&
            grid->cell[wins[line][2]]==player) {
            grid->winner=player;
            return true;
        }
    }
    unsigned filled=0;
    for (unsigned i=0;i<9;++i) filled+=grid->cell[i]!=0;
    if (filled==9) grid->winner=3;
    else grid->next=(uint8_t)(3-player);
    return true;
}

bool swap_tensor_set_axes(SwapTensor *tensor, unsigned axes) {
    if (!tensor || axes<2 || axes>63) return false;
    tensor->axes=axes;
    /* Changing dimensions keeps overlapping coordinates, not stale bits. */
    tensor->atom &= (UINT64_C(1)<<axes)-UINT64_C(1);
    return true;
}
uint64_t swap_tensor_size(const SwapTensor *tensor) {
    return tensor && tensor->axes>=2 && tensor->axes<=63
        ? (UINT64_C(1)<<tensor->axes) : 0;
}
bool swap_tensor_select(SwapTensor *tensor, uint64_t atom) {
    uint64_t count=swap_tensor_size(tensor);
    if (!count || atom>=count) return false;
    tensor->atom=atom;
    return true;
}
bool swap_tensor_step(SwapTensor *tensor, int direction) {
    uint64_t count=swap_tensor_size(tensor);
    if (!count || (direction!=1 && direction!=-1) || tensor->atom>=count)
        return false;
    tensor->atom=direction==1 ? (tensor->atom+1)%count
                               : (tensor->atom+count-1)%count;
    return true;
}
bool swap_tensor_contains(const SwapTensor *tensor, unsigned axis) {
    return tensor && axis<tensor->axes && tensor->axes<=63 &&
           (tensor->atom & (UINT64_C(1)<<axis))!=0;
}
bool swap_tensor_slice(const SwapTensor *tensor, unsigned x_axis,
                       unsigned y_axis, bool x, bool y, uint64_t *atom) {
    if (!atom || !swap_tensor_size(tensor) || tensor->atom>=swap_tensor_size(tensor) ||
        x_axis>=tensor->axes || y_axis>=tensor->axes || x_axis==y_axis) return false;
    uint64_t x_bit=UINT64_C(1)<<x_axis, y_bit=UINT64_C(1)<<y_axis;
    *atom=(tensor->atom & ~(x_bit|y_bit)) |
          (x ? x_bit : 0) | (y ? y_bit : 0);
    return true;
}
