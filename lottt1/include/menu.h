#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "player.h"

#define CHAR_COUNT 2

typedef enum {
    CHAR_BATMAN   = 0,
    CHAR_CATWOMAN = 1
} CharacterID;

typedef enum {
    MODE_SOLO  = 0,
    MODE_MULTI = 1
} GameMode;

typedef struct {
    const char *name;
    const char *sheetRight[COSTUME_COUNT];
    const char *sheetLeft [COSTUME_COUNT];
    const char *costumeNames[COSTUME_COUNT];
    /* Image avatar pour chaque costume (affichee dans le menu) */
    const char *costumeAvatars[COSTUME_COUNT];
} CharacterDef;

extern const CharacterDef CHARACTERS[CHAR_COUNT];

typedef struct {
    GameMode    mode;

    CharacterID charP1;
    int         costumeP1;
    InputConfig inputP1;

    CharacterID charP2;
    int         costumeP2;
    InputConfig inputP2;
} MenuResult;

/*
 * Lance le menu.
 * Retourne 1 = jouer, 0 = quitter.
 */
int runMenu(SDL_Renderer *renderer, TTF_Font *font, MenuResult *result);

#endif
