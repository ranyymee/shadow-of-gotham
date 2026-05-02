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

<<<<<<< HEAD
=======
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

>>>>>>> 75f7f12 (add bck folder)
/* ==================== MENU PRINCIPAL ==================== */
typedef enum {
    PLAY,
    OPTIONS,
    PLAYERS,
    SCORES,
    HISTORY,
    QUITTER
} Action;

typedef struct {
    SDL_Texture *normal;
    SDL_Texture *hover;
    SDL_Rect     rect;
    int          state;
} Button;

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;

    SDL_Texture *frames[FRAME_COUNT];
    int          currentFrame;
<<<<<<< HEAD
=======
    int          bgDirection;
>>>>>>> 75f7f12 (add bck folder)

    Button buttons[BUTTON_COUNT];

    SDL_Texture *logo;
    SDL_Rect     logoRect;

    SDL_Texture *textShadow;
    SDL_Rect     textShadowRect;

    SDL_Texture *textOfGotham;
    SDL_Rect     textOfGothamRect;

    TTF_Font  *font;
    Mix_Chunk *clickSound;
    Mix_Music *music;

    bool running;
} Menu;

/* ScoreMenuState / PlayerScore / ScoreMenu sont définis dans integrated.h.
   On l'inclut ici pour les rendre disponibles à quiconque inclut game.h. */
#include "integrated.h"

/* ==================== PROTOTYPES MENU PRINCIPAL ==================== */
int  init        (Menu *menu);
void loadAssets  (Menu *menu);
void events      (Menu *menu);
void render      (Menu *menu);
void update      (Menu *menu);
void destroy     (Menu *menu);
void action      (Menu *menu, Action a);
void loadButtons (Menu *menu);

/* ==================== PROTOTYPE loadTexture (enigme.c) ==================== */
SDL_Texture *loadTexture(const char *path, SDL_Renderer *renderer);

#endif /* GAME_H */
