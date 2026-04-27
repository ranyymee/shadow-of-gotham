#include "minimap.h"

/* ═══════════════════════════════════════════════════════
   MINIMAP
═══════════════════════════════════════════════════════ */

static SDL_Texture *makeDot(SDL_Renderer *r, int size, Uint8 red, Uint8 green, Uint8 blue)
{
    SDL_Surface *s = SDL_CreateRGBSurface(0, size, size, 32,
                         0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    if (!s) return NULL;
    SDL_FillRect(s, NULL, SDL_MapRGB(s->format, red, green, blue));
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_FreeSurface(s);
    return t;
}

Minimap *createMinimap(SDL_Renderer *renderer,
                       const char   *imagePath,
                       SDL_Rect      position)
{
    Minimap *m = (Minimap *)malloc(sizeof(Minimap));
    if (!m) return NULL;

    /* try to load the miniature background image */
    m->backgroundTexture = NULL;
    if (imagePath) {
        SDL_Surface *s = IMG_Load(imagePath);
        if (s) {
            m->backgroundTexture = SDL_CreateTextureFromSurface(renderer, s);
            SDL_FreeSurface(s);
        } else {
            printf("[minimap] image not found: %s — using solid background\n", imagePath);
        }
    }

    m->minimapPosition = position;

    /* red dot — player 1 */
    m->playerTexture  = makeDot(renderer, PLAYER_DOT_SIZE,  255,   0,   0);
    /* magenta dot — player 2 */
    m->player2Texture = makeDot(renderer, PLAYER2_DOT_SIZE, 220,  60, 220);
    /* yellow dot — enemies */
    m->enemyTexture   = makeDot(renderer, ENEMY_DOT_SIZE,   255, 255,   0);

    m->playerPosition  = (SDL_Rect){position.x, position.y,
                                    PLAYER_DOT_SIZE,  PLAYER_DOT_SIZE};
    m->player2Position = (SDL_Rect){position.x, position.y,
                                    PLAYER2_DOT_SIZE, PLAYER2_DOT_SIZE};
    return m;
}

/* clamp helper */
static int clampInt(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void updateMinimap(Minimap *m,
                   int p1WorldX, int p1WorldY,
                   int p2WorldX, int p2WorldY,
                   int worldW,   int worldH)
{
    if (!m || worldW <= 0 || worldH <= 0) return;

    int mw = m->minimapPosition.w;
    int mh = m->minimapPosition.h;
    int ox = m->minimapPosition.x;
    int oy = m->minimapPosition.y;

    /* player 1 */
    m->playerPosition.x = clampInt(ox + p1WorldX * mw / worldW,
                                   ox, ox + mw - PLAYER_DOT_SIZE);
    m->playerPosition.y = clampInt(oy + p1WorldY * mh / worldH,
                                   oy, oy + mh - PLAYER_DOT_SIZE);

    /* player 2 */
    m->player2Position.x = clampInt(ox + p2WorldX * mw / worldW,
                                    ox, ox + mw - PLAYER2_DOT_SIZE);
    m->player2Position.y = clampInt(oy + p2WorldY * mh / worldH,
                                    oy, oy + mh - PLAYER2_DOT_SIZE);
}

void renderMinimap(SDL_Renderer  *renderer,
                   Minimap       *m,
                   MinimapEnemy   enemies[],
                   int            enemyCount,
                   int            worldW,
                   int            worldH)
{
    if (!m) return;

    /* 1. background */
    if (m->backgroundTexture) {
        SDL_RenderCopy(renderer, m->backgroundTexture,
                       NULL, &m->minimapPosition);
    } else {
        /* fallback: dark semi-transparent rect */
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 10, 10, 30, 200);
        SDL_RenderFillRect(renderer, &m->minimapPosition);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }

    /* border */
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
    SDL_RenderDrawRect(renderer, &m->minimapPosition);

    int mw = m->minimapPosition.w;
    int mh = m->minimapPosition.h;
    int ox = m->minimapPosition.x;
    int oy = m->minimapPosition.y;

    /* 2. enemy yellow dots */
    if (worldW > 0 && worldH > 0) {
        for (int i = 0; i < enemyCount; i++) {
            if (!enemies[i].active) continue;
            int ex = clampInt(ox + enemies[i].x * mw / worldW,
                              ox, ox + mw - ENEMY_DOT_SIZE);
            int ey = clampInt(oy + enemies[i].y * mh / worldH,
                              oy, oy + mh - ENEMY_DOT_SIZE);
            SDL_Rect dot = {ex, ey, ENEMY_DOT_SIZE, ENEMY_DOT_SIZE};
            SDL_RenderCopy(renderer, m->enemyTexture, NULL, &dot);
        }
    }

    /* 3. player 2 dot */
    SDL_RenderCopy(renderer, m->player2Texture, NULL, &m->player2Position);

    /* 4. player 1 dot — always on top */
    SDL_RenderCopy(renderer, m->playerTexture, NULL, &m->playerPosition);
}

void freeMinimap(Minimap *m)
{
    if (!m) return;
    if (m->backgroundTexture) SDL_DestroyTexture(m->backgroundTexture);
    if (m->playerTexture)     SDL_DestroyTexture(m->playerTexture);
    if (m->player2Texture)    SDL_DestroyTexture(m->player2Texture);
    if (m->enemyTexture)      SDL_DestroyTexture(m->enemyTexture);
    free(m);
}

/* ═══════════════════════════════════════════════════════
   BACKGROUND SHAKE ANIMATION
═══════════════════════════════════════════════════════ */

void triggerShake(ShakeState *shake)
{
    if (!shake) return;
    /* only restart if not already shaking or current shake is almost done */
    if (!shake->active || shake->framesLeft < 4) {
        shake->active     = 1;
        shake->framesLeft = SHAKE_FRAMES;
        shake->offsetX    = 0;
        shake->offsetY    = 0;
    }
}

void updateShake(ShakeState *shake)
{
    if (!shake || !shake->active) {
        if (shake) { shake->offsetX = 0; shake->offsetY = 0; }
        return;
    }
    if (shake->framesLeft > 0) {
        float t   = (float)shake->framesLeft / SHAKE_FRAMES;
        int range = (int)(SHAKE_INTENSITY * t) + 1;
        shake->offsetX = (rand() % (2 * range + 1)) - range;
        shake->offsetY = (rand() % (2 * range + 1)) - range;
        shake->framesLeft--;
    } else {
        shake->active  = 0;
        shake->offsetX = 0;
        shake->offsetY = 0;
    }
}
