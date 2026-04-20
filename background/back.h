#ifndef BACK_H
#define BACK_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

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
} Platform;

typedef struct {
    SDL_Texture *image;
    SDL_Rect     position;
    int          w;
    int          h;
} GuideButton;

typedef struct {
    SDL_Texture *image;
    int          w;
    int          h;
} SDL_TextureWithRect;

typedef struct {
    SDL_Texture        *img[1];
    SDL_Rect            posimg;
    SDL_Rect            camera_pos;
    int                 direction;
    GuideButton         guide;
    SDL_TextureWithRect commentJouer;
    int                 afficherCommentJouer;
} Background;

void initBackgroundAndPlatforms(SDL_Renderer *renderer, Background *bg, Platform platforms[], int *taille, int level, int screenW, int screenH);
void afficherPlatforms(SDL_Renderer *renderer, Platform platforms[], int taille, int bgX, int bgY);
void updatePlatforms(Platform platforms[], int taille);
void gererScrollingDeuxJoueurs(SDL_Event event, Background *bg1, Background *bg2, int scrollSpeed, int level);
void gererTemps(int *timeLeft, Uint32 *lastTime);
void afficherTemps(SDL_Renderer *renderer, TTF_Font *font, int timeLeft, int screenW);
void gererGuideEtClic(SDL_Event event, GuideButton *guide, SDL_TextureWithRect *commentJouer, int *afficherCommentJouer);
void afficherBackgroundEtElements(SDL_Renderer *renderer, Background *bg, Platform platforms[], int taille, TTF_Font *font, SDL_Color textColor, int timeLeft, int lives, int bgX, int bgY, int screenW, int screenH, int mode);
void saisirNomEtAfficherScore(SDL_Renderer *renderer, TTF_Font *font, int score, int screenW, int screenH);

#endif
