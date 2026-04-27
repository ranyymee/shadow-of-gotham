#include "menu.h"
#include <string.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>

#define SCREEN_W 900
#define SCREEN_H 620

const CharacterDef CHARACTERS[CHAR_COUNT] = {
    {
        "Batman",
        { "batman_right_1.png", "batman_right_2.png", "batman_right_3.png" },
        { "batman_left_1.png",  "batman_left_2.png",  "batman_left_3.png"  },
        { "Classic Crusader", "Shadow Stalker", "Cobalt Guardian" },
        { "assets/batman_avatar_1.png",
          "assets/batman_avatar_2.png",
          "assets/batman_avatar_3.png" }
    },
    {
        "Catwoman",
        { "catwoman_right_1.png", "catwoman_right_2.png", "catwoman_right_3.png" },
        { "catwoman_left_1.png",  "catwoman_left_2.png",  "catwoman_left_3.png"  },
        { "Midnight Stalker", "Amethyst Prowler", "Crimson Shadow" },
        { "assets/catwoman_avatar_1.png",
          "assets/catwoman_avatar_2.png",
          "assets/catwoman_avatar_3.png" }
    }
};

SDL_Texture *loadTex(SDL_Renderer *r, const char *path)
{
    return IMG_LoadTexture(r, path);
}

void drawText(SDL_Renderer *r, TTF_Font *f,
              const char *txt, int x, int y, SDL_Color col)
{
    SDL_Surface *s;
    SDL_Texture *t;
    SDL_Rect dst;
    if (!f || !txt) return;
    s = TTF_RenderUTF8_Blended(f, txt, col);
    if (!s) return;
    t = SDL_CreateTextureFromSurface(r, s);
    dst.x = x; dst.y = y; dst.w = s->w; dst.h = s->h;
    SDL_FreeSurface(s);
    SDL_RenderCopy(r, t, NULL, &dst);
    SDL_DestroyTexture(t);
}

void drawTextCentered(SDL_Renderer *r, TTF_Font *f,
                      const char *txt, int cx, int y, SDL_Color col)
{
    SDL_Surface *s;
    SDL_Texture *t;
    SDL_Rect dst;
    if (!f || !txt) return;
    s = TTF_RenderUTF8_Blended(f, txt, col);
    if (!s) return;
    t = SDL_CreateTextureFromSurface(r, s);
    dst.x = cx - s->w / 2; dst.y = y; dst.w = s->w; dst.h = s->h;
    SDL_FreeSurface(s);
    SDL_RenderCopy(r, t, NULL, &dst);
    SDL_DestroyTexture(t);
}

void drawBtn(SDL_Renderer *r, TTF_Font *f,
             SDL_Rect rect, const char *label,
             int hovered, SDL_Color borderCol)
{
    SDL_Color white = {255, 255, 255, 255};
    if (hovered)
        SDL_SetRenderDrawColor(r, 30, 30, 80, 230);
    else
        SDL_SetRenderDrawColor(r, 15, 15, 50, 210);
    SDL_RenderFillRect(r, &rect);
    SDL_SetRenderDrawColor(r, borderCol.r, borderCol.g, borderCol.b, 255);
    SDL_RenderDrawRect(r, &rect);
    drawTextCentered(r, f, label, rect.x + rect.w / 2,
                     rect.y + rect.h / 2 - 10, white);
}

int inRect(int mx, int my, SDL_Rect rect)
{
    return mx >= rect.x && mx <= rect.x + rect.w &&
           my >= rect.y && my <= rect.y + rect.h;
}

void drawOverlay(SDL_Renderer *r, SDL_Rect rect)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 20, 180);
    SDL_RenderFillRect(r, &rect);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

int remapMenu(SDL_Renderer *renderer, TTF_Font *font,
              SDL_Texture *bg, InputConfig *cfg, const char *title)
{
    const char  *actionNames[7];
    SDL_Scancode *ptrs[7];
    int N = 7;
    int waiting = -1;
    int i;
    SDL_Color white  = {255, 255, 255, 255};
    SDL_Color yellow = {255, 220,   0, 255};
    SDL_Color orange = {255, 160,   0, 255};
    SDL_Color border = {180, 180, 180, 255};
    SDL_Color cyan   = {180, 220, 255, 255};

    actionNames[0] = "Gauche";
    actionNames[1] = "Droite";
    actionNames[2] = "Sauter";
    actionNames[3] = "Double Saut";
    actionNames[4] = "Attaque";
    actionNames[5] = "Kick";
    actionNames[6] = "Baisser";

    ptrs[0] = &cfg->left;
    ptrs[1] = &cfg->right;
    ptrs[2] = &cfg->up;
    ptrs[3] = &cfg->fly;
    ptrs[4] = &cfg->punch;
    ptrs[5] = &cfg->kick;
    ptrs[6] = &cfg->crouch;

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
                SDL_Rect ok = { 330, 100 + N * 52 + 16, 180, 42 };
                for (i = 0; i < N; i++) {
                    SDL_Rect btn = { 300, 100 + i * 52, 250, 40 };
                    if (inRect(mx, my, btn)) { waiting = i; break; }
                }
                if (inRect(mx, my, ok)) return 1;
            }
        }

        SDL_RenderClear(renderer);
        if (bg) SDL_RenderCopy(renderer, bg, NULL, NULL);
        {
            SDL_Rect overlay = { 0, 0, SCREEN_W, SCREEN_H };
            drawOverlay(renderer, overlay);
        }

        drawTextCentered(renderer, font, title, SCREEN_W / 2, 50, yellow);
        drawTextCentered(renderer, font,
            "Clique sur une action pour changer la touche",
            SCREEN_W / 2, 78, orange);
        drawTextCentered(renderer, font,
            "SHIFT + direction = Courir  |  ESPACE x2 = Double Saut",
            SCREEN_W / 2, 96, cyan);

        for (i = 0; i < N; i++) {
            SDL_Rect btn = { 300, 100 + i * 52, 250, 40 };
            SDL_Color bc;
            const char *label;
            drawText(renderer, font, actionNames[i], 110, 112 + i * 52, white);
            if (waiting == i) {
                label = ">>> appuie <<<";
                bc.r = 255; bc.g = 200; bc.b = 0; bc.a = 255;
            } else {
                label = SDL_GetKeyName(SDL_GetKeyFromScancode(*ptrs[i]));
                bc = border;
            }
            drawBtn(renderer, font, btn, label, waiting == i, bc);
        }

        {
            SDL_Rect ok = { 330, 100 + N * 52 + 16, 180, 42 };
            SDL_Color redBorder = {255, 80, 80, 255};
            drawBtn(renderer, font, ok, "OK", 0, redBorder);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}

void drawCostumeCard(SDL_Renderer *r, TTF_Font *f,
                     SDL_Rect rect,
                     SDL_Texture *avatar,
                     const char *costumeName,
                     int selected, int isCurrent)
{
    int avatarW;
    SDL_Rect avatarRect;
    SDL_Color nameCol;
    int textX, textY;
    char label[64];

    SDL_SetRenderDrawColor(r, 10, 10, 60, 200);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(r, &rect);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

    if (selected)
        SDL_SetRenderDrawColor(r, 200, 160, 0, 255);
    else
        SDL_SetRenderDrawColor(r, 60, 80, 160, 255);
    SDL_RenderDrawRect(r, &rect);

    avatarW = rect.h - 8;
    avatarRect.x = rect.x + 4;
    avatarRect.y = rect.y + 4;
    avatarRect.w = avatarW;
    avatarRect.h = avatarW;

    if (avatar) {
        SDL_RenderCopy(r, avatar, NULL, &avatarRect);
    } else {
        SDL_SetRenderDrawColor(r, 40, 40, 100, 255);
        SDL_RenderFillRect(r, &avatarRect);
        SDL_SetRenderDrawColor(r, 80, 80, 150, 255);
        SDL_RenderDrawRect(r, &avatarRect);
    }

    if (selected) {
        nameCol.r = 255; nameCol.g = 220; nameCol.b = 0; nameCol.a = 255;
    } else {
        nameCol.r = 220; nameCol.g = 220; nameCol.b = 255; nameCol.a = 255;
    }
    textX = rect.x + avatarW + 12;
    textY = rect.y + rect.h / 2 - 10;

    if (isCurrent)
        snprintf(label, sizeof(label), "~ %s", costumeName);
    else
        snprintf(label, sizeof(label), "%s", costumeName);

    drawText(r, f, label, textX, textY, nameCol);
}

int charSelectScreen(SDL_Renderer *renderer, TTF_Font *font,
                     SDL_Texture *bg, MenuResult *result,
                     SDL_Texture *avatarsP1[COSTUME_COUNT],
                     SDL_Texture *avatarsP2[COSTUME_COUNT])
{
    int isSolo = (result->mode == MODE_SOLO);
    int i;

    SDL_Color yellow    = {255, 220,   0, 255};
    SDL_Color cyan      = {  0, 210, 210, 255};
    SDL_Color magenta   = {210,  30, 210, 255};
    SDL_Color border    = {160, 160, 160, 255};
    SDL_Color redBorder = {255,  80,  80, 255};

    int CARD_H   = 95;
    int CARD_GAP = 8;
    int CARD_Y0  = 125;

    int L1 = 15,  W1 = 420;
    int L2 = 455, W2 = 420;

    int curP1 = result->costumeP1;
    int curP2 = result->costumeP2;
    int focus  = 0;

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
                if (sc == SDL_SCANCODE_UP) {
                    if (!isSolo && focus == 1)
                        curP2 = (curP2 - 1 + COSTUME_COUNT) % COSTUME_COUNT;
                    else
                        curP1 = (curP1 - 1 + COSTUME_COUNT) % COSTUME_COUNT;
                }
                if (sc == SDL_SCANCODE_DOWN) {
                    if (!isSolo && focus == 1)
                        curP2 = (curP2 + 1) % COSTUME_COUNT;
                    else
                        curP1 = (curP1 + 1) % COSTUME_COUNT;
                }
                if (!isSolo) {
                    if (sc == SDL_SCANCODE_W)
                        curP2 = (curP2 - 1 + COSTUME_COUNT) % COSTUME_COUNT;
                    if (sc == SDL_SCANCODE_S)
                        curP2 = (curP2 + 1) % COSTUME_COUNT;
                }
                if (sc == SDL_SCANCODE_TAB && !isSolo)
                    focus = 1 - focus;
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x, my = e.button.y;
                SDL_Rect btnVal = { SCREEN_W/2 - 120, 460, 240, 44 };
                SDL_Rect btnRet = { SCREEN_W/2 - 120, 512, 240, 40 };

                for (i = 0; i < COSTUME_COUNT; i++) {
                    SDL_Rect card = { L1, CARD_Y0 + i * (CARD_H + CARD_GAP), W1, CARD_H };
                    if (inRect(mx, my, card)) { curP1 = i; focus = 0; }
                }
                if (!isSolo) {
                    for (i = 0; i < COSTUME_COUNT; i++) {
                        SDL_Rect card = { L2, CARD_Y0 + i * (CARD_H + CARD_GAP), W2, CARD_H };
                        if (inRect(mx, my, card)) { curP2 = i; focus = 1; }
                    }
                }
                if (inRect(mx, my, btnVal)) {
                    result->costumeP1 = curP1;
                    result->costumeP2 = curP2;
                    return 1;
                }
                if (inRect(mx, my, btnRet)) return -1;
            }
        }

        SDL_RenderClear(renderer);
        if (bg) SDL_RenderCopy(renderer, bg, NULL, NULL);

        {
            SDL_Rect full = { 0, 0, SCREEN_W, SCREEN_H };
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 15, 150);
            SDL_RenderFillRect(renderer, &full);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }

        drawTextCentered(renderer, font, "COSTUME SELECTION", SCREEN_W / 2, 46, yellow);
        drawTextCentered(renderer, font, "BATMAN", L1 + W1/2, 95, cyan);

        for (i = 0; i < COSTUME_COUNT; i++) {
            SDL_Rect card = { L1, CARD_Y0 + i * (CARD_H + CARD_GAP), W1, CARD_H };
            drawCostumeCard(renderer, font, card,
                            avatarsP1[i],
                            CHARACTERS[result->charP1].costumeNames[i],
                            i == curP1,
                            i == curP1);
        }

        if (!isSolo) {
            drawTextCentered(renderer, font, "CATWOMAN", L2 + W2/2, 95, magenta);
            for (i = 0; i < COSTUME_COUNT; i++) {
                SDL_Rect card = { L2, CARD_Y0 + i * (CARD_H + CARD_GAP), W2, CARD_H };
                drawCostumeCard(renderer, font, card,
                                avatarsP2[i],
                                CHARACTERS[result->charP2].costumeNames[i],
                                i == curP2,
                                i == curP2);
            }
        }

        {
            SDL_Rect btnVal = { SCREEN_W/2 - 120, 460, 240, 44 };
            SDL_Rect btnRet = { SCREEN_W/2 - 120, 512, 240, 40 };
            drawBtn(renderer, font, btnVal, "CONFIRM", 0, border);
            drawBtn(renderer, font, btnRet, "BACK",    0, redBorder);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}

int runMenu(SDL_Renderer *renderer, TTF_Font *font, MenuResult *result)
{
    SDL_Texture *bg;
    SDL_Texture *avatarsBatman[COSTUME_COUNT];
    SDL_Texture *avatarsCatwoman[COSTUME_COUNT];
    SDL_Texture **avP1;
    SDL_Texture **avP2;
    int i, sel, ret;

    result->mode      = MODE_MULTI;
    result->charP1    = CHAR_BATMAN;
    result->costumeP1 = 0;
    result->inputP1   = DEFAULT_INPUT_P1;
    result->charP2    = CHAR_CATWOMAN;
    result->costumeP2 = 0;
    result->inputP2   = DEFAULT_INPUT_P2;

    bg = IMG_LoadTexture(renderer, "back.png");

    for (i = 0; i < COSTUME_COUNT; i++) {
        avatarsBatman[i]   = loadTex(renderer, CHARACTERS[CHAR_BATMAN].costumeAvatars[i]);
        avatarsCatwoman[i] = loadTex(renderer, CHARACTERS[CHAR_CATWOMAN].costumeAvatars[i]);
    }

    ret = 0;
    result->mode   = MODE_MULTI;
    result->charP1 = CHAR_BATMAN;
    result->charP2 = CHAR_CATWOMAN;

    avP1 = avatarsBatman;
    avP2 = avatarsCatwoman;

    sel = charSelectScreen(renderer, font, bg, result, avP1, avP2);
    if (sel == 1) ret = 1;
    else          ret = 0;

    for (i = 0; i < COSTUME_COUNT; i++) {
        if (avatarsBatman[i])   SDL_DestroyTexture(avatarsBatman[i]);
        if (avatarsCatwoman[i]) SDL_DestroyTexture(avatarsCatwoman[i]);
    }
    if (bg) SDL_DestroyTexture(bg);

    return ret;
}
