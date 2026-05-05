#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

/* ==================== CONSTANTES ==================== */
#define FRAME_COUNT   102
#define BUTTON_COUNT  5

#ifndef MAX_SCORES
#define MAX_SCORES    10
#endif
#ifndef MAX_NAME
#define MAX_NAME      100
#endif
#ifndef MAX_FRAMES
#define MAX_FRAMES    50
#endif

/* ==================== GAME STATE (Main.c) ==================== */
typedef enum {
    STATE_MENU,
    STATE_SAUVEGARDE,
    STATE_OPTIONS,
    STATE_PLAYER,
    STATE_SCORES,
    STATE_ENIGME,
    STATE_QUIT
} GameState;

/* ==================== BOUTON DU MENU PRINCIPAL ==================== */
typedef struct {
    SDL_Texture *texture;   /* normal (pas hover)  */
    SDL_Texture *hoverTx;   /* version hover       */
    SDL_Rect     rect;
    int          state;     /* 0=normal, 1=hovered */
    char         label[32]; /* nom du bouton       */
} Button;

/* ==================== MENU PRINCIPAL ==================== */
typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    int           winW, winH;   /* dimensions réelles */

    SDL_Texture *bgFrames[FRAME_COUNT];
    int          currentFrame;
    int          bgDirection;

    Button buttons[BUTTON_COUNT];

    SDL_Texture *logoTexture;  /* logo.png (optionnel) */
    TTF_Font    *font;

    Mix_Chunk   *clickSound;
    Mix_Chunk   *hoverSound;
    Mix_Music   *music;

    bool running;
} Menu;

/* integrated.h est inclus ici pour ScoreMenu, ScoreMenuState, PlayerScore */
#include "integrated.h"

/* ==================== PROTOTYPES ==================== */
bool init       (Menu *menu);
void loadAssets (Menu *menu);
void render     (Menu *menu);
void update     (Menu *menu);
void destroy    (Menu *menu);
void loadButtons(Menu *menu);
void loadBackground(Menu *menu);

/* loadTexture — défini dans score.c/integrated.c */
SDL_Texture *loadTexture(const char *path, SDL_Renderer *renderer);

#endif /* GAME_H */
