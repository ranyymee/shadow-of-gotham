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

// Fonction pour redimensionner une texture à une taille cible
static SDL_Texture* resizeTexture(SDL_Renderer *renderer, SDL_Texture *original, int targetW, int targetH) {
    if (!original) return NULL;
    
    // Obtenir les dimensions originales
    int origW, origH;
    SDL_QueryTexture(original, NULL, NULL, &origW, &origH);
    
    // Si déjà à la bonne taille, retourner la texture originale
    if (origW == targetW && origH == targetH) {
        return original;
    }
    
    // Créer une nouvelle texture redimensionnée avec transparence
    SDL_Texture *resized = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, 
                                              SDL_TEXTUREACCESS_TARGET, targetW, targetH);
    if (!resized) {
        printf("Erreur création texture redimensionnée: %s\n", SDL_GetError());
        return original;
    }
    
    // Définir le mode de blending pour la transparence
    SDL_SetTextureBlendMode(resized, SDL_BLENDMODE_BLEND);
    
    // Sauvegarder la cible de rendu actuelle
    SDL_Texture *oldTarget = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, resized);
    
    // Remplir avec du transparent (au lieu de noir)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    
    // Rendre la texture originale redimensionnée
    SDL_Rect destRect = {0, 0, targetW, targetH};
    SDL_RenderCopy(renderer, original, NULL, &destRect);
    
    // Restaurer la cible
    SDL_SetRenderTarget(renderer, oldTarget);
    
    return resized;
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

Button createButton(SDL_Renderer *renderer, int x, int y, int w, int h, 
                   const char *normalPath, const char *hoverPath, 
                   const char *buttonText, bool showTextOnButton) {
    Button btn;
    btn.rect.x = x;
    btn.rect.y = y;
    btn.rect.w = w;
    btn.rect.h = h;
    
    // Charger les textures originales
    SDL_Texture *normalTex = loadTexture(renderer, normalPath);
    SDL_Texture *hoverTex = loadTexture(renderer, hoverPath);
    
    // Redimensionner les textures à la taille du bouton
    btn.normalTexture = resizeTexture(renderer, normalTex, w, h);
    btn.hoverTexture = resizeTexture(renderer, hoverTex, w, h);
    
    // Libérer les textures originales si elles ont été redimensionnées (différentes)
    if (btn.normalTexture != normalTex && normalTex) {
        SDL_DestroyTexture(normalTex);
    }
    if (btn.hoverTexture != hoverTex && hoverTex) {
        SDL_DestroyTexture(hoverTex);
    }
    
    // S'assurer que les textures utilisent la transparence
    if (btn.normalTexture) {
        SDL_SetTextureBlendMode(btn.normalTexture, SDL_BLENDMODE_BLEND);
    }
    if (btn.hoverTexture) {
        SDL_SetTextureBlendMode(btn.hoverTexture, SDL_BLENDMODE_BLEND);
    }
    
    btn.text = buttonText;
    btn.showText = showTextOnButton;
    
    return btn;
}

void destroyButton(Button *btn) {
    if (btn->normalTexture) SDL_DestroyTexture(btn->normalTexture);
    if (btn->hoverTexture) SDL_DestroyTexture(btn->hoverTexture);
    btn->normalTexture = NULL;
    btn->hoverTexture = NULL;
}

void renderButton(SDL_Renderer *renderer, Button *btn, bool isHovered, 
                  TTF_Font *font, SDL_Color textColor) {
    if (isHovered && btn->hoverTexture) {
        // Rendre la texture hover dans le rectangle du bouton (même taille)
        SDL_RenderCopy(renderer, btn->hoverTexture, NULL, &btn->rect);
    } else if (btn->normalTexture) {
        // Rendre la texture normale dans le rectangle du bouton (même taille)
        SDL_RenderCopy(renderer, btn->normalTexture, NULL, &btn->rect);
    } else {
        // Fallback: dessiner un rectangle coloré (sans transparence)
        SDL_SetRenderDrawColor(renderer, isHovered ? 100 : 50, isHovered ? 150 : 50, 200, 255);
        SDL_RenderFillRect(renderer, &btn->rect);
        // Bordure
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &btn->rect);
    }
    
    // Afficher le texte si demandé
    if (btn->showText && btn->text && font) {
        renderTextCentered(renderer, font, btn->text, textColor, btn->rect);
    }
}

bool isMouseOverButton(Button *btn, int mouseX, int mouseY) {
    SDL_Point point = {mouseX, mouseY};
    return SDL_PointInRect(&point, &btn->rect);
}
