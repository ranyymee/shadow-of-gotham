#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>
#include "button.h"

// Structure principale du menu
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
    
    // Boutons
    Button monoButton;
    Button multiButton;
    Button avatar1Button;
    Button avatar2Button;
    Button input1Button;
    Button input2Button;
    Button validateButton;
    Button backButton;
    
    // État du menu
    int modeSelected;
    int avatarSelection;
    int inputSelection;
    bool showModeButtons;
    
    // Gestion du survol
    int lastHoveredButton;
} Menu;

// Initialise le menu
bool initMenu(Menu *menu);

// Charge les ressources du menu
bool loadMenuResources(Menu *menu);

// Gère les événements
void handleMenuEvents(Menu *menu, SDL_Event *event, bool *quit);

// Met à jour le menu
void updateMenu(Menu *menu);

// Rend le menu
void renderMenu(Menu *menu);

// Nettoie le menu
void cleanupMenu(Menu *menu);

#endif // MENU_H
