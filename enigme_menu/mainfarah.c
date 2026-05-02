#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <string.h>
#include "enigme.h"
#include "header.h"

/* Defined in enigme.c */
extern int SCR_W;
extern int SCR_H;

/* ── Puzzle globals (defined here, used by main.c / puzzle.c) ── */
SDL_Renderer *g_ren     = NULL;
TTF_Font     *g_fntBig  = NULL;
TTF_Font     *g_fntMid  = NULL;
TTF_Font     *g_fntSm   = NULL;
TTF_Font     *g_fntTiny = NULL;
Mix_Music    *g_music   = NULL;

typedef enum { STATE_MENU, STATE_QUIZ, STATE_PUZZLE, STATE_SCORE } MenuState;

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

static void initMenu(Menu *m, SDL_Renderer *r)
{
    m->bgTexture      = loadTex("assets/background.png", r);
    m->quizTex        = loadTex("assets/qcm.png",        r);
    m->quizHoverTex   = loadTex("assets/qcmh.png",       r);
    m->puzzleTex      = loadTex("assets/puzzlee.png",     r);
    m->puzzleHoverTex = loadTex("assets/puzzleh.png",    r);
    m->hoverQuiz = m->hoverPuzzle = 0;

    int bW = SCR_W * 25 / 100;
    int bH = bW * 50 / 100;
    int gap = SCR_W * 8 / 100;
    int bY  = SCR_H * 46 / 100;
    m->quizRect   = (SDL_Rect){ SCR_W/2 - bW - gap/2, bY, bW, bH };
    m->puzzleRect = (SDL_Rect){ SCR_W/2 + gap/2,      bY, bW, bH };
}

static int pointInRect(int x, int y, SDL_Rect *rect)
{
    return x>=rect->x && x<rect->x+rect->w && y>=rect->y && y<rect->y+rect->h;
}

static void freeMenu(Menu *m)
{
    if (m->bgTexture)      { SDL_DestroyTexture(m->bgTexture);      m->bgTexture=NULL; }
    if (m->quizTex)        { SDL_DestroyTexture(m->quizTex);        m->quizTex=NULL; }
    if (m->quizHoverTex)   { SDL_DestroyTexture(m->quizHoverTex);   m->quizHoverTex=NULL; }
    if (m->puzzleTex)      { SDL_DestroyTexture(m->puzzleTex);      m->puzzleTex=NULL; }
    if (m->puzzleHoverTex) { SDL_DestroyTexture(m->puzzleHoverTex); m->puzzleHoverTex=NULL; }
}

static void drawTextCX(SDL_Renderer *r, TTF_Font *font,
                       const char *txt, SDL_Color col, int y)
{
    SDL_Surface *s = TTF_RenderUTF8_Blended(font, txt, col);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_Rect rc = { SCR_W/2 - s->w/2, y, s->w, s->h };
    if (t) { SDL_RenderCopy(r, t, NULL, &rc); SDL_DestroyTexture(t); }
    SDL_FreeSurface(s);
}

static void renderMenu(Menu *m, SDL_Renderer *r, TTF_Font *fontBig)
{
    if (m->bgTexture) SDL_RenderCopy(r, m->bgTexture, NULL, NULL);

    int rW = SCR_W * 72 / 100;
    int rH = SCR_H * 59 / 100;
    int rY = SCR_H * 17 / 100;
    SDL_Rect box = { SCR_W/2 - rW/2, rY, rW, rH };
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 255, 255, 255, 58);
    SDL_RenderFillRect(r, &box);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    SDL_Color white = {255, 255, 255, 255};
    drawTextCX(r, fontBig, "ENIGME", white, SCR_H * 26 / 100);

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    if (m->hoverQuiz) {
        SDL_Rect *b = &m->quizRect;
        for (int i = 0; i < 8; i++) {
            int gW = b->w * (80 - i * 8) / 100;
            int gX = b->x + (b->w - gW) / 2;
            int gY = b->y + b->h + i * 3;
            Uint8 alpha = (Uint8)(30 - i * 3);
            SDL_SetRenderDrawColor(r, 255, 210, 80, alpha);
            SDL_Rect gr = { gX, gY, gW, 4 };
            SDL_RenderFillRect(r, &gr);
        }
    }
    if (m->hoverPuzzle) {
        SDL_Rect *b = &m->puzzleRect;
        for (int i = 0; i < 8; i++) {
            int gW = b->w * (80 - i * 8) / 100;
            int gX = b->x + (b->w - gW) / 2;
            int gY = b->y + b->h + i * 3;
            Uint8 alpha = (Uint8)(30 - i * 3);
            SDL_SetRenderDrawColor(r, 255, 210, 80, alpha);
            SDL_Rect gr = { gX, gY, gW, 4 };
            SDL_RenderFillRect(r, &gr);
        }
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    SDL_Texture *qt = (m->hoverQuiz   && m->quizHoverTex)   ? m->quizHoverTex   : m->quizTex;
    SDL_Texture *pt = (m->hoverPuzzle && m->puzzleHoverTex) ? m->puzzleHoverTex : m->puzzleTex;
    if (qt) SDL_RenderCopy(r, qt, NULL, &m->quizRect);
    if (pt) SDL_RenderCopy(r, pt, NULL, &m->puzzleRect);
}

static void renderScore(SDL_Renderer *r, TTF_Font *fontBig, TTF_Font *fontSub,
                        Enigme *e, SDL_Texture *bg)
{
    if (bg) SDL_RenderCopy(r, bg, NULL, NULL);
    int bW = SCR_W * 44 / 100, bH = SCR_H * 50 / 100;
    SDL_Rect box = { SCR_W/2 - bW/2, SCR_H/2 - bH/2, bW, bH };
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 220);
    SDL_RenderFillRect(r, &box);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(r, 255, 200, 0, 255);
    SDL_Rect fr = box;
    for (int i = 0; i < 3; i++) {
        SDL_RenderDrawRect(r, &fr);
        fr.x++; fr.y++; fr.w -= 2; fr.h -= 2;
    }
    SDL_Color yellow = {255,220,  0,255};
    SDL_Color white  = {255,255,255,255};
    SDL_Color cyan   = {  0,200,255,255};
    int step = SCR_H / 12;
    drawTextCX(r, fontBig, "= FINAL RESULTS =",        yellow, SCR_H/2 - step*2);
    char msg[80];
    snprintf(msg, sizeof(msg), "Score: %d / %d", e->score, NB_QUESTIONS);
    drawTextCX(r, fontSub, msg,                         white,  SCR_H/2 - step);
    snprintf(msg, sizeof(msg), "Level reached: %d", e->niveau);
    drawTextCX(r, fontSub, msg,                         cyan,   SCR_H/2);
    const char *perf; SDL_Color cp;
    if      (e->score == NB_QUESTIONS)        { perf="Perfect! True Batman expert!"; cp=(SDL_Color){  0,255,  0,255}; }
    else if (e->score >= NB_QUESTIONS*7/10)   { perf="Excellent! Well played!";      cp=(SDL_Color){  0,220,100,255}; }
    else if (e->score >= NB_QUESTIONS/2)      { perf="Good job! Keep it up!";        cp=(SDL_Color){255,200,  0,255}; }
    else                                      { perf="Keep practicing!";              cp=(SDL_Color){255, 80, 80,255}; }
    drawTextCX(r, fontSub, perf,                        cp,     SCR_H/2 + step);
    drawTextCX(r, fontSub, "Press ENTER to play again", white,  SCR_H/2 + step*2);
}

/* ============================================================
   MAIN
   ============================================================ */
int main(void)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags))
        fprintf(stderr, "[WARN] IMG_Init: %s\n", IMG_GetError());
    Mix_Init(MIX_INIT_MP3);
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 512) < 0)
        fprintf(stderr, "[WARN] Mix_OpenAudio: %s\n", Mix_GetError());

    SDL_Window *window = SDL_CreateWindow(
        "Batman Enigma Quiz",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_RESIZABLE);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_GetRendererOutputSize(renderer, &SCR_W, &SCR_H);
    fprintf(stdout, "[INFO] Window size: %d x %d\n", SCR_W, SCR_H);

    int fontBigPt   = SCR_H * 38 / 720;
    int fontSmallPt = SCR_H * 20 / 720;
    int fontTinyPt  = SCR_H * 14 / 720;
    int fontTitlePt = SCR_H * 90 / 720;
    if (fontBigPt   < 20)  fontBigPt   = 20;
    if (fontSmallPt < 12)  fontSmallPt = 12;
    if (fontTinyPt  < 10)  fontTinyPt  = 10;
    if (fontTitlePt < 60)  fontTitlePt = 60;

    TTF_Font *font      = TTF_OpenFont("assets/batmfa.ttf", fontBigPt);
    TTF_Font *fontSmall = TTF_OpenFont("assets/batmfa.ttf", fontSmallPt);
    TTF_Font *fontTiny  = TTF_OpenFont("assets/batmfa.ttf", fontTinyPt);
    TTF_Font *fontTitle = TTF_OpenFont("assets/batmfa.ttf", fontTitlePt);
    if (!font || !fontSmall || !fontTiny || !fontTitle) {
        fprintf(stderr, "[ERROR] assets/batmfa.ttf: %s\n", TTF_GetError());
        return 1;
    }
    fprintf(stdout, "[INFO] Fonts: big=%dpt small=%dpt tiny=%dpt title=%dpt\n",
            fontBigPt, fontSmallPt, fontTinyPt, fontTitlePt);

    Menu   menu;
    Enigme enigme;
    initMenu(&menu, renderer);
    initEnigme(&enigme, renderer);

    MenuState state   = STATE_MENU;
    int       running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { running=0; break; }
            if (event.type == SDL_KEYDOWN &&
                event.key.keysym.sym == SDLK_ESCAPE) { running=0; break; }

            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                SDL_GetRendererOutputSize(renderer, &SCR_W, &SCR_H);
                freeEnigme(&enigme);
                initEnigme(&enigme, renderer);
                initMenu(&menu, renderer);
            }

            if (state == STATE_MENU) {
                if (event.type == SDL_MOUSEMOTION) {
                    int mx=event.motion.x, my=event.motion.y;
                    menu.hoverQuiz   = pointInRect(mx,my,&menu.quizRect);
                    menu.hoverPuzzle = pointInRect(mx,my,&menu.puzzleRect);
                }
                if (event.type == SDL_MOUSEBUTTONDOWN &&
                    event.button.button == SDL_BUTTON_LEFT) {
                    int mx=event.button.x, my=event.button.y;
                    if (pointInRect(mx,my,&menu.quizRect)) {
                        freeEnigme(&enigme);
                        initEnigme(&enigme, renderer);
                        state = STATE_QUIZ;
                    } else if (pointInRect(mx,my,&menu.puzzleRect)) {
                        g_ren = renderer;
                        const char *bf  = "assets/batmfa__.ttf";
                        const char *fbo = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
                        const char *fno = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
                        if (!g_fntBig)  { g_fntBig  = TTF_OpenFont(bf,24); if(!g_fntBig)  g_fntBig  = TTF_OpenFont(fbo,22); }
                        if (!g_fntMid)  { g_fntMid  = TTF_OpenFont(bf,16); if(!g_fntMid)  g_fntMid  = TTF_OpenFont(fbo,15); }
                        if (!g_fntSm)   { g_fntSm   = TTF_OpenFont(bf,11); if(!g_fntSm)   g_fntSm   = TTF_OpenFont(fno,10); }
                        if (!g_fntTiny) { g_fntTiny = TTF_OpenFont(bf, 9); if(!g_fntTiny) g_fntTiny = TTF_OpenFont(fno, 9); }
                        SDL_SetWindowSize(window, WIN_W, WIN_H);
                        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                        int choice = run_menu(window);
                        if (choice >= 0) run_puzzle(window, choice);
                        SDL_SetWindowSize(window, SCR_W, SCR_H);
                        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                        SDL_RenderSetViewport(renderer, NULL);
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    }
                }
            }
            else if (state == STATE_QUIZ) {
                handleEnigmeEvent(&enigme, &event);
            }
            else if (state == STATE_SCORE) {
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

        SDL_SetRenderDrawColor(renderer, 10,10,20,255);
        SDL_RenderClear(renderer);
        switch (state) {
            case STATE_MENU:
                renderMenu(&menu, renderer, fontTitle);
                break;
            case STATE_QUIZ:
                renderEnigme(&enigme, renderer, font, fontSmall, fontTiny);
                break;
            case STATE_PUZZLE:
                break;
            case STATE_SCORE:
                renderScore(renderer, font, fontSmall, &enigme, menu.bgTexture);
                break;
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    freeEnigme(&enigme);
    freeMenu(&menu);
    TTF_CloseFont(font);
    TTF_CloseFont(fontSmall);
    TTF_CloseFont(fontTiny);
    TTF_CloseFont(fontTitle);
    if (g_fntBig)  { TTF_CloseFont(g_fntBig);  g_fntBig=NULL;  }
    if (g_fntMid)  { TTF_CloseFont(g_fntMid);  g_fntMid=NULL;  }
    if (g_fntSm)   { TTF_CloseFont(g_fntSm);   g_fntSm=NULL;   }
    if (g_fntTiny) { TTF_CloseFont(g_fntTiny); g_fntTiny=NULL; }
    Mix_HaltMusic();
    Mix_CloseAudio();
    Mix_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
