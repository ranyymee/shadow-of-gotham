#ifndef OPTIONS_H
#define OPTIONS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>

/* ── Constants ── */
#define HOVER_SCALE 6
#define BACK_X  16
#define BACK_Y  524
#define BACK_W  165
#define BACK_H  62

/* ── Contrast modes ── */
typedef enum {
    CONTRAST_LOW  = 0,
    CONTRAST_MID  = 1,
    CONTRAST_HIGH = 2,
    CONTRAST_COUNT = 3
} ContrastMode;

/* ── Menu states ── */
typedef enum {
    MENU_MAIN     = 0,
    MENU_GRAPHICS = 1,
    MENU_SOUND    = 2
} MenuState;

/* ── Texture loader macro ── */
#define LOAD_TEXTURE(path, dst) \
    do { \
        SDL_Surface *_s = IMG_Load(path); \
        if (!_s) { \
            fprintf(stderr, "[WARN] Could not load %s: %s\n", path, IMG_GetError()); \
            (dst) = NULL; \
        } else { \
            (dst) = SDL_CreateTextureFromSurface(renderer, _s); \
            SDL_FreeSurface(_s); \
            if (!(dst)) fprintf(stderr, "[WARN] Texture from %s: %s\n", path, SDL_GetError()); \
        } \
    } while(0)

/* ── Options struct ── */
typedef struct {
    /* State */
    MenuState   state;
    MenuState   previousState;
    MenuState   nextState;

    /* Fade */
    int   isFading;
    int   fadeAlpha;

    /* Audio values */
    int volumeMusic;
    int volumeSFX;

    /* Contrast */
    ContrastMode contrast;

    /* Pulse animation */
    float pulseT;

    /* Font */
    TTF_Font *font;

    /* Textures — background & banner */
    SDL_Texture *background;
    SDL_Texture *topBanner;

    /* Main menu buttons */
    SDL_Texture *btnGraphics;
    SDL_Texture *btnGraphicsHover;
    SDL_Texture *btnSound;
    SDL_Texture *btnSoundHover;

    /* Back buttons — normal */
    SDL_Texture *btnBackMain;
    SDL_Texture *btnBackGraphics;
    SDL_Texture *btnBackSound;

    /* Back buttons — hover */
    SDL_Texture *btnBackMainHover;
    SDL_Texture *btnBackGraphicsHover;
    SDL_Texture *btnBackSoundHover;

    /* Graphics screen */
    SDL_Texture *btnWindow;
    SDL_Texture *btnWide;
    SDL_Texture *contrastImg[CONTRAST_COUNT];

    /* Sound screen */
    SDL_Texture *btnPlus;
    SDL_Texture *btnMinus;
    SDL_Texture *btnPlusSFX;
    SDL_Texture *btnMinusSFX;

    /* Audio */
    Mix_Music *music;
    Mix_Chunk *clickSound;

    /* Rects — background & banner */
    SDL_Rect bgPos;
    SDL_Rect topPos;

    /* Rects — main menu */
    SDL_Rect posGraphics;
    SDL_Rect posSound;
    SDL_Rect posBackMain;
    SDL_Rect posBackMainHover;

    /* Rects — graphics */
    SDL_Rect posWindow;
    SDL_Rect posWide;
    SDL_Rect posContrastLabel;
    SDL_Rect posContrastBtn[CONTRAST_COUNT];
    SDL_Rect posBackGraphics;

    /* Rects — sound */
    SDL_Rect posLabelMusic;
    SDL_Rect posMinus;
    SDL_Rect posPlus;
    SDL_Rect posLabelSFX;
    SDL_Rect posMinusSFX;
    SDL_Rect posPlusSFX;
    SDL_Rect posBackSound;
} Options;

/* ── Function prototypes ── */
int  initOptions   (Options *opt, SDL_Renderer *renderer);
void inputOptions  (Options *opt, SDL_Event event, SDL_Window *window, int *running);
void renderOptions (Options *opt, SDL_Renderer *renderer);
void freeOptions   (Options *opt);
void startFade     (Options *opt, MenuState target);

#endif /* OPTIONS_H */
