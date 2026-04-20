#ifndef GAME_H
#define GAME_H

typedef enum {
    MENU,
    PLAY_SOLO,
    PLAY_MULTI,
    QUIT
} GameState;

typedef struct {
    GameState state;
} Jeu;

#endif
