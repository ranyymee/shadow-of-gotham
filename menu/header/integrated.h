#ifndef INTEGRATED_H
#define INTEGRATED_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#define MAX_SCORES  10
#define MAX_NAME    100
#define MAX_FRAMES  50

/* ==================== ENIGME (forward) ==================== */
/* enigme.h est inclus dans les .c qui en ont besoin */

/* ==================== ÉTATS DU SCORE MENU ==================== */
typedef enum {
    MENU_INPUT,
    MENU_SCORES_DISPLAY,
    MENU_ENIGME
} ScoreMenuState;

/* ==================== STRUCTURE SCORE JOUEUR ==================== */
typedef struct {
    char name[MAX_NAME];
    int  score;
} PlayerScore;

/* ==================== STRUCTURE SCORE MENU ==================== */
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

    SDL_Texture *overlay;  /* gardé pour compatibilité */
    SDL_Texture *panel;    /* zina.jpg — panel gris arrondi */
    SDL_Texture *label;    /* label.png — barre saisie nom */
} ScoreMenu;

/* ==================== PROTOTYPES SCORE MENU ==================== */
void initScoreMenu   (ScoreMenu *menu, SDL_Renderer *renderer);
void cleanupScoreMenu(ScoreMenu *menu);
void saveScore       (const char *player_name, int score);
void displayScores   (SDL_Renderer *renderer, ScoreMenu *menu);
void handleEvents    (SDL_Event, ScoreMenu*, ScoreMenuState*, int*, int, SDL_Renderer*, int*);
void display         (SDL_Renderer *renderer, ScoreMenu *menu, ScoreMenuState state, int score);
int  scoreMenuLoop   (SDL_Window *window, SDL_Renderer *renderer, int final_score);

/* ==================== UTILITAIRE ==================== */
/* loadTexture est publique, définie dans enigme.c */
SDL_Texture *loadTexture(const char *path, SDL_Renderer *renderer);

#endif /* INTEGRATED_H */
