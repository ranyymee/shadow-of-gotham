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

/* ================= INIT ================= */
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
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_FULLSCREEN_DESKTOP);

    if (!menu->window) return 0;

    menu->renderer = SDL_CreateRenderer(menu->window, -1, SDL_RENDERER_ACCELERATED);
    if (!menu->renderer) return 0;

    menu->running = true;
    menu->currentFrame = 0;

    return 1;
}

/* ================= BACKGROUND LOAD (START ONLY) ================= */
void loadBackground(Menu *menu)
{
    for (int i = 0; i < 5; i++)
    {
        char path[128];
        sprintf(path, "assets/image/ezgif-frame-%03d.png", i + 1);
        menu->frames[i] = loadTextureMenu(menu, path);
    }

    for (int i = 5; i < FRAME_COUNT; i++)
        menu->frames[i] = NULL;
}

/* ================= UPDATE (🔥 FIX IMPORTANT) ================= */
void update(Menu *menu)
{
    menu->currentFrame++;

    if (menu->currentFrame >= FRAME_COUNT)
        menu->currentFrame = 0;

    /* load frame if needed */
    if (!menu->frames[menu->currentFrame])
    {
        char path[128];
        sprintf(path,
            "assets/image/ezgif-frame-%03d.png",
            menu->currentFrame + 1);

        menu->frames[menu->currentFrame] =
            loadTextureMenu(menu, path);
    }

    /* free old frame */
    int old = menu->currentFrame - 2;
    if (old >= 0 && menu->frames[old])
    {
        SDL_DestroyTexture(menu->frames[old]);
        menu->frames[old] = NULL;
    }
}

/* ================= RENDER (CLEAN ONLY) ================= */
void render(Menu *menu)
{
    SDL_RenderClear(menu->renderer);

    /* afficher frame actuelle */
    if (menu->frames[menu->currentFrame])
        SDL_RenderCopy(menu->renderer,
                       menu->frames[menu->currentFrame],
                       NULL, NULL);

    /* boutons */
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        Button *b = &menu->buttons[i];
        SDL_Texture *tex = b->state ? b->hover : b->normal;

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

    menu->clickSound = Mix_LoadWAV("assets/audio/click.wav");
    menu->music = Mix_LoadMUS("assets/audio/menu_music.mp3");

    if (menu->music)
        Mix_PlayMusic(menu->music, -1);

    menu->font = TTF_OpenFont("assets/font/font.ttf", 52);
}

/* ================= DESTROY ================= */
void destroy(Menu *menu)
{
    for (int i = 0; i < FRAME_COUNT; i++)
        if (menu->frames[i])
            SDL_DestroyTexture(menu->frames[i]);

    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        if (menu->buttons[i].normal)
            SDL_DestroyTexture(menu->buttons[i].normal);
        if (menu->buttons[i].hover)
            SDL_DestroyTexture(menu->buttons[i].hover);
    }

    if (menu->clickSound) Mix_FreeChunk(menu->clickSound);
    if (menu->music) Mix_FreeMusic(menu->music);

    SDL_DestroyRenderer(menu->renderer);
    SDL_DestroyWindow(menu->window);

    Mix_CloseAudio();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}
