#include "enigme.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* Runtime screen size — set by main() after window creation */
int SCR_W = 1280;
int SCR_H = 720;

/* ============================================================
   playSound
   ============================================================ */
static void playSound(Enigme *e, SoundID id)
{
    if (!e) return;
    if (e->currentSound == id && Mix_PlayingMusic()) return;
    Mix_HaltMusic();
    e->currentSound = id;
    switch (id) {
        case SND_BAT:
            if (e->soundBat) { Mix_VolumeMusic(70); Mix_PlayMusic(e->soundBat, -1); }
            break;
        case SND_SUSPENSE:
            if (e->soundSuspense) { Mix_VolumeMusic(MIX_MAX_VOLUME); Mix_PlayMusic(e->soundSuspense, 0); }
            break;
        case SND_CORRECT:
            if (e->soundCorrect) { Mix_VolumeMusic(MIX_MAX_VOLUME); Mix_PlayMusic(e->soundCorrect, 0); }
            break;
        case SND_WRONG:
            if (e->soundWrong) { Mix_VolumeMusic(MIX_MAX_VOLUME); Mix_PlayMusic(e->soundWrong, 0); }
            break;
        default: break;
    }
}

/* ============================================================
   Helpers
   ============================================================ */
static SDL_Texture *loadTexture(const char *path, SDL_Renderer *renderer)
{
    SDL_Surface *s = IMG_Load(path);
    if (!s) { fprintf(stderr, "[WARN] %s : %s\n", path, IMG_GetError()); return NULL; }
    SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
    SDL_FreeSurface(s);
    return t;
}

static void renderTextCentered(SDL_Renderer *renderer, TTF_Font *font,
                                const char *text, SDL_Color color, SDL_Rect *rect)
{
    if (!font || !text || !rect) return;
    int pad  = 14;
    int wrap = rect->w - 2 * pad;
    if (wrap < 10) wrap = 10;
    SDL_Surface *s = TTF_RenderUTF8_Blended_Wrapped(font, text, color, wrap);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
    if (!t) { SDL_FreeSurface(s); return; }
    int x = rect->x + (rect->w - s->w) / 2;
    int y = rect->y + (rect->h - s->h) / 2;
    if (x < rect->x + pad) x = rect->x + pad;
    if (y < rect->y + pad) y = rect->y + pad;
    SDL_RenderSetClipRect(renderer, rect);
    SDL_Rect dst = {x, y, s->w, s->h};
    SDL_RenderCopy(renderer, t, NULL, &dst);
    SDL_RenderSetClipRect(renderer, NULL);
    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}

static void renderTextureRotated(SDL_Renderer *renderer, SDL_Texture *tex,
                                  SDL_Rect *rect, double angle)
{
    if (!tex || !rect) return;
    SDL_RenderCopyEx(renderer, tex, NULL, rect, angle, NULL, SDL_FLIP_NONE);
}

/* ============================================================
   Load + shuffle questions
   ============================================================ */
static int loadQuestions(Enigme *e, const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) { fprintf(stderr, "[WARN] %s not found\n", filename); return 0; }
    char line[512];
    int  count = 0;
    while (fgets(line, sizeof(line), f) && count < MAX_QUESTIONS) {
        int len = (int)strlen(line);
        while (len > 0 && (line[len-1]=='\n'||line[len-1]=='\r')) line[--len]='\0';
        if (len == 0) continue;
        char buf[512];
        strncpy(buf, line, sizeof(buf)-1);
        buf[sizeof(buf)-1] = '\0';
        char *tok[5];
        tok[0]=strtok(buf,"|"); tok[1]=strtok(NULL,"|");
        tok[2]=strtok(NULL,"|"); tok[3]=strtok(NULL,"|");
        tok[4]=strtok(NULL,"|");
        if (!tok[0]||!tok[1]||!tok[2]||!tok[3]||!tok[4]) continue;
        strncpy(e->questions[count].question, tok[0], MAX_QUESTION_LEN-1);
        strncpy(e->questions[count].prop[0],  tok[1], MAX_PROP_LEN-1);
        strncpy(e->questions[count].prop[1],  tok[2], MAX_PROP_LEN-1);
        strncpy(e->questions[count].prop[2],  tok[3], MAX_PROP_LEN-1);
        e->questions[count].bonneReponse = atoi(tok[4]);
        e->questions[count].deja_vu      = 0;
        count++;
    }
    fclose(f);
    return count;
}

static void shuffleQuestions(Enigme *e)
{
    for (int i = 0; i < e->nbQuestionsPool; i++) e->questions[i].deja_vu = 0;
    int pool[MAX_QUESTIONS], nb = 0;
    for (int i = 0; i < e->nbQuestionsPool; i++) pool[nb++] = i;
    for (int i = nb-1; i > 0; i--) {
        int j = rand()%(i+1), tmp = pool[i];
        pool[i] = pool[j]; pool[j] = tmp;
    }
    int n = (nb < NB_QUESTIONS) ? nb : NB_QUESTIONS;
    for (int i = 0; i < n; i++) {
        e->ordreJeu[i] = pool[i];
        e->questions[pool[i]].deja_vu = 1;
    }
}

/* ============================================================
   setupLayout — 100% based on SCR_W / SCR_H
   ─────────────────────────────────────────────────────────
   HUD band  : top 8% of screen height  (min 60px, max 90px)
   Safe margin: 2% of width on left/right

   Content area (below HUD):
     Left 30%  → question card (centered vertically)
     Right 70% → 3 answer cards in 2-col grid
                  top-left, top-right, bottom-center
   ============================================================ */
static void setupLayout(Enigme *e)
{
    int W = SCR_W;
    int H = SCR_H;

    /* ── HUD height: 8% of screen, clamped 60-90px ────── */
    int hudH = H * 8 / 100;
    if (hudH < 60)  hudH = 60;
    if (hudH > 90)  hudH = 90;
    e->hudH = hudH;

    /* ── Safe horizontal margin: 2% each side ──────────── */
    int marginX = W * 2 / 100;
    if (marginX < 20) marginX = 20;

    /* ── Content area ────────────────────────────────────── */
    int areaX = marginX;
    int areaY = hudH + 8;
    int areaW = W - 2 * marginX;
    int areaH = H - areaY - 10;

    /* ── Question card: left 28% of area ─────────────────── */
    int cardW = areaW * 28 / 100;
    int cardH = cardW * 3 / 2;          /* aspect ratio 2:3 */
    if (cardH > areaH * 90 / 100)       /* never taller than 90% of area */
        cardH = areaH * 90 / 100;
    int cardX = areaX;
    int cardY = areaY + (areaH - cardH) / 2;

    e->cardRect  = (SDL_Rect){ cardX, cardY, cardW, cardH };
    e->cardAngle = -6.0;

    /* ── Right zone for the 3 answer cards ───────────────── */
    int gap    = areaW * 2 / 100;       /* 2% gap between question and answers */
    int rightX = areaX + cardW + gap;
    int rightW = areaX + areaW - rightX;

    /* prop card size: 2-column, each ~32% of rightW (narrower) */
    int propW = rightW * 32 / 100;
    int propH = propW * 17 / 10;        /* aspect ratio ~1:1.7  (taller) */
    if (propH > areaH * 48 / 100)
        propH = areaH * 48 / 100;

    /* gap between the two columns */
    int colGap = rightW - 2 * propW;
    if (colGap < 10) colGap = 10;

    int colL   = rightX;
    int colR   = rightX + propW + colGap;
    int rowTop = areaY + 8;
    int rowBot = areaY + areaH - propH - 8;

    /* prop[0]: top-left */
    e->propRect[0]  = (SDL_Rect){ colL, rowTop, propW, propH };
    e->propAngle[0] = -5.0;

    /* prop[1]: top-right */
    e->propRect[1]  = (SDL_Rect){ colR, rowTop, propW, propH };
    e->propAngle[1] = +5.0;

    /* prop[2]: bottom-center */
    int colC = rightX + (rightW - propW) / 2;
    e->propRect[2]  = (SDL_Rect){ colC, rowBot, propW, propH };
    e->propAngle[2] = -3.0;

    /* ── Chrono bar & bat logo (right 32% of HUD) ─────────── */
    int hudRightStart = W * 68 / 100;
    int hudRightW     = W - marginX - hudRightStart;

    /* bat logo: right-most, square-ish */
    int logoSz = hudH * 85 / 100;
    int logoX  = W - marginX - logoSz;
    int logoY  = (hudH - logoSz) / 2;
    e->batLogoRect = (SDL_Rect){ logoX, logoY, logoSz, logoSz };

    /* chrono bar: left of logo, vertically centered */
    int barH = hudH * 25 / 100;
    if (barH < 10) barH = 10;
    if (barH > 22) barH = 22;
    int barW = hudRightW - logoSz - 8;
    if (barW < 40) barW = 40;
    int barX = hudRightStart;
    int barY = (hudH - barH) / 2;
    e->chronoRect = (SDL_Rect){ barX, barY, barW, barH };

    /* chrono design image: smaller, vertically centered in HUD */
    int designH = hudH * 55 / 100;
    int designY = (hudH - designH) / 2;
    e->chronoDesignRect = (SDL_Rect){ barX, designY, barW + logoSz + 8, designH };
}

/* ============================================================
   initEnigme
   ============================================================ */
void initEnigme(Enigme *e, SDL_Renderer *renderer)
{
    if (!e || !renderer) return;
    srand((unsigned)time(NULL));
    memset(e, 0, sizeof(Enigme));

    e->nbQuestionsPool = loadQuestions(e, "questions.txt");
    if (e->nbQuestionsPool == 0)
        fprintf(stderr, "[ERROR] No questions loaded!\n");
    shuffleQuestions(e);

    e->questionIndex   = 0;
    e->score           = 0;
    e->niveau          = 1;
    e->selected        = -1;
    e->hovered         = -1;
    e->showResult      = 0;
    e->correct         = 0;
    e->resultTime      = 0;
    e->waitingSuspense = 0;
    e->suspenseTime    = 0;
    e->chronoSecondes  = CHRONO_SECS;
    e->chronoLastTick  = SDL_GetTicks();
    e->chronoExpire    = 0;
    e->chronoRatio     = 1.0f;
    e->currentSound    = SND_NONE;

    e->bgTexture       = loadTexture("background.png",   renderer);
    e->cardTexture     = loadTexture("carte.png",         renderer);
    e->propTexture[0]  = loadTexture("carte.png",         renderer);
    e->propTexture[1]  = loadTexture("carte.png",         renderer);
    e->propTexture[2]  = loadTexture("carte.png",         renderer);
    e->batLogoTex      = loadTexture("BatLogo.png",       renderer);

    e->soundBat      = Mix_LoadMUS("bat.mp3");
    e->soundSuspense = Mix_LoadMUS("suspince.mp3");
    e->soundCorrect  = Mix_LoadMUS("correct.mp3");
    e->soundWrong    = Mix_LoadMUS("ghalet.mp3");

    if (!e->soundBat)        fprintf(stderr, "[WARN] bat.mp3 missing\n");
    if (!e->soundSuspense)   fprintf(stderr, "[WARN] suspince.mp3 missing\n");
    if (!e->soundCorrect)    fprintf(stderr, "[WARN] correct.mp3 missing\n");
    if (!e->soundWrong)      fprintf(stderr, "[WARN] ghalet.mp3 missing\n");

    playSound(e, SND_BAT);
    setupLayout(e);
}

/* ============================================================
   updateEnigme
   ============================================================ */
void updateEnigme(Enigme *e)
{
    if (!e || e->questionIndex >= NB_QUESTIONS) return;
    Uint32 now = SDL_GetTicks();

    if (e->currentSound == SND_BAT && !Mix_PlayingMusic())
        playSound(e, SND_BAT);

    if (!e->showResult && !e->waitingSuspense && !e->chronoExpire) {
        if (now - e->chronoLastTick >= 1000) {
            e->chronoLastTick = now;
            e->chronoSecondes--;
            if (e->chronoSecondes <= 0) {
                e->chronoSecondes  = 0;
                e->chronoExpire    = 1;
                e->selected        = -1;
                e->correct         = 0;
                e->waitingSuspense = 1;
                e->suspenseTime    = now;
                playSound(e, SND_SUSPENSE);
            }
        }
    }

    if (e->waitingSuspense && e->suspenseTime > 0) {
        if (now - e->suspenseTime >= SUSPENSE_DELAY) {
            e->waitingSuspense = 0;
            e->showResult      = 1;
            e->resultTime      = now;
            if (e->correct) e->score++;   /* +1 after suspense sound */
            playSound(e, e->correct ? SND_CORRECT : SND_WRONG);
        }
    }

    if (e->showResult && e->resultTime > 0) {
        if (now - e->resultTime >= RESULT_DELAY) {
            e->questionIndex++;
            e->selected        = -1;
            e->hovered         = -1;
            e->showResult      = 0;
            e->correct         = 0;
            e->resultTime      = 0;
            e->waitingSuspense = 0;
            e->suspenseTime    = 0;
            e->chronoExpire    = 0;

            if (e->questionIndex > 0 &&
                e->questionIndex % QUESTIONS_PAR_NIVEAU == 0)
                e->niveau++;

            int bonus = (e->niveau - 1) * 2;
            e->chronoSecondes = CHRONO_SECS - bonus;
            if (e->chronoSecondes < 5) e->chronoSecondes = 5;
            e->chronoLastTick = SDL_GetTicks();
            e->chronoRatio    = 1.0f;
            playSound(e, SND_BAT);
        }
    }

    /* Always keep chronoRatio in sync */
    e->chronoRatio = (float)e->chronoSecondes / (float)CHRONO_SECS;
    if (e->chronoRatio < 0.0f) e->chronoRatio = 0.0f;
    if (e->chronoRatio > 1.0f) e->chronoRatio = 1.0f;
}

/* ============================================================
   drawChronoBar — fill rect + design image overlay
   ============================================================ */
static void drawChronoBar(SDL_Renderer *r, SDL_Texture *designTex,
                           SDL_Rect *barRect, SDL_Rect *designRect, float ratio)
{
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;
    (void)designTex; (void)designRect;

    Uint32 now = SDL_GetTicks();
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);

    /* Background */
    SDL_SetRenderDrawColor(r, 0, 18, 38, 205);
    SDL_RenderFillRect(r, barRect);
    /* border */
    SDL_SetRenderDrawColor(r, 0, 220, 255, 85);
    SDL_RenderDrawRect(r, barRect);

    /* Colored fill — LEFT-anchored, shrinks rightward like puzzle */
    int fillW = (int)(barRect->w * ratio);
    if (fillW > 0) {
        Uint8 tR, tG;
        if      (ratio > 0.5f)  { tR=0;   tG=210; }
        else if (ratio > 0.25f) { tR=255; tG=200; }
        else                    { tR=220; tG=30;  }
        SDL_Rect fill = { barRect->x, barRect->y, fillW, barRect->h };
        SDL_SetRenderDrawColor(r, tR, tG, 40, 230);
        SDL_RenderFillRect(r, &fill);
        /* white glow on right edge of fill */
        SDL_SetRenderDrawColor(r, 255, 255, 255, 160);
        SDL_Rect glow = { barRect->x + fillW - 3, barRect->y, 3, barRect->h };
        SDL_RenderFillRect(r, &glow);
    }

    /* Red flash pulse when low */
    if (ratio < 0.25f) {
        float p = 0.5f + 0.5f * sinf((float)now / 180.0f);
        SDL_SetRenderDrawColor(r, 255, 60, 0, (Uint8)(p * 70));
        SDL_RenderFillRect(r, barRect);
        SDL_SetRenderDrawColor(r, 255, 80, 0, (Uint8)(p * 180));
        SDL_RenderDrawLine(r, barRect->x + fillW, barRect->y,
                              barRect->x + fillW, barRect->y + barRect->h);
    }

    /* Corner accents like puzzle */
    int cl = 9;
    SDL_SetRenderDrawColor(r, 0, 220, 255, 140);
    SDL_RenderDrawLine(r, barRect->x, barRect->y, barRect->x+cl, barRect->y);
    SDL_RenderDrawLine(r, barRect->x, barRect->y, barRect->x, barRect->y+cl);
    SDL_RenderDrawLine(r, barRect->x+barRect->w, barRect->y, barRect->x+barRect->w-cl, barRect->y);
    SDL_RenderDrawLine(r, barRect->x+barRect->w, barRect->y, barRect->x+barRect->w, barRect->y+cl);
    SDL_RenderDrawLine(r, barRect->x, barRect->y+barRect->h, barRect->x+cl, barRect->y+barRect->h);
    SDL_RenderDrawLine(r, barRect->x+barRect->w, barRect->y+barRect->h, barRect->x+barRect->w-cl, barRect->y+barRect->h);

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

/* ============================================================
   renderEnigme
   ============================================================ */
void renderEnigme(Enigme *e, SDL_Renderer *renderer,
                  TTF_Font *font, TTF_Font *fontSmall, TTF_Font *fontTiny)
{
    if (!e || !renderer || !font) return;
    if (!fontSmall) fontSmall = font;
    if (!fontTiny)  fontTiny  = fontSmall;

    int W = SCR_W;
    int H = SCR_H;

    /* Background */
    if (e->bgTexture) SDL_RenderCopy(renderer, e->bgTexture, NULL, NULL);

    if (e->questionIndex >= NB_QUESTIONS) {
        SDL_Color white = {255,255,255,255};
        SDL_Rect  r = {0,0,W,H};
        renderTextCentered(renderer, fontSmall, "Well done! Quiz complete!", white, &r);
        return;
    }

    /* =========================================================
       HUD band
       ========================================================= */
    int hudH    = e->hudH;
    int marginX = W * 2 / 100;
    if (marginX < 20) marginX = 20;

    {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 10, 180);
        SDL_Rect hud = {0, 0, W, hudH};
        SDL_RenderFillRect(renderer, &hud);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        SDL_Color cyan   = {0,   220, 255, 255};
        SDL_Color yellow = {255, 220,   0, 255};
        char buf[64];

        /* SCORE — left-aligned, vertically centered in HUD */
        snprintf(buf, sizeof(buf), "SCORE: %d", e->score);
        SDL_Surface *s = TTF_RenderUTF8_Blended(font, buf, cyan);
        if (s) {
            SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_Rect d = { marginX, (hudH - s->h) / 2, s->w, s->h };
            if (t) { SDL_RenderCopy(renderer, t, NULL, &d); SDL_DestroyTexture(t); }
            SDL_FreeSurface(s);
        }

        /* LEVEL — style SELECT A PUZZLE: zoom pulse + arrows + glow */
        {
            Uint32 now2 = SDL_GetTicks();
            float zoom2 = 1.0f + 0.08f * sinf((float)now2 / 700.0f);
            float gp2   = 0.5f + 0.5f * sinf((float)now2 / 700.0f);
            float lp2   = 0.5f + 0.5f * sinf((float)now2 / 500.0f);
            snprintf(buf, sizeof(buf), "LEVEL %d", e->niveau);
            SDL_Surface *sl2 = TTF_RenderUTF8_Blended(font, buf, cyan);
            if (sl2) {
                SDL_Texture *tl2 = SDL_CreateTextureFromSurface(renderer, sl2);
                int tw2 = (int)(sl2->w * zoom2);
                int th2 = (int)(sl2->h * zoom2);
                int ty2 = (hudH - th2) / 2 - 4;
                if (ty2 < 2) ty2 = 2;
                SDL_Rect d2 = { W/2 - tw2/2, ty2, tw2, th2 };
                SDL_Rect glowR2 = { d2.x - 20, d2.y - 4, tw2 + 40, th2 + 8 };
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 0, 220, 255, (Uint8)(gp2 * 18));
                SDL_RenderFillRect(renderer, &glowR2);
                if (tl2) {
                    SDL_SetTextureAlphaMod(tl2, (Uint8)(200 + gp2 * 55));
                    SDL_RenderCopy(renderer, tl2, NULL, &d2);
                    SDL_DestroyTexture(tl2);
                }
                SDL_FreeSurface(sl2);
                int ly2 = d2.y + th2 / 2;
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 0, 220, 255, (Uint8)(lp2 * 80));
                SDL_RenderDrawLine(renderer, W/2 - tw2/2 - 40, ly2, W/2 - tw2/2 - 10, ly2);
                SDL_RenderDrawLine(renderer, W/2 + tw2/2 + 10, ly2, W/2 + tw2/2 + 40, ly2);
                SDL_SetRenderDrawColor(renderer, 0, 220, 255, (Uint8)(lp2 * 160));
                SDL_RenderDrawLine(renderer, W/2-tw2/2-18, ly2-5, W/2-tw2/2-10, ly2);
                SDL_RenderDrawLine(renderer, W/2-tw2/2-10, ly2,   W/2-tw2/2-18, ly2+5);
                SDL_RenderDrawLine(renderer, W/2+tw2/2+18, ly2-5, W/2+tw2/2+10, ly2);
                SDL_RenderDrawLine(renderer, W/2+tw2/2+10, ly2,   W/2+tw2/2+18, ly2+5);
            }
        }

        /* Question counter — below LEVEL */
        snprintf(buf, sizeof(buf), "%02d/%d", e->questionIndex+1, NB_QUESTIONS);
        s = TTF_RenderUTF8_Blended(fontSmall, buf, yellow);
        if (s) {
            SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_Rect d = { W/2 - s->w/2, hudH - s->h - 4, s->w, s->h };
            if (t) { SDL_RenderCopy(renderer, t, NULL, &d); SDL_DestroyTexture(t); }
            SDL_FreeSurface(s);
        }

        /* Bat logo — shrinks + pulses as time runs out */
        if (e->batLogoTex) {
            float ratio  = e->chronoRatio;
            Uint32 nowB  = SDL_GetTicks();
            int fullSz   = e->batLogoRect.w;
            /* size shrinks from 100% to 40% as time runs out */
            int sz = (int)(fullSz * (0.4f + 0.6f * ratio));
            int cx = e->batLogoRect.x + fullSz / 2;
            int cy = e->batLogoRect.y + fullSz / 2;
            SDL_Rect lr = { cx - sz/2, cy - sz/2, sz, sz };
            /* pulse speed increases as time runs out */
            float speed = 300.0f + (1.0f - ratio) * 1200.0f;
            float pulse = 0.5f + 0.5f * sinf((float)nowB / speed);
            /* brightness: full when time ok, flashes red when low */
            if (ratio > 0.25f) {
                Uint8 mod = (Uint8)(180 + pulse * 75);
                SDL_SetTextureColorMod(e->batLogoTex, mod, mod, mod);
                SDL_SetTextureAlphaMod(e->batLogoTex, 255);
            } else {
                /* red flash when critical */
                Uint8 r2 = (Uint8)(200 + pulse * 55);
                Uint8 gb = (Uint8)(pulse * 80);
                SDL_SetTextureColorMod(e->batLogoTex, r2, gb, gb);
                SDL_SetTextureAlphaMod(e->batLogoTex, (Uint8)(180 + pulse * 75));
            }
            SDL_RenderCopy(renderer, e->batLogoTex, NULL, &lr);
            /* reset modulation */
            SDL_SetTextureColorMod(e->batLogoTex, 255, 255, 255);
            SDL_SetTextureAlphaMod(e->batLogoTex, 255);
        }
    }

    /* Chrono bar */
    {
        float ratio = (float)e->chronoSecondes / (float)CHRONO_SECS;
        drawChronoBar(renderer, e->chronoDesignTex,
                      &e->chronoRect, &e->chronoDesignRect, ratio);
    }

    /* =========================================================
       Question card — left zone
       ========================================================= */
    QuestionData *q     = &e->questions[e->ordreJeu[e->questionIndex]];
    SDL_Color     black = {0, 0, 0, 255};

    if (e->cardTexture)
        renderTextureRotated(renderer, e->cardTexture, &e->cardRect, e->cardAngle);
    else {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
        SDL_RenderFillRect(renderer, &e->cardRect);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }

    /* Text zone — shifted left inside card */
    SDL_Rect qZone = {
        e->cardRect.x + e->cardRect.w / 8,
        e->cardRect.y + e->cardRect.h / 7,
        e->cardRect.w * 3 / 5,
        e->cardRect.h * 5 / 7
    };
    renderTextCentered(renderer, fontTiny, q->question, black, &qZone);

    /* =========================================================
       3 Answer cards
       ========================================================= */
    for (int i = 0; i < NB_PROPS; i++) {

        if (e->propTexture[i])
            renderTextureRotated(renderer, e->propTexture[i],
                                 &e->propRect[i], e->propAngle[i]);
        else {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 240, 240, 255, 200);
            SDL_RenderFillRect(renderer, &e->propRect[i]);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }

        if (!e->showResult && e->hovered == i) {
            SDL_Rect hr = { e->propRect[i].x - 20, e->propRect[i].y - 20,
                            e->propRect[i].w + 40, e->propRect[i].h + 40 };
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 200, 0, 130);
            SDL_RenderFillRect(renderer, &hr);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }

        if (e->showResult && e->selected == i) {
            SDL_Rect hr = { e->propRect[i].x - 20, e->propRect[i].y - 20,
                            e->propRect[i].w + 40, e->propRect[i].h + 40 };
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            if (e->correct) SDL_SetRenderDrawColor(renderer, 0,   200, 0,   190);
            else            SDL_SetRenderDrawColor(renderer, 200, 0,   0,   190);
            SDL_RenderFillRect(renderer, &hr);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }

        if (e->showResult && i == q->bonneReponse && e->selected != i) {
            SDL_Rect hr = { e->propRect[i].x - 20, e->propRect[i].y - 20,
                            e->propRect[i].w + 40, e->propRect[i].h + 40 };
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 200, 0, 140);
            SDL_RenderFillRect(renderer, &hr);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }

        SDL_Rect tZone = {
            e->propRect[i].x + e->propRect[i].w * 15 / 100,
            e->propRect[i].y + e->propRect[i].h * 2 / 7,
            e->propRect[i].w * 5 / 10,
            e->propRect[i].h * 3 / 7
        };
        renderTextCentered(renderer, fontSmall, q->prop[i], black, &tZone);
    }

    /* =========================================================
       Result banner — bottom center
       ========================================================= */
    if (e->showResult || e->chronoExpire) {
        const char *msg;
        SDL_Color   col;
        if (e->chronoExpire && !e->showResult)
            { msg="Time is up! Next question...";   col=(SDL_Color){255,120,0,255}; }
        else if (e->correct)
            { msg="Correct! +1 point";              col=(SDL_Color){0,220,0,255};   }
        else
            { msg="Wrong answer! Next question..."; col=(SDL_Color){220,0,0,255};   }

        int tw=0, th=0;
        TTF_SizeUTF8(fontSmall, msg, &tw, &th);
        int bx = (W - tw) / 2 - 20;
        int by = H - th - 20;
        SDL_Rect bg2 = {bx, by, tw + 40, th + 14};
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 210);
        SDL_RenderFillRect(renderer, &bg2);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_Surface *s = TTF_RenderUTF8_Blended(fontSmall, msg, col);
        if (s) {
            SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_Rect d = {(W - s->w)/2, by + 7, s->w, s->h};
            if (t) { SDL_RenderCopy(renderer, t, NULL, &d); SDL_DestroyTexture(t); }
            SDL_FreeSurface(s);
        }
    }
}

/* ============================================================
   handleEnigmeEvent
   ============================================================ */
void handleEnigmeEvent(Enigme *e, SDL_Event *event)
{
    if (!e || !event || e->questionIndex >= NB_QUESTIONS) return;

    if (event->type == SDL_MOUSEMOTION) {
        int mx=event->motion.x, my=event->motion.y;
        e->hovered = -1;
        for (int i = 0; i < NB_PROPS; i++)
            if (mx>=e->propRect[i].x && mx<e->propRect[i].x+e->propRect[i].w &&
                my>=e->propRect[i].y && my<e->propRect[i].y+e->propRect[i].h)
                { e->hovered=i; break; }
    }

    if (e->showResult || e->waitingSuspense) return;

    int choix = -1;

    if (event->type == SDL_MOUSEBUTTONDOWN &&
        event->button.button == SDL_BUTTON_LEFT) {
        int mx=event->button.x, my=event->button.y;
        for (int i = 0; i < NB_PROPS; i++)
            if (mx>=e->propRect[i].x && mx<e->propRect[i].x+e->propRect[i].w &&
                my>=e->propRect[i].y && my<e->propRect[i].y+e->propRect[i].h)
                { choix=i; break; }
    }

    if (event->type == SDL_KEYDOWN && !event->key.repeat) {
        switch (event->key.keysym.sym) {
            case SDLK_a: case SDLK_1: choix=0; break;
            case SDLK_b: case SDLK_2: choix=1; break;
            case SDLK_c: case SDLK_3: choix=2; break;
            default: break;
        }
    }

    if (choix >= 0 && choix < NB_PROPS) {
        QuestionData *q = &e->questions[e->ordreJeu[e->questionIndex]];
        e->selected        = choix;
        e->correct         = (choix == q->bonneReponse);
        /* score incremented after suspense, in updateEnigme */
        e->waitingSuspense = 1;
        e->suspenseTime    = SDL_GetTicks();
        playSound(e, SND_SUSPENSE);
    }
}

/* ============================================================
   freeEnigme
   ============================================================ */
void freeEnigme(Enigme *e)
{
    if (!e) return;
    Mix_HaltMusic();
    if (e->bgTexture)       { SDL_DestroyTexture(e->bgTexture);       e->bgTexture=NULL; }
    if (e->cardTexture)     { SDL_DestroyTexture(e->cardTexture);      e->cardTexture=NULL; }
    if (e->batLogoTex)      { SDL_DestroyTexture(e->batLogoTex);       e->batLogoTex=NULL; }
    if (e->chronoDesignTex) { SDL_DestroyTexture(e->chronoDesignTex);  e->chronoDesignTex=NULL; }
    for (int i=0; i<NB_PROPS; i++)
        if (e->propTexture[i]) { SDL_DestroyTexture(e->propTexture[i]); e->propTexture[i]=NULL; }
    if (e->soundBat)      { Mix_FreeMusic(e->soundBat);      e->soundBat=NULL; }
    if (e->soundSuspense) { Mix_FreeMusic(e->soundSuspense); e->soundSuspense=NULL; }
    if (e->soundCorrect)  { Mix_FreeMusic(e->soundCorrect);  e->soundCorrect=NULL; }
    if (e->soundWrong)    { Mix_FreeMusic(e->soundWrong);    e->soundWrong=NULL; }
}
