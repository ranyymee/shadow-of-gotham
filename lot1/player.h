#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

/* ── bullets ── */
#define MAX_BULLETS    8

/* ── animation states ── */
#define ANIM_COUNT      11

#define ANIM_IDLE        0
#define ANIM_WALK        1
#define ANIM_RUN         2
#define ANIM_JUMP        3   /* parabolic jump (SPACE) – shares frames with double-jump */
#define ANIM_DOUBLE_JUMP 4   /* double-jump (2nd SPACE in air) – same row, different colour tint */
#define ANIM_UP          5   /* vertical hop-in-place (UP key) */
#define ANIM_ATTACK      6   /* punch */
#define ANIM_KICK        7
#define ANIM_SHOOT       8
#define ANIM_CROUCH      9
#define ANIM_DEAD       10

/* ── max frames per anim (for offset table) ── */
#define MAX_ANIM_FRAMES 16

/* ── sprite-sheet frame dimensions ── */
/* Measured from actual pixel content in batman_right_1.png / batman_left_1.png */
#define BAT_FRAME_W    448   /* ~average frame cell width  (right sheet 3136/7) */
#define BAT_FRAME_H    484   /* ~average frame cell height (5325/11)            */
#define CAT_FRAME_W    448
#define CAT_FRAME_H    484

/* ── screen ── */
#define SCREEN_W       900
#define SCREEN_H       620
#define GROUND_Y       190   /* p->y when standing: feet at pixel 490 (190+300=490) */

/* ── movement speeds ── */
#define SPEED        220.0
#define SPEED_RUN    380.0

/* ── jump / physics tuning ── */
#define JUMP_VY        -600.0f   /* initial upward velocity (pixels/s, negative = up) */
#define JUMP_GRAVITY    1400.0f  /* gravity during jump (pixels/s²)                   */
#define JUMP_CUT_MULT     0.45f  /* multiply vy when button released early (var height)*/
#define HOP_VY         -480.0f   /* in-place UP hop initial velocity                  */

/* coyote time & jump buffer (in seconds) */
#define COYOTE_TIME     0.08f
#define JUMP_BUFFER     0.10f

/* keep old names as aliases so nothing else breaks */
#define SAUT_VX          1.2
#define SAUT_X_MIN     -50.0
#define SAUT_X_MAX      50.0
#define UP_X_MIN       -31.6
#define UP_X_MAX        31.6

/* ── costumes per character ── */
#define COSTUME_COUNT  3

/* ────────────────────────────────────────────────────────── */

typedef struct {
    SDL_Texture *sheetRight;
    SDL_Texture *sheetLeft;
    int          frameW;
    int          frameH;
    /* per-animation: number of frames and their x-offsets in the sheet */
    int          frameCounts[ANIM_COUNT];
    int          frameOffsets[ANIM_COUNT][MAX_ANIM_FRAMES]; /* x pixel offset of each frame */
    int          frameWidths[ANIM_COUNT][MAX_ANIM_FRAMES];  /* pixel width  of each frame   */
    int          frameRows[ANIM_COUNT];                     /* absolute Y pixel offset of the animation row in the sheet */
    int          frameHeights[ANIM_COUNT];                  /* pixel height of each animation row */
} SpriteData;

typedef struct {
    float x, y, vx;
    int   active, owner;
} Bullet;

typedef struct {
    SDL_Scancode left, right, up, jump;
    SDL_Scancode punch, kick, shoot, crouch;
    /* 'fly' is an alias for jump — kept for menu compatibility */
    SDL_Scancode fly;
} InputConfig;

/* Default key bindings */
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
    /* physics */
    double     x, y;
    double     vitesse;
    double     acceleration;
    float      vy;
    int        onGround;

    /* jump state */
    int        jumping;        /* in the air from a jump */
    double     posinit_y;      /* kept for compat */
    double     saut_x_rel;     /* kept for compat */
    int        canDoubleJump;
    int        jumpPressed;    /* to detect rising edge */
    int        jumpCut;        /* variable height: vy cut already applied */
    float      coyoteTimer;    /* seconds since left ground (grace period) */
    float      jumpBuffer;     /* seconds since jump pressed while in air */

    /* vertical up-hop state */
    int        hopping;        /* doing the in-place UP hop */
    double     hop_posinit_y;
    double     hop_x_rel;

    /* flying (Batman only) */
    int        isFlying;

    /* screen rect */
    SDL_Rect   posScreen;
    int        w, h;

    /* direction: 1=right, 0=left */
    int        direction;

    /* animation */
    int        animState;
    int        animFrame;
    float      animTimer;
    float      animSpeed;
    SpriteData sprite;

    /* actions */
    int        isAttacking, isKicking, isShooting;
    int        isCrouching, isRunning;
    int        actionTimer;

    /* game state */
    int        vies, hp, isAlive, score;
    int        isCat, costume;

    Bullet     bullets[MAX_BULLETS];
} Player;

/* ── public API ── */
void initSpriteData(SpriteData *s, SDL_Texture *right, SDL_Texture *left,
                    int frameW, int frameH, int isCat);
void initPlayer(Player *p, float x, float y, int isCat);
void animatePlayer(Player *p, float dt);
void blitPlayer(SDL_Renderer *r, Player *p);
void updatePlayer(Player *p, float dt);
void drawHUD(SDL_Renderer *r, TTF_Font *font, Player *p1, Player *p2);

#endif /* PLAYER_H */
