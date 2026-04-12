/*
 * main.c — TACTICAL DECRYPTION ENGINE
 * Menu: no title bar box, clean "SELECT A PUZZLE" with zoom pulse,
 * no side panels.
 */

#include "header.h"

SDL_Renderer *g_ren     = NULL;
TTF_Font     *g_fntBig  = NULL;
TTF_Font     *g_fntMid  = NULL;
TTF_Font     *g_fntSm   = NULL;
TTF_Font     *g_fntTiny = NULL;
Mix_Music    *g_music   = NULL;

/* ─────────────────────────────────────────────────────────────────────────
   HELPERS
   ───────────────────────────────────────────────────────────────────────── */
static SDL_Texture *loadTex(const char *path){
    SDL_Texture *t=IMG_LoadTexture(g_ren,path);
    if(!t) SDL_Log("Cannot load '%s': %s",path,IMG_GetError());
    return t;
}
static SDL_Texture *makePlaceholder(int w,int h,Uint8 R,Uint8 G,Uint8 B){
    SDL_Texture *t=SDL_CreateTexture(g_ren,SDL_PIXELFORMAT_RGBA8888,
                                     SDL_TEXTUREACCESS_TARGET,w,h);
    if(!t)return NULL;
    SDL_SetRenderTarget(g_ren,t);
    SDL_SetRenderDrawColor(g_ren,R,G,B,255); SDL_RenderClear(g_ren);
    SDL_SetRenderDrawColor(g_ren,0,60,90,110);
    for(int x=0;x<w;x+=32) SDL_RenderDrawLine(g_ren,x,0,x,h);
    for(int y=0;y<h;y+=32) SDL_RenderDrawLine(g_ren,0,y,w,y);
    SDL_SetRenderTarget(g_ren,NULL);
    return t;
}

/* ─────────────────────────────────────────────────────────────────────────
   CARD
   ───────────────────────────────────────────────────────────────────────── */
static void renderCard(Card *c,Uint32 ticks){
    SDL_Rect r=c->rect;
    float    h=c->hoverT;
    int    clk=c->clicked;

    if(h>0.01f) GL(r,clk>0?AM_R:CY_R,clk>0?AM_G:CY_G,clk>0?AM_B:CY_B,h*1.2f);
    FR(r,PNL_R,PNL_G,PNL_B,(Uint8)(175+(int)(h*55)));

    if(c->img){
        SDL_Rect imgDst={r.x+3,r.y+3,r.w-6,r.h-6};
        Uint8 mod=(Uint8)(130+(int)(h*125));
        SDL_SetTextureColorMod(c->img,mod,mod,mod);
        SDL_RenderCopy(g_ren,c->img,NULL,&imgDst);
        SDL_SetTextureColorMod(c->img,255,255,255);
        SL(imgDst,30);
    }
    if(clk>0){Uint8 fa=(Uint8)(clk*7); FR(r,AM_R,AM_G,0,fa);}

    int   thick=(int)(1+h*4);
    Uint8 bA=(Uint8)(110+(int)(h*145));
    if(clk>0) SR(r,AM_R,AM_G,0,bA,thick);
    else       SR(r,CY_R,CY_G,CY_B,bA,thick);
    Uint8 cA=(Uint8)(70+(int)(h*185));
    if(clk>0) CO(r,AM_R,AM_G,0,255,ACCENT);
    else       CO(r,CY_R,CY_G,CY_B,cA,ACCENT);

    if(h>0.25f){
        float sweep=fmodf((float)ticks/850.0f,1.0f);
        int   sy=r.y+(int)(sweep*r.h);
        SC(CY_R,CY_G,CY_B,(Uint8)(h*85));
        SDL_RenderDrawLine(g_ren,r.x,sy,r.x+r.w,sy);
    }
    char badge[4]; snprintf(badge,sizeof(badge),"%d",(c->puzzleIdx&0xFF)+1);
    SDL_Rect badgeR={r.x+r.w-34,r.y+r.h-20,30,16};
    FR(badgeR,0,10,25,215); SR(badgeR,CY_R,CY_G,CY_B,65,1);
    TC(badge,badgeR,CY_R,CY_G,CY_B,g_fntTiny);
}

/* ─────────────────────────────────────────────────────────────────────────
   "SELECT A PUZZLE" — centred, zoom-in/out pulse, no box around title
   ───────────────────────────────────────────────────────────────────────── */
static void renderSelectLabel(Uint32 ticks){
    /* Zoom pulse: scale oscillates between 0.92 and 1.08 */
    float zoom = 1.0f + 0.08f * sinf((float)ticks / 700.0f);

    const char *txt = "SELECT A PUZZLE";
    SDL_Color col   = {CY_R, CY_G, CY_B, 255};
    SDL_Surface *su = TTF_RenderText_Blended(g_fntBig, txt, col);
    if(!su) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(g_ren, su);
    SDL_FreeSurface(su);
    if(!t) return;

    int tw, th;
    SDL_QueryTexture(t, NULL, NULL, &tw, &th);
    int sw = (int)(tw * zoom);
    int sh = (int)(th * zoom);
    SDL_Rect dst = { WIN_W/2 - sw/2, 16, sw, sh };

    /* Soft glow behind text */
    float gp = 0.5f + 0.5f * sinf((float)ticks / 700.0f);
    SDL_Rect glowR = { dst.x - 20, dst.y - 6, sw + 40, sh + 12 };
    FR(glowR, CY_R, CY_G, CY_B, (Uint8)(gp * 18));

    SDL_SetTextureAlphaMod(t, (Uint8)(200 + gp * 55));
    SDL_RenderCopy(g_ren, t, NULL, &dst);
    SDL_DestroyTexture(t);

    /* Thin animated lines either side */
    float lp = 0.5f + 0.5f * sinf((float)ticks / 500.0f);
    SC(CY_R, CY_G, CY_B, (Uint8)(lp * 80));
    int ly = dst.y + sh / 2;
    SDL_RenderDrawLine(g_ren, 24,           ly, WIN_W/2 - sw/2 - 16, ly);
    SDL_RenderDrawLine(g_ren, WIN_W/2+sw/2+16, ly, WIN_W - 24,       ly);
    /* Small diamond caps */
    SC(CY_R, CY_G, CY_B, (Uint8)(lp * 160));
    SDL_RenderDrawLine(g_ren, WIN_W/2-sw/2-22, ly-4, WIN_W/2-sw/2-16, ly);
    SDL_RenderDrawLine(g_ren, WIN_W/2-sw/2-16, ly,   WIN_W/2-sw/2-22, ly+4);
    SDL_RenderDrawLine(g_ren, WIN_W/2+sw/2+16, ly-4, WIN_W/2+sw/2+22, ly);
    SDL_RenderDrawLine(g_ren, WIN_W/2+sw/2+22, ly,   WIN_W/2+sw/2+16, ly+4);
}

/* ─────────────────────────────────────────────────────────────────────────
   DIVIDER
   ───────────────────────────────────────────────────────────────────────── */
static void renderDivider(Uint32 ticks){
    float    pulse=0.5f+0.5f*sinf((float)ticks/900.0f);
    SDL_Rect div={MAIN_X,MAIN_Y+MAIN_H+8,MAIN_W,14};
    FR(div,0,22,50,185);
    SC(CY_R,CY_G,CY_B,(Uint8)(38+pulse*34));
    SDL_RenderDrawLine(g_ren,div.x+14,div.y+7,div.x+div.w-14,div.y+7);
    CO(div,CY_R,CY_G,CY_B,(Uint8)(60+pulse*45),7);
    float dpos=fmodf((float)ticks/1200.0f,1.0f);
    int   dx2=div.x+14+(int)(dpos*(div.w-28));
    FR((SDL_Rect){dx2-2,div.y+4,5,5},AM_R,AM_G,AM_B,(Uint8)(pulse*210));
}

/* ─────────────────────────────────────────────────────────────────────────
   BACK BUTTON
   ───────────────────────────────────────────────────────────────────────── */
static void renderBack(int hovered,int clicked,Uint32 ticks){
    SDL_Rect r={BACK_X,BACK_Y,BACK_W,BACK_H};
    float pulse=0.5f+0.5f*sinf((float)ticks/500.0f);
    if(hovered||clicked){
        GL(r,0,200,60,hovered?0.7f:1.0f);
        FR(r,0,40,15,225); SR(r,0,255,80,225,2); CO(r,0,255,80,255,8);
        TC("BACK",r,0,255,80,g_fntBig);
    } else {
        FR(r,0,20,10,185);
        SR(r,0,(Uint8)(115+pulse*80),40,185,1);
        TC("BACK",r,0,(Uint8)(175+pulse*80),50,g_fntBig);
    }
    BAT(BACK_X-18,BACK_Y+BACK_H/2,11,0,200,60,hovered?235:115);
}

/* ─────────────────────────────────────────────────────────────────────────
   VIGNETTE
   ───────────────────────────────────────────────────────────────────────── */
static void renderVignette(void){
    for(int i=0;i<70;i++){Uint8 a=(Uint8)(70-i);SC(0,5,15,a);SDL_RenderDrawLine(g_ren,i,0,i,WIN_H);}
    for(int i=0;i<70;i++){Uint8 a=(Uint8)(70-i);SC(0,5,15,a);SDL_RenderDrawLine(g_ren,WIN_W-1-i,0,WIN_W-1-i,WIN_H);}
    for(int i=0;i<50;i++){Uint8 a=(Uint8)(50-i);SC(0,5,15,a);SDL_RenderDrawLine(g_ren,0,WIN_H-1-i,WIN_W,WIN_H-1-i);}
}

/* ─────────────────────────────────────────────────────────────────────────
   HEX STRIP (bottom)
   ───────────────────────────────────────────────────────────────────────── */
static void renderHexStrip(Uint32 ticks){
    static const char *hc="0123456789ABCDEF";
    FR((SDL_Rect){0,WIN_H-22,WIN_W,22},0,6,18,232);
    SC(CY_R,CY_G,CY_B,55); SDL_RenderDrawLine(g_ren,0,WIN_H-22,WIN_W,WIN_H-22);
    int off=(int)(ticks/55)%28;
    for(int i=0;i<WIN_W/12+2;i++){
        char hx[3];
        hx[0]=hc[((i*7+off)^(i*3))&15];
        hx[1]=hc[((i*5+off*2)^i)&15];
        hx[2]=0;
        Uint8 a=(i%5==0)?190:55;
        TL(hx,i*12-(off*12/28),WIN_H-16,CY_R,(Uint8)(CY_G*a/200),(Uint8)(CY_B*a/200),g_fntTiny);
    }
}

/* ─────────────────────────────────────────────────────────────────────────
   RUN MENU
   ───────────────────────────────────────────────────────────────────────── */
int run_menu(SDL_Window *win){
    (void)win;

    SDL_Texture *bgTex=loadTex("bg.png");
    if(!bgTex) bgTex=makePlaceholder(WIN_W,WIN_H,0,8,18);

    SDL_Texture *imgs[NUM_PUZZLES];
    int plW[NUM_PUZZLES],plH[NUM_PUZZLES];
    plW[0]=MAIN_W; plH[0]=MAIN_H;
    plW[1]=plW[2]=plW[3]=CARD_W; plH[1]=plH[2]=plH[3]=CARD_H;
    Uint8 plC[NUM_PUZZLES][3]={{0,25,50},{10,40,20},{0,20,45},{35,10,40}};
    for(int i=0;i<NUM_PUZZLES;i++){
        imgs[i]=loadTex(PUZZLE_IMAGES[i]);
        if(!imgs[i]) imgs[i]=makePlaceholder(plW[i],plH[i],plC[i][0],plC[i][1],plC[i][2]);
    }

    Card cards[NUM_PUZZLES]={
        {{MAIN_X,  MAIN_Y,MAIN_W,MAIN_H},imgs[0],0,0.0f,0},
        {{CARD_A_X,CARD_Y,CARD_W,CARD_H},imgs[1],1,0.0f,0},
        {{CARD_B_X,CARD_Y,CARD_W,CARD_H},imgs[2],2,0.0f,0},
        {{CARD_C_X,CARD_Y,CARD_W,CARD_H},imgs[3],3,0.0f,0},
    };

    SDL_Rect backRect={BACK_X,BACK_Y,BACK_W,BACK_H};
    int backHover=0,backClicked=0,selectedPuzzle=-1,running=1;
    SDL_Event ev;
    float dt=1.0f/60.0f;

    while(running){
        Uint32 ticks=SDL_GetTicks();
        while(SDL_PollEvent(&ev)){
            if(ev.type==SDL_QUIT){selectedPuzzle=-1;running=0;}
            if(ev.type==SDL_KEYDOWN&&ev.key.keysym.sym==SDLK_ESCAPE){selectedPuzzle=-1;running=0;}
            if(ev.type==SDL_MOUSEBUTTONDOWN&&ev.button.button==SDL_BUTTON_LEFT){
                int mx=ev.button.x,my=ev.button.y;
                if(mx>=backRect.x&&mx<=backRect.x+backRect.w&&
                   my>=backRect.y&&my<=backRect.y+backRect.h){
                    backClicked=20; SDL_Delay(200); selectedPuzzle=-1; running=0;
                }
                for(int i=0;i<NUM_PUZZLES;i++){
                    SDL_Rect r=cards[i].rect;
                    if(mx>=r.x&&mx<=r.x+r.w&&my>=r.y&&my<=r.y+r.h){
                        cards[i].clicked=30; SDL_Delay(180);
                        selectedPuzzle=cards[i].puzzleIdx; running=0;
                    }
                }
            }
        }
        int mx,my; SDL_GetMouseState(&mx,&my);
        for(int i=0;i<NUM_PUZZLES;i++){
            SDL_Rect r=cards[i].rect;
            float target=(mx>=r.x&&mx<=r.x+r.w&&my>=r.y&&my<=r.y+r.h)?1.0f:0.0f;
            cards[i].hoverT+=(target-cards[i].hoverT)*8.0f*dt;
            if(cards[i].hoverT<0.01f)cards[i].hoverT=0.0f;
            if(cards[i].hoverT>0.99f)cards[i].hoverT=1.0f;
            if(cards[i].clicked>0)cards[i].clicked--;
        }
        backHover=(mx>=backRect.x&&mx<=backRect.x+backRect.w&&
                   my>=backRect.y&&my<=backRect.y+backRect.h);
        if(backClicked>0)backClicked--;

        /* RENDER */
        SDL_SetRenderDrawColor(g_ren,0,0,0,255);
        SDL_RenderClear(g_ren);
        SDL_RenderCopy(g_ren,bgTex,NULL,&(SDL_Rect){0,0,WIN_W,WIN_H});
        FR((SDL_Rect){0,0,WIN_W,WIN_H},0,8,20,148);

        /* Subtle grid */
        SC(CY_R,CY_G,CY_B,11);
        for(int y=0;y<WIN_H;y+=48) SDL_RenderDrawLine(g_ren,0,y,WIN_W,y);
        for(int x=0;x<WIN_W;x+=48) SDL_RenderDrawLine(g_ren,x,0,x,WIN_H);

        for(int i=0;i<NUM_PUZZLES;i++) renderCard(&cards[i],ticks);

        renderSelectLabel(ticks);   /* zooming label — no title bar box */
        renderDivider(ticks);
        renderBack(backHover,backClicked,ticks);
        renderVignette();
        renderHexStrip(ticks);

        SDL_RenderPresent(g_ren);
        SDL_Delay(8);
    }

    SDL_DestroyTexture(bgTex);
    for(int i=0;i<NUM_PUZZLES;i++) SDL_DestroyTexture(imgs[i]);
    return selectedPuzzle;
}

/* ─────────────────────────────────────────────────────────────────────────
   ENTRY POINT
   ───────────────────────────────────────────────────────────────────────── */
int main(int argc,char *argv[]){
    (void)argc;(void)argv;
    srand((unsigned)time(NULL));

    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_AUDIO)<0){
        fprintf(stderr,"SDL_Init: %s\n",SDL_GetError()); return 1;
    }
    if(!(IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG)&(IMG_INIT_PNG|IMG_INIT_JPG))){
        fprintf(stderr,"IMG_Init failed\n"); SDL_Quit(); return 1;
    }
    if(TTF_Init()<0){
        fprintf(stderr,"TTF_Init: %s\n",TTF_GetError()); IMG_Quit(); SDL_Quit(); return 1;
    }
    if(Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,2048)<0)
        SDL_Log("Mix_OpenAudio: %s — no audio",Mix_GetError());
    else{
        g_music=Mix_LoadMUS("music.ogg");
        if(!g_music) g_music=Mix_LoadMUS("music.mp3");
        if(g_music){ Mix_VolumeMusic(72); Mix_PlayMusic(g_music,-1); }
    }

    SDL_Window *win=SDL_CreateWindow(
        "TACTICAL DECRYPTION ENGINE",
        SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,
        WIN_W,WIN_H,SDL_WINDOW_SHOWN);
    if(!win){fprintf(stderr,"CreateWindow: %s\n",SDL_GetError());return 1;}

    g_ren=SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if(!g_ren){fprintf(stderr,"CreateRenderer: %s\n",SDL_GetError());return 1;}
    SDL_SetRenderDrawBlendMode(g_ren,SDL_BLENDMODE_BLEND);

    const char *bf ="batmfa__.ttf";
    const char *fbo="/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
    const char *fno="/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
    g_fntBig =TTF_OpenFont(bf,24); if(!g_fntBig) g_fntBig =TTF_OpenFont(fbo,22);
    g_fntMid =TTF_OpenFont(bf,16); if(!g_fntMid) g_fntMid =TTF_OpenFont(fbo,15);
    g_fntSm  =TTF_OpenFont(bf,11); if(!g_fntSm)  g_fntSm  =TTF_OpenFont(fno,10);
    g_fntTiny=TTF_OpenFont(bf, 9); if(!g_fntTiny)g_fntTiny=TTF_OpenFont(fno, 9);

    int running=1;
    while(running){
        int choice=run_menu(win);
        if(choice<0){running=0;break;}
        int result=run_puzzle(win,choice);
        if(result<0)running=0;
    }

    if(g_fntBig) TTF_CloseFont(g_fntBig);
    if(g_fntMid) TTF_CloseFont(g_fntMid);
    if(g_fntSm)  TTF_CloseFont(g_fntSm);
    if(g_fntTiny)TTF_CloseFont(g_fntTiny);
    if(g_music){Mix_HaltMusic();Mix_FreeMusic(g_music);}
    Mix_CloseAudio(); TTF_Quit();
    SDL_DestroyRenderer(g_ren); SDL_DestroyWindow(win);
    IMG_Quit(); SDL_Quit();
    return 0;
}

