#ifndef HEADER_H
#define HEADER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>

#define FRAME_COUNT   60
#define BUTTON_COUNT  5
#define SCREEN_W 1280
#define SCREEN_H  720

typedef enum GameState {
    STATE_MENU, STATE_SAUVEGARDE, STATE_PLAYER,
    STATE_SCORES, STATE_ENIGME, STATE_OPTIONS, STATE_QUIT
} GameState;

typedef enum { PLAY, OPTIONS, SCORES, HISTORY, QUITTER } Action;

typedef struct {
    SDL_Texture *normal;
    SDL_Texture *hover;
    SDL_Rect     rect;
    int          state;
} Button;

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    bool          running;
    SDL_Texture  *frames[FRAME_COUNT];
    int           currentFrame;
    int           bgDirection;
    Button        buttons[BUTTON_COUNT];
    SDL_Texture  *logo;
    SDL_Rect      logoRect;
    SDL_Texture  *textShadow;
    SDL_Rect      textShadowRect;
    SDL_Texture  *textOfGotham;
    SDL_Rect      textOfGothamRect;
    TTF_Font     *font;
    Mix_Chunk    *clickSound;
    Mix_Music    *music;
} Menu;

int  init          (Menu *menu);
void loadBackground(Menu *menu);
void loadButtons   (Menu *menu);
void loadAssets    (Menu *menu);
void events        (Menu *menu);
void update        (Menu *menu);
void render        (Menu *menu);
void destroy       (Menu *menu);
void action        (Menu *menu, Action a);

#endif
