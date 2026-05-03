#include "puzzle.h"

void initContextPuzzle(ContextPuzzle *ctx, SDL_Renderer *ren,
                       TTF_Font *big, TTF_Font *mid,
                       TTF_Font *sm,  TTF_Font *tiny)
{
    int i;
    if(!ctx||!ren) return;
    ctx->ren       = ren;
    ctx->fntBig    = big;
    ctx->fntMid    = mid;
    ctx->fntSm     = sm;
    ctx->fntTiny   = tiny;
    ctx->bgTex     = IMG_LoadTexture(ren, "assets/assets/bg.png");
    ctx->puzzW     = 0;
    ctx->puzzH     = 0;
    ctx->puzzTex   = NULL;
    ctx->hintCipher[0] = '\0';
    ctx->hintSlot  = -1;
    ctx->hintPiece = -1;
    ctx->hintGlow  = 0;
    ctx->sparkCount= 0;
    for(i=0;i<MAX_DUST;i++){
        ctx->dust[i].x    =(float)(rand()%WIN_W);
        ctx->dust[i].y    =(float)(rand()%WIN_H);
        ctx->dust[i].spd  =0.1f+((float)(rand()%20))/40.0f;
        ctx->dust[i].phase=(float)(rand()%628)/100.0f;
        ctx->dust[i].size =(float)(1+rand()%2);
    }
}

void initMenuPuzzle(CartePuzzle cards[NUM_PUZZLES], ContextPuzzle *ctx,
                    const char *images[NUM_PUZZLES])
{
    int i;
    int plW[NUM_PUZZLES]={MAIN_W,CARD_W,CARD_W,CARD_W};
    int plH[NUM_PUZZLES]={MAIN_H,CARD_H,CARD_H,CARD_H};
    SDL_Rect rects[NUM_PUZZLES]={
        {MAIN_X,  MAIN_Y, MAIN_W, MAIN_H},
        {CARD_A_X,CARD_Y, CARD_W, CARD_H},
        {CARD_B_X,CARD_Y, CARD_W, CARD_H},
        {CARD_C_X,CARD_Y, CARD_W, CARD_H}
    };
    for(i=0;i<NUM_PUZZLES;i++){
        cards[i].rect      = rects[i];
        cards[i].puzzleIdx = i;
        cards[i].hoverT    = 0.0f;
        cards[i].clicked   = 0;
        cards[i].img       = IMG_LoadTexture(ctx->ren, images[i]);
        if(!cards[i].img){
            cards[i].img = SDL_CreateTexture(ctx->ren,
                SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                plW[i], plH[i]);
            if(cards[i].img){
                SDL_SetRenderTarget(ctx->ren, cards[i].img);
                SDL_SetRenderDrawColor(ctx->ren, 0, 20, 40, 255);
                SDL_RenderClear(ctx->ren);
                SDL_SetRenderTarget(ctx->ren, NULL);
            }
        }
    }
}

void renderMenuPuzzle(CartePuzzle cards[NUM_PUZZLES], ContextPuzzle *ctx,
                      Uint32 ticks, int backHov, int backClicked)
{
    int i, seg;
    SDL_Renderer *ren = ctx->ren;

    SDL_SetRenderDrawColor(ren,0,0,0,255); SDL_RenderClear(ren);
    if(ctx->bgTex) SDL_RenderCopy(ren,ctx->bgTex,NULL,&(SDL_Rect){0,0,WIN_W,WIN_H});
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren,0,8,20,148);
    {SDL_Rect bg={0,0,WIN_W,WIN_H}; SDL_RenderFillRect(ren,&bg);}
    SDL_SetRenderDrawColor(ren,0,220,255,11);
    for(int y=0;y<WIN_H;y+=84) SDL_RenderDrawLine(ren,0,y,WIN_W,y);
    for(int x=0;x<WIN_W;x+=84) SDL_RenderDrawLine(ren,x,0,x,WIN_H);

    for(i=0;i<NUM_PUZZLES;i++){
        SDL_Rect r=cards[i].rect;
        float h=cards[i].hoverT;
        int clk=cards[i].clicked;
        SDL_SetRenderDrawColor(ren,PNL_R,PNL_G,PNL_B,(Uint8)(175+(int)(h*55)));
        SDL_RenderFillRect(ren,&r);
        if(cards[i].img){
            SDL_Rect dst={r.x+5,r.y+5,r.w-10,r.h-10};
            Uint8 mod=(Uint8)(130+(int)(h*125));
            SDL_SetTextureColorMod(cards[i].img,mod,mod,mod);
            SDL_RenderCopy(ren,cards[i].img,NULL,&dst);
            SDL_SetTextureColorMod(cards[i].img,255,255,255);
        }
        if(clk>0){Uint8 fa=(Uint8)(clk*7);SDL_SetRenderDrawColor(ren,AM_R,AM_G,0,fa);SDL_RenderFillRect(ren,&r);}
        Uint8 bA=(Uint8)(110+(int)(h*145));
        if(clk>0) SDL_SetRenderDrawColor(ren,AM_R,AM_G,0,bA);
        else       SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,bA);
        SDL_RenderDrawRect(ren,&r);
        if(h>0.25f){
            float sweep=fmodf((float)ticks/850.0f,1.0f);
            int sy=r.y+(int)(sweep*r.h);
            SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,(Uint8)(h*85));
            SDL_RenderDrawLine(ren,r.x,sy,r.x+r.w,sy);
        }
        {char badge[4];snprintf(badge,sizeof(badge),"%d",i+1);
        SDL_Rect badgeR={r.x+r.w-59,r.y+r.h-35,52,28};
        SDL_SetRenderDrawColor(ren,0,10,25,215);SDL_RenderFillRect(ren,&badgeR);
        SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,65);SDL_RenderDrawRect(ren,&badgeR);
        SDL_Color bc={CY_R,CY_G,CY_B,255};SDL_Surface *su=TTF_RenderText_Blended(ctx->fntTiny,badge,bc);
        if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);int tw,th;SDL_QueryTexture(t,NULL,NULL,&tw,&th);SDL_Rect d={badgeR.x+(badgeR.w-tw)/2,badgeR.y+(badgeR.h-th)/2,tw,th};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}}
    }

    {
        float zoom=1.0f+0.08f*sinf((float)ticks/700.0f);
        float gp=0.5f+0.5f*sinf((float)ticks/700.0f);
        SDL_Color col={CY_R,CY_G,CY_B,255};
        SDL_Surface *su=TTF_RenderText_Blended(ctx->fntBig,"SELECT A PUZZLE",col);
        if(su){
            SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);
            int tw,th; SDL_QueryTexture(t,NULL,NULL,&tw,&th);
            int sw=(int)(tw*zoom),sh=(int)(th*zoom);
            SDL_Rect dst={WIN_W/2-sw/2,28,sw,sh};
            SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,(Uint8)(gp*18));
            {SDL_Rect gr={dst.x-35,dst.y-10,sw+70,sh+21};SDL_RenderFillRect(ren,&gr);}
            SDL_SetTextureAlphaMod(t,(Uint8)(200+gp*55));
            if(t){SDL_RenderCopy(ren,t,NULL,&dst);SDL_DestroyTexture(t);}
            SDL_FreeSurface(su);
        }
    }

    {
        SDL_Rect backR={BACK_X,BACK_Y,BACK_W,BACK_H};
        float pulse=0.5f+0.5f*sinf((float)ticks/500.0f);
        if(backHov||backClicked){
            SDL_SetRenderDrawColor(ren,0,40,15,225);SDL_RenderFillRect(ren,&backR);
            SDL_SetRenderDrawColor(ren,0,255,80,225);SDL_RenderDrawRect(ren,&backR);
        } else {
            SDL_SetRenderDrawColor(ren,0,20,10,185);SDL_RenderFillRect(ren,&backR);
            SDL_SetRenderDrawColor(ren,0,(Uint8)(115+pulse*80),40,185);SDL_RenderDrawRect(ren,&backR);
        }
        SDL_Color bc={0,(Uint8)(backHov?255:(175+(Uint8)(pulse*80))),50,255};
        SDL_Surface *su=TTF_RenderText_Blended(ctx->fntBig,"BACK",bc);
        if(su){
            SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);
            int tw,th; SDL_QueryTexture(t,NULL,NULL,&tw,&th);
            SDL_Rect d={backR.x+(backR.w-tw)/2,backR.y+(backR.h-th)/2,tw,th};
            if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}
            SDL_FreeSurface(su);
        }
    }

    {
        const char *hc="0123456789ABCDEF";
        SDL_SetRenderDrawColor(ren,0,6,18,232);
        {SDL_Rect hx={0,WIN_H-38,WIN_W,38};SDL_RenderFillRect(ren,&hx);}
        SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,55);
        SDL_RenderDrawLine(ren,0,WIN_H-38,WIN_W,WIN_H-38);
        int off=(int)(ticks/55)%28;
        for(i=0;i<WIN_W/12+2;i++){
            char hxs[3];
            hxs[0]=hc[((i*7+off)^(i*3))&15];
            hxs[1]=hc[((i*5+off*2)^i)&15];
            hxs[2]=0;
            Uint8 a=(i%5==0)?190:55;
            SDL_Color c={CY_R,(Uint8)(CY_G*a/200),(Uint8)(CY_B*a/200),255};
            SDL_Surface *su=TTF_RenderText_Blended(ctx->fntTiny,hxs,c);
            if(su){
                SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);
                int tw,th; SDL_QueryTexture(t,NULL,NULL,&tw,&th);
                SDL_Rect d={i*12-(off*12/28),WIN_H-28,tw,th};
                if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}
                SDL_FreeSurface(su);
            }
        }
    }
    (void)seg;
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
    SDL_RenderPresent(ren);
}

int runMenuPuzzle(SDL_Window *win, ContextPuzzle *ctx)
{
    (void)win;
    const char *images[NUM_PUZZLES]={"assets/assets/puzzle.png","assets/assets/puzzle1.png","assets/assets/puzzle2.png","assets/assets/puzzle3.png"};
    CartePuzzle cards[NUM_PUZZLES];
    SDL_Rect backRect={BACK_X,BACK_Y,BACK_W,BACK_H};
    int backHov=0, backClicked=0, selected=-1, running=1;
    int mouseLX=0, mouseLY=0;
    float dt=1.0f/60.0f;
    SDL_Event ev;

    SDL_RenderSetLogicalSize(ctx->ren, WIN_W, WIN_H);
    initMenuPuzzle(cards, ctx, images);

    while(running){
        Uint32 ticks=SDL_GetTicks();
        while(SDL_PollEvent(&ev)){
            if(ev.type==SDL_QUIT){selected=-1;running=0;}
            if(ev.type==SDL_KEYDOWN&&ev.key.keysym.sym==SDLK_ESCAPE){selected=-1;running=0;}
            if(ev.type==SDL_MOUSEMOTION){mouseLX=ev.motion.x;mouseLY=ev.motion.y;}
            if(ev.type==SDL_MOUSEBUTTONDOWN&&ev.button.button==SDL_BUTTON_LEFT){
                int mx=ev.button.x,my=ev.button.y;
                mouseLX=mx;mouseLY=my;
                if(mx>=backRect.x&&mx<=backRect.x+backRect.w&&my>=backRect.y&&my<=backRect.y+backRect.h){selected=-1;running=0;}
                for(int i=0;i<NUM_PUZZLES;i++){
                    SDL_Rect r=cards[i].rect;
                    if(mx>=r.x&&mx<=r.x+r.w&&my>=r.y&&my<=r.y+r.h){
                        cards[i].clicked=30;selected=cards[i].puzzleIdx;running=0;
                    }
                }
            }
        }
        {int mx=mouseLX,my=mouseLY;
        for(int i=0;i<NUM_PUZZLES;i++){
            SDL_Rect r=cards[i].rect;
            float target=(mx>=r.x&&mx<=r.x+r.w&&my>=r.y&&my<=r.y+r.h)?1.0f:0.0f;
            cards[i].hoverT+=(target-cards[i].hoverT)*8.0f*dt;
            if(cards[i].hoverT<0.01f)cards[i].hoverT=0.0f;
            if(cards[i].hoverT>0.99f)cards[i].hoverT=1.0f;
            if(cards[i].clicked>0)cards[i].clicked--;
        }
        backHov=(mx>=backRect.x&&mx<=backRect.x+backRect.w&&my>=backRect.y&&my<=backRect.y+backRect.h);
        if(backClicked>0)backClicked--;}

        renderMenuPuzzle(cards, ctx, ticks, backHov, backClicked);
        SDL_Delay(8);
    }
    for(int i=0;i<NUM_PUZZLES;i++) if(cards[i].img){SDL_DestroyTexture(cards[i].img);cards[i].img=NULL;}
    SDL_RenderSetLogicalSize(ctx->ren,0,0);
    return selected;
}

int runPuzzle(SDL_Window *win, ContextPuzzle *ctx, int puzzleIdx)
{
    (void)win;
    const char *images[NUM_PUZZLES]={"assets/assets/puzzle.png","assets/assets/puzzle1.png","assets/assets/puzzle2.png","assets/assets/puzzle3.png"};
    SDL_Renderer *ren=ctx->ren;
    int i, s, result=0, running=1;
    int state=ST_PLAY, hintsLeft=MAX_HINTS;
    int dragging=-1, dragOX=0, dragOY=0, correctCount=0;
    int wantRestart=0, wantBack=0;
    int mouseLX=0, mouseLY=0;
    float endProgress=0.0f, winWaveT=0.0f;
    TrembEcran shake={0,0,0,0};
    Uint32 startT=SDL_GetTicks(), lastTick=SDL_GetTicks();
    SDL_Event ev;
    SDL_Rect gridSlot[GPIECES], backBtn={17,972,150,56};
    SDL_Rect hintBtn={1629,258,265,209};
    int slotOcc[GPIECES], order[GPIECES];
    Piece pieces[GPIECES];
    int totalW, repoStartX;

    SDL_RenderSetLogicalSize(ren, WIN_W, WIN_H);

    ctx->puzzTex=IMG_LoadTexture(ren,images[puzzleIdx]);
    if(ctx->puzzTex) SDL_QueryTexture(ctx->puzzTex,NULL,NULL,&ctx->puzzW,&ctx->puzzH);
    else {
        ctx->puzzW=390; ctx->puzzH=390;
        ctx->puzzTex=SDL_CreateTexture(ren,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_TARGET,390,390);
        SDL_SetRenderTarget(ren,ctx->puzzTex);
        SDL_SetRenderDrawColor(ren,0,14,34,255); SDL_RenderClear(ren);
        SDL_SetRenderTarget(ren,NULL);
    }
    ctx->hintCipher[0]='\0'; ctx->hintSlot=-1; ctx->hintPiece=-1; ctx->hintGlow=0; ctx->sparkCount=0;

    for(i=0;i<GPIECES;i++) gridSlot[i]=(SDL_Rect){GRID_X+(i%GCOLS)*CELL,GRID_Y+(i/GCOLS)*CELL,CELL,CELL};
    for(i=0;i<GPIECES;i++){slotOcc[i]=-1; order[i]=i;}
    for(i=GPIECES-1;i>0;i--){int j=rand()%(i+1),tmp=order[i];order[i]=order[j];order[j]=tmp;}
    totalW=GPIECES*(RPW+RGAP)-RGAP; repoStartX=(WIN_W-totalW)/2;
    for(i=0;i<GPIECES;i++){
        int rx=repoStartX+i*(RPW+RGAP);
        pieces[i].idx=order[i]; pieces[i].slot=-1;
        pieces[i].home=(SDL_Rect){rx,REPO_Y,RPW,RPH}; pieces[i].cur=pieces[i].home;
        pieces[i].shakeX=0; pieces[i].shakeTmr=0; pieces[i].flashT=0; pieces[i].flashOk=0;
    }

    while(running){
        Uint32 now=SDL_GetTicks();
        float dt=(float)(now-lastTick)/1000.0f; if(dt>0.05f)dt=0.05f; lastTick=now;
        int mouseClicked=0, mx=mouseLX, my=mouseLY;

        while(SDL_PollEvent(&ev)){
            if(ev.type==SDL_QUIT){result=-1;running=0;}
            if(ev.type==SDL_MOUSEMOTION){
                mouseLX=ev.motion.x; mouseLY=ev.motion.y; mx=mouseLX; my=mouseLY;
                if(dragging>=0){pieces[dragging].cur.x=ev.motion.x-dragOX;pieces[dragging].cur.y=ev.motion.y-dragOY;}
            }
            if(ev.type==SDL_KEYDOWN){
                if(ev.key.keysym.sym==SDLK_ESCAPE){result=0;running=0;}
                if(ev.key.keysym.sym==SDLK_r&&state!=ST_PLAY)wantRestart=1;
                if(ev.key.keysym.sym==SDLK_h&&state==ST_PLAY&&hintsLeft>0){
                    hintsLeft--; ctx->hintGlow=280;
                    int cands[GPIECES],nc=0;
                    for(s=0;s<GPIECES;s++){int pi=slotOcc[s];if(pi<0||pieces[pi].idx!=s)cands[nc++]=s;}
                    if(nc){ctx->hintSlot=cands[rand()%nc];ctx->hintPiece=-1;
                    for(i=0;i<GPIECES;i++) if(pieces[i].idx==ctx->hintSlot){ctx->hintPiece=i;break;}
                    snprintf(ctx->hintCipher,sizeof(ctx->hintCipher),"NODE_%02d",ctx->hintSlot+1);}
                }
            }
            if(ev.type==SDL_MOUSEBUTTONDOWN&&ev.button.button==SDL_BUTTON_LEFT){
                int bx=ev.button.x,by=ev.button.y;
                mouseLX=bx;mouseLY=by;mx=bx;my=by;mouseClicked=1;
                if(bx>=backBtn.x&&bx<=backBtn.x+backBtn.w&&by>=backBtn.y&&by<=backBtn.y+backBtn.h){result=0;running=0;}
                if(state==ST_PLAY&&bx>=hintBtn.x&&bx<=hintBtn.x+hintBtn.w&&by>=hintBtn.y&&by<=hintBtn.y+hintBtn.h&&hintsLeft>0){
                    hintsLeft--; ctx->hintGlow=280;
                    int cands[GPIECES],nc=0;
                    for(s=0;s<GPIECES;s++){int pi=slotOcc[s];if(pi<0||pieces[pi].idx!=s)cands[nc++]=s;}
                    if(nc){ctx->hintSlot=cands[rand()%nc];ctx->hintPiece=-1;
                    for(i=0;i<GPIECES;i++) if(pieces[i].idx==ctx->hintSlot){ctx->hintPiece=i;break;}
                    snprintf(ctx->hintCipher,sizeof(ctx->hintCipher),"NODE_%02d",ctx->hintSlot+1);}
                }
                if(state==ST_PLAY&&dragging<0){
                    for(i=0;i<GPIECES;i++){if(pieces[i].slot>=0)continue;SDL_Rect r=pieces[i].cur;if(bx>=r.x&&bx<=r.x+r.w&&by>=r.y&&by<=r.y+r.h){dragging=i;dragOX=bx-r.x;dragOY=by-r.y;break;}}
                    if(dragging<0){for(s=0;s<GPIECES;s++){int pi=slotOcc[s];if(pi<0)continue;SDL_Rect r=pieces[pi].cur;if(bx>=r.x&&bx<=r.x+r.w&&by>=r.y&&by<=r.y+r.h){slotOcc[s]=-1;pieces[pi].slot=-1;dragging=pi;dragOX=bx-r.x;dragOY=by-r.y;for(s=0;s<GPIECES;s++){int p=slotOcc[s];if(p>=0&&pieces[p].idx==s)correctCount++;}correctCount=0;for(s=0;s<GPIECES;s++){int p=slotOcc[s];if(p>=0&&pieces[p].idx==s)correctCount++;}break;}}}
                }
                if(state!=ST_PLAY){
                    int panH=453,panY=WIN_H/2-panH/2;
                    SDL_Rect btnR={WIN_W/2-488+94,panY+293,342,91};
                    SDL_Rect btnB={WIN_W/2-488+541,panY+293,342,91};
                    if(bx>=btnR.x&&bx<=btnR.x+btnR.w&&by>=btnR.y&&by<=btnR.y+btnR.h)wantRestart=1;
                    if(bx>=btnB.x&&bx<=btnB.x+btnB.w&&by>=btnB.y&&by<=btnB.y+btnB.h)wantBack=1;
                }
            }
            if(ev.type==SDL_MOUSEBUTTONUP&&ev.button.button==SDL_BUTTON_LEFT&&dragging>=0){
                int pcx=pieces[dragging].cur.x+RPW/2,pcy=pieces[dragging].cur.y+RPH/2;
                int best=-1; float bestD=1e9f;
                for(s=0;s<GPIECES;s++){if(slotOcc[s]>=0)continue;float dx2=(float)(pcx-(gridSlot[s].x+CELL/2)),dy2=(float)(pcy-(gridSlot[s].y+CELL/2)),d=dx2*dx2+dy2*dy2;if(d<(float)(CELL*CELL)*0.55f&&d<bestD){bestD=d;best=s;}}
                if(best>=0){
                    slotOcc[best]=dragging; pieces[dragging].slot=best;
                    pieces[dragging].cur=(SDL_Rect){gridSlot[best].x+1,gridSlot[best].y+1,CELL-2,CELL-2};
                    int wasOk=(pieces[dragging].idx==best);
                    pieces[dragging].flashT=0.8f; pieces[dragging].flashOk=wasOk;
                    correctCount=0; for(s=0;s<GPIECES;s++){int p=slotOcc[s];if(p>=0&&pieces[p].idx==s)correctCount++;}
                    if(correctCount==GPIECES){state=ST_WIN;winWaveT=0;}
                    if(!wasOk){shake.intensity=14.0f;shake.timeLeft=0.45f;}
                    int nc=0; for(s=0;s<MAX_SPARKS&&ctx->sparkCount<MAX_SPARKS;s++){if(nc>=20)break;
                        Etincelle *sp=&ctx->sparks[ctx->sparkCount++];
                        float ang=(float)nc/20*6.283f; float spd=1.8f+((float)(rand()%100))/100.0f*3.5f;
                        sp->x=(float)(gridSlot[best].x+CELL/2); sp->y=(float)(gridSlot[best].y+CELL/2);
                        sp->vx=cosf(ang)*spd; sp->vy=sinf(ang)*spd-1.8f;
                        sp->life=sp->maxLife=0.6f+((float)(rand()%60))/100.0f;
                        if(wasOk){sp->r=0;sp->g=(Uint8)(180+rand()%75);sp->b=(Uint8)(100+rand()%155);}
                        else{sp->r=255;sp->g=(Uint8)(rand()%80);sp->b=(Uint8)(rand()%60);}
                        nc++;
                    }
                }else pieces[dragging].cur=pieces[dragging].home;
                dragging=-1;
            }
        }

        if(wantRestart){
            state=ST_PLAY;startT=SDL_GetTicks();hintsLeft=MAX_HINTS;
            ctx->hintGlow=0;ctx->hintSlot=-1;ctx->hintPiece=-1;ctx->hintCipher[0]=0;
            dragging=-1;correctCount=0;endProgress=0;winWaveT=0;wantRestart=0;wantBack=0;ctx->sparkCount=0;
            shake.timeLeft=0;shake.ox=shake.oy=0;
            for(i=0;i<GPIECES;i++)slotOcc[i]=-1;
            for(i=GPIECES-1;i>0;i--){int j=rand()%(i+1),tmp=order[i];order[i]=order[j];order[j]=tmp;}
            for(i=0;i<GPIECES;i++){int rx=repoStartX+i*(RPW+RGAP);pieces[i].idx=order[i];pieces[i].slot=-1;pieces[i].home=(SDL_Rect){rx,REPO_Y,RPW,RPH};pieces[i].cur=pieces[i].home;pieces[i].shakeX=0;pieces[i].shakeTmr=0;pieces[i].flashT=0;}
        }
        if(wantBack){result=0;running=0;}

        Uint32 elapsed=(Uint32)(now-startT);
        float timerFrac=(state==ST_PLAY)?1.0f-(float)elapsed/TIMER_MS:(correctCount==GPIECES?1.0f:0.0f);
        if(timerFrac<0)timerFrac=0;
        if(state==ST_PLAY&&elapsed>=TIMER_MS)state=ST_LOSE;
        if(state!=ST_PLAY&&endProgress<1.0f)endProgress+=dt*0.65f;
        if(endProgress>1.0f)endProgress=1.0f;
        if(state==ST_WIN)winWaveT+=dt;

        for(i=0;i<GPIECES;i++){
            if(pieces[i].shakeTmr>0){pieces[i].shakeTmr-=dt*60;pieces[i].shakeX=sinf(pieces[i].shakeTmr*1.4f)*7.0f*(pieces[i].shakeTmr/22.0f);}
            else pieces[i].shakeX=0;
            if(pieces[i].flashT>0)pieces[i].flashT-=dt;
        }
        if(ctx->hintGlow>0)ctx->hintGlow--;
        if(shake.timeLeft>0){shake.timeLeft-=dt;float mag=shake.intensity*(shake.timeLeft>0?shake.timeLeft/0.45f:0);shake.ox=(int)(((float)(rand()%2001)-1000)/1000.0f*mag);shake.oy=(int)(((float)(rand()%2001)-1000)/1000.0f*mag*0.6f);}
        else{shake.ox=shake.oy=0;}
        for(i=0;i<ctx->sparkCount;){Etincelle *sp=&ctx->sparks[i];sp->x+=sp->vx;sp->y+=sp->vy;sp->vy+=0.12f;sp->life-=dt;if(sp->life<=0){ctx->sparks[i]=ctx->sparks[--ctx->sparkCount];}else i++;}

        SDL_SetRenderDrawColor(ren,0,0,0,255); SDL_RenderClear(ren);
        {SDL_Rect vp={shake.ox,shake.oy,WIN_W,WIN_H};SDL_RenderSetViewport(ren,&vp);}

        if(ctx->bgTex) SDL_RenderCopy(ren,ctx->bgTex,NULL,&(SDL_Rect){0,0,WIN_W,WIN_H});
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren,0,6,16,105);
        {SDL_Rect bg={0,0,WIN_W,WIN_H};SDL_RenderFillRect(ren,&bg);}

        for(i=0;i<MAX_DUST;i++){
            ctx->dust[i].y-=ctx->dust[i].spd;
            if(ctx->dust[i].y<0)ctx->dust[i].y=(float)WIN_H;
            ctx->dust[i].x+=sinf(ctx->dust[i].phase+(float)now/2000.0f)*0.3f;
            float bright=0.3f+0.5f*sinf(ctx->dust[i].phase+(float)now/1500.0f);
            SDL_SetRenderDrawColor(ren,0,200,255,(Uint8)(bright*48));
            SDL_RenderDrawPoint(ren,(int)ctx->dust[i].x,(int)ctx->dust[i].y);
        }

        {
            Uint8 tR=(Uint8)(255*(1-timerFrac)),tG=(Uint8)(210*timerFrac);
            SDL_SetRenderDrawColor(ren,0,6,18,238);
            {SDL_Rect tb={0,0,WIN_W,77};SDL_RenderFillRect(ren,&tb);}
            SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,60);SDL_RenderDrawLine(ren,0,77,WIN_W,77);
            SDL_Rect tbar={105,21,1535,28};
            SDL_SetRenderDrawColor(ren,0,18,38,205);SDL_RenderFillRect(ren,&tbar);
            SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,85);SDL_RenderDrawRect(ren,&tbar);
            SDL_SetRenderDrawColor(ren,tR,tG,40,230);
            {SDL_Rect tf={tbar.x,tbar.y,(int)(1535*timerFrac),tbar.h};SDL_RenderFillRect(ren,&tf);}
            if(timerFrac<0.25f&&state==ST_PLAY){float p=0.5f+0.5f*sinf((float)now/180.0f);SDL_SetRenderDrawColor(ren,255,60,0,(Uint8)(p*70));SDL_RenderFillRect(ren,&tbar);}
        }

        {
            SDL_Rect lp={14,87,398,488};
            SDL_SetRenderDrawColor(ren,PNL_R,PNL_G,PNL_B,210);SDL_RenderFillRect(ren,&lp);
            SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,88);SDL_RenderDrawRect(ren,&lp);
            SDL_SetRenderDrawColor(ren,0,28,62,230);
            {SDL_Rect hdr={14,87,398,38};SDL_RenderFillRect(ren,&hdr);}
            {SDL_Color tc={CY_R,CY_G,CY_B,255};SDL_Surface *su=TTF_RenderText_Blended(ctx->fntSm,"REFERENCE",tc);
            if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);SDL_Rect d={24,91,su->w,su->h};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}}
            if(ctx->puzzTex){SDL_Rect ref={21,125,384,341};SDL_RenderCopy(ren,ctx->puzzTex,NULL,&ref);}
            {char pg[32];snprintf(pg,32,"%d / %d",correctCount,GPIECES);
            SDL_Rect pl={14,477,398,56};
            SDL_SetRenderDrawColor(ren,0,18,42,220);SDL_RenderFillRect(ren,&pl);
            SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,85);SDL_RenderDrawRect(ren,&pl);
            SDL_Color tc={CY_R,CY_G,CY_B,255};
            SDL_Surface *su=TTF_RenderText_Blended(ctx->fntMid,pg,tc);
            if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);int tw,th;SDL_QueryTexture(t,NULL,NULL,&tw,&th);SDL_Rect d={pl.x+(pl.w-tw)/2,pl.y+(pl.h-th)/2,tw,th};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}}
        }

        {
            int gpX=GRID_X-28,gpY=GRID_Y-28,gpW=GCOLS*CELL+56,gpH=GROWS*CELL+56;
            SDL_SetRenderDrawColor(ren,PNL_R,PNL_G,PNL_B,192);
            {SDL_Rect gp={gpX,gpY,gpW,gpH};SDL_RenderFillRect(ren,&gp);}
            SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,105);
            {SDL_Rect gp={gpX,gpY,gpW,gpH};SDL_RenderDrawRect(ren,&gp);}
            for(s=0;s<GPIECES;s++){
                SDL_Rect sr=gridSlot[s];
                if(slotOcc[s]>=0){
                    int pi=slotOcc[s];
                    SDL_Rect dst={sr.x+1,sr.y+1,CELL-2,CELL-2};
                    if(ctx->puzzTex){
                        SDL_Rect src={(pieces[pi].idx%GCOLS)*(ctx->puzzW/GCOLS),(pieces[pi].idx/GCOLS)*(ctx->puzzH/GROWS),ctx->puzzW/GCOLS,ctx->puzzH/GROWS};
                        SDL_RenderCopy(ren,ctx->puzzTex,&src,&dst);
                    }
                    if(pieces[pi].flashT>0){Uint8 fa=(Uint8)(pieces[pi].flashT/0.8f*140);SDL_SetRenderDrawColor(ren,255,255,255,fa);SDL_RenderFillRect(ren,&dst);}
                    SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,58);SDL_RenderDrawRect(ren,&sr);
                }else{
                    SDL_SetRenderDrawColor(ren,0,35,65,42);SDL_RenderFillRect(ren,&(SDL_Rect){sr.x+1,sr.y+1,CELL-2,CELL-2});
                    SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,105);SDL_RenderDrawRect(ren,&sr);
                    {char sn[4];snprintf(sn,4,"%d",s+1);
                    SDL_Color tc={CY_R,CY_G,CY_B,255};
                    SDL_Surface *su=TTF_RenderText_Blended(ctx->fntTiny,sn,tc);
                    if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);int tw,th;SDL_QueryTexture(t,NULL,NULL,&tw,&th);SDL_Rect d={sr.x+(sr.w-tw)/2,sr.y+(sr.h-th)/2,tw,th};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}}
                }
            }
        }

        {
            SDL_Rect rp={1604,87,300,854};
            SDL_SetRenderDrawColor(ren,PNL_R,PNL_G,PNL_B,196);SDL_RenderFillRect(ren,&rp);
            SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,84);SDL_RenderDrawRect(ren,&rp);
            SDL_SetRenderDrawColor(ren,0,28,62,230);
            {SDL_Rect hdr={1604,87,300,38};SDL_RenderFillRect(ren,&hdr);}
            {SDL_Color tc={CY_R,CY_G,CY_B,255};SDL_Surface *su=TTF_RenderText_Blended(ctx->fntSm,"SYS LOG",tc);
            if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);SDL_Rect d={1616,92,su->w,su->h};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}}
            {SDL_Color tc={GR_R,GR_G,GR_B,255};SDL_Surface *su=TTF_RenderText_Blended(ctx->fntTiny,"INTEGRITY",tc);
            if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);SDL_Rect d={1620,136,su->w,su->h};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}}
            {SDL_Rect bg={1620,157,261,17},fg={1620,157,(int)(261*correctCount/GPIECES),17};
            SDL_SetRenderDrawColor(ren,0,18,38,165);SDL_RenderFillRect(ren,&bg);
            SDL_SetRenderDrawColor(ren,GR_R,GR_G,GR_B,215);SDL_RenderFillRect(ren,&fg);
            SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,52);SDL_RenderDrawRect(ren,&bg);}
            {SDL_Color tc={AM_R,AM_G,AM_B,255};SDL_Surface *su=TTF_RenderText_Blended(ctx->fntTiny,"TIMER",tc);
            if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);SDL_Rect d={1620,188,su->w,su->h};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}}
            {Uint8 tR=(Uint8)(255*(1-timerFrac)),tG=(Uint8)(210*timerFrac);
            SDL_Rect bg={1620,209,261,17},fg={1620,209,(int)(261*timerFrac),17};
            SDL_SetRenderDrawColor(ren,0,18,38,165);SDL_RenderFillRect(ren,&bg);
            SDL_SetRenderDrawColor(ren,tR,tG,40,205);SDL_RenderFillRect(ren,&fg);
            SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,52);SDL_RenderDrawRect(ren,&bg);}
            SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,35);SDL_RenderDrawLine(ren,1620,240,1892,240);
            {SDL_Rect btn=hintBtn;int hov=(mx>=btn.x&&mx<=btn.x+btn.w&&my>=btn.y&&my<=btn.y+btn.h);int active=(hintsLeft>0);
            float pulse=0.5f+0.5f*sinf((float)now/380.0f);
            SDL_SetRenderDrawColor(ren,active?(Uint8)(20+hov*20):8,active?(Uint8)(14+hov*10):8,0,230);SDL_RenderFillRect(ren,&btn);
            SDL_SetRenderDrawColor(ren,active?AM_R:60,active?AM_G:60,active?AM_B:60,active?(Uint8)(180+pulse*75):140);SDL_RenderDrawRect(ren,&btn);
            SDL_Color htc={active?AM_R:50,active?AM_G:50,0,255};
            SDL_Surface *su=TTF_RenderText_Blended(ctx->fntTiny,"[H]",htc);
            if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);SDL_Rect d={btn.x+btn.w/2-su->w/2,btn.y+btn.h-13,su->w,su->h};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}
            char hstr[8];snprintf(hstr,8,"H:%d",hintsLeft);
            SDL_Surface *su2=TTF_RenderText_Blended(ctx->fntMid,hstr,htc);
            if(su2){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su2);int tw,th;SDL_QueryTexture(t,NULL,NULL,&tw,&th);SDL_Rect d={btn.x+(btn.w-tw)/2,btn.y+(btn.h-th)/2,tw,th};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su2);}}
            SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,30);SDL_RenderDrawLine(ren,1620,505,1892,505);
            {SDL_Color tc={CY_R,CY_G,CY_B,255};SDL_Surface *su=TTF_RenderText_Blended(ctx->fntTiny,"CIPHER:",tc);
            if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);SDL_Rect d={1620,516,su->w,su->h};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}}
            {SDL_Rect hd={1613,537,272,56};SDL_SetRenderDrawColor(ren,0,14,30,205);SDL_RenderFillRect(ren,&hd);SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,58);SDL_RenderDrawRect(ren,&hd);
            SDL_Color tc={CY_R,CY_G,CY_B,255};
            const char *ctxt=(ctx->hintGlow>0)?ctx->hintCipher:"- AWAITING -";
            SDL_Surface *su=TTF_RenderText_Blended(ctx->fntTiny,ctxt,tc);
            if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);int tw,th;SDL_QueryTexture(t,NULL,NULL,&tw,&th);SDL_Rect d={hd.x+(hd.w-tw)/2,hd.y+(hd.h-th)/2,tw,th};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}}
            {SDL_Color tc={CY_R,CY_G,CY_B,255};SDL_Surface *su=TTF_RenderText_Blended(ctx->fntTiny,"PLACEMENT:",tc);
            if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);SDL_Rect d={1620,638,su->w,su->h};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}}
            {char l1[32],l2[32];snprintf(l1,32,"OK:  %d",correctCount);snprintf(l2,32,"LEFT:%d",GPIECES-correctCount);
            SDL_Color c1={GR_R,GR_G,GR_B,255},c2={AM_R,AM_G,AM_B,255};
            SDL_Surface *su=TTF_RenderText_Blended(ctx->fntTiny,l1,c1);if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);SDL_Rect d={1620,662,su->w,su->h};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}
            su=TTF_RenderText_Blended(ctx->fntTiny,l2,c2);if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);SDL_Rect d={1620,686,su->w,su->h};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}}
            SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,28);SDL_RenderDrawLine(ren,1620,718,1878,718);
            {SDL_Color tc={CY_R,CY_G,CY_B,255};SDL_Surface *su=TTF_RenderText_Blended(ctx->fntTiny,"AUDIO:",tc);
            if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);SDL_Rect d={1620,728,su->w,su->h};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}}
            {float mp=0.5f+0.5f*sinf((float)now/400.0f);int b;
            for(b=0;b<5;b++){int bh=3+(int)(mp*(2+b)*3);
            SDL_SetRenderDrawColor(ren,CY_R,(Uint8)(CY_G*mp),(Uint8)(CY_B*0.8f),(Uint8)(120+mp*135));
            {SDL_Rect bar={1697+b*17,756-bh,12,bh};SDL_RenderFillRect(ren,&bar);}}}
        }

        {
            int totalW2=GPIECES*(RPW+RGAP)-RGAP;
            SDL_Rect repoBg={repoStartX-24,REPO_Y-24,totalW2+48,RPH+48};
            SDL_SetRenderDrawColor(ren,PNL_R,PNL_G,PNL_B,205);SDL_RenderFillRect(ren,&repoBg);
            SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,105);SDL_RenderDrawRect(ren,&repoBg);
            for(i=0;i<GPIECES;i++){
                if(pieces[i].slot>=0||i==dragging) continue;
                SDL_Rect r=pieces[i].cur; r.x+=(int)pieces[i].shakeX;
                SDL_SetRenderDrawColor(ren,0,8,22,215);SDL_RenderFillRect(ren,&r);
                if(ctx->puzzTex){
                    SDL_Rect src={(pieces[i].idx%GCOLS)*(ctx->puzzW/GCOLS),(pieces[i].idx/GCOLS)*(ctx->puzzH/GROWS),ctx->puzzW/GCOLS,ctx->puzzH/GROWS};
                    SDL_RenderCopy(ren,ctx->puzzTex,&src,&r);
                }
                SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,68);SDL_RenderDrawRect(ren,&r);
            }
        }

        if(dragging>=0){
            SDL_Rect r=pieces[dragging].cur;
            SDL_SetRenderDrawColor(ren,0,28,55,215);SDL_RenderFillRect(ren,&r);
            if(ctx->puzzTex){SDL_Rect src={(pieces[dragging].idx%GCOLS)*(ctx->puzzW/GCOLS),(pieces[dragging].idx/GCOLS)*(ctx->puzzH/GROWS),ctx->puzzW/GCOLS,ctx->puzzH/GROWS};SDL_RenderCopy(ren,ctx->puzzTex,&src,&r);}
            SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,255);SDL_RenderDrawRect(ren,&r);
        }

        for(i=0;i<ctx->sparkCount;i++){
            Etincelle *sp=&ctx->sparks[i];
            Uint8 a=(Uint8)(sp->life/sp->maxLife*230);
            SDL_SetRenderDrawColor(ren,sp->r,sp->g,sp->b,a);
            SDL_RenderDrawPoint(ren,(int)sp->x,(int)sp->y);
            SDL_RenderDrawPoint(ren,(int)sp->x+1,(int)sp->y);
        }

        if(shake.timeLeft>0){float t=shake.timeLeft/0.45f;if(t>1)t=1;int border=(int)(t*32);for(i=0;i<border;i++){Uint8 ia=(Uint8)(t*200*(border-i)/border);SDL_SetRenderDrawColor(ren,255,0,0,ia);SDL_RenderDrawLine(ren,i,0,i,WIN_H);SDL_RenderDrawLine(ren,WIN_W-1-i,0,WIN_W-1-i,WIN_H);SDL_RenderDrawLine(ren,0,i,WIN_W,i);SDL_RenderDrawLine(ren,0,WIN_H-1-i,WIN_W,WIN_H-1-i);}SDL_SetRenderDrawColor(ren,255,0,0,(Uint8)(t*50));SDL_RenderFillRect(ren,&(SDL_Rect){0,0,WIN_W,WIN_H});}

        {
            SDL_Rect backR=backBtn;
            int bHov=(mx>=backR.x&&mx<=backR.x+backR.w&&my>=backR.y&&my<=backR.y+backR.h);
            SDL_SetRenderDrawColor(ren,0,bHov?34:16,bHov?14:10,bHov?238:210);SDL_RenderFillRect(ren,&backR);
            SDL_SetRenderDrawColor(ren,0,bHov?225:115,42,bHov?225:115);SDL_RenderDrawRect(ren,&backR);
            SDL_Color bc={0,(Uint8)(bHov?255:185),62,255};
            SDL_Surface *su=TTF_RenderText_Blended(ctx->fntMid,"BACK",bc);
            if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);int tw,th;SDL_QueryTexture(t,NULL,NULL,&tw,&th);SDL_Rect d={backR.x+(backR.w-tw)/2,backR.y+(backR.h-th)/2,tw,th};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}
        }

        SDL_SetRenderDrawColor(ren,0,6,18,230);
        {SDL_Rect bot={0,WIN_H-35,WIN_W,35};SDL_RenderFillRect(ren,&bot);}

        SDL_RenderSetViewport(ren,NULL);

        if(state!=ST_PLAY){
            float pp=(endProgress-0.3f)/0.7f; if(pp<0)pp=0; if(pp>1)pp=1;
            int panH=453,panY=WIN_H/2-panH/2;
            SDL_Rect panel={WIN_W/2-488,panY,976,panH};
            SDL_SetRenderDrawColor(ren,state==ST_WIN?0:40,state==ST_WIN?40:0,state==ST_WIN?10:0,(Uint8)(endProgress*210));
            SDL_RenderFillRect(ren,&(SDL_Rect){0,0,WIN_W,WIN_H});
            if(endProgress>0.3f){
                SDL_SetRenderDrawColor(ren,0,10,22,245);SDL_RenderFillRect(ren,&panel);
                SDL_SetRenderDrawColor(ren,state==ST_WIN?GR_R:RD_R,state==ST_WIN?GR_G:RD_G,state==ST_WIN?GR_B:RD_B,(Uint8)(pp>0.8f?230:170));
                SDL_RenderDrawRect(ren,&panel);
                SDL_Rect btnR={panel.x+94,panY+293,342,91},btnB={panel.x+541,panY+293,342,91};
                int hR=(mx>=btnR.x&&mx<=btnR.x+btnR.w&&my>=btnR.y&&my<=btnR.y+btnR.h);
                int hB=(mx>=btnB.x&&mx<=btnB.x+btnB.w&&my>=btnB.y&&my<=btnB.y+btnB.h);
                SDL_SetRenderDrawColor(ren,0,hR?45:18,hR?65:38,215);SDL_RenderFillRect(ren,&btnR);
                SDL_SetRenderDrawColor(ren,CY_R,CY_G,CY_B,hR?235:115);SDL_RenderDrawRect(ren,&btnR);
                SDL_SetRenderDrawColor(ren,0,hB?40:18,hB?18:10,215);SDL_RenderFillRect(ren,&btnB);
                SDL_SetRenderDrawColor(ren,0,hB?225:105,hB?85:40,hB?225:105);SDL_RenderDrawRect(ren,&btnB);
                SDL_Color cwh={CY_R,CY_G,CY_B,255}, cgn={0,hB?255:175,65,255};
                const char *stxtR="START OVER", *stxtB="GO BACK";
                const char *title=state==ST_WIN?"DECRYPTION SUCCESS":"DECRYPTION FAILED";
                SDL_Color ctitle={state==ST_WIN?GR_R:RD_R,state==ST_WIN?GR_G:RD_G,state==ST_WIN?GR_B:RD_B,255};
                SDL_Surface *su;
                su=TTF_RenderText_Blended(ctx->fntBig,title,ctitle);
                if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);int tw,th;SDL_QueryTexture(t,NULL,NULL,&tw,&th);SDL_Rect d={panel.x+(panel.w-tw)/2,panY+105,tw,th};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}
                su=TTF_RenderText_Blended(ctx->fntMid,stxtR,cwh);
                if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);int tw,th;SDL_QueryTexture(t,NULL,NULL,&tw,&th);SDL_Rect d={btnR.x+(btnR.w-tw)/2,btnR.y+(btnR.h-th)/2,tw,th};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}
                su=TTF_RenderText_Blended(ctx->fntMid,stxtB,cgn);
                if(su){SDL_Texture *t=SDL_CreateTextureFromSurface(ren,su);int tw,th;SDL_QueryTexture(t,NULL,NULL,&tw,&th);SDL_Rect d={btnB.x+(btnB.w-tw)/2,btnB.y+(btnB.h-th)/2,tw,th};if(t){SDL_RenderCopy(ren,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(su);}
            }
        }

        SDL_RenderPresent(ren);
        SDL_Delay(8);
        (void)mouseClicked;
    }

    if(ctx->puzzTex){SDL_DestroyTexture(ctx->puzzTex);ctx->puzzTex=NULL;}
    SDL_RenderSetLogicalSize(ren,0,0);
    return result;
}

void freeContextPuzzle(ContextPuzzle *ctx)
{
    if(!ctx) return;
    if(ctx->puzzTex){SDL_DestroyTexture(ctx->puzzTex);ctx->puzzTex=NULL;}
    if(ctx->bgTex)  {SDL_DestroyTexture(ctx->bgTex);  ctx->bgTex=NULL;}
}
