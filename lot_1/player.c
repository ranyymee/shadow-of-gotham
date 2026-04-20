#include "player.h"
#include <string.h>
#include <stdio.h>

#define SPEED        220.0
#define SPEED_RUN    380.0
#define ACCEL          0.0
#define GRAVITY      900.0f
#define GROUND_Y_VAL 460.0
#define ACTION_DUR    20
#define BULLET_SPEED 500.0f
#define ANIM_SPD     0.1f

#define SAUT_C       100.0
#define SAUT_A      -0.04
#define SAUT_X_MIN  -50.0
#define SAUT_X_MAX   50.0
#define SAUT_VX       1.2

void initSpriteData(SpriteData *s, SDL_Texture *right, SDL_Texture *left, int frameW, int frameH)
{
    int i;
    s->sheetRight = right;
    s->sheetLeft  = left;
    s->frameW     = frameW;
    s->frameH     = frameH;

    if (right) SDL_SetTextureBlendMode(right, SDL_BLENDMODE_BLEND);
    if (left)  SDL_SetTextureBlendMode(left,  SDL_BLENDMODE_BLEND);

    s->posSprite.x = 0;
    s->posSprite.y = 0;
    s->posSprite.w = frameW;
    s->posSprite.h = frameH;

    for (i = 0; i < ANIM_COUNT; i++)
        s->frameCounts[i] = 4;
    s->frameCounts[ANIM_IDLE] = 1;
}

void initPlayer(Player *p, float x, float y, int isCat)
{
    int i;
    memset(p, 0, sizeof(Player));

    p->x            = (double)x;
    p->y            = (double)y;
    p->vitesse      = 0.0;
    p->acceleration = ACCEL;
    p->vy           = 0.0f;

    p->w            = 100;
    p->h            = 150;
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

    p->onGround = 1;
    p->vies     = 3;
    p->hp       = 100;
    p->isAlive  = 1;
    p->score    = 0;
    p->isCat    = isCat;

    for (i = 0; i < MAX_BULLETS; i++)
        p->bullets[i].active = 0;
}

void animatePlayer(Player *p)
{
    int maxF;
    p->sprite.posSprite.y = p->animRow * p->sprite.frameH;
    maxF = p->sprite.frameCounts[p->animRow];
    if (maxF < 1) maxF = 1;
    p->animFrame++;
    if (p->animFrame >= maxF)
        p->animFrame = 0;
    p->sprite.posSprite.x = p->animFrame * p->sprite.frameW;
}

void blitPlayer(SDL_Renderer *r, Player *p)
{
    SDL_Texture *sheet;
    SDL_Rect src;

    if (!p->isAlive) return;

    sheet = p->direction ? p->sprite.sheetRight : p->sprite.sheetLeft;

    p->posScreen.x = (int)p->x;
    p->posScreen.y = (int)p->y;
    p->posScreen.w = p->w;
    p->posScreen.h = p->h;

    if (sheet && p->sprite.frameW > 0 && p->sprite.frameH > 0) {
        src.x = p->sprite.posSprite.x;
        src.y = p->sprite.posSprite.y;
        src.w = p->sprite.posSprite.w;
        src.h = p->sprite.posSprite.h;
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

void updatePlayer(Player *p, float dt)
{
    int row;
    float grav;
    double dx;
    int i;

    if (!p->isAlive) return;

    if (p->actionTimer > 0) {
        p->actionTimer--;
        if (p->actionTimer == 0) {
            p->isAttacking = 0;
            p->isKicking   = 0;
            p->isShooting  = 0;
        }
    }

    dx = 0.5 * p->acceleration * dt * dt + p->vitesse * dt;
    p->x += dx;
    p->vitesse += p->acceleration * dt;
    p->posScreen.x = (int)p->x;
    if (p->x < 0) { p->x = 0; p->vitesse = 0; }
    if (p->x > SCREEN_W - p->w) { p->x = SCREEN_W - p->w; p->vitesse = 0; }

    if (p->up) {
        p->saut_x_rel += SAUT_VX * 60.0 * (double)dt;
        p->y = p->posinit_y - (SAUT_A * p->saut_x_rel * p->saut_x_rel + SAUT_C);
        if (p->saut_x_rel >= SAUT_X_MAX) {
            p->y        = p->posinit_y;
            p->up       = 0;
            p->onGround = 1;
            p->canDoubleJump = p->isCat;
        }
        p->posScreen.y = (int)p->y;
    } else {
        if (!p->onGround) {
            grav = (p->isFlying && !p->isCat) ? GRAVITY * 0.3f : GRAVITY;
            p->vy += grav * dt;
            if (p->vy > 700) p->vy = 700;
            p->y += p->vy * dt;
        }
        if (p->y >= GROUND_Y_VAL) {
            p->y         = GROUND_Y_VAL;
            p->vy        = 0;
            p->onGround  = 1;
            p->canDoubleJump = p->isCat;
            p->isFlying  = 0;
        }
    }

    if (p->y < 0) { p->y = 0; p->vy = 0; }

    if      (p->isAttacking)          row = ANIM_ATTACK;
    else if (p->isKicking)            row = ANIM_KICK;
    else if (p->isShooting)           row = ANIM_SHOOT;
    else if (p->isFlying)             row = ANIM_FLY;
    else if (p->up || !p->onGround)   row = ANIM_JUMP;
    else if (p->isRunning)            row = ANIM_RUN;
    else if (p->vitesse != 0.0)       row = ANIM_WALK;
    else                              row = ANIM_IDLE;

    if (p->animRow != row) {
        p->animRow            = row;
        p->animFrame          = 0;
        p->animTimer          = 0.0f;
        p->sprite.posSprite.x = 0;
        p->sprite.posSprite.y = row * p->sprite.frameH;
    }

    p->animTimer += dt;
    if (p->animTimer >= p->animSpeed) {
        p->animTimer = 0.0f;
        animatePlayer(p);
    }

    for (i = 0; i < MAX_BULLETS; i++) {
        if (!p->bullets[i].active) continue;
        p->bullets[i].x += p->bullets[i].vx * dt;
        if (p->bullets[i].x < 0 || p->bullets[i].x > SCREEN_W)
            p->bullets[i].active = 0;
    }
}

void drawHUD(SDL_Renderer *r, TTF_Font *font, Player *p1, Player *p2)
{
    char buf[80];
    SDL_Surface *s;
    SDL_Texture *t;
    SDL_Rect d, bg1, fg1, bg2, fg2;
    SDL_Color cyan    = {  0, 220, 220, 255};
    SDL_Color magenta = {220,  60, 220, 255};
    int hp1w, hp2w;

    if (!font) return;

    snprintf(buf, sizeof(buf), "BATMAN   Vies:%d  Score:%d", p1->vies, p1->score);
    s = TTF_RenderUTF8_Blended(font, buf, cyan);
    if (s) {
        t = SDL_CreateTextureFromSurface(r, s);
        d.x = 10; d.y = 8; d.w = s->w; d.h = s->h;
        SDL_RenderCopy(r, t, NULL, &d);
        SDL_FreeSurface(s); SDL_DestroyTexture(t);
    }

    bg1.x = 10; bg1.y = 30; bg1.w = 200; bg1.h = 12;
    hp1w = p1->hp * 2; if (hp1w < 0) hp1w = 0;
    fg1.x = 10; fg1.y = 30; fg1.w = hp1w; fg1.h = 12;
    SDL_SetRenderDrawColor(r,  40,  0,  0, 255); SDL_RenderFillRect(r, &bg1);
    SDL_SetRenderDrawColor(r, 220, 60, 60, 255); SDL_RenderFillRect(r, &fg1);
    SDL_SetRenderDrawColor(r, 180,180,180, 255); SDL_RenderDrawRect(r, &bg1);

    snprintf(buf, sizeof(buf), "CATWOMAN  Vies:%d  Score:%d", p2->vies, p2->score);
    s = TTF_RenderUTF8_Blended(font, buf, magenta);
    if (s) {
        t = SDL_CreateTextureFromSurface(r, s);
        d.x = SCREEN_W - s->w - 10; d.y = 8; d.w = s->w; d.h = s->h;
        SDL_RenderCopy(r, t, NULL, &d);
        SDL_FreeSurface(s); SDL_DestroyTexture(t);
    }

    bg2.x = SCREEN_W - 210; bg2.y = 30; bg2.w = 200; bg2.h = 12;
    hp2w = p2->hp * 2; if (hp2w < 0) hp2w = 0;
    fg2.x = SCREEN_W - 210; fg2.y = 30; fg2.w = hp2w; fg2.h = 12;
    SDL_SetRenderDrawColor(r,  40,  0, 40, 255); SDL_RenderFillRect(r, &bg2);
    SDL_SetRenderDrawColor(r, 200, 60,220, 255); SDL_RenderFillRect(r, &fg2);
    SDL_SetRenderDrawColor(r, 180,180,180, 255); SDL_RenderDrawRect(r, &bg2);
}
