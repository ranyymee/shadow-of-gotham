/*
 * mainenigme.c  —  Remplace mainfarah.c dans le projet menu_prin
 *
 * Contient :
 *   - initMenu / freeMenu / drawTextCentre  (menu Quiz/Puzzle)
 *   - runGameMenu()  <-- appelé depuis Main.c (STATE_ENIGME)
 *
 * Assets attendus dans  enigme_menu_2/assets/  (copier dans menu_prin/assets/)
 *   background.png  qcm.png  qcmh.png  puzzlee.png  puzzleh.png
 *   batmfa.ttf  batmfa__.ttf  questions.txt
 *   bat.mp3  suspince.mp3  correct.mp3  ghalet.mp3
 *   puzzle1.png  puzzle2.png  puzzle3.png  puzzle.png
 *   carte.png  BatLogo.png
 */

#include "enigme.h"
#include "puzzle.h"
#include <stdio.h>

/* ============================================================
   Fonctions du menu Quiz / Puzzle
   ============================================================ */

static void initMenuPrinc(MenuPrincipal *m, SDL_Renderer *r, int scrW, int scrH)
{
    SDL_Surface *s;
    int bW, bH, gap, bY;

    s = IMG_Load("assets/assets/background.png");
    m->bgTexture = s ? SDL_CreateTextureFromSurface(r, s) : NULL;
    if (s) SDL_FreeSurface(s);

    s = IMG_Load("assets/assets/qcm.png");
    m->quizTex = s ? SDL_CreateTextureFromSurface(r, s) : NULL;
    if (s) SDL_FreeSurface(s);

    s = IMG_Load("assets/assets/qcmh.png");
    m->quizHoverTex = s ? SDL_CreateTextureFromSurface(r, s) : NULL;
    if (s) SDL_FreeSurface(s);

    s = IMG_Load("assets/assets/puzzlee.png");
    m->puzzleTex = s ? SDL_CreateTextureFromSurface(r, s) : NULL;
    if (s) SDL_FreeSurface(s);

    s = IMG_Load("assets/assets/puzzleh.png");
    m->puzzleHoverTex = s ? SDL_CreateTextureFromSurface(r, s) : NULL;
    if (s) SDL_FreeSurface(s);

    m->hoverQuiz = m->hoverPuzzle = 0;
    bW = scrW * 25 / 100;
    bH = bW   * 50 / 100;
    gap = scrW *  8 / 100;
    bY  = scrH * 46 / 100;
    m->quizRect   = (SDL_Rect){ scrW/2 - bW - gap/2, bY, bW, bH };
    m->puzzleRect = (SDL_Rect){ scrW/2 + gap/2,       bY, bW, bH };
}

static void freeMenuPrinc(MenuPrincipal *m)
{
    if (m->bgTexture)      { SDL_DestroyTexture(m->bgTexture);      m->bgTexture      = NULL; }
    if (m->quizTex)        { SDL_DestroyTexture(m->quizTex);        m->quizTex        = NULL; }
    if (m->quizHoverTex)   { SDL_DestroyTexture(m->quizHoverTex);   m->quizHoverTex   = NULL; }
    if (m->puzzleTex)      { SDL_DestroyTexture(m->puzzleTex);      m->puzzleTex      = NULL; }
    if (m->puzzleHoverTex) { SDL_DestroyTexture(m->puzzleHoverTex); m->puzzleHoverTex = NULL; }
}

static void drawTextCentre(SDL_Renderer *r, TTF_Font *font,
                            const char *txt, SDL_Color col, int scrW, int y)
{
    SDL_Surface *s = TTF_RenderUTF8_Blended(font, txt, col);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_Rect rc = { scrW/2 - s->w/2, y, s->w, s->h };
    if (t) { SDL_RenderCopy(r, t, NULL, &rc); SDL_DestroyTexture(t); }
    SDL_FreeSurface(s);
}

/* ============================================================
   renderScore  —  écran de résultats final
   ============================================================ */
static void renderScore(SDL_Renderer *renderer, TTF_Font *font,
                        TTF_Font *fontSmall, Enigme *e,
                        SDL_Texture *bg, int scrW, int scrH)
{
    int step = scrH / 12;
    char msg[80];
    const char *perf;
    SDL_Color cp;
    SDL_Color yellow = {255,220,0,255}, white = {255,255,255,255}, cyan = {0,200,255,255};

    if (bg) SDL_RenderCopy(renderer, bg, NULL, NULL);

    int bW = scrW*44/100, bH = scrH*50/100;
    SDL_Rect box = { scrW/2 - bW/2, scrH/2 - bH/2, bW, bH };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 220);
    SDL_RenderFillRect(renderer, &box);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);
    SDL_Rect fr = box;
    for (int i = 0; i < 3; i++) {
        SDL_RenderDrawRect(renderer, &fr);
        fr.x++; fr.y++; fr.w -= 2; fr.h -= 2;
    }

    drawTextCentre(renderer, font,      "= FINAL RESULTS =",       yellow, scrW, scrH/2 - step*2);
    snprintf(msg, sizeof(msg), "Score: %d / %d", e->score, NB_QUESTIONS);
    drawTextCentre(renderer, fontSmall, msg,                        white,  scrW, scrH/2 - step);
    snprintf(msg, sizeof(msg), "Level reached: %d", e->niveau);
    drawTextCentre(renderer, fontSmall, msg,                        cyan,   scrW, scrH/2);

    if      (e->score == NB_QUESTIONS)              { perf = "Perfect! True Batman expert!"; cp = (SDL_Color){0,255,0,255};   }
    else if (e->score >= NB_QUESTIONS * 7 / 10)     { perf = "Excellent! Well played!";      cp = (SDL_Color){0,220,100,255}; }
    else if (e->score >= NB_QUESTIONS / 2)           { perf = "Good job! Keep it up!";        cp = (SDL_Color){255,200,0,255}; }
    else                                             { perf = "Keep practicing!";              cp = (SDL_Color){255,80,80,255}; }

    drawTextCentre(renderer, fontSmall, perf,                       cp,     scrW, scrH/2 + step);
    drawTextCentre(renderer, fontSmall, "Press ENTER to play again",white,  scrW, scrH/2 + step*2);
}

/* ============================================================
   runGameMenu  —  Point d'entrée appelé depuis Main.c
                   (remplace l'ancien bloc STATE_ENIGME)
   ============================================================ */
void runGameMenu(SDL_Window *window, SDL_Renderer *renderer)
{
    int scrW, scrH;
    SDL_GetRendererOutputSize(renderer, &scrW, &scrH);

    /* --- Polices --- */
    int fontBigPt   = scrH*38/1080; if (fontBigPt   < 20) fontBigPt   = 20;
    int fontSmallPt = scrH*20/1080; if (fontSmallPt < 12) fontSmallPt = 12;
    int fontTinyPt  = scrH*14/1080; if (fontTinyPt  < 10) fontTinyPt  = 10;
    int fontTitlePt = scrH*90/1080; if (fontTitlePt < 60) fontTitlePt = 60;

    TTF_Font *font      = TTF_OpenFont("assets/assets/batmfa.ttf",   fontBigPt);
    TTF_Font *fontSmall = TTF_OpenFont("assets/assets/batmfa.ttf",   fontSmallPt);
    TTF_Font *fontTiny  = TTF_OpenFont("assets/assets/batmfa.ttf",   fontTinyPt);
    TTF_Font *fontTitle = TTF_OpenFont("assets/assets/batmfa.ttf",   fontTitlePt);
    TTF_Font *fntBig    = TTF_OpenFont("assets/assets/batmfa__.ttf", 42);
    TTF_Font *fntMid    = TTF_OpenFont("assets/assets/batmfa__.ttf", 28);
    TTF_Font *fntSm     = TTF_OpenFont("assets/assets/batmfa__.ttf", 19);
    TTF_Font *fntTiny   = TTF_OpenFont("assets/assets/batmfa__.ttf", 16);

    /* Polices de secours si les fichiers batmfa__.ttf manquent */
    if (!fntBig)  fntBig  = TTF_OpenFont("assets/assets/font/font.ttf", 42);
    if (!fntMid)  fntMid  = TTF_OpenFont("assets/assets/font/font.ttf", 28);
    if (!fntSm)   fntSm   = TTF_OpenFont("assets/assets/font/font.ttf", 19);
    if (!fntTiny) fntTiny = TTF_OpenFont("assets/assets/font/font.ttf", 16);
    if (!font)    font     = TTF_OpenFont("assets/assets/font/font.ttf", fontBigPt);
    if (!fontSmall) fontSmall = TTF_OpenFont("assets/assets/font/font.ttf", fontSmallPt);
    if (!fontTiny)  fontTiny  = TTF_OpenFont("assets/assets/font/font.ttf", fontTinyPt);
    if (!fontTitle) fontTitle = TTF_OpenFont("assets/assets/font/font.ttf", fontTitlePt);

    if (!font || !fontSmall || !fontTiny || !fontTitle ||
        !fntBig || !fntMid || !fntSm || !fntTiny) {
        fprintf(stderr, "[ENIGME] Erreur chargement polices: %s\n", TTF_GetError());
        /* on continue sans planter le jeu principal */
    }

    /* --- Structures --- */
    MenuPrincipal menu;
    Enigme        enigme;
    ContextPuzzle ctx;

    initMenuPrinc(&menu, renderer, scrW, scrH);
    initEnigme(&enigme, renderer, scrW, scrH);
    if (fntBig && fntMid && fntSm && fntTiny)
        initContextPuzzle(&ctx, renderer, fntBig, fntMid, fntSm, fntTiny);
    else
        memset(&ctx, 0, sizeof(ctx));

    int state   = STATE_MENU;
    int running = 1;
    SDL_Event event;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) { running = 0; break; }

            /* ESC → retour au menu principal du jeu */
            if (event.type == SDL_KEYDOWN && !event.key.repeat &&
                event.key.keysym.sym == SDLK_ESCAPE) { running = 0; break; }

            /* Resize */
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                SDL_GetRendererOutputSize(renderer, &scrW, &scrH);
                freeEnigme(&enigme); initEnigme(&enigme, renderer, scrW, scrH);
                freeMenuPrinc(&menu); initMenuPrinc(&menu, renderer, scrW, scrH);
            }

            /* ---- MENU SELECTION ---- */
            if (state == STATE_MENU) {
                if (event.type == SDL_MOUSEMOTION) {
                    int mx = event.motion.x, my = event.motion.y;
                    menu.hoverQuiz   = (mx >= menu.quizRect.x   && mx < menu.quizRect.x   + menu.quizRect.w   &&
                                        my >= menu.quizRect.y   && my < menu.quizRect.y   + menu.quizRect.h);
                    menu.hoverPuzzle = (mx >= menu.puzzleRect.x && mx < menu.puzzleRect.x + menu.puzzleRect.w &&
                                        my >= menu.puzzleRect.y && my < menu.puzzleRect.y + menu.puzzleRect.h);
                }
                if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                    int mx = event.button.x, my = event.button.y;
                    /* Bouton QUIZ */
                    if (mx >= menu.quizRect.x && mx < menu.quizRect.x + menu.quizRect.w &&
                        my >= menu.quizRect.y && my < menu.quizRect.y + menu.quizRect.h) {
                        freeEnigme(&enigme);
                        initEnigme(&enigme, renderer, scrW, scrH);
                        state = STATE_QUIZ;
                    }
                    /* Bouton PUZZLE */
                    else if (mx >= menu.puzzleRect.x && mx < menu.puzzleRect.x + menu.puzzleRect.w &&
                             my >= menu.puzzleRect.y && my < menu.puzzleRect.y + menu.puzzleRect.h) {
                        if (ctx.ren) {
                            int choice = runMenuPuzzle(window, &ctx);
                            if (choice >= 0) runPuzzle(window, &ctx, choice);
                            SDL_RenderSetLogicalSize(renderer, 0, 0);
                            SDL_RenderSetViewport(renderer, NULL);
                        }
                    }
                }
            }
            /* ---- QUIZ ---- */
            else if (state == STATE_QUIZ) {
                handleEnigmeEvent(&enigme, &event);
            }
            /* ---- SCORE ---- */
            else if (state == STATE_SCORE) {
                if (event.type == SDL_KEYDOWN &&
                    event.key.keysym.sym == SDLK_RETURN)
                    state = STATE_MENU;
            }
        }

        /* Update */
        if (state == STATE_QUIZ) {
            updateEnigme(&enigme);
            if (enigme.questionIndex >= NB_QUESTIONS)
                state = STATE_SCORE;
        }

        /* Render */
        SDL_SetRenderDrawColor(renderer, 10, 10, 20, 255);
        SDL_RenderClear(renderer);

        if (state == STATE_MENU) {
            int rW = scrW*72/100, rH = scrH*59/100, rY = scrH*17/100;
            SDL_Rect box = { scrW/2 - rW/2, rY, rW, rH };
            SDL_Color white = {255,255,255,255};

            if (menu.bgTexture) SDL_RenderCopy(renderer, menu.bgTexture, NULL, NULL);

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 58);
            SDL_RenderFillRect(renderer, &box);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

            if (fontTitle)
                drawTextCentre(renderer, fontTitle, "ENIGME", white, scrW, scrH*26/100);

            /* Glow sous bouton quiz survolé */
            if (menu.hoverQuiz) {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_Rect *b = &menu.quizRect;
                for (int i = 0; i < 8; i++) {
                    int gW = b->w*(80-i*8)/100;
                    SDL_Rect gr = { b->x + (b->w - gW)/2, b->y + b->h + i*3, gW, 4 };
                    SDL_SetRenderDrawColor(renderer, 255, 210, 80, (Uint8)(30 - i*3));
                    SDL_RenderFillRect(renderer, &gr);
                }
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            }

            SDL_Texture *qt = (menu.hoverQuiz   && menu.quizHoverTex)   ? menu.quizHoverTex   : menu.quizTex;
            SDL_Texture *pt = (menu.hoverPuzzle && menu.puzzleHoverTex) ? menu.puzzleHoverTex : menu.puzzleTex;
            if (qt) SDL_RenderCopy(renderer, qt, NULL, &menu.quizRect);
            if (pt) SDL_RenderCopy(renderer, pt, NULL, &menu.puzzleRect);
        }
        else if (state == STATE_QUIZ) {
            if (font && fontSmall && fontTiny)
                renderEnigme(&enigme, renderer, font, fontSmall, fontTiny);
        }
        else if (state == STATE_SCORE) {
            if (font && fontSmall)
                renderScore(renderer, font, fontSmall, &enigme, menu.bgTexture, scrW, scrH);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    /* Nettoyage */
    freeEnigme(&enigme);
    freeMenuPrinc(&menu);
    if (ctx.ren) freeContextPuzzle(&ctx);

    if (font)      TTF_CloseFont(font);
    if (fontSmall) TTF_CloseFont(fontSmall);
    if (fontTiny)  TTF_CloseFont(fontTiny);
    if (fontTitle) TTF_CloseFont(fontTitle);
    if (fntBig)    TTF_CloseFont(fntBig);
    if (fntMid)    TTF_CloseFont(fntMid);
    if (fntSm)     TTF_CloseFont(fntSm);
    if (fntTiny)   TTF_CloseFont(fntTiny);
}
