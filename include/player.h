#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define COSTUME_COUNT   3
#define ANIM_COUNT      9
#define MAX_BULLETS     8

/* ── Lignes animation Batman ── */
typedef enum {
    BAT_IDLE=0, BAT_WALK=1, BAT_RUN=2, BAT_JUMP=3,
    BAT_ATTACK=4, BAT_KICK=5, BAT_SHOOT=6, BAT_FLY=7, BAT_DEAD=8
} BatAnimState;

/* ── Lignes animation Catwoman ── */
typedef enum {
    CAT_IDLE=0, CAT_WALK=1, CAT_RUN=2, CAT_JUMP=3,
    CAT_ATTACK=4, CAT_KICK=5, CAT_SHOOT=6, CAT_DOUBLEJUMP=7, CAT_DEAD=8
} CatAnimState;

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

static inline InputConfig defaultInputP1(void) {
    InputConfig c;
    c.left=SDL_SCANCODE_LEFT;  c.right=SDL_SCANCODE_RIGHT;
    c.up=SDL_SCANCODE_UP;      c.fly=SDL_SCANCODE_SPACE;
    c.punch=SDL_SCANCODE_KP_1; c.kick=SDL_SCANCODE_KP_2;
    c.shoot=SDL_SCANCODE_KP_3; c.crouch=SDL_SCANCODE_DOWN;
    return c;
}
static inline InputConfig defaultInputP2(void) {
    InputConfig c;
    c.left=SDL_SCANCODE_A;  c.right=SDL_SCANCODE_D;
    c.up=SDL_SCANCODE_W;    c.fly=SDL_SCANCODE_W;
    c.punch=SDL_SCANCODE_J; c.kick=SDL_SCANCODE_K;
    c.shoot=SDL_SCANCODE_L; c.crouch=SDL_SCANCODE_S;
    return c;
}
#define DEFAULT_INPUT_P1 defaultInputP1()
#define DEFAULT_INPUT_P2 defaultInputP2()

typedef struct {
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

    SDL_Rect   posScreen;
    int        direction;
    int        animRow;
    int        animFrame;
    float      animTimer;
    float      animSpeed;
    SpriteData sprite;

    int w, h;

    int isAttacking, isKicking, isShooting, isCrouching, isRunning;
    int actionTimer;

    int vies, hp, isAlive, score;
    int isCat, costume;

    Bullet bullets[MAX_BULLETS];
} Player;

void initSpriteData (SpriteData *s, SDL_Texture *right, SDL_Texture *left,
                     int frameW, int frameH, int isCat);
void animateEntity  (Player *p);
void blitEntity     (SDL_Renderer *r, Player *p);
void initPlayer     (Player *p, float x, float y, int isCat);
void movePerso      (Player *p, float dt);
void saut           (Player *p);
void updateSaut     (Player *p, float dt);
void handleInput    (Player *p, const Uint8 *keys, InputConfig cfg);
void updatePlayer   (Player *p, float dt,
                     SDL_Rect platforms[], int nbPlatforms);
void renderPlayer   (SDL_Renderer *r, Player *p);
void renderBullets  (SDL_Renderer *r, Player *p);

#endif
