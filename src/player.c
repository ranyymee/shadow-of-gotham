#include "../include/player.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════
   Constantes physique / jeu
═══════════════════════════════════════════════════ */
#define SPEED        220.0
#define SPEED_RUN    380.0
#define ACCEL          0.0
#define GRAVITY      900.0f
#define FLY_FORCE   -200.0f
#define JUMP_FORCE  -480.0f
/* GROUND_Y adapte a la hauteur du background (427px) - h_perso(150) - marge */
#define GROUND_Y     267.0
#define SCREEN_W     1000
#define ACTION_DUR    20
#define BULLET_SPEED 500.0f
#define ANIM_SPD     0.1f

/* Saut parabolique */
#define SAUT_C      100.0
#define SAUT_A      -0.04
#define SAUT_X_MIN  -50.0
#define SAUT_X_MAX   50.0
#define SAUT_VX       1.2

/* ═══════════════════════════════════════════════════
   Dimensions sprite sheets (mesurees sur les PNG)
   Batman:   688 x 1568  =>  4 cols x 9 rows  =>  frameW=172  frameH=174
   Catwoman: 672 x 1550  =>  4 cols x 9 rows  =>  frameW=168  frameH=172
═══════════════════════════════════════════════════ */
#define BAT_FRAME_W  172
#define BAT_FRAME_H  174
#define CAT_FRAME_W  168
#define CAT_FRAME_H  172

/* ═══════════════════════════════════════════════════
   initSpriteData
═══════════════════════════════════════════════════ */
void initSpriteData(SpriteData *s,
                    SDL_Texture *right, SDL_Texture *left,
                    int frameW, int frameH, int isCat)
{
    s->sheetRight = right;
    s->sheetLeft  = left;
    s->frameW     = frameW;
    s->frameH     = frameH;

    /* CRUCIAL : transparence alpha — sans ca les pixels blancs du fond
       du sprite sheet apparaissent comme des lignes blanches en jeu   */
    if (right) SDL_SetTextureBlendMode(right, SDL_BLENDMODE_BLEND);
    if (left)  SDL_SetTextureBlendMode(left,  SDL_BLENDMODE_BLEND);

    s->posSprite.x = 0;
    s->posSprite.y = 0;
    s->posSprite.w = frameW;
    s->posSprite.h = frameH;

    /* Tous les rows ont 4 frames */
    for (int i = 0; i < ANIM_COUNT; i++)
        s->frameCounts[i] = 4;
    s->frameCounts[0] = 1;   /* IDLE = 1 frame fixe */
    if (isCat) s->frameCounts[7] = 4;  /* Catwoman DOUBLEJUMP = 4 frames */
}

/* ═══════════════════════════════════════════════════
   initPlayer
═══════════════════════════════════════════════════ */
void initPlayer(Player *p, float x, float y, int isCat)
{
    memset(p, 0, sizeof(Player));

    p->x            = (double)x;
    p->y            = (double)y;
    p->vitesse      = 0.0;
    p->acceleration = ACCEL;
    p->vy           = 0.0f;

    p->w            = 130;
    p->h            = 160;
    p->posScreen.x  = (int)x;
    p->posScreen.y  = (int)y;
    p->posScreen.w  = p->w;
    p->posScreen.h  = p->h;

    p->up            = 0;
    p->posinit_y     = (double)y;
    p->saut_x_rel    = SAUT_X_MIN;
    p->canDoubleJump = isCat;
    p->jumpPressed   = 0;
    p->isFlying      = 0;

    p->direction  = 1;
    p->animRow    = 0;
    p->animFrame  = 0;
    p->animTimer  = 0.0f;
    p->animSpeed  = ANIM_SPD;

    p->onGround   = 1;
    p->vies       = 3;
    p->hp         = 100;
    p->isAlive    = 1;
    p->score      = 0;
    p->isCat      = isCat;
}

/* ═══════════════════════════════════════════════════
   setRow
═══════════════════════════════════════════════════ */
static void setRow(Player *p, int row)
{
    if (p->animRow == row) return;
    p->animRow   = row;
    p->animFrame = 0;
    p->animTimer = 0.0f;
    p->sprite.posSprite.x = 0;
    p->sprite.posSprite.y = row * p->sprite.frameH;
}

/* ═══════════════════════════════════════════════════
   animateEntity
═══════════════════════════════════════════════════ */
void animateEntity(Player *p)
{
    int maxF = p->sprite.frameCounts[p->animRow];
    if (maxF < 1) maxF = 1;

    p->sprite.posSprite.y = p->animRow * p->sprite.frameH;
    p->animFrame++;
    if (p->animFrame >= maxF)
        p->animFrame = 0;
    p->sprite.posSprite.x = p->animFrame * p->sprite.frameW;
}

/* ═══════════════════════════════════════════════════
   blitEntity
═══════════════════════════════════════════════════ */
void blitEntity(SDL_Renderer *r, Player *p)
{
    if (!p->isAlive) return;

    SDL_Texture *sheet = p->direction
                         ? p->sprite.sheetRight
                         : p->sprite.sheetLeft;

    p->posScreen.x = (int)p->x;
    p->posScreen.y = (int)p->y;
    p->posScreen.w = p->w;
    p->posScreen.h = p->h;

    if (sheet && p->sprite.frameW > 0 && p->sprite.frameH > 0) {
        SDL_Rect src = {
            p->sprite.posSprite.x,
            p->sprite.posSprite.y,
            p->sprite.posSprite.w,
            p->sprite.posSprite.h
        };
        SDL_RenderCopy(r, sheet, &src, &p->posScreen);
    } else if (sheet) {
        SDL_RenderCopy(r, sheet, NULL, &p->posScreen);
    } else {
        SDL_SetRenderDrawColor(r, p->isCat ? 210 : 80,
                                  p->isCat ?  50 : 80,
                                  p->isCat ? 210 : 200, 255);
        SDL_RenderFillRect(r, &p->posScreen);
    }
}

void renderPlayer(SDL_Renderer *r, Player *p) { blitEntity(r, p); }

/* ═══════════════════════════════════════════════════
   fireBullet
═══════════════════════════════════════════════════ */
static void fireBullet(Player *p, int ownerIdx)
{
    int i;
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!p->bullets[i].active) {
            p->bullets[i].active = 1;
            p->bullets[i].owner  = ownerIdx;
            p->bullets[i].x      = (float)(p->x + (p->direction ? p->w : 0));
            p->bullets[i].y      = (float)(p->y + p->h / 2);
            p->bullets[i].vx     = p->direction ? BULLET_SPEED : -BULLET_SPEED;
            break;
        }
    }
}

/* ═══════════════════════════════════════════════════
   movePerso
═══════════════════════════════════════════════════ */
void movePerso(Player *p, float dt)
{
    double dx = 0.5 * p->acceleration * dt * dt + p->vitesse * dt;
    p->x += dx;
    p->vitesse += p->acceleration * dt;
    p->posScreen.x = (int)p->x;

    if (p->x < 0) { p->x = 0; p->vitesse = 0; }
    if (p->x > SCREEN_W - p->w) { p->x = SCREEN_W - p->w; p->vitesse = 0; }
}

/* ═══════════════════════════════════════════════════
   saut
═══════════════════════════════════════════════════ */
void saut(Player *p)
{
    if (p->up != 0) return;
    p->up        = 1;
    p->posinit_y = p->y;
    p->saut_x_rel = SAUT_X_MIN;
}

/* ═══════════════════════════════════════════════════
   updateSaut
═══════════════════════════════════════════════════ */
void updateSaut(Player *p, float dt)
{
    double y_rel;
    if (p->up == 0) return;

    p->saut_x_rel += SAUT_VX * 60.0 * (double)dt;
    y_rel = SAUT_A * p->saut_x_rel * p->saut_x_rel + SAUT_C;
    p->y  = p->posinit_y - y_rel;

    if (p->saut_x_rel >= SAUT_X_MAX) {
        p->y  = p->posinit_y;
        p->up = 0;
    }
    p->posScreen.y = (int)p->y;
}

/* ═══════════════════════════════════════════════════
   handleInput
═══════════════════════════════════════════════════ */
void handleInput(Player *p, const Uint8 *keys, InputConfig cfg)
{
    int moving;
    double spd;

    if (!p->isAlive) return;
    if (p->actionTimer > 0) { p->vitesse = 0; return; }

    p->vitesse     = 0.0;
    p->isCrouching = 0;
    p->isRunning   = 0;

    if (p->onGround && keys[cfg.crouch])
        p->isCrouching = 1;

    if (!p->isCrouching) {
        moving = 0;
        if (keys[cfg.left])  { p->direction = 0; moving = 1; }
        if (keys[cfg.right]) { p->direction = 1; moving = 1; }
        if (moving) {
            spd = keys[cfg.crouch] ? SPEED_RUN : SPEED;
            p->vitesse = p->direction ? spd : -spd;
            if (keys[cfg.crouch]) p->isRunning = 1;
        }
    }

    if (keys[cfg.up]) {
        if (!p->jumpPressed) {
            if (p->onGround) {
                saut(p);
                p->onGround      = 0;
                p->canDoubleJump = p->isCat;
                p->jumpPressed   = 1;
            } else if (p->isCat && p->canDoubleJump) {
                saut(p);
                p->canDoubleJump = 0;
                p->jumpPressed   = 1;
            }
        }
    } else {
        p->jumpPressed = 0;
    }

    if (!p->isCat && keys[cfg.fly]) {
        p->isFlying = 1;
        p->vy       = FLY_FORCE;
        p->onGround = 0;
    } else if (!p->isCat && !keys[cfg.fly]) {
        p->isFlying = 0;
    }

    if (keys[cfg.punch] && !p->isAttacking && !p->isKicking && !p->isShooting) {
        p->isAttacking = 1;
        p->actionTimer = ACTION_DUR;
        p->vitesse = 0;
    }
    if (keys[cfg.kick] && !p->isKicking && !p->isAttacking && !p->isShooting) {
        p->isKicking   = 1;
        p->actionTimer = ACTION_DUR;
        p->vitesse = 0;
    }
    if (keys[cfg.shoot] && !p->isShooting && !p->isAttacking && !p->isKicking) {
        p->isShooting  = 1;
        p->actionTimer = ACTION_DUR;
        fireBullet(p, p->isCat ? 1 : 0);
        p->vitesse = 0;
    }
}

/* ═══════════════════════════════════════════════════
   updatePlayer  — avec collision plateformes
═══════════════════════════════════════════════════ */
void updatePlayer(Player *p, float dt,
                  SDL_Rect platforms[], int nbPlatforms)
{
    int row, i;
    float grav;
    SDL_Rect pRect, plat;
    int feetY, platTop;

    if (!p->isAlive) return;

    if (p->actionTimer > 0) {
        p->actionTimer--;
        if (p->actionTimer == 0) {
            p->isAttacking = 0;
            p->isKicking   = 0;
            p->isShooting  = 0;
        }
    }

    movePerso(p, dt);

    /* ── Physique verticale ── */
    if (p->up) {
        updateSaut(p, dt);
        if (p->up == 0) {
            p->y        = p->posinit_y;
            p->onGround = 1;
            p->canDoubleJump = p->isCat;
        }
    } else {
        if (!p->onGround) {
            grav = (p->isFlying && !p->isCat) ? GRAVITY * 0.3f : GRAVITY;
            p->vy += grav * dt;
            if (p->vy > 700) p->vy = 700;
            p->y += p->vy * dt;
        }

        /* ── Collision sol ── */
        if (p->y >= GROUND_Y) {
            p->y        = GROUND_Y;
            p->vy       = 0;
            p->onGround = 1;
            p->canDoubleJump = p->isCat;
            p->isFlying = 0;
        }

        /* ── Collision plateformes ── */
        if (p->vy >= 0 && platforms != NULL) {
            pRect.x = (int)p->x;
            pRect.y = (int)p->y;
            pRect.w = p->w;
            pRect.h = p->h;

            feetY = pRect.y + pRect.h;

            for (i = 0; i < nbPlatforms; i++) {
                plat = platforms[i];
                /* Le joueur doit tomber SUR la plateforme par le dessus */
                if (pRect.x + pRect.w <= plat.x ||
                    pRect.x >= plat.x + plat.w)
                    continue;

                platTop = plat.y;

                /* Verifie si les pieds traversent le dessus de la plateforme */
                if (feetY >= platTop &&
                    feetY <= platTop + plat.h + 20 &&
                    (int)p->y < platTop)
                {
                    p->y        = (double)(platTop - p->h);
                    p->vy       = 0;
                    p->onGround = 1;
                    p->canDoubleJump = p->isCat;
                    p->isFlying = 0;
                    break;
                }
            }
        }
    }

    if (p->y < 0) { p->y = 0; p->vy = 0; }

    /* ── Choix ligne animation ── */
    if (p->isAttacking)
        row = p->isCat ? CAT_ATTACK : BAT_ATTACK;
    else if (p->isKicking)
        row = p->isCat ? CAT_KICK   : BAT_KICK;
    else if (p->isShooting)
        row = p->isCat ? CAT_SHOOT  : BAT_SHOOT;
    else if (!p->isCat && p->isFlying)
        row = BAT_FLY;
    else if (p->isCat && p->up && !p->canDoubleJump)
        row = CAT_DOUBLEJUMP;
    else if (p->up || !p->onGround)
        row = p->isCat ? CAT_JUMP  : BAT_JUMP;
    else if (p->isRunning)
        row = p->isCat ? CAT_RUN   : BAT_RUN;
    else if (p->vitesse != 0.0)
        row = p->isCat ? CAT_WALK  : BAT_WALK;
    else
        row = p->isCat ? CAT_IDLE  : BAT_IDLE;

    setRow(p, row);

    p->animTimer += dt;
    if (p->animTimer >= p->animSpeed) {
        p->animTimer = 0.0f;
        animateEntity(p);
    }

    /* ── Balles ── */
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!p->bullets[i].active) continue;
        p->bullets[i].x += p->bullets[i].vx * dt;
        if (p->bullets[i].x < 0 || p->bullets[i].x > SCREEN_W)
            p->bullets[i].active = 0;
    }
}

/* ═══════════════════════════════════════════════════
   renderBullets
═══════════════════════════════════════════════════ */
void renderBullets(SDL_Renderer *r, Player *p)
{
    int i;
    SDL_Rect br;
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!p->bullets[i].active) continue;
        br.x = (int)p->bullets[i].x - 6;
        br.y = (int)p->bullets[i].y - 4;
        br.w = 12; br.h = 8;
        if (p->isCat)
            SDL_SetRenderDrawColor(r, 200, 50, 200, 255);
        else
            SDL_SetRenderDrawColor(r, 255, 220, 0, 255);
        SDL_RenderFillRect(r, &br);
    }
}
