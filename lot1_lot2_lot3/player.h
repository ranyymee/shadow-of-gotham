#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define MAX_BULLETS    8
#define ANIM_COUNT     9

#define ANIM_IDLE      0
#define ANIM_WALK      1
#define ANIM_RUN       2
#define ANIM_JUMP      3
#define ANIM_ATTACK    4
#define ANIM_KICK      5
#define ANIM_SHOOT     6
#define ANIM_FLY       7
#define ANIM_DEAD      8

#define BAT_FRAME_W    172
#define BAT_FRAME_H    174
#define CAT_FRAME_W    168
#define CAT_FRAME_H    172

#define SCREEN_W       900
#define SCREEN_H       620
#define GROUND_Y       460

typedef struct {
    SDL_Texture *sheetRight;
    SDL_Texture *sheetLeft;
    int          frameW;
    int          frameH;
    int          frameCounts[ANIM_COUNT];
    SDL_Rect     posSprite;
} SpriteData;

typedef struct {
    float x, y, vx;
    int   active, owner;
} Bullet;

typedef struct {
    SDL_Scancode left, right, up, fly;
    SDL_Scancode punch, kick, shoot, crouch;
} InputConfig;

typedef struct {
    double     x, y;
    double     vitesse;
    double     acceleration;
    double     vy;
    int        onGround;
    int        up;
    double     posinit_y;
    double     saut_x_rel;
    int        canDoubleJump;
    int        jumpPressed;
    int        isFlying;
    SDL_Rect   posScreen;
    int        direction;
    int        animRow;
    int        animFrame;
    float      animTimer;
    float      animSpeed;
    SpriteData sprite;
    int        w, h;
    int        isAttacking, isKicking, isShooting, isCrouching, isRunning;
    int        actionTimer;
    int        vies, hp, isAlive, score;
    int        isCat, costume;
    Bullet     bullets[MAX_BULLETS];
} Player;

void initSpriteData(SpriteData *s, SDL_Texture *right, SDL_Texture *left, int frameW, int frameH);
void initPlayer(Player *p, float x, float y, int isCat);
void animatePlayer(Player *p);
void blitPlayer(SDL_Renderer *r, Player *p);
void updatePlayer(Player *p, float dt);
void drawHUD(SDL_Renderer *r, TTF_Font *font, Player *p1, Player *p2);

#endif
