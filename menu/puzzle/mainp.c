/*
 * main.c — TACTICAL DECRYPTION ENGINE
 * =====================================
 * Entry point + Main Selection Screen
 *
 * COMPILE:  make
 */

#include "header.h"

/* ─────────────────────────────────────────────────────────────────────────
   GLOBAL RENDERER / FONT CONTEXT  (defined here, extern'd in header)
   ───────────────────────────────────────────────────────────────────────── */
SDL_Renderer *g_ren    = NULL;
TTF_Font     *g_fntBig  = NULL;
TTF_Font     *g_fntMid  = NULL;
TTF_Font     *g_fntSm   = NULL;
TTF_Font     *g_fntTiny = NULL;

/* ─────────────────────────────────────────────────────────────────────────
   MENU LAYOUT CONSTANTS
   ───────────────────────────────────────────────────────────────────────── */
#define MAIN_X   204
#define MAIN_Y    56
#define MAIN_W   492
#define MAIN_H   274

#define CARD_Y   352
#define CARD_H   120
#define CARD_W   169
#define CARD_A_X  42
#define CARD_B_X 366
#define CARD_C_X 690

#define BACK_X    21
#define BACK_Y   464
#define BACK_W    77
#define BACK_H    28

#define ACCENT    15

/* ─────────────────────────────────────────────────────────────────────────
   CARD  (one selectable puzzle entry)
   ───────────────────────────────────────────────────────────────────────── */
typedef struct {
    SDL_Rect     rect;
    SDL_Texture *img;
    int          puzzleIdx;   /* which puzzle this card launches */
    float        hoverT;
    int          clicked;     /* countdown frames for click flash */
} Card;

/* ─────────────────────────────────────────────────────────────────────────
   HELPERS
   ───────────────────────────────────────────────────────────────────────── */
static SDL_Texture *loadTex(const char *path){
    SDL_Texture *t = IMG_LoadTexture(g_ren, path);
    if(!t) SDL_Log("Cannot load '%s': %s", path, IMG_GetError());
    return t;
}

static SDL_Texture *makePlaceholder(int w, int h, Uint8 R, Uint8 G, Uint8 B){
    SDL_Texture *t = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_TARGET, w, h);
    if(!t) return NULL;
    SDL_SetRenderTarget(g_ren, t);
    SDL_SetRenderDrawColor(g_ren, R, G, B, 255);
    SDL_RenderClear(g_ren);
    SDL_SetRenderDrawColor(g_ren, 0, 60, 90, 120);
    for(int x=0;x<w;x+=28) SDL_RenderDrawLine(g_ren,x,0,x,h);
    for(int y=0;y<h;y+=28) SDL_RenderDrawLine(g_ren,0,y,w,y);
    SDL_SetRenderTarget(g_ren, NULL);
    return t;
}

/* ─────────────────────────────────────────────────────────────────────────
   RENDER HELPERS
   ───────────────────────────────────────────────────────────────────────── */
static void renderCard(Card *c, Uint32 ticks){
    SDL_Rect r = c->rect;
    float    h = c->hoverT;
    int  clk   = c->clicked;

    /* Glow layer */
    if(h > 0.01f)
        GL(r, clk>0?AM_R:CY_R, clk>0?AM_G:CY_G, clk>0?AM_B:CY_B, h);

    /* Panel fill — slightly brightens on hover */
    FR(r, PNL_R, PNL_G, PNL_B, (Uint8)(180+(int)(h*40)));

    /* Thumbnail image */
    if(c->img){
        SDL_Rect imgDst={r.x+2,r.y+2,r.w-4,r.h-4};
        Uint8 mod=(Uint8)(155+(int)(h*100));
        SDL_SetTextureColorMod(c->img,mod,mod,mod);
        SDL_RenderCopy(g_ren,c->img,NULL,&imgDst);
        SDL_SetTextureColorMod(c->img,255,255,255);
        SL(imgDst,40);
    }

    /* Click amber flash overlay */
    if(clk>0){ Uint8 fa=(Uint8)(clk*6); FR(r,AM_R,AM_G,0,fa); }

    /* Border */
    int   thick = (int)(1+h*3);
    Uint8 bA    = (Uint8)(120+(int)(h*135));
    if(clk>0) SR(r,AM_R,AM_G,0,bA,thick);
    else       SR(r,CY_R,CY_G,CY_B,bA,thick);

    /* Corner accents */
    Uint8 cA=(Uint8)(80+(int)(h*175));
    if(clk>0) CO(r,AM_R,AM_G,0,255,ACCENT);
    else       CO(r,CY_R,CY_G,CY_B,cA,ACCENT);

    /* Hover scan line sweep */
    if(h>0.3f){
        float sweep=fmodf((float)ticks/800.0f,1.0f);
        int   sy   =r.y+(int)(sweep*r.h);
        SC(CY_R,CY_G,CY_B,(Uint8)(h*80));
        SDL_RenderDrawLine(g_ren,r.x,sy,r.x+r.w,sy);
    }

    /* Puzzle index badge — bottom-right corner */
    {
        char badge[8]; snprintf(badge,sizeof(badge),"%d",(c->puzzleIdx&0xFF)+1);
        SDL_Rect badgeR={r.x+r.w-32,r.y+r.h-18,28,14};
        FR(badgeR,0,10,25,200);
        SR(badgeR,CY_R,CY_G,CY_B,60,1);
        TC(badge,badgeR,CY_R,CY_G,CY_B,g_fntTiny);
    }
}

/* ── Title bar ────────────────────────────────────────────────────────── */
static void renderTitleBar(Uint32 ticks){
    FR((SDL_Rect){0,0,WIN_W,36}, 0,8,20,230);
    SC(CY_R,CY_G,CY_B,60);
    SDL_RenderDrawLine(g_ren,0,36,WIN_W,36);

    float pulse=0.5f+0.5f*sinf((float)ticks/700.0f);
    Uint8 pa=(Uint8)(80+pulse*80);
    SC(CY_R,CY_G,CY_B,pa);
    SDL_RenderDrawLine(g_ren,14,6,14,30);
    SDL_RenderDrawLine(g_ren,14,6,98,6);
    SDL_RenderDrawLine(g_ren,WIN_W-14,6,WIN_W-14,30);
    SDL_RenderDrawLine(g_ren,WIN_W-14,6,WIN_W-98,6);

    SDL_Rect titleR={0,3,WIN_W,30};
    TC("SELECT PUZZLE",titleR,CY_R,CY_G,CY_B,g_fntBig);

    Uint8 dotA=(Uint8)(180+75*sinf((float)ticks/400.0f));
    FR((SDL_Rect){WIN_W-12,14,6,6},0,255,100,dotA);
    BAT(WIN_W-22,19,11,CY_R,CY_G,CY_B,80);
}

/* ── Divider between main card and side cards ─────────────────────────── */
static void renderDivider(Uint32 ticks){
    float pulse=0.5f+0.5f*sinf((float)ticks/900.0f);
    Uint8 lineA=(Uint8)(40+pulse*30);
    SDL_Rect div={MAIN_X, MAIN_Y+MAIN_H+6, MAIN_W, 12};
    FR(div,0,22,50,180);
    SC(CY_R,CY_G,CY_B,lineA);
    SDL_RenderDrawLine(g_ren,div.x+10,div.y+6,div.x+div.w-10,div.y+6);
    CO(div,CY_R,CY_G,CY_B,(Uint8)(60+pulse*40),6);
}

/* ── Right status panel ───────────────────────────────────────────────── */
static void renderRightPanel(Uint32 ticks){
    SDL_Rect panel={WIN_W-176,46,168,226};
    FR(panel,PNL_R,PNL_G,PNL_B,200);
    SR(panel,CY_R,CY_G,CY_B,80,1);
    CO(panel,CY_R,CY_G,CY_B,160,10);

    /* Header strip */
    FR((SDL_Rect){panel.x,panel.y,panel.w,16},0,35,70,220);
    TL("STATUS",panel.x+6,panel.y+2,CY_R,CY_G,CY_B,g_fntSm);

    /* Animated rings */
    float p=0.5f+0.5f*sinf((float)ticks/500.0f);
    int cx=panel.x+panel.w/2, cy=panel.y+110;
    for(int ring=1;ring<=3;ring++){
        Uint8 ra=(Uint8)(p*160/ring);
        SC(CY_R,CY_G,CY_B,ra);
        SDL_Rect rr={cx-ring*14,cy-ring*10,ring*28,ring*20};
        SDL_RenderDrawRect(g_ren,&rr);
    }
    FR((SDL_Rect){cx-3,cy-3,6,6},CY_R,CY_G,CY_B,200);

    /* Online dot + label */
    Uint8 sa=(Uint8)(180+p*75);
    FR((SDL_Rect){panel.x+8,panel.y+188,6,6},0,sa,80,sa);
    TL("ONLINE",panel.x+20,panel.y+186,0,sa,80,g_fntSm);

    /* Horizontal separators */
    SC(CY_R,CY_G,CY_B,40);
    SDL_RenderDrawLine(g_ren,panel.x+6,panel.y+70,panel.x+panel.w-6,panel.y+70);
    SDL_RenderDrawLine(g_ren,panel.x+6,panel.y+150,panel.x+panel.w-6,panel.y+150);

    /* Bat deco */
    BAT(panel.x+panel.w/2, panel.y+panel.h-16, 12, CY_R,CY_G,CY_B,60);
}

/* ── Left info panel ──────────────────────────────────────────────────── */
static void renderLeftPanel(Uint32 ticks){
    SDL_Rect panel={7,46,140,126};
    FR(panel,PNL_R,PNL_G,PNL_B,180);
    SR(panel,CY_R,CY_G,CY_B,70,1);
    CO(panel,CY_R,CY_G,CY_B,140,9);

    /* Header strip */
    FR((SDL_Rect){panel.x,panel.y,panel.w,16},0,35,70,220);
    TL("SIGNAL",panel.x+4,panel.y+2,CY_R,CY_G,CY_B,g_fntSm);

    /* Radar rings */
    int cx=panel.x+panel.w/2, cy=panel.y+78;
    float p=0.5f+0.5f*sinf((float)ticks/400.0f);
    for(int ring=1;ring<=3;ring++){
        Uint8 ra=(Uint8)(p*180/ring);
        SC(CY_R,CY_G,CY_B,ra);
        SDL_Rect rr={cx-ring*8,cy-ring*6,ring*17,ring*11};
        SDL_RenderDrawRect(g_ren,&rr);
    }
    FR((SDL_Rect){cx-2,cy-2,4,4},CY_R,CY_G,CY_B,220);
}

/* ── Back button ──────────────────────────────────────────────────────── */
static void renderBack(int hovered, int clicked, Uint32 ticks){
    SDL_Rect r={BACK_X,BACK_Y,BACK_W,BACK_H};
    float pulse=0.5f+0.5f*sinf((float)ticks/500.0f);
    if(hovered||clicked){
        GL(r,0,200,60,hovered?0.6f:0.9f);
        FR(r,0,40,15,220);
        SR(r,0,255,80,220,2);
        CO(r,0,255,80,255,7);
        TC("BACK",r,0,255,80,g_fntBig);
    } else {
        FR(r,0,20,10,180);
        Uint8 ba=(Uint8)(120+pulse*80);
        SR(r,0,ba,40,180,1);
        TC("BACK",r,0,(Uint8)(180+pulse*75),50,g_fntBig);
    }
    BAT(BACK_X-15,BACK_Y+BACK_H/2,9,0,200,60,hovered?220:100);
}

/* ── Edge vignette ────────────────────────────────────────────────────── */
static void renderVignette(void){
    for(int i=0;i<56;i++){ Uint8 a=(Uint8)(56-i); SC(0,5,15,a); SDL_RenderDrawLine(g_ren,i,0,i,WIN_H); }
    for(int i=0;i<56;i++){ Uint8 a=(Uint8)(56-i); SC(0,5,15,a); SDL_RenderDrawLine(g_ren,WIN_W-1-i,0,WIN_W-1-i,WIN_H); }
    for(int i=0;i<42;i++){ Uint8 a=(Uint8)(42-i); SC(0,5,15,a); SDL_RenderDrawLine(g_ren,0,WIN_H-1-i,WIN_W,WIN_H-1-i); }
}

/* ─────────────────────────────────────────────────────────────────────────
   RUN MENU  — returns selected puzzleIdx or -1 to quit
   ───────────────────────────────────────────────────────────────────────── */
int run_menu(SDL_Window *win){
    (void)win;

    /* Load assets */
    SDL_Texture *bgTex   = loadTex("bg.png");
    if(!bgTex) bgTex     = makePlaceholder(WIN_W,WIN_H,0,8,18);

    SDL_Texture *imgs[NUM_PUZZLES];
    int plW[NUM_PUZZLES], plH[NUM_PUZZLES];
    plW[0]=MAIN_W; plH[0]=MAIN_H;
    plW[1]=plW[2]=plW[3]=CARD_W; plH[1]=plH[2]=plH[3]=CARD_H;
    Uint8 plC[NUM_PUZZLES][3]={{0,25,50},{10,40,20},{0,20,45},{35,10,40}};
    for(int i=0;i<NUM_PUZZLES;i++){
        imgs[i]=loadTex(PUZZLE_IMAGES[i]);
        if(!imgs[i]) imgs[i]=makePlaceholder(plW[i],plH[i],plC[i][0],plC[i][1],plC[i][2]);
    }

    /* Cards */
    Card cards[NUM_PUZZLES] = {
        { {MAIN_X, MAIN_Y, MAIN_W, MAIN_H}, imgs[0], 0, 0.0f, 0 },
        { {CARD_A_X, CARD_Y, CARD_W, CARD_H}, imgs[1], 1, 0.0f, 0 },
        { {CARD_B_X, CARD_Y, CARD_W, CARD_H}, imgs[2], 2, 0.0f, 0 },
        { {CARD_C_X, CARD_Y, CARD_W, CARD_H}, imgs[3], 3, 0.0f, 0 },
    };

    SDL_Rect backRect={BACK_X,BACK_Y,BACK_W,BACK_H};
    int backHover=0, backClicked=0;
    int selectedPuzzle=-1;
    int running=1;
    SDL_Event ev;
    float dt=1.0f/60.0f;

    while(running){
        Uint32 ticks=SDL_GetTicks();

        while(SDL_PollEvent(&ev)){
            if(ev.type==SDL_QUIT){ selectedPuzzle=-1; running=0; }
            if(ev.type==SDL_KEYDOWN&&ev.key.keysym.sym==SDLK_ESCAPE){ selectedPuzzle=-1; running=0; }

            if(ev.type==SDL_MOUSEBUTTONDOWN&&ev.button.button==SDL_BUTTON_LEFT){
                int mx=ev.button.x, my=ev.button.y;

                /* Back button */
                if(mx>=backRect.x&&mx<=backRect.x+backRect.w&&my>=backRect.y&&my<=backRect.y+backRect.h){
                    backClicked=20;
                    SDL_Delay(200);
                    selectedPuzzle=-1;
                    running=0;
                }

                /* Card click */
                for(int i=0;i<NUM_PUZZLES;i++){
                    SDL_Rect r=cards[i].rect;
                    if(mx>=r.x&&mx<=r.x+r.w&&my>=r.y&&my<=r.y+r.h){
                        cards[i].clicked=30;
                        /* Short flash then transition */
                        SDL_Delay(180);
                        selectedPuzzle=cards[i].puzzleIdx;
                        running=0;
                    }
                }
            }
        }

        /* Mouse hover */
        int mx,my; SDL_GetMouseState(&mx,&my);
        for(int i=0;i<NUM_PUZZLES;i++){
            SDL_Rect r=cards[i].rect;
            int over=(mx>=r.x&&mx<=r.x+r.w&&my>=r.y&&my<=r.y+r.h);
            float target=over?1.0f:0.0f;
            cards[i].hoverT+=(target-cards[i].hoverT)*8.0f*dt;
            if(cards[i].hoverT<0.01f) cards[i].hoverT=0.0f;
            if(cards[i].hoverT>0.99f) cards[i].hoverT=1.0f;
            if(cards[i].clicked>0) cards[i].clicked--;
        }
        backHover=(mx>=backRect.x&&mx<=backRect.x+backRect.w&&my>=backRect.y&&my<=backRect.y+backRect.h);
        if(backClicked>0) backClicked--;

        /* ════════════════ RENDER ════════════════ */
        SDL_SetRenderDrawColor(g_ren,0,0,0,255);
        SDL_RenderClear(g_ren);

        /* Background */
        SDL_RenderCopy(g_ren,bgTex,NULL,&(SDL_Rect){0,0,WIN_W,WIN_H});

        /* Dark tint */
        FR((SDL_Rect){0,0,WIN_W,WIN_H},0,8,20,155);

        /* Subtle grid lines */
        SC(CY_R,CY_G,CY_B,14);
        for(int y=0;y<WIN_H;y+=42)
            SDL_RenderDrawLine(g_ren,0,y,WIN_W,y);

        /* Cards */
        for(int i=0;i<NUM_PUZZLES;i++) renderCard(&cards[i],ticks);

        /* UI chrome */
        renderTitleBar(ticks);
        renderDivider(ticks);
        renderLeftPanel(ticks);
        renderRightPanel(ticks);
        renderBack(backHover,backClicked,ticks);
        renderVignette();

        /* Bottom bar */
        FR((SDL_Rect){0,WIN_H-20,WIN_W,20},0,8,20,220);
        SC(CY_R,CY_G,CY_B,50);
        SDL_RenderDrawLine(g_ren,0,WIN_H-20,WIN_W,WIN_H-20);
        TL("SELECT A PUZZLE TO BEGIN DECRYPTION",10,WIN_H-15,CY_R,120,CY_B,g_fntSm);
        BAT(WIN_W-22,WIN_H-10,7,CY_R,CY_G,CY_B,50);

        SDL_RenderPresent(g_ren);
        SDL_Delay(8);
    }

    /* Cleanup */
    SDL_DestroyTexture(bgTex);
    for(int i=0;i<NUM_PUZZLES;i++) SDL_DestroyTexture(imgs[i]);

    return selectedPuzzle;
}

/* ─────────────────────────────────────────────────────────────────────────
   APPLICATION ENTRY POINT
   ───────────────────────────────────────────────────────────────────────── */
int main(int argc, char *argv[]){
    (void)argc; (void)argv;
    srand((unsigned)time(NULL));

    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)<0){
        fprintf(stderr,"SDL_Init: %s\n",SDL_GetError()); return 1;
    }
    if(!(IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG)&(IMG_INIT_PNG|IMG_INIT_JPG))){
        fprintf(stderr,"IMG_Init failed\n"); SDL_Quit(); return 1;
    }
    if(TTF_Init()<0){
        fprintf(stderr,"TTF_Init: %s\n",TTF_GetError()); IMG_Quit(); SDL_Quit(); return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "TACTICAL DECRYPTION ENGINE",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H, SDL_WINDOW_SHOWN
    );
    if(!win){ fprintf(stderr,"CreateWindow: %s\n",SDL_GetError()); return 1; }

    g_ren = SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if(!g_ren){ fprintf(stderr,"CreateRenderer: %s\n",SDL_GetError()); return 1; }
    SDL_SetRenderDrawBlendMode(g_ren,SDL_BLENDMODE_BLEND);

    /* Fonts — try game font first, fall back to system */
    const char *bf  = "batmfa__.ttf";
    const char *fbo = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
    const char *fno = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";

    g_fntBig  = TTF_OpenFont(bf,22); if(!g_fntBig)  g_fntBig  = TTF_OpenFont(fbo,20);
    g_fntMid  = TTF_OpenFont(bf,14); if(!g_fntMid)  g_fntMid  = TTF_OpenFont(fbo,13);
    g_fntSm   = TTF_OpenFont(bf,10); if(!g_fntSm)   g_fntSm   = TTF_OpenFont(fno,9);
    g_fntTiny = TTF_OpenFont(bf,8);  if(!g_fntTiny) g_fntTiny = TTF_OpenFont(fno,8);

    /* Main loop: menu → puzzle → menu → … */
    int running = 1;
    while(running){
        int choice = run_menu(win);
        if(choice < 0){ running=0; break; }

        int result = run_puzzle(win, choice);
        if(result < 0) running=0;
        /* result == 0  →  go back to menu (loop continues) */
    }

    /* Teardown */
    if(g_fntBig)  TTF_CloseFont(g_fntBig);
    if(g_fntMid)  TTF_CloseFont(g_fntMid);
    if(g_fntSm)   TTF_CloseFont(g_fntSm);
    if(g_fntTiny) TTF_CloseFont(g_fntTiny);
    TTF_Quit();
    SDL_DestroyRenderer(g_ren);
    SDL_DestroyWindow(win);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
