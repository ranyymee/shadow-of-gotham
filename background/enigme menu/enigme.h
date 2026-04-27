#ifndef ENIGME_H
#define ENIGME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

/* These are set at runtime from the actual window size */
extern int SCR_W;
extern int SCR_H;

#define MAX_QUESTION_LEN     300
#define MAX_PROP_LEN         150
#define NB_PROPS             3
#define NB_QUESTIONS         10
#define MAX_QUESTIONS        50
#define RESULT_DELAY         2000
#define SUSPENSE_DELAY       4000
#define CHRONO_SECS          20
#define QUESTIONS_PAR_NIVEAU 5

typedef enum {
    SND_NONE = 0,
    SND_BAT,
    SND_SUSPENSE,
    SND_CORRECT,
    SND_WRONG
} SoundID;

typedef struct {
    char question[MAX_QUESTION_LEN];
    char prop[NB_PROPS][MAX_PROP_LEN];
    int  bonneReponse;
    int  deja_vu;
} QuestionData;

typedef struct {
    QuestionData questions[MAX_QUESTIONS];
    int          nbQuestionsPool;
    int          ordreJeu[NB_QUESTIONS];

    int    questionIndex;
    int    score;
    int    niveau;
    int    selected;
    int    hovered;
    int    showResult;
    int    correct;
    Uint32 resultTime;

    int    waitingSuspense;
    Uint32 suspenseTime;

    int    chronoSecondes;
    Uint32 chronoLastTick;
    int    chronoExpire;
    float  chronoRatio;     /* 1.0 = full time, 0.0 = expired */

    char      msgTexte[128];
    Uint32    msgTime;
    int       msgVisible;
    SDL_Color msgColor;

    SDL_Texture *bgTexture;
    SDL_Texture *cardTexture;
    SDL_Texture *propTexture[NB_PROPS];

    Mix_Music *soundBat;
    Mix_Music *soundSuspense;
    Mix_Music *soundCorrect;
    Mix_Music *soundWrong;

    SoundID currentSound;

    SDL_Texture *batLogoTex;
    SDL_Texture *chronoDesignTex;

    /* layout rects — recomputed from actual window size */
    SDL_Rect cardRect;
    SDL_Rect propRect[NB_PROPS];
    SDL_Rect chronoRect;
    SDL_Rect chronoDesignRect;
    SDL_Rect batLogoRect;
    int      hudH;          /* HUD band height in pixels */

    double cardAngle;
    double propAngle[NB_PROPS];

    /* font sizes (pt) stored so we know what was loaded */
    int fontSizeBig;
    int fontSizeSmall;
    int fontSizeTiny;

} Enigme;

void initEnigme       (Enigme *e, SDL_Renderer *renderer);
void updateEnigme     (Enigme *e);
void renderEnigme     (Enigme *e, SDL_Renderer *renderer,
                       TTF_Font *font, TTF_Font *fontSmall, TTF_Font *fontTiny);
void handleEnigmeEvent(Enigme *e, SDL_Event *event);
void freeEnigme       (Enigme *e);

#endif
