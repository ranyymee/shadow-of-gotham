#include "sauvegarde.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

static SDL_Texture* backgroundFrames[60]; // Increased size to hold 51+ frames
static SDL_Texture* textTexture = NULL;
static SDL_Texture* btnYesTex = NULL;
static SDL_Texture* btnYesHoverTex = NULL;
static SDL_Texture* btnNoTex = NULL;
static SDL_Texture* btnNoHoverTex = NULL;

static Mix_Chunk* clickSound = NULL;
static TTF_Font* menuFont = NULL;

static int currentFrame = 0, totalFrames = 0, frameDelayCounter = 0;
static int mouseX = 0, mouseY = 0;

// Button and Text Positions
static SDL_Rect textRect  = {350, 80, 500, 80}; 
static SDL_Rect btnYesRect = {300, 350, 300, 120}; 
static SDL_Rect btnNoRect  = {650, 350, 300, 120}; 

void initialiser_sauvegarde(SDL_Renderer *renderer) {
    char path[150];
    totalFrames = 0;

    // 1. Loading New Animation Frames (ezgif-frame-001.png to ezgif-frame-051.png)
    for (int i = 1; i <= 51; i++) {
        // %03d makes the number 3 digits (1 becomes 001)
        sprintf(path, "assets/images/ezgif-frame-%03d.png", i); 
        
        SDL_Surface* surf = IMG_Load(path);
        if (surf) {
            backgroundFrames[totalFrames] = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_FreeSurface(surf);
            totalFrames++;
        } else {
            // Log error if a frame is missing to help debugging
            if (i == 1) printf("Error: Could not find first frame at %s\n", path);
            break; 
        }
    }

    // 2. Load Font and "SAVE PROGRESS?" text
    if (TTF_WasInit() || TTF_Init() != -1) {
        menuFont = TTF_OpenFont("assets/font/font.ttf", 48);
        if (menuFont) {
            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface* textSurf = TTF_RenderText_Blended(menuFont, "SAVE PROGRESS?", white);
            textTexture = SDL_CreateTextureFromSurface(renderer, textSurf);
            SDL_FreeSurface(textSurf);
        }
    }

    // 3. Load Batman Button Assets
    btnYesTex      = IMG_LoadTexture(renderer, "assets/images/yes.png");
    btnYesHoverTex = IMG_LoadTexture(renderer, "assets/images/yes_hover.png");
    btnNoTex       = IMG_LoadTexture(renderer, "assets/images/no.png");
    btnNoHoverTex  = IMG_LoadTexture(renderer, "assets/images/no_hover.png");

    // 4. Load Sound
    clickSound = Mix_LoadWAV("assets/sounds/click.wav");
}

void gerer_evenement_sauvegarde(SDL_Event event, GameState *state) {
    // Track mouse for hover and clicks
    if (event.type == SDL_MOUSEMOTION) {
        mouseX = event.motion.x;
        mouseY = event.motion.y;
    }

    // Keyboard Logic (Fix for 'n' key)
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_n) {
            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
            *state = STATE_PLAYER; 
        }
        if (event.key.keysym.sym == SDLK_ESCAPE) {
            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
            *state = STATE_MENU;
        }
    }

    // Mouse Button Logic
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (mouseX >= btnYesRect.x && mouseX <= (btnYesRect.x + btnYesRect.w) &&
            mouseY >= btnYesRect.y && mouseY <= (btnYesRect.y + btnYesRect.h)) {
            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
            *state = STATE_PLAYER; 
        }
        if (mouseX >= btnNoRect.x && mouseX <= (btnNoRect.x + btnNoRect.w) &&
            mouseY >= btnNoRect.y && mouseY <= (btnNoRect.y + btnNoRect.h)) {
            if (clickSound) Mix_PlayChannel(-1, clickSound, 0);
            *state = STATE_MENU; 
        }
    }
}

void afficher_sous_menu_sauvegarde(SDL_Renderer *renderer) {
    // LAYER 1: Background Animation
    if (totalFrames > 0) {
        SDL_RenderCopy(renderer, backgroundFrames[currentFrame], NULL, NULL);
        if (++frameDelayCounter > 4) { // Adjusted speed for 51 frames
            currentFrame = (currentFrame + 1) % totalFrames;
            frameDelayCounter = 0;
        }
    }

    // LAYER 2: Text
    if (textTexture) SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    // LAYER 3: Buttons (with Hover)
    // YES Button
    if (mouseX >= btnYesRect.x && mouseX <= (btnYesRect.x + btnYesRect.w) &&
        mouseY >= btnYesRect.y && mouseY <= (btnYesRect.y + btnYesRect.h)) {
        SDL_RenderCopy(renderer, btnYesHoverTex, NULL, &btnYesRect);
    } else {
        SDL_RenderCopy(renderer, btnYesTex, NULL, &btnYesRect);
    }

    // NO Button
    if (mouseX >= btnNoRect.x && mouseX <= (btnNoRect.x + btnNoRect.w) &&
        mouseY >= btnNoRect.y && mouseY <= (btnNoRect.y + btnNoRect.h)) {
        SDL_RenderCopy(renderer, btnNoHoverTex, NULL, &btnNoRect);
    } else {
        SDL_RenderCopy(renderer, btnNoTex, NULL, &btnNoRect);
    }
}

void nettoyer_sauvegarde() {
    for (int i = 0; i < totalFrames; i++) {
        if (backgroundFrames[i]) SDL_DestroyTexture(backgroundFrames[i]);
    }
    if (textTexture) SDL_DestroyTexture(textTexture);
    if (btnYesTex) SDL_DestroyTexture(btnYesTex);
    if (btnYesHoverTex) SDL_DestroyTexture(btnYesHoverTex);
    if (btnNoTex) SDL_DestroyTexture(btnNoTex);
    if (btnNoHoverTex) SDL_DestroyTexture(btnNoHoverTex);
    if (menuFont) TTF_CloseFont(menuFont);
    if (clickSound) Mix_FreeChunk(clickSound);
}
