#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>
#include "button.h"

/* ─── Player selection menu ─── */
typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    int           windowWidth;
    int           windowHeight;

    SDL_Texture  *backgroundTexture;
    SDL_Texture  *background2Texture;
    TTF_Font     *font;
    SDL_Color     textColor;
    Mix_Chunk    *hoverSound;

    PlayerButton  monoButton;
    PlayerButton  multiButton;
    PlayerButton  avatar1Button;
    PlayerButton  avatar2Button;
    PlayerButton  input1Button;
    PlayerButton  input2Button;
    PlayerButton  validateButton;
    PlayerButton  backButton;

    int           modeSelected;
    int           avatarSelection;
    int           inputSelection;
    bool          showModeButtons;
    int           lastHoveredButton;
    int           playerScore;
} PlayerMenu;

/* ─── Functions ─── */
bool initPlayerMenu(PlayerMenu *menu, SDL_Window *window, SDL_Renderer *renderer);
bool loadPlayerMenuResources(PlayerMenu *menu);
bool handlePlayerMenuEvents(PlayerMenu *menu, SDL_Event *event, bool *quit);
void updatePlayerMenu(PlayerMenu *menu);
void renderPlayerMenu(PlayerMenu *menu);
void cleanupPlayerMenu(PlayerMenu *menu);
