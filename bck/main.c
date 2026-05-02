#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include "back.h"

#define SCREEN_W 1980
#define SCREEN_H 1440

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    SDL_Window   *window;
    SDL_Renderer *renderer;
    TTF_Font     *font;
    Mix_Music    *music = NULL;
    Background    bg;
    Background    bg2;   /* caméra indépendante pour le split P2 */
    Platform      platforms[MAX_PLATFORMS];
    SDL_Event     event;
    int           taille   = 0;
    int           level    = 1;
    int           affMode  = MODE_MONO;
    int           running  = 1;
    int           screenW  = SCREEN_W;
    int           screenH  = SCREEN_H;
    int           timeLeft = 600;
    int           lives    = 3;
    Uint32        lastTime;
    SDL_Color     textColor = {255, 255, 255, 255};

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0) {
        printf("Erreur SDL_Init : %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() == -1) {
        printf("Erreur TTF_Init : %s\n", TTF_GetError());
        SDL_Quit(); return 1;
    }
    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_WEBP) == 0) {
        printf("Erreur IMG_Init : %s\n", IMG_GetError());
        TTF_Quit(); SDL_Quit(); return 1;
    }

    window = SDL_CreateWindow("Batman vs Catwoman",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              screenW, screenH,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!window) {
        printf("Erreur fenetre : %s\n", SDL_GetError());
        SDL_Quit(); return 1;
    }

    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED |
                                  SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Erreur renderer : %s\n", SDL_GetError());
        SDL_DestroyWindow(window); SDL_Quit(); return 1;
    }

    SDL_GetRendererOutputSize(renderer, &screenW, &screenH);

    /* Audio optionnel - si PulseAudio crash, on continue sans son */
    {
        SDL_setenv("SDL_AUDIODRIVER", "dummy", 0);
        if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) == 0) {
            music = Mix_LoadMUS("menu_music.mp3");
            if (music)
                Mix_PlayMusic(music, -1);
            else
                printf("Avertissement : menu_music non charge : %s\n", Mix_GetError());
        } else {
            printf("Avertissement audio (continue sans son) : %s\n", Mix_GetError());
        }
    }

    initBackgroundAndPlatforms(renderer, &bg, platforms, &taille, level, screenW, screenH);
    /* bg2 partage les mêmes textures mais a sa propre caméra */
    bg2 = bg;
    bg2.camera_pos.x = 0;
    bg2.camera_pos.y = bg.camera_pos.y;

    font = TTF_OpenFont("font.ttf", 20);
    if (!font) font = TTF_OpenFont("arial.ttf", 20);
    if (!font) printf("Avertissement : police non chargee\n");

    lastTime = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { running = 0; break; }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: running = 0; break;
                    case SDLK_p:
                        affMode = (affMode == MODE_MONO) ? MODE_MULTI : MODE_MONO;
                        break;
                    case SDLK_F1:
                        if (level != 1) {
                            level = 1; taille = 0;
                            initBackgroundAndPlatforms(renderer, &bg, platforms,
                                                       &taille, level, screenW, screenH);
                            bg2 = bg; bg2.camera_pos.x = 0;
                        }
                        break;
                    case SDLK_F2:
                        if (level != 2) {
                            level = 2; taille = 0;
                            initBackgroundAndPlatforms(renderer, &bg, platforms,
                                                       &taille, level, screenW, screenH);
                            bg2 = bg; bg2.camera_pos.x = 0;
                        }
                        break;
                    case SDLK_F3:
                        if (level != 3) {
                            level = 3; taille = 0;
                            initBackgroundAndPlatforms(renderer, &bg, platforms,
                                                       &taille, level, screenW, screenH);
                            bg2 = bg; bg2.camera_pos.x = 0;
                        }
                        break;
                    default: break;
                }
            }
            gererGuideEtClic(event, &bg.guide, &bg.commentJouer,
                             &bg.afficherCommentJouer);
        }

        gererScrollingDeuxJoueurs(event, &bg, &bg2, 20, level);

        gererTemps(&timeLeft, &lastTime);
        if (timeLeft <= 0) timeLeft = 0;

        updatePlatforms(platforms, taille);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (affMode == MODE_MULTI) {
            SDL_Rect vp1 = {0,         0, screenW/2, screenH};
            SDL_Rect vp2 = {screenW/2, 0, screenW/2, screenH};

            SDL_RenderSetViewport(renderer, &vp1);
            afficherBackgroundEtElements(renderer, &bg, platforms, taille,
                                         font, textColor, timeLeft, lives,
                                         bg.camera_pos.x, bg.camera_pos.y, screenW/2, screenH,
                                         MODE_MULTI);

            SDL_RenderSetViewport(renderer, &vp2);
            afficherBackgroundEtElements(renderer, &bg2, platforms, taille,
                                         font, textColor, timeLeft, lives,
                                         bg2.camera_pos.x, bg2.camera_pos.y, screenW/2, screenH,
                                         MODE_MULTI);

            SDL_RenderSetViewport(renderer, NULL);
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_RenderDrawLine(renderer, screenW/2, 0, screenW/2, screenH);

        } else {
            SDL_RenderSetViewport(renderer, NULL);
            afficherBackgroundEtElements(renderer, &bg, platforms, taille,
                                         font, textColor, timeLeft, lives,
                                         bg.camera_pos.x, bg.camera_pos.y, screenW, screenH,
                                         MODE_MONO);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_RenderSetViewport(renderer, NULL);
    if (font)
        saisirNomEtAfficherScore(renderer, font, 0, screenW, screenH);

    {
        int _bi;
        for (_bi = 0; _bi < 8; _bi++)
            if (bg.img[_bi]) SDL_DestroyTexture(bg.img[_bi]);
    }
    if (bg.guide.image)        SDL_DestroyTexture(bg.guide.image);
    if (bg.commentJouer.image) SDL_DestroyTexture(bg.commentJouer.image);
    {
        int i, f;
        Platform *pp;
        for (i = 0; i < taille; i++) {
            pp = &platforms[i];
            if (pp->isAnimated) {
                for (f = 0; f < MAX_FRAMES; f++)
                    if (pp->frames[f]) SDL_DestroyTexture(pp->frames[f]);
            } else {
                if (pp->image) SDL_DestroyTexture(pp->image);
            }
        }
    }
    if (font) TTF_CloseFont(font);
    if (music) { Mix_HaltMusic(); Mix_FreeMusic(music); }
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
