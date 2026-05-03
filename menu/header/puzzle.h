#ifndef PUZZLE_H
#define PUZZLE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define WIN_W      1920
#define WIN_H      1080
#define GCOLS         3
#define GROWS         3
#define GPIECES       9
#define CELL        226
#define GRID_X      541
#define GRID_Y      157
#define RPW         143
#define RPH         101
#define REPO_Y      885
#define RGAP         17
#define TIMER_MS  60000
#define MAX_HINTS     3
#define MAX_SPARKS  200
#define MAX_DUST     70
#define NUM_PUZZLES   4

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

#define MAIN_X   419
#define MAIN_Y    94
#define MAIN_W  1040
#define MAIN_H   540
#define CARD_Y   683
#define CARD_H   237
#define CARD_W   342
#define CARD_A_X  80
#define CARD_B_X 785
#define CARD_C_X 1491
#define BACK_X    42
#define BACK_Y   993
#define BACK_W   157
#define BACK_H    59
#define ACCENT    32

#define ST_PLAY  0
#define ST_WIN   1
#define ST_LOSE  2

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
} Etincelle;

typedef struct {
    float x, y, spd, phase, size;
} Poussiere;

typedef struct {
    float intensity;
    float timeLeft;
    int   ox, oy;
} TrembEcran;

typedef struct {
    SDL_Rect     rect;
    SDL_Texture *img;
    int          puzzleIdx;
    float        hoverT;
    int          clicked;
} CartePuzzle;

typedef struct {
    SDL_Renderer *ren;
    TTF_Font     *fntBig;
    TTF_Font     *fntMid;
    TTF_Font     *fntSm;
    TTF_Font     *fntTiny;
    SDL_Texture  *bgTex;
    int           puzzW;
    int           puzzH;
    SDL_Texture  *puzzTex;
    char          hintCipher[32];
    int           hintSlot;
    int           hintPiece;
    int           hintGlow;
    int           sparkCount;
    Etincelle     sparks[MAX_SPARKS];
    Poussiere     dust[MAX_DUST];
} ContextPuzzle;

void initContextPuzzle  (ContextPuzzle *ctx, SDL_Renderer *ren,
                         TTF_Font *big, TTF_Font *mid,
                         TTF_Font *sm,  TTF_Font *tiny);
void initMenuPuzzle     (CartePuzzle cards[NUM_PUZZLES], ContextPuzzle *ctx,
                         const char *images[NUM_PUZZLES]);
void renderMenuPuzzle   (CartePuzzle cards[NUM_PUZZLES], ContextPuzzle *ctx,
                         Uint32 ticks, int backHov, int backClicked);
int  runMenuPuzzle      (SDL_Window *win, ContextPuzzle *ctx);
int  runPuzzle          (SDL_Window *win, ContextPuzzle *ctx, int puzzleIdx);
void freeContextPuzzle  (ContextPuzzle *ctx);

#endif
