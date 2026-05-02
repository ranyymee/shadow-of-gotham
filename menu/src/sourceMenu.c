#include "header.h"
#include "game.h"
#include <stdio.h>

/* ================ TEXTURE INTERNE ================ */
static SDL_Texture *loadTextureMenu(Menu *menu, const char *path)
{
    if (!path || path[0] == '\0') {
        printf("[WARN] loadTextureMenu: NULL ou chemin vide\n");
        return NULL;
    }
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
<<<<<<< HEAD
=======
    printf("[OK] SDL_Init\n");
>>>>>>> 75f7f12 (add bck folder)

    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_Init(MIX_INIT_MP3);
    printf("[OK] IMG/TTF/Mix init\n");

    menu->window = SDL_CreateWindow(
        "Shadow Of Gotham",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_FULLSCREEN_DESKTOP);

<<<<<<< HEAD
    if (!menu->window) return 0;

    menu->renderer = SDL_CreateRenderer(menu->window, -1, SDL_RENDERER_ACCELERATED);
    if (!menu->renderer) return 0;

    menu->running = true;
    menu->currentFrame = 0;
=======
    if (!menu->window) {
        printf("[ERR] SDL_CreateWindow: %s\n", SDL_GetError());
        return 0;
    }
    printf("[OK] Window created\n");

    menu->renderer = SDL_CreateRenderer(menu->window, -1, SDL_RENDERER_ACCELERATED);
    if (!menu->renderer) {
        printf("[ERR] SDL_CreateRenderer: %s\n", SDL_GetError());
        return 0;
    }
    printf("[OK] Renderer created\n");

    menu->running = true;
    menu->currentFrame = 0;
    menu->bgDirection  = 1;
>>>>>>> 75f7f12 (add bck folder)

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

<<<<<<< HEAD
    /* afficher frame actuelle */
=======
    /* arrière-plan animé */
>>>>>>> 75f7f12 (add bck folder)
    if (menu->frames[menu->currentFrame])
        SDL_RenderCopy(menu->renderer,
                       menu->frames[menu->currentFrame],
                       NULL, NULL);

<<<<<<< HEAD
    /* boutons */
=======
    /* ── Titre SHADOW OF GOTHAM ── */
    if (menu->font) {
        int W, H;
        SDL_GetRendererOutputSize(menu->renderer, &W, &H);

        /* "SHADOW" en vert */
        SDL_Color green  = {0, 200, 0, 255};
        SDL_Color orange = {220, 100, 0, 255};
        SDL_Color white  = {255, 255, 255, 255};

        SDL_Surface *s1 = TTF_RenderText_Blended(menu->font, "SHADOW", green);
        SDL_Surface *s2 = TTF_RenderText_Blended(menu->font, "OF", white);
        SDL_Surface *s3 = TTF_RenderText_Blended(menu->font, "GOTHAM", orange);

        int titleY = H * 6 / 100;
        int gap2   = W * 1 / 100;

        int totalW = 0;
        if (s1) totalW += s1->w + gap2;
        if (s2) totalW += s2->w + gap2;
        if (s3) totalW += s3->w;

        int curX = (W - totalW) / 2;

        if (s1) {
            SDL_Texture *t = SDL_CreateTextureFromSurface(menu->renderer, s1);
            SDL_Rect r = {curX, titleY, s1->w, s1->h};
            SDL_RenderCopy(menu->renderer, t, NULL, &r);
            curX += s1->w + gap2;
            SDL_DestroyTexture(t); SDL_FreeSurface(s1);
        }
        if (s2) {
            SDL_Texture *t = SDL_CreateTextureFromSurface(menu->renderer, s2);
            SDL_Rect r = {curX, titleY, s2->w, s2->h};
            SDL_RenderCopy(menu->renderer, t, NULL, &r);
            curX += s2->w + gap2;
            SDL_DestroyTexture(t); SDL_FreeSurface(s2);
        }
        if (s3) {
            SDL_Texture *t = SDL_CreateTextureFromSurface(menu->renderer, s3);
            SDL_Rect r = {curX, titleY, s3->w, s3->h};
            SDL_RenderCopy(menu->renderer, t, NULL, &r);
            SDL_DestroyTexture(t); SDL_FreeSurface(s3);
        }
    }

    /* ── Boutons ── */
>>>>>>> 75f7f12 (add bck folder)
    for (int i = 0; i < BUTTON_COUNT; i++)
    {
        Button *b = &menu->buttons[i];
        SDL_Texture *tex = b->state ? b->hover : b->normal;
<<<<<<< HEAD

=======
>>>>>>> 75f7f12 (add bck folder)
        if (tex)
            SDL_RenderCopy(menu->renderer, tex, NULL, &b->rect);
    }

    SDL_RenderPresent(menu->renderer);
}

/* ================= LOAD ASSETS ================= */
void loadAssets(Menu *menu)
{
    printf("[OK] loadBackground...\n"); fflush(stdout);
    loadBackground(menu);
    printf("[OK] loadButtons...\n");   fflush(stdout);
    loadButtons(menu);
    printf("[OK] buttons loaded\n");   fflush(stdout);

    menu->clickSound = Mix_LoadWAV("assets/audio/click.wav");
    menu->music = Mix_LoadMUS("assets/audio/menu_music.mp3");
<<<<<<< HEAD
=======
    printf("[OK] audio loaded (click=%p music=%p)\n",
           (void*)menu->clickSound, (void*)menu->music); fflush(stdout);
>>>>>>> 75f7f12 (add bck folder)

    if (menu->music)
        Mix_PlayMusic(menu->music, -1);

    menu->font = TTF_OpenFont("assets/font/font.ttf", 52);
<<<<<<< HEAD
=======
    printf("[OK] font=%p\n", (void*)menu->font); fflush(stdout);
>>>>>>> 75f7f12 (add bck folder)
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

/* ================= LOAD BUTTONS ================= */
void loadButtons(Menu *menu)
{
    const char *normalPaths[BUTTON_COUNT] = {
        "assets/image/bouton play.png",
        "assets/image/bouton option.png",
        "assets/image/bouton score.png",
        "assets/image/bouton history.png",
        "assets/image/bouton quitter.png"
    };
    const char *hoverPaths[BUTTON_COUNT] = {
        "assets/image/bouton play 1.png",
        "assets/image/bouton option 1.png",
        "assets/image/bouton score 1.png",
        "assets/image/bouton history 1.png",
        "assets/image/bouton quitter 1.png"
    };

    int W, H;
    SDL_GetRendererOutputSize(menu->renderer, &W, &H);

    int bW     = W * 28 / 100;   /* largeur bouton — plus large comme image cible */
    int bH     = H * 12 / 100;   /* hauteur bouton — plus haute */
    int bX     = W * 50 / 100;   /* centré-droite, bX = centre - bW/2 + décalage */
    int startY = H * 25 / 100;   /* commence à 25% du haut */
    int gap    = H * 12 / 100;   /* espacement vertical égal à bH */

    /* Centrer le bloc de boutons horizontalement dans la moitié droite */
    bX = W * 52 / 100;

    for (int i = 0; i < BUTTON_COUNT; i++) {
        menu->buttons[i].normal = loadTextureMenu(menu, normalPaths[i]);
        menu->buttons[i].hover  = loadTextureMenu(menu, hoverPaths[i]);
        menu->buttons[i].rect   = (SDL_Rect){ bX, startY + i * gap, bW, bH };
        menu->buttons[i].state  = 0;  /* 0 = normal, pas hover par défaut */
    }
}

/* ================= LOAD BUTTONS ================= */
