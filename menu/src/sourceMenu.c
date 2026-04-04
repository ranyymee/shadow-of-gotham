/*
 * sourceMenu.c — Implémentation du menu principal
 * Utilise uniquement header.h
 */
#include "header.h"
#include <stdio.h>

/* ================ TEXTURE INTERNE ================ */
static SDL_Texture *loadTextureMenu(Menu *menu, const char *path)
{
    SDL_Surface *surf = IMG_Load(path);
    if (!surf) {
        printf("[WARN] Erreur chargement %s\n", path);
        return NULL;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(menu->renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

/* -------------------------------------------------------
 * Dégradé vertical sur le texte
 * ----------------------------------------------------- */
static SDL_Texture *makeGradientText(Menu *menu, TTF_Font *font,
                                     const char *text,
                                     SDL_Color topColor,
                                     SDL_Color botColor)
{
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *base = TTF_RenderText_Blended(font, text, white);
    if (!base) return NULL;

    SDL_Surface *src = SDL_ConvertSurfaceFormat(base, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(base);
    if (!src) return NULL;

    int w = src->w, h = src->h;
    SDL_LockSurface(src);
    Uint32 *pixels = (Uint32 *)src->pixels;
    for (int y = 0; y < h; y++) {
        float t = (float)y / (float)(h - 1);
        Uint8 r = (Uint8)(topColor.r + t * ((int)botColor.r - topColor.r));
        Uint8 g = (Uint8)(topColor.g + t * ((int)botColor.g - topColor.g));
        Uint8 b = (Uint8)(topColor.b + t * ((int)botColor.b - topColor.b));
        for (int x = 0; x < w; x++) {
            Uint8 a = (pixels[y * w + x] >> 24) & 0xFF;
            if (a > 0)
                pixels[y * w + x] = ((Uint32)a << 24)|((Uint32)r << 16)|
                                     ((Uint32)g <<  8)| b;
        }
    }
    SDL_UnlockSurface(src);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(menu->renderer, src);
    SDL_FreeSurface(src);
    return tex;
}

/* ================ INIT ================ */
int init(Menu *menu)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("[ERR] SDL_Init: %s\n", SDL_GetError());
        return 0;
    }
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_Init(MIX_INIT_MP3);

    menu->window = SDL_CreateWindow(
        "Shadow Of Gotham",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!menu->window) { printf("[ERR] %s\n", SDL_GetError()); return 0; }

    menu->renderer = SDL_CreateRenderer(menu->window, -1, SDL_RENDERER_ACCELERATED);
    if (!menu->renderer) { printf("[ERR] %s\n", SDL_GetError()); return 0; }

    menu->running      = true;
    menu->currentFrame = 0;
    menu->bgDirection  = 1;
    return 1;
}

/* ================ BACKGROUND ================ */
void loadBackground(Menu *menu)
{
    for (int i = 0; i < FRAME_COUNT; i++) {
        char path[128];
        sprintf(path, "assets/image/ezgif-frame-%03d.png", i + 1);
        menu->frames[i] = loadTextureMenu(menu, path);
    }
}

/* ================ BUTTONS ================ */
void loadButtons(Menu *menu)
{
    const char *normal[BUTTON_COUNT] = {
        "bouton play 1.png",   "bouton option 1.png",
        "bouton score 1.png",  "bouton history 1.png",
        "bouton quitter 1.png"
    };
    const char *hover[BUTTON_COUNT] = {
        "bouton play.png",  "bouton option.png",
        "bouton score.png", "bouton history.png",
        "bouton quitter.png"
    };

    int winW, winH;
    SDL_GetWindowSize(menu->window, &winW, &winH);

    for (int i = 0; i < BUTTON_COUNT; i++) {
        char path[120];
        sprintf(path, "assets/image/%s", normal[i]);
        menu->buttons[i].normal = loadTextureMenu(menu, path);
        sprintf(path, "assets/image/%s", hover[i]);
        menu->buttons[i].hover  = loadTextureMenu(menu, path);
        menu->buttons[i].state  = 0;
    }

    int btnW  = 300;
    int btnH  = 105;
    int gap   = 20;

    int totalH = 4 * btnH + 3 * gap;
    int startY = (winH - totalH) / 2 + 90;
    int btnX   = winW / 2 + (winW / 2 - btnW) / 2 - 150;

    for (int i = 0; i < 4; i++) {
        menu->buttons[i].rect.w = btnW;
        menu->buttons[i].rect.h = btnH;
        menu->buttons[i].rect.x = btnX;
        menu->buttons[i].rect.y = startY + i * (btnH + gap);
    }

    {
        int qW = 120;
        int qH = 120;
        menu->buttons[4].rect.w = qW;
        menu->buttons[4].rect.h = qH;
        menu->buttons[4].rect.x = winW - qW - 20;
        menu->buttons[4].rect.y = winH - qH - 20;
    }
}

/* ================ ACTION ================ */
void action(Menu *menu, Action a)
{
    if (menu->clickSound) Mix_PlayChannel(-1, menu->clickSound, 0);
    switch (a) {
        case PLAY:    printf("PLAY\n");    break;
        case OPTIONS: printf("OPTIONS\n"); break;
        case SCORES:  printf("SCORES\n");  break;
        case HISTORY: printf("HISTORY\n"); break;
        case QUITTER: menu->running = false; break;
    }
}

/* ================ EVENTS (compatibilité) ================ */
void events(Menu *menu) { (void)menu; }

/* ================ RENDER ================ */
void render(Menu *menu)
{
    SDL_RenderClear(menu->renderer);

    if (menu->frames[menu->currentFrame])
        SDL_RenderCopy(menu->renderer, menu->frames[menu->currentFrame], NULL, NULL);

    if (menu->textShadow)
        SDL_RenderCopy(menu->renderer, menu->textShadow,   NULL, &menu->textShadowRect);
    if (menu->textOfGotham)
        SDL_RenderCopy(menu->renderer, menu->textOfGotham, NULL, &menu->textOfGothamRect);

    for (int i = 0; i < BUTTON_COUNT; i++) {
        Button *b        = &menu->buttons[i];
        SDL_Texture *tex = b->state ? b->hover : b->normal;
        if (tex) SDL_RenderCopy(menu->renderer, tex, NULL, &b->rect);
    }

    SDL_RenderPresent(menu->renderer);
}

/* ================ LOAD ASSETS ================ */
void loadAssets(Menu *menu)
{
    loadBackground(menu);
    loadButtons(menu);

    menu->clickSound = Mix_LoadWAV("assets/audio/click.wav");
    if (!menu->clickSound) printf("[WARN] click.wav: %s\n", Mix_GetError());

    menu->music = Mix_LoadMUS("assets/audio/menu_music.mp3");
    if (!menu->music) printf("[WARN] menu_music.mp3: %s\n", Mix_GetError());
    Mix_VolumeMusic(64);
    if (menu->music) Mix_PlayMusic(menu->music, -1);

    menu->font = TTF_OpenFont("assets/font/font.ttf", 52);
    if (!menu->font) printf("[WARN] font: %s\n", TTF_GetError());

    if (menu->font) {
        SDL_Color gTop = {  0, 230,  50, 255};
        SDL_Color gBot = {  0,  70,  10, 255};
        SDL_Color rTop = {255, 110,   0, 255};
        SDL_Color rBot = {130,   8,   0, 255};

        menu->textShadow   = makeGradientText(menu, menu->font,
                                              "SHADOW",     gTop, gBot);
        menu->textOfGotham = makeGradientText(menu, menu->font,
                                              " OF GOTHAM", rTop, rBot);

        int sw = 0, sh = 0, gw = 0, gh = 0;
        if (menu->textShadow)
            SDL_QueryTexture(menu->textShadow,   NULL, NULL, &sw, &sh);
        if (menu->textOfGotham)
            SDL_QueryTexture(menu->textOfGotham, NULL, NULL, &gw, &gh);

        int titleX = 480;
        int titleY = 120;

        menu->textShadowRect   = (SDL_Rect){ titleX,      titleY, sw, sh };
        menu->textOfGothamRect = (SDL_Rect){ titleX + sw, titleY, gw, gh };
    }
}

/* ================ DESTROY ================ */
void destroy(Menu *menu)
{
    for (int i = 0; i < FRAME_COUNT; i++)
        if (menu->frames[i]) SDL_DestroyTexture(menu->frames[i]);
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (menu->buttons[i].normal) SDL_DestroyTexture(menu->buttons[i].normal);
        if (menu->buttons[i].hover)  SDL_DestroyTexture(menu->buttons[i].hover);
    }
    if (menu->textShadow)   SDL_DestroyTexture(menu->textShadow);
    if (menu->textOfGotham) SDL_DestroyTexture(menu->textOfGotham);
    if (menu->font)         TTF_CloseFont(menu->font);
    if (menu->clickSound)   Mix_FreeChunk(menu->clickSound);
    if (menu->music)        Mix_FreeMusic(menu->music);
    SDL_DestroyRenderer(menu->renderer);
    SDL_DestroyWindow(menu->window);
    Mix_CloseAudio(); Mix_Quit(); IMG_Quit(); TTF_Quit(); SDL_Quit();
}
