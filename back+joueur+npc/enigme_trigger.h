/*
 * enigme_trigger.h
 * Système de déclenchement d'énigme par collision avec un objet sur la map.
 *
 * Intégration :
 *   1. Ajouter un EnigmeTrigger dans votre structure de jeu (ou en global).
 *   2. Appeler initEnigmeTrigger() au démarrage.
 *   3. Dans la boucle de jeu, appeler updateEnigmeTrigger() à chaque frame.
 *   4. Dans le rendu, appeler renderEnigmeTrigger() pour dessiner l'objet.
 *   5. Passer le résultat à votre logique (score, vie, etc.).
 */

#ifndef ENIGME_TRIGGER_H
#define ENIGME_TRIGGER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "enigme.h"   /* QuestionData, Enigme, NB_QUESTIONS, etc. */

/* ── Durée du feedback visuel après résolution (millisecondes) ──────── */
#define FEEDBACK_SUCCESS_MS  2000   /* 2 secondes */
#define FEEDBACK_FAIL_MS     2000   /* 2 secondes */

/* ── États du trigger ───────────────────────────────────────────────── */
typedef enum {
    ETRIG_IDLE,        /* L'objet est sur la map, pas encore touché      */
    ETRIG_RUNNING,     /* Énigme en cours (interface affichée)            */
    ETRIG_SUCCESS,     /* Joueur a répondu correctement                   */
    ETRIG_FAIL,        /* Joueur a échoué (mauvaise réponse ou timeout)   */
    ETRIG_DONE         /* Animation de fin terminée, objet consommé       */
} ETrigState;

/* ── Objet déclencheur ──────────────────────────────────────────────── */
typedef struct {
    SDL_Rect     hitbox;        /* Position et taille sur la map          */
    SDL_Texture *sprite;        /* Texture enigme.png (l'icône sur la map)*/
    float        bobPhase;      /* Phase pour l'animation bob             */

    ETrigState   state;
    Enigme       enigme;        /* Instance du quiz embarquée             */

    /* Feedback visuel après résolution */
    Uint32       feedbackTimer;
    int          feedbackDuration; /* ms */

    /* Résultat final : +1 score, -1 vie, etc. */
    int          resultBonus;   /* appliqué à l'appelant quand ETRIG_DONE */
} EnigmeTrigger;

/* ── API publique ───────────────────────────────────────────────────── */

/**
 * Initialise le trigger.
 * @param t         Pointeur vers l'objet trigger.
 * @param renderer  Renderer SDL.
 * @param x, y      Position sur la map (pixels).
 * @param w, h      Taille du hitbox/sprite (ex: 64x64).
 * @param spritePath Chemin vers l'image de l'objet (enigme.png).
 */
void initEnigmeTrigger(EnigmeTrigger *t, SDL_Renderer *renderer,
                       int x, int y, int w, int h,
                       const char *spritePath);

/**
 * À appeler chaque frame depuis votre boucle de jeu.
 *
 * @param t          Le trigger.
 * @param renderer   Renderer SDL.
 * @param font       Police principale (grande).
 * @param fontSmall  Police secondaire.
 * @param fontTiny   Police mini (chrono/badges).
 * @param playerRect SDL_Rect du joueur actif (hitbox).
 * @param event      Événement SDL courant (NULL si aucun).
 * @param dt         Delta-time en secondes.
 */
void updateEnigmeTrigger(EnigmeTrigger *t,
                         SDL_Renderer  *renderer,
                         TTF_Font      *font,
                         TTF_Font      *fontSmall,
                         TTF_Font      *fontTiny,
                         SDL_Rect      *playerRect,
                         SDL_Event     *event,
                         float          dt);

/**
 * Dessine l'objet sur la map (uniquement si état IDLE ou approche).
 */
void renderEnigmeTrigger(EnigmeTrigger *t, SDL_Renderer *renderer, Uint32 ticks);

/**
 * Libère les ressources du trigger.
 */
void freeEnigmeTrigger(EnigmeTrigger *t);

/**
 * Retourne 1 si les deux rectangles se chevauchent.
 */
static inline int rectsOverlap(SDL_Rect *a, SDL_Rect *b) {
    return a->x < b->x + b->w && a->x + a->w > b->x &&
           a->y < b->y + b->h && a->y + a->h > b->y;
}

#endif /* ENIGME_TRIGGER_H */
