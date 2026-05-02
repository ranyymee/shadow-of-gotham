#include "menu.h"
#include <stdio.h>

// Fonction utilitaire pour charger une texture
SDL_Texture* loadTexture(SDL_Renderer *renderer, const char *path) {
    SDL_Texture *texture = IMG_LoadTexture(renderer, path);
    if (!texture) {
        printf("Erreur chargement texture %s: %s\n", path, IMG_GetError());
    }
    return texture;
}

// Fonction pour rendre le texte centré
void renderTextCentered(SDL_Renderer *renderer, TTF_Font *font, 
                               const char *text, SDL_Color color, SDL_Rect rect) {
    if (!text) return;
    
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

// Fonction pour jouer un son au survol
void playHoverSound(Mix_Chunk *sound) {
    if (sound) {
        Mix_PlayChannel(-1, sound, 0);
    }
}

bool initMenu(Menu *menu) {
    // Initialisation des SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("Erreur SDL_Init: %s\n", SDL_GetError());
        return false;
    }
    
    if (IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) == 0) {
        printf("Erreur IMG_Init: %s\n", IMG_GetError());
        SDL_Quit();
        return false;
    }
    
    if (TTF_Init() == -1) {
        printf("Erreur TTF_Init: %s\n", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    
    if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) == 0) {
        printf("Erreur Mix_Init: %s\n", Mix_GetError());
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Erreur Mix_OpenAudio: %s\n", Mix_GetError());
        Mix_Quit();
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    
    // Création de la fenêtre
    menu->windowWidth = 800;
    menu->windowHeight = 600;
    
    menu->window = SDL_CreateWindow("Sous Menu Joueur", 
                                    SDL_WINDOWPOS_CENTERED, 
                                    SDL_WINDOWPOS_CENTERED, 
                                    menu->windowWidth, 
                                    menu->windowHeight, 
                                    SDL_WINDOW_SHOWN);
    if (!menu->window) {
        printf("Erreur création fenêtre: %s\n", SDL_GetError());
        Mix_CloseAudio();
        Mix_Quit();
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    
    menu->renderer = SDL_CreateRenderer(menu->window, -1, SDL_RENDERER_ACCELERATED);
    if (!menu->renderer) {
        printf("Erreur création renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(menu->window);
        Mix_CloseAudio();
        Mix_Quit();
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
        return false;
    }
    
    // Initialisation des variables d'état
    menu->modeSelected = 0;
    menu->avatarSelection = 0;
    menu->inputSelection = 0;
    menu->showModeButtons = true;
    menu->lastHoveredButton = -1;
    menu->textColor = (SDL_Color){255, 255, 255, 255};
    
    return true;
}

bool loadMenuResources(Menu *menu) {
    // Charger les arrière-plans
    menu->backgroundTexture = loadTexture(menu->renderer, "assets/background.jpg");
    menu->background2Texture = loadTexture(menu->renderer, "assets/background2.jpg");
    
    // Charger la police
    menu->font = TTF_OpenFont("assets/font.ttf", 24);
    if (!menu->font) {
        printf("Erreur chargement police: %s\n", TTF_GetError());
        return false;
    }
    
    // Charger le son
    menu->hoverSound = Mix_LoadWAV("assets/hover_sound.wav");
    if (!menu->hoverSound) {
        printf("Attention: Impossible de charger le son de survol\n");
    }
    
    // Dimensions des boutons
    int buttonWidth = 400;
    int buttonHeight = 60;
    int smallButtonWidth = 200;
    int smallButtonHeight = 50;
    
    // Positions calculées
    int centerX = (menu->windowWidth - buttonWidth) / 2;
    int leftColumnX = 150;
    int rightColumnX = 450;
    
    // Créer les boutons (showTextOnButton = false car on utilise des images)
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

void handleMenuEvents(Menu *menu, SDL_Event *event, bool *quit) {
    if (event->type == SDL_QUIT) {
        *quit = true;
    }
    
    // Gestion des clics souris
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        
        if (menu->showModeButtons) {
            if (isMouseOverButton(&menu->monoButton, x, y)) {
                menu->modeSelected = 1;
                menu->showModeButtons = false;
                printf("Mode mono joueur sélectionné\n");
            } else if (isMouseOverButton(&menu->multiButton, x, y)) {
                menu->modeSelected = 2;
                menu->showModeButtons = false;
                printf("Mode multi joueur sélectionné\n");
            }
        } else {
            if (isMouseOverButton(&menu->avatar1Button, x, y)) {
                menu->avatarSelection = 1;
                printf("Avatar 1 sélectionné\n");
            } else if (isMouseOverButton(&menu->avatar2Button, x, y)) {
                menu->avatarSelection = 2;
                printf("Avatar 2 sélectionné\n");
            } else if (isMouseOverButton(&menu->input1Button, x, y)) {
                menu->inputSelection = 1;
                printf("Input 1 sélectionné\n");
            } else if (isMouseOverButton(&menu->input2Button, x, y)) {
                menu->inputSelection = 2;
                printf("Input 2 sélectionné\n");
            }
            
            if (isMouseOverButton(&menu->validateButton, x, y)) {
                printf("=== VALIDATION ===\n");
                printf("Mode: %s\n", menu->modeSelected == 1 ? "Mono joueur" : "Multi joueur");
                printf("Avatar sélectionné: %d\n", menu->avatarSelection);
                printf("Input sélectionné: %d\n", menu->inputSelection);
                printf("==================\n");
            }
        }
        
        if (isMouseOverButton(&menu->backButton, x, y)) {
            if (!menu->showModeButtons) {
                menu->modeSelected = 0;
                menu->avatarSelection = 0;
                menu->inputSelection = 0;
                menu->showModeButtons = true;
                printf("Retour au menu principal\n");
            }
        }
    }
    
    // Gestion des touches clavier
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_v:
                if (!menu->showModeButtons) {
                    printf("=== VALIDATION (touche V) ===\n");
                    printf("Mode: %s\n", menu->modeSelected == 1 ? "Mono joueur" : "Multi joueur");
                    printf("Avatar sélectionné: %d\n", menu->avatarSelection);
                    printf("Input sélectionné: %d\n", menu->inputSelection);
                    printf("=============================\n");
                }
                break;
            case SDLK_ESCAPE:
                *quit = true;
                break;
        }
    }
}

void updateMenu(Menu *menu) {
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
        playHoverSound(menu->hoverSound);
    }
    menu->lastHoveredButton = currentHoveredButton;
}

void renderMenu(Menu *menu) {
    SDL_RenderClear(menu->renderer);
    
    // Afficher l'arrière-plan approprié
    if (menu->showModeButtons) {
        if (menu->backgroundTexture) {
            SDL_RenderCopy(menu->renderer, menu->backgroundTexture, NULL, NULL);
        }
    } else {
        if (menu->background2Texture) {
            SDL_RenderCopy(menu->renderer, menu->background2Texture, NULL, NULL);
        }
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
        SDL_Rect titleRect = {(menu->windowWidth - 400) / 2, 80, 400, 50};
        renderTextCentered(menu->renderer, menu->font, modeText, titleColor, titleRect);
        
        // Sous-titres
        SDL_Color subColor = {200, 200, 200, 255};
        SDL_Rect avatarTitleRect = {(menu->windowWidth - 400) / 2, 150, 400, 30};
        SDL_Rect inputTitleRect = {(menu->windowWidth - 400) / 2, 250, 400, 30};
        renderTextCentered(menu->renderer, menu->font, "Choisissez votre avatar:", subColor, avatarTitleRect);
        renderTextCentered(menu->renderer, menu->font, "Choisissez votre input:", subColor, inputTitleRect);
        
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
        SDL_Color hintColor = {180, 180, 180, 255};
        SDL_Rect hintRect = {(menu->windowWidth - 400) / 2, 440, 400, 30};
        renderTextCentered(menu->renderer, menu->font, "Appuyez sur V pour valider", hintColor, hintRect);
    }
    
    // Bouton Retour (toujours affiché)
    renderButton(menu->renderer, &menu->backButton, 
                isMouseOverButton(&menu->backButton, mx, my), 
                menu->font, menu->textColor);
    
    SDL_RenderPresent(menu->renderer);
}

void cleanupMenu(Menu *menu) {
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
    if (menu->renderer) SDL_DestroyRenderer(menu->renderer);
    if (menu->window) SDL_DestroyWindow(menu->window);
    
    Mix_CloseAudio();
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
