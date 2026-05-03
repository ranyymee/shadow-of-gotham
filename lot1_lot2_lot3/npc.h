#ifndef NPC_H
#define NPC_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define ENEMY 0
#define POWERUP 1
#define JOKER 0
#define HARLEY 1
#define ALIVE 0
#define INJURED 1
#define DEAD 2
#define HORIZONTAL 0
#define VERTICAL 1
#define DIAGONAL 2
#define MAX_BLOOD 100
#define ANIM_FRAMES 5
#define MAX_ENEMIES 20
#define MAX_ITEMS 20

typedef struct {
    int x, y, life, active;
} Blood;

typedef struct {
    int type, enemyType;
    SDL_Rect rect;
    int x, y, w, h, speed;
    int traj, dx, dy, minX, maxX, minY, maxY;
    int health, maxHealth, healthState;
    int damage, score;
    SDL_Texture *run[ANIM_FRAMES], *attack[ANIM_FRAMES], *death[ANIM_FRAMES];
    SDL_Texture *tex;
    int frame, step;
    int attacking, dying, active;
} NPC;

typedef struct {
    NPC enemies[MAX_ENEMIES];
    NPC items[MAX_ITEMS];
    int enemyCnt, itemCnt, level;
    SDL_Renderer *renderer;
    int *pHealth, *pScore;
    unsigned int invincibleEnd;
    SDL_Texture *bg, *itemTex;
    Blood blood[MAX_BLOOD];
} GameNPC;

void NPC_init(GameNPC *n, SDL_Renderer *r, const char *bgPath);
void NPC_loadLevel(GameNPC *n, SDL_Renderer *r, int lvl);
void NPC_update(GameNPC *n, SDL_Rect *player, int *health, int *score, int cx, int cy);
void NPC_draw(GameNPC *n, int cx, int cy);
void NPC_clean(GameNPC *n);

#endif
