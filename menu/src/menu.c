#include "menu.h"
#include <stdio.h>

// ============ FONCTIONS LOCALES ============

// Fonction pour charger une texture (locale)
static SDL_Texture* loadTextureLocal(const char *path, SDL_Renderer *renderer) {
    SDL_Texture *texture = IMG_LoadTexture(renderer, path);
    if (!texture) {
        printf("Erreur chargement texture %s: %s\n", path, IMG_GetError());
    }
    return texture;
}

// Fonction pour jouer un son au survol (locale)
static void playHoverSoundLocal(Mix_Chunk *sound) {
    if (sound) {
        Mix_PlayChannel(-1, sound, 0);
    }
}

// Fonction pour rendre le texte centré (locale)
static void renderTextCenteredLocal(SDL_Renderer *renderer, TTF_Font *font, 
                                    const char *text, SDL_Color color, SDL_Rect rect) {
    if (!text || !font) return;
    
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, color);
    if (!textSurface) {
        printf("Erreur TTF_RenderText_Solid: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        printf("Erreur SDL_CreateTextureFromSurface: %s\n", SDL_GetError());
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect textRect;
    textRect.w = textSurface->w;
    textRect.h = textSurface->h;
    textRect.x = rect.x + (rect.w - textRect.w) / 2;
    textRect.y = rect.y + (rect.h - textRect.h) / 2;

    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

// ============ FONCTIONS DU MENU ============

bool initPlayerMenu(PlayerMenu *menu, SDL_Window *window, SDL_Renderer *renderer) {
    menu->window = window;
    menu->renderer = renderer;
    
    SDL_GetWindowSize(window, &menu->windowWidth, &menu->windowHeight);
    
    menu->modeSelected = 0;
    menu->avatarSelection = 0;
    menu->inputSelection = 0;
    menu->showModeButtons = true;
    menu->lastHoveredButton = -1;
    menu->playerScore = 0;
    menu->textColor = (SDL_Color){255, 255, 255, 255};
    
    return true;
}

bool loadPlayerMenuResources(PlayerMenu *menu) {
    // Utiliser la fonction locale
    menu->backgroundTexture = loadTextureLocal("assets/background.jpg", menu->renderer);
    menu->background2Texture = loadTextureLocal("assets/background2.jpg", menu->renderer);
    
    menu->font = TTF_OpenFont("assets/font/font.ttf", 24);
    if (!menu->font) {
        printf("Erreur chargement police: %s\n", TTF_GetError());
        menu->font = TTF_OpenFont("assets/font.ttf", 24);
    }
    
    menu->hoverSound = Mix_LoadWAV("assets/audio/hover_sound.wav");
    if (!menu->hoverSound) {
        printf("Erreur chargement son: %s\n", Mix_GetError());
    }
    
    // Dimensions des boutons (comme dans l'ancien code)
    int buttonWidth = 400;
    int buttonHeight = 60;
    int smallButtonWidth = 200;
    int smallButtonHeight = 50;
    
    // Positions centrées (comme dans l'ancien code)
    int centerX = (menu->windowWidth - buttonWidth) / 2;
    int leftColumnX = (menu->windowWidth / 2) - smallButtonWidth - 25;  // 25px d'espace
    int rightColumnX = (menu->windowWidth / 2) + 25;                     // 25px d'espace
    
    // Créer les boutons avec les positions de l'ancien code
    menu->monoButton = createButton(menu->renderer, centerX, 200, buttonWidth, buttonHeight,
                                    "assets/button_mono_normal.png",
                                    "assets/button_mono_hover.png",
                                    "Mono joueur", false);
    
    menu->multiButton = createButton(menu->renderer, centerX, 300, buttonWidth, buttonHeight,
                                     "assets/button_multi_normal.png",
                                     "assets/button_multi_hover.png",
                                     "Multi joueur", false);
    
    menu->avatar1Button = createButton(menu->renderer, leftColumnX, 200, smallButtonWidth, smallButtonHeight,
                                       "assets/button_avatar1_normal.png",
                                       "assets/button_avatar1_hover.png",
                                       "Avatar 1", false);
    
    menu->avatar2Button = createButton(menu->renderer, rightColumnX, 200, smallButtonWidth, smallButtonHeight,
                                       "assets/button_avatar2_normal.png",
                                       "assets/button_avatar2_hover.png",
                                       "Avatar 2", false);
    
    menu->input1Button = createButton(menu->renderer, leftColumnX, 280, smallButtonWidth, smallButtonHeight,
                                      "assets/button_input1_normal.png",
                                      "assets/button_input1_hover.png",
                                      "Input 1", false);
    
    menu->input2Button = createButton(menu->renderer, rightColumnX, 280, smallButtonWidth, smallButtonHeight,
                                      "assets/button_input2_normal.png",
                                      "assets/button_input2_hover.png",
                                      "Input 2", false);
    
    menu->validateButton = createButton(menu->renderer, centerX, 370, buttonWidth, buttonHeight,
                                        "assets/button_valider_normal.png",
                                        "assets/button_valider_hover.png",
                                        "Valider", false);
    
    menu->backButton = createButton(menu->renderer, centerX, 500, buttonWidth, buttonHeight,
                                    "assets/button_retour_normal.png",
                                    "assets/button_retour_hover.png",
                                    "Retour", false);
    
    return true;
}

bool handlePlayerMenuEvents(PlayerMenu *menu, SDL_Event *event, bool *quit) {
    if (event->type == SDL_QUIT) {
        *quit = true;
        return false;
    }
    
    // Gestion des clics souris
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        
        if (menu->showModeButtons) {
            if (isMouseOverButton(&menu->monoButton, x, y)) {
                menu->modeSelected = 1;
                menu->showModeButtons = false;
            } else if (isMouseOverButton(&menu->multiButton, x, y)) {
                menu->modeSelected = 2;
                menu->showModeButtons = false;
            }
        } else {
            if (isMouseOverButton(&menu->avatar1Button, x, y)) {
                menu->avatarSelection = 1;
            } else if (isMouseOverButton(&menu->avatar2Button, x, y)) {
                menu->avatarSelection = 2;
            } else if (isMouseOverButton(&menu->input1Button, x, y)) {
                menu->inputSelection = 1;
            } else if (isMouseOverButton(&menu->input2Button, x, y)) {
                menu->inputSelection = 2;
            }
            
            // VALIDATION - passage au menu score
            if (isMouseOverButton(&menu->validateButton, x, y)) {
                printf("Validation: Mode %d, Avatar %d, Input %d\n", 
                       menu->modeSelected, menu->avatarSelection, menu->inputSelection);
                
                // Calculer un score basé sur les sélections
                menu->playerScore = menu->modeSelected * 100 + 
                                   menu->avatarSelection * 50 + 
                                   menu->inputSelection * 25;
                
                return true; // Indique qu'on doit passer au menu score
            }
        }
        
        if (isMouseOverButton(&menu->backButton, x, y)) {
            menu->modeSelected = 0;
            menu->avatarSelection = 0;
            menu->inputSelection = 0;
            menu->showModeButtons = true;
        }
    }
    
    // Gestion des touches clavier
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_v:
                if (!menu->showModeButtons) {
                    printf("Validation (touche V): Mode %d, Avatar %d, Input %d\n", 
                           menu->modeSelected, menu->avatarSelection, menu->inputSelection);
                    
                    menu->playerScore = menu->modeSelected * 100 + 
                                       menu->avatarSelection * 50 + 
                                       menu->inputSelection * 25;
                    
                    return true;
                }
                break;
            case SDLK_ESCAPE:
                *quit = true;
                break;
        }
    }
    
    return false;
}

void updatePlayerMenu(PlayerMenu *menu) {
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    
    int currentHoveredButton = -1;
    
    if (menu->showModeButtons) {
        if (isMouseOverButton(&menu->monoButton, mx, my)) currentHoveredButton = 0;
        if (isMouseOverButton(&menu->multiButton, mx, my)) currentHoveredButton = 1;
    } else {
        if (isMouseOverButton(&menu->avatar1Button, mx, my)) currentHoveredButton = 2;
        if (isMouseOverButton(&menu->avatar2Button, mx, my)) currentHoveredButton = 3;
        if (isMouseOverButton(&menu->input1Button, mx, my)) currentHoveredButton = 4;
        if (isMouseOverButton(&menu->input2Button, mx, my)) currentHoveredButton = 5;
        if (isMouseOverButton(&menu->validateButton, mx, my)) currentHoveredButton = 6;
    }
    
    if (isMouseOverButton(&menu->backButton, mx, my)) currentHoveredButton = 7;
    
    // Jouer le son si nouveau bouton survolé
    if (currentHoveredButton != -1 && currentHoveredButton != menu->lastHoveredButton) {
        playHoverSoundLocal(menu->hoverSound);
    }
    menu->lastHoveredButton = currentHoveredButton;
}

void renderPlayerMenu(PlayerMenu *menu) {
    SDL_RenderClear(menu->renderer);
    
    // Afficher l'arrière-plan approprié
    if (menu->showModeButtons) {
        if (menu->backgroundTexture)
            SDL_RenderCopy(menu->renderer, menu->backgroundTexture, NULL, NULL);
    } else {
        if (menu->background2Texture)
            SDL_RenderCopy(menu->renderer, menu->background2Texture, NULL, NULL);
    }
    
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    
    // Afficher les boutons selon l'état
    if (menu->showModeButtons) {
        renderButton(menu->renderer, &menu->monoButton, 
                    isMouseOverButton(&menu->monoButton, mx, my), 
                    menu->font, menu->textColor);
        renderButton(menu->renderer, &menu->multiButton, 
                    isMouseOverButton(&menu->multiButton, mx, my), 
                    menu->font, menu->textColor);
    } else {
        // Titre du mode choisi
        const char *modeText = (menu->modeSelected == 1) ? "Mode: Mono joueur" : "Mode: Multi joueur";
        SDL_Color titleColor = {255, 255, 0, 255};
        SDL_Rect titleRect = {(menu->windowWidth - 400) / 2, 100, 400, 50};
        if (menu->font)
            renderTextCenteredLocal(menu->renderer, menu->font, modeText, titleColor, titleRect);
        
        // Afficher les boutons de sélection
        renderButton(menu->renderer, &menu->avatar1Button, 
                    isMouseOverButton(&menu->avatar1Button, mx, my), 
                    menu->font, menu->textColor);
        renderButton(menu->renderer, &menu->avatar2Button, 
                    isMouseOverButton(&menu->avatar2Button, mx, my), 
                    menu->font, menu->textColor);
        renderButton(menu->renderer, &menu->input1Button, 
                    isMouseOverButton(&menu->input1Button, mx, my), 
                    menu->font, menu->textColor);
        renderButton(menu->renderer, &menu->input2Button, 
                    isMouseOverButton(&menu->input2Button, mx, my), 
                    menu->font, menu->textColor);
        renderButton(menu->renderer, &menu->validateButton, 
                    isMouseOverButton(&menu->validateButton, mx, my), 
                    menu->font, menu->textColor);
        
        // Indication touche V
        SDL_Color hintColor = {200, 200, 200, 255};
        SDL_Rect hintRect = {(menu->windowWidth - 400) / 2, 440, 400, 30};
        if (menu->font)
            renderTextCenteredLocal(menu->renderer, menu->font, "Appuyez sur V pour valider", hintColor, hintRect);
    }
    
    // Bouton Retour (toujours affiché)
    renderButton(menu->renderer, &menu->backButton, 
                isMouseOverButton(&menu->backButton, mx, my), 
                menu->font, menu->textColor);
    
    SDL_RenderPresent(menu->renderer);
}

void cleanupPlayerMenu(PlayerMenu *menu) {
    // Libérer les ressources
    if (menu->hoverSound) Mix_FreeChunk(menu->hoverSound);
    
    destroyButton(&menu->monoButton);
    destroyButton(&menu->multiButton);
    destroyButton(&menu->avatar1Button);
    destroyButton(&menu->avatar2Button);
    destroyButton(&menu->input1Button);
    destroyButton(&menu->input2Button);
    destroyButton(&menu->validateButton);
    destroyButton(&menu->backButton);
    
    if (menu->font) TTF_CloseFont(menu->font);
    if (menu->background2Texture) SDL_DestroyTexture(menu->background2Texture);
    if (menu->backgroundTexture) SDL_DestroyTexture(menu->backgroundTexture);
    
    // Ne pas détruire window et renderer ici (ils sont partagés)
}
