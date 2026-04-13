#ifndef FONCTION_H
#define FONCTION_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

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
} Platform;

typedef struct {
    SDL_Texture *image;
    SDL_Rect position;
    int w, h;
} GuideButton;

typedef struct {
    SDL_Texture *image;
    int w, h;
} SDL_TextureWithRect;

typedef struct {
    SDL_Texture *img[1];
    SDL_Rect posimg;
    GuideButton guide;
    SDL_TextureWithRect commentJouer;
    int afficherCommentJouer;
} Background;

void initBackgroundAndPlatforms(SDL_Renderer *renderer, Background *bg, Platform platforms[], int *taille);
void afficherPlatforms(SDL_Renderer *renderer, Platform platforms[], int taille, int bgX, int bgY);
void gererScrollingDeuxJoueurs(SDL_Event event, int *bgX1, int *bgY1, int *bgX2, int *bgY2, int scrollSpeed);
void gererTemps(int *timeLeft, Uint32 *lastTime);
void gererGuideEtClic(SDL_Event event, GuideButton *guide, SDL_TextureWithRect *commentJouer, int *afficherCommentJouer);
void afficherBackgroundEtElements(SDL_Renderer *renderer, Background *bg, Platform platforms[], int taille, TTF_Font *font, SDL_Color textColor, int timeLeft, int lives, int bgX, int bgY, int screenW, int screenH);
void saisirNomEtAfficherScore(SDL_Renderer *renderer, TTF_Font *font, int score, int screenW, int screenH);

#endif
