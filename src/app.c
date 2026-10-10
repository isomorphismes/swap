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
#include "views.h"
#include "permsix.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* A deliberately fixed palette: identity -> colour, never slot -> colour. */
typedef struct { float r,g,b; } Colour;
typedef SwapPointer Point2;
static const Colour ink[3]={{0.96f,0.36f,0.43f},{0.28f,0.75f,0.91f},{0.98f,0.78f,0.31f}};
static const Colour six_ink[6]={{0.96f,0.36f,0.43f},{0.28f,0.75f,0.91f},
    {0.98f,0.78f,0.31f},{0.76f,0.54f,0.99f},
    {0.48f,0.88f,0.58f},{1.00f,0.57f,0.27f}};
static const Colour background={0.055f,0.065f,0.095f};
static const Colour text_colour={0.88f,0.90f,0.94f};
static struct {
    SwapScene scene;
    SwapScene pair;
    SwapGrid grid;
    SwapTensor tensor;
    SwapSix six; /* a separate finite-group action, not a braid history */
    SwapView view;
    float scale, offset_x, offset_y;
    float head_y;
    Point2 head[3];               /* current slot order, for idle picking */
    Point2 pair_head[2];
    float pair_head_y;
    int drag_slot, pressed_button;
    int pressed_mode, pressed_cell, pressed_panel, pressed_six;
    float scroll_y, scroll_target; /* second 720px section is BELOW the first */
    Point2 down_origin;
    bool press_tracking, scroll_swipe;
    const char *six_notice;
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
    sdtx_pos((app.offset_x÷app.scale+x)÷8.0f,(app.offset_y÷app.scale+y-app.scroll_y)÷8.0f);
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
    sgl_matrix_mode_projection(); sgl_ortho(0,360,720+app.scroll_y,app.scroll_y,-1,1);
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
    label(14,80,"SAME THREE IDENTITIES",text_colour);
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

/* Distinct small views share canvas coordinates, not conflated semantics. */
static void outline_circle(Point2 centre, float radius, Colour c, float width) {
    for (unsigned i=0;i<64;++i) {
        float a=6.28318530718f×(float)i÷64.0f;
        float b=6.28318530718f×(float)(i+1)÷64.0f;
        line((Point2){centre.x+radius×cosf(a),centre.y+radius×sinf(a)},
             (Point2){centre.x+radius×cosf(b),centre.y+radius×sinf(b)},
             width,c);
    }
}
static void draw_mode_tabs(void) {
    static const char *names[SWAP_VIEW_COUNT]={"THREE","TWO","GRID","SETS"};
    for (int i=0;i<SWAP_VIEW_COUNT;++i) {
        float x=14+86×(float)i;
        Colour fill=app.view==i ? (Colour){0.28f,0.33f,0.39f} :
                    app.pressed_mode==i ? (Colour){0.29f,0.25f,0.32f} :
                     (Colour){0.13f,0.17f,0.23f};
        rectangle(x,32,78,32,fill);
        label(x+8,42,names[i],text_colour);
    }
}
static Point2 pair_project(SwapPoint p,float top,float height) {
    return (Point2){180+((float)p.x+0.5f)×94,top+(float)p.y×height};
}
static void pair_strand(SwapOrder order,SwapMove move,unsigned identity,
                        double upto,float top,float height) {
    for (unsigned pass=0;pass<2;++pass) {
        SwapPoint prev[SWAP_STRANDS],cur[SWAP_STRANDS];
        swap_crossing_points(order,move,0,prev);
        for (unsigned s=1;s<=24;++s) {
            swap_crossing_points(order,move,upto×(double)s÷24.0,cur);
            line(pair_project(prev[identity],top,height),
                 pair_project(cur[identity],top,height),pass?7:13,
                 pass?ink[identity]:background);
            for(unsigned k=0;k<SWAP_STRANDS;++k) prev[k]=cur[k];
        }
    }
}
static void draw_pair(void) {
    SwapScene *scene=&app.pair;
    size_t completed=swap_completed(scene);
    SwapOrder now=swap_order_at(scene,completed);
    SwapPoint points[SWAP_STRANDS];
    if(completed<scene->count)
        swap_crossing_points(now,scene->moves[completed],swap_phase(scene),points);
    else for (unsigned slot=0;slot<SWAP_STRANDS;++slot)
        points[now.at[slot]]=(SwapPoint){(double)slot-1,0,0};
    line((Point2){133,151},(Point2){227,151},4,text_colour);
    for(unsigned id=0;id<2;++id) {
        Point2 p=pair_project(points[id],151,24);
        disc(p,18,ink[id]); identity_label(p,id);
    }
    label(14,89,"ONE GENERATOR / TWO IDENTITIES",text_colour);
    char status[60];
    snprintf(status,sizeof(status),"B2: %zu MOVES / WRITHE %d",
             scene->count,swap_writhe(scene));
    label(14,209,status,text_colour);
    label(14,242,"TWO TRACKS / SIGNED HISTORY",text_colour);

    size_t start=completed>7?completed-7:0;
    SwapOrder order=swap_order_at(scene,start);
    if(start) {
        snprintf(status,sizeof(status),"%zu EARLIER CROSSINGS",start);
        label(14,263,status,text_colour);
    }
    for(unsigned slot=0;slot<2;++slot) {
        Point2 a={133+94×(float)slot,283},b={133+94×(float)slot,291};
        line(a,b,7,ink[order.at[slot]]);
    }
    const float top=291,row_height=25;
    for(size_t step=start;step<scene->count && (double)step<scene->cursor;++step) {
        SwapMove move=scene->moves[step];
        double upto=fmin(1.0,scene->cursor-(double)step);
        unsigned under=order.at[move.sign>0 ? 1 : 0];
        unsigned over=order.at[move.sign>0 ? 0 : 1];
        pair_strand(order,move,under,upto,top+(float)(step-start)×row_height,row_height);
        pair_strand(order,move,over,upto,top+(float)(step-start)×row_height,row_height);
        if(upto>=1.0) order=swap_order_at(scene,step+1);
    }
    SwapPoint heads[SWAP_STRANDS];
    if(completed<scene->count)
        swap_crossing_points(swap_order_at(scene,completed),
                             scene->moves[completed],swap_phase(scene),heads);
    else for(unsigned slot=0;slot<SWAP_STRANDS;++slot)
        heads[order.at[slot]]=(SwapPoint){(double)slot-1,0,0};
    float tail=top+(float)(scene->cursor-(double)start)×row_height;
    app.pair_head_y=tail+26;
    for(unsigned id=0;id<2;++id) {
        Point2 from={180+((float)heads[id].x+0.5f)×94,tail};
        Point2 to={from.x,app.pair_head_y};
        line(from,to,7,ink[id]); disc(to,12,ink[id]); identity_label(to,id);
    }
    for(unsigned slot=0;slot<2;++slot)
        app.pair_head[slot]=(Point2){133+94×(float)slot,app.pair_head_y};
    if(app.drag_slot>=0) {
        SwapOrder end=swap_order_at(scene,scene->count);
        unsigned id=end.at[app.drag_slot];
        line(app.press_origin,app.pointer,3,ink[id]);
        disc(app.pointer,12,ink[id]); identity_label(app.pointer,id);
    }
    label(14,486,"DRAG TWO DOTS / CHOOSE DEPTH",text_colour);
}
static void draw_board(void) {
    label(14,90,"THREE BY THREE / TWO MARKS",text_colour);
    for(unsigned i=1;i<3;++i) {
        float x=48+88×(float)i,y=148+88×(float)i;
        line((Point2){x,148},(Point2){x,412},5,text_colour);
        line((Point2){48,y},(Point2){312,y},5,text_colour);
    }
    for(unsigned cell=0;cell<9;++cell) {
        unsigned row=cell÷3,col=cell%3;
        Point2 center={92+88×(float)col,192+88×(float)row};
        if(app.grid.cell[cell]==1) {
            line((Point2){center.x-23,center.y-23},
                 (Point2){center.x+23,center.y+23},7,ink[0]);
            line((Point2){center.x-23,center.y+23},
                 (Point2){center.x+23,center.y-23},7,ink[0]);
        } else if(app.grid.cell[cell]==2)
            outline_circle(center,30,ink[1],7);
    }
    const char *message=app.grid.winner==1?"X WINS":app.grid.winner==2?"O WINS":
                        app.grid.winner==3?"DRAW":
                        app.grid.next==1?"X TO PLAY":"O TO PLAY";
    label(14,452,message,text_colour);
    rectangle(14,550,332,62,(Colour){0.17f,0.22f,0.29f});
    label(126,575,"RESET BOARD",text_colour);
}
static int sets_button_at(Point2 p) {
    if(!isfinite(p.x)||!isfinite(p.y)) return -1;
    for(int i=0;i<4;++i) {
        float x=(i%2)==0?14:186, y=i<2?550:620;
        if(p.x>=x && p.x<x+160 && p.y>=y && p.y<y+46) return i;
    }
    return -1;
}
static void draw_sets(void) {
    unsigned n=app.tensor.axes;
    char text[90];
    snprintf(text,sizeof(text),"%u BINARY AXES / %llu ATOMS",n,
             (unsigned long long)swap_tensor_size(&app.tensor));
    label(14,90,text,text_colour);
    if(n<=3) {
        if(n==2) {
            outline_circle((Point2){145,220},65,ink[0],4);
            outline_circle((Point2){215,220},65,ink[1],4);
            label(96,148,"A",ink[0]); label(252,148,"B",ink[1]);
        } else {
            outline_circle((Point2){148,192},60,ink[0],4);
            outline_circle((Point2){212,192},60,ink[1],4);
            outline_circle((Point2){180,246},60,ink[2],4);
            label(91,132,"A",ink[0]); label(267,132,"B",ink[1]);
            label(180,310,"C",ink[2]);
        }
    } else label(14,186,"HIGHER AXES: TENSOR SLICE",text_colour);

    if(n==3) {
        Point2 cube[8];
        for(unsigned mask=0;mask<8;++mask)
            cube[mask]=(Point2){96+100×(float)(mask&1)+43×(float)((mask>>2)&1),
                                423+63×(float)((mask>>1)&1)-43×(float)((mask>>2)&1)};
        for(unsigned a=0;a<8;++a)
            for(unsigned axis=0;axis<3;++axis) {
                unsigned b=a^(1u<<axis);
                if(a<b) line(cube[a],cube[b],2,text_colour);
            }
        for(unsigned mask=0;mask<8;++mask)
            disc(cube[mask],app.tensor.atom==mask?10:5,
                 app.tensor.atom==mask ? ink[2] : text_colour);
    } else {
        label(14,322,"2 x 2 SLICE / AXES A AND B",text_colour);
        for(unsigned y=0;y<2;++y) for(unsigned x=0;x<2;++x) {
            uint64_t atom=0;
            if(!swap_tensor_slice(&app.tensor,0,1,x!=0,y!=0,&atom)) continue;
            float cx=78+105×(float)x,cy=350+74×(float)y;
            rectangle(cx,cy,98,66,atom==app.tensor.atom ?
                      (Colour){0.38f,0.34f,0.25f} : (Colour){0.15f,0.20f,0.27f});
            char bits[15];
            snprintf(bits,sizeof(bits),"A%u B%u",x,y);
            label(cx+15,cy+26,bits,text_colour);
            if(atom==app.tensor.atom) label(cx+40,cy+43,"*",ink[2]);
        }
    }
    char bits[64];
    for(unsigned axis=0;axis<n;++axis) bits[axis]=swap_tensor_contains(&app.tensor,axis)?'1':'0';
    bits[n]=0;
    snprintf(text,sizeof(text),"ATOM %llu: %s (A FIRST)",
             (unsigned long long)app.tensor.atom,bits);
    label(14,513,text,text_colour);
    static const char *names[4]={"PREV ATOM","NEXT ATOM","FEWER AXES","MORE AXES"};
    for(int i=0;i<4;++i) {
        float x=(i%2)==0?14:186,y=i<2?550:620;
        rectangle(x,y,160,46,(Colour){0.17f,0.22f,0.29f});
        label(x+12,y+18,names[i],text_colour);
    }
}

/* A second, genuinely BELOW-the-fold section of the THREE canvas.
   Fixed canvas slots and six permanently coloured identities are distinct.
   These paths are display interpolations, not braid lifts. */
static bool six_open_at(Point2 p) {
    return swap_six_box_contains(swap_six_open_box(),p.x,p.y);
}
static bool six_back_at(Point2 p) {
    return swap_six_box_contains(swap_six_back_box(),p.x,p.y);
}
static int six_control_at(Point2 p) {
    return swap_six_control_hit(p.x,p.y);
}
static void six_command(unsigned index) {
    if (index<=2) {
        bool accepted=index<2 ? swap_six_request_generator(&app.six,index) :
                                 swap_six_request_random_nonidentity(&app.six);
        app.six_notice=accepted?"MOTION QUEUED":"QUEUE FULL";
    } else if (index==3 || index==5) {
        unsigned next=(app.six.group+SWAP_SIX_GROUP_COUNT+
                       (index==3?SWAP_SIX_GROUP_COUNT-1:1))
                       %SWAP_SIX_GROUP_COUNT;
        swap_six_select_group(&app.six,next);
        app.six_notice="GROUP CHANGED / ACTION RESET";
    } else if (index==4) {
        swap_six_reset(&app.six);
        app.six_notice="SIX-DISK ACTION RESET";
    }
}
static void draw_six_section(void) {
    rectangle(14,691,332,29,(Colour){0.20f,0.27f,0.33f});
    label(106,702,"SIX DISKS BELOW  v",text_colour);

    rectangle(14,736,332,46,(Colour){0.20f,0.27f,0.33f});
    label(122,754,"^  BACK TO BRAIDS",text_colour);
    label(14,807,"SIX COLOURED OBJECTS",text_colour);
    label(14,835,"GROUP ACTION / NOT BRAID LIFT",text_colour);
    char status[80];
    snprintf(status,sizeof(status),"%s / ORDER %u",
             swap_six_group_name(app.six.group),
             swap_six_group_order(app.six.group));
    label(14,866,status,text_colour);

    const Point2 centre={180,1002};
    const float radius=110;
    outline_circle(centre,radius,text_colour,1.8f);
    for (unsigned slot=0;slot<SWAP_SIX_N;++slot) {
        float theta=-1.57079632679f+6.28318530718f×(float)slot÷6.0f;
        Point2 point={centre.x+radius×cosf(theta),
                      centre.y+radius×sinf(theta)};
        disc(point,22,background);
        outline_circle(point,21,text_colour,1.4f);
    }
    for (unsigned identity=0;identity<SWAP_SIX_N;++identity) {
        SwapSixPoint unit;
        if(!swap_six_position(&app.six,identity,&unit)) continue;
        Point2 point={centre.x+radius×unit.x,centre.y+radius×unit.y};
        disc(point,18,six_ink[identity]);
        identity_label(point,identity);
    }
    if (app.six.moving) label(118,982,"MOVING",text_colour);
    snprintf(status,sizeof(status),"COMPLETED SLOTS: %u %u %u %u %u %u",
             app.six.current.identity_at[0]+1,app.six.current.identity_at[1]+1,
             app.six.current.identity_at[2]+1,app.six.current.identity_at[3]+1,
             app.six.current.identity_at[4]+1,app.six.current.identity_at[5]+1);
    label(14,1140,status,text_colour);
    snprintf(status,sizeof(status),"COMPLETED %u  /  QUEUED %u",
             app.six.completed,app.six.queued+(app.six.moving?1u:0u));
    label(14,1164,status,text_colour);
    char cycles[64];
    swap_six_cycles(app.six.current,cycles,sizeof(cycles));
    snprintf(status,sizeof(status),"CYCLES: %s",cycles);
    label(14,1186,status,text_colour);

    for (unsigned i=0;i<6;++i) {
        SwapSixBox box=swap_six_control_box(i);
        rectangle(box.x,box.y,box.width,box.height,app.pressed_six==(int)i ?
                  (Colour){0.32f,0.28f,0.38f} :
                  (Colour){0.14f,0.20f,0.28f});
        const char *title=i==0?swap_six_generator_name(app.six.group,0):
                          i==1?swap_six_generator_name(app.six.group,1):
                          i==2?"RANDOM NON-IDENTITY":
                          i==3?"PREV":i==4?"RESET":"NEXT";
        label(box.x+(i>=3?28:18),box.y+17,title,text_colour);
    }
    label(14,1391,app.six_notice?app.six_notice:
          "MOVES PERMUTE SLOTS; COLOURS STAY",text_colour);
}

static bool board_reset_hit(Point2 p) {
    return p.x>=14 && p.x<346 && p.y>=550 && p.y<612;
}
static void apply_sets_control(int index) {
    if(index==0) swap_tensor_step(&app.tensor,-1);
    else if(index==1) swap_tensor_step(&app.tensor,1);
    else if(index==2 && app.tensor.axes>2)
        swap_tensor_set_axes(&app.tensor,app.tensor.axes-1);
    else if(index==3 && app.tensor.axes<7)
        swap_tensor_set_axes(&app.tensor,app.tensor.axes+1);
}

static void command(int command) {
    SwapScene *active=app.view==SWAP_VIEW_TWO?&app.pair:&app.scene;
    if(command<4) {
        if(app.view==SWAP_VIEW_TWO && (command%2)) return;
        SwapMove move={(uint8_t)(app.view==SWAP_VIEW_TWO?0:command%2),
                       (int8_t)(command<2?1:-1)};
        SwapResult result=swap_append(active,move);
        app.notice=result==SWAP_HISTORY_FULL ? "HISTORY FULL - RESET" :
                   result==SWAP_BUSY ? "CROSSING BUSY" :
                   result==SWAP_ACCEPTED ? "CROSSING ADDED" : "INVALID MOVE";
    } else if(command==SWAP_RESET) {
        swap_reset(active); app.notice="RESET";
    } else if(command==SWAP_REPLAY) {
        swap_replay(active); app.notice="REPLAY";
    } else if(command==SWAP_PAUSE) {
        active->paused=!active->paused;
        app.notice=active->paused?"PAUSED":"RESUMED";
    } else if(command==SWAP_DRAG_OVER || command==SWAP_DRAG_UNDER) {
        app.dragged_over=command==SWAP_DRAG_OVER;
        app.notice=app.dragged_over ? "DRAGGED STRAND GOES OVER" :
                                     "DRAGGED STRAND GOES UNDER";
    }
}
static Point2 pointer_position(float x,float y) {
    SwapCanvas canvas={app.scale,app.offset_x,app.offset_y};
    Point2 result=swap_pointer_on_canvas(canvas,x,y);
    if (app.view==SWAP_VIEW_THREE) result.y+=app.scroll_y;
    return result;
}

static void pointer_down(Point2 p) {
    app.press_tracking=false;
    if (fabsf(app.scroll_y-app.scroll_target)>1.0f) return;
    app.press_tracking=true; app.scroll_swipe=false;
    app.down_origin=p;
    app.pointer=p; app.drag_slot=-1; app.pressed_button=-1;
    app.pressed_cell=-1; app.pressed_panel=-1; app.pressed_six=-1;
    if (app.view==SWAP_VIEW_THREE) {
        if (six_open_at(p)) {app.pressed_six=-2; return;}
        if (six_back_at(p)) {app.pressed_six=-3; return;}
        app.pressed_six=six_control_at(p);
        if (app.pressed_six>=0) return;
    }
    app.pressed_mode=swap_view_at(p);
    if(app.pressed_mode>=0) return;
    if(app.view==SWAP_VIEW_GRID) {
        app.pressed_cell=swap_grid_at(p);
        if(app.pressed_cell<0 && board_reset_hit(p)) app.pressed_panel=0;
        return;
    }
    if(app.view==SWAP_VIEW_SETS) {
        app.pressed_panel=sets_button_at(p);
        return;
    }
    int button=swap_button_at(p);
    if(button>=0) {
        /* There is only one adjacent pair in B2. */
        if(app.view!=SWAP_VIEW_TWO || (button!=1 && button!=3))
            app.pressed_button=button;
        return;
    }
    app.drag_slot=app.view==SWAP_VIEW_TWO?
                  swap_pair_head_at(p,app.pair_head_y):
                  swap_head_at(p,app.head_y);
    if(app.drag_slot>=0)
        app.press_origin=app.view==SWAP_VIEW_TWO ?
             app.pair_head[app.drag_slot] : app.head[app.drag_slot];
    else app.notice="TOUCH A COLOURED DOT OR CONTROL";
}
static void pointer_move(Point2 p) {
    app.pointer=p;
    if (!app.press_tracking || app.scroll_swipe ||
        app.view!=SWAP_VIEW_THREE) return;
    float dy=p.y-app.down_origin.y,dx=p.x-app.down_origin.x;
    /* Vertical swipe navigates between stacked panels. Horizontal motions
       are still braid gestures, and a vertical swipe cancels a pressed button. */
    if (fabsf(dy)>52.0f && fabsf(dy)>1.35f×fabsf(dx)) {
        app.scroll_target=dy<0.0f?720.0f:0.0f;
        app.scroll_swipe=true;
        app.pressed_six=-1; app.pressed_mode=-1; app.pressed_button=-1;
        app.pressed_panel=-1; app.pressed_cell=-1; app.drag_slot=-1;
    }
}
static void pointer_up(Point2 p) {
    if (!app.press_tracking) return;
    app.press_tracking=false;
    if (app.scroll_swipe) {app.scroll_swipe=false; return;}
    app.pointer=p;
    if (app.pressed_six==-2) {
        if(six_open_at(p)) app.scroll_target=720.0f;
    } else if (app.pressed_six==-3) {
        if(six_back_at(p)) app.scroll_target=0.0f;
    } else if (app.pressed_six>=0) {
        if(six_control_at(p)==app.pressed_six)
            six_command((unsigned)app.pressed_six);
    } else if(app.pressed_mode>=0) {
        if(swap_view_at(p)==app.pressed_mode) {
            app.view=(SwapView)app.pressed_mode;
            app.scroll_y=0.0f; app.scroll_target=0.0f;
            app.notice=0;
        }
    } else if(app.view==SWAP_VIEW_GRID) {
        if(app.pressed_cell>=0 && swap_grid_at(p)==app.pressed_cell)
            swap_grid_mark(&app.grid,(unsigned)app.pressed_cell);
        else if(app.pressed_panel==0 && board_reset_hit(p))
            swap_grid_reset(&app.grid);
    } else if(app.view==SWAP_VIEW_SETS) {
        if(app.pressed_panel>=0 && sets_button_at(p)==app.pressed_panel)
            apply_sets_control(app.pressed_panel);
    } else if(app.pressed_button>=0) {
        if(swap_point_on_button(p,(unsigned)app.pressed_button))
            command(swap_buttons[app.pressed_button].command);
        else app.notice="BUTTON CANCELLED";
    } else if(app.drag_slot>=0) {
        SwapMove move;
        bool valid=app.view==SWAP_VIEW_TWO ?
            swap_pair_drag_move(app.drag_slot,app.press_origin,p,
                                app.dragged_over,&move):
            swap_drag_move(app.drag_slot,app.press_origin,p,
                           app.dragged_over,&move);
        if(valid) {
            SwapScene *active=app.view==SWAP_VIEW_TWO?&app.pair:&app.scene;
            SwapResult result=swap_append(active,move);
            app.notice=result==SWAP_ACCEPTED?"DRAG CROSSING ADDED":
                       result==SWAP_HISTORY_FULL?"HISTORY FULL - RESET":
                       result==SWAP_BUSY?"CROSSING BUSY":"INVALID DRAG";
        } else app.notice="DRAG TO THE OTHER STRAND";
    }
    app.drag_slot=-1; app.pressed_button=-1; app.pressed_mode=-1;
    app.pressed_cell=-1; app.pressed_panel=-1; app.pressed_six=-1;
}
static void event(const sapp_event *event) {
    if(event->type==SAPP_EVENTTYPE_MOUSE_DOWN &&
       event->mouse_button==SAPP_MOUSEBUTTON_LEFT && !app.touch_active) {
        pointer_down(pointer_position(event->mouse_x,event->mouse_y));
    } else if(event->type==SAPP_EVENTTYPE_MOUSE_MOVE && !app.touch_active) {
        pointer_move(pointer_position(event->mouse_x,event->mouse_y));
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
                pointer_move(pointer_position(event->touches[i].pos_x,
                                              event->touches[i].pos_y));
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
        app.pressed_mode=-1; app.pressed_cell=-1; app.pressed_panel=-1;
        app.pressed_six=-1; app.press_tracking=false; app.scroll_swipe=false;
        if(event->type==SAPP_EVENTTYPE_SUSPENDED) app.scene.paused=true;
    }
}
static void init(void) {
    swap_reset(&app.scene); swap_reset(&app.pair);
    swap_grid_reset(&app.grid);
    swap_tensor_set_axes(&app.tensor,2);
    swap_six_init(&app.six);
    app.view=SWAP_VIEW_THREE;
    app.drag_slot=-1; app.pressed_button=-1; app.pressed_mode=-1;
    app.pressed_cell=-1; app.pressed_panel=-1; app.pressed_six=-1;
    app.dragged_over=true; app.scale=1;
    sg_setup(&(sg_desc){.environment=sglue_environment(),.logger.func=slog_func});
    sgl_setup(&(sgl_desc_t){.max_vertices=60000,.max_commands=6000,.logger.func=slog_func});
    sdtx_setup(&(sdtx_desc_t){.fonts={sdtx_font_kc853()},.logger.func=slog_func});
}
static void frame(void) {
    double seconds=sapp_frame_duration();
    /* Do not fast-forward an entire crossing after a lifecycle stall. */
    swap_tick(&app.scene,fmin(seconds,0.10));
    swap_tick(&app.pair,fmin(seconds,0.10));
    swap_six_tick(&app.six,fmin(seconds,0.10));
    /* Scroll is animated in canvas coordinates. Neither section overwrites
       the other, and all original THREE hit-box coordinates remain intact. */
    float scroll_difference=app.scroll_target-app.scroll_y;
    float scroll_step=(float)fmin(seconds,0.06)×3600.0f;
    if (fabsf(scroll_difference)<=scroll_step) app.scroll_y=app.scroll_target;
    else app.scroll_y+=scroll_difference>0.0f?scroll_step:-scroll_step;
    viewport();
    label(14,16,"SWAP",text_colour);
    label(174,16,app.view==SWAP_VIEW_THREE?"BRAIDS / TRIANGLE":
                  app.view==SWAP_VIEW_TWO?"TWO STRANDS":
                  app.view==SWAP_VIEW_GRID?"TIC-TAC-TOE":"BINARY TENSORS",
                  text_colour);
    draw_mode_tabs();
    if(app.view==SWAP_VIEW_THREE) {
        draw_triangle_view(); draw_rivers();
    } else if(app.view==SWAP_VIEW_TWO) draw_pair();
    else if(app.view==SWAP_VIEW_GRID) draw_board();
    else draw_sets();
    if(app.view==SWAP_VIEW_THREE || app.view==SWAP_VIEW_TWO) {
        if(app.view==SWAP_VIEW_THREE)
            label(14,496,"DRAG HORIZONTALLY / CHOOSE DEPTH",text_colour);
        for(unsigned i=0;i<SWAP_BUTTON_COUNT;++i) {
            /* The right-pair crossings don't exist for B2. */
            if(app.view==SWAP_VIEW_TWO && (i==1 || i==3)) continue;
            const SwapButton *b=&swap_buttons[i];
            SwapScene *active=app.view==SWAP_VIEW_TWO?&app.pair:&app.scene;
            bool selected=(b->command==SWAP_DRAG_OVER && app.dragged_over) ||
                          (b->command==SWAP_DRAG_UNDER && !app.dragged_over);
            bool pressed=app.pressed_button==(int)i;
            Colour fill=selected ? (Colour){0.28f,0.32f,0.39f} :
                        pressed ? (Colour){0.30f,0.26f,0.37f} :
                                  (Colour){0.13f,0.17f,0.23f};
            rectangle(b->x,b->y,b->width,b->height,fill);
            const char *title=b->command==SWAP_PAUSE && active->paused ?
                              "RESUME" : b->title;
            label(b->x+10,b->y+16,title,text_colour);
        }
        if(app.view==SWAP_VIEW_TWO) {
            label(196,569,"B2: ONE PAIR",text_colour);
            label(196,610,"SIGN +/-",text_colour);
        }
        if(app.notice) label(14,229,app.notice,text_colour);
    }
    if(app.view==SWAP_VIEW_THREE) draw_six_section();
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
