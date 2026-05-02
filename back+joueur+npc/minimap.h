#ifndef MINIMAP_H
#define MINIMAP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

/* ── minimap display size on screen ── */
#define MINIMAP_WIDTH   320
#define MINIMAP_HEIGHT   80

/* ── dot sizes ── */
#define PLAYER_DOT_SIZE   8
#define ENEMY_DOT_SIZE    6
#define PLAYER2_DOT_SIZE  8

/* ── shake animation ── */
#define SHAKE_FRAMES     14
#define SHAKE_INTENSITY   6

/* ─────────────────────────────────────────────────────
   MinimapEnemy — lightweight struct used only by the
   minimap. Built each frame from GameNPC in main.c.
───────────────────────────────────────────────────── */
typedef struct {
    int x, y;      /* world-space position */
    int active;    /* 1 = show on minimap  */
} MinimapEnemy;

#define MINIMAP_MAX_ENEMIES 20

/* ─────────────────────────────────────────────────────
   ShakeState — background shake on collision
───────────────────────────────────────────────────── */
typedef struct {
    int active;
    int framesLeft;
    int offsetX;
    int offsetY;
} ShakeState;

/* ─────────────────────────────────────────────────────
   Minimap struct
───────────────────────────────────────────────────── */
typedef struct {
    SDL_Texture *backgroundTexture; /* miniature level image  */
    SDL_Rect     minimapPosition;   /* where it sits on screen */
    SDL_Texture *playerTexture;     /* red dot  — player 1    */
    SDL_Texture *player2Texture;    /* magenta dot — player 2 */
    SDL_Texture *enemyTexture;      /* yellow dot — enemies   */
    SDL_Rect     playerPosition;
    SDL_Rect     player2Position;
} Minimap;

/* ─────────────────────────────────────────────────────
   MINIMAP FUNCTIONS
───────────────────────────────────────────────────── */

/* imagePath: path to the miniature background PNG.
   If NULL or missing, falls back to a plain dark rect. */
Minimap *createMinimap(SDL_Renderer *renderer,
                       const char   *imagePath,
                       SDL_Rect      position);

/* Update both player dots.
   p1WorldX/Y = p1.x + camera_pos.x  (world coords)
   worldW/worldH = bg.partW*bg.imgCount / bg.partH    */
void updateMinimap(Minimap *m,
                   int p1WorldX, int p1WorldY,
                   int p2WorldX, int p2WorldY,
                   int worldW,   int worldH);

/* Draw: background → enemy dots → p2 dot → p1 dot   */
void renderMinimap(SDL_Renderer  *renderer,
                   Minimap       *m,
                   MinimapEnemy   enemies[],
                   int            enemyCount,
                   int            worldW,
                   int            worldH);

void freeMinimap(Minimap *m);

/* ─────────────────────────────────────────────────────
   SHAKE FUNCTIONS
───────────────────────────────────────────────────── */

/* Call once when a collision happens */
void triggerShake(ShakeState *shake);

/* Call every frame — updates offsetX / offsetY */
void updateShake(ShakeState *shake);

#endif /* MINIMAP_H */
