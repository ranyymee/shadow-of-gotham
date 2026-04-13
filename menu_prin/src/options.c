#include "options.h"
#include <stdio.h>
#include <math.h>



static void drawPanel(SDL_Renderer *r, int x, int y, int w, int h,
                      Uint8 br, Uint8 bg2, Uint8 bb)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 4, 28, 32, 100);
    SDL_Rect p = {x, y, w, h};
    SDL_RenderFillRect(r, &p);

    SDL_SetRenderDrawColor(r, br, bg2, bb, 160);
    SDL_RenderDrawRect(r, &p);

    SDL_SetRenderDrawColor(r, br, bg2, bb, 50);
    SDL_Rect top = {x+2, y+1, w-4, 1};
    SDL_RenderFillRect(r, &top);

    SDL_SetRenderDrawColor(r, br, bg2, bb, 25);
    SDL_Rect inner = {x+2, y+2, w-4, h-4};
    SDL_RenderDrawRect(r, &inner);
}

static void drawBatText(SDL_Renderer *r, TTF_Font *font,
                        const char *text, SDL_Rect area,
                        Uint8 cr, Uint8 cg, Uint8 cb)
{
    if (!font) return;
    SDL_Color col = {cr, cg, cb, 255};
    SDL_Surface *surf = TTF_RenderText_Blended(font, text, col);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    if (tex) {
        SDL_Rect dst;
        dst.w = surf->w;
        dst.h = surf->h;
        dst.x = area.x + (area.w - dst.w) / 2;
        dst.y = area.y + (area.h - dst.h) / 2;
        SDL_RenderCopy(r, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

static void drawVolBar(SDL_Renderer *r, int x, int y, int w, int h,
                       int val, int maxV,
                       Uint8 cr, Uint8 cg, Uint8 cb, float pulse)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 18, 28, 35, 200);
    SDL_Rect bg = {x, y, w, h};
    SDL_RenderFillRect(r, &bg);

    int totalSegs  = maxV / 8;
    int filledSegs = val  / 8;
    int segW       = (w - totalSegs) / totalSegs;

    for (int i = 0; i < totalSegs; i++) {
        int sx = x + i * (segW + 1);
        SDL_Rect seg = {sx, y+2, segW, h-4};
        if (i < filledSegs) {
            float ratio = (float)i / totalSegs;
            Uint8 R = (Uint8)(cr * (1.0f - ratio * 0.3f));
            Uint8 G = (Uint8)(cg * (1.0f - ratio * 0.1f));
            Uint8 B = cb;
            if (i == filledSegs - 1) {
                float p = 0.7f + 0.3f * pulse;
                int ri = (int)(R * p); R = ri > 255 ? 255 : (Uint8)ri;
                int gi = (int)(G * p); G = gi > 255 ? 255 : (Uint8)gi;
            }
            SDL_SetRenderDrawColor(r, R, G, B, 255);
        } else {
            SDL_SetRenderDrawColor(r, 30, 38, 48, 160);
        }
        SDL_RenderFillRect(r, &seg);
    }

    SDL_SetRenderDrawColor(r, cr, cg, cb, 80);
    SDL_RenderDrawRect(r, &bg);
}

static void renderButtonHover(SDL_Renderer *r, SDL_Texture *tex,
                               SDL_Texture *hover, SDL_Rect rect)
{
    if (!tex) return;
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    SDL_Point p = {mx, my};
    SDL_Rect dr = rect;
    int isHover = SDL_PointInRect(&p, &rect);

    if (isHover) {
        dr.x -= HOVER_SCALE;
        dr.y -= HOVER_SCALE;
        dr.w += HOVER_SCALE * 2;
        dr.h += HOVER_SCALE * 2;
    }

    SDL_Texture *toRender = (isHover && hover) ? hover : tex;
    SDL_SetTextureColorMod(toRender, 255, 255, 255);

    if (isHover && !hover)
        SDL_SetTextureColorMod(toRender, 255, 255, 180);

    SDL_RenderCopy(r, toRender, NULL, &dr);
}

static void renderButton(SDL_Renderer *r, SDL_Texture *tex, SDL_Rect rect)
{
    renderButtonHover(r, tex, NULL, rect);
}

static void renderContrastBtn(SDL_Renderer *r, SDL_Texture *img,
                               SDL_Rect rect, int active, float pulse)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);

    SDL_Rect box = {rect.x - 3, rect.y - 3, rect.w + 6, rect.h + 6};

    if (active) {
        Uint8 glow = (Uint8)(140 + 80 * pulse);
        SDL_SetRenderDrawColor(r, 0, glow, 200, 55);
        SDL_Rect halo = {box.x - 4, box.y - 4, box.w + 8, box.h + 8};
        SDL_RenderFillRect(r, &halo);
        SDL_SetRenderDrawColor(r, 0, 210, 200, 230);
        SDL_RenderDrawRect(r, &box);
        SDL_SetRenderDrawColor(r, 100, 255, 240, 80);
        SDL_Rect inner = {box.x+1, box.y+1, box.w-2, box.h-2};
        SDL_RenderDrawRect(r, &inner);
    } else {
        SDL_SetRenderDrawColor(r, 60, 80, 90, 120);
        SDL_RenderDrawRect(r, &box);
    }

    if (img) {
        SDL_SetTextureColorMod(img, active ? 255 : 160,
                                    active ? 255 : 160,
                                    active ? 255 : 170);
        SDL_RenderCopy(r, img, NULL, &rect);
    }
}

static void applyContrast(SDL_Renderer *r, ContrastMode c)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    switch (c) {
        case CONTRAST_LOW:
            SDL_SetRenderDrawColor(r, 0, 0, 0, 65);
            break;
        case CONTRAST_MID:
            return;
        case CONTRAST_HIGH:
            SDL_SetRenderDrawColor(r, 255, 255, 255, 40);
            break;
        default: return;
    }
    SDL_Rect full = {0, 0, 800, 600};
    SDL_RenderFillRect(r, &full);
}

/* ── INIT ── */
int initOptions(Options *opt, SDL_Renderer *renderer)
{
    opt->fadeAlpha   = 0;
    opt->isFading    = 0;
    opt->volumeMusic = 80;
    opt->volumeSFX   = 96;
    opt->contrast    = CONTRAST_MID;
    opt->pulseT      = 0.0f;

    /* ── SDL_image init ── */
    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) {
        fprintf(stderr, "[WARN] IMG_Init: %s\n", IMG_GetError());
    }

    /* Font */
    if (TTF_Init() == -1) {
        fprintf(stderr, "[WARN] TTF_Init: %s\n", TTF_GetError());
        opt->font = NULL;
    } else {
opt->font = TTF_OpenFont("assets/font/batmfa__.ttf", 28);
        if (!opt->font)
            fprintf(stderr, "[WARN] batmfa__.ttf: %s\n", TTF_GetError());
    }


LOAD_TEXTURE("assets/options/background.jpg", opt->background);
    opt->bgPos  = (SDL_Rect){0, 0, 1280, 810};

LOAD_TEXTURE("assets/options/options.png",  opt->topBanner);
    opt->topPos = (SDL_Rect){250, 18, 300, 100};

LOAD_TEXTURE("assets/options/screen.png",   opt->btnGraphics);
LOAD_TEXTURE("assets/options/sound.png",    opt->btnSound);
LOAD_TEXTURE("assets/options/graphic.png",  opt->btnGraphicsHover);
LOAD_TEXTURE("assets/options/soud2.png",    opt->btnSoundHover);

LOAD_TEXTURE("assets/options/back1.png",    opt->btnBackMain);
LOAD_TEXTURE("assets/options/back2.png",    opt->btnBackGraphics);
LOAD_TEXTURE("assets/options/back3.png",    opt->btnBackSound);

LOAD_TEXTURE("assets/options/backb1.png",   opt->btnBackMainHover);
LOAD_TEXTURE("assets/options/backb2.png",   opt->btnBackGraphicsHover);
LOAD_TEXTURE("assets/options/backb3.png",   opt->btnBackSoundHover);

LOAD_TEXTURE("assets/options/window.png",   opt->btnWindow);
LOAD_TEXTURE("assets/options/full.png",     opt->btnWide);

LOAD_TEXTURE("assets/options/low.png",      opt->contrastImg[CONTRAST_LOW]);
LOAD_TEXTURE("assets/options/mid.png",      opt->contrastImg[CONTRAST_MID]);
LOAD_TEXTURE("assets/options/high.png",     opt->contrastImg[CONTRAST_HIGH]);

LOAD_TEXTURE("assets/options/plus.png",     opt->btnPlus);
LOAD_TEXTURE("assets/options/minus.png",    opt->btnMinus);
LOAD_TEXTURE("assets/options/plus.png",     opt->btnPlusSFX);
LOAD_TEXTURE("assets/options/minus.png",    opt->btnMinusSFX);

    /* ── POSITIONS ── */
    opt->posGraphics = (SDL_Rect){250, 180, 300, 110};
    opt->posSound    = (SDL_Rect){250, 320, 300, 110};

    opt->posBackMain     = (SDL_Rect){BACK_X, BACK_Y, BACK_W, BACK_H};
    opt->posBackGraphics = (SDL_Rect){BACK_X, BACK_Y, BACK_W, BACK_H};
    opt->posBackSound    = (SDL_Rect){BACK_X, BACK_Y, BACK_W, BACK_H};

    opt->posWindow         = (SDL_Rect){130, 190, 220, 85};
    opt->posWide           = (SDL_Rect){450, 190, 220, 85};
    opt->posContrastLabel  = (SDL_Rect){270, 300, 260, 38};
    opt->posContrastBtn[0] = (SDL_Rect){120, 348, 150, 70};
    opt->posContrastBtn[1] = (SDL_Rect){325, 348, 150, 70};
    opt->posContrastBtn[2] = (SDL_Rect){530, 348, 150, 70};

    opt->posLabelMusic = (SDL_Rect){ 70, 190, 130, 44};   /* label MUSIC */
    opt->posMinus      = (SDL_Rect){220, 190,  52, 44};   /* − music     */
    opt->posPlus       = (SDL_Rect){630, 190,  52, 44};   /* + music     */
    opt->posLabelSFX   = (SDL_Rect){ 70, 280, 130, 44};   /* label SFX   */
    opt->posMinusSFX   = (SDL_Rect){220, 280,  52, 44};   /* − sfx       */
    opt->posPlusSFX    = (SDL_Rect){630, 280,  52, 44};   /* + sfx       */

    /* ── Audio ── */


    
    
    opt->music = Mix_LoadMUS("assets/audio/menu_music.mp3");
    if (opt->music) Mix_PlayMusic(opt->music, -1);
    Mix_VolumeMusic(opt->volumeMusic);

opt->clickSound = Mix_LoadWAV("assets/audio/click.wav");
    if (!opt->clickSound)
        fprintf(stderr, "[WARN] click.wav: %s\n", Mix_GetError());
    else
        Mix_Volume(-1, opt->volumeSFX);

    opt->state         = MENU_MAIN;
    opt->previousState = MENU_MAIN;

    return 1;
}

/* ── FADE ── */
void startFade(Options *opt, MenuState target)
{
    opt->isFading  = 1;
    opt->fadeAlpha = 0;
    opt->nextState = target;
}

/* ── INPUT ── */
static void playClick(Options *opt)
{
    if (opt->clickSound) Mix_PlayChannel(-1, opt->clickSound, 0);
}

static void goBack(Options *opt)
{
    playClick(opt);
    startFade(opt, opt->previousState);
}

void inputOptions(Options *opt, SDL_Event event, SDL_Window *window, int *running)
{
    if (event.type == SDL_QUIT) { *running = 0; return; }
    if (opt->isFading) return;

    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                if (opt->state != MENU_MAIN) goBack(opt);
                else *running = 0;
                break;
            case SDLK_PLUS:
            case SDLK_KP_PLUS:
            case SDLK_EQUALS:
                if (opt->volumeMusic < 128) {
                    opt->volumeMusic += 8;
                    if (opt->volumeMusic > 128) opt->volumeMusic = 128;
                    Mix_VolumeMusic(opt->volumeMusic);
                }
                break;
            case SDLK_MINUS:
            case SDLK_KP_MINUS:
                if (opt->volumeMusic > 0) {
                    opt->volumeMusic -= 8;
                    if (opt->volumeMusic < 0) opt->volumeMusic = 0;
                    Mix_VolumeMusic(opt->volumeMusic);
                }
                break;
            default: break;
        }
        return;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        SDL_Point p = {event.button.x, event.button.y};

        if (opt->state == MENU_MAIN     && SDL_PointInRect(&p, &opt->posBackMain))     { playClick(opt); *running = 0; return; }
        if (opt->state == MENU_GRAPHICS && SDL_PointInRect(&p, &opt->posBackGraphics)) { goBack(opt); return; }
        if (opt->state == MENU_SOUND    && SDL_PointInRect(&p, &opt->posBackSound))    { goBack(opt); return; }

        if (opt->state == MENU_MAIN) {
            if      (SDL_PointInRect(&p, &opt->posGraphics)) { playClick(opt); opt->previousState = MENU_MAIN; startFade(opt, MENU_GRAPHICS); }
            else if (SDL_PointInRect(&p, &opt->posSound))    { playClick(opt); opt->previousState = MENU_MAIN; startFade(opt, MENU_SOUND);    }
        }
        else if (opt->state == MENU_GRAPHICS) {
            if (SDL_PointInRect(&p, &opt->posWindow)) { playClick(opt); SDL_SetWindowFullscreen(window, 0); }
            if (SDL_PointInRect(&p, &opt->posWide))   { playClick(opt); SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP); }
            for (int i = 0; i < CONTRAST_COUNT; i++)
                if (SDL_PointInRect(&p, &opt->posContrastBtn[i])) { playClick(opt); opt->contrast = (ContrastMode)i; }
        }
        else if (opt->state == MENU_SOUND) {
            if (SDL_PointInRect(&p, &opt->posPlus)     && opt->volumeMusic < 128) { opt->volumeMusic += 8; if (opt->volumeMusic > 128) opt->volumeMusic = 128; Mix_VolumeMusic(opt->volumeMusic); playClick(opt); }
            if (SDL_PointInRect(&p, &opt->posMinus)    && opt->volumeMusic > 0)   { opt->volumeMusic -= 8; if (opt->volumeMusic < 0)   opt->volumeMusic = 0;   Mix_VolumeMusic(opt->volumeMusic); playClick(opt); }
            if (SDL_PointInRect(&p, &opt->posPlusSFX)  && opt->volumeSFX  < 128)  { opt->volumeSFX  += 8; if (opt->volumeSFX  > 128) opt->volumeSFX  = 128;   Mix_Volume(-1, opt->volumeSFX); playClick(opt); }
            if (SDL_PointInRect(&p, &opt->posMinusSFX) && opt->volumeSFX  > 0)    { opt->volumeSFX  -= 8; if (opt->volumeSFX  < 0)   opt->volumeSFX  = 0;     Mix_Volume(-1, opt->volumeSFX); playClick(opt); }
        }
    }
}

/* ── RENDER ── */
void renderOptions(Options *opt, SDL_Renderer *renderer)
{
    opt->pulseT += 0.04f;
    if (opt->pulseT > 6.2832f) opt->pulseT -= 6.2832f;
    float pulse  = sinf(opt->pulseT);
    float pulseP = (pulse + 1.0f) * 0.5f;

    if (opt->background)
        SDL_RenderCopy(renderer, opt->background, NULL, &opt->bgPos);
    else {
        SDL_SetRenderDrawColor(renderer, 10, 10, 28, 255);
        SDL_RenderClear(renderer);
    }

    applyContrast(renderer, opt->contrast);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < 5; i++) {
        int alpha = (int)(14 + 10 * sinf(opt->pulseT + i * 1.2f));
        SDL_SetRenderDrawColor(renderer, 0, 200, 180, (Uint8)alpha);
        SDL_Rect line = {0, 100 + i * 100, 800, 1};
        SDL_RenderFillRect(renderer, &line);
    }

    if (opt->topBanner)
        SDL_RenderCopy(renderer, opt->topBanner, NULL, &opt->topPos);

    if (opt->state == MENU_MAIN) {
        /* Same transparent panel as other screens */
        drawPanel(renderer, 100, 120, 600, 380, 0, 185, 180);

        /* Subtle separator between the two buttons */
        SDL_SetRenderDrawColor(renderer, 0, 185, 180, 50);
        SDL_Rect s1 = {120, 300, 560, 1};
        SDL_RenderFillRect(renderer, &s1);

        renderButtonHover(renderer, opt->btnGraphics, opt->btnGraphicsHover, opt->posGraphics);
        renderButtonHover(renderer, opt->btnSound,    opt->btnSoundHover,    opt->posSound);
        renderButtonHover(renderer, opt->btnBackMain, opt->btnBackMainHover, opt->posBackMain);
    }
    else if (opt->state == MENU_GRAPHICS) {
        drawPanel(renderer, 100, 120, 600, 380, 0, 180, 200);

        SDL_SetRenderDrawColor(renderer, 0, 160, 200, 80);
        SDL_Rect s1 = {120, 305, 560, 1};
        SDL_RenderFillRect(renderer, &s1);

        renderButton(renderer, opt->btnWindow, opt->posWindow);
        renderButton(renderer, opt->btnWide,   opt->posWide);

        drawBatText(renderer, opt->font, "CONTRAST",
                    opt->posContrastLabel, 180, 230, 240);

        for (int i = 0; i < CONTRAST_COUNT; i++) {
            renderContrastBtn(renderer,
                              opt->contrastImg[i],
                              opt->posContrastBtn[i],
                              (opt->contrast == (ContrastMode)i),
                              pulseP);
        }

        renderButtonHover(renderer, opt->btnBackGraphics, opt->btnBackGraphicsHover, opt->posBackGraphics);
    }
    else if (opt->state == MENU_SOUND) {
        drawPanel(renderer, 55, 120, 690, 380, 0, 190, 160);

        SDL_SetRenderDrawColor(renderer, 0, 160, 140, 50);
        SDL_Rect sm = {75, 178, 650, 1};
        SDL_Rect ss = {75, 268, 650, 1};
        SDL_Rect sb = {75, 358, 650, 1};
        SDL_RenderFillRect(renderer, &sm);
        SDL_RenderFillRect(renderer, &ss);
        SDL_RenderFillRect(renderer, &sb);

        /* ── MUSIC row ── */
        drawBatText(renderer, opt->font, "MUSIC",
                    opt->posLabelMusic, 220, 230, 235);

        renderButton(renderer, opt->btnMinus, opt->posMinus);
        drawVolBar(renderer, 290, 194, 330, 38,
                   opt->volumeMusic, 128, 30, 210, 120, pulseP);
        renderButton(renderer, opt->btnPlus, opt->posPlus);

        /* ── SFX row ── */
        drawBatText(renderer, opt->font, "SFX",
                    opt->posLabelSFX, 200, 210, 220);

        renderButton(renderer, opt->btnMinusSFX, opt->posMinusSFX);
        drawVolBar(renderer, 290, 285, 330, 38,
                   opt->volumeSFX, 128, 40, 100, 210, pulseP);
        renderButton(renderer, opt->btnPlusSFX, opt->posPlusSFX);

        renderButtonHover(renderer, opt->btnBackSound, opt->btnBackSoundHover, opt->posBackSound);
    }

    if (opt->isFading) {
        opt->fadeAlpha += 12;
        if (opt->fadeAlpha >= 255) {
            opt->state     = opt->nextState;
            opt->fadeAlpha = 0;
            opt->isFading  = 0;
        }
        Uint8 fa = (opt->fadeAlpha > 255) ? 255 : (Uint8)opt->fadeAlpha;
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, fa);
        SDL_Rect fr = {0, 0, 800, 600};
        SDL_RenderFillRect(renderer, &fr);
    }
}

/* ── FREE ── */
void freeOptions(Options *opt)
{
    SDL_DestroyTexture(opt->background);
    SDL_DestroyTexture(opt->topBanner);

    SDL_DestroyTexture(opt->btnGraphics);
    SDL_DestroyTexture(opt->btnGraphicsHover);
    SDL_DestroyTexture(opt->btnSound);
    SDL_DestroyTexture(opt->btnSoundHover);

    SDL_DestroyTexture(opt->btnBackMain);
    SDL_DestroyTexture(opt->btnBackGraphics);
    SDL_DestroyTexture(opt->btnBackGraphicsHover);
    SDL_DestroyTexture(opt->btnBackSound);
    SDL_DestroyTexture(opt->btnBackSoundHover);
    SDL_DestroyTexture(opt->btnBackMainHover);

    SDL_DestroyTexture(opt->btnWindow);
    SDL_DestroyTexture(opt->btnWide);
    for (int i = 0; i < CONTRAST_COUNT; i++)
        SDL_DestroyTexture(opt->contrastImg[i]);

    SDL_DestroyTexture(opt->btnPlus);
    SDL_DestroyTexture(opt->btnMinus);
    SDL_DestroyTexture(opt->btnPlusSFX);
    SDL_DestroyTexture(opt->btnMinusSFX);

    if (opt->font)       TTF_CloseFont(opt->font);
    TTF_Quit();
    if (opt->music)      Mix_FreeMusic(opt->music);
    if (opt->clickSound) Mix_FreeChunk(opt->clickSound);

    IMG_Quit();
}
