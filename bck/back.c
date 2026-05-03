#include "back.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void initBackgroundAndPlatforms(SDL_Renderer *renderer, Background *bg,
                                 Platform platforms[], int *taille, int level,
                                 int screenW, int screenH)
{
    SDL_Surface *surface;
    int imgW, imgH;

    imgW = 0;
    imgH = 0;

    bg->posimg.x = 0;
    bg->posimg.y = 0;
    {
        int _bi;
        for (_bi = 0; _bi < 8; _bi++) { bg->img[_bi] = NULL; bg->partWReal[_bi] = 0; }
    }
    bg->imgCount = 1;
    bg->partW    = 0;
    bg->partH    = 0;
    bg->zoom     = 1.0f;

    /*
     * Load background with automatic slicing for images wider than MAX_TEX_W.
     * We load the full image as a surface, then blit strips into sub-surfaces
     * and upload each strip as a separate texture stored in bg->img[0..2].
     * bg->partW = width of each strip, bg->imgCount = number of strips.
     */
    {
#define MAX_TEX_W 4096
        char bgPath[32];
        SDL_Surface *fullSurf;
        int p, numParts, sliceW;

        if (level == 1)      snprintf(bgPath, sizeof(bgPath), "back/level1.png");
        else if (level == 2) snprintf(bgPath, sizeof(bgPath), "back/level2.png");
        else                 snprintf(bgPath, sizeof(bgPath), "back/level3.png");

        fullSurf = IMG_Load(bgPath);
        if (!fullSurf) {
            printf("Erreur chargement background %s : %s\n", bgPath, IMG_GetError());
            exit(EXIT_FAILURE);
        }

        imgW = fullSurf->w;
        imgH = fullSurf->h;

        /* How many slices of MAX_TEX_W do we need? (max 3 = bg->img capacity) */
        numParts = (imgW + MAX_TEX_W - 1) / MAX_TEX_W;
        if (numParts > 8) numParts = 8;
        sliceW   = (imgW + numParts - 1) / numParts;
        if (sliceW > MAX_TEX_W) sliceW = MAX_TEX_W;

        bg->imgCount = numParts;
        bg->partW    = sliceW;
        bg->partH    = imgH;

        /* Zoom fixe par level:
         * level 1 : 1082px hauteur ~ ecran -> zoom 1.0
         * level 2/3 : ~4500px hauteur -> zoom 0.32 pour voir le sol + scroller */
        if (level == 1)
            bg->zoom = 1.0f;
        else
            bg->zoom = 0.32f;

        for (p = 0; p < numParts; p++) {
            SDL_Surface *part;
            SDL_Rect srcRect;
            int thisW = sliceW;
            if (p * sliceW + thisW > imgW) thisW = imgW - p * sliceW;

            srcRect.x = p * sliceW;
            srcRect.y = 0;
            srcRect.w = thisW;
            srcRect.h = imgH;

            part = SDL_CreateRGBSurface(0, thisW, imgH,
                                        fullSurf->format->BitsPerPixel,
                                        fullSurf->format->Rmask,
                                        fullSurf->format->Gmask,
                                        fullSurf->format->Bmask,
                                        fullSurf->format->Amask);
            if (!part) {
                printf("Erreur SDL_CreateRGBSurface part %d : %s\n", p, SDL_GetError());
                SDL_FreeSurface(fullSurf);
                exit(EXIT_FAILURE);
            }
            SDL_BlitSurface(fullSurf, &srcRect, part, NULL);
            bg->img[p] = SDL_CreateTextureFromSurface(renderer, part);
            bg->partWReal[p] = thisW;
            SDL_FreeSurface(part);
            if (!bg->img[p]) {
                printf("Erreur texture part %d : %s\n", p, SDL_GetError());
                SDL_FreeSurface(fullSurf);
                exit(EXIT_FAILURE);
            }
        }
        SDL_FreeSurface(fullSurf);
#undef MAX_TEX_W
    }
    bg->camera_pos.x = 0;
    /* Start camera at bottom so the ground/street is visible */
    {
        int scaledH = (int)(imgH * bg->zoom);
        if (scaledH > screenH)
            bg->camera_pos.y = scaledH - screenH;
        else
            bg->camera_pos.y = 0;
    }
    bg->camera_pos.w = screenW;
    bg->camera_pos.h = screenH;
    bg->direction = 0;


    surface = IMG_Load("how.png");
    if (!surface) { printf("Erreur how.png\n"); exit(EXIT_FAILURE); }
    bg->commentJouer.image = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_QueryTexture(bg->commentJouer.image, NULL, NULL,
                     &bg->commentJouer.w, &bg->commentJouer.h);
    SDL_FreeSurface(surface);
    bg->afficherCommentJouer = 0;

    *taille = 0;
    {
        int k;
        for (k = 0; k < MAX_PLATFORMS; k++) {
            platforms[k].image       = NULL;
            platforms[k].isAnimated  = 0;
            platforms[k].type        = PLATFORM_FIXE;
            platforms[k].destroyed   = 0;
            platforms[k].hp          = 0;
            platforms[k].vitesse     = 0;
            platforms[k].moveDir     = 0;
            platforms[k].moveAxis    = 0;
            platforms[k].moveMin     = 0;
            platforms[k].moveMax     = 0;
            platforms[k].currentFrame= 0;
            platforms[k].lastFrameTime=0;
            platforms[k].role        = 0;
            platforms[k].collected   = 0;
            platforms[k].bonusValue  = 0;
        }
    }

    if (level == 1) {
        int streetY = screenH + 160;
        int carW = 750, carH = 300;
        int bW   = 180, bH   = 90;
        int eW   = 110, eH   = 140;
        int f;

        /* ============================================================
         * POSITIONS FIXES extraites du screenshot de reference
         * Cars   : x = 1942, 3513, 5566, 7425, 8955, 10250
         * Fires  : x = 2700, 6089
         * Bonus  : x = 895(h458), 3320(h347), 4119(h49), 4546(h61), 6778(h347), 8183(h74), 11077(h111)
         * Enigme : x = 5538(haut), 9423(haut)
         * Car lv2 fin : x = imgW-2400
         * ============================================================ */

        /* --- CARS (obstaclelevel1) --- */
        {
            int carPosX[] = {2200, 3800, 8200};
            int k;
            for (k = 0; k < 4 && *taille < MAX_PLATFORMS; k++) {
                SDL_Surface *s = IMG_Load("obstaclelevel1/car.png");
                if (!s) continue;
                for (f = 0; f < MAX_FRAMES; f++) platforms[*taille].frames[f] = NULL;
                platforms[*taille].image        = SDL_CreateTextureFromSurface(renderer, s);
                platforms[*taille].isAnimated   = 0;
                platforms[*taille].type         = PLATFORM_FIXE;
                platforms[*taille].destroyed    = 0; platforms[*taille].hp = 0;
                platforms[*taille].role         = ROLE_OBSTACLE;
                platforms[*taille].collected    = 0; platforms[*taille].bonusValue = 0;
                platforms[*taille].currentFrame = 0; platforms[*taille].lastFrameTime = 0;
                platforms[*taille].position.w   = carW;
                platforms[*taille].position.h   = carH;
                platforms[*taille].position.x   = carPosX[k];
                platforms[*taille].position.y   = streetY - carH;
                SDL_FreeSurface(s); (*taille)++;
            }
        }

        /* --- BONUS / BATARANGS (animated) --- */
        {
            /* x, height_above_ground */
            int bonusPosX[] = {895,  3320,  4546,4819, 6778, 8183, 11077};
            int bonusH[]    = {458,  347,    61, 89,   347,  74,   111  };
            int k;
            for (k = 0; k < 7 && *taille < MAX_PLATFORMS; k++) {
                int loaded = 0;
                for (f = 0; f < MAX_FRAMES; f++) platforms[*taille].frames[f] = NULL;
                /* frames: bonus/ 5,7,8,9,...,46 (6 manquant) */
                {
                    int fNums[] = {5,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
                                   21,22,23,24,25,26,27,28,29,30,31,32,33,34,
                                   35,36,37,38,39,40,41,42,43,44,45,46};
                    int fi, nf = 41;
                    for (fi = 0; fi < nf && fi < MAX_FRAMES; fi++) {
                        char path[128];
                        SDL_Surface *fs;
                        snprintf(path, sizeof(path), "bonus/Untitled (%d).png", fNums[fi]);
                        fs = IMG_Load(path);
                        if (fs) {
                            platforms[*taille].frames[fi] = SDL_CreateTextureFromSurface(renderer, fs);
                            SDL_FreeSurface(fs);
                            loaded++;
                        }
                    }
                }
                if (loaded > 0) {
                    platforms[*taille].image      = NULL;
                    platforms[*taille].isAnimated = 1;
                } else {
                    /* fallback: static bonus.png */
                    SDL_Surface *s = IMG_Load("bonus.png");
                    if (!s) continue;
                    platforms[*taille].image      = SDL_CreateTextureFromSurface(renderer, s);
                    platforms[*taille].isAnimated = 0;
                    SDL_FreeSurface(s);
                }
                platforms[*taille].type         = PLATFORM_FIXE;
                platforms[*taille].destroyed    = 0; platforms[*taille].hp = 0;
                platforms[*taille].role         = ROLE_BONUS;
                platforms[*taille].collected    = 0; platforms[*taille].bonusValue = 50;
                platforms[*taille].currentFrame = 0; platforms[*taille].lastFrameTime = 0;
                platforms[*taille].position.w   = bW;
                platforms[*taille].position.h   = bH;
                platforms[*taille].position.x   = bonusPosX[k];
                platforms[*taille].position.y   = streetY - bH - bonusH[k];
                (*taille)++;
            }
        }

        /* --- BOXES (obstaclelevel1) --- */
        {
            int boxPosX[] = {800, 5400, 6800};
            int k;
            for (k = 0; k < 3 && *taille < MAX_PLATFORMS; k++) {
                SDL_Surface *s = IMG_Load("obstaclelevel1/box.png");
                if (!s) continue;
                for (f = 0; f < MAX_FRAMES; f++) platforms[*taille].frames[f] = NULL;
                platforms[*taille].image        = SDL_CreateTextureFromSurface(renderer, s);
                platforms[*taille].isAnimated   = 0;
                platforms[*taille].type         = PLATFORM_FIXE;
                platforms[*taille].destroyed    = 0; platforms[*taille].hp = 0;
                platforms[*taille].role         = ROLE_OBSTACLE;
                platforms[*taille].collected    = 0; platforms[*taille].bonusValue = 0;
                platforms[*taille].currentFrame = 0; platforms[*taille].lastFrameTime = 0;
                platforms[*taille].position.w   = 150;
                platforms[*taille].position.h   = 150;
                platforms[*taille].position.x   = boxPosX[k];
                platforms[*taille].position.y   = streetY - 150;
                SDL_FreeSurface(s); (*taille)++;
            }
        }

        /* --- ENIGMES (joker cards) --- */
        {
            int enigmePosX[] = {5538, 9423};
            int k;
            for (k = 0; k < 2 && *taille < MAX_PLATFORMS; k++) {
                SDL_Surface *s = IMG_Load("enigme.png");
                if (!s) continue;
                for (f = 0; f < MAX_FRAMES; f++) platforms[*taille].frames[f] = NULL;
                platforms[*taille].image        = SDL_CreateTextureFromSurface(renderer, s);
                platforms[*taille].isAnimated   = 0;
                platforms[*taille].type         = PLATFORM_FIXE;
                platforms[*taille].destroyed    = 0; platforms[*taille].hp = 0;
                platforms[*taille].role         = ROLE_ENIGME;
                platforms[*taille].collected    = 0; platforms[*taille].bonusValue = 100;
                platforms[*taille].currentFrame = 0; platforms[*taille].lastFrameTime = 0;
                platforms[*taille].position.w   = eW;
                platforms[*taille].position.h   = eH;
                platforms[*taille].position.x   = enigmePosX[k];
                platforms[*taille].position.y   = streetY - carH - eH - 350;
                SDL_FreeSurface(s); (*taille)++;
            }
        }

        /* --- CAR obstaclelevel2 a la toute fin --- */
        if (*taille < MAX_PLATFORMS) {
            SDL_Surface *s = IMG_Load("obstaclelevel2/car.png");
            if (s) {
                int carW2 = 700, carH2 = 300;
                for (f = 0; f < MAX_FRAMES; f++) platforms[*taille].frames[f] = NULL;
                platforms[*taille].image        = SDL_CreateTextureFromSurface(renderer, s);
                platforms[*taille].isAnimated   = 0;
                platforms[*taille].type         = PLATFORM_FIXE;
                platforms[*taille].destroyed    = 0; platforms[*taille].hp = 0;
                platforms[*taille].role         = ROLE_OBSTACLE;
                platforms[*taille].collected    = 0; platforms[*taille].bonusValue = 0;
                platforms[*taille].currentFrame = 0; platforms[*taille].lastFrameTime = 0;
                platforms[*taille].position.w   = carW2;
                platforms[*taille].position.h   = carH2;
                platforms[*taille].position.x   = imgW - carW2 - 200;
                platforms[*taille].position.y   = streetY - carH2;
                SDL_FreeSurface(s); (*taille)++;
            }
        }
    } else if (level == 2) {
        int k;
        int streetY2 = (int)((screenH + 30) / bg->zoom);
        int carW2 = 2200, carH2 = 780;
        int boxW2 =  560, boxH2 = 560;
        int firW2 =  380, firH2 = 690;

        int safeZone2 = imgW / 8;
        int obsZoneW2 = imgW - safeZone2 - 200;
        int usedObs2[8]; int nbObs2 = 0;

        #define RAND_X2(minGap, objW) ({             int _rx = 0, _ok = 0, _tr = 0, _t;             do {                 _rx = safeZone2 + rand() % (obsZoneW2 - (objW));                 _ok = 1;                 for (_t = 0; _t < nbObs2; _t++)                     if (abs(_rx - usedObs2[_t]) < (minGap)) { _ok = 0; break; }                 _tr++;             } while (!_ok && _tr < 60);             usedObs2[nbObs2++] = _rx; _rx; })

        /* 2 cars aleatoires */
        for (k = 0; k < 2 && *taille < MAX_PLATFORMS; k++) {
            SDL_Surface *s = IMG_Load("obstaclelevel2/car.png");
            if (s) {
                int rx = RAND_X2(carW2 + 1800, carW2);
                platforms[*taille].image      = SDL_CreateTextureFromSurface(renderer, s);
                platforms[*taille].isAnimated = 0; platforms[*taille].type = PLATFORM_FIXE;
                platforms[*taille].destroyed  = 0; platforms[*taille].hp   = 0;
                platforms[*taille].role       = ROLE_OBSTACLE;
                platforms[*taille].collected  = 0; platforms[*taille].bonusValue = 0;
                platforms[*taille].position.w = carW2; platforms[*taille].position.h = carH2;
                platforms[*taille].position.x = rx;
                platforms[*taille].position.y = streetY2 - carH2;
                SDL_FreeSurface(s); (*taille)++;
            }
        }
        /* 2 boxes aleatoires */
        for (k = 0; k < 2 && *taille < MAX_PLATFORMS; k++) {
            SDL_Surface *s = IMG_Load("obstaclelevel2/box.png");
            if (s) {
                int rx = RAND_X2(boxW2 + 1500, boxW2);
                platforms[*taille].image      = SDL_CreateTextureFromSurface(renderer, s);
                platforms[*taille].isAnimated = 0; platforms[*taille].type = PLATFORM_FIXE;
                platforms[*taille].destroyed  = 0; platforms[*taille].hp   = 0;
                platforms[*taille].role       = ROLE_OBSTACLE;
                platforms[*taille].collected  = 0; platforms[*taille].bonusValue = 0;
                platforms[*taille].position.w = boxW2; platforms[*taille].position.h = boxH2;
                platforms[*taille].position.x = rx;
                platforms[*taille].position.y = streetY2 - boxH2;
                SDL_FreeSurface(s); (*taille)++;
            }
        }
        /* 2 fires aleatoires */
        for (k = 0; k < 2 && *taille < MAX_PLATFORMS; k++) {
            SDL_Surface *s = IMG_Load("obstaclelevel2/fire.png");
            if (s) {
                int rx = RAND_X2(firW2 + 1500, firW2);
                platforms[*taille].image      = SDL_CreateTextureFromSurface(renderer, s);
                platforms[*taille].isAnimated = 0; platforms[*taille].type = PLATFORM_FIXE;
                platforms[*taille].destroyed  = 0; platforms[*taille].hp   = 0;
                platforms[*taille].role       = ROLE_OBSTACLE;
                platforms[*taille].collected  = 0; platforms[*taille].bonusValue = 0;
                platforms[*taille].position.w = firW2; platforms[*taille].position.h = firH2;
                platforms[*taille].position.x = rx;
                platforms[*taille].position.y = streetY2 - firH2;
                SDL_FreeSurface(s); (*taille)++;
            }
        }
        /* 1 car supplementaire obstaclelevel2 a la fin */
        if (*taille < MAX_PLATFORMS) {
            SDL_Surface *s = IMG_Load("obstaclelevel2/car.png");
            if (s) {
                int rx = imgW - carW2 - 200;
                platforms[*taille].image      = SDL_CreateTextureFromSurface(renderer, s);
                platforms[*taille].isAnimated = 0; platforms[*taille].type = PLATFORM_FIXE;
                platforms[*taille].destroyed  = 0; platforms[*taille].hp   = 0;
                platforms[*taille].role       = ROLE_OBSTACLE;
                platforms[*taille].collected  = 0; platforms[*taille].bonusValue = 0;
                platforms[*taille].position.w = carW2; platforms[*taille].position.h = carH2;
                platforms[*taille].position.x = rx;
                platforms[*taille].position.y = streetY2 - carH2;
                SDL_FreeSurface(s); (*taille)++;
            }
        }
        #undef RAND_X2
    } else if (level == 3) {
        /*
         * Level 3 – animated obstacles:
         * car (karhba): 147 frames in obstaclelevel3/karhba/ezgif-frame-NNN.png
         * box        : 16 frames in obstaclelevel3/box/ezgif-frame-NNN.png
         * We load MAX_FRAMES (16) frames for each.
         */
        int k, f;
        /* streetY en coords monde: sol ecran (screenH-20) / zoom */
        int streetY3 = (int)((screenH + 30) / bg->zoom);
        int carW3 = 2200, carH3 = 780;
        int boxW3 =  560, boxH3 = 560;
        int firW3 =  380, firH3 = 690;
        int posX3[6];
        { int step3 = imgW / 6;
          posX3[0] = step3 * 1; posX3[1] = step3 * 2;
          posX3[2] = step3 * 3; posX3[3] = step3 * 4;
          posX3[4] = step3 * 5; posX3[5] = step3 * 6 - step3 / 2; }

        /* karhba (car) at positions 0, 3 */
        for (k = 0; k < 2 && *taille < MAX_PLATFORMS; k++) {
            int newW = carW3;
            int newH = carH3;
            int loaded = 0;
            /* Reset all frames to NULL first */
            for (f = 0; f < MAX_FRAMES; f++) platforms[*taille].frames[f] = NULL;
            for (f = 0; f < MAX_FRAMES; f++) {
                char path[64];
                snprintf(path, sizeof(path),
                         "obstaclelevel3/karhba/ezgif-frame-%03d.png", f + 1);
                SDL_Surface *s = IMG_Load(path);
                if (s) {
                    platforms[*taille].frames[f] = SDL_CreateTextureFromSurface(renderer, s);
                    SDL_FreeSurface(s);
                    loaded++;
                }
            }
            if (loaded > 0) {
                platforms[*taille].isAnimated    = 1;
                platforms[*taille].image         = NULL;
            } else {
                /* Fallback: use static car.png */
                printf("Level3 karhba frames non trouvees, fallback car.png\n");
                SDL_Surface *s = IMG_Load("obstaclelevel3/car.png");
                if (!s) s = IMG_Load("obstaclelevel2/car.png");
                platforms[*taille].isAnimated = 0;
                platforms[*taille].image      = s ? SDL_CreateTextureFromSurface(renderer, s) : NULL;
                if (s) SDL_FreeSurface(s);
                if (!platforms[*taille].image) continue; /* skip if both fail */
            }
            platforms[*taille].currentFrame  = 0;
            platforms[*taille].lastFrameTime = 0;
            platforms[*taille].type          = PLATFORM_FIXE;
            platforms[*taille].destroyed     = 0;
            platforms[*taille].hp            = 0;
            platforms[*taille].position.w    = newW;
            platforms[*taille].position.h    = newH;
            platforms[*taille].position.x    = posX3[k == 0 ? 0 : 3] - newW / 2;
            platforms[*taille].position.y    = streetY3 - newH;
            (*taille)++;
        }

        /* box at positions 1, 4 */
        for (k = 0; k < 2 && *taille < MAX_PLATFORMS; k++) {
            int newW = boxW3;
            int newH = boxH3;
            int loaded = 0;
            /* Reset all frames to NULL first */
            for (f = 0; f < MAX_FRAMES; f++) platforms[*taille].frames[f] = NULL;
            for (f = 0; f < MAX_FRAMES; f++) {
                char path[64];
                snprintf(path, sizeof(path),
                         "obstaclelevel3/box/ezgif-frame-%03d.png", f + 1);
                SDL_Surface *s = IMG_Load(path);
                if (s) {
                    platforms[*taille].frames[f] = SDL_CreateTextureFromSurface(renderer, s);
                    SDL_FreeSurface(s);
                    loaded++;
                }
            }
            if (loaded > 0) {
                platforms[*taille].isAnimated = 1;
                platforms[*taille].image      = NULL;
            } else {
                /* Fallback: use static box.png */
                printf("Level3 box frames non trouvees, fallback box.png\n");
                SDL_Surface *s = IMG_Load("obstaclelevel3/box.png");
                if (!s) s = IMG_Load("obstaclelevel2/box.png");
                platforms[*taille].isAnimated = 0;
                platforms[*taille].image      = s ? SDL_CreateTextureFromSurface(renderer, s) : NULL;
                if (s) SDL_FreeSurface(s);
                if (!platforms[*taille].image) continue; /* skip if both fail */
            }
            platforms[*taille].currentFrame  = 0;
            platforms[*taille].lastFrameTime = 0;
            platforms[*taille].type          = PLATFORM_FIXE;
            platforms[*taille].destroyed     = 0;
            platforms[*taille].hp            = 0;
            platforms[*taille].position.w    = newW;
            platforms[*taille].position.h    = newH;
            platforms[*taille].position.x    = posX3[k == 0 ? 1 : 4] - newW / 2;
            platforms[*taille].position.y    = streetY3 - newH;
            (*taille)++;
        }

        /* fire aux positions 2, 5 */
        for (k = 0; k < 2 && *taille < MAX_PLATFORMS; k++) {
            SDL_Surface *s = IMG_Load("obstaclelevel3/fire.png");
            if (!s) s = IMG_Load("obstaclelevel2/fire.png");
            if (s) {
                for (f = 0; f < MAX_FRAMES; f++) platforms[*taille].frames[f] = NULL;
                platforms[*taille].image      = SDL_CreateTextureFromSurface(renderer, s);
                platforms[*taille].isAnimated = 0;
                platforms[*taille].type       = PLATFORM_FIXE;
                platforms[*taille].destroyed  = 0; platforms[*taille].hp = 0;
                platforms[*taille].currentFrame = 0; platforms[*taille].lastFrameTime = 0;
                platforms[*taille].position.w = firW3; platforms[*taille].position.h = firH3;
                platforms[*taille].position.x = posX3[k == 0 ? 2 : 5] - firW3 / 2;
                platforms[*taille].position.y = streetY3 - firH3;
                SDL_FreeSurface(s); (*taille)++;
            }
        }

        /* 1 car obstaclelevel2 a la fin du niveau 3 */
        if (*taille < MAX_PLATFORMS) {
            SDL_Surface *s = IMG_Load("obstaclelevel2/car.png");
            if (s) {
                int carW2 = 2200, carH2 = 780;
                for (f = 0; f < MAX_FRAMES; f++) platforms[*taille].frames[f] = NULL;
                platforms[*taille].image      = SDL_CreateTextureFromSurface(renderer, s);
                platforms[*taille].isAnimated = 0;
                platforms[*taille].type       = PLATFORM_FIXE;
                platforms[*taille].destroyed  = 0; platforms[*taille].hp = 0;
                platforms[*taille].role       = ROLE_OBSTACLE;
                platforms[*taille].collected  = 0; platforms[*taille].bonusValue = 0;
                platforms[*taille].currentFrame = 0; platforms[*taille].lastFrameTime = 0;
                platforms[*taille].position.w = carW2;
                platforms[*taille].position.h = carH2;
                platforms[*taille].position.x = imgW - carW2 - 300;
                platforms[*taille].position.y = streetY3 - carH2;
                SDL_FreeSurface(s); (*taille)++;
            }
        }
    }
}

void updatePlatforms(Platform platforms[], int taille)
{
    int i;
    Platform *p;
    for (i = 0; i < taille; i++) {
        p = &platforms[i];
        if (p->destroyed || p->type != PLATFORM_MOBILE) continue;
        if (p->moveAxis == 0) {
            p->position.x += p->vitesse * p->moveDir;
            /*
             * Wrap-around mode: if moveDir == -1 (left-only mover) and the
             * obstacle exits the left edge, teleport it back to the right.
             * If moveDir can be +1 too (patrol), use classic bounce instead.
             */
            if (p->moveDir == -1) {
                /* left-moving wrap: reappear from the right once fully off-screen */
                if (p->position.x + p->position.w < 0)
                    p->position.x = p->moveMax;
            } else {
                /* patrol / bounce */
                if (p->position.x <= p->moveMin)                  p->moveDir =  1;
                if (p->position.x + p->position.w >= p->moveMax)  p->moveDir = -1;
            }
        } else {
            p->position.y += p->vitesse * p->moveDir;
            if (p->position.y <= p->moveMin)                  p->moveDir =  1;
            if (p->position.y + p->position.h >= p->moveMax)  p->moveDir = -1;
        }
    }
}

/* Glow multi-couches autour d'un rect (bonus/enigme) */
static void drawGlow(SDL_Renderer *renderer, SDL_Rect *dest,
                     Uint8 r, Uint8 g, Uint8 b, int layers, Uint8 baseAlpha)
{
    int l;
    SDL_Rect gr;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (l = layers; l >= 1; l--) {
        int expand = l * 6;
        Uint8 a = (Uint8)(baseAlpha / (l + 1));
        gr.x = dest->x - expand;
        gr.y = dest->y - expand;
        gr.w = dest->w + expand * 2;
        gr.h = dest->h + expand * 2;
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
        SDL_RenderFillRect(renderer, &gr);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void afficherPlatforms(SDL_Renderer *renderer, Platform platforms[],
                        int taille, int bgX, int bgY, float zoom)
{
    int i, h, segW;
    Platform *p;
    SDL_Rect dest;
    SDL_Rect seg;
    Uint32 now = SDL_GetTicks();

    for (i = 0; i < taille; i++) {
        p = &platforms[i];
        if (p->destroyed) continue;
        if (p->collected) continue;

        dest.x = (int)(p->position.x * zoom) - bgX;
        dest.y = (int)(p->position.y * zoom) - bgY;
        dest.w = (int)(p->position.w * zoom);
        dest.h = (int)(p->position.h * zoom);

        /* === BONUS : glow dore pulsant (kif Cobblebot Caper) === */
        if (p->role == ROLE_BONUS) {
            float pulse = (float)(now % 900) / 900.0f;
            float s     = (pulse < 0.5f) ? pulse * 2.0f : (1.0f - pulse) * 2.0f;
            Uint8 alpha = (Uint8)(40 + (int)(s * 70));
            drawGlow(renderer, &dest, 255, 210, 0, 4, alpha);
        }
        /* === ENIGME : glow violet pulsant === */
        else if (p->role == ROLE_ENIGME) {
            float pulse = (float)(now % 1200) / 1200.0f;
            float s     = (pulse < 0.5f) ? pulse * 2.0f : (1.0f - pulse) * 2.0f;
            Uint8 alpha = (Uint8)(50 + (int)(s * 80));
            drawGlow(renderer, &dest, 0, 150, 255, 4, alpha);
        }
        /* obstacles: pas d'effet supplementaire */

        /* === Rendu image / animation === */
        if (p->isAnimated) {
            int numFrames = 0;
            while (numFrames < MAX_FRAMES && p->frames[numFrames]) numFrames++;
            if (numFrames == 0) continue;
            if (now - p->lastFrameTime > 80) {
                p->currentFrame = (p->currentFrame + 1) % numFrames;
                p->lastFrameTime = now;
            }
            SDL_RenderCopy(renderer, p->frames[p->currentFrame], NULL, &dest);
        } else {
            if (p->image)
                SDL_RenderCopy(renderer, p->image, NULL, &dest);
        }

        /* === Barre de vie plateformes destructibles === */
        if (p->type == PLATFORM_DESTRUCTIBLE && p->hp > 0) {
            segW = p->position.w / 3;
            for (h = 0; h < p->hp && h < 3; h++) {
                seg.x = dest.x + h * segW + 2;
                seg.y = dest.y - 7;
                seg.w = segW - 4;
                seg.h = 5;
                SDL_SetRenderDrawColor(renderer, 210, 80, 20, 255);
                SDL_RenderFillRect(renderer, &seg);
            }
        }

        /* === Contour lumineux anime apres l'image (bonus = or, enigme = violet) === */
        if (p->role == ROLE_BONUS) {
            float pulse = (float)(now % 900) / 900.0f;
            float s     = (pulse < 0.5f) ? pulse * 2.0f : (1.0f - pulse) * 2.0f;
            Uint8 alpha = (Uint8)(80 + (int)(s * 120));
            SDL_Rect inner = {dest.x+1, dest.y+1, dest.w-2, dest.h-2};
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 230, 80, alpha);
            SDL_RenderDrawRect(renderer, &dest);
            SDL_SetRenderDrawColor(renderer, 255, 200, 0, (Uint8)(alpha/2));
            SDL_RenderDrawRect(renderer, &inner);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        } else if (p->role == ROLE_ENIGME) {
            float pulse = (float)(now % 1200) / 1200.0f;
            float s     = (pulse < 0.5f) ? pulse * 2.0f : (1.0f - pulse) * 2.0f;
            Uint8 alpha = (Uint8)(100 + (int)(s * 130));
            SDL_Rect inner = {dest.x+1, dest.y+1, dest.w-2, dest.h-2};
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 180, 255, alpha);
            SDL_RenderDrawRect(renderer, &dest);
            SDL_SetRenderDrawColor(renderer, 0, 120, 220, (Uint8)(alpha/2));
            SDL_RenderDrawRect(renderer, &inner);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }
    }
}

void gererScrollingDeuxJoueurs(SDL_Event event,
                                Background *bg1, Background *bg2,
                                int scrollSpeed, int level)
{
    int imgW, imgH;
    const Uint8 *keys;

    imgW = 0;
    imgH = 0;
    (void)event;

    keys = SDL_GetKeyboardState(NULL);

    if (bg1->img[0]) {
        int _pi;
        imgW = 0;
        for (_pi = 0; _pi < bg1->imgCount; _pi++) imgW += bg1->partWReal[_pi];
        imgH = bg1->partH;
    }

    /* Joueur 1 : fleches, scrolling 4 sens */
    if (keys[SDL_SCANCODE_RIGHT]) { bg1->direction = 0; bg1->camera_pos.x += scrollSpeed; }
    if (keys[SDL_SCANCODE_LEFT])  { bg1->direction = 1; bg1->camera_pos.x -= scrollSpeed; }
    if (keys[SDL_SCANCODE_UP])    { bg1->direction = 2; bg1->camera_pos.y -= scrollSpeed; }
    if (keys[SDL_SCANCODE_DOWN])  { bg1->direction = 3; bg1->camera_pos.y += scrollSpeed; }

    /* Joueur 2 : WASD/ZQSD, scrolling 4 sens */
    if (keys[SDL_SCANCODE_D])                         { bg2->direction = 0; bg2->camera_pos.x += scrollSpeed; }
    if (keys[SDL_SCANCODE_Q] || keys[SDL_SCANCODE_A]) { bg2->direction = 1; bg2->camera_pos.x -= scrollSpeed; }
    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_Z]) { bg2->direction = 2; bg2->camera_pos.y -= scrollSpeed; }
    if (keys[SDL_SCANCODE_S])                         { bg2->direction = 3; bg2->camera_pos.y += scrollSpeed; }

    /* Clamp camera bg1 — clamp against scaled world size */
    {
        int sw = (int)(imgW * bg1->zoom);
        int sh = (int)(imgH * bg1->zoom);
        if (sw > 0) {
            if (bg1->camera_pos.x < 0) bg1->camera_pos.x = 0;
            if (bg1->camera_pos.w > 0 && bg1->camera_pos.x + bg1->camera_pos.w > sw)
                bg1->camera_pos.x = sw - bg1->camera_pos.w;
            if (bg1->camera_pos.x < 0) bg1->camera_pos.x = 0;
        }
        if (sh > 0) {
            if (bg1->camera_pos.y < 0) bg1->camera_pos.y = 0;
            if (bg1->camera_pos.h > 0 && bg1->camera_pos.y + bg1->camera_pos.h > sh)
                bg1->camera_pos.y = sh - bg1->camera_pos.h;
            if (bg1->camera_pos.y < 0) bg1->camera_pos.y = 0;
        }
    }

    /* Clamp camera bg2 (independant meme si bg2 == bg1) */
    if (bg2 != bg1) {
        int sw = (int)(imgW * bg2->zoom);
        int sh = (int)(imgH * bg2->zoom);
        if (sw > 0) {
            if (bg2->camera_pos.x < 0) bg2->camera_pos.x = 0;
            if (bg2->camera_pos.w > 0 && bg2->camera_pos.x + bg2->camera_pos.w > sw)
                bg2->camera_pos.x = sw - bg2->camera_pos.w;
            if (bg2->camera_pos.x < 0) bg2->camera_pos.x = 0;
        }
        if (sh > 0) {
            if (bg2->camera_pos.y < 0) bg2->camera_pos.y = 0;
            if (bg2->camera_pos.h > 0 && bg2->camera_pos.y + bg2->camera_pos.h > sh)
                bg2->camera_pos.y = sh - bg2->camera_pos.h;
            if (bg2->camera_pos.y < 0) bg2->camera_pos.y = 0;
        }
    }

    (void)level;
}

void gererTemps(int *timeLeft, Uint32 *lastTime)
{
    Uint32 now   = SDL_GetTicks();
    Uint32 delta = now - *lastTime;
    if (delta >= 1000) {
        *timeLeft -= (int)(delta / 1000);
        *lastTime += (delta / 1000) * 1000;
    }
}

void afficherTemps(SDL_Renderer *renderer, TTF_Font *font,
                   int timeLeft, int screenW)
{
    SDL_Color couleur;
    SDL_Surface *surf;
    SDL_Texture *tex;
    SDL_Rect dst;
    char buf[16];
    int minutes  = timeLeft / 60;
    int secondes = timeLeft % 60;

    if (!font) return;
    snprintf(buf, sizeof(buf), "%02d:%02d", minutes, secondes);

    if (timeLeft <= 30) {
        couleur.r = 255; couleur.g = 60;  couleur.b = 60;  couleur.a = 255;
    } else {
        couleur.r = 0;   couleur.g = 230; couleur.b = 255; couleur.a = 255;
    }

    surf = TTF_RenderText_Blended(font, buf, couleur);
    if (!surf) return;
    tex  = SDL_CreateTextureFromSurface(renderer, surf);
    dst.x = screenW / 2 - surf->w / 2;
    dst.y = 12;
    dst.w = surf->w;
    dst.h = surf->h;
    SDL_RenderCopy(renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

void gererGuideEtClic(SDL_Event event, GuideButton *guide,
                       SDL_TextureWithRect *commentJouer,
                       int *afficherCommentJouer)
{
    int x, y;
    (void)commentJouer;

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        x = event.button.x;
        y = event.button.y;
        if (*afficherCommentJouer) {
            *afficherCommentJouer = 0;
        } else if (x >= guide->position.x && x <= guide->position.x + guide->w &&
                   y >= guide->position.y && y <= guide->position.y + guide->h) {
            *afficherCommentJouer = 1;
        }
    }
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_g)
            *afficherCommentJouer = !(*afficherCommentJouer);
        if (event.key.keysym.sym == SDLK_r)
            *afficherCommentJouer = 0;
    }
}

void afficherBackgroundEtElements(SDL_Renderer *renderer, Background *bg,
                                   Platform platforms[], int taille,
                                   TTF_Font *font, SDL_Color textColor,
                                   int timeLeft, int lives,
                                   int bgX, int bgY,
                                   int screenW, int screenH, int mode)
{
    int maxTime, barW, barH, rad, barX, barY;
    int filled, fr, y, dy0, dy1, dx;
    int bat[14][2];
    int bw, bh, spacing, startX, startY, i, row, cx2, ox, rw;
    int maxW, maxH;
    float scaleW, scaleH, scale;
    char livesStr[8];
    SDL_Color cyan;
    SDL_Surface *lblSurf;
    SDL_Texture *lblTex;
    SDL_Rect r, destImg;
    int imgW, imgH;

    (void)textColor;

    /* --- Compute total world size (sum of real slice widths) --- */
    {
        int _pi;
        imgW = 0;
        for (_pi = 0; _pi < bg->imgCount; _pi++) imgW += bg->partWReal[_pi];
    }
    imgH = bg->partH;
    {
        int scaledTotalW = (int)(imgW * bg->zoom);
        int scaledTotalH = (int)(imgH * bg->zoom);
        int scaledPartH  = (int)(bg->partH * bg->zoom);

        /* Camera clamp in world space */
        if (bg->camera_pos.x < 0) bg->camera_pos.x = 0;
        if (scaledTotalW > screenW && bg->camera_pos.x + screenW > scaledTotalW)
            bg->camera_pos.x = scaledTotalW - screenW;
        if (bg->camera_pos.x < 0) bg->camera_pos.x = 0;

        if (bg->camera_pos.y < 0) bg->camera_pos.y = 0;
        if (scaledTotalH > screenH && bg->camera_pos.y + screenH > scaledTotalH)
            bg->camera_pos.y = scaledTotalH - screenH;
        if (bg->camera_pos.y < 0) bg->camera_pos.y = 0;

        /* --- Draw background parts with zoom --- */
        {
            int p;
            {
            int _cumX = 0;
            for (p = 0; p < bg->imgCount; p++) {
                int partScreenX = (int)(_cumX * bg->zoom) - bgX;
                int thisScaledW = (int)(bg->partWReal[p] * bg->zoom);

                if (partScreenX + thisScaledW <= 0) { _cumX += bg->partWReal[p]; continue; }
                if (partScreenX >= screenW)          { _cumX += bg->partWReal[p]; continue; }

                {
                    SDL_Rect tileDst;
                    tileDst.x = partScreenX;
                    tileDst.y = -bgY;
                    tileDst.w = (int)(bg->partWReal[p] * bg->zoom);
                    tileDst.h = scaledPartH;

                    SDL_RenderCopy(renderer, bg->img[p], NULL, &tileDst);
                }
                _cumX += bg->partWReal[p];
            }
            }
        }
    }

        afficherPlatforms(renderer, platforms, taille, bgX, bgY, bg->zoom);

    maxTime = 600;
    barW = (mode == MODE_MULTI) ? 200 : 300;
    barH = 16; rad = 7;
    barX = screenW / 2 - barW / 2;
    barY = 6;

    SDL_SetRenderDrawColor(renderer, 10, 18, 40, 255);
    for (y = barY; y < barY + barH; y++) {
        dy0 = (y - barY < rad)        ? rad - (y - barY)         : 0;
        dy1 = (y - barY >= barH - rad) ? rad - (barH - 1 - (y - barY)) : 0;
        dx  = (dy0 > dy1) ? dy0 : dy1;
        SDL_RenderDrawLine(renderer, barX + dx, y, barX + barW - 1 - dx, y);
    }

    filled = (timeLeft > 0) ? (timeLeft * barW / maxTime) : 0;
    if (filled > barW) filled = barW;
    if (filled > 0) {
        fr = (rad < filled) ? rad : filled;
        if (timeLeft <= 30)
            SDL_SetRenderDrawColor(renderer, 200, 40, 40, 255);
        else
            SDL_SetRenderDrawColor(renderer, 0, 210, 230, 255);
        for (y = barY; y < barY + barH; y++) {
            dy0 = (y - barY < fr)        ? fr - (y - barY)         : 0;
            dy1 = (y - barY >= barH - fr) ? fr - (barH - 1 - (y - barY)) : 0;
            dx  = (dy0 > dy1) ? dy0 : dy1;
            SDL_RenderDrawLine(renderer, barX + dx, y, barX + filled - 1 - dx, y);
        }
        SDL_SetRenderDrawColor(renderer, 160, 255, 255, 255);
        SDL_RenderDrawLine(renderer, barX + rad, barY + 2,
                           barX + filled - rad - 1, barY + 2);
    }
    SDL_SetRenderDrawColor(renderer, 0, 200, 220, 255);
    SDL_RenderDrawLine(renderer, barX + rad, barY,
                       barX + barW - rad - 1, barY);
    SDL_RenderDrawLine(renderer, barX + rad, barY + barH - 1,
                       barX + barW - rad - 1, barY + barH - 1);
    SDL_RenderDrawLine(renderer, barX, barY + rad,
                       barX, barY + barH - rad - 1);
    SDL_RenderDrawLine(renderer, barX + barW - 1, barY + rad,
                       barX + barW - 1, barY + barH - rad - 1);

    afficherTemps(renderer, font, timeLeft, screenW);

    if (mode == MODE_MONO) {
        bg->guide.position.x = screenW - bg->guide.w - 10;
        SDL_RenderCopy(renderer, bg->guide.image, NULL, &bg->guide.position);
    }

    bat[0][0]=7;  bat[0][1]=2;
    bat[1][0]=4;  bat[1][1]=4;
    bat[2][0]=2;  bat[2][1]=20;
    bat[3][0]=0;  bat[3][1]=24;
    bat[4][0]=0;  bat[4][1]=24;
    bat[5][0]=0;  bat[5][1]=24;
    bat[6][0]=1;  bat[6][1]=22;
    bat[7][0]=2;  bat[7][1]=8;
    bat[8][0]=3;  bat[8][1]=6;
    bat[9][0]=4;  bat[9][1]=4;
    bat[10][0]=5; bat[10][1]=6;
    bat[11][0]=6; bat[11][1]=5;
    bat[12][0]=8; bat[12][1]=3;
    bat[13][0]=10;bat[13][1]=1;

    bw = 24; bh = 14; spacing = 4; startX = 12; startY = 12;
    snprintf(livesStr, sizeof(livesStr), "x%d", lives);
    cyan.r = 0; cyan.g = 220; cyan.b = 255; cyan.a = 255;
    lblSurf = TTF_RenderText_Solid(font, livesStr, cyan);
    if (lblSurf) {
        lblTex = SDL_CreateTextureFromSurface(renderer, lblSurf);
        r.x = startX;
        r.y = startY + (bh - lblSurf->h) / 2;
        r.w = lblSurf->w;
        r.h = lblSurf->h;
        SDL_RenderCopy(renderer, lblTex, NULL, &r);
        SDL_DestroyTexture(lblTex);
        startX += lblSurf->w + 4;
        SDL_FreeSurface(lblSurf);
    }
    for (i = 0; i < 1; i++) {
        cx2 = startX + i * (bw + spacing);
        for (row = 0; row < bh; row++) {
            ox = bat[row][0]; rw = bat[row][1];
            if (rw <= 0) continue;
            if (i < lives) SDL_SetRenderDrawColor(renderer, 255, 210, 0, 255);
            else           SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
            SDL_RenderDrawLine(renderer, cx2 + ox, startY + row,
                               cx2 + ox + rw - 1, startY + row);
        }
    }
    (void)bw; (void)spacing;

    (void)mode;

    if (bg->afficherCommentJouer) {
        maxW = screenW - 80;
        maxH = screenH - 80;
        scaleW = (float)maxW / bg->commentJouer.w;
        scaleH = (float)maxH / bg->commentJouer.h;
        scale  = (scaleW < scaleH) ? scaleW : scaleH;
        destImg.x = (screenW - (int)(bg->commentJouer.w * scale)) / 2;
        destImg.y = (screenH - (int)(bg->commentJouer.h * scale)) / 2;
        destImg.w = (int)(bg->commentJouer.w * scale);
        destImg.h = (int)(bg->commentJouer.h * scale);
        SDL_RenderCopy(renderer, bg->commentJouer.image, NULL, &destImg);
    }
}

void saisirNomEtAfficherScore(SDL_Renderer *renderer, TTF_Font *font,
                               int score, int screenW, int screenH)
{
    char nom[64], aff[66], ligne1[80], ligne2[80];
    int len, done, showing, boxW, boxH, boxX, boxY, cx;
    SDL_Color blanc, jaune;
    SDL_Event ev;
    SDL_Surface *s;
    SDL_Texture *tex;
    SDL_Rect r;

    len = 0; done = 0; showing = 1;
    boxW = 500; boxH = 250;
    boxX = screenW / 2 - boxW / 2;

    memset(nom, 0, sizeof(nom));
    blanc.r = 255; blanc.g = 255; blanc.b = 255; blanc.a = 255;
    jaune.r = 255; jaune.g = 220; jaune.b = 0;   jaune.a = 255;

    SDL_StartTextInput();
    while (!done) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { done = 1; break; }
            if (ev.type == SDL_KEYDOWN) {
                if (ev.key.keysym.sym == SDLK_RETURN && len > 0) done = 1;
                else if (ev.key.keysym.sym == SDLK_BACKSPACE && len > 0) nom[--len] = '\0';
                else if (ev.key.keysym.sym == SDLK_ESCAPE) done = 1;
            } else if (ev.type == SDL_TEXTINPUT && len < 63) {
                strncat(nom, ev.text.text, 63 - len);
                len = (int)strlen(nom);
            }
        }
        boxY = screenH / 2 - boxH / 2;
        cx   = screenW / 2;
        SDL_SetRenderDrawColor(renderer, 0, 0, 20, 255); SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 30, 30, 80, 255);
        r.x = boxX; r.y = boxY; r.w = boxW; r.h = boxH;
        SDL_RenderFillRect(renderer, &r);
        SDL_SetRenderDrawColor(renderer, jaune.r, jaune.g, jaune.b, 255);
        SDL_RenderDrawRect(renderer, &r);

        s = TTF_RenderText_Solid(font, "Entrez votre nom :", jaune);
        if (s) { tex = SDL_CreateTextureFromSurface(renderer, s);
            r.x = cx - s->w/2; r.y = boxY + 20; r.w = s->w; r.h = s->h;
            SDL_RenderCopy(renderer, tex, NULL, &r);
            SDL_DestroyTexture(tex); SDL_FreeSurface(s); }

        snprintf(aff, sizeof(aff), "%s_", nom);
        s = TTF_RenderText_Solid(font, aff, blanc);
        if (s) { tex = SDL_CreateTextureFromSurface(renderer, s);
            r.x = cx - s->w/2; r.y = boxY + 90; r.w = s->w; r.h = s->h;
            SDL_RenderCopy(renderer, tex, NULL, &r);
            SDL_DestroyTexture(tex); SDL_FreeSurface(s); }

        s = TTF_RenderText_Solid(font, "ENTREE pour confirmer", blanc);
        if (s) { tex = SDL_CreateTextureFromSurface(renderer, s);
            r.x = cx - s->w/2; r.y = boxY + boxH - 40; r.w = s->w; r.h = s->h;
            SDL_RenderCopy(renderer, tex, NULL, &r);
            SDL_DestroyTexture(tex); SDL_FreeSurface(s); }

        SDL_RenderPresent(renderer); SDL_Delay(16);
    }
    SDL_StopTextInput();

    while (showing) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) showing = 0;
            if (ev.type == SDL_KEYDOWN) showing = 0;
        }
        boxY = screenH / 2 - boxH / 2;
        cx   = screenW / 2;
        SDL_SetRenderDrawColor(renderer, 0, 0, 20, 255); SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 30, 30, 80, 255);
        r.x = boxX; r.y = boxY; r.w = boxW; r.h = boxH;
        SDL_RenderFillRect(renderer, &r);
        SDL_SetRenderDrawColor(renderer, jaune.r, jaune.g, jaune.b, 255);
        SDL_RenderDrawRect(renderer, &r);

        snprintf(ligne1, sizeof(ligne1), "Joueur : %s", nom[0] ? nom : "Anonyme");
        s = TTF_RenderText_Solid(font, ligne1, blanc);
        if (s) { tex = SDL_CreateTextureFromSurface(renderer, s);
            r.x = cx - s->w/2; r.y = boxY + 20; r.w = s->w; r.h = s->h;
            SDL_RenderCopy(renderer, tex, NULL, &r);
            SDL_DestroyTexture(tex); SDL_FreeSurface(s); }

        snprintf(ligne2, sizeof(ligne2), "Score : %d", score);
        s = TTF_RenderText_Solid(font, ligne2, jaune);
        if (s) { tex = SDL_CreateTextureFromSurface(renderer, s);
            r.x = cx - s->w/2; r.y = boxY + 100; r.w = s->w; r.h = s->h;
            SDL_RenderCopy(renderer, tex, NULL, &r);
            SDL_DestroyTexture(tex); SDL_FreeSurface(s); }

        s = TTF_RenderText_Solid(font, "Appuyez sur une touche pour quitter", blanc);
        if (s) { tex = SDL_CreateTextureFromSurface(renderer, s);
            r.x = cx - s->w/2; r.y = boxY + boxH - 40; r.w = s->w; r.h = s->h;
            SDL_RenderCopy(renderer, tex, NULL, &r);
            SDL_DestroyTexture(tex); SDL_FreeSurface(s); }

        SDL_RenderPresent(renderer); SDL_Delay(16);
    }
}
