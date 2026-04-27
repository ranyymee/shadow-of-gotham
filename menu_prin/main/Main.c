#include "header.h"
#include "integrated.h"
#include "enigme.h"
#include "menu.h"
#include "options.h"
#include <stdio.h>
#include "sauvegarde.h"

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    Menu menu;
    if (!init(&menu)) return 1;
    loadAssets(&menu);
    initialiser_sauvegarde(menu.renderer);

    GameState state = STATE_MENU;
    PlayerMenu playerMenu;
    bool playerMenuInitialized = false;
    int  playerScore = 0;
    Options options;
    int optionsInit = 0;

    while (menu.running && state != STATE_QUIT)
    {
        SDL_Event e;

        if (state == STATE_MENU)
        {
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT) { menu.running = false; state = STATE_QUIT; break; }
                if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                    switch (e.key.keysym.sym) {
                        case SDLK_s:      state = STATE_SCORES;     break;
                        case SDLK_e:      state = STATE_ENIGME;     break;
                        case SDLK_p:      state = STATE_SAUVEGARDE; break;
                        case SDLK_o:      state = STATE_OPTIONS;    break;
                        case SDLK_ESCAPE: state = STATE_QUIT; menu.running = false; break;
                        default: break;
                    }
                }
                if (e.type == SDL_MOUSEMOTION) {
                    int mx = e.motion.x, my = e.motion.y;
                    for (int i = 0; i < BUTTON_COUNT; i++) {
                        SDL_Point pt = {mx, my};
                        menu.buttons[i].state = SDL_PointInRect(&pt, &menu.buttons[i].rect) ? 1 : 0;
                    }
                }
                if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                    int mx = e.button.x, my = e.button.y;
                    SDL_Point pt = {mx, my};
                    for (int i = 0; i < BUTTON_COUNT; i++) {
                        if (SDL_PointInRect(&pt, &menu.buttons[i].rect)) {
                            if (menu.clickSound) Mix_PlayChannel(-1, menu.clickSound, 0);
                            switch (i) {
                                case 0: state = STATE_SAUVEGARDE; break;
                                case 1: state = STATE_OPTIONS;    break;
                                case 2: state = STATE_SCORES;     break;
                                case 3: state = STATE_ENIGME;     break;
                                case 4: state = STATE_QUIT; menu.running = false; break;
                            }
                            break;
                        }
                    }
                }
            }
            menu.currentFrame += menu.bgDirection;
            if (menu.currentFrame >= FRAME_COUNT - 1) menu.bgDirection = -1;
            if (menu.currentFrame <= 0)               menu.bgDirection =  1;
            update(&menu);
            render(&menu);
        }

        else if (state == STATE_SAUVEGARDE)
        {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) { menu.running = false; state = STATE_QUIT; break; }
                gerer_evenement_sauvegarde(e, &state);
            }
            SDL_RenderClear(menu.renderer);
            afficher_sous_menu_sauvegarde(menu.renderer);
            SDL_RenderPresent(menu.renderer);
        }

        else if (state == STATE_OPTIONS)
        {
            if (!optionsInit) {
                Mix_HaltMusic();
                if (!initOptions(&options, menu.renderer)) {
                    printf("Erreur init options\n");
                    state = STATE_MENU;
                    if (menu.music) Mix_PlayMusic(menu.music, -1);
                    SDL_Delay(16); continue;
                }
                optionsInit = 1;
            }
            int running_opt = 1;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) { menu.running = false; state = STATE_QUIT; running_opt = 0; break; }
                if (e.type == SDL_KEYDOWN && e.key.repeat == 0 && e.key.keysym.sym == SDLK_ESCAPE) {
                    freeOptions(&options); optionsInit = 0;
                    if (menu.music) Mix_PlayMusic(menu.music, -1);
                    state = STATE_MENU; running_opt = 0; break;
                }
                inputOptions(&options, e, menu.window, &running_opt);
                if (!running_opt) {
                    freeOptions(&options); optionsInit = 0;
                    if (menu.music) Mix_PlayMusic(menu.music, -1);
                    state = STATE_MENU; break;
                }
            }
            if (state == STATE_OPTIONS) {
                SDL_RenderClear(menu.renderer);
                renderOptions(&options, menu.renderer);
                SDL_RenderPresent(menu.renderer);
            }
        }

        else if (state == STATE_PLAYER)
        {
            if (!playerMenuInitialized) {
                Mix_HaltMusic();
                if (!initPlayerMenu(&playerMenu, menu.window, menu.renderer)) {
                    state = STATE_MENU;
                    if (menu.music) Mix_PlayMusic(menu.music, -1);
                    continue;
                }
                if (!loadPlayerMenuResources(&playerMenu)) {
                    cleanupPlayerMenu(&playerMenu);
                    state = STATE_MENU;
                    if (menu.music) Mix_PlayMusic(menu.music, -1);
                    continue;
                }
                playerMenuInitialized = true;
            }
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) { menu.running = false; state = STATE_QUIT; break; }
                bool goToScore = handlePlayerMenuEvents(&playerMenu, &e, &menu.running);
                if (goToScore) { playerScore = playerMenu.playerScore; state = STATE_SCORES; break; }
                if (e.type == SDL_KEYDOWN && e.key.repeat == 0 && e.key.keysym.sym == SDLK_ESCAPE) {
                    state = STATE_MENU;
                    if (menu.music) Mix_PlayMusic(menu.music, -1);
                    break;
                }
            }
            if (state != STATE_PLAYER) {
                if (state == STATE_MENU || state == STATE_QUIT) {
                    cleanupPlayerMenu(&playerMenu); playerMenuInitialized = false;
                }
                SDL_Delay(16); continue;
            }
            updatePlayerMenu(&playerMenu);
            renderPlayerMenu(&playerMenu);
        }

        else if (state == STATE_SCORES)
        {
            Mix_HaltMusic();
            int final_score = playerScore > 0 ? playerScore : 123;
            int result = scoreMenuLoop(menu.window, menu.renderer, final_score);
            if (playerMenuInitialized) { cleanupPlayerMenu(&playerMenu); playerMenuInitialized = false; }
            if (menu.music) Mix_PlayMusic(menu.music, -1);
            state = (result == 2) ? STATE_MENU : STATE_QUIT;
        }

        /* FIX : appel runGameMenu() qui gère le menu QCM/Puzzle complet */
        else if (state == STATE_ENIGME)
        {
            Mix_HaltMusic();
            runGameMenu(menu.window, menu.renderer);
            if (menu.music) Mix_PlayMusic(menu.music, -1);
            state = STATE_MENU;
        }

        SDL_Delay(16);
    }

    if (playerMenuInitialized) cleanupPlayerMenu(&playerMenu);
    if (optionsInit) freeOptions(&options);
    destroy(&menu);
    return 0;
}
