#ifndef BACK_H
#define BACK_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

<<<<<<< HEAD
#define MAX_FRAMES 3

typedef struct {
    SDL_Texture *image;
    SDL_Texture *frames[4];
    SDL_Rect position;
    int vitesse;
    int isAnimated;
    int currentFrame;
    int frameWidth;
    int frameHeight;
    Uint32 lastFrameTime;
=======
#define MAX_FRAMES    3
#define MAX_PLATFORMS 30

#define PLATFORM_FIXE         0
#define PLATFORM_MOBILE       1
#define PLATFORM_DESTRUCTIBLE 2

#define MODE_MONO  0
#define MODE_MULTI 1

typedef struct {
    SDL_Texture *image;
    SDL_Texture *frames[4];
    SDL_Rect     position;
    int          vitesse;
    int          isAnimated;
    int          currentFrame;
    int          frameWidth;
    int          frameHeight;
    Uint32       lastFrameTime;
    int          type;
    int          destroyed;
    int          moveDir;
    int          moveMin;
    int          moveMax;
    int          moveAxis;
    int          hp;
>>>>>>> f854e34 (first commit)
} Platform;

typedef struct {
    SDL_Texture *image;
<<<<<<< HEAD
    SDL_Rect position;
    int w, h;
=======
    SDL_Rect     position;
    int          w;
    int          h;
>>>>>>> f854e34 (first commit)
} GuideButton;

typedef struct {
    SDL_Texture *image;
<<<<<<< HEAD
    int w, h;
=======
    int          w;
    int          h;
>>>>>>> f854e34 (first commit)
} SDL_TextureWithRect;

typedef struct {
    SDL_Texture *img[1];
    SDL_Rect posimg;
    GuideButton guide;
    SDL_TextureWithRect commentJouer;
    int afficherCommentJouer;
} Background;

<<<<<<< HEAD
void initBackgroundAndPlatforms(SDL_Renderer *renderer, Background *bg, Platform platforms[], int *taille);
void afficherPlatforms(SDL_Renderer *renderer, Platform platforms[], int taille, int bgX, int bgY);
void gererScrollingDeuxJoueurs(SDL_Event event, int *bgX1, int *bgY1, int *bgX2, int *bgY2, int scrollSpeed);
void gererTemps(int *timeLeft, Uint32 *lastTime);
void gererGuideEtClic(SDL_Event event, GuideButton *guide, SDL_TextureWithRect *commentJouer, int *afficherCommentJouer);
void afficherBackgroundEtElements(SDL_Renderer *renderer, Background *bg, Platform platforms[], int taille, TTF_Font *font, SDL_Color textColor, int timeLeft, int lives, int bgX, int bgY, int screenW, int screenH);
=======
void initBackgroundAndPlatforms(SDL_Renderer *renderer, Background *bg, Platform platforms[], int *taille, int level);
void afficherPlatforms(SDL_Renderer *renderer, Platform platforms[], int taille, int bgX, int bgY);
void updatePlatforms(Platform platforms[], int taille);
void gererScrollingDeuxJoueurs(SDL_Event event, int *bgX1, int *bgY1, int *bgX2, int *bgY2, int scrollSpeed);
void gererTemps(int *timeLeft, Uint32 *lastTime);
void afficherTemps(SDL_Renderer *renderer, TTF_Font *font, int timeLeft, int screenW);
void gererGuideEtClic(SDL_Event event, GuideButton *guide, SDL_TextureWithRect *commentJouer, int *afficherCommentJouer);
void afficherBackgroundEtElements(SDL_Renderer *renderer, Background *bg, Platform platforms[], int taille, TTF_Font *font, SDL_Color textColor, int timeLeft, int lives, int bgX, int bgY, int screenW, int screenH, int mode);
>>>>>>> f854e34 (first commit)
void saisirNomEtAfficherScore(SDL_Renderer *renderer, TTF_Font *font, int score, int screenW, int screenH);

#endif
