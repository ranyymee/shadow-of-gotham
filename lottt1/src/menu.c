#include "menu.h"
#include <string.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>

/* ══════════════════════════════════════════════════════════════════════════
   Definition des personnages
   Adapte les chemins costumeAvatars[] selon tes vraies images PNG.
   ══════════════════════════════════════════════════════════════════════════ */
const CharacterDef CHARACTERS[CHAR_COUNT] = {
    [CHAR_BATMAN] = {
        .name         = "Batman",
        .sheetRight   = { "batman_right_1.png", "batman_right_2.png", "batman_right_3.png" },
        .sheetLeft    = { "batman_left_1.png",  "batman_left_2.png",  "batman_left_3.png"  },
        .costumeNames = { "COSTUME GRIS", "COSTUME BLEU", "COSTUME ROUGE" },
        /* Une image portrait par costume — mets tes PNG ici */
        .costumeAvatars = { "assets/batman_avatar_1.png",
                            "assets/batman_avatar_2.png",
                            "assets/batman_avatar_3.png" }
    },
    [CHAR_CATWOMAN] = {
        .name         = "Catwoman",
        .sheetRight   = { "catwoman_right_1.png", "catwoman_right_2.png", "catwoman_right_3.png" },
        .sheetLeft    = { "catwoman_left_1.png",  "catwoman_left_2.png",  "catwoman_left_3.png"  },
        .costumeNames = { "COSTUME NOIR", "COSTUME GRIS", "COSTUME VIOLET" },
        .costumeAvatars = { "assets/catwoman_avatar_1.png",
                            "assets/catwoman_avatar_2.png",
                            "assets/catwoman_avatar_3.png" }
    }
};

/* ══════════════════════════════════════════════════════════════════════════
   Constantes ecran
   ══════════════════════════════════════════════════════════════════════════ */
#define SCREEN_W 900
#define SCREEN_H 620

/* ══════════════════════════════════════════════════════════════════════════
   Utilitaires de rendu
   ══════════════════════════════════════════════════════════════════════════ */
static SDL_Texture *loadTex(SDL_Renderer *r, const char *path)
{
    SDL_Texture *t = IMG_LoadTexture(r, path);
    /* Pas de printf d'erreur pour les avatars manquants — le code gere NULL */
    return t;
}

static void drawText(SDL_Renderer *r, TTF_Font *f,
                     const char *txt, int x, int y, SDL_Color col)
{
    if (!f || !txt) return;
    SDL_Surface *s = TTF_RenderUTF8_Blended(f, txt, col);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_Rect dst = { x, y, s->w, s->h };
    SDL_FreeSurface(s);
    SDL_RenderCopy(r, t, NULL, &dst);
    SDL_DestroyTexture(t);
}

static void drawTextCentered(SDL_Renderer *r, TTF_Font *f,
                              const char *txt, int cx, int y, SDL_Color col)
{
    if (!f || !txt) return;
    SDL_Surface *s = TTF_RenderUTF8_Blended(f, txt, col);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    SDL_Rect dst = { cx - s->w / 2, y, s->w, s->h };
    SDL_FreeSurface(s);
    SDL_RenderCopy(r, t, NULL, &dst);
    SDL_DestroyTexture(t);
}

/* Bouton style screenshot : fond sombre, bordure claire, texte centre */
static void drawBtn(SDL_Renderer *r, TTF_Font *f,
                    SDL_Rect rect, const char *label,
                    int hovered, SDL_Color borderCol)
{
    /* Fond */
    if (hovered)
        SDL_SetRenderDrawColor(r, 30, 30, 80, 230);
    else
        SDL_SetRenderDrawColor(r, 15, 15, 50, 210);
    SDL_RenderFillRect(r, &rect);

    /* Bordure */
    SDL_SetRenderDrawColor(r, borderCol.r, borderCol.g, borderCol.b, 255);
    SDL_RenderDrawRect(r, &rect);

    SDL_Color white = {255, 255, 255, 255};
    drawTextCentered(r, f, label, rect.x + rect.w / 2,
                     rect.y + rect.h / 2 - 10, white);
}

static int inRect(int mx, int my, SDL_Rect rect)
{
    return mx >= rect.x && mx <= rect.x + rect.w &&
           my >= rect.y && my <= rect.y + rect.h;
}

/* Dessine le fond semi-transparent de l'overlay menu */
static void drawOverlay(SDL_Renderer *r, SDL_Rect rect)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 20, 180);
    SDL_RenderFillRect(r, &rect);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

/* ══════════════════════════════════════════════════════════════════════════
   Sous-menu remapping des touches
   ══════════════════════════════════════════════════════════════════════════ */
static int remapMenu(SDL_Renderer *renderer, TTF_Font *font,
                     SDL_Texture *bg, InputConfig *cfg, const char *title)
{
    const char  *actionNames[] = {
        "Gauche", "Droite", "Sauter", "Voler",
        "Attaque","Kick",   "Baisser"
    };
    SDL_Scancode *ptrs[] = {
        &cfg->left, &cfg->right, &cfg->up, &cfg->fly,
        &cfg->punch, &cfg->kick, &cfg->crouch
    };
    const int N = 7;
    int waiting = -1;

    SDL_Color white  = {255, 255, 255, 255};
    SDL_Color yellow = {255, 220,   0, 255};
    SDL_Color orange = {255, 160,   0, 255};
    SDL_Color border = {180, 180, 180, 255};

    while (1) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return 0;
            if (e.type == SDL_KEYDOWN) {
                if (waiting >= 0) {
                    *ptrs[waiting] = e.key.keysym.scancode;
                    waiting = -1;
                } else if (e.key.keysym.scancode == SDL_SCANCODE_RETURN ||
                           e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    return 1;
                }
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x, my = e.button.y;
                for (int i = 0; i < N; i++) {
                    SDL_Rect btn = { 300, 100 + i * 52, 250, 40 };
                    if (inRect(mx, my, btn)) { waiting = i; break; }
                }
                SDL_Rect ok = { 330, 100 + N * 52 + 16, 180, 42 };
                if (inRect(mx, my, ok)) return 1;
            }
        }

        /* Fond */
        SDL_RenderClear(renderer);
        if (bg) SDL_RenderCopy(renderer, bg, NULL, NULL);
        SDL_Rect overlay = { 0, 0, SCREEN_W, SCREEN_H };
        drawOverlay(renderer, overlay);

        drawTextCentered(renderer, font, title, SCREEN_W / 2, 50, yellow);
        drawTextCentered(renderer, font,
            "Clique sur une action pour changer la touche",
            SCREEN_W / 2, 78, orange);

        for (int i = 0; i < N; i++) {
            drawText(renderer, font, actionNames[i], 110, 112 + i * 52, white);
            const char *label = (waiting == i)
                ? ">>> appuie <<<"
                : SDL_GetKeyName(SDL_GetKeyFromScancode(*ptrs[i]));
            SDL_Rect btn = { 300, 100 + i * 52, 250, 40 };
            SDL_Color bc = (waiting == i) ? (SDL_Color){255, 200, 0, 255} : border;
            drawBtn(renderer, font, btn, label, waiting == i, bc);
        }

        SDL_Rect ok = { 330, 100 + N * 52 + 16, 180, 42 };
        SDL_Color redBorder = {255, 80, 80, 255};
        drawBtn(renderer, font, ok, "OK", 0, redBorder);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}

/* ══════════════════════════════════════════════════════════════════════════
   Ecran 1 — SOUS MENU JOUEUR  (comme Image 1)
   Retourne : 1=Solo, 2=Multi, 0=quitter/retour
   ══════════════════════════════════════════════════════════════════════════ */
static int modeScreen(SDL_Renderer *renderer, TTF_Font *font, SDL_Texture *bg)
{
    SDL_Color yellow = {255, 220,   0, 255};
    SDL_Color white  = {200, 200, 200, 255};
    SDL_Color border = {180, 180, 180, 255};
    SDL_Color redBorder = {255, 80, 80, 255};

    /* Boutons centres comme sur le screenshot */
    SDL_Rect btnSolo   = { SCREEN_W/2 - 150, 200, 300, 60 };
    SDL_Rect btnMulti  = { SCREEN_W/2 - 150, 295, 300, 60 };
    SDL_Rect btnRetour = { SCREEN_W/2 - 150, 420, 300, 60 };

    int mx = 0, my = 0;

    while (1) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return 0;
            if (e.type == SDL_KEYDOWN) {
                SDL_Scancode sc = e.key.keysym.scancode;
                if (sc == SDL_SCANCODE_ESCAPE) return 0;
                if (sc == SDL_SCANCODE_RETURN) return 1; /* defaut solo */
            }
            if (e.type == SDL_MOUSEMOTION) { mx = e.motion.x; my = e.motion.y; }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                mx = e.button.x; my = e.button.y;
                if (inRect(mx, my, btnSolo))   return 1;
                if (inRect(mx, my, btnMulti))  return 2;
                if (inRect(mx, my, btnRetour)) return 0;
            }
        }

        SDL_RenderClear(renderer);
        if (bg) SDL_RenderCopy(renderer, bg, NULL, NULL);

        /* Overlay leger */
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 10, 120);
        SDL_Rect full = { 0, 0, SCREEN_W, SCREEN_H };
        SDL_RenderFillRect(renderer, &full);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        drawTextCentered(renderer, font, "SOUS MENU JOUEUR",
                         SCREEN_W / 2, 90, yellow);

        drawBtn(renderer, font, btnSolo,   "MONO JOUEUR",   inRect(mx,my,btnSolo),   border);
        drawBtn(renderer, font, btnMulti,  "MULTI JOUEUR",  inRect(mx,my,btnMulti),  border);
        drawBtn(renderer, font, btnRetour, "RETOUR",        inRect(mx,my,btnRetour), redBorder);

        drawTextCentered(renderer, font,
            "CLIC OU ENTREE POUR SELECTIONNER",
            SCREEN_W / 2, SCREEN_H - 36, white);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}

/* ══════════════════════════════════════════════════════════════════════════
   Carte costume avec avatar image  (comme Images 2 et 3)
   ══════════════════════════════════════════════════════════════════════════ */
static void drawCostumeCard(SDL_Renderer *r, TTF_Font *f,
                             SDL_Rect rect,
                             SDL_Texture *avatar,
                             const char *costumeName,
                             int selected, int isCurrent)
{
    /* Fond de la carte */
    if (selected) {
        /* Dore comme sur le screenshot selectionne */
        SDL_SetRenderDrawColor(r, 100, 80, 0, 220);
    } else {
        SDL_SetRenderDrawColor(r, 10, 10, 60, 200);
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(r, &rect);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    /* Bordure */
    if (selected)
        SDL_SetRenderDrawColor(r, 200, 160, 0, 255);  /* doree */
    else
        SDL_SetRenderDrawColor(r, 60, 80, 160, 255);  /* bleu fonce */
    SDL_RenderDrawRect(r, &rect);

    /* Image avatar dans la case gauche de la carte */
    int avatarW = rect.h - 8;
    SDL_Rect avatarRect = { rect.x + 4, rect.y + 4, avatarW, avatarW };
    if (avatar) {
        SDL_RenderCopy(r, avatar, NULL, &avatarRect);
    } else {
        /* Placeholder si pas d'image */
        SDL_SetRenderDrawColor(r, 40, 40, 100, 255);
        SDL_RenderFillRect(r, &avatarRect);
        SDL_SetRenderDrawColor(r, 80, 80, 150, 255);
        SDL_RenderDrawRect(r, &avatarRect);
    }

    /* Nom du costume, a droite de l'avatar */
    SDL_Color nameCol = selected
        ? (SDL_Color){ 255, 220, 0, 255 }   /* dore si selectionne */
        : (SDL_Color){ 220, 220, 255, 255 };
    int textX = rect.x + avatarW + 12;
    int textY = rect.y + rect.h / 2 - 10;

    /* Prefixe "~ " si selectionne (comme sur le screenshot) */
    char label[64];
    if (isCurrent)
        snprintf(label, sizeof(label), "~ %s", costumeName);
    else
        snprintf(label, sizeof(label), "%s", costumeName);

    drawText(r, f, label, textX, textY, nameCol);
}

/* ══════════════════════════════════════════════════════════════════════════
   Ecran de selection personnage/costume  (Images 2 et 3)
   Retourne : 1=valider, -1=retour, 0=quitter
   ══════════════════════════════════════════════════════════════════════════ */
static int charSelectScreen(SDL_Renderer *renderer, TTF_Font *font,
                             SDL_Texture *bg, MenuResult *result,
                             SDL_Texture *avatarsP1[COSTUME_COUNT],
                             SDL_Texture *avatarsP2[COSTUME_COUNT])
{
    const int isSolo = (result->mode == MODE_SOLO);

    SDL_Color yellow  = {255, 220,   0, 255};
    SDL_Color cyan    = {  0, 210, 210, 255};
    SDL_Color magenta = {210,  30, 210, 255};
    SDL_Color orange  = {255, 160,   0, 255};
    SDL_Color white   = {220, 220, 220, 255};
    SDL_Color dim     = {120, 120, 160, 255};
    SDL_Color border  = {160, 160, 160, 255};
    SDL_Color redBorder = {255, 80, 80, 255};

    /* Geometrie cartes */
    const int CARD_H   = 76;
    const int CARD_GAP = 6;
    const int CARD_Y0  = 140;

    /* Colonnes */
    const int L1 = 15,  W1 = 420;   /* J1 */
    const int L2 = 455, W2 = 420;   /* J2 */

    /* Curseur clavier */
    int curP1 = result->costumeP1;
    int curP2 = result->costumeP2;
    int focus  = 0;  /* 0=J1, 1=J2 (utile en mode multi) */

    /* Titre */
    char titleBuf[64];
    snprintf(titleBuf, sizeof(titleBuf), "MODE: %s",
             isSolo ? "MONO JOUEUR" : "MULTI JOUEUR");

    while (1) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return 0;

            if (e.type == SDL_KEYDOWN) {
                SDL_Scancode sc = e.key.keysym.scancode;

                if (sc == SDL_SCANCODE_ESCAPE) return -1;
                if (sc == SDL_SCANCODE_RETURN) {
                    result->costumeP1 = curP1;
                    result->costumeP2 = curP2;
                    return 1;
                }

                /* Navigation J1 avec fleches H/B */
                if (sc == SDL_SCANCODE_UP) {
                    if (!isSolo && focus == 1) {
                        curP2 = (curP2 - 1 + COSTUME_COUNT) % COSTUME_COUNT;
                    } else {
                        curP1 = (curP1 - 1 + COSTUME_COUNT) % COSTUME_COUNT;
                    }
                }
                if (sc == SDL_SCANCODE_DOWN) {
                    if (!isSolo && focus == 1) {
                        curP2 = (curP2 + 1) % COSTUME_COUNT;
                    } else {
                        curP1 = (curP1 + 1) % COSTUME_COUNT;
                    }
                }
                /* En multi, W/S pour naviguer colonne J2 */
                if (!isSolo) {
                    if (sc == SDL_SCANCODE_W)
                        curP2 = (curP2 - 1 + COSTUME_COUNT) % COSTUME_COUNT;
                    if (sc == SDL_SCANCODE_S)
                        curP2 = (curP2 + 1) % COSTUME_COUNT;
                }
                /* Tab pour switcher le focus clavier */
                if (sc == SDL_SCANCODE_TAB && !isSolo)
                    focus = 1 - focus;
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x, my = e.button.y;

                /* Cartes J1 */
                for (int i = 0; i < COSTUME_COUNT; i++) {
                    SDL_Rect card = { L1, CARD_Y0 + i * (CARD_H + CARD_GAP), W1, CARD_H };
                    if (inRect(mx, my, card)) { curP1 = i; focus = 0; }
                }

                if (!isSolo) {
                    /* Cartes J2 */
                    for (int i = 0; i < COSTUME_COUNT; i++) {
                        SDL_Rect card = { L2, CARD_Y0 + i * (CARD_H + CARD_GAP), W2, CARD_H };
                        if (inRect(mx, my, card)) { curP2 = i; focus = 1; }
                    }
                }

                /* Bouton Valider */
                SDL_Rect btnVal = { SCREEN_W/2 - 120, 500, 240, 48 };
                if (inRect(mx, my, btnVal)) {
                    result->costumeP1 = curP1;
                    result->costumeP2 = curP2;
                    return 1;
                }

                /* Bouton Retour */
                SDL_Rect btnRet = { SCREEN_W/2 - 120, 558, 240, 42 };
                if (inRect(mx, my, btnRet)) return -1;
            }
        }

        /* ════════════════════ Rendu ════════════════════ */
        SDL_RenderClear(renderer);
        if (bg) SDL_RenderCopy(renderer, bg, NULL, NULL);

        /* Overlay */
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 15, 150);
        SDL_Rect full = { 0, 0, SCREEN_W, SCREEN_H };
        SDL_RenderFillRect(renderer, &full);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        /* Titre */
        drawTextCentered(renderer, font, titleBuf, SCREEN_W / 2, 46, yellow);

        /* ── Colonne J1 ── */
        drawText(renderer, font, "JOUEUR 1", L1, 98, cyan);
        {
            const char *hint = isSolo
                ? "FLECHES H/B OU CLIC"
                : "FLECHES H/B OU CLIC";
            drawText(renderer, font, hint, L1, 116, dim);
        }

        for (int i = 0; i < COSTUME_COUNT; i++) {
            SDL_Rect card = { L1, CARD_Y0 + i * (CARD_H + CARD_GAP), W1, CARD_H };
            drawCostumeCard(renderer, font, card,
                            avatarsP1[i],
                            CHARACTERS[result->charP1].costumeNames[i],
                            i == curP1,
                            i == curP1);
        }

        /* Rappel touches J1 */
        {
            SDL_Rect box = { L1, CARD_Y0 + COSTUME_COUNT*(CARD_H+CARD_GAP)+4,
                             W1, 84 };
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 5, 5, 40, 180);
            SDL_RenderFillRect(renderer, &box);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawColor(renderer, 60, 60, 120, 255);
            SDL_RenderDrawRect(renderer, &box);

            char buf[100];
            SDL_Color oc = {255, 160, 0, 255};
            drawText(renderer, font, "TOUCHES J1:", L1+8, box.y+8, white);
            snprintf(buf, sizeof(buf), "G:%s  D:%s  SAUT:%s",
                SDL_GetKeyName(SDL_GetKeyFromScancode(result->inputP1.left)),
                SDL_GetKeyName(SDL_GetKeyFromScancode(result->inputP1.right)),
                SDL_GetKeyName(SDL_GetKeyFromScancode(result->inputP1.up)));
            drawText(renderer, font, buf, L1+8, box.y+28, oc);
            snprintf(buf, sizeof(buf), "ATT:%s  RUN:%s  VOL:%s",
                SDL_GetKeyName(SDL_GetKeyFromScancode(result->inputP1.punch)),
                SDL_GetKeyName(SDL_GetKeyFromScancode(result->inputP1.kick)),
                SDL_GetKeyName(SDL_GetKeyFromScancode(result->inputP1.fly)));
            drawText(renderer, font, buf, L1+8, box.y+48, oc);

            /* Bouton config touches J1 en bas de la boite */
            SDL_Rect cfgBtn = { L1+8, box.y+66, 160, 14 };
            SDL_Color linkCol = {100, 180, 255, 255};
            drawText(renderer, font, "[Changer les touches]", L1+8, box.y+66, linkCol);
            (void)cfgBtn;
        }

        /* ── Colonne J2 (mode multi uniquement) ── */
        if (!isSolo) {
            drawText(renderer, font, "JOUEUR 2", L2, 98, magenta);
            drawText(renderer, font, "W/S OU CLIC", L2, 116, dim);

            for (int i = 0; i < COSTUME_COUNT; i++) {
                SDL_Rect card = { L2, CARD_Y0 + i * (CARD_H + CARD_GAP), W2, CARD_H };
                drawCostumeCard(renderer, font, card,
                                avatarsP2[i],
                                CHARACTERS[result->charP2].costumeNames[i],
                                i == curP2,
                                i == curP2);
            }

            /* Rappel touches J2 */
            {
                SDL_Rect box = { L2, CARD_Y0 + COSTUME_COUNT*(CARD_H+CARD_GAP)+4,
                                 W2, 84 };
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 5, 5, 40, 180);
                SDL_RenderFillRect(renderer, &box);
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                SDL_SetRenderDrawColor(renderer, 80, 30, 80, 255);
                SDL_RenderDrawRect(renderer, &box);

                char buf[100];
                SDL_Color oc = {255, 160, 0, 255};
                drawText(renderer, font, "TOUCHES J2:", L2+8, box.y+8, white);
                snprintf(buf, sizeof(buf), "G:%s  D:%s  SAUT:%s",
                    SDL_GetKeyName(SDL_GetKeyFromScancode(result->inputP2.left)),
                    SDL_GetKeyName(SDL_GetKeyFromScancode(result->inputP2.right)),
                    SDL_GetKeyName(SDL_GetKeyFromScancode(result->inputP2.up)));
                drawText(renderer, font, buf, L2+8, box.y+28, oc);
                snprintf(buf, sizeof(buf), "ATT:%s  RUN:%s  VOL:%s",
                    SDL_GetKeyName(SDL_GetKeyFromScancode(result->inputP2.punch)),
                    SDL_GetKeyName(SDL_GetKeyFromScancode(result->inputP2.kick)),
                    SDL_GetKeyName(SDL_GetKeyFromScancode(result->inputP2.fly)));
                drawText(renderer, font, buf, L2+8, box.y+48, oc);

                SDL_Color linkCol = {100, 180, 255, 255};
                drawText(renderer, font, "[Changer les touches]", L2+8, box.y+66, linkCol);
            }
        }

        /* ── Boutons Valider / Retour ── */
        SDL_Rect btnVal = { SCREEN_W/2 - 120, 500, 240, 48 };
        SDL_Rect btnRet = { SCREEN_W/2 - 120, 558, 240, 42 };
        drawBtn(renderer, font, btnVal, "VALIDER", 0, border);
        drawBtn(renderer, font, btnRet, "RETOUR",  0, redBorder);

        /* Bas de l'ecran */
        drawTextCentered(renderer, font,
            "ENTREE=VALIDER  |  ECHAP=RETOUR",
            SCREEN_W / 2, SCREEN_H - 22, (SDL_Color){180,180,180,255});

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}

/* ══════════════════════════════════════════════════════════════════════════
   Point d'entree public
   ══════════════════════════════════════════════════════════════════════════ */
int runMenu(SDL_Renderer *renderer, TTF_Font *font, MenuResult *result)
{
    /* Valeurs par defaut */
    result->mode      = MODE_MULTI;
    result->charP1    = CHAR_BATMAN;
    result->costumeP1 = 0;
    result->inputP1   = DEFAULT_INPUT_P1;
    result->charP2    = CHAR_CATWOMAN;
    result->costumeP2 = 0;
    result->inputP2   = DEFAULT_INPUT_P2;

    /* Fond du menu */
    SDL_Texture *bg = IMG_LoadTexture(renderer, "back.png");
    if (!bg)
        bg = IMG_LoadTexture(renderer, "back.png"); /* fallback */

    /* Pre-charger les avatars (NULL si image absente = placeholder) */
    SDL_Texture *avatarsBatman[COSTUME_COUNT];
    SDL_Texture *avatarsCatwoman[COSTUME_COUNT];
    for (int i = 0; i < COSTUME_COUNT; i++) {
        avatarsBatman[i]   = loadTex(renderer, CHARACTERS[CHAR_BATMAN].costumeAvatars[i]);
        avatarsCatwoman[i] = loadTex(renderer, CHARACTERS[CHAR_CATWOMAN].costumeAvatars[i]);
    }

    int ret = 0;

    while (1) {
        /* Ecran 1 : choix mode */
        int mode = modeScreen(renderer, font, bg);
        if (mode == 0) { ret = 0; break; }

        result->mode = (mode == 1) ? MODE_SOLO : MODE_MULTI;

        /* En solo J1=Batman, J2=Batman (IA) */
        /* En multi J1=Batman, J2=Catwoman */
        if (result->mode == MODE_SOLO) {
            result->charP1 = CHAR_BATMAN;
            result->charP2 = CHAR_BATMAN;
        } else {
            result->charP1 = CHAR_BATMAN;
            result->charP2 = CHAR_CATWOMAN;
        }

        /* Ecran 2 : selection costume */
        SDL_Texture **avP1 = (result->charP1 == CHAR_BATMAN)
                             ? avatarsBatman : avatarsCatwoman;
        SDL_Texture **avP2 = (result->charP2 == CHAR_BATMAN)
                             ? avatarsBatman : avatarsCatwoman;

        int sel = charSelectScreen(renderer, font, bg, result, avP1, avP2);
        if (sel == 0)  { ret = 0; break; }
        if (sel == 1)  { ret = 1; break; }
        /* sel == -1  => retour au choix de mode, on reboucle */
    }

    /* Liberer les avatars */
    for (int i = 0; i < COSTUME_COUNT; i++) {
        if (avatarsBatman[i])   SDL_DestroyTexture(avatarsBatman[i]);
        if (avatarsCatwoman[i]) SDL_DestroyTexture(avatarsCatwoman[i]);
    }
    if (bg) SDL_DestroyTexture(bg);

    return ret;
}
