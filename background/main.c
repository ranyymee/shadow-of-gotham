#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include "back.h"

#define MAX_BG_X 2000
#define MAX_BG_Y 1500

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("Erreur SDL_Init : %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("Erreur TTF_Init : %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_WEBP) == 0) {
        printf("Erreur IMG_Init : %s\n", IMG_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    int SCREEN_W = 1000;
    int SCREEN_H = 900; /* sera mis a jour apres chargement du background */

    SDL_Window *window = SDL_CreateWindow(
        "Jeu SDL2",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        printf("Erreur SDL_CreateWindow : %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Erreur SDL_CreateRenderer : %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Background bg;
    Platform platforms[20];
    int taille = 0;
    initBackgroundAndPlatforms(renderer, &bg, platforms, &taille);

    /* Adapter la hauteur de la fenetre a celle du background */
    {
        int bgW, bgH;
        SDL_QueryTexture(bg.img[0], NULL, NULL, &bgW, &bgH);
        SCREEN_H = bgH;
        SDL_SetWindowSize(window, SCREEN_W, SCREEN_H);
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }

    TTF_Font *font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        printf("Erreur TTF_OpenFont : %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Color textColor = {255, 255, 255, 255};
    int    timeLeft    = 600;
    Uint32 lastTime    = SDL_GetTicks();
    int    scrollSpeed = 20;
    int    bgX1 = 0, bgY1 = 0;
    int    bgX2 = 0, bgY2 = 0;
    int    partageEcran = 0;
    int    lives       = 3;

    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;

            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_p)
                partageEcran = !partageEcran;

            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                running = 0;

            gererGuideEtClic(event, &bg.guide, &bg.commentJouer, &bg.afficherCommentJouer);
            gererScrollingDeuxJoueurs(event, &bgX1, &bgY1, &bgX2, &bgY2, scrollSpeed);
        }

        gererTemps(&timeLeft, &lastTime);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (partageEcran) {
            /* Gauche — joueur 1 */
            SDL_Rect rectGauche = {0, 0, SCREEN_W / 2, SCREEN_H};
            SDL_RenderSetViewport(renderer, &rectGauche);
            afficherBackgroundEtElements(renderer, &bg, platforms, taille, font, textColor, timeLeft, lives, bgX1, bgY1, SCREEN_W / 2, SCREEN_H);

            /* Droite — joueur 2 */
            SDL_Rect rectDroite = {SCREEN_W / 2, 0, SCREEN_W / 2, SCREEN_H};
            SDL_RenderSetViewport(renderer, &rectDroite);
            afficherBackgroundEtElements(renderer, &bg, platforms, taille, font, textColor, timeLeft, lives, bgX2, bgY2, SCREEN_W / 2, SCREEN_H);

            SDL_RenderSetViewport(renderer, NULL);
        } else {
            afficherBackgroundEtElements(renderer, &bg, platforms, taille, font, textColor, timeLeft, lives, bgX1, bgY1, SCREEN_W, SCREEN_H);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    /* Saisie nom + affichage score final */
    saisirNomEtAfficherScore(renderer, font, timeLeft, SCREEN_W, SCREEN_H);

    /* Cleanup */
    SDL_DestroyTexture(bg.img[0]);
    SDL_DestroyTexture(bg.guide.image);
    SDL_DestroyTexture(bg.commentJouer.image);

    for (int p = 0; p < taille; p++) {
        if (platforms[p].isAnimated) {
            for (int i = 0; i < MAX_FRAMES; i++)
                if (platforms[p].frames[i])
                    SDL_DestroyTexture(platforms[p].frames[i]);
        } else {
            if (p == 0 && platforms[p].image)
                SDL_DestroyTexture(platforms[p].image);
        }
    }

    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
