/*
 * main_jeu.c  —  Boucle de jeu principale
 *
 * Ce fichier gère :
 *   - L'affichage du background de jeu
 *   - Le déplacement de Batman / Catwoman (via player.c)
 *   - La détection de collision avec l'objet EnigmeTrigger (enigme.png)
 *   - Le lancement et le rendu de l'énigme QCM (enigme.c)
 *   - L'écran de succès / échec final
 *
 * Pour compiler (exemple) :
 *   gcc main_jeu.c enigme_trigger.c enigme.c player.c \
 *       -o jeu $(sdl2-config --cflags --libs) \
 *       -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm
 *
 * Adaptez les chemins d'assets selon votre projet.
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "player.h"
#include "enigme.h"
#include "enigme_trigger.h"

/* ── Fenêtre ─────────────────────────────────────────────────────────── */
#define WIN_W  SCREEN_W
#define WIN_H  SCREEN_H

/* ── Nombre d'objets énigme sur la map ───────────────────────────────── */
#define NB_TRIGGERS 3

/* ── Vitesse de déplacement joueur (pixels/s) ────────────────────────── */
#define PLAYER_SPEED 250.0f

/* ─────────────────────────────────────────────────────────────────────
   États du jeu
   ───────────────────────────────────────────────────────────────────── */
typedef enum {
    GS_PLAY,       /* Jeu normal (déplacement + carte)     */
    GS_ENIGME,     /* Énigme en cours (géré par trigger)   */
    GS_WIN,        /* Tous les triggers résolus avec succès */
    GS_LOSE        /* Le joueur a perdu toutes ses vies     */
} GameState;

/* ─────────────────────────────────────────────────────────────────────
   Helpers de chargement
   ───────────────────────────────────────────────────────────────────── */
static SDL_Texture *loadTexture(SDL_Renderer *r, const char *path)
{
    SDL_Texture *t = IMG_LoadTexture(r, path);
    if (!t) fprintf(stderr, "[WARN] Cannot load '%s': %s\n", path, IMG_GetError());
    return t;
}

static SDL_Texture *makePlaceholder(SDL_Renderer *r, int w, int h,
                                    Uint8 R, Uint8 G, Uint8 B)
{
    SDL_Texture *t = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888,
                                       SDL_TEXTUREACCESS_TARGET, w, h);
    if (!t) return NULL;
    SDL_SetRenderTarget(r, t);
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    SDL_RenderClear(r);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 60);
    for (int x = 0; x < w; x += 32) SDL_RenderDrawLine(r, x, 0, x, h);
    for (int y = 0; y < h; y += 32) SDL_RenderDrawLine(r, 0, y, w, y);
    SDL_SetRenderTarget(r, NULL);
    return t;
}

/* ─────────────────────────────────────────────────────────────────────
   Rendu plein écran coloré semi-transparent + texte centré
   ───────────────────────────────────────────────────────────────────── */
static void overlayText(SDL_Renderer *r, TTF_Font *font,
                        const char *line1, const char *line2,
                        Uint8 bgR, Uint8 bgG, Uint8 bgB,
                        Uint8 fgR, Uint8 fgG, Uint8 fgB)
{
    /* Fond */
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, bgR, bgG, bgB, 200);
    SDL_RenderFillRect(r, NULL);

    if (!font) return;
    SDL_Color col = { fgR, fgG, fgB, 255 };
    int sw, sh;
    SDL_GetRendererOutputSize(r, &sw, &sh);

    /* Ligne 1 */
    SDL_Surface *s1 = TTF_RenderUTF8_Blended(font, line1, col);
    if (s1) {
        SDL_Texture *t1 = SDL_CreateTextureFromSurface(r, s1);
        SDL_Rect d1 = { sw/2 - s1->w/2, sh/2 - s1->h - 10, s1->w, s1->h };
        SDL_RenderCopy(r, t1, NULL, &d1);
        SDL_FreeSurface(s1); SDL_DestroyTexture(t1);
    }

    /* Ligne 2 */
    SDL_Color col2 = { 255, 255, 255, 200 };
    SDL_Surface *s2 = TTF_RenderUTF8_Blended(font, line2, col2);
    if (s2) {
        SDL_Texture *t2 = SDL_CreateTextureFromSurface(r, s2);
        SDL_Rect d2 = { sw/2 - s2->w/2, sh/2 + 10, s2->w, s2->h };
        SDL_RenderCopy(r, t2, NULL, &d2);
        SDL_FreeSurface(s2); SDL_DestroyTexture(t2);
    }
}

/* ─────────────────────────────────────────────────────────────────────
   HUD simplifié (vies + score + énigmes résolues)
   ───────────────────────────────────────────────────────────────────── */
static void renderHUD(SDL_Renderer *r, TTF_Font *font,
                      Player *p, int solved, int total)
{
    if (!font) return;
    char buf[128];
    snprintf(buf, sizeof(buf),
             "Vies: %d   Score: %d   Enigmes: %d/%d",
             p->vies, p->score, solved, total);
    SDL_Color c = { 255, 220, 0, 255 };
    SDL_Surface *s = TTF_RenderUTF8_Blended(font, buf, c);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_Rect d = { 10, 8, s->w, s->h };
    SDL_RenderCopy(r, t, NULL, &d);
    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}

/* ─────────────────────────────────────────────────────────────────────
   runJeu  —  boucle de jeu principale
   Retourne : 0 = normal, -1 = quit
   ───────────────────────────────────────────────────────────────────── */
int runJeu(SDL_Renderer *renderer,
           TTF_Font     *font,
           TTF_Font     *fontSmall,
           TTF_Font     *fontTiny,
           int           isCat)      /* 0=Batman, 1=Catwoman */
{
    int sw, sh;
    SDL_GetRendererOutputSize(renderer, &sw, &sh);

    /* ── Background ── */
    SDL_Texture *bg = loadTexture(renderer, "assets/back.png");
    if (!bg) bg = makePlaceholder(renderer, sw, sh, 20, 10, 40);

    /* ── Joueur ── */
    Player player;
    initPlayer(&player,
               (float)(sw / 2 - 50),
               (float)(GROUND_Y),
               isCat);

    /* Charger les sprites du joueur */
    {
        const char *rPath = isCat ? "assets/catwoman_right_1.png"
                                  : "assets/batman_right_1.png";
        const char *lPath = isCat ? "assets/catwoman_left_1.png"
                                  : "assets/batman_left_1.png";
        SDL_Texture *rTex = loadTexture(renderer, rPath);
        SDL_Texture *lTex = loadTexture(renderer, lPath);
        int fw = isCat ? CAT_FRAME_W : BAT_FRAME_W;
        int fh = isCat ? CAT_FRAME_H : BAT_FRAME_H;
        initSpriteData(&player.sprite, rTex, lTex, fw, fh, isCat);
    }

    /* ── Triggers énigme sur la map ── */
    /*
     * Positions des objets énigme : adaptez selon votre niveau.
     * enigme.png  = icône visible sur la map (ex: une bougie, un point
     *               d'interrogation, un parchemin …).
     * Remplacez "assets/enigme.png" par votre image réelle.
     */
    EnigmeTrigger triggers[NB_TRIGGERS];
    int triggerPositions[NB_TRIGGERS][4] = {
        /* { x,   y,   w,  h  } */
        {  200, 140,  64, 64 },
        {  500, 140,  64, 64 },
        {  750, 140,  64, 64 },
    };
    for (int i = 0; i < NB_TRIGGERS; i++) {
        initEnigmeTrigger(&triggers[i], renderer,
                          triggerPositions[i][0],
                          triggerPositions[i][1],
                          triggerPositions[i][2],
                          triggerPositions[i][3],
                          "assets/enigme.png");
    }

    /* ── État ── */
    GameState  gs       = GS_PLAY;
    int        activeTrigger = -1;   /* index du trigger en cours   */
    int        solved   = 0;         /* nombre d'énigmes résolues   */
    int        running  = 1;
    Uint32     lastTime = SDL_GetTicks();
    SDL_Event  ev;

    while (running) {
        Uint32 now = SDL_GetTicks();
        float  dt  = (float)(now - lastTime) / 1000.0f;
        if (dt > 0.05f) dt = 0.05f;
        lastTime = now;

        /* ── EVENTS ─────────────────────────────────────────────────── */
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { running = 0; break; }
            if (ev.type == SDL_KEYDOWN &&
                ev.key.keysym.sym == SDLK_ESCAPE) { running = 0; break; }

            /* Entrée clavier pour le déplacement (uniquement en jeu) */
            if (gs == GS_PLAY) {
                const Uint8 *keys = SDL_GetKeyboardState(NULL);
                (void)keys; /* géré dans update ci-dessous */
            }

            /* Passer l'événement au trigger actif */
            if (gs == GS_ENIGME && activeTrigger >= 0) {
                handleEnigmeEvent(&triggers[activeTrigger].enigme, &ev);
            }

            /* Écrans WIN / LOSE : ENTER pour rejouer */
            if ((gs == GS_WIN || gs == GS_LOSE) &&
                ev.type == SDL_KEYDOWN &&
                ev.key.keysym.sym == SDLK_RETURN) {
                running = 0;   /* retour au menu appelant */
            }
        }

        /* ── UPDATE ─────────────────────────────────────────────────── */
        if (gs == GS_PLAY) {
            const Uint8 *keys = SDL_GetKeyboardState(NULL);

            /* Déplacement horizontal */
            player.vitesse = 0.0;
            if (keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_A]) {
                player.vitesse  = -PLAYER_SPEED;
                player.direction = 0;
            }
            if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
                player.vitesse  = +PLAYER_SPEED;
                player.direction = 1;
            }

            /* Saut */
            if ((keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_UP] ||
                 keys[SDL_SCANCODE_W]) && player.onGround) {
                player.vy       = JUMP_VY;
                player.onGround = 0;
                player.jumping  = 1;
            }

            /* Courir */
            player.isRunning = keys[SDL_SCANCODE_LSHIFT] ? 1 : 0;
            if (player.isRunning && player.vitesse != 0.0)
                player.vitesse *= (SPEED_RUN / SPEED);

            updatePlayer(&player, dt);

            /* ── Collision joueur ↔ triggers ── */
            SDL_Rect pRect = player.posScreen;
            for (int i = 0; i < NB_TRIGGERS; i++) {
                if (triggers[i].state == ETRIG_IDLE &&
                    rectsOverlap(&pRect, &triggers[i].hitbox)) {
                    gs            = GS_ENIGME;
                    activeTrigger = i;
                    /* Réinitialiser l'énigme pour cette session */
                    freeEnigme(&triggers[i].enigme);
                    initEnigme(&triggers[i].enigme, renderer);
                    fprintf(stdout,
                            "[JEU] Collision avec trigger %d → énigme!\n", i);
                    break;
                }
                /* Vérifier si un trigger vient de se terminer */
                if (triggers[i].state == ETRIG_DONE) {
                    if (triggers[i].resultBonus > 0) {
                        player.score += 10;
                        player.vies++;
                        if (player.vies > 9) player.vies = 9;
                    } else {
                        player.vies--;
                        if (player.score > 0) player.score -= 5;
                    }
                    triggers[i].state = ETRIG_IDLE + 100; /* désactivé */
                    solved++;
                    if (player.vies <= 0)   gs = GS_LOSE;
                    if (solved >= NB_TRIGGERS) gs = GS_WIN;
                }
            }
        }
        else if (gs == GS_ENIGME && activeTrigger >= 0) {
            EnigmeTrigger *at = &triggers[activeTrigger];

            /* Mise à jour de l'énigme */
            updateEnigme(&at->enigme);

            /* Vérifier fin de l'énigme */
            if (at->enigme.questionIndex >= NB_QUESTIONS) {
                if (at->enigme.score >= NB_QUESTIONS / 2) {
                    at->state       = ETRIG_SUCCESS;
                    at->resultBonus = +1;
                    fprintf(stdout, "[JEU] Énigme %d : SUCCÈS\n", activeTrigger);
                } else {
                    at->state       = ETRIG_FAIL;
                    at->resultBonus = -1;
                    fprintf(stdout, "[JEU] Énigme %d : ÉCHEC\n", activeTrigger);
                }
                at->feedbackTimer   = 0;
                at->feedbackDuration = FEEDBACK_SUCCESS_MS;
            }

            /* Feedback affiché, attendre la fin du timer */
            if (at->state == ETRIG_SUCCESS || at->state == ETRIG_FAIL) {
                at->feedbackTimer += (Uint32)(dt * 1000.0f);
                if (at->feedbackTimer >= (Uint32)at->feedbackDuration) {
                    /* Appliquer résultat */
                    if (at->resultBonus > 0) {
                        player.score += 10;
                        player.vies  = (player.vies < 9) ? player.vies + 1 : 9;
                    } else {
                        player.vies--;
                        if (player.score >= 5) player.score -= 5;
                    }
                    at->state = ETRIG_DONE + 100; /* désactivé */
                    solved++;
                    activeTrigger = -1;
                    gs = GS_PLAY;

                    if (player.vies <= 0)      gs = GS_LOSE;
                    if (solved >= NB_TRIGGERS) gs = GS_WIN;
                }
            }
        }

        /* ── RENDER ─────────────────────────────────────────────────── */
        SDL_SetRenderDrawColor(renderer, 10, 5, 20, 255);
        SDL_RenderClear(renderer);

        /* Background */
        SDL_RenderCopy(renderer, bg, NULL, NULL);

        /* Sol (ligne de référence) */
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 80, 40, 20, 180);
        SDL_Rect sol = { 0, GROUND_Y + player.h - 4, sw, 8 };
        SDL_RenderFillRect(renderer, &sol);

        /* Objets énigme sur la map */
        for (int i = 0; i < NB_TRIGGERS; i++)
            renderEnigmeTrigger(&triggers[i], renderer, now);

        /* Joueur */
        blitPlayer(renderer, &player);

        /* HUD */
        renderHUD(renderer, font, &player, solved, NB_TRIGGERS);

        /* Si énigme en cours → superposer l'interface */
        if (gs == GS_ENIGME && activeTrigger >= 0) {
            EnigmeTrigger *at = &triggers[activeTrigger];
            if (at->state == ETRIG_RUNNING) {
                renderEnigme(&at->enigme, renderer, font, fontSmall, fontTiny);
            } else if (at->state == ETRIG_SUCCESS ||
                       at->state == ETRIG_FAIL) {
                /* Overlay feedback */
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                int bw = sw * 50 / 100, bh = sh * 28 / 100;
                SDL_Rect box = { sw/2-bw/2, sh/2-bh/2, bw, bh };
                if (at->state == ETRIG_SUCCESS) {
                    SDL_SetRenderDrawColor(renderer, 0, 40, 15, 230);
                    SDL_RenderFillRect(renderer, &box);
                    SDL_SetRenderDrawColor(renderer, 50, 220, 80, 255);
                    SDL_RenderDrawRect(renderer, &box);
                    overlayText(renderer, font,
                        "BONNE REPONSE !",
                        "Appuyez sur ENTREE pour continuer",
                        0, 30, 10,
                        50, 220, 80);
                } else {
                    SDL_SetRenderDrawColor(renderer, 40, 0, 0, 230);
                    SDL_RenderFillRect(renderer, &box);
                    SDL_SetRenderDrawColor(renderer, 220, 50, 50, 255);
                    SDL_RenderDrawRect(renderer, &box);
                    overlayText(renderer, font,
                        "MAUVAISE REPONSE...",
                        "Appuyez sur ENTREE pour continuer",
                        30, 0, 0,
                        220, 50, 50);
                }
                /* Barre de délai */
                float ratio = (float)at->feedbackTimer /
                              (float)at->feedbackDuration;
                if (ratio > 1.0f) ratio = 1.0f;
                SDL_Rect bar = { box.x + 20, box.y + bh - 20,
                                 (int)((bw-40)*ratio), 8 };
                if (at->state == ETRIG_SUCCESS)
                    SDL_SetRenderDrawColor(renderer, 50, 220, 80, 200);
                else
                    SDL_SetRenderDrawColor(renderer, 220, 50, 50, 200);
                SDL_RenderFillRect(renderer, &bar);
            }
        }

        /* Écrans WIN / LOSE */
        if (gs == GS_WIN) {
            overlayText(renderer, font,
                "FELICITATIONS ! Toutes les enigmes resolues !",
                "Appuyez sur ENTREE pour quitter",
                0, 30, 10, 50, 255, 80);
        }
        if (gs == GS_LOSE) {
            overlayText(renderer, font,
                "GAME OVER ! Vous avez perdu toutes vos vies...",
                "Appuyez sur ENTREE pour quitter",
                30, 0, 0, 255, 50, 50);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    /* ── Libération ── */
    for (int i = 0; i < NB_TRIGGERS; i++)
        freeEnigmeTrigger(&triggers[i]);
    if (player.sprite.sheetRight) SDL_DestroyTexture(player.sprite.sheetRight);
    if (player.sprite.sheetLeft)  SDL_DestroyTexture(player.sprite.sheetLeft);
    SDL_DestroyTexture(bg);

    return 0;
}

/* ─────────────────────────────────────────────────────────────────────
   MAIN  (point d'entrée autonome — fusionnez avec mainfarah.c si besoin)
   ───────────────────────────────────────────────────────────────────── */
int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    Mix_Init(MIX_INIT_MP3);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 512);

    int scw = SCREEN_W, sch = SCREEN_H;
    SDL_Window   *win = SDL_CreateWindow("Shadow of Gotham",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        scw, sch, SDL_WINDOW_SHOWN);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    /* Polices — adaptez le chemin selon votre projet */
    const char *fontPath = "assets/batmfa.ttf";
    TTF_Font *fontBig   = TTF_OpenFont(fontPath, 28);
    TTF_Font *fontSmall = TTF_OpenFont(fontPath, 18);
    TTF_Font *fontTiny  = TTF_OpenFont(fontPath, 12);
    if (!fontBig) {
        /* Fallback système */
        fontPath  = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
        fontBig   = TTF_OpenFont(fontPath, 28);
        fontSmall = TTF_OpenFont(fontPath, 18);
        fontTiny  = TTF_OpenFont(fontPath, 12);
    }
    if (!fontBig) {
        fprintf(stderr, "[ERROR] Impossible de charger la police: %s\n",
                TTF_GetError());
        return 1;
    }

    /* Choisir le personnage : 0=Batman, 1=Catwoman */
    int isCat = 0;   /* changez à 1 pour Catwoman */

    /* Lancer la boucle de jeu */
    runJeu(ren, fontBig, fontSmall, fontTiny, isCat);

    TTF_CloseFont(fontBig);
    TTF_CloseFont(fontSmall);
    TTF_CloseFont(fontTiny);
    Mix_CloseAudio();
    Mix_Quit();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}
