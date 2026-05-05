/*
 * enigme_trigger.c
 * Implémentation du système de déclenchement d'énigme par collision.
 *
 * Flux :
 *   IDLE  ──(collision)──► RUNNING ──(bonne réponse)──► SUCCESS ──► DONE
 *                                  └──(mauvaise/timeout)──► FAIL  ──► DONE
 */

#include "enigme_trigger.h"
#include <stdio.h>
#include <math.h>

/* ── Durées ─────────────────────────────────────────────────────────── */
#define FEEDBACK_SUCCESS_MS  2200
#define FEEDBACK_FAIL_MS     2200
#define BOB_SPEED            2.5f   /* rad/s pour l'animation de flottement */

/* ── Couleurs feedback ──────────────────────────────────────────────── */
#define COL_SUCCESS_R  50
#define COL_SUCCESS_G  220
#define COL_SUCCESS_B  80
#define COL_FAIL_R     220
#define COL_FAIL_G     50
#define COL_FAIL_B     50

/* ─────────────────────────────────────────────────────────────────────
   Helpers de rendu (inline, pas de dépendance à header.h)
   ───────────────────────────────────────────────────────────────────── */
static void fillRect(SDL_Renderer *r, SDL_Rect rect,
                     Uint8 R, Uint8 G, Uint8 B, Uint8 A)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, R, G, B, A);
    SDL_RenderFillRect(r, &rect);
}

static void drawRect(SDL_Renderer *r, SDL_Rect rect,
                     Uint8 R, Uint8 G, Uint8 B, Uint8 A, int thick)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, R, G, B, A);
    for (int i = 0; i < thick; i++) {
        SDL_Rect b = { rect.x - i, rect.y - i,
                       rect.w + i * 2, rect.h + i * 2 };
        SDL_RenderDrawRect(r, &b);
    }
}

static void drawTextCentered(SDL_Renderer *r, TTF_Font *font,
                             const char *txt,
                             Uint8 R, Uint8 G, Uint8 B,
                             SDL_Rect container)
{
    if (!font || !txt || !txt[0]) return;
    SDL_Color col = { R, G, B, 255 };
    SDL_Surface *su = TTF_RenderUTF8_Blended(font, txt, col);
    if (!su) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, su);
    SDL_FreeSurface(su);
    if (!tex) return;
    int tw, th;
    SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
    SDL_Rect dst = {
        container.x + (container.w - tw) / 2,
        container.y + (container.h - th) / 2,
        tw, th
    };
    SDL_RenderCopy(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

/* ─────────────────────────────────────────────────────────────────────
   Rendu du feedback (SUCCESS / FAIL)
   ───────────────────────────────────────────────────────────────────── */
static void renderFeedback(EnigmeTrigger *t, SDL_Renderer *r, TTF_Font *font)
{
    int sw, sh;
    SDL_GetRendererOutputSize(r, &sw, &sh);

    /* Fond semi-transparent */
    SDL_Rect full = { 0, 0, sw, sh };
    if (t->state == ETRIG_SUCCESS)
        fillRect(r, full, 0, 30, 10, 180);
    else
        fillRect(r, full, 30, 0,  0, 180);

    /* Boîte centrale */
    int bw = sw * 50 / 100, bh = sh * 30 / 100;
    SDL_Rect box = { sw / 2 - bw / 2, sh / 2 - bh / 2, bw, bh };

    if (t->state == ETRIG_SUCCESS) {
        fillRect(r, box, 0,  50, 20, 230);
        drawRect(r, box, COL_SUCCESS_R, COL_SUCCESS_G, COL_SUCCESS_B, 255, 3);
        drawTextCentered(r, font,
            "✓  BONNE REPONSE !",
            COL_SUCCESS_R, COL_SUCCESS_G, COL_SUCCESS_B, box);
    } else {
        fillRect(r, box, 50, 0, 0, 230);
        drawRect(r, box, COL_FAIL_R, COL_FAIL_G, COL_FAIL_B, 255, 3);
        drawTextCentered(r, font,
            "✗  MAUVAISE REPONSE",
            COL_FAIL_R, COL_FAIL_G, COL_FAIL_B, box);
    }

    /* Barre de progression du délai */
    float ratio = (float)t->feedbackTimer / (float)t->feedbackDuration;
    if (ratio > 1.0f) ratio = 1.0f;
    SDL_Rect barBg = { box.x + 20, box.y + bh - 22, bw - 40, 10 };
    SDL_Rect barFg = { barBg.x, barBg.y, (int)(barBg.w * ratio), 10 };
    fillRect(r, barBg, 20, 20, 20, 200);
    if (t->state == ETRIG_SUCCESS)
        fillRect(r, barFg, COL_SUCCESS_R, COL_SUCCESS_G, COL_SUCCESS_B, 220);
    else
        fillRect(r, barFg, COL_FAIL_R, COL_FAIL_G, COL_FAIL_B, 220);
}

/* ─────────────────────────────────────────────────────────────────────
   initEnigmeTrigger
   ───────────────────────────────────────────────────────────────────── */
void initEnigmeTrigger(EnigmeTrigger *t, SDL_Renderer *renderer,
                       int x, int y, int w, int h,
                       const char *spritePath)
{
    /* Hitbox sur la map */
    t->hitbox    = (SDL_Rect){ x, y, w, h };
    t->bobPhase  = 0.0f;
    t->state     = ETRIG_IDLE;
    t->resultBonus     = 0;
    t->feedbackTimer   = 0;
    t->feedbackDuration = FEEDBACK_SUCCESS_MS;

    /* Sprite de l'objet énigme */
    t->sprite = IMG_LoadTexture(renderer, spritePath);
    if (!t->sprite) {
        fprintf(stderr, "[WARN] enigme_trigger: cannot load '%s': %s\n",
                spritePath, IMG_GetError());
        /* Placeholder coloré si l'image est absente */
        t->sprite = SDL_CreateTexture(renderer,
                        SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET, w, h);
        if (t->sprite) {
            SDL_SetRenderTarget(renderer, t->sprite);
            SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);
            SDL_RenderClear(renderer);
            SDL_SetRenderTarget(renderer, NULL);
        }
    }
    if (t->sprite)
        SDL_SetTextureBlendMode(t->sprite, SDL_BLENDMODE_BLEND);

    /* Initialiser l'énigme embarquée */
    initEnigme(&t->enigme, renderer);
}

/* ─────────────────────────────────────────────────────────────────────
   updateEnigmeTrigger
   ───────────────────────────────────────────────────────────────────── */
void updateEnigmeTrigger(EnigmeTrigger *t,
                         SDL_Renderer  *renderer,
                         TTF_Font      *font,
                         TTF_Font      *fontSmall,
                         TTF_Font      *fontTiny,
                         SDL_Rect      *playerRect,
                         SDL_Event     *event,
                         float          dt)
{
    switch (t->state) {

    /* ── IDLE : attendre la collision ── */
    case ETRIG_IDLE:
        t->bobPhase += BOB_SPEED * dt;
        if (playerRect && rectsOverlap(&t->hitbox, playerRect)) {
            /* Collision ! On lance l'énigme */
            t->state = ETRIG_RUNNING;
            fprintf(stdout, "[ENIGME] Collision détectée, énigme lancée.\n");
        }
        break;

    /* ── RUNNING : énigme en cours ── */
    case ETRIG_RUNNING:
        /* Passer l'événement à l'énigme */
        if (event)
            handleEnigmeEvent(&t->enigme, event);

        /* Mise à jour logique de l'énigme (chrono, etc.) */
        updateEnigme(&t->enigme);

        /* Rendu de l'énigme (appelé ici pour simplicité) */
        renderEnigme(&t->enigme, renderer, font, fontSmall, fontTiny);

        /* Vérifier si l'énigme est terminée */
        if (t->enigme.questionIndex >= NB_QUESTIONS) {
            /* Toutes les questions répondues */
            if (t->enigme.score >= NB_QUESTIONS / 2) {
                t->state            = ETRIG_SUCCESS;
                t->resultBonus      = +1;   /* +1 vie ou score */
                t->feedbackDuration = FEEDBACK_SUCCESS_MS;
            } else {
                t->state            = ETRIG_FAIL;
                t->resultBonus      = -1;   /* -1 vie ou score */
                t->feedbackDuration = FEEDBACK_FAIL_MS;
            }
            t->feedbackTimer = 0;
        }
        /* Aussi : si le chrono expire sur UNE question et que le joueur
           répond mal plusieurs fois, l'enigme.c gère ça en interne.
           On surveille juste questionIndex. */
        break;

    /* ── SUCCESS / FAIL : afficher le feedback ── */
    case ETRIG_SUCCESS:
    case ETRIG_FAIL:
        renderFeedback(t, renderer, font);
        t->feedbackTimer += (Uint32)(dt * 1000.0f);
        if (t->feedbackTimer >= (Uint32)t->feedbackDuration) {
            t->state = ETRIG_DONE;
            fprintf(stdout, "[ENIGME] Résultat final : %s (bonus=%d)\n",
                    t->state == ETRIG_DONE && t->resultBonus > 0
                    ? "SUCCÈS" : "ECHEC",
                    t->resultBonus);
        }
        break;

    /* ── DONE : rien à faire, l'appelant lit resultBonus ── */
    case ETRIG_DONE:
        break;
    }
}

/* ─────────────────────────────────────────────────────────────────────
   renderEnigmeTrigger  (objet sur la map)
   ───────────────────────────────────────────────────────────────────── */
void renderEnigmeTrigger(EnigmeTrigger *t, SDL_Renderer *renderer, Uint32 ticks)
{
    if (t->state != ETRIG_IDLE) return;  /* ne dessiner que si actif sur la map */

    /* Animation de flottement (bob) */
    float bob = sinf((float)ticks / 600.0f) * 6.0f;
    SDL_Rect dst = {
        t->hitbox.x,
        t->hitbox.y + (int)bob,
        t->hitbox.w,
        t->hitbox.h
    };

    /* Halo pulsant sous l'objet */
    float pulse = 0.5f + 0.5f * sinf((float)ticks / 400.0f);
    SDL_Rect halo = { dst.x - 8, dst.y + dst.h - 8,
                      dst.w + 16, 12 };
    fillRect(renderer, halo, 255, 200, 0, (Uint8)(30 + pulse * 40));

    /* Sprite */
    if (t->sprite) {
        /* Légère pulsation de luminosité */
        Uint8 mod = (Uint8)(200 + (int)(pulse * 55));
        SDL_SetTextureColorMod(t->sprite, 255, mod, (Uint8)(mod * 180 / 255));
        SDL_RenderCopy(renderer, t->sprite, NULL, &dst);
        SDL_SetTextureColorMod(t->sprite, 255, 255, 255);
    }

    /* Contour jaune */
    drawRect(renderer, dst, 255, 200, 0, (Uint8)(80 + (int)(pulse * 80)), 2);

    /* Indicateur "!" flottant au-dessus */
    SDL_Rect bubble = { dst.x + dst.w / 2 - 14,
                        dst.y - 30,
                        28, 24 };
    fillRect(renderer, bubble, 255, 200, 0, 220);
    drawRect(renderer, bubble, 255, 255, 255, 200, 1);
    /* Le "!" est dessiné avec des lignes SDL */
    int cx = bubble.x + bubble.w / 2;
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderDrawLine(renderer, cx, bubble.y + 4, cx, bubble.y + 14);
    SDL_RenderDrawLine(renderer, cx, bubble.y + 17, cx, bubble.y + 19);
    SDL_RenderDrawLine(renderer, cx - 1, bubble.y + 4, cx - 1, bubble.y + 14);
    SDL_RenderDrawLine(renderer, cx + 1, bubble.y + 4, cx + 1, bubble.y + 14);
}

/* ─────────────────────────────────────────────────────────────────────
   freeEnigmeTrigger
   ───────────────────────────────────────────────────────────────────── */
void freeEnigmeTrigger(EnigmeTrigger *t)
{
    freeEnigme(&t->enigme);
    if (t->sprite) {
        SDL_DestroyTexture(t->sprite);
        t->sprite = NULL;
    }
    t->state = ETRIG_IDLE;
}
