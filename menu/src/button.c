#include "button.h"
#include <stdio.h>

// Fonction pour charger une texture
static SDL_Texture* loadTexture(SDL_Renderer *renderer, const char *path) {
    SDL_Texture *texture = IMG_LoadTexture(renderer, path);
    if (!texture) {
        printf("Erreur chargement texture %s: %s\n", path, IMG_GetError());
    }
    return texture;
}

// Fonction pour rendre le texte centré sur un rectangle
static void renderTextCentered(SDL_Renderer *renderer, TTF_Font *font, 
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

// Remplacer Button par PlayerButton
PlayerButton createButton(SDL_Renderer *renderer, int x, int y, int w, int h, 
                         const char *normalPath, const char *hoverPath, 
                         const char *buttonText, bool showTextOnButton) {
    PlayerButton btn;  // RENOMMÉ
    btn.rect.x = x;
    btn.rect.y = y;
    btn.rect.w = w;
    btn.rect.h = h;
    btn.normalTexture = loadTexture(renderer, normalPath);
    btn.hoverTexture = loadTexture(renderer, hoverPath);
    btn.text = buttonText;
    btn.showText = showTextOnButton;
    return btn;
}

void destroyButton(PlayerButton *btn) {  // RENOMMÉ
    if (btn->normalTexture) SDL_DestroyTexture(btn->normalTexture);
    if (btn->hoverTexture) SDL_DestroyTexture(btn->hoverTexture);
    btn->normalTexture = NULL;
    btn->hoverTexture = NULL;
}

void renderButton(SDL_Renderer *renderer, PlayerButton *btn, bool isHovered, 
                  TTF_Font *font, SDL_Color textColor) {
    if (isHovered && btn->hoverTexture) {
        SDL_RenderCopy(renderer, btn->hoverTexture, NULL, &btn->rect);
    } else if (btn->normalTexture) {
        SDL_RenderCopy(renderer, btn->normalTexture, NULL, &btn->rect);
    } else {
        SDL_SetRenderDrawColor(renderer, isHovered ? 100 : 50, 
                               isHovered ? 100 : 50, 200, 255);
        SDL_RenderFillRect(renderer, &btn->rect);
    }
    
    if (btn->showText && btn->text && font) {
        renderTextCentered(renderer, font, btn->text, textColor, btn->rect);
    }
}

bool isMouseOverButton(PlayerButton *btn, int mouseX, int mouseY) {
    SDL_Point point = {mouseX, mouseY};
    return SDL_PointInRect(&point, &btn->rect);
}
