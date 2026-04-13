#include "back.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>

/* Helper: load PNG/WEBP/etc as SDL_Texture */
static SDL_Texture *loadTextureIMG(SDL_Renderer *renderer, const char *path) {
    SDL_Surface *surface = IMG_Load(path);
    if (!surface) {
        printf("Erreur chargement image %s : %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return tex;
}

void initBackgroundAndPlatforms(SDL_Renderer *renderer, Background *bg, Platform platforms[], int *taille) {
    (void)platforms;
    (void)taille;
    bg->posimg.x = 0;
    bg->posimg.y = 0;

    bg->img[0] = loadTextureIMG(renderer, "backg.webp");
    if (!bg->img[0]) { exit(EXIT_FAILURE); }

    bg->guide.image = loadTextureIMG(renderer, "guide.png");
    SDL_QueryTexture(bg->guide.image, NULL, NULL, &bg->guide.w, &bg->guide.h);
    bg->guide.w = 70;
    bg->guide.h = 70;
    bg->guide.position.x = 1000 - bg->guide.w - 10;
    bg->guide.position.y = 10;
    bg->guide.position.w = bg->guide.w;
    bg->guide.position.h = bg->guide.h;

    bg->commentJouer.image = loadTextureIMG(renderer, "how.png");
    SDL_QueryTexture(bg->commentJouer.image, NULL, NULL, &bg->commentJouer.w, &bg->commentJouer.h);

    bg->afficherCommentJouer = 0;

    if (!bg->guide.image || !bg->commentJouer.image) {
        printf("Erreur chargement une ou plusieurs textures.\n");
        exit(EXIT_FAILURE);
    }
}

void afficherPlatforms(SDL_Renderer *renderer, Platform platforms[], int taille, int bgX, int bgY) {
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
            }
            SDL_RenderCopy(renderer, platforms[i].frames[platforms[i].currentFrame], NULL, &dest);
        } else {
            SDL_RenderCopy(renderer, platforms[i].image, NULL, &dest);
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
            case SDLK_d: *bgX2 += scrollSpeed; break;
            case SDLK_q: *bgX2 -= scrollSpeed; break;
            case SDLK_z: *bgY2 -= scrollSpeed; break;
            case SDLK_s: *bgY2 += scrollSpeed; break;
            default: break;
        }
    }
}

void gererTemps(int *timeLeft, Uint32 *lastTime) {
    Uint32 now   = SDL_GetTicks();
    Uint32 delta = now - *lastTime;
    if (delta >= 1000) {
        *timeLeft -= (int)(delta / 1000);
        *lastTime += (delta / 1000) * 1000;
    }
}

void gererGuideEtClic(SDL_Event event, GuideButton *guide, SDL_TextureWithRect *commentJouer, int *afficherCommentJouer) {
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        int x = event.button.x;
        int y = event.button.y;
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

void afficherBackgroundEtElements(SDL_Renderer *renderer, Background *bg, Platform platforms[], int taille,
                                   TTF_Font *font, SDL_Color textColor, int timeLeft, int lives,
                                   int bgX, int bgY, int screenW, int screenH) {
    (void)textColor;

    int bgWidth, bgHeight;
    SDL_QueryTexture(bg->img[0], NULL, NULL, &bgWidth, &bgHeight);

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
                SDL_RenderCopy(renderer, bg->img[0], NULL, &dest);
            }
        }
    }

    afficherPlatforms(renderer, platforms, taille, bgX, bgY);

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
    }

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
    }
}

void saisirNomEtAfficherScore(SDL_Renderer *renderer, TTF_Font *font, int score, int screenW, int screenH) {
    char nom[64] = {0};
    int len = 0, done = 0;
    SDL_StartTextInput();

    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Color jaune = {255, 220,   0, 255};
    int boxW = 500, boxH = 250;
    int boxX = screenW / 2 - boxW / 2;

    while (!done) {
        SDL_Event ev;
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
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) showing = 0;
            if (ev.type == SDL_KEYDOWN) showing = 0;
        }
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
    }
}
