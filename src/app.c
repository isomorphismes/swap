/* Android NativeActivity is supplied by sokol_app. No Java/DEX frame loop. */
#define SOKOL_IMPL
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#include "util/sokol_gl.h"
#include "util/sokol_debugtext.h"
#include "swap.h"
#include "input.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* A deliberately fixed palette: identity -> colour, never slot -> colour. */
typedef struct { float r,g,b; } Colour;
typedef SwapPointer Point2;
static const Colour ink[3]={{0.96f,0.36f,0.43f},{0.28f,0.75f,0.91f},{0.98f,0.78f,0.31f}};
static const Colour background={0.055f,0.065f,0.095f};
static const Colour text_colour={0.88f,0.90f,0.94f};
static struct {
    SwapScene scene;
    float scale, offset_x, offset_y;
    float head_y;
    Point2 head[3];               /* current slot order, for idle picking */
    int drag_slot, pressed_button;
    bool dragged_over;           /* explicit, visible choice for dragged identity */
    Point2 press_origin;
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
    SwapCanvas canvas=swap_canvas_for_screen(w,h);
    app.scale=canvas.scale;
    app.offset_x=canvas.offset_x;
    app.offset_y=canvas.offset_y;
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
    const float top=276, row_height=25;
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
        line(app.press_origin,app.pointer,3,ink[id]);
        disc(app.pointer,12,ink[id]); identity_label(app.pointer,id);
        SwapMove preview;
        if(swap_drag_move(app.drag_slot,app.press_origin,app.pointer,
                          app.dragged_over,&preview)) {
            char text[52];
            snprintf(text,sizeof(text),"STRAND %u %s / PAIR %u",
                     id+1,app.dragged_over?"OVER":"UNDER",
                     (unsigned)preview.adjacent+1);
            label(14,226,text,text_colour);
        }
    }
}
static void command(int command) {
    if(command<4) {
        SwapMove move={(uint8_t)(command%2),(int8_t)(command<2?1:-1)};
        SwapResult result=swap_append(&app.scene,move);
        app.notice=result==SWAP_HISTORY_FULL ? "HISTORY FULL - RESET" :
                   result==SWAP_BUSY ? "CROSSING BUSY" :
                   result==SWAP_ACCEPTED ? "CROSSING ADDED" : "INVALID MOVE";
    } else if(command==SWAP_RESET) {
        swap_reset(&app.scene); app.notice="RESET";
    } else if(command==SWAP_REPLAY) {
        swap_replay(&app.scene); app.notice="REPLAY";
    } else if(command==SWAP_PAUSE) {
        app.scene.paused=!app.scene.paused;
        app.notice=app.scene.paused?"PAUSED":"RESUMED";
    } else if(command==SWAP_DRAG_OVER || command==SWAP_DRAG_UNDER) {
        app.dragged_over=command==SWAP_DRAG_OVER;
        app.notice=app.dragged_over ? "DRAGGED STRAND GOES OVER" :
                                     "DRAGGED STRAND GOES UNDER";
    }
}
static Point2 pointer_position(float x,float y) {
    SwapCanvas canvas={app.scale,app.offset_x,app.offset_y};
    return swap_pointer_on_canvas(canvas,x,y);
}
static void pointer_down(Point2 p) {
    app.pointer=p; app.drag_slot=-1; app.pressed_button=-1;
    int button=swap_button_at(p);
    if(button>=0) {
        app.pressed_button=button;
        return;
    }
    app.drag_slot=swap_head_at(p,app.head_y);
    if(app.drag_slot>=0) {
        app.press_origin=app.head[app.drag_slot];
    } else {
        app.notice="TOUCH A COLOURED DOT OR CONTROL";
    }
}
static void pointer_up(Point2 p) {
    app.pointer=p;
    if(app.pressed_button>=0) {
        if(swap_point_on_button(p,(unsigned)app.pressed_button)) {
            command(swap_buttons[app.pressed_button].command);
        } else app.notice="BUTTON CANCELLED";
    } else if(app.drag_slot>=0) {
        SwapMove move;
        if(swap_drag_move(app.drag_slot,app.press_origin,p,app.dragged_over,&move)) {
            SwapResult result=swap_append(&app.scene,move);
            app.notice=result==SWAP_ACCEPTED ? "DRAG CROSSING ADDED" :
                       result==SWAP_HISTORY_FULL ? "HISTORY FULL - RESET" :
                       result==SWAP_BUSY ? "CROSSING BUSY" : "INVALID DRAG";
        } else app.notice="DRAG LEFT OR RIGHT TO A NEIGHBOUR";
    }
    app.drag_slot=-1; app.pressed_button=-1;
}
static void event(const sapp_event *event) {
    if(event->type==SAPP_EVENTTYPE_MOUSE_DOWN &&
       event->mouse_button==SAPP_MOUSEBUTTON_LEFT && !app.touch_active) {
        pointer_down(pointer_position(event->mouse_x,event->mouse_y));
    } else if(event->type==SAPP_EVENTTYPE_MOUSE_MOVE && !app.touch_active) {
        app.pointer=pointer_position(event->mouse_x,event->mouse_y);
    } else if(event->type==SAPP_EVENTTYPE_MOUSE_UP &&
              event->mouse_button==SAPP_MOUSEBUTTON_LEFT && !app.touch_active) {
        pointer_up(pointer_position(event->mouse_x,event->mouse_y));
    } else if(event->type==SAPP_EVENTTYPE_TOUCHES_BEGAN && !app.touch_active) {
        for(int i=0;i<event->num_touches;++i) if(event->touches[i].changed) {
            app.touch_id=event->touches[i].identifier;
            app.touch_active=true;
            pointer_down(pointer_position(event->touches[i].pos_x,
                                          event->touches[i].pos_y));
            break;
        }
    } else if(event->type==SAPP_EVENTTYPE_TOUCHES_MOVED ||
              event->type==SAPP_EVENTTYPE_TOUCHES_ENDED) {
        for(int i=0;i<event->num_touches;++i)
            if(app.touch_active && event->touches[i].identifier==app.touch_id &&
               (event->touches[i].changed || event->type==SAPP_EVENTTYPE_TOUCHES_ENDED)) {
                app.pointer=pointer_position(event->touches[i].pos_x,
                                             event->touches[i].pos_y);
                if(event->type==SAPP_EVENTTYPE_TOUCHES_ENDED) {
                    pointer_up(app.pointer);
                    app.touch_active=false;
                }
                break;
            }
    } else if(event->type==SAPP_EVENTTYPE_TOUCHES_CANCELLED ||
              event->type==SAPP_EVENTTYPE_UNFOCUSED ||
              event->type==SAPP_EVENTTYPE_SUSPENDED) {
        app.touch_active=false; app.drag_slot=-1; app.pressed_button=-1;
        if(event->type==SAPP_EVENTTYPE_SUSPENDED) app.scene.paused=true;
    }
}
static void init(void) {
    swap_reset(&app.scene); app.drag_slot=-1; app.pressed_button=-1;
    app.dragged_over=true; app.scale=1;
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
    label(14,496,"DRAG HORIZONTALLY / CHOOSE DEPTH",text_colour);
    for(unsigned i=0;i<SWAP_BUTTON_COUNT;++i) {
        const SwapButton *b=&swap_buttons[i];
        bool selected=(b->command==SWAP_DRAG_OVER && app.dragged_over) ||
                      (b->command==SWAP_DRAG_UNDER && !app.dragged_over);
        bool pressed=app.pressed_button==(int)i;
        Colour fill=selected ? (Colour){0.28f,0.32f,0.39f} :
                    pressed ? (Colour){0.30f,0.26f,0.37f} :
                              (Colour){0.13f,0.17f,0.23f};
        rectangle(b->x,b->y,b->width,b->height,fill);
        const char *title=b->command==SWAP_PAUSE && app.scene.paused ?
                          "RESUME" : b->title;
        label(b->x+10,b->y+16,title,text_colour);
    }
    if(app.notice) label(14,229,app.notice,text_colour);
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
