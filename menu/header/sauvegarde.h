#ifndef SAUVEGARDE_H
#define SAUVEGARDE_H

#include <SDL2/SDL.h>
#include "game.h"   /* GameState, STATE_* sont définis ici */

void initialiser_sauvegarde(SDL_Renderer *renderer);
void gerer_evenement_sauvegarde(SDL_Event event, GameState *state);
void afficher_sous_menu_sauvegarde(SDL_Renderer *renderer);
void nettoyer_sauvegarde();

#endif
