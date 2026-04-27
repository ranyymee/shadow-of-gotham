#pragma once
/*
 * header.h — TACTICAL DECRYPTION ENGINE
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* ─────────────────────────────────────────────────────────────────────────
   WINDOW
   ───────────────────────────────────────────────────────────────────────── */
#define WIN_W   1100
#define WIN_H    620

/* ─────────────────────────────────────────────────────────────────────────
   COLOUR PALETTE
   ───────────────────────────────────────────────────────────────────────── */
#define CY_R  0
#define CY_G  220
#define CY_B  255
#define AM_R  255
#define AM_G  160
#define AM_B  0
#define GR_R  0
#define GR_G  255
#define GR_B  120
#define RD_R  255
#define RD_G  55
#define RD_B  55
#define PU_R  180
#define PU_G  80
#define PU_B  255
#define MG_R  255
#define MG_G  60
#define MG_B  200
#define PNL_R 0
#define PNL_G 12
#define PNL_B 30

/* ─────────────────────────────────────────────────────────────────────────
   PUZZLE CONFIG
   ───────────────────────────────────────────────────────────────────────── */
#define GCOLS      3
#define GROWS      3
#define GPIECES    9
#define CELL       130
#define GRID_X     310
#define GRID_Y      90
#define RPW        82
#define RPH        58
#define REPO_Y     508
#define RGAP       10
#define TIMER_MS   60000
#define MAX_HINTS  3
#define MAX_SPARKS 200
#define MAX_DUST    70

/* ─────────────────────────────────────────────────────────────────────────
   PUZZLE SELECTION  — paths relative to executable (assets/ subfolder)
   ───────────────────────────────────────────────────────────────────────── */
#define NUM_PUZZLES  4
static const char *PUZZLE_IMAGES[NUM_PUZZLES] = {
    "assets/puzzle.png",
    "assets/puzzle1.png",
    "assets/puzzle2.png",
    "assets/puzzle3.png"
};

/* ─────────────────────────────────────────────────────────────────────────
   MENU LAYOUT
   ───────────────────────────────────────────────────────────────────────── */
#define MAIN_X   240
#define MAIN_Y    54
#define MAIN_W   596
#define MAIN_H   310
#define CARD_Y   392
#define CARD_H   136
#define CARD_W   196
#define CARD_A_X  46
#define CARD_B_X 450
#define CARD_C_X 854
#define BACK_X    24
#define BACK_Y   570
#define BACK_W    90
#define BACK_H    34
#define ACCENT    18

/* ─────────────────────────────────────────────────────────────────────────
   ENUMS
   ───────────────────────────────────────────────────────────────────────── */
typedef enum { ST_PLAY, ST_WIN, ST_LOSE } GameState;

/* ─────────────────────────────────────────────────────────────────────────
   STRUCTS
   ───────────────────────────────────────────────────────────────────────── */
typedef struct {
    SDL_Rect     rect;
    SDL_Texture *img;
    int          puzzleIdx;
    float        hoverT;
    int          clicked;
} Card;

typedef struct {
    int      idx;
    int      slot;
    SDL_Rect cur;
    SDL_Rect home;
    float    shakeX;
    float    shakeTmr;
    float    flashT;
    int      flashOk;
} Piece;

typedef struct {
    float x, y, vx, vy, life, maxLife;
    Uint8 r, g, b;
} Spark;

typedef struct {
    float x, y, spd, phase, size;
} Dust;

typedef struct {
    float intensity;
    float timeLeft;
    int   ox, oy;
} ScreenShake;

/* ─────────────────────────────────────────────────────────────────────────
   SHARED RENDERER + AUDIO
   ───────────────────────────────────────────────────────────────────────── */
extern SDL_Renderer *g_ren;
extern TTF_Font     *g_fntBig;
extern TTF_Font     *g_fntMid;
extern TTF_Font     *g_fntSm;
extern TTF_Font     *g_fntTiny;
extern Mix_Music    *g_music;

/* ─────────────────────────────────────────────────────────────────────────
   DRAW PRIMITIVES
   ───────────────────────────────────────────────────────────────────────── */
static inline void SC(Uint8 r,Uint8 g,Uint8 b,Uint8 a){
    SDL_SetRenderDrawBlendMode(g_ren,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g_ren,r,g,b,a);
}
static inline void FR(SDL_Rect r,Uint8 R,Uint8 G,Uint8 B,Uint8 A){
    SC(R,G,B,A); SDL_RenderFillRect(g_ren,&r);
}
static inline void SR(SDL_Rect r,Uint8 R,Uint8 G,Uint8 B,Uint8 A,int t){
    SC(R,G,B,A);
    for(int i=0;i<t;i++){SDL_Rect b={r.x-i,r.y-i,r.w+i*2,r.h+i*2}; SDL_RenderDrawRect(g_ren,&b);}
}
static inline void CO(SDL_Rect r,Uint8 R,Uint8 G,Uint8 B,Uint8 A,int l){
    SC(R,G,B,A); int x=r.x,y=r.y,w=r.w,h=r.h;
    SDL_RenderDrawLine(g_ren,x,y,x+l,y);     SDL_RenderDrawLine(g_ren,x,y,x,y+l);
    SDL_RenderDrawLine(g_ren,x+w,y,x+w-l,y); SDL_RenderDrawLine(g_ren,x+w,y,x+w,y+l);
    SDL_RenderDrawLine(g_ren,x,y+h,x+l,y+h); SDL_RenderDrawLine(g_ren,x,y+h,x,y+h-l);
    SDL_RenderDrawLine(g_ren,x+w,y+h,x+w-l,y+h); SDL_RenderDrawLine(g_ren,x+w,y+h,x+w,y+h-l);
}
static inline void GL(SDL_Rect r,Uint8 R,Uint8 G,Uint8 B,float s){
    for(int i=9;i>=1;i--){
        Uint8 a=(Uint8)(s*(33-i*3)); if(!a)continue;
        SDL_Rect g={r.x-i*2,r.y-i*2,r.w+i*4,r.h+i*4}; FR(g,R,G,B,a);
    }
}
static inline void SL(SDL_Rect r,Uint8 a){
    SC(0,0,0,a);
    for(int y=r.y;y<r.y+r.h;y+=3) SDL_RenderDrawLine(g_ren,r.x,y,r.x+r.w,y);
}
static inline void TC(const char *s,SDL_Rect r,Uint8 R,Uint8 G,Uint8 B,TTF_Font *f){
    if(!f||!s||!s[0])return;
    SDL_Color c={R,G,B,255};
    SDL_Surface *su=TTF_RenderText_Blended(f,s,c); if(!su)return;
    SDL_Texture *t=SDL_CreateTextureFromSurface(g_ren,su); SDL_FreeSurface(su); if(!t)return;
    int tw,th; SDL_QueryTexture(t,NULL,NULL,&tw,&th);
    SDL_Rect d={r.x+(r.w-tw)/2,r.y+(r.h-th)/2,tw,th};
    SDL_RenderCopy(g_ren,t,NULL,&d); SDL_DestroyTexture(t);
}
static inline void TL(const char *s,int x,int y,Uint8 R,Uint8 G,Uint8 B,TTF_Font *f){
    if(!f||!s||!s[0])return;
    SDL_Color c={R,G,B,255};
    SDL_Surface *su=TTF_RenderText_Blended(f,s,c); if(!su)return;
    SDL_Texture *t=SDL_CreateTextureFromSurface(g_ren,su); SDL_FreeSurface(su); if(!t)return;
    int tw,th; SDL_QueryTexture(t,NULL,NULL,&tw,&th);
    SDL_Rect d={x,y,tw,th}; SDL_RenderCopy(g_ren,t,NULL,&d); SDL_DestroyTexture(t);
}
static inline void BAT(int cx,int cy,int sz,Uint8 R,Uint8 G,Uint8 B,Uint8 A){
    SC(R,G,B,A); int h=sz/2;
    SDL_RenderDrawLine(g_ren,cx,cy-h,cx-h*2,cy+h/2);
    SDL_RenderDrawLine(g_ren,cx-h*2,cy+h/2,cx-h,cy);
    SDL_RenderDrawLine(g_ren,cx-h,cy,cx,cy+h/3);
    SDL_RenderDrawLine(g_ren,cx,cy+h/3,cx+h,cy);
    SDL_RenderDrawLine(g_ren,cx+h,cy,cx+h*2,cy+h/2);
    SDL_RenderDrawLine(g_ren,cx+h*2,cy+h/2,cx,cy-h);
}

/* ─────────────────────────────────────────────────────────────────────────
   MODULE ENTRY POINTS
   ───────────────────────────────────────────────────────────────────────── */
int run_menu(SDL_Window *win);
int run_puzzle(SDL_Window *win, int puzzleIdx);
