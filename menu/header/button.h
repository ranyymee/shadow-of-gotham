#ifndef BUTTON_H
#define BUTTON_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

/* ─────────────────────────────────────────────────────────────────────────
   PlayerButton  — field names match button.c implementation
   ───────────────────────────────────────────────────────────────────────── */
typedef struct {
    SDL_Rect     rect;
    SDL_Texture *normalTexture;
    SDL_Texture *hoverTexture;
    const char  *text;
    bool         showText;
    int          state;      /* 0 = normal, 1 = hovered */
} PlayerButton;

/* ─────────────────────────────────────────────────────────────────────────
   Function declarations  (implemented in src/button.c)
   ───────────────────────────────────────────────────────────────────────── */
PlayerButton createButton     (SDL_Renderer *renderer,
                                int x, int y, int w, int h,
                                const char *normalPath,
                                const char *hoverPath,
                                const char *buttonText,
                                bool showTextOnButton);

void         destroyButton    (PlayerButton *btn);

void         renderButton     (SDL_Renderer *renderer,
                                PlayerButton *btn,
                                bool isHovered,
                                TTF_Font *font,
                                SDL_Color textColor);

bool         isMouseOverButton(PlayerButton *btn, int mouseX, int mouseY);

#endif /* BUTTON_H */
