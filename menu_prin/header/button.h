#ifndef BUTTON_H
#define BUTTON_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

// RENOMMER de "Button" à "PlayerButton"
typedef struct {
    SDL_Rect rect;
    SDL_Texture *normalTexture;
    SDL_Texture *hoverTexture;
    const char *text;
    bool showText;
} PlayerButton;  // RENOMMÉ

// Crée un nouveau bouton
PlayerButton createButton(SDL_Renderer *renderer, int x, int y, int w, int h, 
                         const char *normalPath, const char *hoverPath, 
                         const char *buttonText, bool showTextOnButton);

// Détruit un bouton et libère ses textures
void destroyButton(PlayerButton *btn);

// Rendu d'un bouton avec gestion du survol
void renderButton(SDL_Renderer *renderer, PlayerButton *btn, bool isHovered, 
                  TTF_Font *font, SDL_Color textColor);

// Vérifie si la souris est sur le bouton
bool isMouseOverButton(PlayerButton *btn, int mouseX, int mouseY);

#endif // BUTTON_H
