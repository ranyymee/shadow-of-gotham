#ifndef SAUVEGARDE_H
#define SAUVEGARDE_H

#include <SDL2/SDL.h>
#include "header.h" // This gives you access to GameState and STATE_PLAYER

void initialiser_sauvegarde(SDL_Renderer *renderer);
void gerer_evenement_sauvegarde(SDL_Event event, GameState *state);
void afficher_sous_menu_sauvegarde(SDL_Renderer *renderer);
void nettoyer_sauvegarde();

#endif
