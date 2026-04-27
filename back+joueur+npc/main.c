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
        
        obstacleScreenRect.x = platforms[i].position.x - (int)bg->camera_pos.x;
        obstacleScreenRect.y = platforms[i].position.y - (int)bg->camera_pos.y;
        obstacleScreenRect.w = platforms[i].position.w;
        obstacleScreenRect.h = platforms[i].position.h;
        
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
                case 0: p->x = obstacleScreenRect.x - p->w; p->vitesse = 0; break;
                case 1: p->x = obstacleScreenRect.x + obstacleScreenRect.w; p->vitesse = 0; break;
                case 2: p->y = obstacleScreenRect.y - p->h; p->vy = 0; p->onGround = 1; break;
                case 3: p->y = obstacleScreenRect.y + obstacleScreenRect.h; if (p->vy > 0) p->vy = 0; break;
            }
            
            p->hp -= OBSTACLE_DAMAGE;
            if (p->hp < 0) p->hp = 0;
            *invTimer = INVINCIBILITY_FRAMES;
            
            float kbDir = (p->x + p->w/2 < obstacleScreenRect.x + obstacleScreenRect.w/2) ? -1 : 1;
            p->x += kbDir * OBSTACLE_KNOCKBACK;
            
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
    Uint32        lastTime, prevTick;
    SDL_Event     event;
    Player        p1, p2;
    MenuResult    menuRes;
    float         groundEcran;

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
    
    printf("World width: %d, Platforms: %d\n", worldW, taille);

    bg.camera_pos.x = 0;
    bg.camera_pos.y = 0;
    bg.camera_pos.w = screenW;
    bg.camera_pos.h = screenH;

    groundEcran = (float)(screenH - JOUEUR_H - 20);

    initPlayer(&p1, 150.0f, groundEcran, 0);
    initPlayer(&p2, (float)(screenW - 250), groundEcran, 1);
    p1.w = JOUEUR_W; p1.h = JOUEUR_H;
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
    NPC_loadLevel(&gameNPC, renderer, 1);
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
                if (event.type == SDL_KEYDOWN &&
                    event.key.keysym.sym == SDLK_ESCAPE) { running = 0; break; }
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

            doScrolling(&bg, &p1, &p2, screenW, screenH, worldW, dt);
            bgX = bg.camera_pos.x;
            bgY = (int)bg.camera_pos.y;

            handleObstacleCollision(&p1, platforms, taille, &invObstacleP1, groundEcran, &bg);
            handleObstacleCollision(&p2, platforms, taille, &invObstacleP2, groundEcran, &bg);

            resolvePlayerCombat(&p1, &p2, &invP2);
            resolvePlayerCombat(&p2, &p1, &invP1);

            gererTemps(&timeLeft, &lastTime);
            if (timeLeft < 0) timeLeft = 0;

            if (timeLeft == 0 || !p1.isAlive || !p2.isAlive) {
                if (!p1.isAlive || !p2.isAlive) SDL_Delay(1500);
                running = 0;
            }

            updatePlatforms(platforms, taille);

            {
                SDL_Rect playerRect = { (int)p1.x, (int)p1.y, p1.w, p1.h };
                NPC_update(&gameNPC, &playerRect, &p1.hp, &p1.score, bg.camera_pos.x, bg.camera_pos.y);
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            afficherBackgroundEtElements(renderer, &bg, platforms, taille,
                                         font, dummyColor, timeLeft,
                                         (p1.vies > p2.vies) ? p1.vies : p2.vies,
                                         bgX, bgY, screenW, screenH, MODE_MONO);

            NPC_draw(&gameNPC, bg.camera_pos.x, bg.camera_pos.y);

            blitPlayer(renderer, &p1);
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

    if (batarang) SDL_DestroyTexture(batarang);
    if (p1.sprite.sheetRight) SDL_DestroyTexture(p1.sprite.sheetRight);
    if (p1.sprite.sheetLeft)  SDL_DestroyTexture(p1.sprite.sheetLeft);
    if (p2.sprite.sheetRight) SDL_DestroyTexture(p2.sprite.sheetRight);
    if (p2.sprite.sheetLeft)  SDL_DestroyTexture(p2.sprite.sheetLeft);

    {
        int i, f;
        if (bg.img[0]) SDL_DestroyTexture(bg.img[0]);
        if (bg.img[1]) SDL_DestroyTexture(bg.img[1]);
        if (bg.img[2]) SDL_DestroyTexture(bg.img[2]);
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
