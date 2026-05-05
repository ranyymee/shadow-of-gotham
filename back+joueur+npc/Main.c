#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "back.h"
#include "player.h"
#include "menu.h"
#include "npc.h"
#include "minimap.h"
#include "enigme.h"

/* ── Variables globales exigées par enigme.c ── */
int SCR_W = 900;
int SCR_H = 620;

#define JOUEUR_H            300
#define JOUEUR_W            100
#define DMG_PUNCH             8
#define DMG_KICK             12
#define ATK_RANGE            90
#define INVINCIBILITY_FRAMES 20
#define OBSTACLE_DAMAGE      10
#define OBSTACLE_KNOCKBACK   80

static int rectsOverlap(SDL_Rect a, SDL_Rect b)
{
    return !(a.x+a.w<=b.x || b.x+b.w<=a.x || a.y+a.h<=b.y || b.y+b.h<=a.y);
}

static void renderHUD(SDL_Renderer *r, TTF_Font *font,
                      Player *p1, Player *p2, int screenW)
{
    char         buf[80];
    SDL_Surface *surf;
    SDL_Texture *tex;
    SDL_Rect     d, bg1, fg1, bg2, fg2;
    SDL_Color    cyan    = {  0, 220, 220, 255 };
    SDL_Color    magenta = {220,  60, 220, 255 };
    int          hp1w, hp2w;

    if (!font) return;

    snprintf(buf, sizeof(buf), "BATMAN   Vies:%d  Score:%d", p1->vies, p1->score);
    surf = TTF_RenderUTF8_Blended(font, buf, cyan);
    if (surf) {
        tex = SDL_CreateTextureFromSurface(r, surf);
        d.x = 10; d.y = 8; d.w = surf->w; d.h = surf->h;
        SDL_RenderCopy(r, tex, NULL, &d);
        SDL_FreeSurface(surf); SDL_DestroyTexture(tex);
    }
    bg1.x = 10; bg1.y = 30; bg1.w = 200; bg1.h = 12;
    hp1w  = p1->hp * 2; if (hp1w < 0) hp1w = 0;
    fg1.x = 10; fg1.y = 30; fg1.w = hp1w; fg1.h = 12;
    SDL_SetRenderDrawColor(r,  40,  0,  0, 255); SDL_RenderFillRect(r, &bg1);
    SDL_SetRenderDrawColor(r, 220, 60, 60, 255); SDL_RenderFillRect(r, &fg1);
    SDL_SetRenderDrawColor(r, 180,180,180, 255); SDL_RenderDrawRect(r, &bg1);

    snprintf(buf, sizeof(buf), "CATWOMAN  Vies:%d  Score:%d", p2->vies, p2->score);
    surf = TTF_RenderUTF8_Blended(font, buf, magenta);
    if (surf) {
        tex = SDL_CreateTextureFromSurface(r, surf);
        d.x = screenW - surf->w - 10; d.y = 8; d.w = surf->w; d.h = surf->h;
        SDL_RenderCopy(r, tex, NULL, &d);
        SDL_FreeSurface(surf); SDL_DestroyTexture(tex);
    }
    bg2.x = screenW - 210; bg2.y = 30; bg2.w = 200; bg2.h = 12;
    hp2w  = p2->hp * 2; if (hp2w < 0) hp2w = 0;
    fg2.x = screenW - 210; fg2.y = 30; fg2.w = hp2w; fg2.h = 12;
    SDL_SetRenderDrawColor(r,  40,  0, 40, 255); SDL_RenderFillRect(r, &bg2);
    SDL_SetRenderDrawColor(r, 200, 60,220, 255); SDL_RenderFillRect(r, &fg2);
    SDL_SetRenderDrawColor(r, 180,180,180, 255); SDL_RenderDrawRect(r, &bg2);
}

static void handleObstacleCollision(Player *p, Platform platforms[], int taille,
                                     int *invTimer, float groundEcran,
                                     Background *bg)
{
    int i;
    SDL_Rect playerRect, obstacleScreenRect;
    
    if (*invTimer > 0) {
        (*invTimer)--;
        return;
    }
    if (!p->isAlive) return;
    
    playerRect.x = (int)p->x;
    playerRect.y = (int)p->y;
    playerRect.w = p->w;
    playerRect.h = p->h;
    
    for (i = 0; i < taille; i++) {
        if (platforms[i].destroyed) continue;
        
        {
            int marginX = platforms[i].position.w / 6;
            int marginY = platforms[i].position.h / 8;
            obstacleScreenRect.x = platforms[i].position.x - (int)bg->camera_pos.x + marginX;
            obstacleScreenRect.y = platforms[i].position.y - (int)bg->camera_pos.y + marginY;
            obstacleScreenRect.w = platforms[i].position.w - marginX * 2;
            obstacleScreenRect.h = platforms[i].position.h - marginY * 2;
            if (obstacleScreenRect.w < 1) obstacleScreenRect.w = 1;
            if (obstacleScreenRect.h < 1) obstacleScreenRect.h = 1;
        }
        
        if (rectsOverlap(playerRect, obstacleScreenRect)) {
            
            int overlapLeft   = (playerRect.x + playerRect.w) - obstacleScreenRect.x;
            int overlapRight  = (obstacleScreenRect.x + obstacleScreenRect.w) - playerRect.x;
            int overlapTop    = (playerRect.y + playerRect.h) - obstacleScreenRect.y;
            int overlapBottom = (obstacleScreenRect.y + obstacleScreenRect.h) - playerRect.y;
            
            int minOverlap = overlapLeft;
            int collisionSide = 0;
            
            if (overlapRight < minOverlap) { minOverlap = overlapRight; collisionSide = 1; }
            if (overlapTop   < minOverlap) { minOverlap = overlapTop;   collisionSide = 2; }
            if (overlapBottom< minOverlap) { minOverlap = overlapBottom; collisionSide = 3; }
            
            switch(collisionSide) {
                case 0: p->x = (float)(obstacleScreenRect.x - p->w - 1); if (p->vitesse < 0) p->vitesse = 0; break;
                case 1: p->x = (float)(obstacleScreenRect.x + obstacleScreenRect.w + 1); if (p->vitesse > 0) p->vitesse = 0; break;
                case 2: p->y = (float)(obstacleScreenRect.y - p->h); p->vy = 0; p->onGround = 1; break;
                case 3: p->y = (float)(obstacleScreenRect.y + obstacleScreenRect.h); if (p->vy > 0) p->vy = 0; break;
            }
            
            p->hp -= OBSTACLE_DAMAGE;
            if (p->hp < 0) p->hp = 0;
            *invTimer = INVINCIBILITY_FRAMES;
            
            float kbDir = (p->x + p->w/2 < obstacleScreenRect.x + obstacleScreenRect.w/2) ? -1.0f : 1.0f;
            float newX = p->x + kbDir * OBSTACLE_KNOCKBACK;
            if (kbDir < 0) {
                if (newX + p->w > obstacleScreenRect.x)
                    newX = (float)(obstacleScreenRect.x - p->w - 1);
            } else {
                if (newX < obstacleScreenRect.x + obstacleScreenRect.w)
                    newX = (float)(obstacleScreenRect.x + obstacleScreenRect.w + 1);
            }
            p->x = newX;
            
            if (p->x < 0) p->x = 0;
            if (p->x > SCREEN_W - p->w) p->x = SCREEN_W - p->w;
            
            if (p->hp <= 0) {
                p->hp = 100;
                p->vies--;
                if (p->vies <= 0) {
                    p->isAlive = 0;
                    p->animState = ANIM_DEAD;
                    p->animFrame = 0;
                } else {
                    p->x = 150.0f;
                    p->y = groundEcran;
                    p->vy = 0;
                    p->onGround = 1;
                }
            }
            return;
        }
    }
}

static void handlePlayerInput(Player *p, const Uint8 *keys,
                               InputConfig *cfg, float groundEcran)
{
    (void)groundEcran;
    int left   = keys[cfg->left];
    int right  = keys[cfg->right];
    int crouch = keys[cfg->crouch];
    int run    = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];

    if (!p->isAttacking && !p->isKicking && !p->isShooting && p->actionTimer == 0) {
        if (left && !crouch) {
            p->direction = 0;
            p->isRunning = run;
            p->vitesse   = run ? -SPEED_RUN : -SPEED;
        } else if (right && !crouch) {
            p->direction = 1;
            p->isRunning = run;
            p->vitesse   = run ? SPEED_RUN : SPEED;
        } else {
            p->vitesse  = 0.0;
            p->isRunning = 0;
        }
    }

    p->isCrouching = (crouch && p->onGround) ? 1 : 0;

    if (keys[cfg->fly]) {
        if (!p->jumpPressed) {
            p->jumpPressed = 1;
            int canJump = p->onGround || (p->coyoteTimer < COYOTE_TIME);
            int canDbl  = (!p->onGround && p->canDoubleJump);

            if (canJump) {
                p->onGround      = 0;
                p->jumping       = 1;
                p->jumpCut       = 0;
                p->vy            = JUMP_VY;
                p->animState     = ANIM_JUMP;
                p->animFrame     = 0;
                p->coyoteTimer   = COYOTE_TIME + 1.0f;
                p->canDoubleJump = 1;
            } else if (canDbl) {
                p->canDoubleJump = 0;
                p->vy            = JUMP_VY * 1.5f;
                p->animState     = ANIM_DOUBLE_JUMP;
                p->animFrame     = 0;
            } else {
                p->jumpBuffer = JUMP_BUFFER;
            }
        }
    } else {
        if (p->jumpPressed && !p->onGround && p->vy < 0)
            p->vy *= JUMP_CUT_MULT;
        p->jumpPressed = 0;
    }

    if (keys[cfg->up] && p->onGround && !p->jumping && !p->hopping) {
        p->hopping  = 1;
        p->onGround = 0;
        p->vy       = HOP_VY;
        p->animState = ANIM_UP;
        p->animFrame = 0;
    }
}

static void updatePhysics(Player *p, float dt, float groundEcran)
{
    float grav;

    if (!p->onGround && !p->jumping && !p->hopping)
        p->coyoteTimer += dt;
    else if (p->onGround)
        p->coyoteTimer = 0.0f;

    if (p->jumpBuffer > 0.0f) {
        p->jumpBuffer -= dt;
        if (p->jumpBuffer < 0.0f) p->jumpBuffer = 0.0f;
    }

    if (p->actionTimer > 0) {
        p->actionTimer--;
        if (p->actionTimer == 0) {
            p->isAttacking = 0;
            p->isKicking   = 0;
            p->isShooting  = 0;
        }
    }

    grav = JUMP_GRAVITY;
    if (p->animState == ANIM_DOUBLE_JUMP && p->vy < 0)
        grav = JUMP_GRAVITY * 0.5f;
    else if (p->vy > 0)
        grav = JUMP_GRAVITY * 1.4f;

    if (!p->onGround || p->vy < 0) {
        p->vy += grav * dt;
        if (p->vy > 900.0f) p->vy = 900.0f;
        p->y  += p->vy * dt;
    }

    if (p->y >= (double)groundEcran) {
        p->y           = groundEcran;
        p->vy          = 0.0f;
        p->onGround    = 1;
        p->jumping     = 0;
        p->hopping     = 0;
        p->jumpCut     = 0;
        p->coyoteTimer = 0.0f;
        p->canDoubleJump = 1;

        if (p->jumpBuffer > 0.0f) {
            p->jumpBuffer  = 0.0f;
            p->onGround    = 0;
            p->jumping     = 1;
            p->jumpCut     = 0;
            p->vy          = JUMP_VY;
            p->animState   = ANIM_JUMP;
            p->animFrame   = 0;
        }
    }

    if (p->y < -50.0) { p->y = -50.0; if (p->vy < 0) p->vy = 0.0f; }

    {
        int i;
        for (i = 0; i < MAX_BULLETS; i++) {
            if (!p->bullets[i].active) continue;
            p->bullets[i].x += p->bullets[i].vx * dt;
            if (p->bullets[i].x < -100 || p->bullets[i].x > 4000)
                p->bullets[i].active = 0;
        }
    }
}

static void updateAnimState(Player *p, float dt)
{
    int ns;

    if (!p->isAlive) {
        if (p->animState != ANIM_DEAD) {
            p->animState = ANIM_DEAD;
            p->animFrame = 0;
            p->animTimer = 0.0f;
        }
        animatePlayer(p, dt);
        return;
    }

    ns = p->isAttacking  ? ANIM_ATTACK :
         p->isKicking    ? ANIM_KICK   :
         p->isShooting   ? ANIM_SHOOT  :
         p->hopping      ? ANIM_UP     :
         (!p->onGround && p->canDoubleJump == 0 && !p->hopping) ? ANIM_DOUBLE_JUMP :
         !p->onGround    ? ANIM_JUMP   :
         p->isCrouching  ? ANIM_CROUCH :
         p->isRunning    ? ANIM_RUN    :
         (p->vitesse != 0.0) ? ANIM_WALK : ANIM_IDLE;

    if (ns != p->animState) {
        p->animState = ns;
        p->animFrame = 0;
        p->animTimer = 0.0f;
    }

    if (p->animState == ANIM_JUMP || p->animState == ANIM_DOUBLE_JUMP) {
        if (p->animState == ANIM_DOUBLE_JUMP) {
            p->animFrame = 0;
            return;
        }
        {
            int maxF = p->sprite.frameCounts[ANIM_JUMP];
            if (maxF > 1) {
                float vy_min = JUMP_VY;
                float vy_max = -vy_min * 1.4f;
                float t = (p->vy - vy_min) / (vy_max - vy_min);
                int   target;
                if (t < 0.0f) t = 0.0f;
                if (t > 1.0f) t = 1.0f;
                target = (int)(t * (maxF - 1) + 0.5f);
                if (target > p->animFrame) p->animFrame = target;
            }
        }
        return;
    }

    animatePlayer(p, dt);
}

static void resolvePlayerCombat(Player *atk, Player *def, int *invTimer)
{
    SDL_Rect atkZone, defRect;
    int dmg;

    if (*invTimer > 0) { (*invTimer)--; return; }
    if (!atk->isAlive || !def->isAlive) return;
    if (!atk->isAttacking && !atk->isKicking) return;

    atkZone.y = (int)atk->y + atk->h / 4;
    atkZone.h = atk->h / 2;
    atkZone.w = ATK_RANGE;
    atkZone.x = atk->direction ? (int)atk->x + atk->w
                               : (int)atk->x - ATK_RANGE;

    defRect.x = (int)def->x; defRect.y = (int)def->y;
    defRect.w = def->w;      defRect.h = def->h;

    if (!rectsOverlap(atkZone, defRect)) return;

    dmg = atk->isKicking ? DMG_KICK : DMG_PUNCH;
    def->hp -= dmg;
    if (def->hp < 0) def->hp = 0;
    *invTimer = INVINCIBILITY_FRAMES;

    if (atk->direction) def->x += 20; else def->x -= 20;

    if (def->hp == 0) {
        def->hp = 100;
        def->vies--;
        atk->score += 100;
        if (def->vies <= 0) {
            def->isAlive   = 0;
            def->animState = ANIM_DEAD;
            def->animFrame = 0;
        }
        def->x = atk->direction ? (atk->x + 300.0) : (atk->x - 300.0);
        if (def->x < 0) def->x = 50.0;
        def->y    = atk->y;
        def->vy   = 0;
        def->onGround = 1;
    }
}

static SDL_Texture *loadSheet(SDL_Renderer *r, const char *path)
{
    SDL_Texture *t = IMG_LoadTexture(r, path);
    if (!t) printf("Sprite non charge : %s — %s\n", path, IMG_GetError());
    return t;
}

static void doScrolling(Background *bg, Player *p1, Player *p2,
                        int screenW, int screenH, int worldW, float dt)
{
    double dx1, dx2;
    int    sdx, newCamX;
    float  targetCamY;
    int    worldH;

    worldH = bg->partH;

    dx1 = (p1->actionTimer == 0) ? (p1->vitesse * dt) : 0.0;
    dx2 = (p2->actionTimer == 0) ? (p2->vitesse * dt) : 0.0;

    if (dx1 > 0 && p1->x >= (double)(screenW / 2) &&
        bg->camera_pos.x + screenW < worldW)
    {
        sdx = (int)(dx1 + 0.5);
        if (sdx < 1) sdx = 1;
        newCamX = bg->camera_pos.x + sdx;
        if (newCamX + screenW > worldW) newCamX = worldW - screenW;
        bg->camera_pos.x = newCamX;
    }
    else if (dx1 < 0 && p1->x <= (double)(screenW / 2) &&
             bg->camera_pos.x > 0)
    {
        sdx = (int)(-dx1 + 0.5);
        if (sdx < 1) sdx = 1;
        newCamX = bg->camera_pos.x - sdx;
        if (newCamX < 0) newCamX = 0;
        bg->camera_pos.x = newCamX;
    }
    else
    {
        p1->x += dx1;
    }

    if (dx2 > 0 && p2->x >= (double)(screenW / 2) &&
        bg->camera_pos.x + screenW < worldW)
    {
        sdx = (int)(dx2 + 0.5);
        if (sdx < 1) sdx = 1;
        newCamX = bg->camera_pos.x + sdx;
        if (newCamX + screenW > worldW) newCamX = worldW - screenW;
        bg->camera_pos.x = newCamX;
    }
    else if (dx2 < 0 && p2->x <= (double)(screenW / 2) &&
             bg->camera_pos.x > 0)
    {
        sdx = (int)(-dx2 + 0.5);
        if (sdx < 1) sdx = 1;
        newCamX = bg->camera_pos.x - sdx;
        if (newCamX < 0) newCamX = 0;
        bg->camera_pos.x = newCamX;
    }
    else
    {
        p2->x += dx2;
    }

    float highestY = (p1->y < p2->y) ? p1->y : p2->y;
    
    targetCamY = highestY + JOUEUR_H / 2 - screenH / 2;
    
    if (targetCamY < 0) targetCamY = 0;
    if (targetCamY > worldH - screenH) targetCamY = worldH - screenH;
    
    bg->camera_pos.y += (targetCamY - bg->camera_pos.y) * 0.1f;
    
    if (bg->camera_pos.y < 0) bg->camera_pos.y = 0;
    if (bg->camera_pos.y > worldH - screenH) bg->camera_pos.y = worldH - screenH;

    if (p1->x < 0)               p1->x = 0;
    if (p1->x > screenW - p1->w) p1->x = screenW - p1->w;
    if (p2->x < 0)               p2->x = 0;
    if (p2->x > screenW - p2->w) p2->x = screenW - p2->w;
}

static void doScrollingSplit(Player *p1, Player *p2,
                             int worldW, int worldH, float dt)
{
    /* En mode split, chaque joueur a sa propre camera calculee au moment
       du rendu. Ici on met juste a jour les positions absolues des joueurs
       dans le monde (pas de camera partagee a modifier).
       p.x est en coordonnees ecran relatives a la camera globale, donc
       on applique simplement le deplacement. */
    double dx1 = (p1->actionTimer == 0) ? (p1->vitesse * dt) : 0.0;
    double dx2 = (p2->actionTimer == 0) ? (p2->vitesse * dt) : 0.0;

    p1->x += (float)dx1;
    p2->x += (float)dx2;

    (void)worldH;

    /* Bornes du monde */
    if (p1->x < 0)                p1->x = 0;
    if (p1->x > worldW - p1->w)  p1->x = (float)(worldW - p1->w);
    if (p2->x < 0)                p2->x = 0;
    if (p2->x > worldW - p2->w)  p2->x = (float)(worldW - p2->w);
}

static void switchLevel(int level, SDL_Renderer *renderer,
                        Background *bg, Platform platforms[], int *taille,
                        GameNPC *gameNPC,
                        Player *p1, Player *p2,
                        int screenW, int screenH,
                        float groundEcran,
                        int *timeLeft, Uint32 *lastTime,
                        int *invP1, int *invP2,
                        int *invObstacleP1, int *invObstacleP2)
{
    int i, f;

    /* Cleanup old background textures */
    for (i = 0; i < 8; i++) if (bg->img[i]) { SDL_DestroyTexture(bg->img[i]); bg->img[i] = NULL; }
    if (bg->guide.image)        { SDL_DestroyTexture(bg->guide.image);        bg->guide.image = NULL; }
    if (bg->commentJouer.image) { SDL_DestroyTexture(bg->commentJouer.image); bg->commentJouer.image = NULL; }

    /* Cleanup old platforms */
    for (i = 0; i < *taille; i++) {
        Platform *pp = &platforms[i];
        if (pp->isAnimated) {
            for (f = 0; f < MAX_FRAMES; f++)
                if (pp->frames[f]) { SDL_DestroyTexture(pp->frames[f]); pp->frames[f] = NULL; }
        } else {
            if (pp->image) { SDL_DestroyTexture(pp->image); pp->image = NULL; }
        }
    }
    *taille = 0;

    /* Cleanup old NPCs */
    NPC_clean(gameNPC);

    /* Load new level */
    initBackgroundAndPlatforms(renderer, bg, platforms, taille, level, screenW, screenH);
    bg->camera_pos.x = 0;
    bg->camera_pos.y = 0;
    bg->camera_pos.w = screenW;
    bg->camera_pos.h = screenH;

    NPC_init(gameNPC, renderer, NULL);
    NPC_setGroundY(gameNPC, (int)groundEcran);   /* après NPC_init (memset) */
    NPC_loadLevel(gameNPC, renderer, level);
    gameNPC->pH     = &p1->hp;
    gameNPC->pScore = &p1->score;

    /* Reset players position & state (keep score & lives) */
    p1->x = 150.0f;       p1->y = groundEcran;
    p1->vitesse = 0.0;    p1->vy = 0.0f;
    p1->onGround = 1;     p1->jumping = 0;      p1->hopping = 0;
    p1->isAttacking = 0;  p1->isKicking = 0;    p1->isShooting = 0;
    p1->isCrouching = 0;  p1->isRunning = 0;
    p1->actionTimer = 0;  p1->animState = ANIM_IDLE; p1->animFrame = 0;
    p1->hp = 100;         p1->isAlive = 1;
    for (i = 0; i < MAX_BULLETS; i++) p1->bullets[i].active = 0;

    p2->x = (float)(screenW - 250); p2->y = groundEcran;
    p2->vitesse = 0.0;    p2->vy = 0.0f;
    p2->onGround = 1;     p2->jumping = 0;      p2->hopping = 0;
    p2->isAttacking = 0;  p2->isKicking = 0;    p2->isShooting = 0;
    p2->isCrouching = 0;  p2->isRunning = 0;
    p2->actionTimer = 0;  p2->animState = ANIM_IDLE; p2->animFrame = 0;
    p2->hp = 100;         p2->isAlive = 1;
    for (i = 0; i < MAX_BULLETS; i++) p2->bullets[i].active = 0;

    /* Reset timers */
    *timeLeft = 600;
    *lastTime = SDL_GetTicks();
    *invP1 = 0; *invP2 = 0;
    *invObstacleP1 = 0; *invObstacleP2 = 0;
}

static void playerAttackEnemy(GameNPC *npc, SDL_Rect *playerRect, int cx, int cy, int *playerScore)
{
    for (int i = 0; i < npc->enemyCnt; i++) {
        NPC *e = &npc->enemies[i];
        if (!e->active || e->dying) continue;
        if (SDL_HasIntersection(&e->rect, playerRect)) {
            e->health -= 25;
            if (e->health <= 0) {
                e->dying = 1;
                e->attacking = 0;
                e->frame = 0;
                e->step = 0;
                e->tex = e->death[0];
                *playerScore += e->score;
            }
            for (int j = 0; j < 20; j++) {
                for (int k = 0; k < MAX_BLOOD; k++) {
                    if (!npc->blood[k].active) {
                        npc->blood[k].x = (e->x - cx) + (rand() % 60) - 30;
                        npc->blood[k].y = (e->y - cy) + (rand() % 60) - 30;
                        npc->blood[k].life = 30;
                        npc->blood[k].active = 1;
                        break;
                    }
                }
            }
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    SDL_Window   *window   = NULL;
    SDL_Renderer *renderer = NULL;
    TTF_Font     *font     = NULL;
    Mix_Music    *music    = NULL;
    Background    bg;
    Platform      platforms[MAX_PLATFORMS];
    GameNPC       gameNPC;
    int           taille   = 0;
    int           screenW  = 0, screenH = 0;
    int           worldW   = 0;
    int           timeLeft = 600;
    int           invP1    = 0, invP2 = 0;
    int           invObstacleP1 = 0, invObstacleP2 = 0;
    int           running  = 1;
    int           splitScreen = 0;   /* 0 = ecran unique, 1 = split-screen */
    int           currentLevel = 1;
    Uint32        lastTime, prevTick;
    SDL_Event     event;
    Player        p1, p2;
    MenuResult    menuRes;
    float         groundEcran;
    Minimap      *minimap           = NULL;
    Minimap      *minimapLeft      = NULL;   /* split P1 */
    Minimap      *minimapRight     = NULL;   /* split P2 */
    ShakeState    shake             = {0, 0, 0, 0};
    MinimapEnemy  minimapEnemies[MINIMAP_MAX_ENEMIES];
    int           worldH            = 0;
    char          minimapPath[64];

    /* ── Système d'énigmes ── */
    Enigme        enigme;
    int           enigmeActive    = 0;   /* 1 = interface d'énigme affichée */
    int           enigmeDeclenche = 0;   /* garde-fou : déjà déclenché une fois */
    Uint32        enigmeStartTick = 0;
    SDL_Texture  *enigmeObjTex   = NULL; /* texture enigme.png dans le monde */
    SDL_Rect      enigmeObjRect  = { 600, 0, 80, 80 }; /* pos monde, Y fixé après groundEcran */
    int           enigmeObjActive = 1;   /* 1 = pas encore résolu/détruit */
    memset(&enigme, 0, sizeof(Enigme));

    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_AUDIO) != 0) {
        printf("SDL_Init : %s\n", SDL_GetError()); return 1; }
    if (TTF_Init() == -1) {
        printf("TTF_Init : %s\n", TTF_GetError()); SDL_Quit(); return 1; }
    if (IMG_Init(IMG_INIT_PNG|IMG_INIT_WEBP) == 0) {
        printf("IMG_Init : %s\n", IMG_GetError()); TTF_Quit(); SDL_Quit(); return 1; }

    window = SDL_CreateWindow("Batman vs Catwoman",
                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                 0, 0,
                 SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!window) { printf("Window : %s\n", SDL_GetError()); SDL_Quit(); return 1; }

    renderer = SDL_CreateRenderer(window, -1,
                 SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { printf("Renderer : %s\n", SDL_GetError());
        SDL_DestroyWindow(window); SDL_Quit(); return 1; }

    SDL_GetRendererOutputSize(renderer, &screenW, &screenH);
    printf("Screen: %dx%d\n", screenW, screenH);

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == 0) {
        music = Mix_LoadMUS("menu_music.mp3");
        if (music) Mix_PlayMusic(music, -1);
    }

    font = TTF_OpenFont("assets/font.ttf", 20);
    if (!font) font = TTF_OpenFont("font.ttf", 20);
    if (!font) font = TTF_OpenFont("arial.ttf", 20);
    if (!font) printf("Avertissement : police non chargee\n");

    if (!runMenu(renderer, font, &menuRes))
        goto cleanup;

    initBackgroundAndPlatforms(renderer, &bg, platforms, &taille,
                               1, screenW, screenH);
    worldW = bg.partW * bg.imgCount;
    if (worldW < screenW) worldW = screenW;

    worldH = bg.partH;
    if (worldH < screenH) worldH = screenH;

    /* minimap — top-right corner */
    snprintf(minimapPath, sizeof(minimapPath), "back/level%d_mini.png", currentLevel);
    {
        SDL_Rect mpos = { screenW - MINIMAP_WIDTH - 10, 10,
                          MINIMAP_WIDTH, MINIMAP_HEIGHT };
        minimap = createMinimap(renderer, minimapPath, mpos);

        /* minimaps split-screen : coin inferieur de chaque demi-ecran */
        int halfW = screenW / 2;
        SDL_Rect mposL = { halfW  - MINIMAP_WIDTH  - 10, screenH - MINIMAP_HEIGHT - 10,
                           MINIMAP_WIDTH, MINIMAP_HEIGHT };
        SDL_Rect mposR = { screenW - MINIMAP_WIDTH - 10, screenH - MINIMAP_HEIGHT - 10,
                           MINIMAP_WIDTH, MINIMAP_HEIGHT };
        minimapLeft  = createMinimap(renderer, minimapPath, mposL);
        minimapRight = createMinimap(renderer, minimapPath, mposR);
    }
    
    printf("World width: %d, Platforms: %d\n", worldW, taille);

    bg.camera_pos.x = 0;
    bg.camera_pos.y = 0;
    bg.camera_pos.w = screenW;
    bg.camera_pos.h = screenH;

    groundEcran = (float)(screenH - JOUEUR_H - 20);

    /* ── Placer l'objet énigme dans le monde ── */
    enigmeObjRect.x = 600;
    enigmeObjRect.y = (int)groundEcran - enigmeObjRect.h + JOUEUR_H;
    enigmeObjTex = IMG_LoadTexture(renderer, "assets/enigme.png");
    if (!enigmeObjTex) enigmeObjTex = IMG_LoadTexture(renderer, "enigme.png");
    if (!enigmeObjTex) printf("[enigme] enigme.png non trouve — rectangle jaune utilise\n");

    /* ── Synchroniser SCR_W/SCR_H et initialiser le quiz ── */
    SCR_W = screenW;
    SCR_H = screenH;
    initEnigme(&enigme, renderer);

    initPlayer(&p1, 150.0f, groundEcran, 0);
    initPlayer(&p2, (float)(screenW - 250), groundEcran, 1);
    p2.w = JOUEUR_W; p2.h = JOUEUR_H;
    p1.canDoubleJump = 1;
    p2.canDoubleJump = 1;

    printf("Ground: %.0f, P1 y: %.0f, P2 y: %.0f\n", groundEcran, p1.y, p2.y);

    {
        int c1 = menuRes.costumeP1;
        int c2 = menuRes.costumeP2;
        SDL_Texture *batR = loadSheet(renderer, CHARACTERS[CHAR_BATMAN  ].sheetRight[c1]);
        SDL_Texture *batL = loadSheet(renderer, CHARACTERS[CHAR_BATMAN  ].sheetLeft [c1]);
        SDL_Texture *catR = loadSheet(renderer, CHARACTERS[CHAR_CATWOMAN].sheetRight[c2]);
        SDL_Texture *catL = loadSheet(renderer, CHARACTERS[CHAR_CATWOMAN].sheetLeft [c2]);
        initSpriteData(&p1.sprite, batR, batL, BAT_FRAME_W, BAT_FRAME_H, 0);
        initSpriteData(&p2.sprite, catR, catL, CAT_FRAME_W, CAT_FRAME_H, 1);
    }

    NPC_init(&gameNPC, renderer, NULL);
    NPC_setGroundY(&gameNPC, (int)groundEcran);   /* doit être après NPC_init (memset) */
    NPC_loadLevel(&gameNPC, renderer, currentLevel);
    gameNPC.pH = &p1.hp;
    gameNPC.pScore = &p1.score;

    SDL_Texture *batarang = IMG_LoadTexture(renderer, "assets/batarang.png");
    if (!batarang) batarang = IMG_LoadTexture(renderer, "batarang.png");
    if (!batarang) printf("Avertissement : batarang.png non charge\n");

    {
        InputConfig cfg1 = menuRes.inputP1;
        InputConfig cfg2 = menuRes.inputP2;
        int shootCdP1 = 0, shootCdP2 = 0;

        lastTime = SDL_GetTicks();
        prevTick = SDL_GetTicks();

        while (running) {
            Uint32    now;
            float     dt;
            int       bgX, bgY;
            SDL_Color dummyColor = {255, 255, 255, 255};

            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) { running = 0; break; }

                /* ── Si l'énigme est active, elle capture tous les events ── */
                if (enigmeActive) {
                    if (event.type == SDL_KEYDOWN &&
                        event.key.keysym.sym == SDLK_ESCAPE) {
                        enigmeActive = 0; /* ESC = fermer sans pénalité */
                    } else {
                        handleEnigmeEvent(&enigme, &event);
                    }
                    continue; /* ne pas propager au jeu */
                }

                if (event.type == SDL_KEYDOWN) {
                    SDL_Keycode sym = event.key.keysym.sym;
                    if (sym == SDLK_ESCAPE) { running = 0; break; }
                    if (sym == SDLK_p) {
                        if (!splitScreen) {
                            /* Normal -> Split : p.x etait relative a bg.camera_pos.x
                               On la convertit en position absolue dans le monde */
                            p1.x += (float)bg.camera_pos.x;
                            p2.x += (float)bg.camera_pos.x;
                        } else {
                            /* Split -> Normal : p.x est absolue, on la ramene
                               en coordonnee ecran relative a la camera globale */
                            p1.x -= (float)bg.camera_pos.x;
                            p2.x -= (float)bg.camera_pos.x;
                            /* Bornes ecran */
                            if (p1.x < 0) p1.x = 0;
                            if (p1.x > screenW - p1.w) p1.x = (float)(screenW - p1.w);
                            if (p2.x < 0) p2.x = 0;
                            if (p2.x > screenW - p2.w) p2.x = (float)(screenW - p2.w);
                        }
                        splitScreen = !splitScreen;
                    }

                    /* F1/F2/F3 = switch level */
                    int targetLevel = 0;
                    if (sym == SDLK_F1) targetLevel = 1;
                    else if (sym == SDLK_F2) targetLevel = 2;
                    else if (sym == SDLK_F3) targetLevel = 3;

                    if (targetLevel > 0 && targetLevel != currentLevel) {
                        currentLevel = targetLevel;
                        int ww;
                        switchLevel(currentLevel, renderer,
                                    &bg, platforms, &taille,
                                    &gameNPC,
                                    &p1, &p2,
                                    screenW, screenH, groundEcran,
                                    &timeLeft, &lastTime,
                                    &invP1, &invP2,
                                    &invObstacleP1, &invObstacleP2);
                        ww = bg.partW * bg.imgCount;
                        if (ww < screenW) ww = screenW;
                        worldW = ww;
                        worldH = bg.partH;
                        if (worldH < screenH) worldH = screenH;
                        freeMinimap(minimap);
                        freeMinimap(minimapLeft);
                        freeMinimap(minimapRight);
                        snprintf(minimapPath, sizeof(minimapPath),
                                 "back/level%d_mini.png", currentLevel);
                        {
                            int halfW2 = screenW / 2;
                            SDL_Rect mpos  = { screenW - MINIMAP_WIDTH - 10, 10,
                                               MINIMAP_WIDTH, MINIMAP_HEIGHT };
                            SDL_Rect mposL = { halfW2  - MINIMAP_WIDTH  - 10, screenH - MINIMAP_HEIGHT - 10,
                                               MINIMAP_WIDTH, MINIMAP_HEIGHT };
                            SDL_Rect mposR = { screenW - MINIMAP_WIDTH - 10, screenH - MINIMAP_HEIGHT - 10,
                                               MINIMAP_WIDTH, MINIMAP_HEIGHT };
                            minimap      = createMinimap(renderer, minimapPath, mpos);
                            minimapLeft  = createMinimap(renderer, minimapPath, mposL);
                            minimapRight = createMinimap(renderer, minimapPath, mposR);
                        }
                        shootCdP1 = 0; shootCdP2 = 0;
                        prevTick = SDL_GetTicks();
                    }
                }
                gererGuideEtClic(event, &bg.guide, &bg.commentJouer,
                                 &bg.afficherCommentJouer);
            }

            now = SDL_GetTicks();
            dt  = (now - prevTick) / 1000.0f;
            if (dt > 0.05f) dt = 0.05f;
            if (dt < 0.01f) dt = 0.016f;
            prevTick = now;

            if (shootCdP1 > 0) shootCdP1--;
            if (shootCdP2 > 0) shootCdP2--;

            /* ── Geler le jeu pendant l'énigme ── */
            if (enigmeActive) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);
                /* Re-dessiner le fond figé + l'interface énigme */
                afficherBackgroundEtElements(renderer, &bg, platforms, taille,
                                             font, dummyColor, timeLeft,
                                             (p1.vies > p2.vies) ? p1.vies : p2.vies,
                                             bgX + shake.offsetX, bgY + shake.offsetY,
                                             screenW, screenH, MODE_MONO);
                NPC_draw(&gameNPC, bg.camera_pos.x, bg.camera_pos.y);
                blitPlayer(renderer, &p1);
                blitPlayer(renderer, &p2);
                updateEnigme(&enigme);
                renderEnigme(&enigme, renderer, font, font, font);
                if (enigme.questionIndex >= NB_QUESTIONS) {
                    enigmeActive    = 0;
                    enigmeObjActive = 0;
                    p1.score += enigme.score * 50;
                    printf("[enigme] Quiz terminé ! score=%d bonus=%d pts\n",
                           enigme.score, enigme.score * 50);
                }
                SDL_RenderPresent(renderer);
                SDL_Delay(16);
                continue;
            }

            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            if (p1.isAlive) handlePlayerInput(&p1, keys, &cfg1, groundEcran);
            if (p2.isAlive) handlePlayerInput(&p2, keys, &cfg2, groundEcran);

            if (keys[cfg1.punch] && !p1.isAttacking && !p1.isKicking && !p1.isShooting && p1.actionTimer == 0) {
                p1.isAttacking = 1;
                p1.actionTimer = 20;
                p1.vitesse = 0;
                p1.animState = ANIM_ATTACK;
                p1.animFrame = 0;
                SDL_Rect playerRect = { (int)p1.x, (int)p1.y, p1.w, p1.h };
                playerAttackEnemy(&gameNPC, &playerRect, bg.camera_pos.x, bg.camera_pos.y, &p1.score);
            }

            if (keys[cfg2.punch] && !p2.isAttacking && !p2.isKicking && !p2.isShooting && p2.actionTimer == 0) {
                p2.isAttacking = 1;
                p2.actionTimer = 20;
                p2.vitesse = 0;
                p2.animState = ANIM_ATTACK;
                p2.animFrame = 0;
                SDL_Rect playerRect = { (int)p2.x, (int)p2.y, p2.w, p2.h };
                playerAttackEnemy(&gameNPC, &playerRect, bg.camera_pos.x, bg.camera_pos.y, &p2.score);
            }

            if (keys[cfg1.kick] && !p1.isAttacking && !p1.isKicking && !p1.isShooting && p1.actionTimer == 0) {
                p1.isKicking   = 1;
                p1.actionTimer = 20;
                p1.vitesse     = 0;
                p1.animState   = ANIM_KICK;
                p1.animFrame   = 0;
            }

            if (keys[cfg2.kick] && !p2.isAttacking && !p2.isKicking && !p2.isShooting && p2.actionTimer == 0) {
                p2.isKicking   = 1;
                p2.actionTimer = 20;
                p2.vitesse     = 0;
                p2.animState   = ANIM_KICK;
                p2.animFrame   = 0;
            }

            if (keys[cfg1.shoot] && !p1.isAttacking && !p1.isKicking && !p1.isShooting && p1.actionTimer == 0 && shootCdP1 == 0) {
                p1.isShooting = 1;
                p1.actionTimer = 20;
                p1.vitesse = 0;
                p1.animState = ANIM_SHOOT;
                p1.animFrame = 0;
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (!p1.bullets[i].active) {
                        p1.bullets[i].active = 1;
                        p1.bullets[i].owner = 1;
                        p1.bullets[i].y = (float)(p1.y + p1.h * 0.30f);
                        if (p1.direction) {
                            p1.bullets[i].x = (float)(p1.x + p1.w + 5);
                            p1.bullets[i].vx = 600.0f;
                        } else {
                            p1.bullets[i].x = (float)(p1.x - 5);
                            p1.bullets[i].vx = -600.0f;
                        }
                        shootCdP1 = 30;
                        break;
                    }
                }
            }

            if (keys[cfg2.shoot] && !p2.isAttacking && !p2.isKicking && !p2.isShooting && p2.actionTimer == 0 && shootCdP2 == 0) {
                p2.isShooting = 1;
                p2.actionTimer = 20;
                p2.vitesse = 0;
                p2.animState = ANIM_SHOOT;
                p2.animFrame = 0;
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (!p2.bullets[i].active) {
                        p2.bullets[i].active = 1;
                        p2.bullets[i].owner = 2;
                        p2.bullets[i].y = (float)(p2.y + p2.h * 0.30f);
                        if (p2.direction) {
                            p2.bullets[i].x = (float)(p2.x + p2.w + 5);
                            p2.bullets[i].vx = 600.0f;
                        } else {
                            p2.bullets[i].x = (float)(p2.x - 5);
                            p2.bullets[i].vx = -600.0f;
                        }
                        shootCdP2 = 30;
                        break;
                    }
                }
            }

            updatePhysics(&p1, dt, groundEcran);
            updatePhysics(&p2, dt, groundEcran);

            updateAnimState(&p1, dt);
            updateAnimState(&p2, dt);

            if (splitScreen)
                doScrollingSplit(&p1, &p2, worldW, worldH, dt);
            else
                doScrolling(&bg, &p1, &p2, screenW, screenH, worldW, dt);
            bgX = bg.camera_pos.x;
            bgY = (int)bg.camera_pos.y;

            {
                int hp1before = p1.hp;
                int hp2before = p2.hp;
                handleObstacleCollision(&p1, platforms, taille, &invObstacleP1, groundEcran, &bg);
                handleObstacleCollision(&p2, platforms, taille, &invObstacleP2, groundEcran, &bg);
                if (p1.hp < hp1before) triggerShake(&shake);
                if (p2.hp < hp2before) triggerShake(&shake);
            }

            {
                int hp1b = p1.hp, hp2b = p2.hp;
                resolvePlayerCombat(&p1, &p2, &invP2);
                resolvePlayerCombat(&p2, &p1, &invP1);
                if (p2.hp < hp2b || p1.hp < hp1b) triggerShake(&shake);
            }

            gererTemps(&timeLeft, &lastTime);
            if (timeLeft < 0) timeLeft = 0;

            if (timeLeft == 0 || !p1.isAlive || !p2.isAlive) {
                if (!p1.isAlive || !p2.isAlive) SDL_Delay(1500);
                running = 0;
            }

            updatePlatforms(platforms, taille);

            {
                /* IMPORTANT : on passe le rect en coordonnées MONDE (world-space)
                   = position écran + caméra, pour que la comparaison avec
                   les ennemis (aussi en monde) soit cohérente. */
                SDL_Rect playerRect = { (int)p1.x + bg.camera_pos.x,
                                        (int)p1.y + (int)bg.camera_pos.y,
                                        p1.w, p1.h };
                int hpBefore = p1.hp;
                NPC_update(&gameNPC, &playerRect, &p1.hp, &p1.score, bg.camera_pos.x, bg.camera_pos.y);
                if (p1.hp < hpBefore) triggerShake(&shake);
            }

            /* update shake animation */
            updateShake(&shake);

            /* ── Détection collision joueur ↔ objet énigme ── */
            if (enigmeObjActive && !enigmeActive) {
                SDL_Rect p1screenRect = { (int)p1.x, (int)p1.y, p1.w, p1.h };
                /* Convertir position monde → écran */
                SDL_Rect enigmeScreen = {
                    enigmeObjRect.x - bg.camera_pos.x,
                    enigmeObjRect.y - (int)bg.camera_pos.y,
                    enigmeObjRect.w, enigmeObjRect.h
                };
                if (p1.isAlive && SDL_HasIntersection(&p1screenRect, &enigmeScreen)) {
                    enigmeActive    = 1;
                    enigmeStartTick = SDL_GetTicks();
                    /* Réinitialiser le quiz pour cette partie */
                    freeEnigme(&enigme);
                    SCR_W = screenW; SCR_H = screenH;
                    initEnigme(&enigme, renderer);
                    printf("[enigme] Collision ! Interface enigme declenchee.\n");
                }
            }

            /* build enemy array for minimap and update dot positions */
            {
                int ec = 0;
                for (int _i = 0; _i < gameNPC.enemyCnt && ec < MINIMAP_MAX_ENEMIES; _i++) {
                    minimapEnemies[ec].x      = gameNPC.enemies[_i].x;
                    minimapEnemies[ec].y      = gameNPC.enemies[_i].y;
                    minimapEnemies[ec].active = gameNPC.enemies[_i].active &&
                                               !gameNPC.enemies[_i].dying;
                    ec++;
                }
                updateMinimap(minimap,
                    (int)p1.x + bg.camera_pos.x, (int)p1.y + (int)bg.camera_pos.y,
                    (int)p2.x + bg.camera_pos.x, (int)p2.y + (int)bg.camera_pos.y,
                    worldW, worldH);
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            if (!splitScreen) {
                /* ── MODE ECRAN UNIQUE ── */
                afficherBackgroundEtElements(renderer, &bg, platforms, taille,
                                             font, dummyColor, timeLeft,
                                             (p1.vies > p2.vies) ? p1.vies : p2.vies,
                                             bgX + shake.offsetX,
                                             bgY + shake.offsetY,
                                             screenW, screenH, MODE_MONO);

                NPC_draw(&gameNPC, bg.camera_pos.x, bg.camera_pos.y);

                /* ── Dessiner l'objet énigme dans le monde ── */
                if (enigmeObjActive) {
                    SDL_Rect enigmeScreen = {
                        enigmeObjRect.x - bg.camera_pos.x,
                        enigmeObjRect.y - (int)bg.camera_pos.y,
                        enigmeObjRect.w, enigmeObjRect.h
                    };
                    if (enigmeScreen.x + enigmeScreen.w > 0 && enigmeScreen.x < screenW) {
                        if (enigmeObjTex) {
                            SDL_RenderCopy(renderer, enigmeObjTex, NULL, &enigmeScreen);
                        } else {
                            /* Fallback : rectangle jaune scintillant */
                            Uint32 tnow = SDL_GetTicks();
                            Uint8  alpha = (Uint8)(180 + 75 * sinf((float)tnow / 400.0f));
                            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                            SDL_SetRenderDrawColor(renderer, 255, 220, 0, alpha);
                            SDL_RenderFillRect(renderer, &enigmeScreen);
                            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                            SDL_RenderDrawRect(renderer, &enigmeScreen);
                            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                        }
                        /* Texte indicatif "?" au-dessus */
                        if (font) {
                            SDL_Color yellow = {255, 220, 0, 255};
                            SDL_Surface *qs = TTF_RenderUTF8_Blended(font, "?", yellow);
                            if (qs) {
                                SDL_Texture *qt = SDL_CreateTextureFromSurface(renderer, qs);
                                SDL_Rect qd = { enigmeScreen.x + enigmeScreen.w/2 - qs->w/2,
                                                enigmeScreen.y - qs->h - 4,
                                                qs->w, qs->h };
                                SDL_RenderCopy(renderer, qt, NULL, &qd);
                                SDL_FreeSurface(qs); SDL_DestroyTexture(qt);
                            }
                        }
                    }
                }
                blitPlayer(renderer, &p2);

                {
                    int i;
                    int BULLET_W = 50, BULLET_H = 25;

                    for (i = 0; i < MAX_BULLETS; i++) {
                        SDL_Rect br, pr;
                        if (!p1.bullets[i].active) continue;
                        br.x = (int)p1.bullets[i].x; br.y = (int)p1.bullets[i].y;
                        br.w = BULLET_W; br.h = BULLET_H;
                        pr.x = (int)p2.x; pr.y = (int)p2.y;
                        pr.w = p2.w; pr.h = p2.h;
                        if (rectsOverlap(br, pr) && p2.isAlive && invP2 == 0) {
                            p2.hp -= 5;
                            if (p2.hp < 0) p2.hp = 0;
                            p1.bullets[i].active = 0;
                            invP2 = INVINCIBILITY_FRAMES;
                            if (p2.hp == 0) {
                                p2.hp = 100; p2.vies--; p1.score += 100;
                                if (p2.vies <= 0) { p2.isAlive = 0; p2.animState = ANIM_DEAD; p2.animFrame = 0; }
                            }
                        }
                    }

                    for (i = 0; i < MAX_BULLETS; i++) {
                        SDL_Rect br, pr;
                        if (!p2.bullets[i].active) continue;
                        br.x = (int)p2.bullets[i].x; br.y = (int)p2.bullets[i].y;
                        br.w = BULLET_W; br.h = BULLET_H;
                        pr.x = (int)p1.x; pr.y = (int)p1.y;
                        pr.w = p1.w; pr.h = p1.h;
                        if (rectsOverlap(br, pr) && p1.isAlive && invP1 == 0) {
                            p1.hp -= 5;
                            if (p1.hp < 0) p1.hp = 0;
                            p2.bullets[i].active = 0;
                            invP1 = INVINCIBILITY_FRAMES;
                            if (p1.hp == 0) {
                                p1.hp = 100; p1.vies--; p2.score += 100;
                                if (p1.vies <= 0) { p1.isAlive = 0; p1.animState = ANIM_DEAD; p1.animFrame = 0; }
                            }
                        }
                    }

                    for (i = 0; i < MAX_BULLETS; i++) {
                        SDL_Rect br;
                        if (!p1.bullets[i].active) continue;
                        br.w = BULLET_W; br.h = BULLET_H;
                        br.x = (int)p1.bullets[i].x - br.w / 2;
                        br.y = (int)p1.bullets[i].y - br.h / 2;
                        if (batarang) {
                            SDL_SetTextureColorMod(batarang, 0, 220, 255);
                            SDL_SetTextureAlphaMod(batarang, 255);
                            SDL_RenderCopy(renderer, batarang, NULL, &br);
                        } else {
                            SDL_SetRenderDrawColor(renderer, 0, 220, 255, 255);
                            SDL_RenderFillRect(renderer, &br);
                        }
                    }

                    for (i = 0; i < MAX_BULLETS; i++) {
                        SDL_Rect br;
                        if (!p2.bullets[i].active) continue;
                        br.w = BULLET_W; br.h = BULLET_H;
                        br.x = (int)p2.bullets[i].x - br.w / 2;
                        br.y = (int)p2.bullets[i].y - br.h / 2;
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                        {
                            int s, gap = 8;
                            for (s = -1; s <= 1; s++) {
                                int ox = s * gap;
                                SDL_SetRenderDrawColor(renderer, 255, 60, 220, 160);
                                SDL_RenderDrawLine(renderer,
                                    br.x + br.w/2 + ox,          br.y,
                                    br.x + br.w/2 + ox + br.h/2, br.y + br.h);
                            }
                        }
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                    }
                }

                renderHUD(renderer, font, &p1, &p2, screenW);

                /* Level indicator */
                if (font) {
                    char lvlBuf[32];
                    SDL_Surface *lvlSurf;
                    SDL_Texture *lvlTex;
                    SDL_Rect     lvlDst;
                    SDL_Color    gold = {255, 215, 0, 255};
                    snprintf(lvlBuf, sizeof(lvlBuf), "LEVEL %d  [F1/F2/F3]", currentLevel);
                    lvlSurf = TTF_RenderUTF8_Blended(font, lvlBuf, gold);
                    if (lvlSurf) {
                        lvlTex = SDL_CreateTextureFromSurface(renderer, lvlSurf);
                        lvlDst.x = screenW / 2 - lvlSurf->w / 2;
                        lvlDst.y = 8;
                        lvlDst.w = lvlSurf->w;
                        lvlDst.h = lvlSurf->h;
                        SDL_RenderCopy(renderer, lvlTex, NULL, &lvlDst);
                        SDL_FreeSurface(lvlSurf);
                        SDL_DestroyTexture(lvlTex);
                    }
                }

                /* minimap */
                renderMinimap(renderer, minimap, minimapEnemies, gameNPC.enemyCnt,
                              worldW, worldH);

            } else {
                /* ── MODE SPLIT-SCREEN (touche P) ── *
                 * Moitie gauche  = vue centree sur P1
                 * Moitie droite  = vue centree sur P2
                 * Ligne de separation au milieu                      */

                int halfW = screenW / 2;

                /* En mode split, doScrollingSplit met a jour p.x comme
                   position absolue dans le monde (sans camera globale).
                   La camera de chaque demi-ecran est centree sur le joueur. */
                int scaled1X = (int)p1.x;
                int scaled2X = (int)p2.x;

                int cam1X = scaled1X + p1.w / 2 - halfW / 2;
                int cam1Y = bgY;
                if (cam1X < 0) cam1X = 0;
                if (cam1X + halfW > worldW) cam1X = worldW - halfW;

                int cam2X = scaled2X + p2.w / 2 - halfW / 2;
                int cam2Y = bgY;
                if (cam2X < 0) cam2X = 0;
                if (cam2X + halfW > worldW) cam2X = worldW - halfW;

                /* Position des joueurs dans chaque demi-vue (coords écran) :
                   screenPos = scaledWorldPos - camX                        */
                int p1_in_left  = scaled1X - cam1X;
                int p2_in_left  = scaled2X - cam1X;
                int p1_in_right = scaled1X - cam2X;
                int p2_in_right = scaled2X - cam2X;

                /* ---- Moitie gauche (P1) ---- */
                {
                /* ClipRect absolu pour empecher tout debordement */
                SDL_Rect clipLeft = { 0, 0, halfW, screenH };
                SDL_RenderSetClipRect(renderer, &clipLeft);
                SDL_RenderSetViewport(renderer, NULL); /* viewport = ecran complet */

                /* Fond noir uniquement sur la moitie gauche */
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderFillRect(renderer, &clipLeft);

                /* Rendu background avec cam1X : les tiles sont placees a
                   (tileX - cam1X) ce qui tombe dans [0..halfW] */
                int savedCamX = bg.camera_pos.x;
                int savedCamW = bg.camera_pos.w;
                bg.camera_pos.x = cam1X;
                bg.camera_pos.w = halfW;

                afficherBackgroundEtElements(renderer, &bg, platforms, taille,
                                             font, dummyColor, timeLeft, p1.vies,
                                             cam1X + shake.offsetX, cam1Y + shake.offsetY,
                                             halfW, screenH, MODE_MONO);
                NPC_draw(&gameNPC, cam1X, cam1Y);

                bg.camera_pos.x = savedCamX;
                bg.camera_pos.w = savedCamW;

                {
                    float origX1 = p1.x, origX2 = p2.x;
                    p1.x = (float)p1_in_left;
                    p2.x = (float)p2_in_left;
                    blitPlayer(renderer, &p1);
                    blitPlayer(renderer, &p2);
                    p1.x = origX1;
                    p2.x = origX2;
                }

                /* HUD gauche : seulement P1 */
                if (font) {
                    char buf[80];
                    SDL_Surface *surf;
                    SDL_Texture *tex;
                    SDL_Rect d;
                    SDL_Color cyan = { 0, 220, 220, 255 };
                    SDL_Rect bg1r, fg1r;
                    int hp1w;
                    snprintf(buf, sizeof(buf), "BATMAN  Vies:%d  Score:%d", p1.vies, p1.score);
                    surf = TTF_RenderUTF8_Blended(font, buf, cyan);
                    if (surf) {
                        tex = SDL_CreateTextureFromSurface(renderer, surf);
                        d.x = 10; d.y = 8; d.w = surf->w; d.h = surf->h;
                        SDL_RenderCopy(renderer, tex, NULL, &d);
                        SDL_FreeSurface(surf); SDL_DestroyTexture(tex);
                    }
                    bg1r.x = 10; bg1r.y = 30; bg1r.w = 200; bg1r.h = 12;
                    hp1w = p1.hp * 2; if (hp1w < 0) hp1w = 0;
                    fg1r.x = 10; fg1r.y = 30; fg1r.w = hp1w; fg1r.h = 12;
                    SDL_SetRenderDrawColor(renderer, 40, 0, 0, 255);   SDL_RenderFillRect(renderer, &bg1r);
                    SDL_SetRenderDrawColor(renderer, 220, 60, 60, 255); SDL_RenderFillRect(renderer, &fg1r);
                    SDL_SetRenderDrawColor(renderer, 180,180,180, 255); SDL_RenderDrawRect(renderer, &bg1r);
                }

                /* Minimap P1 — coin bas-droite de la moitie gauche */
                if (minimapLeft) {
                    updateMinimap(minimapLeft,
                        scaled1X, (int)p1.y + (int)bg.camera_pos.y,
                        scaled2X, (int)p2.y + (int)bg.camera_pos.y,
                        worldW, worldH);
                    renderMinimap(renderer, minimapLeft, minimapEnemies, gameNPC.enemyCnt,
                                  worldW, worldH);
                }
                } /* fin moitie gauche */

                /* ---- Moitie droite (P2) ---- */
                {
                SDL_Rect clipRight = { halfW, 0, halfW, screenH };
                SDL_RenderSetClipRect(renderer, &clipRight);
                SDL_RenderSetViewport(renderer, NULL);

                /* Fond noir uniquement sur la moitie droite */
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderFillRect(renderer, &clipRight);

                /* Pour la moitie droite, les coordonnees absolues du rendu
                   doivent etre decalees de +halfW car viewport = ecran complet.
                   On utilise un offset : on soustrait cam2X puis on ajoute halfW */
                int savedCamX2 = bg.camera_pos.x;
                int savedCamW2 = bg.camera_pos.w;
                /* cam2X_right = cam2X - halfW => les tiles s'affichent
                   a (tileX - cam2X + halfW), soit dans [halfW..screenW] */
                bg.camera_pos.x = cam2X - halfW;
                bg.camera_pos.w = halfW;

                afficherBackgroundEtElements(renderer, &bg, platforms, taille,
                                             font, dummyColor, timeLeft, p2.vies,
                                             (cam2X - halfW) + shake.offsetX, cam2Y + shake.offsetY,
                                             halfW, screenH, MODE_MONO);
                NPC_draw(&gameNPC, cam2X - halfW, cam2Y);

                bg.camera_pos.x = savedCamX2;
                bg.camera_pos.w = savedCamW2;

                {
                    /* viewport = ecran complet => coordonnees absolues => +halfW */
                    float origX1 = p1.x, origX2 = p2.x;
                    p1.x = (float)(p1_in_right + halfW);
                    p2.x = (float)(p2_in_right + halfW);
                    blitPlayer(renderer, &p1);
                    blitPlayer(renderer, &p2);
                    p1.x = origX1;
                    p2.x = origX2;
                }

                /* HUD droite : seulement P2 — coordonnees absolues (+halfW) */
                if (font) {
                    char buf[80];
                    SDL_Surface *surf;
                    SDL_Texture *tex;
                    SDL_Rect d;
                    SDL_Color magenta = { 220, 60, 220, 255 };
                    SDL_Rect bg2r, fg2r;
                    int hp2w;
                    snprintf(buf, sizeof(buf), "CATWOMAN  Vies:%d  Score:%d", p2.vies, p2.score);
                    surf = TTF_RenderUTF8_Blended(font, buf, magenta);
                    if (surf) {
                        tex = SDL_CreateTextureFromSurface(renderer, surf);
                        d.x = screenW - surf->w - 10; d.y = 8; d.w = surf->w; d.h = surf->h;
                        SDL_RenderCopy(renderer, tex, NULL, &d);
                        SDL_FreeSurface(surf); SDL_DestroyTexture(tex);
                    }
                    bg2r.x = screenW - 210; bg2r.y = 30; bg2r.w = 200; bg2r.h = 12;
                    hp2w = p2.hp * 2; if (hp2w < 0) hp2w = 0;
                    fg2r.x = screenW - 210; fg2r.y = 30; fg2r.w = hp2w; fg2r.h = 12;
                    SDL_SetRenderDrawColor(renderer, 40, 0, 40, 255);   SDL_RenderFillRect(renderer, &bg2r);
                    SDL_SetRenderDrawColor(renderer, 200, 60,220, 255); SDL_RenderFillRect(renderer, &fg2r);
                    SDL_SetRenderDrawColor(renderer, 180,180,180, 255); SDL_RenderDrawRect(renderer, &bg2r);
                }

                /* Minimap P2 — coin bas-droite de la moitie droite (coords absolues) */
                if (minimapRight) {
                    updateMinimap(minimapRight,
                        scaled1X, (int)p1.y + (int)bg.camera_pos.y,
                        scaled2X, (int)p2.y + (int)bg.camera_pos.y,
                        worldW, worldH);
                    renderMinimap(renderer, minimapRight, minimapEnemies, gameNPC.enemyCnt,
                                  worldW, worldH);
                }
                } /* fin moitie droite */

                /* Retablir viewport complet */
                SDL_RenderSetViewport(renderer, NULL);
                SDL_RenderSetClipRect(renderer, NULL);

                /* Ligne de separation blanche au milieu */
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawLine(renderer, halfW, 0, halfW, screenH);
                SDL_RenderDrawLine(renderer, halfW - 1, 0, halfW - 1, screenH);

                /* Indicateur de mode en haut au centre */
                if (font) {
                    SDL_Surface *ms = TTF_RenderUTF8_Blended(font, "SPLIT [P]", (SDL_Color){255,215,0,255});
                    if (ms) {
                        SDL_Texture *mt = SDL_CreateTextureFromSurface(renderer, ms);
                        SDL_Rect md = { screenW/2 - ms->w/2, 2, ms->w, ms->h };
                        SDL_RenderCopy(renderer, mt, NULL, &md);
                        SDL_FreeSurface(ms); SDL_DestroyTexture(mt);
                    }
                }
            } /* fin split-screen */

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
        }
    }

    if (font) {
        int ws = (p1.score >= p2.score) ? p1.score : p2.score;
        saisirNomEtAfficherScore(renderer, font, ws, screenW, screenH);
    }

cleanup:
    NPC_clean(&gameNPC);
    freeMinimap(minimap);
    freeMinimap(minimapLeft);
    freeMinimap(minimapRight);
    freeEnigme(&enigme);
    if (enigmeObjTex) { SDL_DestroyTexture(enigmeObjTex); enigmeObjTex = NULL; }

    if (batarang) SDL_DestroyTexture(batarang);
    if (p1.sprite.sheetRight) SDL_DestroyTexture(p1.sprite.sheetRight);
    if (p1.sprite.sheetLeft)  SDL_DestroyTexture(p1.sprite.sheetLeft);
    if (p2.sprite.sheetRight) SDL_DestroyTexture(p2.sprite.sheetRight);
    if (p2.sprite.sheetLeft)  SDL_DestroyTexture(p2.sprite.sheetLeft);

    {
        int i, f;
        { int _bi; for (_bi = 0; _bi < 8; _bi++) if (bg.img[_bi]) SDL_DestroyTexture(bg.img[_bi]); }
        if (bg.guide.image)        SDL_DestroyTexture(bg.guide.image);
        if (bg.commentJouer.image) SDL_DestroyTexture(bg.commentJouer.image);
        for (i = 0; i < taille; i++) {
            Platform *pp = &platforms[i];
            if (pp->isAnimated) {
                for (f = 0; f < MAX_FRAMES; f++)
                    if (pp->frames[f]) SDL_DestroyTexture(pp->frames[f]);
            } else {
                if (pp->image) SDL_DestroyTexture(pp->image);
            }
        }
    }

    if (font)  TTF_CloseFont(font);
    if (music) { Mix_HaltMusic(); Mix_FreeMusic(music); }
    Mix_CloseAudio();
    TTF_Quit(); IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
