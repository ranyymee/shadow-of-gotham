/*
 * Main.c — Point d'entrée unique du jeu "Shadow Of Gotham"
 */

#include "header.h"
#include "integrated.h"
#include "enigme.h"
#include "menu.h"  
#include "options.h"     /* Menu Options — projet collègue */
#include <stdio.h>
#include "sauvegarde.h" // YOUR HEADER


int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    /* ---- Initialisation ---- */
    Menu menu;
    if (!init(&menu)) return 1;
    loadAssets(&menu);
    initialiser_sauvegarde(menu.renderer);

    GameState state = STATE_MENU;

    /* Menu joueur */
    PlayerMenu playerMenu;  
    bool playerMenuInitialized = false;
    int playerScore = 0;

    /* Options — projet collègue */
    Options     options;
    int         optionsInit = 0;

    /* Enigme — chargée à la demande */
    Enigme    enigme;
    int       enigmeInit       = 0;
    TTF_Font *fontEnigme       = NULL;
    TTF_Font *fontEnigmeSmall  = NULL;

    while (menu.running && state != STATE_QUIT)
    {
        SDL_Event e;

        /* ---- MENU PRINCIPAL ---- */
        if (state == STATE_MENU)
        {
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT) {
                    menu.running = false;
                    state = STATE_QUIT;
                    break;
                }

                if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                    switch (e.key.keysym.sym) {
                        case SDLK_s:      state = STATE_SCORES;  break;
                        case SDLK_e:      state = STATE_ENIGME;  break;
                        case SDLK_p:      state = STATE_SAUVEGARDE;  break;
                        case SDLK_o:      state = STATE_OPTIONS; break; /* Touche O → Options */

                        case SDLK_ESCAPE:
                            state = STATE_QUIT;
                            menu.running = false;
                            break;
                        default: break;
                    }
                }

                if (e.type == SDL_MOUSEMOTION) {
                    int mx = e.motion.x, my = e.motion.y;
                    for (int i = 0; i < BUTTON_COUNT; i++) {
                        SDL_Point pt = {mx, my};
                        menu.buttons[i].state =
                            SDL_PointInRect(&pt, &menu.buttons[i].rect) ? 1 : 0;
                    }
                }

                if (e.type == SDL_MOUSEBUTTONDOWN &&
                    e.button.button == SDL_BUTTON_LEFT)
                {
                    int mx = e.button.x, my = e.button.y;
                    SDL_Point pt = {mx, my};
                    for (int i = 0; i < BUTTON_COUNT; i++) {
                        if (SDL_PointInRect(&pt, &menu.buttons[i].rect)) {
                            if (menu.clickSound)
                                Mix_PlayChannel(-1, menu.clickSound, 0);
                            switch (i) {
                                case 0: // PLAY BUTTON
                                    state = STATE_SAUVEGARDE; // REDIRECTED TO YOUR MENU
                                    break;
                                case 1: state = STATE_OPTIONS; break; /* Bouton Options */
                                case 2: state = STATE_SCORES;       break;
                                case 3: state = STATE_ENIGME;       break;
                                case 4:
                                    state = STATE_QUIT;
                                    menu.running = false;
                                    break;
                            }
                            break;
                        }
                    }
                }
                    update(&menu);   // 🔥 IMPORTANT
    render(&menu);

    SDL_Delay(16);
            }
menu.currentFrame += menu.bgDirection;
if (menu.currentFrame >= FRAME_COUNT - 1) menu.bgDirection = -1;
if (menu.currentFrame <= 0)               menu.bgDirection =  1;
            render(&menu);
        }

        /* ---- VOTRE SOUS-MENU SAUVEGARDE (NOUVEAU) ---- */
        else if (state == STATE_SAUVEGARDE)
        {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    menu.running = false;
                    state = STATE_QUIT;
                    break;
                }
                gerer_evenement_sauvegarde(e, &state);
            }
            
            // 1. Clear the old Main Menu frame
            SDL_RenderClear(menu.renderer); 
            
            // 2. Draw your animated background and buttons
            afficher_sous_menu_sauvegarde(menu.renderer); 
            
            // 3. IMPORTANT: Push the drawing to the screen so it's not "stuck"
            SDL_RenderPresent(menu.renderer); 
        }
        /* ---- MENU OPTIONS (projet collègue) ---- */
        else if (state == STATE_OPTIONS)
        {
            /* Initialisation à la première entrée */
            if (!optionsInit) {
                Mix_HaltMusic();
                if (!initOptions(&options, menu.renderer)) {
                    printf("Erreur init options\n");
                    state = STATE_MENU;
                    if (menu.music) Mix_PlayMusic(menu.music, -1);
                    SDL_Delay(16);
                    continue;
                }
                optionsInit = 1;
            }

            int running_opt = 1;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    menu.running = false;
                    state = STATE_QUIT;
                    running_opt = 0;
                    break;
                }
                /* ESC → retour au menu principal */
                if (e.type == SDL_KEYDOWN && e.key.repeat == 0 &&
                    e.key.keysym.sym == SDLK_ESCAPE) {
                    freeOptions(&options);
                    optionsInit = 0;
                    if (menu.music) Mix_PlayMusic(menu.music, -1);
                    state = STATE_MENU;
                    running_opt = 0;
                    break;
                }
                /* Déléguer les événements à la collègue */
                inputOptions(&options, e, menu.window, &running_opt);
                /* Si la collègue a demandé à quitter son écran (back) */
                if (!running_opt) {
                    freeOptions(&options);
                    optionsInit = 0;
                    if (menu.music) Mix_PlayMusic(menu.music, -1);
                    state = STATE_MENU;
                    break;
                }
            }

            if (state == STATE_OPTIONS) {
                SDL_RenderClear(menu.renderer);
                renderOptions(&options, menu.renderer);
                SDL_RenderPresent(menu.renderer);
            }
        }

        /* ---- MENU JOUEUR ---- */
        else if (state == STATE_PLAYER)
        {
            if (!playerMenuInitialized) {
                Mix_HaltMusic();
                if (!initPlayerMenu(&playerMenu, menu.window, menu.renderer)) {
                    printf("Erreur initialisation menu joueur\n");
                    state = STATE_MENU;
                    if (menu.music) Mix_PlayMusic(menu.music, -1);
                    continue;
                }
                if (!loadPlayerMenuResources(&playerMenu)) {
                    printf("Erreur chargement ressources menu joueur\n");
                    cleanupPlayerMenu(&playerMenu);
                    state = STATE_MENU;
                    if (menu.music) Mix_PlayMusic(menu.music, -1);
                    continue;
                }
                playerMenuInitialized = true;
            }

            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    menu.running = false;
                    state = STATE_QUIT;
                    break;
                }
                
                bool goToScore = handlePlayerMenuEvents(&playerMenu, &e, &menu.running);
                if (goToScore) {
                    playerScore = playerMenu.playerScore;
                    state = STATE_SCORES;
                    break;
                }
                
                if (e.type == SDL_KEYDOWN && e.key.repeat == 0 &&
                    e.key.keysym.sym == SDLK_ESCAPE) {
                    state = STATE_MENU;
                    if (menu.music) Mix_PlayMusic(menu.music, -1);
                    break;
                }
            }

            if (state != STATE_PLAYER) {
                if (state == STATE_MENU || state == STATE_QUIT) {
                    cleanupPlayerMenu(&playerMenu);
                    playerMenuInitialized = false;
                }
                SDL_Delay(16);
                continue;
            }

            updatePlayerMenu(&playerMenu);
            renderPlayerMenu(&playerMenu);
        }

        /* ---- ÉCRAN SCORES ---- */
        else if (state == STATE_SCORES)
        {
            Mix_HaltMusic();
            int final_score = playerScore > 0 ? playerScore : 123;
            int result = scoreMenuLoop(menu.window, menu.renderer, final_score);

            if (playerMenuInitialized) {
                cleanupPlayerMenu(&playerMenu);
                playerMenuInitialized = false;
            }

            if (menu.music) Mix_PlayMusic(menu.music, -1);
            if (result == 2) state = STATE_MENU;
            else state = STATE_QUIT;
        }

        /* ---- ÉCRAN ENIGME ---- */
        else if (state == STATE_ENIGME)
        {
            if (!enigmeInit) {
                Mix_HaltMusic();
                initEnigme(&enigme, menu.renderer);
                fontEnigme      = TTF_OpenFont("assets/font/font.ttf", 32);
                fontEnigmeSmall = TTF_OpenFont("assets/font/font.ttf", 22);
                enigmeInit = 1;
            }

            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    menu.running = false;
                    state = STATE_QUIT;
                    break;
                }
                if (e.type == SDL_KEYDOWN && e.key.repeat == 0 &&
                    e.key.keysym.sym == SDLK_ESCAPE)
                {
                    freeEnigme(&enigme);
                    if (fontEnigme)      { TTF_CloseFont(fontEnigme);      fontEnigme = NULL; }
                    if (fontEnigmeSmall) { TTF_CloseFont(fontEnigmeSmall); fontEnigmeSmall = NULL; }
                    enigmeInit = 0;
                    if (menu.music) Mix_PlayMusic(menu.music, -1);
                    state = STATE_MENU;
                    break;
                }
                handleEnigmeEvent(&enigme, &e);
            }

            if (state != STATE_ENIGME) {
                SDL_Delay(16);
                continue;
            }

            if (enigme.questionIndex >= NB_QUESTIONS) {
                SDL_Delay(2000);
                freeEnigme(&enigme);
                if (fontEnigme)      { TTF_CloseFont(fontEnigme);      fontEnigme = NULL; }
                if (fontEnigmeSmall) { TTF_CloseFont(fontEnigmeSmall); fontEnigmeSmall = NULL; }
                enigmeInit = 0;
                if (menu.music) Mix_PlayMusic(menu.music, -1);
                state = STATE_MENU;
            } else {
                updateEnigme(&enigme);
                SDL_RenderClear(menu.renderer);
                renderEnigme(&enigme, menu.renderer, fontEnigme, fontEnigmeSmall);
                SDL_RenderPresent(menu.renderer);
            }
        }

        SDL_Delay(16);
    }

    if (playerMenuInitialized) cleanupPlayerMenu(&playerMenu);
    if (optionsInit) freeOptions(&options);
    if (enigmeInit) {
        freeEnigme(&enigme);
        if (fontEnigme)      TTF_CloseFont(fontEnigme);
        if (fontEnigmeSmall) TTF_CloseFont(fontEnigmeSmall);
    }

    destroy(&menu);
    return 0;
}
