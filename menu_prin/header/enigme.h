#ifndef ENIGME_H
#define ENIGME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

// Dimensions de l'écran
#define SCREEN_W 1280
#define SCREEN_H 720

// Limites pour les questions et propositions
#define MAX_QUESTION_LEN 256
#define MAX_PROP_LEN 128
#define NB_PROPS 3
#define NB_QUESTIONS 3

// Structure pour une question
typedef struct {
    char question[MAX_QUESTION_LEN];
    char prop[NB_PROPS][MAX_PROP_LEN];
    int bonneReponse;  // index de la bonne réponse (0, 1, ou 2)
} QuestionData;

// Etat interne de l'enigme
typedef enum {
    ENIGME_STATE_CHOOSE,
    ENIGME_STATE_QUIZ,
    ENIGME_STATE_DONE
} EnigmeState;

// Structure principale pour l'énigme
typedef struct {
    QuestionData questions[NB_QUESTIONS];
    int questionIndex;
    int score;
    int selected;
    int hovered;
    int showResult;
    int correct;
    Uint32 resultTime;

    SDL_Texture *cardTexture;
    SDL_Texture *propTexture[NB_PROPS];
    SDL_Texture *bgTexture;

    Mix_Music *soundCorrect;
    Mix_Music *soundWrong;
    Mix_Music *soundSuspense;
    int waitingSuspense;
    Uint32 suspenseTime;

    SDL_Rect cardRect;
    SDL_Rect propRect[NB_PROPS];

    // Page choix Quiz / Puzzle
    EnigmeState  state;
    SDL_Texture *quizTex;
    SDL_Texture *quizHoverTex;
    SDL_Texture *puzzleTex;
    SDL_Texture *puzzleHoverTex;
    SDL_Rect     quizRect;
    SDL_Rect     puzzleRect;
    int          hoverQuiz;
    int          hoverPuzzle;
} Enigme;

// Fonctions principales
void initEnigme(Enigme *e, SDL_Renderer *renderer);
void updateEnigme(Enigme *e);
void renderEnigme(Enigme *e, SDL_Renderer *renderer, TTF_Font *font, TTF_Font *fontSmall);
void handleEnigmeEvent(Enigme *e, SDL_Event *event);
void freeEnigme(Enigme *e);

#endif  // fin de ENIGME_H
