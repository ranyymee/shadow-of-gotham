#include "../include/back.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>

/* ── Charger une texture depuis un fichier image ── */
static SDL_Texture *chargerTexture(SDL_Renderer *renderer, const char *chemin)
{
    SDL_Surface *surface;
    SDL_Texture *tex;
    surface = IMG_Load(chemin);
    if (!surface) {
        printf("Erreur image %s : %s\n", chemin, IMG_GetError());
        return NULL;
    }
    tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return tex;
}

/* ── Creer une texture de couleur unie ── */
static SDL_Texture *creerTextureCouleur(SDL_Renderer *renderer,
                                         int w, int h,
                                         Uint8 r, Uint8 g, Uint8 b)
{
    SDL_Surface *surf;
    SDL_Texture *tex;
    surf = SDL_CreateRGBSurface(0, w, h, 32,
                                0xFF000000, 0x00FF0000,
                                0x0000FF00, 0x000000FF);
    if (!surf) return NULL;
    SDL_FillRect(surf, NULL, SDL_MapRGB(surf->format, r, g, b));
    tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

/* ════════════════════════════════════════════════════════════════════
   initBackgroundAndPlatforms
════════════════════════════════════════════════════════════════════ */
void initBackgroundAndPlatforms(SDL_Renderer *renderer, Background *bg,
                                 Platform platforms[], int *taille, int level)
{
    Platform *p;

    bg->posimg.x = 0;
    bg->posimg.y = 0;
    bg->img[0]   = chargerTexture(renderer, "assets/back.png");
    if (!bg->img[0]) { printf("Erreur back.png\n"); exit(EXIT_FAILURE); }

    bg->guide.image = chargerTexture(renderer, "assets/guide.png");
    if (!bg->guide.image) { printf("Erreur guide.png\n"); exit(EXIT_FAILURE); }
    bg->guide.w = 55;
    bg->guide.h = 55;
    bg->guide.position.x = 1000 - bg->guide.w - 10;
    bg->guide.position.y = 8;
    bg->guide.position.w = bg->guide.w;
    bg->guide.position.h = bg->guide.h;

    bg->commentJouer.image = chargerTexture(renderer, "assets/how.png");
    if (!bg->commentJouer.image) { printf("Erreur how.png\n"); exit(EXIT_FAILURE); }
    SDL_QueryTexture(bg->commentJouer.image, NULL, NULL,
                     &bg->commentJouer.w, &bg->commentJouer.h);
    bg->afficherCommentJouer = 0;

    *taille = 0;

    if (level == 1) {
        /* ── Level 1 — adapte a SCREEN_H=427, GROUND_Y=267 ── */
        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_FIXE; p->hp = -1;
        p->position.x = 80;  p->position.y = 220;
        p->position.w = 200; p->position.h = 15;
        p->image = creerTextureCouleur(renderer, 200, 15, 130, 130, 130);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_FIXE; p->hp = -1;
        p->position.x = 380; p->position.y = 180;
        p->position.w = 200; p->position.h = 15;
        p->image = creerTextureCouleur(renderer, 200, 15, 130, 130, 130);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_FIXE; p->hp = -1;
        p->position.x = 650; p->position.y = 150;
        p->position.w = 200; p->position.h = 15;
        p->image = creerTextureCouleur(renderer, 200, 15, 130, 130, 130);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_FIXE; p->hp = -1;
        p->position.x = 200; p->position.y = 110;
        p->position.w = 150; p->position.h = 15;
        p->image = creerTextureCouleur(renderer, 150, 15, 130, 130, 130);
        (*taille)++;

        /* Plateforme mobile */
        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_MOBILE; p->hp = -1;
        p->position.x = 50;  p->position.y = 190;
        p->position.w = 180; p->position.h = 15;
        p->vitesse = 3; p->moveAxis = 0; p->moveDir = 1;
        p->moveMin = 50; p->moveMax = 450;
        p->image = creerTextureCouleur(renderer, 180, 15, 0, 140, 230);
        (*taille)++;

    } else {
        /* ── Level 2 ── */
        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_FIXE; p->hp = -1;
        p->position.x = 50;  p->position.y = 240;
        p->position.w = 180; p->position.h = 15;
        p->image = creerTextureCouleur(renderer, 180, 15, 100, 100, 100);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_FIXE; p->hp = -1;
        p->position.x = 300; p->position.y = 190;
        p->position.w = 160; p->position.h = 15;
        p->image = creerTextureCouleur(renderer, 160, 15, 100, 100, 100);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_FIXE; p->hp = -1;
        p->position.x = 600; p->position.y = 160;
        p->position.w = 200; p->position.h = 15;
        p->image = creerTextureCouleur(renderer, 200, 15, 100, 100, 100);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_FIXE; p->hp = -1;
        p->position.x = 830; p->position.y = 120;
        p->position.w = 150; p->position.h = 15;
        p->image = creerTextureCouleur(renderer, 150, 15, 100, 100, 100);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_MOBILE; p->hp = -1;
        p->position.x = 80;  p->position.y = 200;
        p->position.w = 160; p->position.h = 15;
        p->vitesse = 4; p->moveAxis = 0; p->moveDir = 1;
        p->moveMin = 80; p->moveMax = 480;
        p->image = creerTextureCouleur(renderer, 160, 15, 0, 160, 220);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_MOBILE; p->hp = -1;
        p->position.x = 700; p->position.y = 220;
        p->position.w = 140; p->position.h = 15;
        p->vitesse = 3; p->moveAxis = 1; p->moveDir = 1;
        p->moveMin = 120; p->moveMax = 280;
        p->image = creerTextureCouleur(renderer, 140, 15, 0, 160, 220);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_DESTRUCTIBLE; p->hp = 3;
        p->position.x = 200; p->position.y = 150;
        p->position.w = 120; p->position.h = 15;
        p->image = creerTextureCouleur(renderer, 120, 15, 210, 70, 20);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_DESTRUCTIBLE; p->hp = 3;
        p->position.x = 470; p->position.y = 120;
        p->position.w = 120; p->position.h = 15;
        p->image = creerTextureCouleur(renderer, 120, 15, 210, 70, 20);
        (*taille)++;

        p = &platforms[*taille]; memset(p, 0, sizeof(Platform));
        p->type = PLATFORM_DESTRUCTIBLE; p->hp = 3;
        p->position.x = 710; p->position.y = 90;
        p->position.w = 120; p->position.h = 15;
        p->image = creerTextureCouleur(renderer, 120, 15, 210, 70, 20);
        (*taille)++;
    }
}

/* ════════════════════════════════════════════════════════════════════
   updatePlatforms — deplace les plateformes mobiles
════════════════════════════════════════════════════════════════════ */
void updatePlatforms(Platform platforms[], int taille)
{
    int i;
    Platform *p;
    for (i = 0; i < taille; i++) {
        p = &platforms[i];
        if (p->destroyed || p->type != PLATFORM_MOBILE) continue;
        if (p->moveAxis == 0) {
            p->position.x += p->vitesse * p->moveDir;
            if (p->position.x <= p->moveMin)                      p->moveDir =  1;
            if (p->position.x + p->position.w >= p->moveMax)      p->moveDir = -1;
        } else {
            p->position.y += p->vitesse * p->moveDir;
            if (p->position.y <= p->moveMin)                      p->moveDir =  1;
            if (p->position.y + p->position.h >= p->moveMax)      p->moveDir = -1;
        }
    }
}

/* ════════════════════════════════════════════════════════════════════
   afficherPlatforms
════════════════════════════════════════════════════════════════════ */
void afficherPlatforms(SDL_Renderer *renderer, Platform platforms[],
                        int taille, int bgX, int bgY)
{
    int i, h, segW;
    Platform *p;
    SDL_Rect dest;
    SDL_Rect seg;
    Uint32 now = SDL_GetTicks();

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
            }
            SDL_RenderCopy(renderer, p->frames[p->currentFrame], NULL, &dest);
        } else {
            SDL_RenderCopy(renderer, p->image, NULL, &dest);
        }

        /* Barre HP pour les plateformes destructibles */
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
    }
}

/* ════════════════════════════════════════════════════════════════════
   gererScrollingDeuxJoueurs
════════════════════════════════════════════════════════════════════ */
void gererScrollingDeuxJoueurs(SDL_Event event,
                                int *bgX1, int *bgY1,
                                int *bgX2, int *bgY2, int scrollSpeed)
{
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_RIGHT: *bgX1 += scrollSpeed; break;
            case SDLK_LEFT:  *bgX1 -= scrollSpeed; break;
            case SDLK_UP:    *bgY1 -= scrollSpeed; break;
            case SDLK_DOWN:  *bgY1 += scrollSpeed; break;
            case SDLK_d:     *bgX2 += scrollSpeed; break;
            case SDLK_q:     *bgX2 -= scrollSpeed; break;
            case SDLK_z:     *bgY2 -= scrollSpeed; break;
            case SDLK_s:     *bgY2 += scrollSpeed; break;
            default: break;
        }
    }
}

/* ════════════════════════════════════════════════════════════════════
   gererTemps
════════════════════════════════════════════════════════════════════ */
void gererTemps(int *timeLeft, Uint32 *lastTime)
{
    Uint32 now   = SDL_GetTicks();
    Uint32 delta = now - *lastTime;
    if (delta >= 1000) {
        *timeLeft -= (int)(delta / 1000);
        *lastTime += (delta / 1000) * 1000;
    }
}

/* ════════════════════════════════════════════════════════════════════
   afficherTemps — format MM:SS centre en haut
════════════════════════════════════════════════════════════════════ */
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

/* ════════════════════════════════════════════════════════════════════
   gererGuideEtClic
════════════════════════════════════════════════════════════════════ */
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
        if (event.key.keysym.sym == SDLK_g || event.key.keysym.sym == SDLK_h)
            *afficherCommentJouer = !(*afficherCommentJouer);
        if (event.key.keysym.sym == SDLK_r || event.key.keysym.sym == SDLK_ESCAPE)
            *afficherCommentJouer = 0;
    }
}

/* ════════════════════════════════════════════════════════════════════
   afficherBackgroundEtElements
════════════════════════════════════════════════════════════════════ */
void afficherBackgroundEtElements(SDL_Renderer *renderer, Background *bg,
                                   Platform platforms[], int taille,
                                   TTF_Font *font, SDL_Color textColor,
                                   int timeLeft, int lives,
                                   int bgX, int bgY,
                                   int screenW, int screenH, int mode)
{
    int bgWidth, bgHeight;
    int normX, normY;
    int row, col;
    int maxTime, barW, barH, rad, barX, barY;
    int filled, fr, y, dy0, dy1, dx;
    int bat[14][2];
    int bw, bh, spacing, startX, startY, i, cx2, ox, rw;
    int maxW, maxH;
    float scaleW, scaleH, scale;
    char livesStr[8];
    SDL_Color cyan, blanc;
    SDL_Surface *lblSurf;
    SDL_Texture *lblTex;
    SDL_Rect r, dest, destImg;
    SDL_Surface *s;
    SDL_Texture *t;
    (void)textColor;

    SDL_QueryTexture(bg->img[0], NULL, NULL, &bgWidth, &bgHeight);
    normX = bgX % bgWidth;  if (normX < 0) normX += bgWidth;
    normY = bgY % bgHeight; if (normY < 0) normY += bgHeight;

    /* ── Tiling background ── */
    for (row = -1; row <= 1; row++) {
        for (col = -1; col <= 1; col++) {
            dest.x = col * bgWidth  - normX;
            dest.y = row * bgHeight - normY;
            dest.w = bgWidth;
            dest.h = bgHeight;
            if (dest.x + bgWidth > 0 && dest.x < screenW &&
                dest.y + bgHeight > 0 && dest.y < screenH)
                SDL_RenderCopy(renderer, bg->img[0], NULL, &dest);
        }
    }

    afficherPlatforms(renderer, platforms, taille, bgX, bgY);

    /* ── Barre de temps ── */
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

    /* ── Bouton guide (mode solo seulement) ── */
    if (mode == MODE_MONO) {
        bg->guide.position.x = screenW - bg->guide.w - 10;
        SDL_RenderCopy(renderer, bg->guide.image, NULL, &bg->guide.position);
    }

    /* ── Vies (logo Batman dessiné pixel par pixel) ── */
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

    /* ── Label CAM en mode multi ── */
    if (mode == MODE_MULTI && font) {
        blanc.r = 255; blanc.g = 255; blanc.b = 255; blanc.a = 200;
        s = TTF_RenderText_Solid(font, "CAM", blanc);
        if (s) {
            t = SDL_CreateTextureFromSurface(renderer, s);
            r.x = screenW - s->w - 6;
            r.y = screenH - s->h - 6;
            r.w = s->w; r.h = s->h;
            SDL_RenderCopy(renderer, t, NULL, &r);
            SDL_DestroyTexture(t);
            SDL_FreeSurface(s);
        }
    }

    /* ── Fenetre guide/comment jouer ── */
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

/* ════════════════════════════════════════════════════════════════════
   saisirNomEtAfficherScore
════════════════════════════════════════════════════════════════════ */
void saisirNomEtAfficherScore(SDL_Renderer *renderer, TTF_Font *font,
                               int score, int screenW, int screenH)
{
    char nom[64], aff[66], l1[80], l2[80];
    int len = 0, done = 0, showing = 1;
    int boxW = 500, boxH = 250;
    int boxX = screenW / 2 - boxW / 2;
    int boxY, cx;
    SDL_Color blanc, jaune;
    SDL_Event ev;
    SDL_Surface *s;
    SDL_Texture *tex;
    SDL_Rect r;

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

        snprintf(l1, sizeof(l1), "Joueur : %s", nom[0] ? nom : "Anonyme");
        s = TTF_RenderText_Solid(font, l1, blanc);
        if (s) { tex = SDL_CreateTextureFromSurface(renderer, s);
            r.x = cx - s->w/2; r.y = boxY + 20; r.w = s->w; r.h = s->h;
            SDL_RenderCopy(renderer, tex, NULL, &r);
            SDL_DestroyTexture(tex); SDL_FreeSurface(s); }

        snprintf(l2, sizeof(l2), "Score : %d", score);
        s = TTF_RenderText_Solid(font, l2, jaune);
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
