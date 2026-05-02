#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <string.h>
#include "enigme.h"

typedef enum { STATE_MENU, STATE_QUIZ, STATE_SCORE } GameState;

typedef struct {
    SDL_Texture *bgTexture;
    SDL_Texture *quizTex,   *quizHoverTex;
    SDL_Texture *puzzleTex, *puzzleHoverTex;
    SDL_Rect     quizRect,   puzzleRect;
    int          hoverQuiz,  hoverPuzzle;
} Menu;

static SDL_Texture *loadTex(const char *path, SDL_Renderer *r)
{
    SDL_Surface *s = IMG_Load(path);
    if (!s) { fprintf(stderr, "[WARN] %s: %s\n", path, IMG_GetError()); return NULL; }
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_FreeSurface(s);
    return t;
}

static void initMenu(Menu *m, SDL_Renderer *renderer)
{
    m->bgTexture      = loadTex("background.png",       renderer);
    m->quizTex        = loadTex("quiz.png",        renderer);
    m->quizHoverTex   = loadTex("quizh.png",       renderer);
    m->puzzleTex      = loadTex("puzzle.png",      renderer);
    m->puzzleHoverTex = loadTex("puzzleh.png",     renderer);
    m->hoverQuiz = m->hoverPuzzle = 0;
    m->quizRect   = (SDL_Rect){ SCREEN_W/2 - 430, (SCREEN_H-220)/2, 400, 220 };
    m->puzzleRect = (SDL_Rect){ SCREEN_W/2 + 30,  (SCREEN_H-220)/2, 400, 220 };
}

static void renderMenuBg(Menu *m, SDL_Renderer *r)
{
    if (m->bgTexture)
        SDL_RenderCopy(r, m->bgTexture, NULL, NULL);
}

static void renderMenu(Menu *m, SDL_Renderer *r)
{
    renderMenuBg(m, r);

    SDL_Texture *qt = (m->hoverQuiz   && m->quizHoverTex)   ? m->quizHoverTex   : m->quizTex;
    SDL_Texture *pt = (m->hoverPuzzle && m->puzzleHoverTex) ? m->puzzleHoverTex : m->puzzleTex;

    if (qt) SDL_RenderCopy(r, qt, NULL, &m->quizRect);
    else { SDL_SetRenderDrawColor(r, 50, 50, 200, 255); SDL_RenderFillRect(r, &m->quizRect); }

    if (pt) SDL_RenderCopy(r, pt, NULL, &m->puzzleRect);
    else { SDL_SetRenderDrawColor(r, 200, 100, 0, 255); SDL_RenderFillRect(r, &m->puzzleRect); }
}

static int pointInRect(int x, int y, SDL_Rect *rect)
{
    return x >= rect->x && x < rect->x + rect->w &&
           y >= rect->y && y < rect->y + rect->h;
}

static void freeMenu(Menu *m)
{
    if (m->bgTexture)      { SDL_DestroyTexture(m->bgTexture);      m->bgTexture      = NULL; }
    if (m->quizTex)        { SDL_DestroyTexture(m->quizTex);        m->quizTex        = NULL; }
    if (m->quizHoverTex)   { SDL_DestroyTexture(m->quizHoverTex);   m->quizHoverTex   = NULL; }
    if (m->puzzleTex)      { SDL_DestroyTexture(m->puzzleTex);      m->puzzleTex      = NULL; }
    if (m->puzzleHoverTex) { SDL_DestroyTexture(m->puzzleHoverTex); m->puzzleHoverTex = NULL; }
}

static void renderScore(SDL_Renderer *r, TTF_Font *font, Enigme *e, SDL_Texture *bg)
{
    if (bg) SDL_RenderCopy(r, bg, NULL, NULL);

    SDL_Rect box = { SCREEN_W/2-320, SCREEN_H/2-160, 640, 320 };
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 210);
    SDL_RenderFillRect(r, &box);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    SDL_Color jaune = {255,220,0,255}, blanc = {255,255,255,255};

    /* Fonction locale pour afficher du texte centre */
    #define DRAW_TEXT(txt, col, ypos) do { \
        SDL_Surface *_s = TTF_RenderUTF8_Blended(font, txt, col); \
        if (_s) { \
            SDL_Texture *_t = SDL_CreateTextureFromSurface(r, _s); \
            SDL_Rect _rc = { SCREEN_W/2 - _s->w/2, ypos, _s->w, _s->h }; \
            if (_t) { SDL_RenderCopy(r, _t, NULL, &_rc); SDL_DestroyTexture(_t); } \
            SDL_FreeSurface(_s); \
        } \
    } while(0)

    DRAW_TEXT("Quiz termine !", jaune, SCREEN_H/2 - 140);

    char msg[64];
    snprintf(msg, sizeof(msg), "Score : %d / %d", e->score, NB_QUESTIONS);
    DRAW_TEXT(msg, blanc, SCREEN_H/2 - 50);

    const char *perf;
    SDL_Color col;
    if (e->score == NB_QUESTIONS)
        { perf = "Parfait ! Tu es un expert Batman !";    col = (SDL_Color){0,255,0,255}; }
    else if (e->score >= NB_QUESTIONS/2)
        { perf = "Bien joue ! Continue comme ca !";      col = (SDL_Color){255,200,0,255}; }
    else
        { perf = "Essaie encore, tu peux mieux faire !"; col = (SDL_Color){255,80,80,255}; }

    DRAW_TEXT(perf, col, SCREEN_H/2 + 30);
    DRAW_TEXT("Appuie sur ENTREE pour revenir au menu", blanc, SCREEN_H/2 + 100);

    #undef DRAW_TEXT
}

/* ============================================================
   runGameMenu  — appelable depuis score.c (touche E)
   Reçoit window+renderer déjà créés, gère le menu Quiz/Puzzle
   ============================================================ */
void runGameMenu(SDL_Window *window, SDL_Renderer *renderer)
{
    (void)window;

    TTF_Font *font      = TTF_OpenFont("assets/font/font.ttf", 22);
    TTF_Font *fontSmall = TTF_OpenFont("assets/font/font.ttf", 16);
    if (!font)      font      = TTF_OpenFont("batmfa.ttf", 22);
    if (!fontSmall) fontSmall = TTF_OpenFont("batmfa.ttf", 16);
    if (!font || !fontSmall) {
        fprintf(stderr, "[ERREUR] Font introuvable: %s\n", TTF_GetError());
        return;
    }

    Menu   menu;
    Enigme enigme;
    initMenu(&menu, renderer);
    initEnigme(&enigme, renderer);

    GameState state   = STATE_MENU;
    int       running = 1;
    SDL_Event event;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) { running = 0; break; }

            if (event.type == SDL_KEYDOWN &&
                event.key.keysym.sym == SDLK_ESCAPE) { running = 0; break; }

            if (state == STATE_MENU)
            {
                if (event.type == SDL_MOUSEMOTION) {
                    int mx = event.motion.x, my = event.motion.y;
                    menu.hoverQuiz   = pointInRect(mx, my, &menu.quizRect);
                    menu.hoverPuzzle = pointInRect(mx, my, &menu.puzzleRect);
                }
                if (event.type == SDL_MOUSEBUTTONDOWN &&
                    event.button.button == SDL_BUTTON_LEFT)
                {
                    int mx = event.button.x, my = event.button.y;
                    if (pointInRect(mx, my, &menu.quizRect)) {
                        freeEnigme(&enigme);
                        initEnigme(&enigme, renderer);
                        state = STATE_QUIZ;
                    }
                    /* Puzzle button — connecte ton puzzle ici */
                    if (pointInRect(mx, my, &menu.puzzleRect)) {
                        /* TODO: lancer puzzle */
                    }
                }
            }
            else if (state == STATE_QUIZ)
            {
                handleEnigmeEvent(&enigme, &event);
            }
            else if (state == STATE_SCORE)
            {
                if (event.type == SDL_KEYDOWN &&
                    event.key.keysym.sym == SDLK_RETURN)
                    state = STATE_MENU;
            }
        }

        if (state == STATE_QUIZ) {
            updateEnigme(&enigme);
            if (enigme.questionIndex >= NB_QUESTIONS)
                state = STATE_SCORE;
        }

        SDL_RenderClear(renderer);
        if      (state == STATE_MENU)  renderMenu(&menu, renderer);
        else if (state == STATE_QUIZ)  renderEnigme(&enigme, renderer, font, fontSmall);
        else if (state == STATE_SCORE) renderScore(renderer, font, &enigme, menu.bgTexture);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    freeEnigme(&enigme);
    freeMenu(&menu);
    TTF_CloseFont(font);
    TTF_CloseFont(fontSmall);
}

/* ============================================================
   main  — point d'entrée standalone (si lancé directement)
   ============================================================ */
int main(void)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_Init(MIX_INIT_MP3);

    SDL_Window   *window   = SDL_CreateWindow("Batman Quiz",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_W, SCREEN_H, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    runGameMenu(window, renderer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    Mix_Quit();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
