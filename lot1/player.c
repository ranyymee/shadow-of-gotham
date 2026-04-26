#include "player.h"
#include <string.h>
#include <stdio.h>

/* ── tuning ── */
#define SPEED        220.0
#define SPEED_RUN    380.0
#define ACCEL          0.0
#define GROUND_Y_VAL 190.0   /* feet at pixel 490 (190+300=490), matches visual ground */
#define ACTION_DUR    20
#define ANIM_SPD       0.1f

/* ══════════════════════════════════════════════════════════════════════
   PIXEL-PERFECT frame data — measured with Python/Pillow directly from
   the actual PNG files.

   batman_RIGHT_1.png  (3136 × 5325 px)
   batman_LEFT_1.png   (3125 × 5325 px)

   Sheet structure (10 content bands, measured from alpha channel):
     Band 0  y=202..688   → IDLE         (1 frame,  h=487)
     Band 1  y=696..1207  → WALK         (6 frames, h=512)
     Band 1b y=1208..1718 → RUN          (6 frames, h=511)
     Band 2  y=1726..2225 → JUMP         (5 frames, h=500)
     Band 3  y=2241..2736 → DOUBLE_JUMP/UP (1 frame, h=496)
     Band 4  y=2746..3251 → ATTACK       (6 frames, h=506)
     Band 5  y=3267..3759 → KICK         (6 frames, h=493)
     Band 6  y=3770..4276 → SHOOT        (2 frames, h=507)
     Band 7  y=4362..4729 → CROUCH       (1 frame,  h=368)
     Band 8  y=4791..5300 → DEAD         (4 frames, h=510)

   LEFT sheet frames are stored in playback order (already left-to-right
   in the left sheet for most anims; reversed only where needed).
   ══════════════════════════════════════════════════════════════════════ */

/* ── RIGHT sheet: X start of each frame ── */
static const int bat_x_right[ANIM_COUNT][MAX_ANIM_FRAMES] = {
    /* IDLE        */ {  160 },
    /* WALK        */ {  137,  697, 1199, 1717, 2192, 2704 },
    /* RUN         */ {  115,  658, 1165, 1686, 2147, 2694 },
    /* JUMP        */ {  118,  634, 1127, 1605, 2227 },
    /* DOUBLE_JUMP */ {   97 },
    /* UP          */ {   97 },
    /* ATTACK      */ {  133,  632, 1102, 1646, 2124, 2702 },
    /* KICK        */ {   92,  667, 1128, 1664, 2148, 2698 },
    /* SHOOT       */ {   94,  642 },
    /* CROUCH      */ {   59 },
    /* DEAD        */ {  105,  558, 1095, 1577 },
};
/* ── RIGHT sheet: pixel width of each frame ── */
static const int bat_w_right[ANIM_COUNT][MAX_ANIM_FRAMES] = {
    /* IDLE        */ { 224 },
    /* WALK        */ { 286, 216, 187, 211, 233, 285 },
    /* RUN         */ { 336, 285, 244, 257, 337, 280 },
    /* JUMP        */ { 297, 297, 354, 354, 205 },
    /* DOUBLE_JUMP */ { 297 },
    /* UP          */ { 297 },
    /* ATTACK      */ { 232, 281, 380, 281, 351, 370 },
    /* KICK        */ { 322, 263, 436, 276, 343, 361 },
    /* SHOOT       */ { 268, 342 },
    /* CROUCH      */ { 347 },
    /* DEAD        */ { 272, 453, 420, 504 },
};

/* ── LEFT sheet: X start of each frame (playback order) ── */
static const int bat_x_left[ANIM_COUNT][MAX_ANIM_FRAMES] = {
    /* IDLE        */ { 2741 },
    /* WALK        */ { 2681, 2190, 1718, 1175,  678,  115 },
    /* RUN         */ { 2652, 2161, 1695, 1160,  619,  130 },
    /* JUMP        */ { 2687, 2173, 1657, 1145,  672 },
    /* DOUBLE_JUMP */ { 2709 },
    /* UP          */ { 2709 },
    /* ATTACK      */ { 2772, 2223, 1654, 1209,  661,   65 },
    /* KICK        */ { 2700, 2184, 1550, 1175,  624,   55 },
    /* SHOOT       */ { 2741, 2120 },
    /* CROUCH      */ { 2697 },
    /* DEAD        */ { 2779, 2148, 1644, 1072 },
};
/* ── LEFT sheet: pixel width of each frame ── */
static const int bat_w_left[ANIM_COUNT][MAX_ANIM_FRAMES] = {
    /* IDLE        */ { 224 },
    /* WALK        */ { 286, 233, 187, 211, 216, 285 },
    /* RUN         */ { 280, 337, 257, 244, 285, 336 },
    /* JUMP        */ { 301, 297, 354, 354, 205 },
    /* DOUBLE_JUMP */ { 297 },
    /* UP          */ { 297 },
    /* ATTACK      */ { 370, 351, 281, 380, 281, 232 },
    /* KICK        */ { 361, 343, 276, 436, 263, 322 },
    /* SHOOT       */ { 268, 342 },
    /* CROUCH      */ { 347 },
    /* DEAD        */ { 272, 453, 420, 504 },
};

/* Absolute Y pixel offset of each animation row (same for both sheets) */
static const int bat_row_y[ANIM_COUNT] = {
    /* IDLE        */  202,
    /* WALK        */  696,
    /* RUN         */ 1208,
    /* JUMP        */ 1726,
    /* DOUBLE_JUMP */ 2241,
    /* UP          */ 2241,
    /* ATTACK      */ 2746,
    /* KICK        */ 3267,
    /* SHOOT       */ 3770,
    /* CROUCH      */ 4362,
    /* DEAD        */ 4791,
};
/* Height of each animation row (exact pixel content height) */
static const int bat_row_h[ANIM_COUNT] = {
    /* IDLE        */ 487,
    /* WALK        */ 512,
    /* RUN         */ 511,
    /* JUMP        */ 500,
    /* DOUBLE_JUMP */ 496,
    /* UP          */ 496,
    /* ATTACK      */ 506,
    /* KICK        */ 493,
    /* SHOOT       */ 507,
    /* CROUCH      */ 368,
    /* DEAD        */ 510,
};
/* Frame count per animation */
static const int bat_frame_count[ANIM_COUNT] = {
    /* IDLE  WALK  RUN  JUMP  DBL_JMP  UP  ATTACK  KICK  SHOOT  CROUCH  DEAD */
       1,    6,    6,   5,    1,       1,  6,      6,    2,     1,      4
};

static void bat_fill_sprite(SpriteData *s)
{
    int a, f;
    /* frameW/frameH are not used for Y calculation anymore (rows are non-uniform).
       We store the absolute Y in frameRows[] and the row height in frameHeights[].
       frameOffsets[][] stores the exact X pixel start of each frame. */
    for (a = 0; a < ANIM_COUNT; a++) {
        s->frameCounts[a]  = bat_frame_count[a];
        s->frameRows[a]    = bat_row_y[a];          /* absolute Y, not row index */
        s->frameHeights[a] = bat_row_h[a];
        for (f = 0; f < bat_frame_count[a]; f++) {
            s->frameOffsets[a][f] = bat_x_right[a][f]; /* default to right; left patched in blitPlayer */
            s->frameWidths[a][f]  = bat_w_right[a][f];
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════
   CATWOMAN frame data — pixel-perfect measured from catwoman PNG files
   Sheet size: 3136 × 5325 px

   Sheet structure (DIFFERENT from Batman — bands measured from alpha):
     Band 0  y=2..491      → IDLE         (1 frame,  h=490)
     Band 1  y=508..1020   → WALK         (6 frames, h=513)
     Band 2a y=1022..1533  → RUN          (6 frames, h=512)
     Band 2b y=1534..2085  → JUMP         (5 frames, h=552)
     Band 3a y=2091..2620  → DOUBLE_JUMP  (1 frame,  h=530)
     Band 3a y=2091..2620  → UP           (1 frame,  h=530)
     Band 3b y=2621..3142  → ATTACK       (6 frames, h=522)
     Band 4a y=3151..3650  → KICK         (6 frames, h=500)
     Band 4b y=3651..4195  → SHOOT        (3 frames, h=545)
     Band 5  y=4260..4720  → CROUCH       (1 frame,  h=461)
     Band 6  y=4777..5256  → DEAD         (4 frames, h=480)
   ══════════════════════════════════════════════════════════════════════ */

/* ── Catwoman RIGHT sheet: X start of each frame ── */
static const int cat_x_right[ANIM_COUNT][MAX_ANIM_FRAMES] = {
    /* IDLE        */ {  287 },
    /* WALK        */ {  235,  779, 1282, 1788, 2254, 2756 },
    /* RUN         */ {  268,  697, 1249, 1758, 2278, 2725 },
    /* JUMP        */ {  247,  725, 1133, 1686, 2265 },
    /* DOUBLE_JUMP */ {  221 },
    /* UP          */ {  221 },
    /* ATTACK      */ {  254,  738, 1190, 1706, 2202, 2711 },
    /* KICK        */ {  201,  721, 1194, 1737, 2177, 2725 },
    /* SHOOT       */ {  188,  663, 1194 },
    /* CROUCH      */ {  179 },
    /* DEAD        */ {  213,  643, 1132, 1620 },
};
/* ── Catwoman RIGHT sheet: pixel width of each frame ── */
static const int cat_w_right[ANIM_COUNT][MAX_ANIM_FRAMES] = {
    /* IDLE        */ { 164 },
    /* WALK        */ { 246, 179, 122, 146, 182, 242 },
    /* RUN         */ { 246, 254, 207, 198, 247, 252 },
    /* JUMP        */ { 204, 229, 327, 272, 144 },
    /* DOUBLE_JUMP */ { 227 },
    /* UP          */ { 227 },
    /* ATTACK      */ { 172, 238, 347, 235, 307, 341 },
    /* KICK        */ { 268, 215, 384, 234, 327, 297 },
    /* SHOOT       */ { 324, 398, 353 },
    /* CROUCH      */ { 298 },
    /* DEAD        */ { 227, 405, 441, 453 },
};

/* ── Catwoman LEFT sheet: X start of each frame (playback order) ── */
static const int cat_x_left[ANIM_COUNT][MAX_ANIM_FRAMES] = {
    /* IDLE        */ { 2685 },
    /* WALK        */ { 2655, 2178, 1733, 1202,  700,  138 },
    /* RUN         */ { 2623, 2185, 1680, 1178,  611,  159 },
    /* JUMP        */ { 2685, 2182, 1676, 1178,  727 },
    /* DOUBLE_JUMP */ { 2688 },
    /* UP          */ { 2688 },
    /* ATTACK      */ { 2710, 2160, 1599, 1195,  627,   84 },
    /* KICK        */ { 2667, 2200, 1559, 1166,  633,  114 },
    /* SHOOT       */ { 2624, 2075, 1589 },
    /* CROUCH      */ { 2660 },
    /* DEAD        */ { 2697, 2089, 1564, 1063 },
};
/* ── Catwoman LEFT sheet: pixel width of each frame ── */
static const int cat_w_left[ANIM_COUNT][MAX_ANIM_FRAMES] = {
    /* IDLE        */ { 164 },
    /* WALK        */ { 246, 179, 122, 146, 182, 242 },
    /* RUN         */ { 246, 254, 207, 198, 247, 252 },
    /* JUMP        */ { 204, 229, 327, 272, 144 },
    /* DOUBLE_JUMP */ { 227 },
    /* UP          */ { 227 },
    /* ATTACK      */ { 172, 238, 347, 235, 307, 341 },
    /* KICK        */ { 268, 215, 384, 234, 327, 297 },
    /* SHOOT       */ { 324, 398, 353 },
    /* CROUCH      */ { 298 },
    /* DEAD        */ { 227, 405, 441, 453 },
};

/* Catwoman Y rows and heights — measured from actual PNG alpha channel */
static const int cat_row_y[ANIM_COUNT] = {
    /* IDLE        */    2,
    /* WALK        */  508,
    /* RUN         */ 1022,
    /* JUMP        */ 1534,
    /* DOUBLE_JUMP */ 2091,
    /* UP          */ 2091,
    /* ATTACK      */ 2621,
    /* KICK        */ 3151,
    /* SHOOT       */ 3651,
    /* CROUCH      */ 4260,
    /* DEAD        */ 4777,
};
static const int cat_row_h[ANIM_COUNT] = {
    /* IDLE        */ 490,
    /* WALK        */ 513,
    /* RUN         */ 512,
    /* JUMP        */ 552,
    /* DOUBLE_JUMP */ 530,
    /* UP          */ 530,
    /* ATTACK      */ 522,
    /* KICK        */ 500,
    /* SHOOT       */ 545,
    /* CROUCH      */ 461,
    /* DEAD        */ 480,
};
static const int cat_frame_count[ANIM_COUNT] = {
    /* IDLE  WALK  RUN  JUMP  DBL_JMP  UP  ATTACK  KICK  SHOOT  CROUCH  DEAD */
       1,    6,    6,   5,    1,       1,  6,      6,    3,     1,      4
};

static void cat_fill_sprite(SpriteData *s)
{
    int a, f;
    for (a = 0; a < ANIM_COUNT; a++) {
        s->frameCounts[a]  = cat_frame_count[a];
        s->frameRows[a]    = cat_row_y[a];
        s->frameHeights[a] = cat_row_h[a];
        for (f = 0; f < cat_frame_count[a]; f++) {
            s->frameOffsets[a][f] = cat_x_right[a][f];
            s->frameWidths[a][f]  = cat_w_right[a][f];
        }
    }
}

/* ─────────────────────────────────────────────────────── */

void initSpriteData(SpriteData *s, SDL_Texture *right, SDL_Texture *left,
                    int frameW, int frameH, int isCat)
{
    memset(s, 0, sizeof(*s));
    s->sheetRight = right;
    s->sheetLeft  = left;
    s->frameW     = frameW;
    s->frameH     = frameH;

    if (right) SDL_SetTextureBlendMode(right, SDL_BLENDMODE_BLEND);
    if (left)  SDL_SetTextureBlendMode(left,  SDL_BLENDMODE_BLEND);

    if (isCat) cat_fill_sprite(s);
    else       bat_fill_sprite(s);
}

/* ─────────────────────────────────────────────────────── */

void initPlayer(Player *p, float x, float y, int isCat)
{
    int i;
    memset(p, 0, sizeof(Player));

    p->x = (double)x;
    p->y = (double)y;
    p->vitesse      = 0.0;
    p->acceleration = ACCEL;
    p->vy           = 0.0f;

    p->w = 100; p->h = 300;
    p->posScreen.x = (int)x;
    p->posScreen.y = (int)y;
    p->posScreen.w = p->w;
    p->posScreen.h = p->h;

    p->jumping       = 0;
    p->hopping       = 0;
    p->posinit_y     = (double)y;
    p->saut_x_rel    = SAUT_X_MIN;
    p->canDoubleJump = isCat;
    p->jumpPressed   = 0;
    p->jumpCut       = 0;
    p->coyoteTimer   = 0.0f;
    p->jumpBuffer    = 0.0f;
    p->isFlying      = 0;

    p->direction  = 1;
    p->animState  = ANIM_IDLE;
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

/* ─────────────────────────────────────────────────────── */

void animatePlayer(Player *p, float dt)
{
    int maxF;
    p->animTimer += dt;
    if (p->animTimer < p->animSpeed) return;
    p->animTimer = 0.0f;

    maxF = p->sprite.frameCounts[p->animState];
    if (maxF < 1) maxF = 1;

    /* For dead: hold last frame, play slower */
    if (p->animState == ANIM_DEAD) {
        if (p->animFrame < maxF - 1) p->animFrame++;
        p->animSpeed = 0.25f;   /* slow dramatic fall */
        return;
    }
    p->animSpeed = ANIM_SPD;    /* restore normal speed for other states */
    p->animFrame = (p->animFrame + 1) % maxF;
}

/* ─────────────────────────────────────────────────────── */

void blitPlayer(SDL_Renderer *r, Player *p)
{
    SDL_Texture *sheet;
    SDL_Rect src, dst;
    int row, xOff, fW, fH, renderH, renderW;

    if (!p->isAlive) return;

    /* left sheet already drawn facing left — no flip needed */
    sheet = p->direction ? p->sprite.sheetRight : p->sprite.sheetLeft;

    /* ── source rect: exact pixel region of this frame ── */
    row  = p->sprite.frameRows[p->animState];   /* absolute Y offset */

    if (p->isCat) {
        if (p->direction) {
            xOff = cat_x_right[p->animState][p->animFrame];
            fW   = cat_w_right[p->animState][p->animFrame];
        } else {
            xOff = cat_x_left[p->animState][p->animFrame];
            fW   = cat_w_left[p->animState][p->animFrame];
        }
    } else {
        if (p->direction) {
            xOff = bat_x_right[p->animState][p->animFrame];
            fW   = bat_w_right[p->animState][p->animFrame];
        } else {
            xOff = bat_x_left[p->animState][p->animFrame];
            fW   = bat_w_left[p->animState][p->animFrame];
        }
    }
    fH = p->sprite.frameHeights[p->animState];

    src.x = xOff;
    src.y = row;
    src.w = (fW > 0) ? fW : 1;
    src.h = (fH > 0) ? fH : 1;

    /* ── destination rect ──
       Scale frame height proportionally to p->h, keep aspect ratio.
       ALWAYS anchor the BOTTOM of the rendered sprite to (p->y + p->h).
       This keeps the feet on the ground regardless of which animation row
       is active (rows vary from 366px to 510px in the sheet). */
    renderH = p->h;
    renderW = (fH > 0) ? (fW * renderH / fH) : p->w;

    /* For CROUCH and DEAD the sprite is shorter — scale proportionally
       so the character is not stretched to standing height. */
    if (p->animState == ANIM_CROUCH || p->animState == ANIM_DEAD) {
        renderW = p->w * 3 / 2;
        renderH = (fW > 0) ? (fH * renderW / fW) : fH;
        if (renderH > p->h) { renderH = p->h; renderW = (fH > 0) ? (fW * renderH / fH) : p->w; }
    }

    /* Anchor feet: bottom of sprite always at p->y + p->h */
    dst.x = (int)p->x - (renderW - p->w) / 2;
    dst.y = (int)(p->y + p->h) - renderH;
    dst.w = renderW;
    dst.h = renderH;
    p->posScreen = dst;

    if (!sheet) {
        SDL_SetRenderDrawColor(r, p->isCat ? 210 : 80,
                                  p->isCat ?  50 : 80,
                                  p->isCat ? 210 : 200, 255);
        SDL_Rect box = { (int)p->x, (int)p->y, p->w, p->h };
        SDL_RenderFillRect(r, &box);
        return;
    }

    /* Tint double-jump cyan */
    if (p->animState == ANIM_DOUBLE_JUMP)
        SDL_SetTextureColorMod(sheet, 80, 220, 255);
    else
        SDL_SetTextureColorMod(sheet, 255, 255, 255);

    SDL_RenderCopy(r, sheet, &src, &dst);
}

/* ─────────────────────────────────────────────────────── */

void updatePlayer(Player *p, float dt)
{
    double dx;
    int    i, newState;

    if (!p->isAlive) {
        p->animState = ANIM_DEAD;
        animatePlayer(p, dt);
        return;
    }

    /* ── action timer: freeze movement during attacks ── */
    if (p->actionTimer > 0) {
        p->actionTimer--;
        p->vitesse = 0.0;   /* lock in place for the full action duration */
        if (p->actionTimer == 0) {
            p->isAttacking = 0;
            p->isKicking   = 0;
            p->isShooting  = 0;
        }
    }

    /* ── horizontal movement ── */
    dx = p->vitesse * dt;
    p->x += dx;
    if (p->x < 0)              { p->x = 0;              p->vitesse = 0; }
    if (p->x > SCREEN_W - p->w){ p->x = SCREEN_W - p->w; p->vitesse = 0; }

    /* ── vertical physics (unified velocity + gravity) ── */
    {
        float grav;

        /* Coyote timer: counts up while off ground without jumping */
        if (!p->onGround && !p->jumping && !p->hopping) {
            p->coyoteTimer += dt;
        } else if (p->onGround) {
            p->coyoteTimer = 0.0f;
        }

        /* Jump buffer: counts down after pressing jump in air */
        if (p->jumpBuffer > 0.0f)
            p->jumpBuffer -= dt;
        if (p->jumpBuffer < 0.0f)
            p->jumpBuffer = 0.0f;

        /* Gravity multiplier: fall faster when moving down (juicy feel) */
        grav = JUMP_GRAVITY;
        if (p->animState == ANIM_DOUBLE_JUMP && p->vy < 0)
            grav = JUMP_GRAVITY * 0.5f;  /* lighter gravity on way up during double jump */
        else if (p->vy > 0)
            grav = JUMP_GRAVITY * 1.4f;  /* faster fall than rise */

        if (!p->onGround || p->vy < 0) {
            p->vy += grav * dt;
            if (p->vy >  900.0f) p->vy =  900.0f;
            p->y  += p->vy * dt;
        }

        /* Land on ground */
        if (p->y >= GROUND_Y_VAL) {
            p->y           = GROUND_Y_VAL;
            p->vy          = 0.0f;
            p->onGround    = 1;
            p->jumping     = 0;
            p->hopping     = 0;
            p->jumpCut     = 0;
            p->coyoteTimer = 0.0f;
            p->canDoubleJump = 0;  /* re-granted on next takeoff in handleInput */
            p->isFlying    = 0;

            /* Consume buffered jump */
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
        if (p->y < 0.0) { p->y = 0.0; if (p->vy < 0) p->vy = 0.0f; }
    }

    /* ── determine animation state ── */
    if (p->isAttacking)                          newState = ANIM_ATTACK;
    else if (p->isKicking)                       newState = ANIM_KICK;
    else if (p->isShooting)                      newState = ANIM_SHOOT;
    else if (p->hopping)                         newState = ANIM_UP;
    else if (!p->onGround && p->animState == ANIM_DOUBLE_JUMP
             && p->canDoubleJump == 0)           newState = ANIM_DOUBLE_JUMP;
    else if (!p->onGround)                       newState = ANIM_JUMP;
    else if (p->isCrouching)                     newState = ANIM_CROUCH;
    else if (p->isRunning)                       newState = ANIM_RUN;
    else if (p->vitesse != 0.0)                  newState = ANIM_WALK;
    else                                         newState = ANIM_IDLE;

    /* Apply state change (resets frame/timer on transition) */
    if (newState != p->animState) {
        p->animState = newState;
        p->animFrame = 0;
        p->animTimer = 0.0f;
    }

    /* ── Jump frame: map vy → frame index (smooth arc, never go backwards) ──
       JUMP has 5 frames:
         0 = crouch/launch  1 = rising  2 = apex  3 = falling  4 = landing
       We drive the frame directly from velocity so it follows the physics arc. */
    if (p->animState == ANIM_JUMP) {
        int maxF = p->sprite.frameCounts[ANIM_JUMP];
        if (maxF > 1) {
            /* vy goes from JUMP_VY (negative, up) to ~+840 (falling fast)
               Map that range linearly onto [0 .. maxF-1] */
            float vy_min = JUMP_VY;            /* e.g. -600  (full rise) */
            float vy_max = -vy_min * 1.4f;     /* e.g. +840  (full fall) */
            float t = (p->vy - vy_min) / (vy_max - vy_min);
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            int targetFrame = (int)(t * (maxF - 1) + 0.5f);
            /* Only advance — never flicker backwards */
            if (targetFrame > p->animFrame)
                p->animFrame = targetFrame;
        }
        /* Skip normal animatePlayer ticker for jump — frame is physics-driven */
        goto skip_animate;
    }

    animatePlayer(p, dt);
skip_animate:

    /* ── bullet update ── */
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!p->bullets[i].active) continue;
        p->bullets[i].x += p->bullets[i].vx * dt;
        if (p->bullets[i].x < 0 || p->bullets[i].x > SCREEN_W)
            p->bullets[i].active = 0;
    }
}

/* ─────────────────────────────────────────────────────── */

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
