#include "back.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>

<<<<<<< HEAD
/* Helper: load PNG/WEBP/etc as SDL_Texture */
static SDL_Texture *loadTextureIMG(SDL_Renderer *renderer, const char *path) {
    SDL_Surface *surface = IMG_Load(path);
=======
SDL_Texture *chargerTexture(SDL_Renderer *renderer, const char *chemin) {
    SDL_Surface *surface;
    SDL_Texture *tex;
    surface = IMG_Load(chemin);
>>>>>>> f854e34 (first commit)
    if (!surface) {
        printf("Erreur image %s : %s\n", chemin, IMG_GetError());
        return NULL;
    }
    tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return tex;
}

<<<<<<< HEAD
void initBackgroundAndPlatforms(SDL_Renderer *renderer, Background *bg, Platform platforms[], int *taille) {
    (void)platforms;
    (void)taille;
    bg->posimg.x = 0;
    bg->posimg.y = 0;

    bg->img[0] = loadTextureIMG(renderer, "backg.webp");
    if (!bg->img[0]) { exit(EXIT_FAILURE); }

    bg->guide.image = loadTextureIMG(renderer, "guide.png");
    SDL_QueryTexture(bg->guide.image, NULL, NULL, &bg->guide.w, &bg->guide.h);
=======
SDL_Texture *creerTextureCouleur(SDL_Renderer *renderer, int w, int h, Uint8 r, Uint8 g, Uint8 b) {
    SDL_Surface *surf;
    SDL_Texture *tex;
    surf = SDL_CreateRGBSurface(0, w, h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
    if (!surf) return NULL;
    SDL_FillRect(surf, NULL, SDL_MapRGB(surf->format, r, g, b));
    tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

void initBackgroundAndPlatforms(SDL_Renderer *renderer, Background *bg, Platform platforms[], int *taille, int level) {
    Platform *p;

    bg->posimg.x = 0;
    bg->posimg.y = 0;
    bg->img[0] = chargerTexture(renderer, "backg.webp");
    if (!bg->img[0]) exit(EXIT_FAILURE);

    bg->guide.image = chargerTexture(renderer, "guide.png");
    if (!bg->guide.image) { printf("Erreur guide.png\n"); exit(EXIT_FAILURE); }
>>>>>>> f854e34 (first commit)
    bg->guide.w = 70;
    bg->guide.h = 70;
    bg->guide.position.x = 1000 - bg->guide.w - 10;
    bg->guide.position.y = 10;
    bg->guide.position.w = bg->guide.w;
    bg->guide.position.h = bg->guide.h;

<<<<<<< HEAD
    bg->commentJouer.image = loadTextureIMG(renderer, "how.png");
    SDL_QueryTexture(bg->commentJouer.image, NULL, NULL, &bg->commentJouer.w, &bg->commentJouer.h);

    bg->afficherCommentJouer = 0;

    if (!bg->guide.image || !bg->commentJouer.image) {
        printf("Erreur chargement une ou plusieurs textures.\n");
        exit(EXIT_FAILURE);
=======
    bg->commentJouer.image = chargerTexture(renderer, "how.png");
    if (!bg->commentJouer.image) { printf("Erreur how.png\n"); exit(EXIT_FAILURE); }
    SDL_QueryTexture(bg->commentJouer.image, NULL, NULL, &bg->commentJouer.w, &bg->commentJouer.h);
    bg->afficherCommentJouer = 0;

    *taille = 0;

    if (level == 1) {
        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_FIXE; p->hp = -1;
        p->position.x = 100; p->position.y = 600; p->position.w = 200; p->position.h = 20;
        p->image = creerTextureCouleur(renderer, 200, 20, 130, 130, 130);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_FIXE; p->hp = -1;
        p->position.x = 380; p->position.y = 500; p->position.w = 200; p->position.h = 20;
        p->image = creerTextureCouleur(renderer, 200, 20, 130, 130, 130);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_FIXE; p->hp = -1;
        p->position.x = 650; p->position.y = 420; p->position.w = 200; p->position.h = 20;
        p->image = creerTextureCouleur(renderer, 200, 20, 130, 130, 130);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_FIXE; p->hp = -1;
        p->position.x = 200; p->position.y = 320; p->position.w = 150; p->position.h = 20;
        p->image = creerTextureCouleur(renderer, 150, 20, 130, 130, 130);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_FIXE; p->hp = -1;
        p->position.x = 520; p->position.y = 700; p->position.w = 250; p->position.h = 20;
        p->image = creerTextureCouleur(renderer, 250, 20, 130, 130, 130);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_MOBILE; p->hp = -1;
        p->position.x = 50; p->position.y = 460; p->position.w = 180; p->position.h = 20;
        p->vitesse = 3; p->moveAxis = 0; p->moveDir = 1;
        p->moveMin = 50; p->moveMax = 450;
        p->image = creerTextureCouleur(renderer, 180, 20, 0, 140, 230);
        (*taille)++;
    } else {
        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_FIXE; p->hp = -1;
        p->position.x = 50; p->position.y = 700; p->position.w = 180; p->position.h = 20;
        p->image = creerTextureCouleur(renderer, 180, 20, 100, 100, 100);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_FIXE; p->hp = -1;
        p->position.x = 300; p->position.y = 580; p->position.w = 160; p->position.h = 20;
        p->image = creerTextureCouleur(renderer, 160, 20, 100, 100, 100);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_FIXE; p->hp = -1;
        p->position.x = 600; p->position.y = 480; p->position.w = 200; p->position.h = 20;
        p->image = creerTextureCouleur(renderer, 200, 20, 100, 100, 100);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_FIXE; p->hp = -1;
        p->position.x = 830; p->position.y = 360; p->position.w = 150; p->position.h = 20;
        p->image = creerTextureCouleur(renderer, 150, 20, 100, 100, 100);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_MOBILE; p->hp = -1;
        p->position.x = 80; p->position.y = 520; p->position.w = 160; p->position.h = 20;
        p->vitesse = 4; p->moveAxis = 0; p->moveDir = 1;
        p->moveMin = 80; p->moveMax = 480;
        p->image = creerTextureCouleur(renderer, 160, 20, 0, 160, 220);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_MOBILE; p->hp = -1;
        p->position.x = 700; p->position.y = 600; p->position.w = 140; p->position.h = 20;
        p->vitesse = 3; p->moveAxis = 1; p->moveDir = 1;
        p->moveMin = 400; p->moveMax = 700;
        p->image = creerTextureCouleur(renderer, 140, 20, 0, 160, 220);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_DESTRUCTIBLE; p->hp = 3;
        p->position.x = 200; p->position.y = 440; p->position.w = 120; p->position.h = 20;
        p->image = creerTextureCouleur(renderer, 120, 20, 210, 70, 20);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_DESTRUCTIBLE; p->hp = 3;
        p->position.x = 470; p->position.y = 340; p->position.w = 120; p->position.h = 20;
        p->image = creerTextureCouleur(renderer, 120, 20, 210, 70, 20);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_DESTRUCTIBLE; p->hp = 3;
        p->position.x = 710; p->position.y = 260; p->position.w = 120; p->position.h = 20;
        p->image = creerTextureCouleur(renderer, 120, 20, 210, 70, 20);
        (*taille)++;
    }
}

void updatePlatforms(Platform platforms[], int taille) {
    int i;
    Platform *p;
    for (i = 0; i < taille; i++) {
        p = &platforms[i];
        if (p->destroyed || p->type != PLATFORM_MOBILE) continue;
        if (p->moveAxis == 0) {
            p->position.x += p->vitesse * p->moveDir;
            if (p->position.x <= p->moveMin) p->moveDir = 1;
            if (p->position.x + p->position.w >= p->moveMax) p->moveDir = -1;
        } else {
            p->position.y += p->vitesse * p->moveDir;
            if (p->position.y <= p->moveMin) p->moveDir = 1;
            if (p->position.y + p->position.h >= p->moveMax) p->moveDir = -1;
        }
>>>>>>> f854e34 (first commit)
    }
}

void afficherPlatforms(SDL_Renderer *renderer, Platform platforms[], int taille, int bgX, int bgY) {
<<<<<<< HEAD
    Uint32 currentTime = SDL_GetTicks();
    for (int i = 0; i < taille; i++) {
        SDL_Rect dest = {
            platforms[i].position.x - bgX,
            platforms[i].position.y - bgY,
            platforms[i].position.w,
            platforms[i].position.h
        };
        if (platforms[i].isAnimated) {
            if (currentTime - platforms[i].lastFrameTime > 150) {
                platforms[i].currentFrame = (platforms[i].currentFrame + 1) % MAX_FRAMES;
                platforms[i].lastFrameTime = currentTime;
=======
    int i, h;
    Platform *p;
    SDL_Rect dest;
    SDL_Rect seg;
    Uint32 now;
    int segW;
    now = SDL_GetTicks();
    for (i = 0; i < taille; i++) {
        p = &platforms[i];
        if (p->destroyed) continue;
        dest.x = p->position.x - bgX;
        dest.y = p->position.y - bgY;
        dest.w = p->position.w;
        dest.h = p->position.h;
        if (p->isAnimated) {
            if (now - p->lastFrameTime > 150) {
                p->currentFrame = (p->currentFrame + 1) % MAX_FRAMES;
                p->lastFrameTime = now;
>>>>>>> f854e34 (first commit)
            }
            SDL_RenderCopy(renderer, platforms[i].frames[platforms[i].currentFrame], NULL, &dest);
        } else {
<<<<<<< HEAD
            SDL_RenderCopy(renderer, platforms[i].image, NULL, &dest);
=======
            SDL_RenderCopy(renderer, p->image, NULL, &dest);
        }
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
>>>>>>> f854e34 (first commit)
        }
    }
}

void gererScrollingDeuxJoueurs(SDL_Event event, int *bgX1, int *bgY1, int *bgX2, int *bgY2, int scrollSpeed) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_RIGHT: *bgX1 += scrollSpeed; break;
            case SDLK_LEFT:  *bgX1 -= scrollSpeed; break;
            case SDLK_UP:    *bgY1 -= scrollSpeed; break;
            case SDLK_DOWN:  *bgY1 += scrollSpeed; break;
<<<<<<< HEAD
            case SDLK_d: *bgX2 += scrollSpeed; break;
            case SDLK_q: *bgX2 -= scrollSpeed; break;
            case SDLK_z: *bgY2 -= scrollSpeed; break;
            case SDLK_s: *bgY2 += scrollSpeed; break;
=======
            case SDLK_d:     *bgX2 += scrollSpeed; break;
            case SDLK_q:     *bgX2 -= scrollSpeed; break;
            case SDLK_z:     *bgY2 -= scrollSpeed; break;
            case SDLK_s:     *bgY2 += scrollSpeed; break;
>>>>>>> f854e34 (first commit)
            default: break;
        }
    }
}

void gererTemps(int *timeLeft, Uint32 *lastTime) {
    Uint32 now;
    Uint32 delta;
    now = SDL_GetTicks();
    delta = now - *lastTime;
    if (delta >= 1000) {
        *timeLeft -= (int)(delta / 1000);
        *lastTime += (delta / 1000) * 1000;
    }
}

<<<<<<< HEAD
void gererGuideEtClic(SDL_Event event, GuideButton *guide, SDL_TextureWithRect *commentJouer, int *afficherCommentJouer) {
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        int x = event.button.x;
        int y = event.button.y;
=======
void afficherTemps(SDL_Renderer *renderer, TTF_Font *font, int timeLeft, int screenW) {
    SDL_Color couleur;
    SDL_Surface *surf;
    SDL_Texture *tex;
    SDL_Rect dst;
    char buf[16];
    int minutes;
    int secondes;
    if (!font) return;
    minutes  = timeLeft / 60;
    secondes = timeLeft % 60;
    snprintf(buf, sizeof(buf), "%02d:%02d", minutes, secondes);
    if (timeLeft <= 30) {
        couleur.r = 255; couleur.g = 60;  couleur.b = 60;  couleur.a = 255;
    } else {
        couleur.r = 0;   couleur.g = 230; couleur.b = 255; couleur.a = 255;
    }
    surf = TTF_RenderText_Blended(font, buf, couleur);
    if (!surf) return;
    tex = SDL_CreateTextureFromSurface(renderer, surf);
    dst.x = screenW / 2 - surf->w / 2;
    dst.y = 12;
    dst.w = surf->w;
    dst.h = surf->h;
    SDL_RenderCopy(renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

void gererGuideEtClic(SDL_Event event, GuideButton *guide, SDL_TextureWithRect *commentJouer, int *afficherCommentJouer) {
    int x;
    int y;
    (void)commentJouer;
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        x = event.button.x;
        y = event.button.y;
>>>>>>> f854e34 (first commit)
        if (*afficherCommentJouer) {
            *afficherCommentJouer = 0;
        } else if (x >= guide->position.x && x <= guide->position.x + guide->w &&
                   y >= guide->position.y && y <= guide->position.y + guide->h) {
            *afficherCommentJouer = 1;
        }
    }
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_g || event.key.keysym.sym == SDLK_h)
            *afficherCommentJouer = !(*afficherCommentJouer);
        if (event.key.keysym.sym == SDLK_r || event.key.keysym.sym == SDLK_ESCAPE)
            *afficherCommentJouer = 0;
    }
    (void)commentJouer;
}

<<<<<<< HEAD
void afficherBackgroundEtElements(SDL_Renderer *renderer, Background *bg, Platform platforms[], int taille,
                                   TTF_Font *font, SDL_Color textColor, int timeLeft, int lives,
                                   int bgX, int bgY, int screenW, int screenH) {
    (void)textColor;

    int bgWidth, bgHeight;
=======
void afficherBackgroundEtElements(SDL_Renderer *renderer, Background *bg, Platform platforms[], int taille, TTF_Font *font, SDL_Color textColor, int timeLeft, int lives, int bgX, int bgY, int screenW, int screenH, int mode) {
    int bgWidth;
    int bgHeight;
    int normX;
    int normY;
    int row;
    int col;
    int maxTime;
    int barW;
    int barH;
    int rad;
    int barX;
    int barY;
    int filled;
    int fr;
    int y;
    int dy0;
    int dy1;
    int dx;
    int bat[14][2];
    int bw;
    int bh;
    int spacing;
    int startX;
    int startY;
    int i;
    int row2;
    int cx2;
    int ox;
    int rw;
    int maxW;
    int maxH;
    float scaleW;
    float scaleH;
    float scale;
    char livesStr[8];
    SDL_Color cyan;
    SDL_Color blanc;
    SDL_Surface *lblSurf;
    SDL_Texture *lblTex;
    SDL_Rect r;
    SDL_Rect dest;
    SDL_Rect destImg;
    SDL_Surface *s;
    SDL_Texture *t;
    (void)textColor;

>>>>>>> f854e34 (first commit)
    SDL_QueryTexture(bg->img[0], NULL, NULL, &bgWidth, &bgHeight);
    normX = bgX % bgWidth;
    if (normX < 0) normX += bgWidth;
    normY = bgY % bgHeight;
    if (normY < 0) normY += bgHeight;

<<<<<<< HEAD
    int normalizedX = bgX % bgWidth;
    if (normalizedX < 0) normalizedX += bgWidth;

    int normalizedY = bgY % bgHeight;
    if (normalizedY < 0) normalizedY += bgHeight;

    /* Tile in a 3x3 grid to cover all 4 scroll directions */
    for (int row = -1; row <= 1; row++) {
        for (int col = -1; col <= 1; col++) {
            SDL_Rect dest = {
                col * bgWidth  - normalizedX,
                row * bgHeight - normalizedY,
                bgWidth,
                bgHeight
            };
            /* Only render tiles visible on screen */
            if (dest.x + bgWidth  > 0 && dest.x < screenW &&
                dest.y + bgHeight > 0 && dest.y < screenH) {
=======
    for (row = -1; row <= 1; row++) {
        for (col = -1; col <= 1; col++) {
            dest.x = col * bgWidth  - normX;
            dest.y = row * bgHeight - normY;
            dest.w = bgWidth;
            dest.h = bgHeight;
            if (dest.x + bgWidth > 0 && dest.x < screenW && dest.y + bgHeight > 0 && dest.y < screenH)
>>>>>>> f854e34 (first commit)
                SDL_RenderCopy(renderer, bg->img[0], NULL, &dest);
            }
        }
    }

    afficherPlatforms(renderer, platforms, taille, bgX, bgY);

<<<<<<< HEAD
    /* Timer — progress bar arrondie cyan */
    {
        #define FILL_ROUNDED(rx, ry, rw, rh, rad, R, G, B)                         \
        do {                                                                         \
            SDL_SetRenderDrawColor(renderer, R, G, B, 255);                         \
            for (int _y = (ry); _y < (ry)+(rh); _y++) {                            \
                int _dy0 = (_y-(ry) < (rad))       ? (rad)-(_y-(ry))          : 0; \
                int _dy1 = (_y-(ry) >= (rh)-(rad)) ? (rad)-((rh)-1-(_y-(ry))): 0; \
                int _dx  = (_dy0 > _dy1) ? _dy0 : _dy1;                            \
                SDL_RenderDrawLine(renderer, (rx)+_dx, _y, (rx)+(rw)-1-_dx, _y);   \
            }                                                                        \
        } while(0)

        #define DRAW_ROUNDED_BORDER(rx, ry, rw, rh, rad, R, G, B)                  \
        do {                                                                         \
            SDL_SetRenderDrawColor(renderer, R, G, B, 255);                         \
            SDL_RenderDrawLine(renderer,(rx)+(rad),(ry),(rx)+(rw)-(rad)-1,(ry));    \
            SDL_RenderDrawLine(renderer,(rx)+(rad),(ry)+(rh)-1,(rx)+(rw)-(rad)-1,(ry)+(rh)-1); \
            SDL_RenderDrawLine(renderer,(rx),(ry)+(rad),(rx),(ry)+(rh)-(rad)-1);    \
            SDL_RenderDrawLine(renderer,(rx)+(rw)-1,(ry)+(rad),(rx)+(rw)-1,(ry)+(rh)-(rad)-1); \
            SDL_RenderDrawPoint(renderer,(rx)+(rad)-1,(ry)+1);                      \
            SDL_RenderDrawPoint(renderer,(rx)+(rw)-(rad),(ry)+1);                   \
            SDL_RenderDrawPoint(renderer,(rx)+(rad)-1,(ry)+(rh)-2);                 \
            SDL_RenderDrawPoint(renderer,(rx)+(rw)-(rad),(ry)+(rh)-2);              \
        } while(0)

        int maxTime = 600;
        int barW = 380, barH = 20;
        int barX = screenW / 2 - barW / 2, barY = 10, rad = 9;

        FILL_ROUNDED(barX, barY, barW, barH, rad, 10, 18, 40);

        int filled = (timeLeft > 0) ? (timeLeft * barW / maxTime) : 0;
        if (filled > barW) filled = barW;
        if (filled > 0) {
            int fr = rad < filled ? rad : filled;
            FILL_ROUNDED(barX, barY, filled, barH, fr, 0, 210, 230);
            SDL_SetRenderDrawColor(renderer, 160, 255, 255, 255);
            SDL_RenderDrawLine(renderer, barX+rad, barY+2, barX+filled-rad-1, barY+2);
        }
        DRAW_ROUNDED_BORDER(barX, barY, barW, barH, rad, 0, 200, 220);
        (void)font;
        #undef FILL_ROUNDED
        #undef DRAW_ROUNDED_BORDER
    }

    bg->guide.position.x = screenW - bg->guide.w - 10;
    SDL_RenderCopy(renderer, bg->guide.image, NULL, &bg->guide.position);

    /* Vies — Batman logo + x3 */
    {
        static const int bat[14][2] = {
            { 7, 2},{ 4, 4},{ 2,20},{ 0,24},{ 0,24},{ 0,24},{ 1,22},
            { 2, 8},{ 3, 6},{ 4, 4},{ 5, 6},{ 6, 5},{ 8, 3},{10, 1},
        };
        int bw = 24, bh = 14, spacing = 4, startX = 12, startY = 12;

        SDL_Color cyan = {0, 220, 255, 255};
        SDL_Surface *lblSurf = TTF_RenderText_Solid(font, "x3", cyan);
        if (lblSurf) {
            SDL_Texture *lblTex = SDL_CreateTextureFromSurface(renderer, lblSurf);
            SDL_Rect lblRect = {startX, startY + (bh - lblSurf->h) / 2, lblSurf->w, lblSurf->h};
            SDL_RenderCopy(renderer, lblTex, NULL, &lblRect);
            SDL_DestroyTexture(lblTex);
            startX += lblSurf->w + 4;
            SDL_FreeSurface(lblSurf);
        }
        for (int i = 0; i < 1; i++) {
            int cx = startX + i * (bw + spacing);
            for (int row = 0; row < bh; row++) {
                int ox = bat[row][0], rw = bat[row][1];
                if (rw <= 0) continue;
                if (i < lives) SDL_SetRenderDrawColor(renderer, 255, 210, 0, 255);
                else           SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
                SDL_RenderDrawLine(renderer, cx+ox, startY+row, cx+ox+rw-1, startY+row);
            }
        }
=======
    maxTime = 600;
    barW = (mode == MODE_MULTI) ? 260 : 380;
    barH = 20;
    rad  = 9;
    barX = screenW / 2 - barW / 2;
    barY = 10;

    SDL_SetRenderDrawColor(renderer, 10, 18, 40, 255);
    for (y = barY; y < barY + barH; y++) {
        dy0 = (y - barY < rad) ? rad - (y - barY) : 0;
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
            dy0 = (y - barY < fr) ? fr - (y - barY) : 0;
            dy1 = (y - barY >= barH - fr) ? fr - (barH - 1 - (y - barY)) : 0;
            dx  = (dy0 > dy1) ? dy0 : dy1;
            SDL_RenderDrawLine(renderer, barX + dx, y, barX + filled - 1 - dx, y);
        }
        SDL_SetRenderDrawColor(renderer, 160, 255, 255, 255);
        SDL_RenderDrawLine(renderer, barX + rad, barY + 2, barX + filled - rad - 1, barY + 2);
    }
    SDL_SetRenderDrawColor(renderer, 0, 200, 220, 255);
    SDL_RenderDrawLine(renderer, barX + rad, barY, barX + barW - rad - 1, barY);
    SDL_RenderDrawLine(renderer, barX + rad, barY + barH - 1, barX + barW - rad - 1, barY + barH - 1);
    SDL_RenderDrawLine(renderer, barX, barY + rad, barX, barY + barH - rad - 1);
    SDL_RenderDrawLine(renderer, barX + barW - 1, barY + rad, barX + barW - 1, barY + barH - rad - 1);

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
>>>>>>> f854e34 (first commit)
    }
    for (i = 0; i < 1; i++) {
        cx2 = startX + i * (bw + spacing);
        for (row2 = 0; row2 < bh; row2++) {
            ox = bat[row2][0];
            rw = bat[row2][1];
            if (rw <= 0) continue;
            if (i < lives) SDL_SetRenderDrawColor(renderer, 255, 210, 0, 255);
            else           SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
            SDL_RenderDrawLine(renderer, cx2 + ox, startY + row2, cx2 + ox + rw - 1, startY + row2);
        }
    }
    (void)bw; (void)spacing;

<<<<<<< HEAD
    if (bg->afficherCommentJouer) {
        SDL_Rect dest;
        int maxW = screenW - 80;
        int maxH = screenH - 80;
        float scaleW = (float)maxW / bg->commentJouer.w;
        float scaleH = (float)maxH / bg->commentJouer.h;
        float scale  = scaleW < scaleH ? scaleW : scaleH;
        dest.w = (int)(bg->commentJouer.w * scale);
        dest.h = (int)(bg->commentJouer.h * scale);
        dest.x = (screenW - dest.w) / 2;
        dest.y = (screenH - dest.h) / 2;
        SDL_RenderCopy(renderer, bg->commentJouer.image, NULL, &dest);
=======
    if (mode == MODE_MULTI && font) {
        blanc.r = 255; blanc.g = 255; blanc.b = 255; blanc.a = 200;
        s = TTF_RenderText_Solid(font, "CAM", blanc);
        if (s) {
            t = SDL_CreateTextureFromSurface(renderer, s);
            r.x = screenW - s->w - 6;
            r.y = screenH - s->h - 6;
            r.w = s->w;
            r.h = s->h;
            SDL_RenderCopy(renderer, t, NULL, &r);
            SDL_DestroyTexture(t);
            SDL_FreeSurface(s);
        }
    }

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
>>>>>>> f854e34 (first commit)
    }
}

void saisirNomEtAfficherScore(SDL_Renderer *renderer, TTF_Font *font, int score, int screenW, int screenH) {
<<<<<<< HEAD
    char nom[64] = {0};
    int len = 0, done = 0;
    SDL_StartTextInput();

    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Color jaune = {255, 220,   0, 255};
    int boxW = 500, boxH = 250;
    int boxX = screenW / 2 - boxW / 2;

=======
    char nom[64];
    char aff[66];
    char l1[80];
    char l2[80];
    int len;
    int done;
    int showing;
    int boxW;
    int boxH;
    int boxX;
    int boxY;
    int cx;
    SDL_Color blanc;
    SDL_Color jaune;
    SDL_Event ev;
    SDL_Surface *s;
    SDL_Texture *tex;
    SDL_Rect r;

    memset(nom, 0, sizeof(nom));
    len = 0;
    done = 0;
    blanc.r = 255; blanc.g = 255; blanc.b = 255; blanc.a = 255;
    jaune.r = 255; jaune.g = 220; jaune.b = 0;   jaune.a = 255;
    boxW = 500; boxH = 250;
    boxX = screenW / 2 - boxW / 2;

    SDL_StartTextInput();
>>>>>>> f854e34 (first commit)
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
<<<<<<< HEAD
        int boxY = screenH/2 - boxH/2, cx = screenW/2;
        SDL_SetRenderDrawColor(renderer, 0, 0, 20, 255); SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 30, 30, 80, 255);
        SDL_RenderFillRect(renderer, &(SDL_Rect){boxX, boxY, boxW, boxH});
        SDL_SetRenderDrawColor(renderer, jaune.r, jaune.g, jaune.b, 255);
        SDL_RenderDrawRect(renderer, &(SDL_Rect){boxX, boxY, boxW, boxH});

        SDL_Surface *s = TTF_RenderText_Solid(font, "Entrez votre nom :", jaune);
        if (s) { SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_RenderCopy(renderer, t, NULL, &(SDL_Rect){cx-s->w/2, boxY+20, s->w, s->h});
            SDL_DestroyTexture(t); SDL_FreeSurface(s); }

        char affiche[66]; snprintf(affiche, sizeof(affiche), "%s_", nom);
        s = TTF_RenderText_Solid(font, affiche, blanc);
        if (s) { SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_RenderCopy(renderer, t, NULL, &(SDL_Rect){cx-s->w/2, boxY+90, s->w, s->h});
            SDL_DestroyTexture(t); SDL_FreeSurface(s); }

        s = TTF_RenderText_Solid(font, "ENTREE pour confirmer", blanc);
        if (s) { SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_RenderCopy(renderer, t, NULL, &(SDL_Rect){cx-s->w/2, boxY+boxH-40, s->w, s->h});
            SDL_DestroyTexture(t); SDL_FreeSurface(s); }

        SDL_RenderPresent(renderer); SDL_Delay(16);
    }
    SDL_StopTextInput();

    int showing = 1;
    while (showing) {
        SDL_Event ev;
=======
        boxY = screenH / 2 - boxH / 2;
        cx = screenW / 2;
        SDL_SetRenderDrawColor(renderer, 0, 0, 20, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 30, 30, 80, 255);
        r.x = boxX; r.y = boxY; r.w = boxW; r.h = boxH;
        SDL_RenderFillRect(renderer, &r);
        SDL_SetRenderDrawColor(renderer, jaune.r, jaune.g, jaune.b, 255);
        SDL_RenderDrawRect(renderer, &r);
        s = TTF_RenderText_Solid(font, "Entrez votre nom :", jaune);
        if (s) {
            tex = SDL_CreateTextureFromSurface(renderer, s);
            r.x = cx - s->w / 2; r.y = boxY + 20; r.w = s->w; r.h = s->h;
            SDL_RenderCopy(renderer, tex, NULL, &r);
            SDL_DestroyTexture(tex); SDL_FreeSurface(s);
        }
        snprintf(aff, sizeof(aff), "%s_", nom);
        s = TTF_RenderText_Solid(font, aff, blanc);
        if (s) {
            tex = SDL_CreateTextureFromSurface(renderer, s);
            r.x = cx - s->w / 2; r.y = boxY + 90; r.w = s->w; r.h = s->h;
            SDL_RenderCopy(renderer, tex, NULL, &r);
            SDL_DestroyTexture(tex); SDL_FreeSurface(s);
        }
        s = TTF_RenderText_Solid(font, "ENTREE pour confirmer", blanc);
        if (s) {
            tex = SDL_CreateTextureFromSurface(renderer, s);
            r.x = cx - s->w / 2; r.y = boxY + boxH - 40; r.w = s->w; r.h = s->h;
            SDL_RenderCopy(renderer, tex, NULL, &r);
            SDL_DestroyTexture(tex); SDL_FreeSurface(s);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    SDL_StopTextInput();

    showing = 1;
    while (showing) {
>>>>>>> f854e34 (first commit)
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) showing = 0;
            if (ev.type == SDL_KEYDOWN) showing = 0;
        }
<<<<<<< HEAD
        int boxY = screenH/2 - boxH/2, cx = screenW/2;
        SDL_SetRenderDrawColor(renderer, 0, 0, 20, 255); SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 30, 30, 80, 255);
        SDL_RenderFillRect(renderer, &(SDL_Rect){boxX, boxY, boxW, boxH});
        SDL_SetRenderDrawColor(renderer, jaune.r, jaune.g, jaune.b, 255);
        SDL_RenderDrawRect(renderer, &(SDL_Rect){boxX, boxY, boxW, boxH});

        char ligne1[80]; snprintf(ligne1, sizeof(ligne1), "Joueur : %s", nom[0] ? nom : "Anonyme");
        SDL_Surface *s = TTF_RenderText_Solid(font, ligne1, blanc);
        if (s) { SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_RenderCopy(renderer, t, NULL, &(SDL_Rect){cx-s->w/2, boxY+20, s->w, s->h});
            SDL_DestroyTexture(t); SDL_FreeSurface(s); }

        char ligne2[80]; snprintf(ligne2, sizeof(ligne2), "Score : %d", score);
        s = TTF_RenderText_Solid(font, ligne2, jaune);
        if (s) { SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_RenderCopy(renderer, t, NULL, &(SDL_Rect){cx-s->w/2, boxY+100, s->w, s->h});
            SDL_DestroyTexture(t); SDL_FreeSurface(s); }

        s = TTF_RenderText_Solid(font, "Appuyez sur une touche pour quitter", blanc);
        if (s) { SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_RenderCopy(renderer, t, NULL, &(SDL_Rect){cx-s->w/2, boxY+boxH-40, s->w, s->h});
            SDL_DestroyTexture(t); SDL_FreeSurface(s); }

        SDL_RenderPresent(renderer); SDL_Delay(16);
=======
        boxY = screenH / 2 - boxH / 2;
        cx = screenW / 2;
        SDL_SetRenderDrawColor(renderer, 0, 0, 20, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 30, 30, 80, 255);
        r.x = boxX; r.y = boxY; r.w = boxW; r.h = boxH;
        SDL_RenderFillRect(renderer, &r);
        SDL_SetRenderDrawColor(renderer, jaune.r, jaune.g, jaune.b, 255);
        SDL_RenderDrawRect(renderer, &r);
        snprintf(l1, sizeof(l1), "Joueur : %s", nom[0] ? nom : "Anonyme");
        s = TTF_RenderText_Solid(font, l1, blanc);
        if (s) {
            tex = SDL_CreateTextureFromSurface(renderer, s);
            r.x = cx - s->w / 2; r.y = boxY + 20; r.w = s->w; r.h = s->h;
            SDL_RenderCopy(renderer, tex, NULL, &r);
            SDL_DestroyTexture(tex); SDL_FreeSurface(s);
        }
        snprintf(l2, sizeof(l2), "Score : %d", score);
        s = TTF_RenderText_Solid(font, l2, jaune);
        if (s) {
            tex = SDL_CreateTextureFromSurface(renderer, s);
            r.x = cx - s->w / 2; r.y = boxY + 100; r.w = s->w; r.h = s->h;
            SDL_RenderCopy(renderer, tex, NULL, &r);
            SDL_DestroyTexture(tex); SDL_FreeSurface(s);
        }
        s = TTF_RenderText_Solid(font, "Appuyez sur une touche pour quitter", blanc);
        if (s) {
            tex = SDL_CreateTextureFromSurface(renderer, s);
            r.x = cx - s->w / 2; r.y = boxY + boxH - 40; r.w = s->w; r.h = s->h;
            SDL_RenderCopy(renderer, tex, NULL, &r);
            SDL_DestroyTexture(tex); SDL_FreeSurface(s);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
>>>>>>> f854e34 (first commit)
    }
}
