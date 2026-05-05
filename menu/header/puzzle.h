#ifndef PUZZLE_H
#define PUZZLE_H

/*
 * puzzle.h — Header du module Puzzle Tactique
 * Inclus par : puzzle.c  et  mainenigme.c uniquement
 */

#include "header.h"   /* WIN_W, WIN_H, macros dessin, PuzzleState, etc. */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

/* ── Screen Shake (alias TrembEcran) ── */
typedef struct {
    float intensity;
    float timeLeft;
    int   ox, oy;
} TrembEcran;

/* ── Etincelle (spark) ── */
typedef struct {
    float x, y, vx, vy, life, maxLife;
    Uint8 r, g, b;
} Etincelle;

/* ── Carte du menu de sélection ── */
typedef struct {
    SDL_Rect     rect;
    SDL_Texture *img;
    int          puzzleIdx;
    float        hoverT;
    int          clicked;
} CartePuzzle;

/* ── Contexte partagé du moteur puzzle ── */
typedef struct {
    SDL_Renderer *ren;
    TTF_Font     *fntBig;
    TTF_Font     *fntMid;
    TTF_Font     *fntSm;
    TTF_Font     *fntTiny;

    SDL_Texture  *bgTex;
    SDL_Texture  *puzzTex;
    int           puzzW, puzzH;

    char  hintCipher[32];
    int   hintSlot;
    int   hintPiece;
    int   hintGlow;

    Etincelle sparks[MAX_SPARKS];
    int       sparkCount;

    Dust  dust[MAX_DUST];
} ContextPuzzle;

/* ── Menu du module Enigme (Quiz + Puzzle) ── */
typedef struct {
    SDL_Texture *bgTexture;
    SDL_Texture *quizTex,   *quizHoverTex;
    SDL_Texture *puzzleTex, *puzzleHoverTex;
    SDL_Rect     quizRect,   puzzleRect;
    int          hoverQuiz,  hoverPuzzle;
} MenuPrincipal;

/* ── Prototypes puzzle.c ── */
void initContextPuzzle (ContextPuzzle *ctx, SDL_Renderer *ren,
                        TTF_Font *big, TTF_Font *mid,
                        TTF_Font *sm,  TTF_Font *tiny);
void initMenuPuzzle    (CartePuzzle cards[NUM_PUZZLES], ContextPuzzle *ctx,
                        const char *images[NUM_PUZZLES]);
void renderMenuPuzzle  (CartePuzzle cards[NUM_PUZZLES], ContextPuzzle *ctx,
                        Uint32 ticks, int backHov, int backClicked);
int  runMenuPuzzle     (SDL_Window *win, ContextPuzzle *ctx);
int  runPuzzle         (SDL_Window *win, ContextPuzzle *ctx, int puzzleIdx);
void freeContextPuzzle (ContextPuzzle *ctx);

#endif /* PUZZLE_H */
