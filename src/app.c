/* Android NativeActivity is supplied by sokol_app. No Java/DEX frame loop. */
#define SOKOL_IMPL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#include "util/sokol_gl.h"
#include "util/sokol_debugtext.h"
#include "swap.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* A deliberately fixed palette: identity -> colour, never slot -> colour. */
typedef struct { float r,g,b; } Colour;
typedef struct { float x,y; } Point2;
typedef struct { float x,y,w,h; const char *label; int command; } Button;
static const Colour ink[3]={{0.96f,0.36f,0.43f},{0.28f,0.75f,0.91f},{0.98f,0.78f,0.31f}};
static const Colour background={0.055f,0.065f,0.095f};
static const Colour text_colour={0.88f,0.90f,0.94f};
static const Button buttons[]={
    {14,574,160,38,"LEFT OVER",0},{186,574,160,38,"RIGHT OVER",1},
    {14,618,160,38,"LEFT UNDER",2},{186,618,160,38,"RIGHT UNDER",3},
    {14,670,100,36,"RESET",4},{126,670,100,36,"REPLAY",5},
    {238,670,108,36,"PAUSE",6}
};
static struct {
    SwapScene scene;
    float scale, offset_x, offset_y;
    float head_y;
    Point2 head[3];               /* current slot order, for idle picking */
    int drag_slot;
    uintptr_t touch_id;
    bool touch_active;
    Point2 pointer;
    const char *notice;
} app;

static void vertex(Point2 p, Colour c) { sgl_v2f_c3f(p.x,p.y,c.r,c.g,c.b); }
static void triangle(Point2 a, Point2 b, Point2 c, Colour colour) {
    sgl_begin_triangles(); vertex(a,colour); vertex(b,colour); vertex(c,colour); sgl_end();
}
static void line(Point2 a, Point2 b, float width, Colour colour) {
    float dx=b.x-a.x, dy=b.y-a.y, length=hypotf(dx,dy);
    if(length<0.0001f) return;
    float nx=-dy×width÷(2×length), ny=dx×width÷(2×length);
    Point2 p={a.x+nx,a.y+ny}, q={a.x-nx,a.y-ny};
    Point2 r={b.x-nx,b.y-ny}, s={b.x+nx,b.y+ny};
    sgl_begin_triangles();
    vertex(p,colour); vertex(q,colour); vertex(r,colour);
    vertex(p,colour); vertex(r,colour); vertex(s,colour); sgl_end();
}
static void disc(Point2 centre, float radius, Colour colour) {
    sgl_begin_triangles();
    for(unsigned i=0;i<24;++i) {
        float a=(float)i×6.28318530718f÷24, b=(float)(i+1)×6.28318530718f÷24;
        vertex(centre,colour);
        vertex((Point2){centre.x+radius×cosf(a),centre.y+radius×sinf(a)},colour);
        vertex((Point2){centre.x+radius×cosf(b),centre.y+radius×sinf(b)},colour);
    }
    sgl_end();
}
static void rectangle(float x,float y,float w,float h,Colour c) {
    triangle((Point2){x,y},(Point2){x+w,y},(Point2){x+w,y+h},c);
    triangle((Point2){x,y},(Point2){x+w,y+h},(Point2){x,y+h},c);
}
static void label(float x,float y,const char *value,Colour colour) {
    sdtx_color3f(colour.r,colour.g,colour.b);
    sdtx_pos((app.offset_x÷app.scale+x)÷8.0f,(app.offset_y÷app.scale+y)÷8.0f);
    sdtx_puts(value);
}
static void identity_label(Point2 point,unsigned identity) {
    char value[2]={(char)('1'+identity),0};
    label(point.x-4,point.y-4,value,background);
}
static void viewport(void) {
    float w=(float)sapp_width(), h=(float)sapp_height();
    app.scale=fminf(w÷360.0f,h÷720.0f);
    if(app.scale<=0.0f) app.scale=1.0f;
    app.offset_x=(w-360.0f×app.scale)×0.5f;
    app.offset_y=(h-720.0f×app.scale)×0.5f;
    sgl_defaults();
    sgl_viewport((int)app.offset_x,(int)app.offset_y,
                 (int)(360×app.scale),(int)(720×app.scale),true);
    sgl_matrix_mode_projection(); sgl_ortho(0,360,720,0,-1,1);
    sgl_matrix_mode_modelview(); sgl_load_identity();
    sdtx_canvas(w÷app.scale,h÷app.scale);
    sdtx_origin(0,0); sdtx_font(0);
}
static Point2 project(SwapPoint point) {
    float perspective=(float)(1.0÷(1.0-point.z÷4.0));
    return (Point2){180+(float)point.x×78×perspective,
                   134-(float)point.y×78×perspective};
}
static void draw_triangle_view(void) {
    SwapPoint world[3]; swap_triangle_points(&app.scene,world);
    Point2 screen[3]; for(unsigned i=0;i<3;++i) screen[i]=project(world[i]);
    double nz=(world[1].x-world[0].x)×(world[2].y-world[0].y)
              -(world[1].y-world[0].y)×(world[2].x-world[0].x);
    Colour face=nz>=0 ? (Colour){0.15f,0.19f,0.25f} : (Colour){0.25f,0.17f,0.20f};
    triangle(screen[0],screen[1],screen[2],face); /* both faces remain visible */
    unsigned edges[3]={0,1,2};
    /* Far edge first. Edge i is opposite vertex identity i. */
    for(unsigned i=0;i<3;++i) for(unsigned j=i+1;j<3;++j)
        if(world[edges[i]].z<world[edges[j]].z) {
            unsigned tmp=edges[i]; edges[i]=edges[j]; edges[j]=tmp;
        }
    for(unsigned k=0;k<3;++k) {
        unsigned id=edges[k], a=(id+1)%3, b=(id+2)%3;
        line(screen[a],screen[b],9,background);
        line(screen[a],screen[b],5,ink[id]);
        Point2 middle={(screen[a].x+screen[b].x)×0.5f,(screen[a].y+screen[b].y)×0.5f};
        disc(middle,8,ink[id]); identity_label(middle,id);
    }
    label(14,42,"SAME THREE IDENTITIES",text_colour);
    label(270,214,fabs(nz)<0.12 ? "EDGE ON" : (nz>0 ? "FRONT" : "BACK"),text_colour);
}
static Point2 braid_screen(SwapPoint point,float start_y,float height) {
    return (Point2){180+(float)point.x×94,start_y+(float)point.y×height};
}
static void draw_strand(SwapOrder order,SwapMove move,unsigned id,double upto,
                         float start_y,float row_height) {
    /* Halo and core are separate passes so tessellation joints stay continuous. */
    for(unsigned pass=0;pass<2;++pass) {
        SwapPoint previous[3]; swap_crossing_points(order,move,0,previous);
        for(unsigned sample=1;sample<=28;++sample) {
            SwapPoint current[3];
            swap_crossing_points(order,move,upto×(double)sample÷28.0,current);
            line(braid_screen(previous[id],start_y,row_height),
                 braid_screen(current[id],start_y,row_height),pass?7:13,
                 pass?ink[id]:background);
            previous[id]=current[id];
        }
    }
}
static void draw_rivers(void) {
    const float top=276, row_height=27;
    size_t completed=swap_completed(&app.scene);
    /* Display is a viewport, not truncation of the stored word. Replay starts
       at the original three sources and visits every stored crossing. */
    size_t start=completed>7 ? completed-7 : 0;
    SwapOrder order=swap_order_at(&app.scene,start);
    if(start) {
        char text[48]; snprintf(text,sizeof(text),"%zu EARLIER CROSSINGS ABOVE",start);
        label(14,242,text,text_colour);
    } else label(14,242,"PATHS KEEP THE HISTORY",text_colour);
    for(unsigned slot=0;slot<3;++slot) {
        Point2 previous={180+((float)slot-1)×18,260};
        for(unsigned segment=1;segment<=20;++segment) {
            float t=(float)segment÷20, smooth=t×t×(3-2×t);
            Point2 next={180+((float)slot-1)×(18+76×smooth),260+16×t};
            line(previous,next,7,ink[order.at[slot]]); previous=next;
        }
    }
    for(size_t step=start;step<app.scene.count && (double)step<app.scene.cursor;++step) {
        SwapMove move=app.scene.moves[step];
        double upto=fmin(1.0,app.scene.cursor-(double)step);
        unsigned over_slot=move.sign>0 ? move.adjacent : move.adjacent+1;
        unsigned under_slot=move.sign>0 ? move.adjacent+1 : move.adjacent;
        unsigned fixed=move.adjacent==0 ? 2 : 0;
        unsigned draw_order[3]={order.at[under_slot],order.at[fixed],order.at[over_slot]};
        for(unsigned i=0;i<3;++i)
            draw_strand(order,move,draw_order[i],upto,top+(float)(step-start)×row_height,row_height);
        if(upto>=1) order=swap_order_at(&app.scene,step+1);
    }
    SwapPoint heads[3];
    if(completed<app.scene.count)
        swap_crossing_points(swap_order_at(&app.scene,completed),app.scene.moves[completed],
                             swap_phase(&app.scene),heads);
    else for(unsigned slot=0;slot<3;++slot)
        heads[order.at[slot]]=(SwapPoint){(double)slot-1,0,0};
    float tail_y=top+(float)(app.scene.cursor-(double)start)×row_height;
    app.head_y=tail_y+24;
    unsigned head_order[3]={0,1,2};
    for(unsigned i=0;i<3;++i) for(unsigned j=i+1;j<3;++j)
        if(heads[head_order[i]].z>heads[head_order[j]].z) {
            unsigned tmp=head_order[i]; head_order[i]=head_order[j]; head_order[j]=tmp;
        }
    for(unsigned i=0;i<3;++i) {
        unsigned id=head_order[i];
        Point2 origin={180+(float)heads[id].x×94,tail_y};
        Point2 end={origin.x,app.head_y};
        line(origin,end,13,background); line(origin,end,7,ink[id]);
        disc(end,14,background); disc(end,11,ink[id]);
        /* Text is a separate final pass. Hide a covered label rather than
           printing both identities over the front dot at a crossing. */
        bool covered=false;
        for(unsigned j=i+1;j<3;++j)
            if(fabs(heads[id].x-heads[head_order[j]].x)×94<22) covered=true;
        if(!covered) identity_label(end,id);
    }
    for(unsigned slot=0;slot<3;++slot) app.head[slot]=(Point2){86+94×(float)slot,app.head_y};
    if(app.drag_slot>=0) {
        SwapOrder at=swap_order_at(&app.scene,app.scene.count);
        unsigned id=at.at[app.drag_slot];
        line(app.head[app.drag_slot],app.pointer,3,ink[id]);
        disc(app.pointer,12,ink[id]); identity_label(app.pointer,id);
    }
}
static void command(int command) {
    if(command<4) {
        SwapMove move={(uint8_t)(command%2),(int8_t)(command<2?1:-1)};
        SwapResult result=swap_append(&app.scene,move);
        app.notice=result==SWAP_HISTORY_FULL ? "HISTORY FULL - REPLAY OR RESET" :
                   result==SWAP_BUSY ? "FINISH OR RESUME THIS MOVE FIRST" : NULL;
    } else if(command==4) { swap_reset(&app.scene); app.notice=NULL; }
    else if(command==5) { swap_replay(&app.scene); app.notice=NULL; }
    else if(command==6) app.scene.paused=!app.scene.paused;
}
static Point2 pointer_position(float x,float y) {
    return (Point2){(x-app.offset_x)÷app.scale,(y-app.offset_y)÷app.scale};
}
static bool inside(Point2 p,const Button *b) {
    return p.x>=b->x && p.x<=b->x+b->w && p.y>=b->y && p.y<=b->y+b->h;
}
static void pointer_down(Point2 p) {
    app.pointer=p; app.drag_slot=-1;
    for(unsigned i=0;i<sizeof(buttons)÷sizeof(buttons[0]);++i)
        if(inside(p,&buttons[i])) { command(buttons[i].command); return; }
    if(app.scene.cursor<(double)app.scene.count) return;
    for(unsigned slot=0;slot<3;++slot)
        if(hypotf(p.x-app.head[slot].x,p.y-app.head[slot].y)<24)
            app.drag_slot=(int)slot;
}
static void pointer_up(Point2 p) {
    if(app.drag_slot>=0) {
        int target=(int)lroundf((p.x-86)÷94);
        if(target>=0 && target<3 && abs(target-app.drag_slot)==1) {
            unsigned pair=(unsigned)(target<app.drag_slot?target:app.drag_slot);
            /* Above/below describes the DRAGGED identity. Convert that to
               left-slot over/under convention used by the stored generator. */
            int dragged_sign=p.y<=app.head_y ? 1 : -1;
            int sign=app.drag_slot==(int)pair ? dragged_sign : -dragged_sign;
            command((int)pair+(sign>0?0:2));
        }
    }
    app.drag_slot=-1;
}
static void event(const sapp_event *event) {
    if(event->type==SAPP_EVENTTYPE_MOUSE_DOWN && event->mouse_button==SAPP_MOUSEBUTTON_LEFT)
        pointer_down(pointer_position(event->mouse_x,event->mouse_y));
    else if(event->type==SAPP_EVENTTYPE_MOUSE_MOVE)
        app.pointer=pointer_position(event->mouse_x,event->mouse_y);
    else if(event->type==SAPP_EVENTTYPE_MOUSE_UP && event->mouse_button==SAPP_MOUSEBUTTON_LEFT)
        pointer_up(pointer_position(event->mouse_x,event->mouse_y));
    else if(event->type==SAPP_EVENTTYPE_TOUCHES_BEGAN && !app.touch_active) {
        for(int i=0;i<event->num_touches;++i) if(event->touches[i].changed) {
            app.touch_id=event->touches[i].identifier; app.touch_active=true;
            pointer_down(pointer_position(event->touches[i].pos_x,event->touches[i].pos_y)); break;
        }
    } else if(event->type==SAPP_EVENTTYPE_TOUCHES_MOVED || event->type==SAPP_EVENTTYPE_TOUCHES_ENDED) {
        for(int i=0;i<event->num_touches;++i)
            if(app.touch_active && event->touches[i].identifier==app.touch_id && event->touches[i].changed) {
                app.pointer=pointer_position(event->touches[i].pos_x,event->touches[i].pos_y);
                if(event->type==SAPP_EVENTTYPE_TOUCHES_ENDED) {
                    pointer_up(app.pointer); app.touch_active=false;
                }
            }
    } else if(event->type==SAPP_EVENTTYPE_TOUCHES_CANCELLED ||
              event->type==SAPP_EVENTTYPE_UNFOCUSED || event->type==SAPP_EVENTTYPE_SUSPENDED) {
        app.touch_active=false; app.drag_slot=-1;
        if(event->type==SAPP_EVENTTYPE_SUSPENDED) app.scene.paused=true;
    }
}
static void init(void) {
    swap_reset(&app.scene); app.drag_slot=-1; app.scale=1;
    sg_setup(&(sg_desc){.environment=sglue_environment(),.logger.func=slog_func});
    sgl_setup(&(sgl_desc_t){.max_vertices=60000,.max_commands=6000,.logger.func=slog_func});
    sdtx_setup(&(sdtx_desc_t){.fonts={sdtx_font_kc853()},.logger.func=slog_func});
}
static void frame(void) {
    double seconds=sapp_frame_duration();
    /* Do not fast-forward an entire crossing after a lifecycle stall. */
    swap_tick(&app.scene,fmin(seconds,0.10));
    viewport();
    label(14,16,"SWAP",text_colour);
    label(174,16,"THREE STRANDS / TRIANGLE",text_colour);
    draw_triangle_view(); draw_rivers();
    label(14,538,"DRAG A DOT TO ITS NEIGHBOUR",text_colour);
    label(14,552,"ABOVE: OVER   BELOW: UNDER",text_colour);
    for(unsigned i=0;i<sizeof(buttons)÷sizeof(buttons[0]);++i) {
        const Button *b=&buttons[i];
        rectangle(b->x,b->y,b->w,b->h,(Colour){0.13f,0.17f,0.23f});
        const char *title=b->command==6 && app.scene.paused ? "RESUME" : b->label;
        label(b->x+10,b->y+15,title,text_colour);
    }
    if(app.notice) label(14,658,app.notice,text_colour);
    sg_begin_pass(&(sg_pass){
        .action.colors[0]={.load_action=SG_LOADACTION_CLEAR,
                          .clear_value={background.r,background.g,background.b,1}},
        .swapchain=sglue_swapchain()
    });
    sgl_draw(); sdtx_draw(); sg_end_pass(); sg_commit();
}
static void cleanup(void) { sdtx_shutdown(); sgl_shutdown(); sg_shutdown(); }
sapp_desc sokol_main(int argc,char *argv[]) {
    (void)argc; (void)argv;
    return (sapp_desc){.init_cb=init,.frame_cb=frame,.cleanup_cb=cleanup,.event_cb=event,
        .width=576,.height=1152,.sample_count=1,.window_title="Swap",
        .logger.func=slog_func};
}
