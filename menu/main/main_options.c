#include "options.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    /* ── SDL init ── */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "[ERROR] SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    /* ── SDL_image init ── */
    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) {
        fprintf(stderr, "[WARN] IMG_Init: %s\n", IMG_GetError());
    }

    /* ── SDL_mixer init ── */
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "[WARN] Mix_OpenAudio: %s\n", Mix_GetError());
    }

    /* ── Window & Renderer ── */
    SDL_Window *window = SDL_CreateWindow(
        "Batman Options",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        fprintf(stderr, "[ERROR] SDL_CreateWindow: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        fprintf(stderr, "[ERROR] SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    /* ── Options init ── */
    Options opt;
    if (!initOptions(&opt, renderer)) {
        fprintf(stderr, "[ERROR] initOptions failed\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    /* ── Main loop ── */
    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            inputOptions(&opt, event, window, &running);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        renderOptions(&opt, renderer);

        SDL_RenderPresent(renderer);
    }

    /* ── Cleanup ── */
    freeOptions(&opt);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;
}
