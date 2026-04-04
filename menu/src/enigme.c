#include "enigme.h"
#include <string.h>
#include <stdio.h>

#define RESULT_DELAY   2000
#define SUSPENSE_DELAY 5000

/* ============================================================
   loadTexture
   ============================================================ */
SDL_Texture *loadTexture(const char *path, SDL_Renderer *renderer)
{
    SDL_Surface *surface = IMG_Load(path);
    if (!surface) {
        fprintf(stderr, "[WARN] loadTexture: %s -> %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

/* ============================================================
   renderTextCentered
   ============================================================ */
static void renderTextCentered(SDL_Renderer *renderer, TTF_Font *font,
                                const char *text, SDL_Color color, SDL_Rect *rect)
{
    if (!font || !text) return;

    int padding = 55;
    int innerW  = rect->w - 2 * padding;
    if (innerW < 30) innerW = 30;

    SDL_Surface *surface = TTF_RenderUTF8_Blended_Wrapped(font, text, color, innerW);
    if (!surface) return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) { SDL_FreeSurface(surface); return; }

    int x = rect->x + (rect->w - surface->w) / 2;
    int y = rect->y + (rect->h - surface->h) / 2;

    if (x < rect->x + padding) x = rect->x + padding;
    if (y < rect->y + padding) y = rect->y + padding;

    SDL_RenderSetClipRect(renderer, rect);
    SDL_Rect dst = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_RenderSetClipRect(renderer, NULL);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

/* ============================================================
   Layout
   ============================================================ */
static void setupLayout(Enigme *e)
{
    int margin = 25;
    int gap    = 20;

    int qW = 430;
    int qH = SCREEN_H - 2 * margin + 10;
    e->cardRect = (SDL_Rect){ margin, margin, qW, qH };

    int p1W = 350, p1H = 450;
    int p1X = margin + qW + 50;
    int p1Y = (SCREEN_H - p1H) / 2;
    e->propRect[0] = (SDL_Rect){ p1X, p1Y, p1W, p1H };

    int p23W = 370;
    int p23H = (SCREEN_H - 2 * margin - gap) / 2 + 5;
    int p23X = p1X + p1W + 40;
    e->propRect[1] = (SDL_Rect){ p23X, margin,              p23W, p23H };
    e->propRect[2] = (SDL_Rect){ p23X, margin + p23H + gap, p23W, p23H };
}

/* ============================================================
   initEnigme
   ============================================================ */
void initEnigme(Enigme *e, SDL_Renderer *renderer)
{
    if (!e || !renderer) return;

    /* --- Questions --- */
    strncpy(e->questions[0].question,
            "Joker veut prouver quoi ?", MAX_QUESTION_LEN - 1);
    strncpy(e->questions[0].prop[0], "A) Gotham est courageuse",       MAX_PROP_LEN - 1);
    strncpy(e->questions[0].prop[1], "B) Les gens deviennent mauvais", MAX_PROP_LEN - 1);
    strncpy(e->questions[0].prop[2], "C) Batman quitte Gotham",        MAX_PROP_LEN - 1);
    e->questions[0].bonneReponse = 1;

    strncpy(e->questions[1].question,
            "Quel est le vrai nom de Batman ?", MAX_QUESTION_LEN - 1);
    strncpy(e->questions[1].prop[0], "A) Bruce Wayne",  MAX_PROP_LEN - 1);
    strncpy(e->questions[1].prop[1], "B) Clark Kent",   MAX_PROP_LEN - 1);
    strncpy(e->questions[1].prop[2], "C) Peter Parker", MAX_PROP_LEN - 1);
    e->questions[1].bonneReponse = 0;

    strncpy(e->questions[2].question,
            "Dans quelle ville vit Batman ?", MAX_QUESTION_LEN - 1);
    strncpy(e->questions[2].prop[0], "A) Metropolis", MAX_PROP_LEN - 1);
    strncpy(e->questions[2].prop[1], "B) New York",   MAX_PROP_LEN - 1);
    strncpy(e->questions[2].prop[2], "C) Gotham",     MAX_PROP_LEN - 1);
    e->questions[2].bonneReponse = 2;

    /* --- Etat initial --- */
    e->questionIndex   = 0;
    e->score           = 0;
    e->selected        = -1;
    e->hovered         = -1;
    e->waitingSuspense = 0;
    e->suspenseTime    = 0;
    e->showResult      = 0;
    e->correct         = 0;
    e->resultTime      = 0;

    /* --- Textures --- */
    e->bgTexture      = loadTexture("background.png",            renderer);
    e->cardTexture    = loadTexture("assets/proposition/question.png",  renderer);
    e->propTexture[0] = loadTexture("assets/proposition/proposition1.png", renderer);
    e->propTexture[1] = loadTexture("assets/proposition/proposition2.png", renderer);
    e->propTexture[2] = loadTexture("assets/proposition/proposition3.png", renderer);

    /* --- Sons --- */
    e->soundCorrect  = Mix_LoadMUS("assets/audio/correct.mp3");
    e->soundWrong    = Mix_LoadMUS("assets/audio/ghalet.mp3");
    e->soundSuspense = Mix_LoadMUS("assets/audio/suspince.mp3");


    setupLayout(e);

    /* --- Page choix Quiz / Puzzle --- */
    e->state       = ENIGME_STATE_CHOOSE;
    e->quizTex        = loadTexture("assets/button/quiz.png",    renderer);
    e->quizHoverTex   = loadTexture("assets/button/quizh.png",   renderer);
    e->puzzleTex      = loadTexture("assets/button/puzzle.png",  renderer);
    e->puzzleHoverTex = loadTexture("assets/button/puzzleh.png", renderer);
    e->hoverQuiz = e->hoverPuzzle = 0;
    e->quizRect   = (SDL_Rect){ SCREEN_W/2 - 430, (SCREEN_H-220)/2, 400, 220 };
    e->puzzleRect = (SDL_Rect){ SCREEN_W/2 + 30,  (SCREEN_H-220)/2, 400, 220 };
}

/* ============================================================
   updateEnigme
   ============================================================ */
void updateEnigme(Enigme *e)
{
    if (!e) return;

    /* Attente suspense -> affichage resultat */
    if (e->waitingSuspense && e->suspenseTime > 0) {
        if (SDL_GetTicks() - e->suspenseTime >= SUSPENSE_DELAY) {
            Mix_HaltMusic();
            e->waitingSuspense = 0;
            e->showResult      = 1;
            e->resultTime      = SDL_GetTicks();
            if (e->correct) {
                if (e->soundCorrect)  Mix_PlayMusic(e->soundCorrect, 1);
            } else {
                if (e->soundWrong)    Mix_PlayMusic(e->soundWrong,   1);
            }
        }
    }

    /* Affichage resultat -> question suivante */
    if (e->showResult && e->resultTime > 0) {
        if (SDL_GetTicks() - e->resultTime >= RESULT_DELAY) {
            e->questionIndex++;
            e->selected        = -1;
            e->hovered         = -1;
            e->showResult      = 0;
            e->correct         = 0;
            e->resultTime      = 0;
            e->waitingSuspense = 0;
            e->suspenseTime    = 0;
        }
    }
}

/* ============================================================
   renderChoose — page de selection Quiz / Puzzle
   ============================================================ */
static void renderChoose(Enigme *e, SDL_Renderer *renderer)
{
    if (e->bgTexture)
        SDL_RenderCopy(renderer, e->bgTexture, NULL, NULL);

    SDL_Texture *qt = (e->hoverQuiz   && e->quizHoverTex)   ? e->quizHoverTex   : e->quizTex;
    SDL_Texture *pt = (e->hoverPuzzle && e->puzzleHoverTex) ? e->puzzleHoverTex : e->puzzleTex;

    if (qt) SDL_RenderCopy(renderer, qt, NULL, &e->quizRect);
    else {
        SDL_SetRenderDrawColor(renderer, 50, 50, 200, 255);
        SDL_RenderFillRect(renderer, &e->quizRect);
    }
    if (pt) SDL_RenderCopy(renderer, pt, NULL, &e->puzzleRect);
    else {
        SDL_SetRenderDrawColor(renderer, 200, 100, 0, 255);
        SDL_RenderFillRect(renderer, &e->puzzleRect);
    }
}

/* ============================================================
   renderEnigme
   ============================================================ */
void renderEnigme(Enigme *e, SDL_Renderer *renderer,
                  TTF_Font *font, TTF_Font *fontSmall)
{
    if (!e || !renderer || !font) return;
    if (!fontSmall) fontSmall = font;

    /* Page choix */
    if (e->state == ENIGME_STATE_CHOOSE) {
        renderChoose(e, renderer);
        return;
    }

    /* Background */
    if (e->bgTexture)
        SDL_RenderCopy(renderer, e->bgTexture, NULL, NULL);

    /* Quiz termine */
    if (e->questionIndex >= NB_QUESTIONS) {
        SDL_Color blanc = {255, 255, 255, 255};
        SDL_Rect  r     = {0, 0, SCREEN_W, SCREEN_H};
        renderTextCentered(renderer, font, "Bravo ! Quiz termine !", blanc, &r);
        return;
    }

    QuestionData *q    = &e->questions[e->questionIndex];
    SDL_Color     noir = {0, 0, 0, 255};

    /* --- Carte question --- */
    if (e->cardTexture)
        SDL_RenderCopy(renderer, e->cardTexture, NULL, &e->cardRect);
    else {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 210);
        SDL_RenderFillRect(renderer, &e->cardRect);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }

    char qbuf[MAX_QUESTION_LEN + 4];
    snprintf(qbuf, sizeof(qbuf), "%s", q->question);
    renderTextCentered(renderer, font, qbuf, noir, &e->cardRect);

    /* --- Propositions --- */
    for (int i = 0; i < NB_PROPS; i++) {
        if (e->propTexture[i])
            SDL_RenderCopy(renderer, e->propTexture[i], NULL, &e->propRect[i]);
        else {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 210);
            SDL_RenderFillRect(renderer, &e->propRect[i]);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }

        /* Hover highlight */
        if (!e->showResult && e->hovered == i) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 200, 0, 100);
            SDL_RenderFillRect(renderer, &e->propRect[i]);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }

        /* Resultat : vert / rouge */
        if (e->showResult && e->selected == i) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            if (e->correct)
                SDL_SetRenderDrawColor(renderer, 0,   220, 0,   160);
            else
                SDL_SetRenderDrawColor(renderer, 220, 0,   0,   160);
            SDL_RenderFillRect(renderer, &e->propRect[i]);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }

        renderTextCentered(renderer, fontSmall, q->prop[i], noir, &e->propRect[i]);
    }

    /* --- Bandeau resultat bas d'ecran --- */
    if (e->showResult) {
        SDL_Color   col = e->correct
            ? (SDL_Color){0,   220, 0,   255}
            : (SDL_Color){220, 0,   0,   255};
        const char *msg = e->correct
            ? "Bonne reponse ! Prochaine question..."
            : "Mauvaise reponse ! Prochaine question...";

        int tw = 0, th = 0;
        TTF_SizeUTF8(font, msg, &tw, &th);
        SDL_Rect bgRect = { (SCREEN_W - tw)/2 - 15, SCREEN_H - 65, tw + 30, th + 10 };
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_RenderFillRect(renderer, &bgRect);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        SDL_Surface *s = TTF_RenderUTF8_Blended(font, msg, col);
        if (s) {
            SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_Rect dst   = { (SCREEN_W - s->w)/2, SCREEN_H - 60, s->w, s->h };
            if (t) { SDL_RenderCopy(renderer, t, NULL, &dst); SDL_DestroyTexture(t); }
            SDL_FreeSurface(s);
        }
    }
}

/* ============================================================
   handleEnigmeEvent
   ============================================================ */
void handleEnigmeEvent(Enigme *e, SDL_Event *event)
{
    if (!e || !event) return;

    /* --- Page choix Quiz / Puzzle --- */
    if (e->state == ENIGME_STATE_CHOOSE) {
        if (event->type == SDL_MOUSEMOTION) {
            int mx = event->motion.x, my = event->motion.y;
            e->hoverQuiz   = (mx >= e->quizRect.x   && mx < e->quizRect.x   + e->quizRect.w   &&
                              my >= e->quizRect.y   && my < e->quizRect.y   + e->quizRect.h);
            e->hoverPuzzle = (mx >= e->puzzleRect.x && mx < e->puzzleRect.x + e->puzzleRect.w &&
                              my >= e->puzzleRect.y && my < e->puzzleRect.y + e->puzzleRect.h);
        }
        if (event->type == SDL_MOUSEBUTTONDOWN &&
            event->button.button == SDL_BUTTON_LEFT) {
            int mx = event->button.x, my = event->button.y;
            if (mx >= e->quizRect.x && mx < e->quizRect.x + e->quizRect.w &&
                my >= e->quizRect.y && my < e->quizRect.y + e->quizRect.h) {
                e->state = ENIGME_STATE_QUIZ;   /* QUIZ choisi */
            }
            /* Puzzle : TODO — pour l'instant ignore */
        }
        return;
    }
    /* -------------------------------- */

    if (e->questionIndex >= NB_QUESTIONS) return;

    /* Hover souris */
    if (event->type == SDL_MOUSEMOTION) {
        int mx = event->motion.x, my = event->motion.y;
        e->hovered = -1;
        for (int i = 0; i < NB_PROPS; i++) {
            if (mx >= e->propRect[i].x && mx < e->propRect[i].x + e->propRect[i].w &&
                my >= e->propRect[i].y && my < e->propRect[i].y + e->propRect[i].h) {
                e->hovered = i;
                break;
            }
        }
    }

    if (e->showResult || e->waitingSuspense) return;

    QuestionData *q   = &e->questions[e->questionIndex];
    int           choix = -1;

    /* Clic souris */
    if (event->type == SDL_MOUSEBUTTONDOWN &&
        event->button.button == SDL_BUTTON_LEFT)
    {
        int mx = event->button.x, my = event->button.y;
        for (int i = 0; i < NB_PROPS; i++) {
            if (mx >= e->propRect[i].x && mx < e->propRect[i].x + e->propRect[i].w &&
                my >= e->propRect[i].y && my < e->propRect[i].y + e->propRect[i].h) {
                choix = i;
                break;
            }
        }
    }

    /* Clavier */
    if (event->type == SDL_KEYDOWN && event->key.repeat == 0) {
        switch (event->key.keysym.sym) {
            case SDLK_a: choix = 0; break;
            case SDLK_b: choix = 1; break;
            case SDLK_c: choix = 2; break;
            default: break;
        }
    }

    /* Traitement du choix */
    if (choix >= 0 && choix < NB_PROPS) {
        e->selected        = choix;
        e->correct         = (choix == q->bonneReponse);
        if (e->correct) e->score++;
        e->waitingSuspense = 1;
        e->suspenseTime    = SDL_GetTicks();
        if (e->soundSuspense) Mix_PlayMusic(e->soundSuspense, 1);
    }
}

/* ============================================================
   freeEnigme
   ============================================================ */
void freeEnigme(Enigme *e)
{
    if (!e) return;
    if (e->bgTexture)      { SDL_DestroyTexture(e->bgTexture);      e->bgTexture      = NULL; }
    if (e->cardTexture)    { SDL_DestroyTexture(e->cardTexture);    e->cardTexture    = NULL; }
    if (e->quizTex)        { SDL_DestroyTexture(e->quizTex);        e->quizTex        = NULL; }
    if (e->quizHoverTex)   { SDL_DestroyTexture(e->quizHoverTex);   e->quizHoverTex   = NULL; }
    if (e->puzzleTex)      { SDL_DestroyTexture(e->puzzleTex);      e->puzzleTex      = NULL; }
    if (e->puzzleHoverTex) { SDL_DestroyTexture(e->puzzleHoverTex); e->puzzleHoverTex = NULL; }
    for (int i = 0; i < NB_PROPS; i++)
        if (e->propTexture[i]) {
            SDL_DestroyTexture(e->propTexture[i]);
            e->propTexture[i] = NULL;
        }
    if (e->soundCorrect)  { Mix_FreeMusic(e->soundCorrect);  e->soundCorrect  = NULL; }
    if (e->soundWrong)    { Mix_FreeMusic(e->soundWrong);    e->soundWrong    = NULL; }
    if (e->soundSuspense) { Mix_FreeMusic(e->soundSuspense); e->soundSuspense = NULL; }
}
