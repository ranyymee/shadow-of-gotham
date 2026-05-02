#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>
#include "button.h"

typedef struct {
    // Fenêtre et renderer
    SDL_Window *window;
    SDL_Renderer *renderer;
    
    // Dimensions
    int windowWidth;
    int windowHeight;
    
    // Textures d'arrière-plan
    SDL_Texture *backgroundTexture;
    SDL_Texture *background2Texture;
    
    // Police
    TTF_Font *font;
    SDL_Color textColor;
    
    // Son
    Mix_Chunk *hoverSound;
    
    // Boutons - Remplacer Button par PlayerButton
    PlayerButton monoButton;
    PlayerButton multiButton;
    PlayerButton avatar1Button;
    PlayerButton avatar2Button;
    PlayerButton input1Button;
    PlayerButton input2Button;
    PlayerButton validateButton;
    PlayerButton backButton;
    
    // État du menu
    int modeSelected;
    int avatarSelection;
    int inputSelection;
    bool showModeButtons;
    
    // Gestion du survol
    int lastHoveredButton;
    
    // Données à passer au menu score
    int playerScore;
} PlayerMenu;

// Initialise le menu avec la fenêtre et le renderer existants
bool initPlayerMenu(PlayerMenu *menu, SDL_Window *window, SDL_Renderer *renderer);

// Charge les ressources du menu
bool loadPlayerMenuResources(PlayerMenu *menu);

// Gère les événements (retourne true si on doit passer au score)
bool handlePlayerMenuEvents(PlayerMenu *menu, SDL_Event *event, bool *quit);

// Met à jour le menu
void updatePlayerMenu(PlayerMenu *menu);

// Rend le menu
void renderPlayerMenu(PlayerMenu *menu);

// Nettoie le menu
void cleanupPlayerMenu(PlayerMenu *menu);

#endif // MENU_H
