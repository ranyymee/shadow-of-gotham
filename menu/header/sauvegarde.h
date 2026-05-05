#ifndef ENIGME_H
#define ENIGME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* WIN_W / WIN_H sont déjà définis dans header.h (1100x620 pour le menu puzzle).
   On ne les redéfinit PAS ici pour éviter le conflit.
   enigme.c utilise scrW/scrH passés en paramètre. */
#ifndef WIN_W
#define WIN_W                1920
#endif
#ifndef WIN_H
#define WIN_H                1080
#endif
#define SCREEN_W             1280
#define SCREEN_H              720
#define MAX_QUESTION_LEN      300
#define MAX_PROP_LEN          150
#define NB_PROPS                3
#define NB_QUESTIONS           10
#define MAX_QUESTIONS          50
#define RESULT_DELAY         2000
#define SUSPENSE_DELAY       4000
#define CHRONO_SECS            20
#define QUESTIONS_PAR_NIVEAU    5
#define SND_NONE     0
#define SND_BAT      1
#define SND_SUSPENSE 2
#define SND_CORRECT  3
#define SND_WRONG    4

// Structure pour une question
typedef struct {
    char question[MAX_QUESTION_LEN];
    char prop[NB_PROPS][MAX_PROP_LEN];
    int bonneReponse;
} QuestionData;

// Etat interne de l'enigme
typedef enum {
    ENIGME_STATE_CHOOSE,
    ENIGME_STATE_QUIZ,
    ENIGME_STATE_DONE
} EnigmeState;

// Structure principale pour l'énigme
typedef struct {
    QuestionData questions[MAX_QUESTIONS];
    int          nbQuestionsPool;
    int          ordreJeu[NB_QUESTIONS];
    int          questionIndex;
    int          score;
    int          niveau;
    int          selected;
    int          hovered;
    int          showResult;
    int          correct;
    Uint32       resultTime;
    int          waitingSuspense;
    Uint32       suspenseTime;
    int          chronoSecondes;
    Uint32       chronoLastTick;
    int          chronoExpire;
    float        chronoRatio;
    int          currentSound;
    SDL_Texture *bgTexture;
    SDL_Texture *cardTexture;
    SDL_Texture *propTexture[NB_PROPS];
    Mix_Music   *soundBat;
    Mix_Music   *soundSuspense;
    Mix_Music   *soundCorrect;
    Mix_Music   *soundWrong;
    SDL_Texture *batLogoTex;
    SDL_Texture *chronoDesignTex;
    SDL_Rect     cardRect;
    SDL_Rect     propRect[NB_PROPS];
    SDL_Rect     chronoRect;
    SDL_Rect     chronoDesignRect;
    SDL_Rect     batLogoRect;
    int          hudH;
    double       cardAngle;
    double       propAngle[NB_PROPS];
    int          scrW;
    int          scrH;
} Enigme;

typedef struct {
    SDL_Texture *bgTexture;
    SDL_Texture *quizTex;
    SDL_Texture *quizHoverTex;
    SDL_Texture *puzzleTex;
    SDL_Texture *puzzleHoverTex;
    SDL_Rect     quizRect;
    SDL_Rect     puzzleRect;
    int          hoverQuiz;
    int          hoverPuzzle;
} MenuEnigme;

typedef struct {
    SDL_Texture *bgTexture;
    SDL_Texture *quizTex;
    SDL_Texture *quizHoverTex;
    SDL_Texture *puzzleTex;
    SDL_Texture *puzzleHoverTex;
    SDL_Rect     quizRect;
    SDL_Rect     puzzleRect;
    int          hoverQuiz;
    int          hoverPuzzle;
} MenuPrincipal;

void initEnigme       (Enigme *e, SDL_Renderer *renderer, int scrW, int scrH);
void updateEnigme     (Enigme *e);
void renderEnigme     (Enigme *e, SDL_Renderer *renderer,
                       TTF_Font *font, TTF_Font *fontSmall, TTF_Font *fontTiny);
void handleEnigmeEvent(Enigme *e, SDL_Event *event);
void freeEnigme       (Enigme *e);
int  runGameMenu      (SDL_Window *window, SDL_Renderer *renderer);

#endif  // ENIGME_H
