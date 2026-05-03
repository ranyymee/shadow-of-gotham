#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include "../include/back.h"
#include "../include/player.h"
#include "../include/game.h"
#define SCREEN_W  1000
#define SCREEN_H  427
#define GROUND_Y  267
#define BAT_FRAME_W  172
#define BAT_FRAME_H  174
#define CAT_FRAME_W  168
#define CAT_FRAME_H  172
static void hudText(SDL_Renderer *r, TTF_Font *f,
                    const char *txt, int x, int y, SDL_Color c)
{
    SDL_Surface *s;
    SDL_Texture *t;
    SDL_Rect d;
    if (!f) return;
    s = TTF_RenderUTF8_Blended(f, txt, c);
    if (!s) return;
    t = SDL_CreateTextureFromSurface(r, s);
    d.x = x; d.y = y; d.w = s->w; d.h = s->h;
    SDL_RenderCopy(r, t, NULL, &d);
    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}
static void hudTextCentered(SDL_Renderer *r, TTF_Font *f,
                             const char *txt, int cx, int y, SDL_Color c)
{
    SDL_Surface *s;
    SDL_Texture *t;
    SDL_Rect d;
    if (!f) return;
    s = TTF_RenderUTF8_Blended(f, txt, c);
    if (!s) return;
    t = SDL_CreateTextureFromSurface(r, s);
    d.x = cx - s->w / 2; d.y = y; d.w = s->w; d.h = s->h;
    SDL_RenderCopy(r, t, NULL, &d);
    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}
static void drawHUD(SDL_Renderer *r, TTF_Font *font,
                    Player *p1, Player *p2)
{
    char buf[80];
    SDL_Color cyan    = {  0, 220, 220, 255};
    SDL_Color magenta = {220,  60, 220, 255};
    int hp1w, hp2w;
    SDL_Surface *s;
    SDL_Texture *t;
    SDL_Rect bg1, fg1, bg2, fg2, d;
    snprintf(buf, sizeof(buf), "BATMAN  Vies:%d  Score:%d", p1->vies, p1->score);
    hudText(r, font, buf, 10, 5, cyan);
    bg1.x = 10; bg1.y = 26; bg1.w = 180; bg1.h = 10;
    hp1w = p1->hp * 180 / 100; if (hp1w < 0) hp1w = 0;
    fg1.x = 10; fg1.y = 26; fg1.w = hp1w; fg1.h = 10;
    SDL_SetRenderDrawColor(r,  40,  0,  0, 255); SDL_RenderFillRect(r, &bg1);
    SDL_SetRenderDrawColor(r, 220, 60, 60, 255); SDL_RenderFillRect(r, &fg1);
    SDL_SetRenderDrawColor(r, 180,180,180, 255); SDL_RenderDrawRect(r, &bg1);
    snprintf(buf, sizeof(buf), "CATWOMAN  Vies:%d  Score:%d", p2->vies, p2->score);
    s = TTF_RenderUTF8_Blended(font, buf, magenta);
    if (s) {
        t = SDL_CreateTextureFromSurface(r, s);
        d.x = SCREEN_W - s->w - 10; d.y = 5; d.w = s->w; d.h = s->h;
        SDL_RenderCopy(r, t, NULL, &d);
        SDL_FreeSurface(s); SDL_DestroyTexture(t);
    }
    bg2.x = SCREEN_W - 190; bg2.y = 26; bg2.w = 180; bg2.h = 10;
    hp2w = p2->hp * 180 / 100; if (hp2w < 0) hp2w = 0;
    fg2.x = SCREEN_W - 190; fg2.y = 26; fg2.w = hp2w; fg2.h = 10;
    SDL_SetRenderDrawColor(r,  40,  0, 40, 255); SDL_RenderFillRect(r, &bg2);
    SDL_SetRenderDrawColor(r, 200, 60,220, 255); SDL_RenderFillRect(r, &fg2);
    SDL_SetRenderDrawColor(r, 180,180,180, 255); SDL_RenderDrawRect(r, &bg2);
}
static void loadPlayerTex(SDL_Renderer *r, Player *p,
                           int isCat, int costume,
                           SDL_Texture **outR, SDL_Texture **outL)
{
    char pr[128], pl[128];
    const char *name = isCat ? "catwoman" : "batman";
    int fw = isCat ? CAT_FRAME_W : BAT_FRAME_W;
    int fh = isCat ? CAT_FRAME_H : BAT_FRAME_H;
    snprintf(pr, sizeof(pr), "assets/%s_right_%d.png", name, costume + 1);
    snprintf(pl, sizeof(pl), "assets/%s_left_%d.png",  name, costume + 1);
    *outR = IMG_LoadTexture(r, pr);
    *outL = IMG_LoadTexture(r, pl);
    if (!*outR) {
        snprintf(pr, sizeof(pr), "assets/%s_right.png", name);
        *outR = IMG_LoadTexture(r, pr);
    }
    if (!*outL) {
        snprintf(pl, sizeof(pl), "assets/%s_left.png", name);
        *outL = IMG_LoadTexture(r, pl);
    }
    /* NULL = use built-in defaults for frameCounts, fps, loop */
    initSpriteData(&p->sprite, *outR, *outL, fw, fh, isCat,
                   NULL, NULL, NULL);
}
static void checkBulletHits(Player *shooter, Player *target)
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
static void checkMeleeHits(Player *attacker, Player *target)
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
static int gameOverScreen(SDL_Renderer *ren, TTF_Font *font,
                           SDL_Texture *bg, const char *winner,
                           int scoreP1, int scoreP2)
{
    SDL_Color yellow = {255, 220,   0, 255};
    SDL_Color white  = {220, 220, 220, 255};
    SDL_Color green  = {  0, 220,  80, 255};
    SDL_Color red    = {255,  70,  70, 255};
    char buf[80];
    SDL_Event e;
    SDL_Rect full;
    while (1) {
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
        full.x = 0; full.y = 0; full.w = SCREEN_W; full.h = SCREEN_H;
        SDL_RenderFillRect(ren, &full);
        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);
        hudTextCentered(ren, font, "GAME OVER",            SCREEN_W/2, 160, red);
        snprintf(buf, sizeof(buf), "%s GAGNE !", winner);
        hudTextCentered(ren, font, buf,                    SCREEN_W/2, 230, yellow);
        snprintf(buf, sizeof(buf), "BATMAN   : %d pts", scoreP1);
        hudTextCentered(ren, font, buf,                    SCREEN_W/2, 300, white);
        snprintf(buf, sizeof(buf), "CATWOMAN : %d pts", scoreP2);
        hudTextCentered(ren, font, buf,                    SCREEN_W/2, 330, white);
        hudTextCentered(ren, font, "[ ENTREE ] - Rejouer", SCREEN_W/2, 410, green);
        hudTextCentered(ren, font, "[ ECHAP  ] - Quitter", SCREEN_W/2, 450, red);
        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }
}
static int gameLoop(SDL_Renderer *ren, TTF_Font *font,
                    Background *bg, Platform platforms[], int *taille,
                    int affMode)
{
    Player p1, p2;
    SDL_Texture *t1R = NULL, *t1L = NULL;
    SDL_Texture *t2R = NULL, *t2L = NULL;
    InputConfig cfg1, cfg2;
    SDL_Color textColor;
    SDL_Rect vp1, vp2;
    Uint32 last;
    int running, timeLeft;
    Uint32 lastTime;
    int lives;
    int bgX1, bgY1, bgX2, bgY2;
    SDL_Event e;
    const char *winner;
    float dt;
    Uint32 now;
    initPlayer(&p1, 150, GROUND_Y, 0);   
    initPlayer(&p2, 750, GROUND_Y, 1);   
    p2.direction = 0;
    loadPlayerTex(ren, &p1, 0, 0, &t1R, &t1L);
    loadPlayerTex(ren, &p2, 1, 0, &t2R, &t2L);
    cfg1 = DEFAULT_INPUT_P1;
    cfg2 = DEFAULT_INPUT_P2;
    textColor.r = 255; textColor.g = 255;
    textColor.b = 255; textColor.a = 255;
    timeLeft  = 600;
    lastTime  = SDL_GetTicks();
    lives     = 3;
    bgX1 = 0; bgY1 = 0;
    bgX2 = 0; bgY2 = 0;
    last    = SDL_GetTicks();
    running = 1;
    while (running) {
        now = SDL_GetTicks();
        dt  = (now - last) / 1000.0f;
        last = now;
        if (dt > 0.05f) dt = 0.05f;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN &&
                e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) running = 0;
            gererGuideEtClic(e, &bg->guide, &bg->commentJouer,
                             &bg->afficherCommentJouer);
            gererScrollingDeuxJoueurs(e, &bgX1, &bgY1, &bgX2, &bgY2, 20);
        }
        gererTemps(&timeLeft, &lastTime);
        if (timeLeft <= 0) running = 0;
        updatePlatforms(platforms, *taille);
        {
            SDL_Rect platRects[MAX_PLATFORMS];
            int nb = 0, ii;
            for (ii = 0; ii < *taille; ii++) {
                if (!platforms[ii].destroyed)
                    platRects[nb++] = platforms[ii].position;
            }
            {
                const Uint8 *keys = SDL_GetKeyboardState(NULL);
                handleInput(&p1, keys, cfg1);
                handleInput(&p2, keys, cfg2);
            }
            updatePlayer(&p1, dt, platRects, nb);
            updatePlayer(&p2, dt, platRects, nb);
        }
        checkBulletHits(&p1, &p2);
        checkBulletHits(&p2, &p1);
        checkMeleeHits(&p1, &p2);
        checkMeleeHits(&p2, &p1);
        handleDeath(&p1, 150, GROUND_Y);
        handleDeath(&p2, 650, GROUND_Y);
        if (!p1.isAlive || !p2.isAlive) running = 0;
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);
        if (affMode == MODE_MULTI) {
            vp1.x = 0;          vp1.y = 0;
            vp1.w = SCREEN_W/2; vp1.h = SCREEN_H;
            SDL_RenderSetViewport(ren, &vp1);
            afficherBackgroundEtElements(ren, bg, platforms, *taille, font,
                                         textColor, timeLeft, lives,
                                         bgX1, bgY1, SCREEN_W/2, SCREEN_H,
                                         MODE_MULTI);
            renderPlayer(ren, &p1);
            renderBullets(ren, &p1);
            drawHUD(ren, font, &p1, &p2);
            vp2.x = SCREEN_W/2; vp2.y = 0;
            vp2.w = SCREEN_W/2; vp2.h = SCREEN_H;
            SDL_RenderSetViewport(ren, &vp2);
            afficherBackgroundEtElements(ren, bg, platforms, *taille, font,
                                         textColor, timeLeft, lives,
                                         bgX2, bgY2, SCREEN_W/2, SCREEN_H,
                                         MODE_MULTI);
            renderPlayer(ren, &p2);
            renderBullets(ren, &p2);
            SDL_RenderSetViewport(ren, NULL);
            SDL_SetRenderDrawColor(ren, 200, 200, 200, 255);
            SDL_RenderDrawLine(ren, SCREEN_W/2, 0, SCREEN_W/2, SCREEN_H);
        } else {
            SDL_RenderSetViewport(ren, NULL);
            afficherBackgroundEtElements(ren, bg, platforms, *taille, font,
                                         textColor, timeLeft, lives,
                                         bgX1, bgY1, SCREEN_W, SCREEN_H,
                                         MODE_MONO);
            renderPlayer(ren, &p1);
            renderBullets(ren, &p1);
            renderPlayer(ren, &p2);
            renderBullets(ren, &p2);
            if (font) drawHUD(ren, font, &p1, &p2);
        }
        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }
    SDL_RenderSetViewport(ren, NULL);
    if      (!p1.isAlive && !p2.isAlive) winner = "MATCH NUL";
    else if (!p1.isAlive)                winner = "CATWOMAN";
    else if (!p2.isAlive)                winner = "BATMAN";
    else winner = (p1.hp >= p2.hp) ? "BATMAN" : "CATWOMAN";
    if (t1R) SDL_DestroyTexture(t1R);
    if (t1L) SDL_DestroyTexture(t1L);
    if (t2R) SDL_DestroyTexture(t2R);
    if (t2L) SDL_DestroyTexture(t2L);
    return gameOverScreen(ren, font, bg->img[0], winner,
                          p1.score, p2.score);
}
int main(int argc, char *argv[])
{
    int          screenW = SCREEN_W;
    int          screenH = SCREEN_H;
    int          level   = 1;
    int          taille  = 0;
    int          affMode = MODE_MONO;
    int          play    = 1;
    int          bgW, bgH;
    SDL_Window   *window;
    SDL_Renderer *renderer;
    TTF_Font     *font;
    Background    bg;
    Platform      platforms[MAX_PLATFORMS];
    SDL_Event     event;
    int           i, f;
    Platform     *p;
    (void)argc; (void)argv;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("Erreur SDL_Init : %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() == -1) {
        printf("Erreur TTF_Init : %s\n", TTF_GetError());
        SDL_Quit(); return 1;
    }
    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_WEBP) == 0) {
        printf("Erreur IMG_Init : %s\n", IMG_GetError());
        TTF_Quit(); SDL_Quit(); return 1;
    }
    window = SDL_CreateWindow("Batman vs Catwoman",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              screenW, screenH,
                              SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Erreur fenetre : %s\n", SDL_GetError());
        SDL_Quit(); return 1;
    }
    renderer = SDL_CreateRenderer(window, -1,
                                   SDL_RENDERER_ACCELERATED |
                                   SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Erreur renderer : %s\n", SDL_GetError());
        SDL_DestroyWindow(window); SDL_Quit(); return 1;
    }
    initBackgroundAndPlatforms(renderer, &bg, platforms, &taille, level);
    (void)bgW; (void)bgH;
    font = TTF_OpenFont("assets/arial.ttf", 14);
    if (!font) font = TTF_OpenFont("assets/font.ttf", 14);
    if (!font) printf("Avertissement : police non chargee\n");
    while (play) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { play = 0; break; }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) { play = 0; break; }
                if (event.key.keysym.sym == SDLK_p)
                    affMode = (affMode == MODE_MONO) ? MODE_MULTI : MODE_MONO;
                if (event.key.keysym.sym == SDLK_F1 && level != 1) {
                    level = 1; taille = 0;
                    initBackgroundAndPlatforms(renderer, &bg, platforms,
                                               &taille, level);
                }
                if (event.key.keysym.sym == SDLK_F2 && level != 2) {
                    level = 2; taille = 0;
                    initBackgroundAndPlatforms(renderer, &bg, platforms,
                                               &taille, level);
                }
            }
        }
        if (!play) break;
        play = gameLoop(renderer, font, &bg, platforms, &taille, affMode);
    }
    if (font)
        saisirNomEtAfficherScore(renderer, font, 0, screenW, screenH);
    if (bg.img[0])           SDL_DestroyTexture(bg.img[0]);
    if (bg.guide.image)      SDL_DestroyTexture(bg.guide.image);
    if (bg.commentJouer.image) SDL_DestroyTexture(bg.commentJouer.image);
    for (i = 0; i < taille; i++) {
        p = &platforms[i];
        if (p->isAnimated) {
            for (f = 0; f < MAX_FRAMES; f++)
                if (p->frames[f]) SDL_DestroyTexture(p->frames[f]);
        } else {
            if (p->image) SDL_DestroyTexture(p->image);
        }
    }
    if (font) TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
