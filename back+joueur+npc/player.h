#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define MAX_BULLETS    8

#define ANIM_COUNT      11

#define ANIM_IDLE        0
#define ANIM_WALK        1
#define ANIM_RUN         2
#define ANIM_JUMP        3
#define ANIM_DOUBLE_JUMP 4
#define ANIM_UP          5
#define ANIM_ATTACK      6
#define ANIM_KICK        7
#define ANIM_SHOOT       8
#define ANIM_CROUCH      9
#define ANIM_DEAD       10

#define MAX_ANIM_FRAMES 16

#define BAT_FRAME_W    448
#define BAT_FRAME_H    484
#define CAT_FRAME_W    448
#define CAT_FRAME_H    484

#define SCREEN_W       900
#define SCREEN_H       620
#define GROUND_Y       190

#define SPEED        220.0
#define SPEED_RUN    380.0

#define JUMP_VY        -600.0f
#define JUMP_GRAVITY    1400.0f
#define JUMP_CUT_MULT     0.45f
#define HOP_VY         -480.0f

#define COYOTE_TIME     0.08f
#define JUMP_BUFFER     0.10f

#define SAUT_VX          1.2
#define SAUT_X_MIN     -50.0
#define SAUT_X_MAX      50.0
#define UP_X_MIN       -31.6
#define UP_X_MAX        31.6

#define COSTUME_COUNT  3

typedef struct {
    SDL_Texture *sheetRight;
    SDL_Texture *sheetLeft;
    int          frameW;
    int          frameH;
    int          frameCounts[ANIM_COUNT];
    int          frameOffsets[ANIM_COUNT][MAX_ANIM_FRAMES];
    int          frameWidths[ANIM_COUNT][MAX_ANIM_FRAMES];
    int          frameRows[ANIM_COUNT];
    int          frameHeights[ANIM_COUNT];
} SpriteData;

typedef struct {
    float x, y, vx;
    int   active, owner;
} Bullet;

typedef struct {
    SDL_Scancode left, right, up, jump;
    SDL_Scancode punch, kick, shoot, crouch;
    SDL_Scancode fly;
} InputConfig;

#define DEFAULT_INPUT_P1 (InputConfig){ \
    SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_UP, SDL_SCANCODE_SPACE, \
    SDL_SCANCODE_KP_1, SDL_SCANCODE_KP_2, SDL_SCANCODE_KP_3, SDL_SCANCODE_DOWN, \
    SDL_SCANCODE_SPACE \
}
#define DEFAULT_INPUT_P2 (InputConfig){ \
    SDL_SCANCODE_A, SDL_SCANCODE_D, SDL_SCANCODE_Q, SDL_SCANCODE_W, \
    SDL_SCANCODE_F, SDL_SCANCODE_E, SDL_SCANCODE_H, SDL_SCANCODE_S, \
    SDL_SCANCODE_W \
}

typedef struct {
    double     x, y;
    double     vitesse;
    double     acceleration;
    float      vy;
    int        onGround;

    int        jumping;
    double     posinit_y;
    double     saut_x_rel;
    int        canDoubleJump;
    int        jumpPressed;
    int        jumpCut;
    float      coyoteTimer;
    float      jumpBuffer;

    int        hopping;
    double     hop_posinit_y;
    double     hop_x_rel;

    int        isFlying;

    SDL_Rect   posScreen;
    int        w, h;

    int        direction;

    int        animState;
    int        animFrame;
    float      animTimer;
    float      animSpeed;
    SpriteData sprite;

    int        isAttacking, isKicking, isShooting;
    int        isCrouching, isRunning;
    int        actionTimer;

    int        vies, hp, isAlive, score;
    int        isCat, costume;

    Bullet     bullets[MAX_BULLETS];
} Player;

void initSpriteData(SpriteData *s, SDL_Texture *right, SDL_Texture *left,
                    int frameW, int frameH, int isCat);
void initPlayer(Player *p, float x, float y, int isCat);
void animatePlayer(Player *p, float dt);
void blitPlayer(SDL_Renderer *r, Player *p);
void updatePlayer(Player *p, float dt);
void drawHUD(SDL_Renderer *r, TTF_Font *font, Player *p1, Player *p2);

#endif
