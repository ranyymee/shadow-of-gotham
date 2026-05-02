#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

/* ═══════════════════════════════════════════════════════════════════════
   CONSTANTS
═══════════════════════════════════════════════════════════════════════ */
#define COSTUME_COUNT   3
#define ANIM_COUNT      9
#define MAX_BULLETS     8

/* ═══════════════════════════════════════════════════════════════════════
   ANIMATION STATE ENUMS
   Both characters share the same row indices (0-8) so existing
   updatePlayer() logic that sets `row` needs zero changes.
═══════════════════════════════════════════════════════════════════════ */
typedef enum {
    BAT_IDLE=0, BAT_WALK=1, BAT_RUN=2, BAT_JUMP=3,
    BAT_ATTACK=4, BAT_KICK=5, BAT_SHOOT=6, BAT_FLY=7, BAT_DEAD=8
} BatAnimState;

typedef enum {
    CAT_IDLE=0, CAT_WALK=1, CAT_RUN=2, CAT_JUMP=3,
    CAT_ATTACK=4, CAT_KICK=5, CAT_SHOOT=6, CAT_DOUBLEJUMP=7, CAT_DEAD=8
} CatAnimState;

/* ═══════════════════════════════════════════════════════════════════════
   AnimReel  — one animation strip (right-facing + left-facing textures)
   
   Layout on each texture:
     frames are laid out horizontally: [f0][f1][f2]...[fN-1]
     texture total width  = frameW * frameCount
     texture total height = frameH
     
   This replaces the old "row N on a shared sheet" approach.
   Each reel is independent: different sizes, speeds, loop modes.
═══════════════════════════════════════════════════════════════════════ */
typedef struct {
    SDL_Texture *right;        /* horizontal strip, right-facing          */
    SDL_Texture *left;         /* horizontal strip, left-facing           */
    int          frameW;       /* width  of one frame (pixels)            */
    int          frameH;       /* height of one frame (pixels)            */
    int          frameCount;   /* total frames in the strip               */
    float        fps;          /* playback speed  (frames per second)     */
    int          loop;         /* 1 = loop forever,  0 = hold last frame  */
} AnimReel;

/* ═══════════════════════════════════════════════════════════════════════
   SpriteData  — table of ANIM_COUNT reels for one character/costume
   
   OLD: one big sheet + frameCounts[] array
   NEW: one AnimReel per animation state, fully independent
═══════════════════════════════════════════════════════════════════════ */
typedef struct {
    AnimReel reels[ANIM_COUNT];  /* indexed by BatAnimState / CatAnimState */
} SpriteData;

/* ═══════════════════════════════════════════════════════════════════════
   AnimState  — runtime cursor (lives on Player, not on SpriteData)
   
   Separating mutable playback state from the shared data table means
   two players can share a SpriteData without stomping each other's frame.
═══════════════════════════════════════════════════════════════════════ */
typedef struct {
    int   row;          /* current animation index (0-8)                  */
    int   frame;        /* current frame within the strip                 */
    float timer;        /* accumulated time since last frame advance (s)  */
    int   finished;     /* 1 when a non-looping anim has reached the end  */
} AnimState;

/* ═══════════════════════════════════════════════════════════════════════
   Bullet, InputConfig  (unchanged from original)
═══════════════════════════════════════════════════════════════════════ */
typedef struct {
    float x, y, vx;
    int   active, owner;
} Bullet;

typedef struct {
    SDL_Scancode left, right, up, fly;
    SDL_Scancode punch, kick, shoot, crouch;
} InputConfig;

static inline InputConfig defaultInputP1(void) {
    InputConfig c;
    c.left   = SDL_SCANCODE_LEFT;  c.right  = SDL_SCANCODE_RIGHT;
    c.up     = SDL_SCANCODE_UP;    c.fly    = SDL_SCANCODE_SPACE;
    c.punch  = SDL_SCANCODE_KP_1;  c.kick   = SDL_SCANCODE_KP_2;
    c.shoot  = SDL_SCANCODE_KP_3;  c.crouch = SDL_SCANCODE_DOWN;
    return c;
}
static inline InputConfig defaultInputP2(void) {
    InputConfig c;
    c.left   = SDL_SCANCODE_A;  c.right  = SDL_SCANCODE_D;
    c.up     = SDL_SCANCODE_W;  c.fly    = SDL_SCANCODE_W;
    c.punch  = SDL_SCANCODE_J;  c.kick   = SDL_SCANCODE_K;
    c.shoot  = SDL_SCANCODE_L;  c.crouch = SDL_SCANCODE_S;
    return c;
}
#define DEFAULT_INPUT_P1 defaultInputP1()
#define DEFAULT_INPUT_P2 defaultInputP2()

/* ═══════════════════════════════════════════════════════════════════════
   Player  — identical fields to original, except:
     • SpriteData no longer holds posSprite (that's in AnimState)
     • animRow / animFrame / animTimer / animSpeed replaced by AnimState anim
═══════════════════════════════════════════════════════════════════════ */
typedef struct {
    /* ── physics ── */
    double x, y;
    double vitesse;
    double acceleration;
    double vy;
    int    onGround;
    int    up;
    double posinit_y;
    double saut_x_rel;
    int    canDoubleJump;
    int    jumpPressed;
    int    isFlying;

    /* ── rendering ── */
    SDL_Rect   posScreen;
    int        direction;      /* 1 = right, 0 = left */
    AnimState  anim;           /* replaces animRow/animFrame/animTimer/animSpeed */
    SpriteData sprite;         /* reel table (shared data, no mutable cursor)    */
    int        w, h;           /* draw size (may differ from frameW/frameH)      */

    /* ── actions ── */
    int isAttacking, isKicking, isShooting, isCrouching, isRunning;
    int actionTimer;

    /* ── game state ── */
    int vies, hp, isAlive, score;
    int isCat, costume;

    Bullet bullets[MAX_BULLETS];
} Player;

/* ═══════════════════════════════════════════════════════════════════════
   PUBLIC API
═══════════════════════════════════════════════════════════════════════ */

/*
 * initAnimReel()
 *   Populate one AnimReel slot.
 *
 *   right/left   — pre-loaded horizontal strip textures (may be NULL
 *                  if you share sheets; NULL falls back to a colour rect)
 *   frameW/H     — size of ONE frame inside the strip
 *   frameCount   — how many frames are in the strip
 *   fps          — playback speed (8.0 for idle, 12.0 for walk, etc.)
 *   loop         — 1 = looping,  0 = one-shot (hold last frame)
 */
void initAnimReel(AnimReel *reel,
                  SDL_Texture *right, SDL_Texture *left,
                  int frameW, int frameH,
                  int frameCount, float fps, int loop);

/*
 * initSpriteData()
 *   Backward-compatible helper: fills all ANIM_COUNT reels from a single
 *   right/left spritesheet laid out as rows (original format).
 *   Each row is frameW*frameCount wide, frameH tall.
 *   frameCounts[] gives per-row frame count (NULL → defaults to 4).
 *   fps[]         gives per-row fps         (NULL → defaults to 12.0).
 *   loop[]        gives per-row loop flag    (NULL → defaults to 1 except
 *                 ATTACK/KICK/SHOOT/DEAD which default to 0).
 */
void initSpriteData(SpriteData *s,
                    SDL_Texture *right, SDL_Texture *left,
                    int frameW, int frameH, int isCat,
                    const int   frameCounts[ANIM_COUNT],   /* NULL = defaults */
                    const float fps       [ANIM_COUNT],    /* NULL = defaults */
                    const int   loop      [ANIM_COUNT]);   /* NULL = defaults */

/* Core player lifecycle */
void initPlayer   (Player *p, float x, float y, int isCat);
void handleInput  (Player *p, const Uint8 *keys, InputConfig cfg);
void updatePlayer (Player *p, float dt, SDL_Rect platforms[], int nbPlatforms);
void renderPlayer (SDL_Renderer *r, Player *p);
void renderBullets(SDL_Renderer *r, Player *p);

/* Lower-level helpers (still public for external use) */
void movePerso    (Player *p, float dt);
void saut         (Player *p);
void updateSaut   (Player *p, float dt);

/*
 * animateEntity() / blitEntity()
 *   Kept for backward compatibility.
 *   Internally now delegate to the AnimReel/AnimState system.
 */
void animateEntity(Player *p);
void blitEntity   (SDL_Renderer *r, Player *p);

#endif /* PLAYER_H */
