/*
 * main.c — Intégration lot back + lot joueur
 *
 * Approche : fighting game classique
 *   - Le background scrolle avec gererScrollingDeuxJoueurs() (exactement comme back.c original)
 *   - Les joueurs vivent en coordonnées ÉCRAN (p->x, p->y = pixels sur l'écran)
 *   - afficherBackgroundEtElements() dessine background + HUD back (timer, vies, guide)
 *   - Les joueurs sont dessinés PAR-DESSUS avec blitPlayer() standard
 *   - drawHUD() dessine HP bars + score des joueurs
 *
 * Compilation :
 *   gcc -Wall -std=c99 -o game main.c player.c back.c -lSDL2 -lSDL2_image -lSDL2_ttf -lm
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>

#include "player.h"
#include "back.h"

#define MAX_KEYS   16
#define ACTION_DUR 20

/* Sol en coordonnées écran — même valeur que GROUND_Y dans player.h (460) */
#define GROUND_SCREEN_Y  GROUND_Y

/* ============================================================
 * Chargement textures joueur
 * ============================================================ */
static void loadPlayerTextures(SDL_Renderer *ren, Player *p)
{
    char pr[128], pl[128];
    SDL_Texture *outR, *outL;
    int fw, fh;
    const char *name = p->isCat ? "catwoman" : "batman";

    snprintf(pr, sizeof(pr), "assets/%s_right_%d.png", name, p->costume + 1);
    snprintf(pl, sizeof(pl), "assets/%s_left_%d.png",  name, p->costume + 1);
    outR = IMG_LoadTexture(ren, pr);
    outL = IMG_LoadTexture(ren, pl);

    if (!outR) { snprintf(pr, sizeof(pr), "assets/%s_right.png", name); outR = IMG_LoadTexture(ren, pr); }
    if (!outL) { snprintf(pl, sizeof(pl), "assets/%s_left.png",  name); outL = IMG_LoadTexture(ren, pl); }

    fw = p->isCat ? CAT_FRAME_W : BAT_FRAME_W;
    fh = p->isCat ? CAT_FRAME_H : BAT_FRAME_H;
    initSpriteData(&p->sprite, outR, outL, fw, fh);
}

/* ============================================================
 * Input joueur
 * ============================================================ */
static void handleInput(Player *p, SDL_Scancode *kp, InputConfig cfg, int n)
{
    int kL=0,kR=0,kU=0,kFly=0,kPu=0,kKi=0,kSh=0,kCr=0,i,moving;

    if (!p->isAlive) return;
    for (i = 0; i < n; i++) {
        if (kp[i]==cfg.left)   kL=1;
        if (kp[i]==cfg.right)  kR=1;
        if (kp[i]==cfg.up)     kU=1;
        if (kp[i]==cfg.fly)    kFly=1;
        if (kp[i]==cfg.punch)  kPu=1;
        if (kp[i]==cfg.kick)   kKi=1;
        if (kp[i]==cfg.shoot)  kSh=1;
        if (kp[i]==cfg.crouch) kCr=1;
    }

    if (p->actionTimer > 0) { p->vitesse = 0; return; }

    p->vitesse=0; p->isCrouching=0; p->isRunning=0;

    if (p->onGround && kCr) p->isCrouching=1;

    if (!p->isCrouching) {
        moving=0;
        if (kL) { p->direction=0; moving=1; }
        if (kR) { p->direction=1; moving=1; }
        if (moving) {
            double spd = kCr ? 380.0 : 220.0;
            p->vitesse = p->direction ? spd : -spd;
            if (kCr) p->isRunning=1;
        }
    }

    if (kU) {
        if (!p->jumpPressed) {
            if (p->onGround) {
                p->up=1; p->posinit_y=p->y; p->saut_x_rel=-50.0;
                p->onGround=0; p->canDoubleJump=p->isCat; p->jumpPressed=1;
            } else if (p->isCat && p->canDoubleJump) {
                p->up=1; p->posinit_y=p->y; p->saut_x_rel=-50.0;
                p->canDoubleJump=0; p->jumpPressed=1;
            }
        }
    } else { p->jumpPressed=0; }

    if (!p->isCat &&  kFly) { p->isFlying=1; p->vy=-200.0f; p->onGround=0; }
    if (!p->isCat && !kFly) { p->isFlying=0; }

    if (kPu && !p->isAttacking && !p->isKicking && !p->isShooting)
        { p->isAttacking=1; p->actionTimer=ACTION_DUR; p->vitesse=0; }
    if (kKi && !p->isKicking && !p->isAttacking && !p->isShooting)
        { p->isKicking=1; p->actionTimer=ACTION_DUR; p->vitesse=0; }
    if (kSh && !p->isShooting && !p->isAttacking && !p->isKicking) {
        int j;
        for (j=0;j<MAX_BULLETS;j++) {
            if (!p->bullets[j].active) {
                p->bullets[j].active=1;
                p->bullets[j].owner =p->isCat?1:0;
                p->bullets[j].x     =(float)(p->x+(p->direction?p->w:0));
                p->bullets[j].y     =(float)(p->y+p->h/2);
                p->bullets[j].vx    =p->direction?500.0f:-500.0f;
                break;
            }
        }
        p->isShooting=1; p->actionTimer=ACTION_DUR; p->vitesse=0;
    }
}

/* ============================================================
 * Combat
 * ============================================================ */
static void checkBulletHits(Player *sh, Player *tg)
{
    int i; SDL_Rect tR,bR,inter;
    tR.x=(int)tg->x; tR.y=(int)tg->y; tR.w=tg->w; tR.h=tg->h;
    for (i=0;i<MAX_BULLETS;i++) {
        if (!sh->bullets[i].active) continue;
        bR.x=(int)sh->bullets[i].x-6; bR.y=(int)sh->bullets[i].y-4; bR.w=12; bR.h=8;
        if (SDL_IntersectRect(&tR,&bR,&inter)) {
            sh->bullets[i].active=0;
            tg->hp-=10; if(tg->hp<0)tg->hp=0;
            sh->score+=50;
        }
    }
}

static void checkMeleeHits(Player *at, Player *tg)
{
    SDL_Rect aB,tB,inter;
    aB.x=at->direction?(int)at->x+at->w:(int)at->x-60;
    aB.y=(int)at->y+20; aB.w=60; aB.h=at->h-40;
    tB.x=(int)tg->x; tB.y=(int)tg->y; tB.w=tg->w; tB.h=tg->h;
    if (!SDL_IntersectRect(&aB,&tB,&inter)) return;
    if (at->isAttacking && at->actionTimer==10) { tg->hp-=15; if(tg->hp<0)tg->hp=0; at->score+=30; }
    if (at->isKicking   && at->actionTimer==10) { tg->hp-=20; if(tg->hp<0)tg->hp=0; at->score+=40; }
}

/* ============================================================
 * Rendu balles (coordonnées écran)
 * ============================================================ */
static void renderBullets(SDL_Renderer *r, Player *p)
{
    int i; SDL_Rect br;
    for (i=0;i<MAX_BULLETS;i++) {
        if (!p->bullets[i].active) continue;
        br.x=(int)p->bullets[i].x-6; br.y=(int)p->bullets[i].y-4; br.w=12; br.h=8;
        if (p->isCat) SDL_SetRenderDrawColor(r,200,50,200,255);
        else          SDL_SetRenderDrawColor(r,255,220,0,255);
        SDL_RenderFillRect(r,&br);
    }
}

/* ============================================================
 * Mort / respawn
 * ============================================================ */
static void handleDeath(Player *p, float sx, float sy)
{
    if (p->hp>0||!p->isAlive) return;
    p->vies--;
    if (p->vies<=0) { p->vies=0; p->isAlive=0; }
    else {
        p->hp=100; p->x=sx; p->y=sy;
        p->vy=0; p->onGround=1;
        p->isAttacking=0; p->isKicking=0; p->isShooting=0; p->actionTimer=0;
    }
}

/* ============================================================
 * Clamp joueur horizontal dans l'écran
 * ============================================================ */
static void clampPlayerScreen(Player *p, int screenW)
{
    if (p->x < 0)            { p->x=0;            p->vitesse=0; }
    if (p->x > screenW-p->w) { p->x=screenW-p->w; p->vitesse=0; }
}

/* ============================================================
 * Écran Game Over
 * ============================================================ */
static int gameOverScreen(SDL_Renderer *ren, TTF_Font *font,
                          SDL_Texture *bgTex, const char *winner,
                          int sc1, int sc2, int W, int H)
{
    SDL_Color yellow={255,220,0,255}, white={220,220,220,255};
    SDL_Color green={0,220,80,255},   red={255,70,70,255};
    SDL_Surface *s; SDL_Texture *t; SDL_Rect d,full; char buf[80]; SDL_Event e;
    full.x=0; full.y=0; full.w=W; full.h=H;

#define RT(txt,col,yy) \
    s=TTF_RenderUTF8_Blended(font,txt,col); \
    if(s){t=SDL_CreateTextureFromSurface(ren,s); \
          d.x=W/2-s->w/2;d.y=yy;d.w=s->w;d.h=s->h; \
          SDL_RenderCopy(ren,t,NULL,&d);SDL_FreeSurface(s);SDL_DestroyTexture(t);}

    while(1) {
        while(SDL_PollEvent(&e)) {
            if(e.type==SDL_QUIT) return 0;
            if(e.type==SDL_KEYDOWN) {
                if(e.key.keysym.scancode==SDL_SCANCODE_RETURN||
                   e.key.keysym.scancode==SDL_SCANCODE_SPACE) return 1;
                if(e.key.keysym.scancode==SDL_SCANCODE_ESCAPE) return 0;
            }
        }
        SDL_RenderClear(ren);
        if(bgTex) SDL_RenderCopy(ren,bgTex,NULL,NULL);
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren,0,0,0,170); SDL_RenderFillRect(ren,&full);
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);

        RT("GAME OVER",red,H/2-160)
        snprintf(buf,sizeof(buf),"%s GAGNE !",winner); RT(buf,yellow,H/2-90)
        snprintf(buf,sizeof(buf),"BATMAN   : %d pts",sc1); RT(buf,white,H/2-20)
        snprintf(buf,sizeof(buf),"CATWOMAN : %d pts",sc2); RT(buf,white,H/2+10)
        RT("[ ENTREE ] - Rejouer",green,H/2+80)
        RT("[ ECHAP  ] - Quitter",red,H/2+115)

        SDL_RenderPresent(ren); SDL_Delay(16);
    }
#undef RT
}

/* ============================================================
 * Boucle de jeu principale
 * ============================================================ */
static int gameLoop(SDL_Renderer *ren, TTF_Font *font,
                    Background *bg, Platform platforms[], int taille,
                    int screenW, int screenH, int affMode,
                    int *timeLeft, Uint32 *lastTime)
{
    Player p1, p2;
    InputConfig cfg1, cfg2;
    Uint32 last, now;
    float dt;
    int running, nPressed, i;
    SDL_Event e;
    const Uint8 *ks;
    SDL_Scancode pressed[MAX_KEYS];
    const char *winner;
    SDL_Color tc = {255,255,255,255};

    /*
     * Positions de départ en coordonnées ÉCRAN
     * Sol = GROUND_SCREEN_Y (460px) — identique à GROUND_Y_VAL dans player.c
     */
    float sx1 = (float)(screenW / 4);
    float sx2 = (float)(screenW * 3 / 4);
    float sy  = (float)GROUND_SCREEN_Y;

    initPlayer(&p1, sx1, sy, 0);   /* Batman  */
    initPlayer(&p2, sx2, sy, 1);   /* Catwoman */
    p2.direction = 0;

    loadPlayerTextures(ren, &p1);
    loadPlayerTextures(ren, &p2);

    /* Contrôles P1 : flèches + pavé num */
    cfg1.left  =SDL_SCANCODE_LEFT;  cfg1.right =SDL_SCANCODE_RIGHT;
    cfg1.up    =SDL_SCANCODE_UP;    cfg1.fly   =SDL_SCANCODE_SPACE;
    cfg1.punch =SDL_SCANCODE_KP_1;  cfg1.kick  =SDL_SCANCODE_KP_2;
    cfg1.shoot =SDL_SCANCODE_KP_3;  cfg1.crouch=SDL_SCANCODE_DOWN;

    /* Contrôles P2 : WASD */
    cfg2.left  =SDL_SCANCODE_A;    cfg2.right =SDL_SCANCODE_D;
    cfg2.up    =SDL_SCANCODE_W;    cfg2.fly   =SDL_SCANCODE_W;
    cfg2.punch =SDL_SCANCODE_F;    cfg2.kick  =SDL_SCANCODE_E;
    cfg2.shoot =SDL_SCANCODE_H;    cfg2.crouch=SDL_SCANCODE_S;

    last=SDL_GetTicks(); running=1;

    while (running) {
        now=SDL_GetTicks(); dt=(now-last)/1000.0f; last=now;
        if (dt>0.05f) dt=0.05f;

        /* --- Événements --- */
        while (SDL_PollEvent(&e)) {
            if (e.type==SDL_QUIT) { running=0; break; }
            if (e.type==SDL_KEYDOWN &&
                e.key.keysym.scancode==SDL_SCANCODE_ESCAPE) { running=0; break; }
            gererGuideEtClic(e, &bg->guide, &bg->commentJouer,
                             &bg->afficherCommentJouer);
        }

        /* --- Scrolling background (identique à back.c original) --- */
        gererScrollingDeuxJoueurs(e, bg, bg, 20, 1);

        /* --- Temps --- */
        gererTemps(timeLeft, lastTime);
        if (*timeLeft<=0) { *timeLeft=0; running=0; }

        /* --- Input joueurs --- */
        ks=SDL_GetKeyboardState(NULL); nPressed=0;
        for (i=0;i<SDL_NUM_SCANCODES&&nPressed<MAX_KEYS;i++)
            if (ks[i]) pressed[nPressed++]=(SDL_Scancode)i;

        handleInput(&p1, pressed, cfg1, nPressed);
        handleInput(&p2, pressed, cfg2, nPressed);

        /* --- Physique joueurs (screen-space, sol fixe à GROUND_SCREEN_Y) --- */
        updatePlayer(&p1, dt);
        updatePlayer(&p2, dt);

        /* Clamp horizontal dans l'écran */
        clampPlayerScreen(&p1, screenW);
        clampPlayerScreen(&p2, screenW);

        /* --- Plateformes --- */
        updatePlatforms(platforms, taille);

        /* --- Combat --- */
        checkBulletHits(&p1,&p2); checkBulletHits(&p2,&p1);
        checkMeleeHits(&p1,&p2);  checkMeleeHits(&p2,&p1);

        /* --- Mort --- */
        handleDeath(&p1, sx1, sy);
        handleDeath(&p2, sx2, sy);
        if (!p1.isAlive||!p2.isAlive) running=0;

        /* ======================================================
         * RENDU
         * Ordre obligatoire :
         *   1. afficherBackgroundEtElements  (background + plateformes + HUD back)
         *   2. blitPlayer  (joueurs par-dessus le background)
         *   3. renderBullets
         *   4. drawHUD  (HP bars, score, vies joueurs)
         * ====================================================== */
        SDL_SetRenderDrawColor(ren, 0,0,0,255);
        SDL_RenderClear(ren);

        if (affMode==MODE_MULTI) {
            int vpW=screenW/2;

            /* Viewport gauche — P1 */
            SDL_Rect vp1={0,0,vpW,screenH};
            SDL_RenderSetViewport(ren,&vp1);
            afficherBackgroundEtElements(ren,bg,platforms,taille,
                                         font,tc,*timeLeft,p1.vies,
                                         bg->camera_pos.x,bg->camera_pos.y,
                                         vpW,screenH,MODE_MULTI);
            blitPlayer(ren,&p1); renderBullets(ren,&p1);
            blitPlayer(ren,&p2); renderBullets(ren,&p2);
            if (font) drawHUD(ren,font,&p1,&p2);

            /* Viewport droit — P2 */
            SDL_Rect vp2={vpW,0,vpW,screenH};
            SDL_RenderSetViewport(ren,&vp2);
            afficherBackgroundEtElements(ren,bg,platforms,taille,
                                         font,tc,*timeLeft,p2.vies,
                                         bg->camera_pos.x,bg->camera_pos.y,
                                         vpW,screenH,MODE_MULTI);
            blitPlayer(ren,&p1); renderBullets(ren,&p1);
            blitPlayer(ren,&p2); renderBullets(ren,&p2);
            if (font) drawHUD(ren,font,&p1,&p2);

            /* Ligne de séparation centrale */
            SDL_RenderSetViewport(ren,NULL);
            SDL_SetRenderDrawColor(ren,200,200,200,255);
            SDL_RenderDrawLine(ren,vpW,0,vpW,screenH);

        } else {
            SDL_RenderSetViewport(ren,NULL);

            /* 1) Background + plateformes + HUD back */
            afficherBackgroundEtElements(ren,bg,platforms,taille,
                                         font,tc,*timeLeft,
                                         (p1.vies+p2.vies)/2,
                                         bg->camera_pos.x,bg->camera_pos.y,
                                         screenW,screenH,MODE_MONO);

            /* 2) Joueurs + balles PAR-DESSUS */
            blitPlayer(ren,&p1); renderBullets(ren,&p1);
            blitPlayer(ren,&p2); renderBullets(ren,&p2);

            /* 3) HUD joueur */
            if (font) drawHUD(ren,font,&p1,&p2);
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    if      (!p1.isAlive&&!p2.isAlive) winner="MATCH NUL";
    else if (!p1.isAlive)              winner="CATWOMAN";
    else if (!p2.isAlive)              winner="BATMAN";
    else                               winner=(p1.hp>=p2.hp)?"BATMAN":"CATWOMAN";

    return gameOverScreen(ren,font,bg->img[0],winner,
                          p1.score,p2.score,screenW,screenH);
}

/* ============================================================
 * main
 * ============================================================ */
int main(int argc, char *argv[])
{
    SDL_Window   *window;
    SDL_Renderer *renderer;
    TTF_Font     *font;
    Background    bg;
    Platform      platforms[MAX_PLATFORMS];
    SDL_Event     event;
    int  taille=0, level=1, affMode=MODE_MONO, running=1, play=1;
    int  screenW, screenH, timeLeft=600;
    Uint32 lastTime;

    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!=0)
        { printf("SDL_Init: %s\n",SDL_GetError()); return 1; }
    if (TTF_Init()==-1)
        { printf("TTF_Init: %s\n",TTF_GetError()); SDL_Quit(); return 1; }
    if (IMG_Init(IMG_INIT_PNG|IMG_INIT_WEBP)==0)
        { printf("IMG_Init: %s\n",IMG_GetError()); TTF_Quit(); SDL_Quit(); return 1; }

    window = SDL_CreateWindow("Batman vs Catwoman",
                              SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
                              0,0,
                              SDL_WINDOW_SHOWN|SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!window) { printf("Window: %s\n",SDL_GetError()); SDL_Quit(); return 1; }

    renderer = SDL_CreateRenderer(window,-1,
                 SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { printf("Renderer: %s\n",SDL_GetError());
                     SDL_DestroyWindow(window); SDL_Quit(); return 1; }

    SDL_GetRendererOutputSize(renderer,&screenW,&screenH);

    initBackgroundAndPlatforms(renderer,&bg,platforms,&taille,
                               level,screenW,screenH);

    font = TTF_OpenFont("assets/font.ttf",18);
    if (!font) font = TTF_OpenFont("arial.ttf",18);
    if (!font) font = TTF_OpenFont("font.ttf",18);
    if (!font) printf("Warning: font not loaded\n");

    lastTime = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type==SDL_QUIT) { running=0; break; }
            if (event.type==SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_ESCAPE: running=0; break;
                    case SDLK_p:
                        affMode=(affMode==MODE_MONO)?MODE_MULTI:MODE_MONO; break;
                    case SDLK_F1:
                        level=1; taille=0;
                        initBackgroundAndPlatforms(renderer,&bg,platforms,
                                                   &taille,level,screenW,screenH);
                        timeLeft=600; lastTime=SDL_GetTicks(); break;
                    case SDLK_F2:
                        level=2; taille=0;
                        initBackgroundAndPlatforms(renderer,&bg,platforms,
                                                   &taille,level,screenW,screenH);
                        timeLeft=600; lastTime=SDL_GetTicks(); break;
                    default: break;
                }
            }
        }
        if (!running) break;

        timeLeft=600; lastTime=SDL_GetTicks();
        play = gameLoop(renderer,font,&bg,platforms,taille,
                        screenW,screenH,affMode,&timeLeft,&lastTime);
        if (!play) running=0;
    }

    /* Nettoyage */
    SDL_RenderSetViewport(renderer,NULL);
    if (font) saisirNomEtAfficherScore(renderer,font,0,screenW,screenH);

    if (bg.img[0])             SDL_DestroyTexture(bg.img[0]);
    if (bg.guide.image)        SDL_DestroyTexture(bg.guide.image);
    if (bg.commentJouer.image) SDL_DestroyTexture(bg.commentJouer.image);
    {
        int ii,ff; Platform *pp;
        for (ii=0;ii<taille;ii++) {
            pp=&platforms[ii];
            if (pp->isAnimated)
                { for(ff=0;ff<MAX_FRAMES;ff++) if(pp->frames[ff]) SDL_DestroyTexture(pp->frames[ff]); }
            else
                { if(pp->image) SDL_DestroyTexture(pp->image); }
        }
    }
    if (font) TTF_CloseFont(font);
    TTF_Quit(); IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
