#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "player.h"

#define CHAR_COUNT 2

#define CHAR_BATMAN   0
#define CHAR_CATWOMAN 1

#define MODE_SOLO  0
#define MODE_MULTI 1

typedef struct {
    const char *name;
    const char *sheetRight[COSTUME_COUNT];
    const char *sheetLeft [COSTUME_COUNT];
    const char *costumeNames[COSTUME_COUNT];
    const char *costumeAvatars[COSTUME_COUNT];
} CharacterDef;

typedef struct {
    int         mode;

    int         charP1;
    int         costumeP1;
    InputConfig inputP1;

    int         charP2;
    int         costumeP2;
    InputConfig inputP2;
} MenuResult;

int runMenu(SDL_Renderer *renderer, TTF_Font *font, MenuResult *result);

#endif
