#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>

#include "../include/player.h"
#include "../include/game.h"

#define SCREEN_W  900
#define SCREEN_H  620
#define GROUND_Y  460

/* ── Dimensions exactes des sprite sheets ──────────────
   Batman:   688 x 1568  => 4 cols x 9 rows  172 x 174
   Catwoman: 672 x 1550  => 4 cols x 9 rows  168 x 172
──────────────────────────────────────────────────────── */
#define BAT_FRAME_W  172
#define BAT_FRAME_H  174
#define CAT_FRAME_W  168
#define CAT_FRAME_H  172

/* ─────────────────────────────────────────────
   HUD helpers
───────────────────────────────────────────── */
static void hudText(SDL_Renderer *r, TTF_Font *f,
                    const char *txt, int x, int y, SDL_Color c)
{
    if (!f) return;
    SDL_Surface *s = TTF_RenderUTF8_Blended(f, txt, c);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_Rect d = { x, y, s->w, s->h };
    SDL_RenderCopy(r, t, NULL, &d);
    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}

static void hudTextCentered(SDL_Renderer *r, TTF_Font *f,
                             const char *txt, int cx, int y, SDL_Color c)
{
    if (!f) return;
    SDL_Surface *s = TTF_RenderUTF8_Blended(f, txt, c);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_Rect d = { cx - s->w / 2, y, s->w, s->h };
    SDL_RenderCopy(r, t, NULL, &d);
    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}

/* ─────────────────────────────────────────────
   drawHUD
───────────────────────────────────────────── */
static void drawHUD(SDL_Renderer *r, TTF_Font *font,
                    Player *p1, Player *p2)
{
    char buf[80];
    SDL_Color cyan    = {  0, 220, 220, 255};
    SDL_Color magenta = {220,  60, 220, 255};

    snprintf(buf, sizeof(buf), "BATMAN   Vies:%d  Score:%d", p1->vies, p1->score);
    hudText(r, font, buf, 10, 8, cyan);

    SDL_Rect bg1 = {10, 30, 200, 12};
    int hp1w = p1->hp * 2; if (hp1w < 0) hp1w = 0;
    SDL_Rect fg1 = {10, 30, hp1w, 12};
    SDL_SetRenderDrawColor(r,  40,  0,  0, 255); SDL_RenderFillRect(r, &bg1);
    SDL_SetRenderDrawColor(r, 220, 60, 60, 255); SDL_RenderFillRect(r, &fg1);
    SDL_SetRenderDrawColor(r, 180,180,180, 255); SDL_RenderDrawRect(r, &bg1);

    snprintf(buf, sizeof(buf), "CATWOMAN  Vies:%d  Score:%d", p2->vies, p2->score);
    SDL_Surface *s = TTF_RenderUTF8_Blended(font, buf, magenta);
    if (s) {
        SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
        SDL_Rect d = {SCREEN_W - s->w - 10, 8, s->w, s->h};
        SDL_RenderCopy(r, t, NULL, &d);
        SDL_FreeSurface(s); SDL_DestroyTexture(t);
    }
    SDL_Rect bg2 = {SCREEN_W - 210, 30, 200, 12};
    int hp2w = p2->hp * 2; if (hp2w < 0) hp2w = 0;
    SDL_Rect fg2 = {SCREEN_W - 210, 30, hp2w, 12};
    SDL_SetRenderDrawColor(r,  40,  0, 40, 255); SDL_RenderFillRect(r, &bg2);
    SDL_SetRenderDrawColor(r, 200, 60,220, 255); SDL_RenderFillRect(r, &fg2);
    SDL_SetRenderDrawColor(r, 180,180,180, 255); SDL_RenderDrawRect(r, &bg2);
}

/* ─────────────────────────────────────────────
   loadTex — charge le sprite sheet
   Batman:   frameW=172, frameH=174 (688x1568)
   Catwoman: frameW=168, frameH=172 (672x1550)
───────────────────────────────────────────── */
static void loadTex(SDL_Renderer *r, Player *p,
                    int isCat, int costume,
                    SDL_Texture **outR, SDL_Texture **outL)
{
    char pr[128], pl[128];
    const char *name = isCat ? "catwoman" : "batman";
    snprintf(pr, sizeof(pr), "assets/%s_right_%d.png", name, costume + 1);
    snprintf(pl, sizeof(pl), "assets/%s_left_%d.png",  name, costume + 1);
    *outR = IMG_LoadTexture(r, pr);
    *outL = IMG_LoadTexture(r, pl);
    /* Fallback sans numéro de costume */
    if (!*outR) {
        snprintf(pr, sizeof(pr), "assets/%s_right.png", name);
        *outR = IMG_LoadTexture(r, pr);
    }
    if (!*outL) {
        snprintf(pl, sizeof(pl), "assets/%s_left.png", name);
        *outL = IMG_LoadTexture(r, pl);
    }

    /* Dimensions selon le personnage
       initSpriteData s'occupe aussi du SDL_BLENDMODE_BLEND */
    int fw = isCat ? CAT_FRAME_W : BAT_FRAME_W;
    int fh = isCat ? CAT_FRAME_H : BAT_FRAME_H;
    initSpriteData(&p->sprite, *outR, *outL, fw, fh, isCat);
}

/* ─────────────────────────────────────────────
   gameOverScreen
───────────────────────────────────────────── */
static int gameOverScreen(SDL_Renderer *ren, TTF_Font *font,
                          SDL_Texture *bg, const char *winner,
                          int scoreP1, int scoreP2)
{
    SDL_Color yellow = {255, 220,   0, 255};
    SDL_Color white  = {220, 220, 220, 255};
    SDL_Color green  = {  0, 220,  80, 255};
    SDL_Color red    = {255,  70,  70, 255};
    char buf[80];

    while (1) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return 0;
            if (e.type == SDL_KEYDOWN) {
                SDL_Scancode sc = e.key.keysym.scancode;
                if (sc == SDL_SCANCODE_RETURN || sc == SDL_SCANCODE_SPACE) return 1;
                if (sc == SDL_SCANCODE_ESCAPE) return 0;
            }
        }

        SDL_RenderClear(ren);
        if (bg) SDL_RenderCopy(ren, bg, NULL, NULL);

        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 170);
        SDL_Rect full = {0, 0, SCREEN_W, SCREEN_H};
        SDL_RenderFillRect(ren, &full);
        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);

        hudTextCentered(ren, font, "GAME OVER",        SCREEN_W/2, 160, red);
        snprintf(buf, sizeof(buf), "%s GAGNE !", winner);
        hudTextCentered(ren, font, buf,                SCREEN_W/2, 230, yellow);
        snprintf(buf, sizeof(buf), "BATMAN   : %d pts", scoreP1);
        hudTextCentered(ren, font, buf,                SCREEN_W/2, 300, white);
        snprintf(buf, sizeof(buf), "CATWOMAN : %d pts", scoreP2);
        hudTextCentered(ren, font, buf,                SCREEN_W/2, 330, white);
        hudTextCentered(ren, font, "[ ENTREE ] - Rejouer", SCREEN_W/2, 410, green);
        hudTextCentered(ren, font, "[ ECHAP  ] - Quitter", SCREEN_W/2, 450, red);

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }
}

/* ─────────────────────────────────────────────
   checkBulletHits
───────────────────────────────────────────── */
static void checkBulletHits(Player *shooter, Player *target)
{
    SDL_Rect tRect = { (int)target->x, (int)target->y, target->w, target->h };
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!shooter->bullets[i].active) continue;
        SDL_Rect bRect = {
            (int)shooter->bullets[i].x - 6,
            (int)shooter->bullets[i].y - 4,
            12, 8
        };
        SDL_Rect inter;
        if (SDL_IntersectRect(&tRect, &bRect, &inter)) {
            shooter->bullets[i].active = 0;
            target->hp -= 10;
            if (target->hp < 0) target->hp = 0;
            shooter->score += 50;
        }
    }
}

/* ─────────────────────────────────────────────
   checkMeleeHits
───────────────────────────────────────────── */
static void checkMeleeHits(Player *attacker, Player *target)
{
    SDL_Rect aBox = {
        attacker->direction
            ? (int)attacker->x + attacker->w
            : (int)attacker->x - 60,
        (int)attacker->y + 20,
        60, attacker->h - 40
    };
    SDL_Rect tBox  = { (int)target->x, (int)target->y, target->w, target->h };
    SDL_Rect inter;
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

/* ─────────────────────────────────────────────
   handleDeath
───────────────────────────────────────────── */
static void handleDeath(Player *p, float startX, float startY)
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
        p->isAttacking = 0;
        p->isKicking   = 0;
        p->isShooting  = 0;
        p->actionTimer = 0;
    }
}

/* ─────────────────────────────────────────────
   gameLoop
───────────────────────────────────────────── */
static int gameLoop(SDL_Renderer *ren, TTF_Font *font, SDL_Texture *bg)
{
    Player p1, p2;
    initPlayer(&p1, 150, GROUND_Y, 0);   /* Batman */
    initPlayer(&p2, 650, GROUND_Y, 1);   /* Catwoman */
    p2.direction = 0;

    SDL_Texture *t1R = NULL, *t1L = NULL;
    SDL_Texture *t2R = NULL, *t2L = NULL;
    loadTex(ren, &p1, 0, 0, &t1R, &t1L);
    loadTex(ren, &p2, 1, 0, &t2R, &t2L);

    InputConfig cfg1 = DEFAULT_INPUT_P1;
    InputConfig cfg2 = DEFAULT_INPUT_P2;

    Uint32 last    = SDL_GetTicks();
    int    running = 1;

    while (running) {
        Uint32 now = SDL_GetTicks();
        float dt   = (now - last) / 1000.0f;
        last = now;
        if (dt > 0.05f) dt = 0.05f;

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN &&
                e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) running = 0;
        }

        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        handleInput(&p1, keys, cfg1);
        handleInput(&p2, keys, cfg2);

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

        renderPlayer(ren, &p1);
        renderBullets(ren, &p1);
        renderPlayer(ren, &p2);
        renderBullets(ren, &p2);

        if (font) drawHUD(ren, font, &p1, &p2);

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    const char *winner;
    if      (!p1.isAlive && !p2.isAlive) winner = "MATCH NUL";
    else if (!p1.isAlive)                winner = "CATWOMAN";
    else if (!p2.isAlive)                winner = "BATMAN";
    else winner = (p1.hp >= p2.hp) ? "BATMAN" : "CATWOMAN";

    if (t1R) SDL_DestroyTexture(t1R);
    if (t1L) SDL_DestroyTexture(t1L);
    if (t2R) SDL_DestroyTexture(t2R);
    if (t2L) SDL_DestroyTexture(t2L);

    return gameOverScreen(ren, font, bg, winner, p1.score, p2.score);
}

/* ─────────────────────────────────────────────
   main
───────────────────────────────────────────── */
int main(void)
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    SDL_Window   *win = SDL_CreateWindow("Batman vs Catwoman",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            SCREEN_W, SCREEN_H, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font    *font = TTF_OpenFont("assets/font.ttf", 15);
    SDL_Texture *bg   = IMG_LoadTexture(ren, "assets/back.png");

    int play = 1;
    while (play) {
        play = gameLoop(ren, font, bg);
    }

    if (bg)   SDL_DestroyTexture(bg);
    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit(); IMG_Quit(); SDL_Quit();
    return 0;
}
