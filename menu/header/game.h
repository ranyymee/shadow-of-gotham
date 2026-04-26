#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

/* ==================== CONSTANTES ==================== */
#define FRAME_COUNT   102
#define BUTTON_COUNT  6
#define MAX_SCORES    10
#define MAX_NAME      100
#define MAX_FRAMES    50

/* ==================== GAME STATE (Main.c) ==================== */
/* FIX: This enum was completely missing — Main.c uses all these values */
typedef enum {
    STATE_MENU,
    STATE_SAUVEGARDE,
    STATE_OPTIONS,
    STATE_PLAYER,
    STATE_SCORES,
    STATE_ENIGME,
    STATE_QUIT
} GameState;

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
    int          bgDirection;   /* FIX: was missing from struct */

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

/* ==================== SCORE MENU ==================== */
typedef enum {
    MENU_INPUT,
    MENU_SCORES_DISPLAY,
    MENU_ENIGME
} ScoreMenuState;

typedef struct {
    char name[MAX_NAME];
    int  score;
} PlayerScore;

typedef struct {
    SDL_Texture *background_frames[MAX_FRAMES];
    int          frame_count;
    int          current_frame;
    int          frame_direction;
    Uint32       last_frame_time;

    SDL_Texture *button_validate_texture;
    SDL_Texture *button_return_texture;
    SDL_Texture *button_quit_texture;
    SDL_Texture *button_enigme_texture;

    SDL_Texture *button_validate_hover_texture;
    SDL_Texture *button_return_hover_texture;
    SDL_Texture *button_quit_hover_texture;
    SDL_Texture *button_enigme_hover_texture;

    SDL_Rect pos_button_validate;
    SDL_Rect pos_button_return;
    SDL_Rect pos_button_quit;
    SDL_Rect pos_button_enigme;

    SDL_Rect zone_input;
    char     player_name[MAX_NAME];

    TTF_Font  *font;
    SDL_Color  textColor;

    Mix_Chunk *click_sound;
    Mix_Music *validation_music;

    int hovered_validate;
    int hovered_quit;
    int hovered_return;
    int hovered_enigme;
} ScoreMenu;

/* ==================== PROTOTYPES MENU PRINCIPAL ==================== */
int          init           (Menu *menu);
SDL_Texture *loadTextureMenu(Menu *menu, const char *path);
void         loadAssets     (Menu *menu);
void         events         (Menu *menu);
void         render         (Menu *menu);
void         destroy        (Menu *menu);
void         action         (Menu *menu, Action a);

/* ==================== PROTOTYPES SCORE MENU ==================== */
void initScoreMenu   (ScoreMenu *menu, SDL_Renderer *renderer);
void cleanupScoreMenu(ScoreMenu *menu);
void saveScore       (const char *player_name, int score);
void displayScores   (SDL_Renderer *renderer, ScoreMenu *menu);
void handleEvents    (SDL_Event, ScoreMenu*, ScoreMenuState*, int*, int, SDL_Renderer*, int*);
void display         (SDL_Renderer *renderer, ScoreMenu *menu, ScoreMenuState state, int score);
int  scoreMenuLoop   (SDL_Window *window, SDL_Renderer *renderer, int final_score);

/* ==================== PROTOTYPE loadTexture (enigme.c) ==================== */
SDL_Texture *loadTexture(const char *path, SDL_Renderer *renderer);

#endif
