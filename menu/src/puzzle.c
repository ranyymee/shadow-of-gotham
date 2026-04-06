/*
 * puzzle.c — TACTICAL DECRYPTION ENGINE: Puzzle Screen
 * Gameplay: drag 9 pieces from the repo onto the grid.
 * Backend validates every frame. Hint system: 3 charges (H key or button).
 */

#include "header.h"

/* ─────────────────────────────────────────────────────────────────────────
   PARTICLE STATE
   ───────────────────────────────────────────────────────────────────────── */
static Spark sparks[MAX_SPARKS];
static int   sparkCount = 0;

static void spawnSparks(int cx, int cy, int ok) {
    int n = ok ? 28 : 14;
    for (int i = 0; i < n && sparkCount < MAX_SPARKS; i++) {
        Spark *s = &sparks[sparkCount++];
        float ang = (float)i / n * 6.283f + ((float)(rand() % 100)) / 200.0f;
        float spd = 1.5f + ((float)(rand() % 100)) / 100.0f * 3.0f;
        s->x = (float)cx; s->y = (float)cy;
        s->vx = cosf(ang) * spd; s->vy = sinf(ang) * spd - 1.5f;
        s->life = s->maxLife = 0.6f + ((float)(rand() % 60)) / 100.0f;
        if (ok) { s->r = 0; s->g = (Uint8)(180 + rand() % 75); s->b = (Uint8)(100 + rand() % 155); }
        else    { s->r = 255; s->g = (Uint8)(rand() % 80);     s->b = (Uint8)(rand() % 60); }
    }
}

static void updateSparks(float dt) {
    for (int i = 0; i < sparkCount;) {
        Spark *s = &sparks[i];
        s->x += s->vx; s->y += s->vy; s->vy += 0.12f; s->life -= dt;
        if (s->life <= 0) { sparks[i] = sparks[--sparkCount]; } else i++;
    }
}

static void drawSparks(void) {
    for (int i = 0; i < sparkCount; i++) {
        Spark *s = &sparks[i];
        Uint8 a = (Uint8)(s->life / s->maxLife * 220);
        SC(s->r, s->g, s->b, a);
        SDL_RenderDrawPoint(g_ren, (int)s->x,     (int)s->y);
        SDL_RenderDrawPoint(g_ren, (int)s->x + 1, (int)s->y);
        SDL_RenderDrawPoint(g_ren, (int)s->x,     (int)s->y + 1);
    }
}

/* ─────────────────────────────────────────────────────────────────────────
   AMBIENT DUST
   ───────────────────────────────────────────────────────────────────────── */
static Dust dust[MAX_DUST];

static void initDust(void) {
    for (int i = 0; i < MAX_DUST; i++) {
        dust[i].x     = (float)(rand() % WIN_W);
        dust[i].y     = (float)(rand() % WIN_H);
        dust[i].spd   = 0.1f + ((float)(rand() % 20)) / 40.0f;
        dust[i].phase = (float)(rand() % 628) / 100.0f;
        dust[i].size  = (float)(1 + rand() % 2);
    }
}

static void drawDust(Uint32 now) {
    for (int i = 0; i < MAX_DUST; i++) {
        Dust *d = &dust[i];
        d->y -= d->spd;
        if (d->y < 0) d->y = (float)WIN_H;
        float t = sinf(d->phase + (float)now / 2000.0f);
        d->x += t * 0.3f;
        float bright = 0.3f + 0.5f * sinf(d->phase + (float)now / 1500.0f);
        SC(0, 200, 255, (Uint8)(bright * 50));
        SDL_RenderDrawPoint(g_ren, (int)d->x, (int)d->y);
    }
}

/* ─────────────────────────────────────────────────────────────────────────
   BACKGROUND CIRCUIT LINES
   ───────────────────────────────────────────────────────────────────────── */
static void drawCircuits(Uint32 now) {
    int pts[][4] = {
        {10,290,200,290}, {200,290,200,350}, {200,350,240,350},
        {10,320,100,320}, {100,320,100,380},
        {756,150,840,150}, {840,150,840,220}, {840,220,890,220},
        {756,180,800,180}, {800,180,800,240},
        {10,420,50,420},  {50,380,50,420},
        {880,350,880,420}, {850,420,880,420},
    };
    float pulse = 0.3f + 0.3f * sinf((float)now / 1800.0f);
    SC(0, 180, 255, (Uint8)(pulse * 45));
    for (int i = 0; i < (int)(sizeof(pts) / sizeof(pts[0])); i++)
        SDL_RenderDrawLine(g_ren, pts[i][0], pts[i][1], pts[i][2], pts[i][3]);

    SC(0, 220, 255, (Uint8)(pulse * 110));
    int dots[][2] = {
        {200,290}, {200,350}, {100,320}, {840,150},
        {840,220}, {800,180}, {50,420},  {880,420}
    };
    for (int i = 0; i < (int)(sizeof(dots) / sizeof(dots[0])); i++) {
        SDL_RenderDrawPoint(g_ren, dots[i][0],     dots[i][1]);
        SDL_RenderDrawPoint(g_ren, dots[i][0] + 1, dots[i][1]);
        SDL_RenderDrawPoint(g_ren, dots[i][0],     dots[i][1] + 1);
        SDL_RenderDrawPoint(g_ren, dots[i][0] + 1, dots[i][1] + 1);
    }
}

/* ─────────────────────────────────────────────────────────────────────────
   SLICE HELPERS
   ───────────────────────────────────────────────────────────────────────── */
static int          g_puzzW   = 0;
static int          g_puzzH   = 0;
static SDL_Texture *g_puzzTex = NULL;

static SDL_Rect sliceFor(int slot) {
    return (SDL_Rect){
        (slot % GCOLS) * (g_puzzW / GCOLS),
        (slot / GCOLS) * (g_puzzH / GROWS),
        g_puzzW / GCOLS,
        g_puzzH / GROWS
    };
}

static void drawSlice(int pieceIdx, SDL_Rect dst, Uint8 alpha) {
    if (!g_puzzTex) return;
    SDL_Rect src = sliceFor(pieceIdx);
    SDL_SetTextureAlphaMod(g_puzzTex, alpha);
    SDL_RenderCopy(g_ren, g_puzzTex, &src, &dst);
    SDL_SetTextureAlphaMod(g_puzzTex, 255);
}

static SDL_Texture *makePuzzlePlaceholder(int puzzleIdx) {
    g_puzzW = 300; g_puzzH = 300;
    SDL_Texture *t = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_RGBA8888,
                                       SDL_TEXTUREACCESS_TARGET, g_puzzW, g_puzzH);
    SDL_SetRenderTarget(g_ren, t);
    SDL_SetRenderDrawColor(g_ren, 0, 14, 34, 255);
    SDL_RenderClear(g_ren);

    Uint8 tints[4][3] = {{0,110,170}, {20,130,60}, {90,40,150}, {170,90,0}};
    int cw = g_puzzW / 3, ch = g_puzzH / 3;
    for (int i = 0; i < 9; i++) {
        SDL_Rect r = { (i % 3) * cw, (i / 3) * ch, cw, ch };
        Uint8 base = 40 + (i * 18);
        SDL_SetRenderDrawColor(g_ren,
            tints[puzzleIdx][0] + base,
            tints[puzzleIdx][1] + base,
            tints[puzzleIdx][2] + base, 255);
        SDL_RenderFillRect(g_ren, &r);
        SDL_SetRenderDrawColor(g_ren, 255, 255, 255, 14);
        for (int d = -ch; d < cw; d += 14)
            SDL_RenderDrawLine(g_ren, r.x + d, r.y, r.x + d + ch, r.y + ch);
        SDL_SetRenderDrawColor(g_ren, 0, 220, 255, 50);
        SDL_RenderDrawRect(g_ren, &r);
    }
    SDL_SetRenderDrawColor(g_ren, 0, 220, 255, 80);
    SDL_RenderDrawLine(g_ren, cw,   0,      cw,   g_puzzH);
    SDL_RenderDrawLine(g_ren, 2*cw, 0,      2*cw, g_puzzH);
    SDL_RenderDrawLine(g_ren, 0,    ch,     g_puzzW, ch);
    SDL_RenderDrawLine(g_ren, 0,    2*ch,   g_puzzW, 2*ch);
    SDL_SetRenderTarget(g_ren, NULL);
    return t;
}

/* ─────────────────────────────────────────────────────────────────────────
   SHUFFLE
   ───────────────────────────────────────────────────────────────────────── */
static void shuffle(int *a, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = a[i]; a[i] = a[j]; a[j] = tmp;
    }
}

/* ─────────────────────────────────────────────────────────────────────────
   BACKEND VALIDATION
   ───────────────────────────────────────────────────────────────────────── */
static int countCorrect(Piece *pieces, int *slotOcc) {
    int ok = 0;
    for (int s = 0; s < GPIECES; s++) {
        int pi = slotOcc[s];
        if (pi >= 0 && pieces[pi].idx == s) ok++;
    }
    return ok;
}

/* ─────────────────────────────────────────────────────────────────────────
   HINT SYSTEM
   ───────────────────────────────────────────────────────────────────────── */
static char g_hintCipher[32] = "";
static int  g_hintSlot       = -1;
static int  g_hintPiece      = -1;
static int  g_hintGlow       =  0;

static void triggerHint(Piece *pieces, int *slotOcc, int *hintsLeft) {
    if (*hintsLeft <= 0) return;
    (*hintsLeft)--;
    g_hintGlow = 280;

    int candidates[GPIECES], nc = 0;
    for (int s = 0; s < GPIECES; s++) {
        int pi = slotOcc[s];
        if (pi < 0 || pieces[pi].idx != s) candidates[nc++] = s;
    }
    if (nc == 0) return;
    g_hintSlot  = candidates[rand() % nc];
    g_hintPiece = -1;
    for (int i = 0; i < GPIECES; i++) {
        if (pieces[i].idx == g_hintSlot) { g_hintPiece = i; break; }
    }
    snprintf(g_hintCipher, sizeof(g_hintCipher), "NODE_%02d", g_hintSlot + 1);
}

static void drawHintCipher(Uint32 now, SDL_Rect area) {
    (void)now;
    if (g_hintGlow <= 0) return;
    float t   = (float)g_hintGlow / 280.0f;
    char  display[32];
    int   len = (int)strlen(g_hintCipher);
    for (int i = 0; i < len; i++) {
        if (((float)(rand() % 100)) / 100.0f < t * 0.6f && g_hintCipher[i] != '_')
            display[i] = 'A' + rand() % 26;
        else
            display[i] = g_hintCipher[i];
    }
    display[len] = 0;
    Uint8      alpha = (Uint8)(t * 255);
    SDL_Color  col   = { CY_R, CY_G, CY_B, alpha };
    SDL_Surface *su  = TTF_RenderText_Blended(g_fntMid, display, col);
    if (!su) return;
    SDL_Texture *tx  = SDL_CreateTextureFromSurface(g_ren, su);
    SDL_FreeSurface(su);
    if (!tx) return;
    SDL_SetTextureAlphaMod(tx, alpha);
    int tw, th; SDL_QueryTexture(tx, NULL, NULL, &tw, &th);
    SDL_Rect d = { area.x + (area.w - tw) / 2, area.y + (area.h - th) / 2, tw, th };
    SDL_RenderCopy(g_ren, tx, NULL, &d);
    SDL_DestroyTexture(tx);
}

/* ─────────────────────────────────────────────────────────────────────────
   WIN / LOSE OVERLAY
   ───────────────────────────────────────────────────────────────────────── */
static void renderEndOverlay(GameState st, float prog, Uint32 ticks,
                              int *wantRestart, int *wantBack,
                              int mx, int my, int clicked)
{
    Uint8 veil = (Uint8)(prog * 200);
    if (st == ST_WIN) FR((SDL_Rect){0, 0, WIN_W, WIN_H}, 0,  40, 10, veil);
    else              FR((SDL_Rect){0, 0, WIN_W, WIN_H}, 40,  0,  0, veil);
    if (prog < 0.3f) return;

    float pp = (prog - 0.3f) / 0.7f; if (pp > 1) pp = 1;
    int   panH = 220;
    int   panY = (int)(WIN_H / 2 - panH / 2 - (1 - pp) * (WIN_H / 2 + panH));
    SDL_Rect panel = { WIN_W / 2 - 240, panY, 480, panH };

    if (st == ST_WIN) {
        GL(panel, GR_R, GR_G, GR_B, pp * 1.6f);
        FR(panel, 0, 10, 22, 240);
        SR(panel, GR_R, GR_G, GR_B, pp > 0.8f ? 220 : 160, 3);
        CO(panel, GR_R, GR_G, GR_B, 255, 20);
    } else {
        GL(panel, RD_R, RD_G, RD_B, pp * 1.6f);
        FR(panel, 0, 10, 22, 240);
        SR(panel, RD_R, RD_G, RD_B, pp > 0.8f ? 220 : 160, 3);
        CO(panel, RD_R, RD_G, RD_B, 255, 20);
    }

    SC(st == ST_WIN ? 0 : 255, st == ST_WIN ? 255 : 55, st == ST_WIN ? 120 : 55, 75);
    SDL_RenderDrawLine(g_ren, panel.x + 20, panY + 2, panel.x + panel.w - 20, panY + 2);

    float bp = 0.5f + 0.5f * sinf((float)ticks / 280.0f);
    Uint8 ba = (Uint8)(160 + bp * 95);
    if (st == ST_WIN) BAT(WIN_W / 2, panY + 32, 24, GR_R, GR_G, GR_B, ba);
    else              BAT(WIN_W / 2, panY + 32, 24, RD_R, RD_G, RD_B, ba);

    SDL_Rect tr = { panel.x, panY + 50, panel.w, 42 };
    if (st == ST_WIN) TC("DECRYPTION SUCCESS", tr, GR_R, GR_G, GR_B, g_fntBig);
    else              TC("DECRYPTION FAILED",  tr, RD_R, RD_G, RD_B, g_fntBig);

    SDL_Rect sr2 = { panel.x, panY + 96, panel.w, 26 };
    if (st == ST_WIN) TC("IMAGE RECONSTRUCTED — FILE UNLOCKED", sr2, 0,   200, 110, g_fntSm);
    else              TC("NEURAL LINK SEVERED — ACCESS DENIED",  sr2, 210, 70,  70,  g_fntSm);

    SC(CY_R, CY_G, CY_B, 55);
    SDL_RenderDrawLine(g_ren, panel.x + 28, panY + 130, panel.x + panel.w - 28, panY + 130);

    SDL_Rect btnR = { panel.x + 46,  panY + 144, 166, 44 };
    SDL_Rect btnB = { panel.x + 268, panY + 144, 166, 44 };
    int hR = (mx >= btnR.x && mx <= btnR.x + btnR.w && my >= btnR.y && my <= btnR.y + btnR.h);
    int hB = (mx >= btnB.x && mx <= btnB.x + btnB.w && my >= btnB.y && my <= btnB.y + btnB.h);

    if (hR) { GL(btnR, CY_R, CY_G, CY_B, 1.0f); FR(btnR, 0, 45, 65, 240); }
    else      FR(btnR, 0, 18, 38, 210);
    SR(btnR, CY_R, CY_G, CY_B, hR ? 230 : 110, hR ? 2 : 1);
    CO(btnR, CY_R, CY_G, CY_B, hR ? 255 : 130, 12);
    TC("START OVER", btnR, CY_R, CY_G, CY_B, g_fntMid);

    if (hB) { GL(btnB, 0, 200, 60, 1.0f); FR(btnB, 0, 40, 18, 240); }
    else      FR(btnB, 0, 18, 10, 210);
    SR(btnB, 0, hB ? 220 : 100, hB ? 80 : 38, hB ? 220 : 100, hB ? 2 : 1);
    CO(btnB, 0, hB ? 255 : 120, 42, hB ? 255 : 120, 12);
    TC("GO BACK", btnB, 0, hB ? 255 : 170, 65, g_fntMid);

    if (clicked) { if (hR) *wantRestart = 1; if (hB) *wantBack = 1; }
}

/* ─────────────────────────────────────────────────────────────────────────
   TOP BAR
   ───────────────────────────────────────────────────────────────────────── */
static void renderTopBar(Uint32 now, Uint32 startT, float timerFrac,
                          GameState state, Uint32 elapsed, Uint8 tR, Uint8 tG)
{
    (void)startT;
    FR((SDL_Rect){0, 0, WIN_W, 38}, 0, 6, 18, 235);
    SC(CY_R, CY_G, CY_B, 55);
    SDL_RenderDrawLine(g_ren, 0, 38, WIN_W, 38);
    SC(CY_R, CY_G, CY_B, 18);
    SDL_RenderDrawLine(g_ren, 0, 40, WIN_W, 40);

    SDL_Rect tbar = { 130, 8, 600, 14 };
    FR(tbar, 0, 18, 38, 200);
    SR(tbar, CY_R, CY_G, CY_B, 80, 1);
    FR((SDL_Rect){tbar.x, tbar.y, (int)(600 * timerFrac), tbar.h}, tR, tG, 40, 225);

    if (timerFrac < 0.25f && state == ST_PLAY) {
        float p = 0.5f + 0.5f * sinf((float)now / 200.0f);
        FR(tbar, 255, 60, 0, (Uint8)(p * 60));
    }
    CO(tbar, CY_R, CY_G, CY_B, 130, 8);

    {
        int  rm = (state == ST_PLAY) ? (int)(TIMER_MS - elapsed) : 0;
        if (rm < 0) rm = 0;
        char tb[16]; snprintf(tb, 16, "%02d:%02d", rm / 60000, (rm % 60000) / 1000);
        SDL_Rect tt = { 736, 5, 58, 26 };
        FR(tt, 0, 18, 38, 210); SR(tt, AM_R, AM_G, AM_B, 180, 1); CO(tt, AM_R, AM_G, AM_B, 130, 6);
        TC(tb, tt, AM_R, AM_G, AM_B, g_fntMid);
    }

    {
        SDL_Rect dl = { 200, 22, 470, 13 };
        FR(dl, 0, 10, 24, 160);
        TC("TACTICAL DECRYPTION — RECONSTRUCT IMAGE", dl, CY_R, CY_G, CY_B, g_fntSm);
    }

    { float bp = 0.5f + 0.5f * sinf((float)now / 550.0f); BAT(874, 19, 14, CY_R, CY_G, CY_B, (Uint8)(90 + bp * 90)); }
    { float bp = 0.5f + 0.5f * sinf((float)now / 800.0f + 1.0f); BAT(856, 19, 8, CY_R, CY_G, CY_B, (Uint8)(40 + bp * 50)); }
}

/* ─────────────────────────────────────────────────────────────────────────
   LEFT REFERENCE PANEL
   ───────────────────────────────────────────────────────────────────────── */
static void renderRefPanel(Uint32 now, int correctCount) {
    SDL_Rect lp = { 8, 44, 192, 238 };
    FR(lp, PNL_R, PNL_G, PNL_B, 205);
    SR(lp, CY_R, CY_G, CY_B, 85, 1);
    CO(lp, CY_R, CY_G, CY_B, 160, 10);

    SC(MG_R, MG_G, MG_B, 55);
    SDL_RenderDrawLine(g_ren, 9, 45, 199, 45);

    FR((SDL_Rect){8, 44, 192, 18}, 0, 28, 62, 225);
    TL("REFERENCE", 12, 46, CY_R, CY_G, CY_B, g_fntSm);

    SDL_Rect refImg = { 11, 64, 186, 162 };
    if (g_puzzTex) SDL_RenderCopy(g_ren, g_puzzTex, NULL, &refImg);
    SL(refImg, 20);
    SR(refImg, CY_R, CY_G, CY_B, 65, 1);

    {
        float sw = fmodf((float)now / 1600.0f, 1.0f);
        int   sy = refImg.y + (int)(sw * refImg.h);
        SC(CY_R, CY_G, CY_B, 35);
        SDL_RenderDrawLine(g_ren, refImg.x, sy,     refImg.x + refImg.w, sy);
        SC(CY_R, CY_G, CY_B, 12);
        SDL_RenderDrawLine(g_ren, refImg.x, sy - 1, refImg.x + refImg.w, sy - 1);
    }

    SC(CY_R, CY_G, CY_B, 40);
    for (int c = 1; c < GCOLS; c++)
        SDL_RenderDrawLine(g_ren,
            refImg.x + c * refImg.w / GCOLS, refImg.y,
            refImg.x + c * refImg.w / GCOLS, refImg.y + refImg.h);
    for (int r = 1; r < GROWS; r++)
        SDL_RenderDrawLine(g_ren,
            refImg.x, refImg.y + r * refImg.h / GROWS,
            refImg.x + refImg.w, refImg.y + r * refImg.h / GROWS);

    if (g_hintGlow > 0 && g_hintSlot >= 0) {
        float ht = (float)g_hintGlow / 280.0f;
        int   hx = refImg.x + (g_hintSlot % GCOLS) * refImg.w / GCOLS;
        int   hy = refImg.y + (g_hintSlot / GCOLS) * refImg.h / GROWS;
        float ap = 0.5f + 0.5f * sinf((float)now / 200.0f);
        FR((SDL_Rect){hx, hy, refImg.w / GCOLS, refImg.h / GROWS}, AM_R, AM_G, AM_B, (Uint8)(ap * ht * 95));
        SR((SDL_Rect){hx, hy, refImg.w / GCOLS, refImg.h / GROWS}, AM_R, AM_G, AM_B, (Uint8)(ht * 215), 2);
    }

    {
        char     pg[32]; snprintf(pg, 32, "%d / %d CORRECT", correctCount, GPIECES);
        SDL_Rect pl = { 8, 230, 192, 28 };
        FR(pl, 0, 18, 42, 215);
        SR(pl,
           correctCount == GPIECES ? GR_R : CY_R,
           correctCount == GPIECES ? GR_G : CY_G,
           correctCount == GPIECES ? GR_B : CY_B, 80, 1);
        TC(pg,
           pl,
           correctCount == GPIECES ? GR_R : CY_R,
           correctCount == GPIECES ? GR_G : CY_G,
           correctCount == GPIECES ? GR_B : CY_B,
           g_fntMid);
    }
}

/* ─────────────────────────────────────────────────────────────────────────
   CENTRE GRID PANEL
   ───────────────────────────────────────────────────────────────────────── */
static void renderGridPanel(Uint32 now, Piece *pieces, int *slotOcc,
                             SDL_Rect *gridSlot, GameState state, float winWaveT,
                             int dragging)
{
    int gpX = GRID_X - 13, gpY = GRID_Y - 24;
    int gpW = GCOLS * CELL + 26, gpH = GROWS * CELL + 38;
    FR((SDL_Rect){gpX, gpY, gpW, gpH}, PNL_R, PNL_G, PNL_B, 188);
    SR((SDL_Rect){gpX, gpY, gpW, gpH}, CY_R, CY_G, CY_B, 100, 2);
    CO((SDL_Rect){gpX, gpY, gpW, gpH}, CY_R, CY_G, CY_B, 185, 14);

    FR((SDL_Rect){gpX, gpY, gpW, 20}, 0, 28, 62, 225);
    SC(MG_R, MG_G, MG_B, 45);
    SDL_RenderDrawLine(g_ren, gpX + 1, gpY + 1, gpX + gpW - 1, gpY + 1);
    TC("DECRYPTION NODE", (SDL_Rect){gpX, gpY, gpW, 20}, CY_R, CY_G, CY_B, g_fntSm);

    int dash = 8, off = (int)(now / 55) % (dash * 2);
    for (int s = 0; s < GPIECES; s++) {
        SDL_Rect sr = gridSlot[s];

        if (slotOcc[s] >= 0) {
            int      pi  = slotOcc[s];
            SDL_Rect dst = { sr.x + 1, sr.y + 1, CELL - 2, CELL - 2 };
            Uint8    alpha = 255;

            if (state == ST_WIN) {
                float wave = sinf(winWaveT * 4.0f - (float)s * 0.5f);
                alpha = (Uint8)(200 + wave * 55);
                if (wave > 0.5f) GL(sr, GR_R, GR_G, GR_B, wave * 0.7f);
            }

            drawSlice(pieces[pi].idx, dst, alpha);

            if (pieces[pi].flashT > 0) {
                Uint8 fa = (Uint8)(pieces[pi].flashT / 0.8f * 130);
                FR(dst, 255, 255, 255, fa);
            }

            if (state == ST_WIN && pieces[pi].idx == s) {
                float wg = 0.3f + 0.3f * sinf(winWaveT * 3.0f - (float)s * 0.6f);
                GL(sr, GR_R, GR_G, GR_B, wg);
                SR(sr, GR_R, GR_G, GR_B, 160, 1);
            } else {
                SR(sr, CY_R, CY_G, CY_B, 55, 1);
            }

        } else {
            drawSlice(s, (SDL_Rect){sr.x + 2, sr.y + 2, CELL - 4, CELL - 4}, 16);
            FR((SDL_Rect){sr.x + 1, sr.y + 1, CELL - 2, CELL - 2}, 0, 35, 65, 40);

            SC(CY_R, CY_G, CY_B, 100);
            for (int dx = sr.x + off; dx < sr.x + CELL; dx += dash * 2)
                SDL_RenderDrawLine(g_ren, dx, sr.y, SDL_min(dx + dash, sr.x + CELL), sr.y);
            for (int dx = sr.x + off; dx < sr.x + CELL; dx += dash * 2)
                SDL_RenderDrawLine(g_ren, dx, sr.y + CELL, SDL_min(dx + dash, sr.x + CELL), sr.y + CELL);
            for (int dy = sr.y + off; dy < sr.y + CELL; dy += dash * 2)
                SDL_RenderDrawLine(g_ren, sr.x, dy, sr.x, SDL_min(dy + dash, sr.y + CELL));
            for (int dy = sr.y + off; dy < sr.y + CELL; dy += dash * 2)
                SDL_RenderDrawLine(g_ren, sr.x + CELL, dy, sr.x + CELL, SDL_min(dy + dash, sr.y + CELL));

            {
                char sn[4]; snprintf(sn, 4, "%d", s + 1);
                TC(sn, sr, CY_R, CY_G, CY_B, g_fntTiny);
            }

            if (g_hintGlow > 0 && g_hintSlot == s) {
                float ht = (float)g_hintGlow / 280.0f;
                float ap = 0.5f + 0.5f * sinf((float)now / 150.0f);
                FR((SDL_Rect){sr.x + 1, sr.y + 1, CELL - 2, CELL - 2}, AM_R, AM_G, AM_B, (Uint8)(ap * ht * 80));
                SR(sr, AM_R, AM_G, AM_B, (Uint8)(ht * 235), 2);
                GL(sr, AM_R, AM_G, AM_B, ht * 1.2f);
            }
        }
    }
    (void)dragging;
}

/* ─────────────────────────────────────────────────────────────────────────
   RIGHT SYSTEM PANEL
   ───────────────────────────────────────────────────────────────────────── */
static void renderSysPanel(Uint32 now, float timerFrac, int correctCount,
                            int hintsLeft, SDL_Rect hintBtn, int mx, int my,
                            Uint8 tR, Uint8 tG)
{
    SDL_Rect rp = { 756, 42, 136, 420 };
    FR(rp, PNL_R, PNL_G, PNL_B, 192);
    SR(rp, CY_R, CY_G, CY_B, 80, 1);
    CO(rp, CY_R, CY_G, CY_B, 150, 10);
    SC(PU_R, PU_G, PU_B, 45);
    SDL_RenderDrawLine(g_ren, 757, 43, 891, 43);

    FR((SDL_Rect){756, 42, 136, 18}, 0, 28, 62, 225);
    TL("SYS LOG", 760, 44, CY_R, CY_G, CY_B, g_fntSm);

    {
        SDL_Rect bg = { 762, 68, 120, 10 };
        SDL_Rect fg = { 762, 68, (int)(120 * correctCount / GPIECES), 10 };
        FR(bg, 0, 18, 38, 160); FR(fg, GR_R, GR_G, GR_B, 210); SR(bg, CY_R, CY_G, CY_B, 50, 1);
        TL("INTEGRITY", 762, 80, GR_R, GR_G, GR_B, g_fntTiny);
    }

    {
        SDL_Rect bg = { 762, 95, 120, 10 };
        SDL_Rect fg = { 762, 95, (int)(120 * timerFrac), 10 };
        FR(bg, 0, 18, 38, 160); FR(fg, tR, tG, 40, 200); SR(bg, CY_R, CY_G, CY_B, 50, 1);
        TL("TIMER", 762, 107, AM_R, AM_G, AM_B, g_fntTiny);
    }

    SC(CY_R, CY_G, CY_B, 35);
    SDL_RenderDrawLine(g_ren, 762, 120, 876, 120);

    int hHov    = (mx >= hintBtn.x && mx <= hintBtn.x + hintBtn.w &&
                   my >= hintBtn.y && my <= hintBtn.y + hintBtn.h);
    int hActive = (hintsLeft > 0);
    if (hHov && hActive) { GL(hintBtn, AM_R, AM_G, AM_B, 0.9f); FR(hintBtn, 50, 35, 0, 235); }
    else                   FR(hintBtn, 0, 18, 38, 210);
    SR(hintBtn, hActive ? 255 : 70, hActive ? 160 : 70, 0, 230, hActive ? 2 : 1);
    CO(hintBtn, AM_R, AM_G, AM_B, hActive ? 225 : 80, 10);

    {
        float lp = 0.5f + 0.5f * sinf((float)now / 350.0f);
        Uint8 la = hActive ? (Uint8)(160 + lp * 95) : 60;
        SC(255, 160, 0, la);
        SDL_Rect bulb = { hintBtn.x + 6, hintBtn.y + 6, 16, 16 };
        SDL_RenderDrawRect(g_ren, &bulb);
        FR((SDL_Rect){hintBtn.x + 10, hintBtn.y + 24, 8, 8}, 255, 160, 0, la);
        SDL_RenderDrawLine(g_ren, hintBtn.x + 14, hintBtn.y + 3,  hintBtn.x + 14, hintBtn.y + 1);
        SDL_RenderDrawLine(g_ren, hintBtn.x + 3,  hintBtn.y + 10, hintBtn.x + 1,  hintBtn.y + 9);
        SDL_RenderDrawLine(g_ren, hintBtn.x + 25, hintBtn.y + 10, hintBtn.x + 27, hintBtn.y + 9);
    }
    TC("DECRYPT [H]",
       (SDL_Rect){hintBtn.x + 28, hintBtn.y, hintBtn.w - 28, hintBtn.h},
       AM_R, AM_G, AM_B, g_fntMid);

    TL("CHARGES:", (int)(hintBtn.x), hintBtn.y + hintBtn.h + 6, CY_R, CY_G, CY_B, g_fntTiny);
    for (int i = 0; i < MAX_HINTS; i++) {
        int      cx2    = hintBtn.x + i * 36, cy2 = hintBtn.y + hintBtn.h + 16;
        int      active = (i < hintsLeft);
        float    cp     = 0.4f + 0.4f * sinf((float)now / 400.0f + i * 1.1f);
        SDL_Rect crystal = { cx2, cy2, 30, 16 };
        if (active) { GL(crystal, PU_R, PU_G, PU_B, cp * 0.8f); FR(crystal, 40, 0, 80, 210); }
        else          FR(crystal, 10, 10, 20, 180);
        SR(crystal, active ? PU_R : 40, active ? PU_G : 20, active ? PU_B : 40, active ? 160 : 60, 1);
        if (active) {
            SC(PU_R, PU_G, PU_B, (Uint8)(cp * 180));
            SDL_RenderDrawLine(g_ren, cx2 + 15, cy2 + 2,  cx2 + 28, cy2 + 8);
            SDL_RenderDrawLine(g_ren, cx2 + 28, cy2 + 8,  cx2 + 15, cy2 + 14);
            SDL_RenderDrawLine(g_ren, cx2 + 15, cy2 + 14, cx2 + 2,  cy2 + 8);
            SDL_RenderDrawLine(g_ren, cx2 + 2,  cy2 + 8,  cx2 + 15, cy2 + 2);
        }
    }

    SDL_Rect hintDisplay = { 758, 248, 130, 30 };
    FR(hintDisplay, 0, 14, 30, 200);
    SR(hintDisplay, CY_R, CY_G, CY_B, 55, 1);
    if (g_hintGlow > 0) {
        drawHintCipher(now, hintDisplay);
        TL("ANALYZING...", hintDisplay.x + 4, hintDisplay.y + hintDisplay.h + 4, 255, 160, 0, g_fntTiny);
    } else {
        TC("— AWAITING —", hintDisplay, CY_R, CY_G, CY_B, g_fntTiny);
    }

    SC(CY_R, CY_G, CY_B, 28);
    SDL_RenderDrawLine(g_ren, 762, 300, 876, 300);

    TL("PLACEMENT LOG:", 762, 306, CY_R, CY_G, CY_B, g_fntTiny);
    {
        char log1[32], log2[32];
        snprintf(log1, 32, "FILLED:    %d", correctCount);
        snprintf(log2, 32, "REMAINING: %d", GPIECES - correctCount);
        TL(log1, 762, 318, GR_R, GR_G, GR_B, g_fntTiny);
        TL(log2, 762, 330, AM_R, AM_G, AM_B, g_fntTiny);
    }

    { float bp = 0.5f + 0.5f * sinf((float)now / 700.0f); BAT(822, 370, 10, CY_R, CY_G, CY_B, (Uint8)(40 + bp * 50)); }
    { float bp = 0.5f + 0.5f * sinf((float)now / 900.0f + 0.5f); BAT(808, 390, 7, PU_R, PU_G, PU_B, (Uint8)(30 + bp * 40)); }
    { float bp = 0.5f + 0.5f * sinf((float)now / 600.0f + 1.0f); BAT(836, 390, 7, PU_R, PU_G, PU_B, (Uint8)(30 + bp * 40)); }
}

/* ─────────────────────────────────────────────────────────────────────────
   REPO STRIP
   ───────────────────────────────────────────────────────────────────────── */
static void renderRepo(Uint32 now, Piece *pieces, int dragging, int repoStartX) {
    int      totalW  = GPIECES * (RPW + RGAP) - RGAP;
    SDL_Rect repoBg  = { repoStartX - 12, REPO_Y - 14, totalW + 24, RPH + 38 };
    FR(repoBg, PNL_R, PNL_G, PNL_B, 200);
    SR(repoBg, CY_R, CY_G, CY_B, 100, 2);
    CO(repoBg, CY_R, CY_G, CY_B, 180, 10);
    SC(PU_R, PU_G, PU_B, 38);
    SDL_RenderDrawLine(g_ren, repoBg.x + 1, repoBg.y + 1, repoBg.x + repoBg.w - 1, repoBg.y + 1);

    FR((SDL_Rect){repoBg.x, repoBg.y + repoBg.h - 18, repoBg.w, 18}, 0, 22, 50, 215);
    SC(CY_R, CY_G, CY_B, 65);
    SDL_RenderDrawLine(g_ren, repoBg.x, repoBg.y + repoBg.h - 18,
                               repoBg.x + repoBg.w, repoBg.y + repoBg.h - 18);
    TC("PIECE REPOSITORY — DRAG TO GRID — REARRANGE FREELY",
       (SDL_Rect){repoBg.x, repoBg.y + repoBg.h - 18, repoBg.w, 18},
       CY_R, CY_G, CY_B, g_fntSm);

    for (int i = 0; i < GPIECES; i++) {
        if (pieces[i].slot >= 0 || i == dragging) continue;
        SDL_Rect r = pieces[i].cur;
        r.x += (int)pieces[i].shakeX;

        int isHinted = (g_hintGlow > 0 && i == g_hintPiece);
        if (isHinted) {
            float ht = (float)g_hintGlow / 280.0f;
            float ap = 0.5f + 0.5f * sinf((float)now / 160.0f);
            GL(r, AM_R, AM_G, AM_B, ap * ht * 2.5f);
            SR(r, AM_R, AM_G, AM_B, (Uint8)(ht * 240), 2);
            CO(r, AM_R, AM_G, AM_B, (Uint8)(ht * 255), 8);
        } else {
            SR(r, CY_R, CY_G, CY_B, 65, 1);
        }

        FR(r, 0, 8, 22, 210);
        drawSlice(pieces[i].idx, r, 250);
        SL(r, 16);

        SDL_Rect nt  = { r.x + r.w / 2 - 6, r.y - 4,        12,  4 };
        SDL_Rect nb  = { r.x + r.w / 2 - 6, r.y + r.h,      12,  4 };
        SDL_Rect nl  = { r.x - 4,            r.y + r.h / 2 - 6, 4, 12 };
        SDL_Rect nr2 = { r.x + r.w,          r.y + r.h / 2 - 6, 4, 12 };
        FR(nt,  0, 12, 30, 180); SR(nt,  CY_R, CY_G, CY_B, 55, 1);
        FR(nb,  0, 12, 30, 180); SR(nb,  CY_R, CY_G, CY_B, 55, 1);
        FR(nl,  0, 12, 30, 180); SR(nl,  CY_R, CY_G, CY_B, 55, 1);
        FR(nr2, 0, 12, 30, 180); SR(nr2, CY_R, CY_G, CY_B, 55, 1);

        if (pieces[i].shakeX != 0)
            FR(r, 255, 0, 0, (Uint8)(fabsf(pieces[i].shakeX) * 6));
    }
}

/* ─────────────────────────────────────────────────────────────────────────
   BACK BUTTON
   ───────────────────────────────────────────────────────────────────────── */
static void renderBackBtn(int hov) {
    SDL_Rect r = { 8, 471, 72, 28 };
    if (hov) { GL(r, GR_R, GR_G, GR_B, 0.7f); FR(r, 0, 34, 14, 235); }
    else       FR(r, 0, 16, 10, 205);
    SR(r, 0, hov ? 220 : 110, 40, hov ? 220 : 110, hov ? 2 : 1);
    CO(r, 0, hov ? 255 : 130, 48, hov ? 255 : 130, 6);
    BAT(r.x - 11, r.y + r.h / 2, 8, 0, 180, 60, 185);
    TC("BACK", r, 0, hov ? 255 : 180, 60, g_fntMid);
}

/* ─────────────────────────────────────────────────────────────────────────
   RUN PUZZLE
   ───────────────────────────────────────────────────────────────────────── */
int run_puzzle(SDL_Window *win, int puzzleIdx) {
    (void)win;

    /* Load background */
    SDL_Texture *bgTex = IMG_LoadTexture(g_ren, "bg.png");
    if (!bgTex) {
        bgTex = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_RGBA8888,
                                  SDL_TEXTUREACCESS_TARGET, WIN_W, WIN_H);
        SDL_SetRenderTarget(g_ren, bgTex);
        SDL_SetRenderDrawColor(g_ren, 0, 4, 12, 255);
        SDL_RenderClear(g_ren);
        for (int y = 0; y < WIN_H; y++) {
            float t = (float)y / WIN_H; Uint8 b = (Uint8)(4 + t * 20);
            SDL_SetRenderDrawColor(g_ren, 0, b / 3, b, 255);
            SDL_RenderDrawLine(g_ren, 0, y, WIN_W, y);
        }
        SDL_SetRenderDrawColor(g_ren, 0, 50, 80, 40);
        for (int y = 0; y < WIN_H; y += 28)
            for (int x = 0; x < WIN_W; x += 28)
                SDL_RenderDrawPoint(g_ren, x, y);
        SDL_SetRenderTarget(g_ren, NULL);
    }

    /* Load puzzle image */
    g_puzzTex = IMG_LoadTexture(g_ren, PUZZLE_IMAGES[puzzleIdx]);
    if (g_puzzTex)
        SDL_QueryTexture(g_puzzTex, NULL, NULL, &g_puzzW, &g_puzzH);
    else {
        g_puzzTex = makePuzzlePlaceholder(puzzleIdx);
        SDL_Log("'%s' missing — using placeholder", PUZZLE_IMAGES[puzzleIdx]);
    }

    initDust();
    sparkCount = 0;

    /* Grid slots */
    SDL_Rect gridSlot[GPIECES];
    for (int i = 0; i < GPIECES; i++)
        gridSlot[i] = (SDL_Rect){ GRID_X + (i % GCOLS) * CELL, GRID_Y + (i / GCOLS) * CELL, CELL, CELL };

    int slotOcc[GPIECES];
    for (int i = 0; i < GPIECES; i++) slotOcc[i] = -1;

    int order[GPIECES];
    for (int i = 0; i < GPIECES; i++) order[i] = i;
    shuffle(order, GPIECES);

    int totalW     = GPIECES * (RPW + RGAP) - RGAP;
    int repoStartX = (WIN_W - totalW) / 2;

    /* Build pieces */
    Piece pieces[GPIECES];
    for (int i = 0; i < GPIECES; i++) {
        int rx = repoStartX + i * (RPW + RGAP);
        pieces[i].idx      = order[i];
        pieces[i].slot     = -1;
        pieces[i].home     = (SDL_Rect){ rx, REPO_Y, RPW, RPH };
        pieces[i].cur      = pieces[i].home;
        pieces[i].shakeX   = 0; pieces[i].shakeTmr = 0;
        pieces[i].flashT   = 0; pieces[i].flashOk  = 0;
    }

    /* Game state */
    GameState state        = ST_PLAY;
    Uint32    startT       = SDL_GetTicks();
    int       hintsLeft    = MAX_HINTS;
    int       dragging     = -1, dragOX = 0, dragOY = 0;
    int       correctCount = 0;
    float     endProgress  = 0.0f;
    float     winWaveT     = 0.0f;
    int       wantRestart  = 0, wantBack = 0;
    int       result       = 0;

    g_hintGlow = 0; g_hintSlot = -1; g_hintPiece = -1; g_hintCipher[0] = 0;

    SDL_Rect hintBtn = { 760, 170, 122, 40 };
    SDL_Rect backBtn = {   8, 471,  72, 28 };

    SDL_Event ev;
    Uint32 lastTick = SDL_GetTicks();
    int    running  = 1;

    while (running) {
        Uint32 now = SDL_GetTicks();
        float  dt  = (float)(now - lastTick) / 1000.0f; if (dt > 0.05f) dt = 0.05f;
        lastTick = now;

        int mouseClicked = 0, mx, my;
        SDL_GetMouseState(&mx, &my);

        /* Events */
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { result = -1; running = 0; }
            if (ev.type == SDL_KEYDOWN) {
                if (ev.key.keysym.sym == SDLK_ESCAPE) { result = 0; running = 0; }
                if (ev.key.keysym.sym == SDLK_r && state != ST_PLAY) wantRestart = 1;
                if (ev.key.keysym.sym == SDLK_h && state == ST_PLAY)
                    triggerHint(pieces, slotOcc, &hintsLeft);
            }

            if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
                mouseClicked = 1;
                int bx = ev.button.x, by = ev.button.y;

                if (state == ST_PLAY &&
                    bx >= hintBtn.x && bx <= hintBtn.x + hintBtn.w &&
                    by >= hintBtn.y && by <= hintBtn.y + hintBtn.h)
                    triggerHint(pieces, slotOcc, &hintsLeft);

                if (state == ST_PLAY && dragging < 0) {
                    for (int i = 0; i < GPIECES; i++) {
                        if (pieces[i].slot >= 0) continue;
                        SDL_Rect r = pieces[i].cur;
                        if (bx >= r.x && bx <= r.x + r.w && by >= r.y && by <= r.y + r.h) {
                            dragging = i; dragOX = bx - r.x; dragOY = by - r.y; break;
                        }
                    }
                    if (dragging < 0) {
                        for (int s = 0; s < GPIECES; s++) {
                            int pi = slotOcc[s]; if (pi < 0) continue;
                            SDL_Rect r = pieces[pi].cur;
                            if (bx >= r.x && bx <= r.x + r.w && by >= r.y && by <= r.y + r.h) {
                                slotOcc[s] = -1;
                                pieces[pi].slot = -1;
                                dragging = pi; dragOX = bx - r.x; dragOY = by - r.y;
                                correctCount = countCorrect(pieces, slotOcc);
                                break;
                            }
                        }
                    }
                }
            }

            if (ev.type == SDL_MOUSEMOTION && dragging >= 0) {
                pieces[dragging].cur.x = ev.motion.x - dragOX;
                pieces[dragging].cur.y = ev.motion.y - dragOY;
            }

            if (ev.type == SDL_MOUSEBUTTONUP && ev.button.button == SDL_BUTTON_LEFT && dragging >= 0) {
                int   pcx  = pieces[dragging].cur.x + RPW / 2;
                int   pcy  = pieces[dragging].cur.y + RPH / 2;
                int   best = -1;
                float bestD = 1e9f;

                for (int s = 0; s < GPIECES; s++) {
                    if (slotOcc[s] >= 0) continue;
                    float dx = (float)(pcx - (gridSlot[s].x + CELL / 2));
                    float dy = (float)(pcy - (gridSlot[s].y + CELL / 2));
                    float d  = dx * dx + dy * dy;
                    if (d < (float)(CELL * CELL) * 0.55f && d < bestD) { bestD = d; best = s; }
                }

                if (best >= 0) {
                    slotOcc[best]          = dragging;
                    pieces[dragging].slot  = best;
                    pieces[dragging].cur   = (SDL_Rect){ gridSlot[best].x + 1, gridSlot[best].y + 1, CELL - 2, CELL - 2 };
                    int wasCorrect         = (pieces[dragging].idx == best);
                    pieces[dragging].flashT  = 0.8f;
                    pieces[dragging].flashOk = wasCorrect;
                    spawnSparks(gridSlot[best].x + CELL / 2, gridSlot[best].y + CELL / 2, wasCorrect);
                    correctCount = countCorrect(pieces, slotOcc);
                    if (correctCount == GPIECES) { state = ST_WIN; winWaveT = 0; }
                    printf("[BACKEND] Piece %d → slot %d — %s (%d/9)\n",
                           pieces[dragging].idx, best, wasCorrect ? "OK" : "WRONG", correctCount);
                } else {
                    pieces[dragging].cur = pieces[dragging].home;
                }
                dragging = -1;
            }
        }

        /* Restart / Back */
        if (wantRestart) {
            state = ST_PLAY; startT = SDL_GetTicks(); hintsLeft = MAX_HINTS;
            g_hintGlow = 0; g_hintSlot = -1; g_hintPiece = -1; g_hintCipher[0] = 0;
            dragging = -1; correctCount = 0; endProgress = 0; winWaveT = 0;
            wantRestart = 0; wantBack = 0; sparkCount = 0;
            for (int i = 0; i < GPIECES; i++) slotOcc[i] = -1;
            shuffle(order, GPIECES);
            for (int i = 0; i < GPIECES; i++) {
                int rx = repoStartX + i * (RPW + RGAP);
                pieces[i].idx    = order[i]; pieces[i].slot = -1;
                pieces[i].home   = (SDL_Rect){ rx, REPO_Y, RPW, RPH };
                pieces[i].cur    = pieces[i].home;
                pieces[i].shakeX = 0; pieces[i].shakeTmr = 0;
                pieces[i].flashT = 0;
            }
        }
        if (wantBack) { result = 0; running = 0; }

        /* Timer & state */
        Uint32 elapsed    = now - startT;
        float  timerFrac  = (state == ST_PLAY) ? 1.0f - (float)elapsed / TIMER_MS
                                                : (correctCount == GPIECES ? 1.0f : 0.0f);
        if (timerFrac < 0) timerFrac = 0;
        if (state == ST_PLAY && elapsed >= TIMER_MS) state = ST_LOSE;
        if (state != ST_PLAY && endProgress < 1.0f) endProgress += dt * 0.65f;
        if (endProgress > 1.0f) endProgress = 1.0f;
        if (state == ST_WIN) winWaveT += dt;

        Uint8 tR = (Uint8)(255 * (1 - timerFrac));
        Uint8 tG = (Uint8)(210 * timerFrac);

        /* Update pieces */
        for (int i = 0; i < GPIECES; i++) {
            if (pieces[i].shakeTmr > 0) {
                pieces[i].shakeTmr -= dt * 60;
                pieces[i].shakeX    = sinf(pieces[i].shakeTmr * 1.4f) * 7.0f * (pieces[i].shakeTmr / 22.0f);
            } else {
                pieces[i].shakeX = 0;
            }
            if (pieces[i].flashT > 0) pieces[i].flashT -= dt;
        }
        if (g_hintGlow > 0) g_hintGlow--;

        updateSparks(dt);

        /* Render */
        SDL_SetRenderDrawColor(g_ren, 0, 0, 0, 255);
        SDL_RenderClear(g_ren);

        SDL_RenderCopy(g_ren, bgTex, NULL, &(SDL_Rect){0, 0, WIN_W, WIN_H});
        FR((SDL_Rect){0, 0, WIN_W, WIN_H}, 0, 6, 16, 100);

        drawCircuits(now);
        drawDust(now);
        renderTopBar(now, startT, timerFrac, state, elapsed, tR, tG);
        renderRefPanel(now, correctCount);
        renderGridPanel(now, pieces, slotOcc, gridSlot, state, winWaveT, dragging);
        renderSysPanel(now, timerFrac, correctCount, hintsLeft, hintBtn, mx, my, tR, tG);
        renderRepo(now, pieces, dragging, repoStartX);

        /* Dragged piece */
        if (dragging >= 0) {
            SDL_Rect r = pieces[dragging].cur;
            GL(r, CY_R, CY_G, CY_B, 1.6f);
            FR(r, 0, 28, 55, 210);
            drawSlice(pieces[dragging].idx, r, 248);
            SL(r, 12);
            SR(r, CY_R, CY_G, CY_B, 255, 2);
            CO(r, CY_R, CY_G, CY_B, 255, 10);
            if (rand() % 3 == 0 && sparkCount < MAX_SPARKS) {
                Spark *sp = &sparks[sparkCount++];
                sp->x = (float)(r.x + rand() % r.w); sp->y = (float)(r.y + rand() % r.h);
                sp->vx = ((float)(rand() % 100) - 50) / 80.0f;
                sp->vy = -((float)(rand() % 100)) / 80.0f;
                sp->life = sp->maxLife = 0.3f; sp->r = 0; sp->g = 200; sp->b = 255;
            }
        }

        drawSparks();

        {
            int backHov = (mx >= backBtn.x && mx <= backBtn.x + backBtn.w &&
                           my >= backBtn.y && my <= backBtn.y + backBtn.h);
            renderBackBtn(backHov);
            if (backHov && mouseClicked) { result = 0; running = 0; }
        }

        /* Bottom status bar */
        FR((SDL_Rect){0, WIN_H - 18, WIN_W, 18}, 0, 6, 18, 225);
        SC(CY_R, CY_G, CY_B, 42);
        SDL_RenderDrawLine(g_ren, 0, WIN_H - 18, WIN_W, WIN_H - 18);
        SC(PU_R, PU_G, PU_B, 22);
        SDL_RenderDrawLine(g_ren, 0, WIN_H - 19, WIN_W, WIN_H - 19);
        TL("DRAG ALL 9 FRAGMENTS TO THE GRID — REARRANGE FREELY — BACKEND VALIDATES EACH FRAME",
           10, WIN_H - 14, CY_R, 120, CY_B, g_fntSm);
        {
            float sp = 0.5f + 0.5f * sinf((float)now / 650.0f);
            SC(CY_R, CY_G, CY_B, (Uint8)(70 + sp * 120));
            int dx = WIN_W - 16, dy = WIN_H - 9;
            SDL_RenderDrawLine(g_ren, dx,     dy - 6, dx + 4, dy    );
            SDL_RenderDrawLine(g_ren, dx + 4, dy,     dx,     dy + 6);
            SDL_RenderDrawLine(g_ren, dx,     dy + 6, dx - 4, dy    );
            SDL_RenderDrawLine(g_ren, dx - 4, dy,     dx,     dy - 6);
        }

        if (state != ST_PLAY)
            renderEndOverlay(state, endProgress, now, &wantRestart, &wantBack, mx, my, mouseClicked);

        SDL_RenderPresent(g_ren);
        SDL_Delay(8);
    }

    SDL_DestroyTexture(bgTex);
    if (g_puzzTex) { SDL_DestroyTexture(g_puzzTex); g_puzzTex = NULL; }

    return result;
}

