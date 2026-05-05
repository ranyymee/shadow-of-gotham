/*
 * mainenigme.c  —  Module Enigme/Puzzle pour Shadow Of Gotham
 *
 * Expose : int runGameMenu(SDL_Window *window, SDL_Renderer *renderer)
 * Appelé depuis Main.c (case 3 / STATE_ENIGME = bouton HISTORY)
 *
 * Affiche d'abord un MENU DE CHOIX entre :
 *   [QCM]    → lance le quiz Batman (enigme.c)
 *   [PUZZLE] → lance le puzzle tactique (puzzle.c)
 *
 * Retourne le score final (>=0) ou -1 si l'utilisateur est sorti.
 *
 * Assets attendus dans assets/assets/ :
 *   background.png  qcm.png  qcmh.png  puzzlee.png  puzzleh.png
 *   batmfa.ttf  batmfa__.ttf  (ou font/font.ttf en secours)
 *   questions.txt  puzzle.png  puzzle1.png  puzzle2.png  puzzle3.png
 */

#include "enigme.h"   /* Enigme, initEnigme, renderEnigme, handleEnigmeEvent,
                         updateEnigme, freeEnigme, NB_QUESTIONS, SCR_W, SCR_H */
#include "puzzle.h"   /* ContextPuzzle, CartePuzzle, MenuPrincipal,
                         runMenuPuzzle, runPuzzle, initContextPuzzle,
                         freeContextPuzzle */
#include <string.h>
#include <stdio.h>

/* ══════════════════════════════════════════════════════════════════
   ÉTATS INTERNES (enum local, sans conflit avec GameState de game.h)
   ══════════════════════════════════════════════════════════════════ */
typedef enum {
    ES_MENU  = 0,   /* Écran de choix Quiz / Puzzle  */
    ES_QUIZ  = 1,   /* Quiz QCM en cours             */
    ES_SCORE = 2    /* Écran résultat final           */
} EState;

/* ══════════════════════════════════════════════════════════════════
   FONCTIONS DU MENU DE CHOIX
   ══════════════════════════════════════════════════════════════════ */

static void initMenuPrinc(MenuPrincipal *m, SDL_Renderer *r, int scrW, int scrH)
{
    SDL_Surface *s;

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

    /* Boutons centrés sur l'écran */
    int bW  = scrW * 25 / 100;
    int bH  = bW   * 50 / 100;
    int gap = scrW *  8 / 100;
    int bY  = scrH * 46 / 100;
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

/* Texte centré horizontalement */
static void drawTextCX(SDL_Renderer *r, TTF_Font *f,
                       const char *txt, SDL_Color col, int scrW, int y)
{
    if (!f || !txt) return;
    SDL_Surface *s = TTF_RenderUTF8_Blended(f, txt, col);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    if (t) {
        SDL_Rect rc = { scrW/2 - s->w/2, y, s->w, s->h };
        SDL_RenderCopy(r, t, NULL, &rc);
        SDL_DestroyTexture(t);
    }
    SDL_FreeSurface(s);
}

/* ══════════════════════════════════════════════════════════════════
   ÉCRAN DE RÉSULTAT QUIZ
   ══════════════════════════════════════════════════════════════════ */
static void renderScore(SDL_Renderer *renderer,
                        TTF_Font *font, TTF_Font *fontSmall,
                        Enigme *e, SDL_Texture *bg, int scrW, int scrH)
{
    if (bg) SDL_RenderCopy(renderer, bg, NULL, NULL);

    int bW = scrW * 44 / 100, bH = scrH * 50 / 100;
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

    int step = scrH / 12;
    SDL_Color yellow = {255, 220,   0, 255};
    SDL_Color white  = {255, 255, 255, 255};
    SDL_Color cyan   = {  0, 200, 255, 255};
    char msg[80];

    drawTextCX(renderer, font, "= FINAL RESULTS =", yellow, scrW, scrH/2 - step*2);

    snprintf(msg, sizeof(msg), "Score: %d / %d", e->score, NB_QUESTIONS);
    drawTextCX(renderer, fontSmall, msg, white, scrW, scrH/2 - step);

    snprintf(msg, sizeof(msg), "Level: %d", e->niveau);
    drawTextCX(renderer, fontSmall, msg, cyan, scrW, scrH/2);

    const char *perf; SDL_Color cp;
    if      (e->score == NB_QUESTIONS)              { perf = "Perfect! True Batman expert!"; cp = (SDL_Color){0,255,0,255};   }
    else if (e->score >= NB_QUESTIONS * 7 / 10)     { perf = "Excellent! Well played!";      cp = (SDL_Color){0,220,100,255}; }
    else if (e->score >= NB_QUESTIONS / 2)           { perf = "Good job! Keep it up!";        cp = (SDL_Color){255,200,0,255}; }
    else                                             { perf = "Keep practicing!";              cp = (SDL_Color){255,80,80,255}; }
    drawTextCX(renderer, fontSmall, perf,                        cp,    scrW, scrH/2 + step);
    drawTextCX(renderer, fontSmall, "Press ENTER to continue",   white, scrW, scrH/2 + step*2);
}

/* ══════════════════════════════════════════════════════════════════
   RENDU DU MENU DE CHOIX
   ══════════════════════════════════════════════════════════════════ */
static void renderMenuChoix(MenuPrincipal *m, SDL_Renderer *r,
                            TTF_Font *fontTitle, int scrW, int scrH)
{
    /* Fond */
    if (m->bgTexture)
        SDL_RenderCopy(r, m->bgTexture, NULL, NULL);

    /* Panneau semi-transparent */
    int rW = scrW * 72 / 100, rH = scrH * 59 / 100, rY = scrH * 17 / 100;
    SDL_Rect box = { scrW/2 - rW/2, rY, rW, rH };
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 255, 255, 255, 58);
    SDL_RenderFillRect(r, &box);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    /* Titre */
    SDL_Color white = {255, 255, 255, 255};
    if (fontTitle)
        drawTextCX(r, fontTitle, "ENIGME", white, scrW, scrH * 26 / 100);

    /* Glow sous bouton quiz survolé */
    if (m->hoverQuiz) {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_Rect *b = &m->quizRect;
        for (int i = 0; i < 8; i++) {
            int gW = b->w * (80 - i * 8) / 100;
            SDL_Rect gr = { b->x + (b->w - gW)/2, b->y + b->h + i*3, gW, 4 };
            SDL_SetRenderDrawColor(r, 255, 210, 80, (Uint8)(30 - i*3));
            SDL_RenderFillRect(r, &gr);
        }
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    }
    /* Glow sous bouton puzzle survolé */
    if (m->hoverPuzzle) {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_Rect *b = &m->puzzleRect;
        for (int i = 0; i < 8; i++) {
            int gW = b->w * (80 - i * 8) / 100;
            SDL_Rect gr = { b->x + (b->w - gW)/2, b->y + b->h + i*3, gW, 4 };
            SDL_SetRenderDrawColor(r, 255, 210, 80, (Uint8)(30 - i*3));
            SDL_RenderFillRect(r, &gr);
        }
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    }

    /* Boutons */
    SDL_Texture *qt = (m->hoverQuiz   && m->quizHoverTex)   ? m->quizHoverTex   : m->quizTex;
    SDL_Texture *pt = (m->hoverPuzzle && m->puzzleHoverTex) ? m->puzzleHoverTex : m->puzzleTex;
    if (qt) SDL_RenderCopy(r, qt, NULL, &m->quizRect);
    if (pt) SDL_RenderCopy(r, pt, NULL, &m->puzzleRect);
}

/* ══════════════════════════════════════════════════════════════════
   POINT D'ENTRÉE PUBLIC — appelé depuis Main.c (STATE_ENIGME)
   Retourne le score du quiz (>=0) ou -1 (sorti sans finir)
   ══════════════════════════════════════════════════════════════════ */
int runGameMenu(SDL_Window *window, SDL_Renderer *renderer)
{
    int scrW, scrH;
    SDL_GetRendererOutputSize(renderer, &scrW, &scrH);

    /* ── Polices ── */
    int bigPt   = scrH * 38 / 1080; if (bigPt   < 20) bigPt   = 20;
    int smallPt = scrH * 20 / 1080; if (smallPt < 12) smallPt = 12;
    int tinyPt  = scrH * 14 / 1080; if (tinyPt  < 10) tinyPt  = 10;
    int titlePt = scrH * 90 / 1080; if (titlePt < 60) titlePt = 60;

    /* Police principale du quiz (batmfa) */
    const char *pA  = "assets/assets/batmfa.ttf";
    const char *pB  = "assets/assets/batmfa__.ttf";
    const char *pFB = "assets/assets/font/font.ttf";  /* secours */

    TTF_Font *font      = TTF_OpenFont(pA, bigPt);
    TTF_Font *fontSmall = TTF_OpenFont(pA, smallPt);
    TTF_Font *fontTiny  = TTF_OpenFont(pA, tinyPt);
    TTF_Font *fontTitle = TTF_OpenFont(pA, titlePt);
    if (!font)      font      = TTF_OpenFont(pFB, bigPt);
    if (!fontSmall) fontSmall = TTF_OpenFont(pFB, smallPt);
    if (!fontTiny)  fontTiny  = TTF_OpenFont(pFB, tinyPt);
    if (!fontTitle) fontTitle = TTF_OpenFont(pFB, titlePt);

    /* Polices pour le moteur puzzle */
    TTF_Font *fntBig  = TTF_OpenFont(pB, 42); if (!fntBig)  fntBig  = TTF_OpenFont(pFB, 42);
    TTF_Font *fntMid  = TTF_OpenFont(pB, 28); if (!fntMid)  fntMid  = TTF_OpenFont(pFB, 28);
    TTF_Font *fntSm   = TTF_OpenFont(pB, 19); if (!fntSm)   fntSm   = TTF_OpenFont(pFB, 19);
    TTF_Font *fntTiny = TTF_OpenFont(pB, 16); if (!fntTiny) fntTiny = TTF_OpenFont(pFB, 16);

    if (!font || !fontSmall || !fontTiny || !fontTitle) {
        fprintf(stderr, "[ENIGME] Polices introuvables: %s\n", TTF_GetError());
        /* On continue quand même pour ne pas bloquer le jeu principal */
    }

    /* ── Structures ── */
    MenuPrincipal menuChoix;
    Enigme        enigme;
    ContextPuzzle ctx;

    initMenuPrinc(&menuChoix, renderer, scrW, scrH);
    initEnigme(&enigme, renderer, scrW, scrH);

    /* Initialiser le contexte puzzle seulement si les polices sont dispo */
    if (fntBig && fntMid && fntSm && fntTiny)
        initContextPuzzle(&ctx, renderer, fntBig, fntMid, fntSm, fntTiny);
    else
        memset(&ctx, 0, sizeof(ctx));   /* puzzle désactivé, pas de crash */

    /* ── Boucle principale ── */
    EState state      = ES_MENU;   /* ← IMPORTANT : commence sur le MENU de choix */
    int    running    = 1;
    int    finalScore = -1;
    SDL_Event ev;

    while (running)
    {
        /* ── Événements ── */
        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_QUIT) { running = 0; break; }

            if (ev.type == SDL_KEYDOWN && !ev.key.repeat) {
                if (ev.key.keysym.sym == SDLK_ESCAPE) {
                    if (state == ES_MENU) {
                        running = 0;          /* ESC sur menu choix → retour jeu */
                    } else {
                        /* ESC pendant quiz → retour menu choix */
                        freeEnigme(&enigme);
                        initEnigme(&enigme, renderer, scrW, scrH);
                        state = ES_MENU;
                    }
                    break;
                }
                /* ENTRÉE sur écran score → sortir et retourner le score */
                if (ev.key.keysym.sym == SDLK_RETURN && state == ES_SCORE) {
                    finalScore = enigme.score;
                    running = 0;
                }
            }

            /* Resize fenêtre */
            if (ev.type == SDL_WINDOWEVENT &&
                ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                SDL_GetRendererOutputSize(renderer, &scrW, &scrH);
                freeEnigme(&enigme);
                initEnigme(&enigme, renderer, scrW, scrH);
                freeMenuPrinc(&menuChoix);
                initMenuPrinc(&menuChoix, renderer, scrW, scrH);
            }

            /* ── Menu de choix ── */
            if (state == ES_MENU) {
                if (ev.type == SDL_MOUSEMOTION) {
                    int mx = ev.motion.x, my = ev.motion.y;
                    SDL_Rect *qr = &menuChoix.quizRect;
                    SDL_Rect *pr = &menuChoix.puzzleRect;
                    menuChoix.hoverQuiz   = (mx >= qr->x && mx < qr->x+qr->w &&
                                             my >= qr->y && my < qr->y+qr->h);
                    menuChoix.hoverPuzzle = (mx >= pr->x && mx < pr->x+pr->w &&
                                             my >= pr->y && my < pr->y+pr->h);
                }
                if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
                    int mx = ev.button.x, my = ev.button.y;
                    SDL_Rect *qr = &menuChoix.quizRect;
                    SDL_Rect *pr = &menuChoix.puzzleRect;

                    /* Clic sur QCM → lancer le quiz */
                    if (mx >= qr->x && mx < qr->x+qr->w &&
                        my >= qr->y && my < qr->y+qr->h) {
                        freeEnigme(&enigme);
                        initEnigme(&enigme, renderer, scrW, scrH);
                        state = ES_QUIZ;
                    }
                    /* Clic sur PUZZLE → lancer le puzzle */
                    else if (mx >= pr->x && mx < pr->x+pr->w &&
                             my >= pr->y && my < pr->y+pr->h) {
                        if (ctx.ren) {
                            int choice = runMenuPuzzle(window, &ctx);
                            if (choice >= 0) runPuzzle(window, &ctx, choice);
                            SDL_RenderSetViewport(renderer, NULL);
                        } else {
                            fprintf(stderr, "[ENIGME] Puzzle désactivé (polices manquantes)\n");
                        }
                    }
                }
            }
            /* ── Quiz en cours ── */
            else if (state == ES_QUIZ) {
                handleEnigmeEvent(&enigme, &ev);
            }
        }

        /* ── Mise à jour ── */
        if (state == ES_QUIZ) {
            updateEnigme(&enigme);
            if (enigme.questionIndex >= NB_QUESTIONS) {
                /* Quiz terminé → afficher résultat */
                finalScore = enigme.score;
                state = ES_SCORE;
            }
        }

        /* ── Rendu ── */
        SDL_SetRenderDrawColor(renderer, 10, 10, 20, 255);
        SDL_RenderClear(renderer);

        switch (state) {
            case ES_MENU:
                renderMenuChoix(&menuChoix, renderer, fontTitle, scrW, scrH);
                break;
            case ES_QUIZ:
                if (font && fontSmall && fontTiny)
                    renderEnigme(&enigme, renderer, font, fontSmall, fontTiny);
                break;
            case ES_SCORE:
                if (font && fontSmall)
                    renderScore(renderer, font, fontSmall, &enigme,
                                menuChoix.bgTexture, scrW, scrH);
                break;
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    /* ── Nettoyage ── */
    freeEnigme(&enigme);
    freeMenuPrinc(&menuChoix);
    if (ctx.ren) freeContextPuzzle(&ctx);

    if (font)      TTF_CloseFont(font);
    if (fontSmall) TTF_CloseFont(fontSmall);
    if (fontTiny)  TTF_CloseFont(fontTiny);
    if (fontTitle) TTF_CloseFont(fontTitle);
    if (fntBig)    TTF_CloseFont(fntBig);
    if (fntMid)    TTF_CloseFont(fntMid);
    if (fntSm)     TTF_CloseFont(fntSm);
    if (fntTiny)   TTF_CloseFont(fntTiny);

    return finalScore;
}
