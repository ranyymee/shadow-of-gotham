#include "player.h"
#include <string.h>
#include <stdio.h>

#define SPEED        220.0
#define SPEED_RUN    380.0
#define ACCEL          0.0
#define GROUND_Y_VAL 190.0
#define ACTION_DUR    20
#define ANIM_SPD       0.1f

const int bat_x_right[ANIM_COUNT][MAX_ANIM_FRAMES] = {
    {  160 },
    {  137,  697, 1199, 1717, 2192, 2704 },
    {  115,  658, 1165, 1686, 2147, 2694 },
    {  118,  634, 1127, 1605, 2227 },
    {   97 },
    {   97 },
    {  133,  632, 1102, 1646, 2124, 2702 },
    {   92,  667, 1128, 1664, 2148, 2698 },
    {   94,  642 },
    {   59 },
    {  105,  558, 1095, 1577 },
};

const int bat_w_right[ANIM_COUNT][MAX_ANIM_FRAMES] = {
    { 224 },
    { 286, 216, 187, 211, 233, 285 },
    { 336, 285, 244, 257, 337, 280 },
    { 297, 297, 354, 354, 205 },
    { 297 },
    { 297 },
    { 232, 281, 380, 281, 351, 370 },
    { 322, 263, 436, 276, 343, 361 },
    { 268, 342 },
    { 347 },
    { 272, 453, 420, 504 },
};

const int bat_x_left[ANIM_COUNT][MAX_ANIM_FRAMES] = {
    { 2741 },
    { 2681, 2190, 1718, 1175,  678,  115 },
    { 2652, 2161, 1695, 1160,  619,  130 },
    { 2687, 2173, 1657, 1145,  672 },
    { 2709 },
    { 2709 },
    { 2772, 2223, 1654, 1209,  661,   65 },
    { 2700, 2184, 1550, 1175,  624,   55 },
    { 2741, 2120 },
    { 2697 },
    { 2779, 2148, 1644, 1072 },
};

const int bat_w_left[ANIM_COUNT][MAX_ANIM_FRAMES] = {
    { 224 },
    { 286, 233, 187, 211, 216, 285 },
    { 280, 337, 257, 244, 285, 336 },
    { 301, 297, 354, 354, 205 },
    { 297 },
    { 297 },
    { 370, 351, 281, 380, 281, 232 },
    { 361, 343, 276, 436, 263, 322 },
    { 268, 342 },
    { 347 },
    { 272, 453, 420, 504 },
};

const int bat_row_y[ANIM_COUNT] = {
     202,
     696,
    1208,
    1726,
    2241,
    2241,
    2746,
    3267,
    3770,
    4362,
    4791,
};

const int bat_row_h[ANIM_COUNT] = {
    487,
    512,
    511,
    500,
    496,
    496,
    506,
    493,
    507,
    368,
    510,
};

const int bat_frame_count[ANIM_COUNT] = {
    1, 6, 6, 5, 1, 1, 6, 6, 2, 1, 4
};

const int cat_x_right[ANIM_COUNT][MAX_ANIM_FRAMES] = {
    {  287 },
    {  235,  779, 1282, 1788, 2254, 2756 },
    {  268,  697, 1249, 1758, 2278, 2725 },
    {  247,  725, 1133, 1686, 2265 },
    {  221 },
    {  221 },
    {  254,  738, 1190, 1706, 2202, 2711 },
    {  201,  721, 1194, 1737, 2177, 2725 },
    {  188,  663, 1194 },
    {  179 },
    {  213,  643, 1132, 1620 },
};

const int cat_w_right[ANIM_COUNT][MAX_ANIM_FRAMES] = {
    { 164 },
    { 246, 179, 122, 146, 182, 242 },
    { 246, 254, 207, 198, 247, 252 },
    { 204, 229, 327, 272, 144 },
    { 227 },
    { 227 },
    { 172, 238, 347, 235, 307, 341 },
    { 268, 215, 384, 234, 327, 297 },
    { 324, 398, 353 },
    { 298 },
    { 227, 405, 441, 453 },
};

const int cat_x_left[ANIM_COUNT][MAX_ANIM_FRAMES] = {
    { 2685 },
    { 2655, 2178, 1733, 1202,  700,  138 },
    { 2623, 2185, 1680, 1178,  611,  159 },
    { 2685, 2182, 1676, 1178,  727 },
    { 2688 },
    { 2688 },
    { 2710, 2160, 1599, 1195,  627,   84 },
    { 2667, 2200, 1559, 1166,  633,  114 },
    { 2624, 2075, 1589 },
    { 2660 },
    { 2697, 2089, 1564, 1063 },
};

const int cat_w_left[ANIM_COUNT][MAX_ANIM_FRAMES] = {
    { 164 },
    { 246, 179, 122, 146, 182, 242 },
    { 246, 254, 207, 198, 247, 252 },
    { 204, 229, 327, 272, 144 },
    { 227 },
    { 227 },
    { 172, 238, 347, 235, 307, 341 },
    { 268, 215, 384, 234, 327, 297 },
    { 324, 398, 353 },
    { 298 },
    { 227, 405, 441, 453 },
};

const int cat_row_y[ANIM_COUNT] = {
       2,
     508,
    1022,
    1534,
    2091,
    2091,
    2621,
    3151,
    3651,
    4260,
    4777,
};

const int cat_row_h[ANIM_COUNT] = {
    490,
    513,
    512,
    552,
    530,
    530,
    522,
    500,
    545,
    461,
    480,
};

const int cat_frame_count[ANIM_COUNT] = {
    1, 6, 6, 5, 1, 1, 6, 6, 3, 1, 4
};

void bat_fill_sprite(SpriteData *s)
{
    int a, f;
    for (a = 0; a < ANIM_COUNT; a++) {
        s->frameCounts[a]  = bat_frame_count[a];
        s->frameRows[a]    = bat_row_y[a];
        s->frameHeights[a] = bat_row_h[a];
        for (f = 0; f < bat_frame_count[a]; f++) {
            s->frameOffsets[a][f] = bat_x_right[a][f];
            s->frameWidths[a][f]  = bat_w_right[a][f];
        }
    }
}

void cat_fill_sprite(SpriteData *s)
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

void animatePlayer(Player *p, float dt)
{
    int maxF;
    p->animTimer += dt;
    if (p->animTimer < p->animSpeed) return;
    p->animTimer = 0.0f;

    maxF = p->sprite.frameCounts[p->animState];
    if (maxF < 1) maxF = 1;

    if (p->animState == ANIM_DEAD) {
        if (p->animFrame < maxF - 1) p->animFrame++;
        p->animSpeed = 0.25f;
        return;
    }
    p->animSpeed = ANIM_SPD;
    p->animFrame = (p->animFrame + 1) % maxF;
}

void blitPlayer(SDL_Renderer *r, Player *p)
{
    SDL_Texture *sheet;
    SDL_Rect src, dst;
    int row, xOff, fW, fH, renderH, renderW;

    if (!p->isAlive) return;

    sheet = p->direction ? p->sprite.sheetRight : p->sprite.sheetLeft;
    row   = p->sprite.frameRows[p->animState];

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

    renderH = p->h;

    /* زد الحجم كي يكون في الهواء — كلما بعد على الأرض كلما كبر */
    if (!p->onGround) {
        float distFromGround = (float)(GROUND_Y_VAL - p->y);
        if (distFromGround < 0.0f) distFromGround = 0.0f;
        float maxDist = (float)(-JUMP_VY / 2.0f);
        float t = distFromGround / maxDist;
        if (t > 1.0f) t = 1.0f;
        float scale = 1.0f + 0.25f * t; /* زيادة حتى 25% */
        renderH = (int)(p->h * scale);
    }

    renderW = (fH > 0) ? (fW * renderH / fH) : p->w;

    if (p->animState == ANIM_CROUCH || p->animState == ANIM_DEAD) {
        renderW = p->w * 3 / 2;
        renderH = (fW > 0) ? (fH * renderW / fW) : fH;
        if (renderH > p->h) { renderH = p->h; renderW = (fH > 0) ? (fW * renderH / fH) : p->w; }
    }

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

    SDL_SetTextureColorMod(sheet, 255, 255, 255);

    SDL_RenderCopy(r, sheet, &src, &dst);
}

void updatePlayer(Player *p, float dt)
{
    double dx;
    int    i, newState;
    float  grav;

    if (!p->isAlive) {
        p->animState = ANIM_DEAD;
        animatePlayer(p, dt);
        return;
    }

    if (p->actionTimer > 0) {
        p->actionTimer--;
        p->vitesse = 0.0;
        if (p->actionTimer == 0) {
            p->isAttacking = 0;
            p->isKicking   = 0;
            p->isShooting  = 0;
        }
    }

    dx = p->vitesse * dt;
    p->x += dx;
    if (p->x < 0)               { p->x = 0;               p->vitesse = 0; }
    if (p->x > SCREEN_W - p->w) { p->x = SCREEN_W - p->w; p->vitesse = 0; }

    if (!p->onGround && !p->jumping && !p->hopping) {
        p->coyoteTimer += dt;
    } else if (p->onGround) {
        p->coyoteTimer = 0.0f;
    }

    if (p->jumpBuffer > 0.0f) p->jumpBuffer -= dt;
    if (p->jumpBuffer < 0.0f) p->jumpBuffer = 0.0f;

    grav = JUMP_GRAVITY;
    if (p->animState == ANIM_DOUBLE_JUMP && p->vy < 0)
        grav = JUMP_GRAVITY * 0.5f;
    else if (p->vy > 0)
        grav = JUMP_GRAVITY * 1.4f;

    if (!p->onGround || p->vy < 0) {
        p->vy += grav * dt;
        if (p->vy > 900.0f) p->vy = 900.0f;
        p->y += p->vy * dt;
    }

    if (p->y >= GROUND_Y_VAL) {
        p->y           = GROUND_Y_VAL;
        p->vy          = 0.0f;
        p->onGround    = 1;
        p->jumping     = 0;
        p->hopping     = 0;
        p->jumpCut     = 0;
        p->coyoteTimer = 0.0f;
        p->canDoubleJump = 0;
        p->isFlying    = 0;

        if (p->jumpBuffer > 0.0f) {
            p->jumpBuffer = 0.0f;
            p->onGround   = 0;
            p->jumping    = 1;
            p->jumpCut    = 0;
            p->vy         = JUMP_VY;
            p->animState  = ANIM_JUMP;
            p->animFrame  = 0;
        }
    }
    if (p->y < 0.0) { p->y = 0.0; if (p->vy < 0) p->vy = 0.0f; }

    if      (p->isAttacking)                                          newState = ANIM_ATTACK;
    else if (p->isKicking)                                            newState = ANIM_KICK;
    else if (p->isShooting)                                           newState = ANIM_SHOOT;
    else if (p->hopping)                                              newState = ANIM_UP;
    else if (!p->onGround && p->animState == ANIM_DOUBLE_JUMP
             && p->canDoubleJump == 0)                                newState = ANIM_DOUBLE_JUMP;
    else if (!p->onGround)                                            newState = ANIM_JUMP;
    else if (p->isCrouching)                                          newState = ANIM_CROUCH;
    else if (p->isRunning)                                            newState = ANIM_RUN;
    else if (p->vitesse != 0.0)                                       newState = ANIM_WALK;
    else                                                              newState = ANIM_IDLE;

    if (newState != p->animState) {
        p->animState = newState;
        p->animFrame = 0;
        p->animTimer = 0.0f;
    }

    if (p->animState == ANIM_JUMP) {
        int maxF = p->sprite.frameCounts[ANIM_JUMP];
        if (maxF > 1) {
            float vy_min = JUMP_VY;
            float vy_max = -vy_min * 1.4f;
            float t = (p->vy - vy_min) / (vy_max - vy_min);
            int targetFrame;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            targetFrame = (int)(t * (maxF - 1) + 0.5f);
            if (targetFrame > p->animFrame)
                p->animFrame = targetFrame;
        }
    } else {
        animatePlayer(p, dt);
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
