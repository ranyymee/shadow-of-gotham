#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
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
    Background    bg;
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

    /* ── Init SDL ── */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
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

    /* ── Fenetre plein ecran ── */
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

    /* Lire les vraies dimensions apres fullscreen */
    SDL_GetRendererOutputSize(renderer, &screenW, &screenH);

    /* ── Init background + plateformes ── */
    initBackgroundAndPlatforms(renderer, &bg, platforms, &taille, level, screenW, screenH);

    /* ── Police ── */
    font = TTF_OpenFont("arial.ttf", 20);
    if (!font) font = TTF_OpenFont("font.ttf", 20);
    if (!font) printf("Avertissement : police non chargee\n");

    lastTime = SDL_GetTicks();

    /* ════════════════════════════════
       Boucle principale
    ════════════════════════════════ */
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { running = 0; break; }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: running = 0; break;
                    /* P = basculer mono/multi */
                    case SDLK_p:
                        affMode = (affMode == MODE_MONO) ? MODE_MULTI : MODE_MONO;
                        break;
                    /* F1 / F2 = changer de level */
                    case SDLK_F1:
                        if (level != 1) {
                            level = 1; taille = 0;
                            initBackgroundAndPlatforms(renderer, &bg, platforms,
                                                       &taille, level, screenW, screenH);
                        }
                        break;
                    case SDLK_F2:
                        if (level != 2) {
                            level = 2; taille = 0;
                            initBackgroundAndPlatforms(renderer, &bg, platforms,
                                                       &taille, level, screenW, screenH);
                        }
                        break;
                    default: break;
                }
            }
            /* Guide (click bouton ou touche G/H) */
            gererGuideEtClic(event, &bg.guide, &bg.commentJouer,
                             &bg.afficherCommentJouer);
        }

        /* Scrolling continu — appele une fois par frame hors event loop */
        gererScrollingDeuxJoueurs(event, &bg, &bg, 20);

        /* Timer */
        gererTemps(&timeLeft, &lastTime);
        if (timeLeft <= 0) timeLeft = 0;

        /* Deplacer plateformes mobiles */
        updatePlatforms(platforms, taille);

        /* ── Rendu ── */
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
            afficherBackgroundEtElements(renderer, &bg, platforms, taille,
                                         font, textColor, timeLeft, lives,
                                         bg.camera_pos.x, bg.camera_pos.y, screenW/2, screenH,
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

    /* ── Saisie nom + score final ── */
    SDL_RenderSetViewport(renderer, NULL);
    if (font)
        saisirNomEtAfficherScore(renderer, font, 0, screenW, screenH);

    /* ── Cleanup ── */
    if (bg.img[0])             SDL_DestroyTexture(bg.img[0]);
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
    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
