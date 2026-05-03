#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>

#include "player.h"
#include "menu.h"

#define MAX_KEYS   16
#define ACTION_DUR 20

void loadPlayerTextures(SDL_Renderer *ren, Player *p)
{
    char pr[128], pl[128];
    int  fw, fh;
    SDL_Texture *outR, *outL;
    const char  *name = p->isCat ? "catwoman" : "batman";

    snprintf(pr, sizeof(pr), "%s_right_%d.png", name, p->costume + 1);
    snprintf(pl, sizeof(pl), "%s_left_%d.png",  name, p->costume + 1);
    outR = IMG_LoadTexture(ren, pr);
    outL = IMG_LoadTexture(ren, pl);

    if (!outR) {
        snprintf(pr, sizeof(pr), "assets/%s_right_%d.png", name, p->costume + 1);
        outR = IMG_LoadTexture(ren, pr);
    }
    if (!outL) {
        snprintf(pl, sizeof(pl), "assets/%s_left_%d.png", name, p->costume + 1);
        outL = IMG_LoadTexture(ren, pl);
    }

    fprintf(stderr, "[TEXTURE] %s right -> %s  (%s)\n",
            name, pr, outR ? "OK" : "FAILED");
    fprintf(stderr, "[TEXTURE] %s left  -> %s  (%s)\n",
            name, pl, outL ? "OK" : "FAILED");
    if (!outR) fprintf(stderr, "  SDL_Error right: %s\n", IMG_GetError());
    if (!outL) fprintf(stderr, "  SDL_Error left : %s\n", IMG_GetError());

    fw = p->isCat ? CAT_FRAME_W : BAT_FRAME_W;
    fh = p->isCat ? CAT_FRAME_H : BAT_FRAME_H;
    initSpriteData(&p->sprite, outR, outL, fw, fh, p->isCat);
}

void handleInput(Player *p, SDL_Scancode *keys_pressed, InputConfig cfg, int numKeys)
{
    int kLeft, kRight, kUp, kJump, kPunch, kKick, kShoot, kCrouch, kShift;
    int moving, i;

    if (!p->isAlive) return;

    kLeft = kRight = kUp = kJump = kPunch = kKick = kShoot = kCrouch = kShift = 0;

    for (i = 0; i < numKeys; i++) {
        if (keys_pressed[i] == cfg.left)              kLeft   = 1;
        if (keys_pressed[i] == cfg.right)             kRight  = 1;
        if (keys_pressed[i] == cfg.up)                kUp     = 1;
        if (keys_pressed[i] == cfg.jump)              kJump   = 1;
        if (keys_pressed[i] == cfg.punch)             kPunch  = 1;
        if (keys_pressed[i] == cfg.kick)              kKick   = 1;
        if (keys_pressed[i] == cfg.shoot)             kShoot  = 1;
        if (keys_pressed[i] == cfg.crouch)            kCrouch = 1;
        if (keys_pressed[i] == SDL_SCANCODE_LSHIFT ||
            keys_pressed[i] == SDL_SCANCODE_RSHIFT)   kShift  = 1;
    }

    if (p->actionTimer > 0) { p->vitesse = 0; return; }

    p->isCrouching = 0;
    p->isRunning   = 0;

    if (p->onGround && kCrouch && !kShift) {
        p->isCrouching = 1;
        p->vitesse = 0.0;
        if (!p->jumping && !p->hopping) {
            if (kLeft)  { p->direction = 0; p->vitesse = -SPEED * 0.5; }
            if (kRight) { p->direction = 1; p->vitesse =  SPEED * 0.5; }
        }
        p->jumpPressed = 0;

        if (kPunch && !p->isAttacking && !p->isKicking && !p->isShooting) {
            p->isAttacking = 1;
            p->actionTimer = ACTION_DUR;
            p->vitesse     = 0;
        }
        if (kKick && !p->isKicking && !p->isAttacking && !p->isShooting) {
            p->isKicking   = 1;
            p->actionTimer = ACTION_DUR;
            p->vitesse     = 0;
        }
        if (kShoot && !p->isShooting && !p->isAttacking && !p->isKicking) {
            int j;
            float bY = (float)(p->posScreen.y + p->posScreen.h * 38 / 100);
            float bX = p->direction
                       ? (float)(p->posScreen.x + p->posScreen.w)
                       : (float)(p->posScreen.x);
            for (j = 0; j < MAX_BULLETS; j++) {
                if (!p->bullets[j].active) {
                    p->bullets[j].active = 1;
                    p->bullets[j].owner  = p->isCat ? 1 : 0;
                    p->bullets[j].x      = bX;
                    p->bullets[j].y      = bY;
                    p->bullets[j].vx     = p->direction ? 500.0f : -500.0f;
                    break;
                }
            }
            p->isShooting  = 1;
            p->actionTimer = ACTION_DUR;
            p->vitesse     = 0;
        }
        return;
    }

    p->vitesse = 0.0;
    moving = 0;
    if (!p->jumping && !p->hopping) {
        if (kLeft)  { p->direction = 0; moving = 1; }
        if (kRight) { p->direction = 1; moving = 1; }
        if (moving) {
            if (kShift) {
                p->isRunning = 1;
                p->vitesse   = p->direction ? SPEED_RUN : -SPEED_RUN;
            } else {
                p->vitesse = p->direction ? SPEED : -SPEED;
            }
        }
    } else {
        if (kLeft)  { p->direction = 0; p->vitesse = kShift ? -SPEED_RUN * 0.7 : -SPEED * 0.6; }
        if (kRight) { p->direction = 1; p->vitesse = kShift ?  SPEED_RUN * 0.7 :  SPEED * 0.6; }
        if (kLeft || kRight) p->isRunning = kShift ? 1 : 0;
    }

    if (kJump) {
        if (!p->jumpPressed) {
            int canJump = (p->onGround || p->coyoteTimer < COYOTE_TIME);
            if (canJump) {
                p->vy            = JUMP_VY;
                p->onGround      = 0;
                p->jumping       = 1;
                p->hopping       = 0;
                p->jumpCut       = 0;
                p->coyoteTimer   = COYOTE_TIME;
                p->canDoubleJump = 1;
                p->jumpPressed   = 1;
                p->animState     = ANIM_JUMP;
                p->animFrame     = 0;
            } else if (p->canDoubleJump) {
                p->vy            = JUMP_VY * 2.0f;
                p->jumping       = 1;
                p->hopping       = 0;
                p->jumpCut       = 0;
                p->canDoubleJump = 0;
                p->isFlying      = 0;
                p->jumpPressed   = 1;
                p->animState     = ANIM_DOUBLE_JUMP;
                p->animFrame     = 0;
            } else {
                p->jumpBuffer  = JUMP_BUFFER;
                p->jumpPressed = 1;
            }
        }
    } else {
        if (p->jumpPressed && !p->jumpCut && p->vy < 0) {
            p->vy      *= JUMP_CUT_MULT;
            p->jumpCut  = 1;
        }
        p->jumpPressed = 0;
        p->isFlying    = 0;
    }

    if (kUp && p->onGround && !p->jumping && !p->hopping && !p->isCrouching) {
        p->hopping    = 1;
        p->onGround   = 0;
        p->jumping    = 0;
        p->jumpCut    = 0;
        p->vy         = HOP_VY;
        p->vitesse    = 0.0;
        p->animState  = ANIM_UP;
        p->animFrame  = 0;
    }

    if (kPunch && !p->isAttacking && !p->isKicking && !p->isShooting) {
        p->isAttacking = 1;
        p->actionTimer = ACTION_DUR;
        p->vitesse     = 0;
    }
    if (kKick && !p->isKicking && !p->isAttacking && !p->isShooting) {
        p->isKicking   = 1;
        p->actionTimer = ACTION_DUR;
        p->vitesse     = 0;
    }
    if (kShoot && !p->isShooting && !p->isAttacking && !p->isKicking) {
        int j;
        float bY = (float)(p->posScreen.y + p->posScreen.h * 38 / 100);
        float bX = p->direction
                   ? (float)(p->posScreen.x + p->posScreen.w)
                   : (float)(p->posScreen.x);
        for (j = 0; j < MAX_BULLETS; j++) {
            if (!p->bullets[j].active) {
                p->bullets[j].active = 1;
                p->bullets[j].owner  = p->isCat ? 1 : 0;
                p->bullets[j].x      = bX;
                p->bullets[j].y      = bY;
                p->bullets[j].vx     = p->direction ? 500.0f : -500.0f;
                break;
            }
        }
        p->isShooting  = 1;
        p->actionTimer = ACTION_DUR;
        p->vitesse     = 0;
    }
}

void checkBulletHits(Player *shooter, Player *target)
{
    int i;
    SDL_Rect tRect, bRect, inter;
    tRect.x = (int)target->x; tRect.y = (int)target->y;
    tRect.w = target->w;      tRect.h = target->h;

    for (i = 0; i < MAX_BULLETS; i++) {
        if (!shooter->bullets[i].active) continue;
        bRect.x = (int)shooter->bullets[i].x - 6;
        bRect.y = (int)shooter->bullets[i].y - 4;
        bRect.w = 12; bRect.h = 8;
        if (SDL_IntersectRect(&tRect, &bRect, &inter)) {
            shooter->bullets[i].active = 0;
            target->hp -= 10;
            if (target->hp < 0) target->hp = 0;
            shooter->score += 50;
        }
    }
}

void checkMeleeHits(Player *attacker, Player *target)
{
    SDL_Rect aBox, tBox, inter;
    aBox.x = attacker->direction
                ? (int)attacker->x + attacker->w
                : (int)attacker->x - 60;
    aBox.y = (int)attacker->y + 20;
    aBox.w = 60;
    aBox.h = attacker->h - 40;
    tBox.x = (int)target->x; tBox.y = (int)target->y;
    tBox.w = target->w;      tBox.h = target->h;

    if (!SDL_IntersectRect(&aBox, &tBox, &inter)) return;

    if (attacker->isAttacking && attacker->actionTimer == 10) {
        target->hp -= 15;
        if (target->hp < 0) target->hp = 0;
        attacker->score += 30;
    }
    if (attacker->isKicking && attacker->actionTimer == 10) {
        target->hp -= 20;
        if (target->hp < 0) target->hp = 0;
        attacker->score += 40;
    }
}

void renderBullets(SDL_Renderer *r, Player *p, SDL_Texture *batarang, Uint32 ticks)
{
    int i;
    SDL_Rect br;
    double spin = (ticks % 300) * (360.0 / 300.0);

    for (i = 0; i < MAX_BULLETS; i++) {
        if (!p->bullets[i].active) continue;

        if (batarang && !p->isCat) {
            double angle = (p->bullets[i].vx > 0) ? spin : -spin;
            br.w = 40; br.h = 40;
            br.x = (int)p->bullets[i].x - br.w / 2;
            br.y = (int)p->bullets[i].y - br.h / 2;
            SDL_RenderCopyEx(r, batarang, NULL, &br, angle, NULL, SDL_FLIP_NONE);
        } else {
            br.w = 12; br.h = 12;
            br.x = (int)p->bullets[i].x - 6;
            br.y = (int)p->bullets[i].y - 6;
            SDL_SetRenderDrawColor(r, 200, 50, 200, 255);
            SDL_RenderFillRect(r, &br);
        }
    }
}

void handleDeath(Player *p, float startX, float startY)
{
    if (p->hp > 0 || !p->isAlive) return;
    p->vies--;
    if (p->vies <= 0) {
        p->vies    = 0;
        p->isAlive = 0;
    } else {
        p->hp          = 100;
        p->x           = startX;
        p->y           = startY;
        p->vy          = 0;
        p->onGround    = 1;
        p->jumping     = 0;
        p->hopping     = 0;
        p->isAttacking = 0;
        p->isKicking   = 0;
        p->isShooting  = 0;
        p->actionTimer = 0;
    }
}

int gameOverScreen(SDL_Renderer *ren, TTF_Font *font, SDL_Texture *bg,
                   const char *winner, int scoreP1, int scoreP2)
{
    SDL_Color yellow = {255, 220,   0, 255};
    SDL_Color white  = {220, 220, 220, 255};
    SDL_Color green  = {  0, 220,  80, 255};
    SDL_Color red    = {255,  70,  70, 255};
    SDL_Surface *s;
    SDL_Texture *t;
    SDL_Rect d, full;
    char buf[80];
    SDL_Event e;

    full.x = 0; full.y = 0; full.w = SCREEN_W; full.h = SCREEN_H;

    while (1) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return 0;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.scancode == SDL_SCANCODE_RETURN ||
                    e.key.keysym.scancode == SDL_SCANCODE_SPACE)  return 1;
                if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) return 0;
            }
        }
        SDL_RenderClear(ren);
        if (bg) SDL_RenderCopy(ren, bg, NULL, NULL);
        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 170);
        SDL_RenderFillRect(ren, &full);
        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);

#define RENDER_TEXT(txt, col, yy) \
        s = TTF_RenderUTF8_Blended(font, txt, col); \
        if (s) { t = SDL_CreateTextureFromSurface(ren, s); \
                 d.x = SCREEN_W/2 - s->w/2; d.y = yy; d.w = s->w; d.h = s->h; \
                 SDL_RenderCopy(ren, t, NULL, &d); \
                 SDL_FreeSurface(s); SDL_DestroyTexture(t); }

        RENDER_TEXT("GAME OVER", red, 160)
        snprintf(buf, sizeof(buf), "%s GAGNE !", winner);
        RENDER_TEXT(buf, yellow, 230)
        snprintf(buf, sizeof(buf), "BATMAN   : %d pts", scoreP1);
        RENDER_TEXT(buf, white, 300)
        snprintf(buf, sizeof(buf), "CATWOMAN : %d pts", scoreP2);
        RENDER_TEXT(buf, white, 330)
        RENDER_TEXT("[ ESPACE ] - Rejouer", green, 410)
        RENDER_TEXT("[ ECHAP  ] - Quitter", red,   450)
#undef RENDER_TEXT

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }
}

int gameLoop(SDL_Renderer *ren, TTF_Font *font, SDL_Texture *bg,
             MenuResult *menu)
{
    Player p1, p2;
    SDL_Texture *batarang;
    Uint32 last, now, ticks;
    float dt;
    int running, nPressed, i;
    SDL_Event e;
    const Uint8 *ks;
    SDL_Scancode pressed[MAX_KEYS];
    const char *winner;
    InputConfig cfg1, cfg2;

    initPlayer(&p1, 150, GROUND_Y, menu->charP1 == CHAR_CATWOMAN);
    initPlayer(&p2, 650, GROUND_Y, menu->charP2 == CHAR_CATWOMAN);
    p1.costume   = menu->costumeP1;
    p2.costume   = menu->costumeP2;
    p2.direction = 0;

    loadPlayerTextures(ren, &p1);
    loadPlayerTextures(ren, &p2);

    cfg1 = menu->inputP1;
    cfg2 = menu->inputP2;
    cfg1.jump = cfg1.fly;
    cfg2.jump = cfg2.fly;

    batarang = IMG_LoadTexture(ren, "batarang.png");
    if (!batarang) batarang = IMG_LoadTexture(ren, "assets/batarang.png");
    if (batarang) SDL_SetTextureBlendMode(batarang, SDL_BLENDMODE_BLEND);

    last    = SDL_GetTicks();
    running = 1;

    while (running) {
        now  = SDL_GetTicks();
        dt   = (now - last) / 1000.0f;
        last = now;
        if (dt > 0.05f) dt = 0.05f;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN &&
                e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) running = 0;
        }

        ks       = SDL_GetKeyboardState(NULL);
        nPressed = 0;
        for (i = 0; i < SDL_NUM_SCANCODES && nPressed < MAX_KEYS; i++) {
            if (ks[i]) { pressed[nPressed] = (SDL_Scancode)i; nPressed++; }
        }

        handleInput(&p1, pressed, cfg1, nPressed);
        handleInput(&p2, pressed, cfg2, nPressed);

        updatePlayer(&p1, dt);
        updatePlayer(&p2, dt);

        checkBulletHits(&p1, &p2);
        checkBulletHits(&p2, &p1);
        checkMeleeHits(&p1, &p2);
        checkMeleeHits(&p2, &p1);

        handleDeath(&p1, 150, GROUND_Y);
        handleDeath(&p2, 650, GROUND_Y);

        if (!p1.isAlive || !p2.isAlive) running = 0;

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);
        if (bg) SDL_RenderCopy(ren, bg, NULL, NULL);

        blitPlayer(ren, &p1);
        ticks = SDL_GetTicks();
        renderBullets(ren, &p1, batarang, ticks);
        blitPlayer(ren, &p2);
        renderBullets(ren, &p2, batarang, ticks);

        if (font) drawHUD(ren, font, &p1, &p2);
        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    if (batarang) SDL_DestroyTexture(batarang);
    if      (!p1.isAlive && !p2.isAlive) winner = "MATCH NUL";
    else if (!p1.isAlive)                winner = (menu->charP2 == CHAR_CATWOMAN) ? "CATWOMAN" : "BATMAN P2";
    else if (!p2.isAlive)                winner = (menu->charP1 == CHAR_BATMAN)   ? "BATMAN"   : "CATWOMAN P1";
    else                                 winner = (p1.hp >= p2.hp) ? "JOUEUR 1" : "JOUEUR 2";

    return gameOverScreen(ren, font, bg, winner, p1.score, p2.score);
}

int main(void)
{
    SDL_Window   *win;
    SDL_Renderer *ren;
    TTF_Font     *font;
    SDL_Texture  *bg;
    MenuResult    menu;
    int           play;

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    win  = SDL_CreateWindow("Batman vs Catwoman",
               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
               SCREEN_W, SCREEN_H, 0);
    ren  = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    font = TTF_OpenFont("assets/font.ttf", 15);
    bg   = IMG_LoadTexture(ren, "assets/back.png");

    play = 1;
    while (play) {
        if (!runMenu(ren, font, &menu)) break;
        bg = IMG_LoadTexture(ren, "assets/back.png");
        play = gameLoop(ren, font, bg, &menu);
        if (bg) { SDL_DestroyTexture(bg); bg = NULL; }
    }

    if (bg)   SDL_DestroyTexture(bg);
    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit(); IMG_Quit(); SDL_Quit();
    return 0;
}
