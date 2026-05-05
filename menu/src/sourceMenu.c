#include "header.h"
#include "game.h"
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

/* ================= INIT ================= */
bool init(Menu *menu)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("[ERR] SDL_Init: %s\n", SDL_GetError());
        return false;
    }

    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_Init(MIX_INIT_MP3);

    menu->window = SDL_CreateWindow(
        "Shadow Of Gotham",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_FULLSCREEN_DESKTOP);

    if (!menu->window) return false;

    menu->renderer = SDL_CreateRenderer(menu->window, -1, SDL_RENDERER_ACCELERATED);
    if (!menu->renderer) return false;

    SDL_GetWindowSize(menu->window, &menu->winW, &menu->winH);

    menu->running      = true;
    menu->currentFrame = 0;
    menu->bgDirection  = 1;

    return true;
}

/* ================= BACKGROUND LOAD ================= */
void loadBackground(Menu *menu)
{
    for (int i = 0; i < FRAME_COUNT; i++)
    {
        char path[128];
        sprintf(path, "assets/image/ezgif-frame-%03d.png", i + 1);
        menu->bgFrames[i] = loadTextureMenu(menu, path);
    }
}

/* ================= LOAD BUTTONS ================= */
/*
 * Positions calées sur le design 1280x720 visible dans la capture :
 * Les boutons bat sont dans la moitié droite, centrés vers x~980,
 * empilés verticalement à partir de y~220 avec un espacement de ~105px.
 *
 * normal  = bouton sans hover  → "bouton play.png"   (sombre)
 * hoverTx = bouton avec hover  → "bouton play 1.png" (éclairé)
 */
void loadButtons(Menu *menu)
{
    /* Facteur de mise à l'échelle selon résolution réelle */
    float sx = menu->winW / 1280.0f;
    float sy = menu->winH / 720.0f;

    /* Dimensions et position de base (sur 1280x720) */
    /* Centre X des boutons ≈ 985, largeur bat ≈ 340, hauteur ≈ 100 */
    int bw  = (int)(340 * sx);
    int bh  = (int)(100 * sy);
    int bx  = (int)((985 - 340/2) * sx);   /* coin gauche */

    /* Y de départ ≈ 220, espacement ≈ 105 */
    int startY = (int)(220 * sy);
    int stepY  = (int)(105 * sy);

    const char *labels[BUTTON_COUNT] = {
        "PLAY", "OPTIONS", "SCORE", "HISTORY", "QUIT"
    };

    /* normal  = version sombre (sans hover) */
    const char *normalPaths[BUTTON_COUNT] = {
       
        
        "assets/image/bouton play 1.png",
        "assets/image/bouton option 1.png",
        "assets/image/bouton score 1.png",
        "assets/image/bouton history 1.png",
        "assets/image/bouton quitter 1.png",
    };

    /* hover = version éclairée */
    const char *hoverPaths[BUTTON_COUNT] = {
         "assets/image/bouton play.png",
        "assets/image/bouton option.png",
        "assets/image/bouton score.png",
        "assets/image/bouton history.png",
        "assets/image/bouton quitter.png",
    };

    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        menu->buttons[i].rect.x  = bx;
        menu->buttons[i].rect.y  = startY + i * stepY;
        menu->buttons[i].rect.w  = bw;
        menu->buttons[i].rect.h  = bh;
        menu->buttons[i].state   = 0;   /* pas de hover au démarrage */
        menu->buttons[i].texture = loadTextureMenu(menu, normalPaths[i]);
        menu->buttons[i].hoverTx = loadTextureMenu(menu, hoverPaths[i]);
        snprintf(menu->buttons[i].label, sizeof(menu->buttons[i].label),
                 "%s", labels[i]);
    }
}

/* ================= UPDATE ================= */
void update(Menu *menu)
{
    if (menu->currentFrame >= FRAME_COUNT) menu->currentFrame = 0;
    if (menu->currentFrame < 0)            menu->currentFrame = 0;

    if (!menu->bgFrames[menu->currentFrame])
    {
        char path[128];
        sprintf(path, "assets/image/ezgif-frame-%03d.png",
                menu->currentFrame + 1);
        menu->bgFrames[menu->currentFrame] = loadTextureMenu(menu, path);
    }
}

/* ================= RENDER TITRE ================= */
/*
 * Dessine "SHADOW OF GOTHAM" en haut centré.
 * "SHADOW" vert | "OF" blanc | "GOTHAM" orange
 * Même style que le logo visible dans la capture.
 */
static void renderTitle(Menu *menu)
{
    if (!menu->font) return;

    int fontSize = (int)(62 * menu->winH / 720.0f);
    TTF_Font *tf = TTF_OpenFont("assets/font/font.ttf", fontSize);
    if (!tf) return;

    struct { const char *w; SDL_Color c; } parts[3] = {
        {"SHADOW", {  0, 210,  60, 255}},
        {"OF",     {255, 255, 255, 255}},
        {"GOTHAM", {255,  90,   0, 255}},
    };

    int gap   = (int)(20 * menu->winW / 1280.0f);
    int totalW = 0, h = 0;
    int widths[3];

    for (int i = 0; i < 3; i++) {
        TTF_SizeText(tf, parts[i].w, &widths[i], &h);
        totalW += widths[i];
    }
    totalW += gap * 2;   /* 2 espaces entre 3 mots */

    int x = (menu->winW - totalW) / 2;
    int y = (int)(28 * menu->winH / 720.0f);

    for (int i = 0; i < 3; i++)
    {
        SDL_Surface *s = TTF_RenderText_Blended(tf, parts[i].w, parts[i].c);
        if (!s) { x += widths[i] + gap; continue; }
        SDL_Texture *t = SDL_CreateTextureFromSurface(menu->renderer, s);
        SDL_FreeSurface(s);
        if (t) {
            SDL_Rect dst = {x, y, widths[i], h};
            SDL_RenderCopy(menu->renderer, t, NULL, &dst);
            SDL_DestroyTexture(t);
        }
        x += widths[i] + gap;
    }

    TTF_CloseFont(tf);
}

/* ================= RENDER ================= */
void render(Menu *menu)
{
    SDL_RenderClear(menu->renderer);

    /* Background animé */
    if (menu->bgFrames[menu->currentFrame])
        SDL_RenderCopy(menu->renderer,
                       menu->bgFrames[menu->currentFrame],
                       NULL, NULL);

    /* Titre (TTF) — remplacé par logo.png si présent */
    if (menu->logoTexture)
        SDL_RenderCopy(menu->renderer, menu->logoTexture, NULL, NULL);
    else
        renderTitle(menu);

    /* Boutons bat */
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        Button *b = &menu->buttons[i];
        /* state=1 → hoverTx (éclairé), state=0 → texture (sombre) */
        SDL_Texture *tex = (b->state && b->hoverTx) ? b->hoverTx : b->texture;
        if (tex)
            SDL_RenderCopy(menu->renderer, tex, NULL, &b->rect);
    }

    SDL_RenderPresent(menu->renderer);
}

/* ================= LOAD ASSETS ================= */
void loadAssets(Menu *menu)
{
    loadBackground(menu);
    loadButtons(menu);

    /* Logo optionnel */
    SDL_Surface *logoSurf = IMG_Load("assets/image/logo.png");
    menu->logoTexture = logoSurf
        ? SDL_CreateTextureFromSurface(menu->renderer, logoSurf)
        : NULL;
    if (logoSurf) SDL_FreeSurface(logoSurf);

    menu->clickSound = Mix_LoadWAV("assets/audio/click.wav");
    menu->hoverSound = Mix_LoadWAV("assets/audio/hover.wav");
    menu->music      = Mix_LoadMUS("assets/audio/menu_music.mp3");

    if (menu->music)
        Mix_PlayMusic(menu->music, -1);

    menu->font = TTF_OpenFont("assets/font/font.ttf", 42);
    if (!menu->font)
        printf("[WARN] Police introuvable : %s\n", TTF_GetError());
}

/* ================= DESTROY ================= */
void destroy(Menu *menu)
{
    for (int i = 0; i < FRAME_COUNT; i++)
        if (menu->bgFrames[i])
            SDL_DestroyTexture(menu->bgFrames[i]);

    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        if (menu->buttons[i].texture) SDL_DestroyTexture(menu->buttons[i].texture);
        if (menu->buttons[i].hoverTx) SDL_DestroyTexture(menu->buttons[i].hoverTx);
    }

    if (menu->logoTexture) SDL_DestroyTexture(menu->logoTexture);
    if (menu->clickSound)  Mix_FreeChunk(menu->clickSound);
    if (menu->hoverSound)  Mix_FreeChunk(menu->hoverSound);
    if (menu->music)       Mix_FreeMusic(menu->music);
    if (menu->font)        TTF_CloseFont(menu->font);

    SDL_DestroyRenderer(menu->renderer);
    SDL_DestroyWindow(menu->window);

    Mix_CloseAudio();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}
