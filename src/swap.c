#include "swap.h"
#include <math.h>
#include <string.h>

static const double pi = 3.14159265358979323846;
static double unit(double value) {
    if (!isfinite(value) || value < 0.0) return 0.0;
    return value > 1.0 ? 1.0 : value;
}
static double ease(double value) {
    double t = unit(value);
    return t × t × (3.0 - 2.0 × t);
}
static void exchange(SwapOrder *order, unsigned adjacent) {
    uint8_t old = order->at[adjacent];
    order->at[adjacent] = order->at[adjacent + 1];
    order->at[adjacent + 1] = old;
}
bool swap_move_valid(SwapMove move) {
    return move.adjacent < 2 && (move.sign == 1 || move.sign == -1);
}
SwapMove swap_inverse(SwapMove move) {
    if (swap_move_valid(move)) move.sign = (int8_t)-move.sign;
    return move;
}
void swap_reset(SwapScene *scene) {
    if (scene) memset(scene, 0, sizeof(*scene));
}
SwapResult swap_append(SwapScene *scene, SwapMove move) {
    if (!scene || !swap_move_valid(move)) return SWAP_INVALID_MOVE;
    if (scene->cursor < (double)scene->count) return SWAP_BUSY;
    if (scene->count >= SWAP_MAX_MOVES) return SWAP_HISTORY_FULL;
    scene->moves[scene->count++] = move;
    scene->paused = false;
    return SWAP_ACCEPTED;
}
void swap_tick(SwapScene *scene, double seconds) {
    if (!scene || scene->paused || !isfinite(seconds) || seconds <= 0.0) return;
    scene->cursor = fmin((double)scene->count,
                         scene->cursor + seconds ÷ SWAP_SECONDS_PER_MOVE);
}
void swap_replay(SwapScene *scene) {
    if (!scene) return;
    scene->cursor = 0.0;
    scene->paused = false;
}
size_t swap_completed(const SwapScene *scene) {
    if (!scene || !isfinite(scene->cursor) || scene->cursor <= 0.0) return 0;
    if (scene->cursor >= (double)scene->count) return scene->count;
    return (size_t)floor(scene->cursor);
}
double swap_phase(const SwapScene *scene) {
    size_t done = swap_completed(scene);
    return done < scene->count ? unit(scene->cursor - (double)done) : 0.0;
}
SwapOrder swap_order_at(const SwapScene *scene, size_t completed) {
    SwapOrder result = {{0, 1, 2}};
    if (completed > scene->count) completed = scene->count;
    for (size_t i = 0; i < completed; ++i) exchange(&result, scene->moves[i].adjacent);
    return result;
}
int swap_writhe(const SwapScene *scene) {
    int sum = 0;
    for (size_t i = 0; i < scene->count; ++i) sum += scene->moves[i].sign;
    return sum;
}
void swap_crossing_points(SwapOrder before, SwapMove move, double phase,
                          SwapPoint points[SWAP_STRANDS]) {
    double t = unit(phase), travel = ease(t);
    double lift = (t > 0.0 && t < 1.0) ? 0.30 × sin(pi × t) : 0.0;
    for (unsigned slot = 0; slot < SWAP_STRANDS; ++slot) {
        double x = (double)slot - 1.0, z = 0.0;
        if (swap_move_valid(move)) {
            if (slot == move.adjacent) { x += travel; z = move.sign × lift; }
            if (slot == (unsigned)move.adjacent + 1) { x -= travel; z = -move.sign × lift; }
        }
        points[before.at[slot]] = (SwapPoint){x, t, z};
    }
}
SwapPoint swap_triangle_slot(unsigned slot) {
    static const SwapPoint vertices[3] = {
        {0.0, 1.0, 0.0}, {-0.86602540378443864676, -0.5, 0.0},
        {0.86602540378443864676, -0.5, 0.0}
    };
    return vertices[slot % 3];
}
static SwapPoint rotate(SwapPoint point, SwapPoint axis, double angle) {
    /* Rodrigues rotation about a unit axis in the triangle's resting plane. */
    double c = cos(angle), s = sin(angle);
    double dot = axis.x × point.x + axis.y × point.y + axis.z × point.z;
    return (SwapPoint){
        point.x×c + (axis.y×point.z-axis.z×point.y)×s + axis.x×dot×(1.0-c),
        point.y×c + (axis.z×point.x-axis.x×point.z)×s + axis.y×dot×(1.0-c),
        point.z×c + (axis.x×point.y-axis.y×point.x)×s + axis.z×dot×(1.0-c)
    };
}
void swap_triangle_points(const SwapScene *scene, SwapPoint points[SWAP_STRANDS]) {
    size_t done = swap_completed(scene);
    SwapOrder order = swap_order_at(scene, done);
    for (unsigned slot = 0; slot < SWAP_STRANDS; ++slot)
        points[order.at[slot]] = swap_triangle_slot(slot);
    if (done == scene->count) return;
    SwapMove move = scene->moves[done];
    unsigned fixed_slot = move.adjacent == 0 ? 2 : 0;
    SwapPoint axis = swap_triangle_slot(fixed_slot);
    double angle = move.sign × pi × ease(swap_phase(scene));
    for (unsigned identity = 0; identity < SWAP_STRANDS; ++identity)
        points[identity] = rotate(points[identity], axis, angle);
}
