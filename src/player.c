#include "../include/player.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════════════════
   PHYSICS / GAME CONSTANTS  (unchanged)
═══════════════════════════════════════════════════════════════════════ */
#define SPEED        220.0
#define SPEED_RUN    380.0
#define ACCEL          0.0
#define GRAVITY      900.0f
#define FLY_FORCE   -200.0f
#define JUMP_FORCE  -480.0f
#define GROUND_Y     267.0
#define SCREEN_W     1000
#define ACTION_DUR    20
#define BULLET_SPEED 500.0f
#define SAUT_C      100.0
#define SAUT_A      -0.04
#define SAUT_X_MIN  -50.0
#define SAUT_X_MAX   50.0
#define SAUT_VX       1.2

#define BAT_FRAME_W  172
#define BAT_FRAME_H  174
#define CAT_FRAME_W  168
#define CAT_FRAME_H  172

/* ═══════════════════════════════════════════════════════════════════════
   DEFAULT ANIMATION TABLES
   These are used when the caller passes NULL for frameCounts/fps/loop.
═══════════════════════════════════════════════════════════════════════ */

/* Default frame counts per animation row (index = BatAnimState/CatAnimState) */
static const int DEFAULT_FRAME_COUNTS[ANIM_COUNT] = {
    /* IDLE */ 1,
    /* WALK */ 4,
    /* RUN  */ 4,
    /* JUMP */ 4,
    /* ATK  */ 6,
    /* KICK */ 6,
    /* SHOT */ 4,
    /* FLY/DJUMP */ 4,
    /* DEAD */ 5
};

/* Default playback speeds (fps) per animation row */
static const float DEFAULT_FPS[ANIM_COUNT] = {
    /* IDLE */ 4.0f,
    /* WALK */ 10.0f,
    /* RUN  */ 14.0f,
    /* JUMP */ 8.0f,
    /* ATK  */ 16.0f,
    /* KICK */ 16.0f,
    /* SHOT */ 14.0f,
    /* FLY/DJUMP */ 10.0f,
    /* DEAD */ 8.0f
};

/* Default loop flags (0 = one-shot, 1 = loop) */
static const int DEFAULT_LOOP[ANIM_COUNT] = {
    /* IDLE */  1,
    /* WALK */  1,
    /* RUN  */  1,
    /* JUMP */  1,
    /* ATK  */  0,   /* one-shot: hold last frame until actionTimer expires */
    /* KICK */  0,
    /* SHOT */  0,
    /* FLY/DJUMP */ 1,
    /* DEAD */  0
};

/* ═══════════════════════════════════════════════════════════════════════
   initAnimReel
═══════════════════════════════════════════════════════════════════════ */
void initAnimReel(AnimReel *reel,
                  SDL_Texture *right, SDL_Texture *left,
                  int frameW, int frameH,
                  int frameCount, float fps, int loop)
{
    reel->right      = right;
    reel->left       = left;
    reel->frameW     = frameW;
    reel->frameH     = frameH;
    reel->frameCount = (frameCount > 0) ? frameCount : 1;
    reel->fps        = (fps > 0.0f)     ? fps        : 12.0f;
    reel->loop       = loop;

    if (right) SDL_SetTextureBlendMode(right, SDL_BLENDMODE_BLEND);
    if (left)  SDL_SetTextureBlendMode(left,  SDL_BLENDMODE_BLEND);
}

/* ═══════════════════════════════════════════════════════════════════════
   initSpriteData
   Backward-compatible helper: builds ANIM_COUNT reels from a single
   shared spritesheet laid out as horizontal rows.

   For each row i, the source rect on the sheet is:
       x = 0,  y = i * frameH,  w = frameW * frameCounts[i],  h = frameH
   (This matches the original sheet format exactly.)
   
   If you have per-animation separate strips, build the reels manually
   with initAnimReel() instead of calling this helper.
═══════════════════════════════════════════════════════════════════════ */
void initSpriteData(SpriteData *s,
                    SDL_Texture *right, SDL_Texture *left,
                    int frameW, int frameH, int isCat,
                    const int   frameCounts[ANIM_COUNT],
                    const float fps       [ANIM_COUNT],
                    const int   loop      [ANIM_COUNT])
{
    int i;
    int   fc;
    float fp;
    int   lp;

    /* Resolve defaults */
    const int   *fc_table = frameCounts ? frameCounts : DEFAULT_FRAME_COUNTS;
    const float *fp_table = fps         ? fps         : DEFAULT_FPS;
    const int   *lp_table = loop        ? loop        : DEFAULT_LOOP;

    for (i = 0; i < ANIM_COUNT; i++) {
        fc = fc_table[i];
        fp = fp_table[i];
        lp = lp_table[i];

        /*
         * For the shared-sheet path we pass right/left as-is.
         * blitEntity() reads reel->frameW and the AnimState::frame to
         * compute the correct SDL_Rect source — see blitEntity().
         *
         * The Y offset into the shared sheet is:  i * frameH
         * We encode that as a tag by storing (-i - 1) in a reserved slot,
         * BUT — simpler and cleaner: we just store frameH * i into a
         * helper field.  See the NOTE in blitEntity() below.
         */
        initAnimReel(&s->reels[i], right, left,
                     frameW, frameH, fc, fp, lp);
    }

    /*
     * Bat double-jump row (BAT_FLY=7) vs Cat double-jump (CAT_DOUBLEJUMP=7)
     * both land on index 7 — the table handles them identically.
     * If you want to override the cat's double-jump separately, call
     * initAnimReel(&s->reels[CAT_DOUBLEJUMP], ...) after this function.
     */
    (void)isCat; /* kept for API compatibility */
}

/* ═══════════════════════════════════════════════════════════════════════
   initPlayer
═══════════════════════════════════════════════════════════════════════ */
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
    p->direction     = 1;
    p->onGround      = 1;
    p->vies          = 3;
    p->hp            = 100;
    p->isAlive       = 1;
    p->score         = 0;
    p->isCat         = isCat;

    /* Initialise animation cursor */
    p->anim.row      = 0;
    p->anim.frame    = 0;
    p->anim.timer    = 0.0f;
    p->anim.finished = 0;
}

/* ═══════════════════════════════════════════════════════════════════════
   setAnimRow  — switch to a new animation row
   Resets the cursor only when the row actually changes.
═══════════════════════════════════════════════════════════════════════ */
static void setAnimRow(Player *p, int row)
{
    if (p->anim.row == row) return;
    p->anim.row      = row;
    p->anim.frame    = 0;
    p->anim.timer    = 0.0f;
    p->anim.finished = 0;
}

/* ═══════════════════════════════════════════════════════════════════════
   advanceAnim  — tick the animation cursor by dt seconds
   Uses each reel's own fps and loop settings.
═══════════════════════════════════════════════════════════════════════ */
static void advanceAnim(Player *p, float dt)
{
    AnimReel  *reel = &p->sprite.reels[p->anim.row];
    AnimState *anim = &p->anim;

    if (anim->finished) return;   /* one-shot already done — hold last frame */

    anim->timer += dt;
    if (reel->fps <= 0.0f) return;

    float frameDur = 1.0f / reel->fps;
    while (anim->timer >= frameDur) {
        anim->timer -= frameDur;
        anim->frame++;
        if (anim->frame >= reel->frameCount) {
            if (reel->loop) {
                anim->frame = 0;           /* loop back */
            } else {
                anim->frame    = reel->frameCount - 1;  /* hold last frame */
                anim->finished = 1;
                break;
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════
   animateEntity  (backward-compatible wrapper)
   Old code that called animateEntity(p) once per game-tick still works;
   it advances by one fixed step (~12 fps, 0.083 s).
═══════════════════════════════════════════════════════════════════════ */
void animateEntity(Player *p)
{
    advanceAnim(p, 1.0f / 12.0f);
}

/* ═══════════════════════════════════════════════════════════════════════
   blitEntity  — render one frame of the current reel
   
   Shared-sheet layout (produced by initSpriteData):
     The sheet rows map 1:1 to animation indices.
     Row i starts at y = i * reel->frameH on the texture.
     Frame f starts at x = f * reel->frameW.
   
   Separate-strip layout (produced by initAnimReel):
     All frames in the strip are on row 0 (y = 0).
     Frame f starts at x = f * reel->frameW.
   
   We detect shared-sheet vs strip by checking whether all reels for this
   character point to the same SDL_Texture pointer.  If so, row offset
   is applied; if not (different textures per reel), y = 0.
═══════════════════════════════════════════════════════════════════════ */
void blitEntity(SDL_Renderer *r, Player *p)
{
    if (!p->isAlive) return;

    AnimReel  *reel     = &p->sprite.reels[p->anim.row];
    SDL_Texture *sheet  = p->direction ? reel->right : reel->left;

    /* Update draw rect */
    p->posScreen.x = (int)p->x;
    p->posScreen.y = (int)p->y;
    p->posScreen.w = p->w;
    p->posScreen.h = p->h;

    if (!sheet) {
        /* Fallback: solid colour rectangle */
        SDL_SetRenderDrawColor(r,
            p->isCat ? 210 : 80,
            p->isCat ?  50 : 80,
            p->isCat ? 210 : 200, 255);
        SDL_RenderFillRect(r, &p->posScreen);
        return;
    }

    /*
     * Determine whether this is a shared sheet (all rows on one texture)
     * or a per-reel strip.  We check if reel[0] and reel[row] share the
     * same texture pointer for the active direction.
     */
    SDL_Texture *baseSheet = p->direction
                             ? p->sprite.reels[0].right
                             : p->sprite.reels[0].left;
    int isSharedSheet = (sheet == baseSheet);

    SDL_Rect src;
    src.x = p->anim.frame * reel->frameW;
    src.y = isSharedSheet ? (p->anim.row * reel->frameH) : 0;
    src.w = reel->frameW;
    src.h = reel->frameH;

    SDL_RenderCopy(r, sheet, &src, &p->posScreen);
}

void renderPlayer(SDL_Renderer *r, Player *p) { blitEntity(r, p); }

/* ═══════════════════════════════════════════════════════════════════════
   Bullet helpers  (unchanged logic)
═══════════════════════════════════════════════════════════════════════ */
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
            SDL_SetRenderDrawColor(r, 200,  50, 200, 255);
        else
            SDL_SetRenderDrawColor(r, 255, 220,   0, 255);
        SDL_RenderFillRect(r, &br);
    }
}

/* ═══════════════════════════════════════════════════════════════════════
   Physics helpers  (unchanged)
═══════════════════════════════════════════════════════════════════════ */
void movePerso(Player *p, float dt)
{
    double dx = 0.5 * p->acceleration * dt * dt + p->vitesse * dt;
    p->x += dx;
    p->vitesse += p->acceleration * dt;
    p->posScreen.x = (int)p->x;
    if (p->x < 0)              { p->x = 0;              p->vitesse = 0; }
    if (p->x > SCREEN_W - p->w){ p->x = SCREEN_W - p->w; p->vitesse = 0; }
}

void saut(Player *p)
{
    if (p->up != 0) return;
    p->up         = 1;
    p->posinit_y  = p->y;
    p->saut_x_rel = SAUT_X_MIN;
}

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

/* ═══════════════════════════════════════════════════════════════════════
   handleInput  (unchanged logic)
═══════════════════════════════════════════════════════════════════════ */
void handleInput(Player *p, const Uint8 *keys, InputConfig cfg)
{
    int    moving;
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
        p->vitesse     = 0;
    }
    if (keys[cfg.kick] && !p->isKicking && !p->isAttacking && !p->isShooting) {
        p->isKicking   = 1;
        p->actionTimer = ACTION_DUR;
        p->vitesse     = 0;
    }
    if (keys[cfg.shoot] && !p->isShooting && !p->isAttacking && !p->isKicking) {
        p->isShooting  = 1;
        p->actionTimer = ACTION_DUR;
        fireBullet(p, p->isCat ? 1 : 0);
        p->vitesse = 0;
    }
}

/* ═══════════════════════════════════════════════════════════════════════
   updatePlayer
   Physics logic is identical to original.
   Animation section now uses setAnimRow() + advanceAnim().
═══════════════════════════════════════════════════════════════════════ */
void updatePlayer(Player *p, float dt,
                  SDL_Rect platforms[], int nbPlatforms)
{
    int      row, i;
    float    grav;
    SDL_Rect pRect, plat;
    int      feetY, platTop;

    if (!p->isAlive) return;

    /* ── action timer ── */
    if (p->actionTimer > 0) {
        p->actionTimer--;
        if (p->actionTimer == 0) {
            p->isAttacking = 0;
            p->isKicking   = 0;
            p->isShooting  = 0;
        }
    }

    /* ── horizontal movement ── */
    movePerso(p, dt);

    /* ── vertical movement ── */
    if (p->up) {
        updateSaut(p, dt);
        if (p->up == 0) {
            p->y             = p->posinit_y;
            p->onGround      = 1;
            p->canDoubleJump = p->isCat;
        }
    } else {
        if (!p->onGround) {
            grav = (p->isFlying && !p->isCat) ? GRAVITY * 0.3f : GRAVITY;
            p->vy += grav * dt;
            if (p->vy > 700) p->vy = 700;
            p->y += p->vy * dt;
        }
        /* Ground collision */
        if (p->y >= GROUND_Y) {
            p->y             = GROUND_Y;
            p->vy            = 0;
            p->onGround      = 1;
            p->canDoubleJump = p->isCat;
            p->isFlying      = 0;
        }
        /* Platform collision */
        if (p->vy >= 0 && platforms != NULL) {
            pRect.x = (int)p->x;
            pRect.y = (int)p->y;
            pRect.w = p->w;
            pRect.h = p->h;
            feetY   = pRect.y + pRect.h;
            for (i = 0; i < nbPlatforms; i++) {
                plat = platforms[i];
                if (pRect.x + pRect.w <= plat.x ||
                    pRect.x >= plat.x + plat.w)
                    continue;
                platTop = plat.y;
                if (feetY >= platTop &&
                    feetY <= platTop + plat.h + 20 &&
                    (int)p->y < platTop)
                {
                    p->y             = (double)(platTop - p->h);
                    p->vy            = 0;
                    p->onGround      = 1;
                    p->canDoubleJump = p->isCat;
                    p->isFlying      = 0;
                    break;
                }
            }
        }
    }

    if (p->y < 0) { p->y = 0; p->vy = 0; }

    /* ── animation row selection (same logic as original) ── */
    if (p->isAttacking)
        row = p->isCat ? CAT_ATTACK     : BAT_ATTACK;
    else if (p->isKicking)
        row = p->isCat ? CAT_KICK       : BAT_KICK;
    else if (p->isShooting)
        row = p->isCat ? CAT_SHOOT      : BAT_SHOOT;
    else if (!p->isCat && p->isFlying)
        row = BAT_FLY;
    else if (p->isCat && p->up && !p->canDoubleJump)
        row = CAT_DOUBLEJUMP;
    else if (p->up || !p->onGround)
        row = p->isCat ? CAT_JUMP       : BAT_JUMP;
    else if (p->isRunning)
        row = p->isCat ? CAT_RUN        : BAT_RUN;
    else if (p->vitesse != 0.0)
        row = p->isCat ? CAT_WALK       : BAT_WALK;
    else
        row = p->isCat ? CAT_IDLE       : BAT_IDLE;

    setAnimRow(p, row);          /* resets cursor only on row change     */
    advanceAnim(p, dt);          /* advances by real dt using reel->fps  */

    /* ── bullet positions ── */
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!p->bullets[i].active) continue;
        p->bullets[i].x += p->bullets[i].vx * dt;
        if (p->bullets[i].x < 0 || p->bullets[i].x > SCREEN_W)
            p->bullets[i].active = 0;
    }
}
