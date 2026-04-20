#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define COSTUME_COUNT   3
#define ANIM_COUNT      9
#define MAX_BULLETS     8

/* ── Lignes (rows) animation Batman ── */
typedef enum {
    BAT_IDLE=0, BAT_WALK=1, BAT_RUN=2, BAT_JUMP=3,
    BAT_ATTACK=4, BAT_KICK=5, BAT_SHOOT=6, BAT_FLY=7, BAT_DEAD=8
} BatAnimState;

/* ── Lignes (rows) animation Catwoman ── */
typedef enum {
    CAT_IDLE=0, CAT_WALK=1, CAT_RUN=2, CAT_JUMP=3,
    CAT_ATTACK=4, CAT_KICK=5, CAT_SHOOT=6, CAT_DOUBLEJUMP=7, CAT_DEAD=8
} CatAnimState;

/* ────────────────────────────────────────────
   SpriteData — données du sprite sheet
   (Atelier Animation : posSprite = SDL_Rect
    indiquant la sous-image à afficher)
──────────────────────────────────────────── */
typedef struct {
    SDL_Texture *sheetRight;   /* sprite sheet direction droite  */
    SDL_Texture *sheetLeft;    /* sprite sheet direction gauche  */
    int          frameW;       /* largeur  d'un frame (NBC)      */
    int          frameH;       /* hauteur  d'un frame (NBL)      */
    int          frameCounts[ANIM_COUNT]; /* nb frames par ligne  */
    SDL_Rect     posSprite;    /* partie du sheet a afficher     */
} SpriteData;

/* ────────────────────────────────────────────
   Balle
──────────────────────────────────────────── */
typedef struct {
    float x, y, vx;
    int   active, owner;
} Bullet;

/* ────────────────────────────────────────────
   InputConfig — touches clavier
──────────────────────────────────────────── */
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
    c.left=SDL_SCANCODE_A; c.right=SDL_SCANCODE_D;
    c.up=SDL_SCANCODE_W;   c.fly=SDL_SCANCODE_W;
    c.punch=SDL_SCANCODE_F; c.kick=SDL_SCANCODE_G;
    c.shoot=SDL_SCANCODE_H; c.crouch=SDL_SCANCODE_S;
    return c;
}
#define DEFAULT_INPUT_P1 defaultInputP1()
#define DEFAULT_INPUT_P2 defaultInputP2()

/* ────────────────────────────────────────────
   Player — structure principale du personnage

   Atelier Déplacement : vitesse / acceleration en double
   Atelier Animation   : posSprite dans SpriteData + direction + posScreen
   Atelier Saut        : champ up (0=sol, 1=en saut)
──────────────────────────────────────────── */
typedef struct {
    /* --- Position et physique (Atelier Déplacement) --- */
    double x, y;            /* position précise (double pour physique) */
    double vitesse;         /* vitesse instantanée horizontale          */
    double acceleration;    /* accélération horizontale                 */
    double vy;              /* vitesse verticale                        */

    /* --- Saut (Atelier Saut) --- */
    int    onGround;        /* 1=au sol, 0=en l'air                     */
    int    up;              /* 0=au sol, 1=en saut                      */
    double posinit_y;       /* ordonnée initiale avant le saut          */
    double saut_x_rel;      /* abscisse relative dans repère du saut    */
    int    canDoubleJump;   /* Catwoman uniquement                      */
    int    jumpPressed;     /* bord montant touche saut                 */
    int    isFlying;        /* Batman vol                               */

    /* --- Affichage (Atelier Animation) --- */
    SDL_Rect  posScreen;    /* position par rapport à l'écran           */
    int       direction;    /* 1=droite, 0=gauche                       */
    int       animRow;      /* ligne courante dans le sprite sheet      */
    int       animFrame;    /* colonne courante                         */
    float     animTimer;    /* timer pour avancer les frames            */
    float     animSpeed;    /* durée par frame (secondes)               */
    SpriteData sprite;      /* données sprite sheet                     */

    /* --- Dimensions --- */
    int w, h;

    /* --- Etats combat --- */
    int isAttacking, isKicking, isShooting, isCrouching, isRunning;
    int actionTimer;

    /* --- Vie / score --- */
    int vies, hp, isAlive, score;

    /* --- Type --- */
    int isCat, costume;

    /* --- Balles --- */
    Bullet bullets[MAX_BULLETS];
} Player;

/* ────────────────────────────────────────────
   Fonctions publiques
   (noms conformes aux ateliers)
──────────────────────────────────────────── */

/* Atelier Animation */
void initSpriteData  (SpriteData *s, SDL_Texture *right, SDL_Texture *left,
                      int frameW, int frameH, int isCat);
void animateEntity   (Player *p);                  /* avance la frame              */
void blitEntity      (SDL_Renderer *r, Player *p); /* affiche le personnage        */

/* Initialisation */
void initPlayer      (Player *p, float x, float y, int isCat);

/* Atelier Déplacement */
void movePerso       (Player *p, float dt);        /* applique vitesse+acceleration */

/* Atelier Saut */
void saut            (Player *p);                  /* déclenche le saut             */
void updateSaut      (Player *p, float dt);        /* met à jour la trajectoire     */

/* Boucle jeu */
void handleInput     (Player *p, const Uint8 *keys, InputConfig cfg);
void updatePlayer    (Player *p, float dt);

/* Rendu personnage (alias blitEntity) */
void renderPlayer    (SDL_Renderer *r, Player *p);

/* Rendu balles */
void renderBullets   (SDL_Renderer *r, Player *p);

#endif
