/*
 * puzzle.c — TACTICAL DECRYPTION ENGINE: Puzzle Screen
 * Clean version:
 *  - No character animation
 *  - No "TACTICAL DECRYPTION — RECONSTRUCT IMAGE" in top bar
 *  - No "DECRYPTION NODE" header on grid panel
 *  - No "PIECE REPOSITORY..." label on repo strip
 *  - Hint button replaced with a styled icon button (lightbulb glyph)
 *  - Screen shake + red flash on wrong placement
 */

#include "header.h"

/* ─────────────────────────────────────────────────────────────────────────
   PARTICLES
   ───────────────────────────────────────────────────────────────────────── */
static Spark sparks[MAX_SPARKS];
static int   sparkCount=0;

static void spawnSparks(int cx,int cy,int ok){
    int n=ok?40:18;
    for(int i=0;i<n&&sparkCount<MAX_SPARKS;i++){
        Spark *s=&sparks[sparkCount++];
        float ang=(float)i/n*6.283f+((float)(rand()%100))/200.0f;
        float spd=1.8f+((float)(rand()%100))/100.0f*3.5f;
        s->x=(float)cx; s->y=(float)cy;
        s->vx=cosf(ang)*spd; s->vy=sinf(ang)*spd-1.8f;
        s->life=s->maxLife=0.6f+((float)(rand()%60))/100.0f;
        if(ok){s->r=0;s->g=(Uint8)(180+rand()%75);s->b=(Uint8)(100+rand()%155);}
        else  {s->r=255;s->g=(Uint8)(rand()%80);  s->b=(Uint8)(rand()%60);}
    }
}
static void updateSparks(float dt){
    for(int i=0;i<sparkCount;){
        Spark *s=&sparks[i];
        s->x+=s->vx; s->y+=s->vy; s->vy+=0.12f; s->life-=dt;
        if(s->life<=0){sparks[i]=sparks[--sparkCount];}else i++;
    }
}
static void drawSparks(void){
    for(int i=0;i<sparkCount;i++){
        Spark *s=&sparks[i];
        Uint8 a=(Uint8)(s->life/s->maxLife*230);
        SC(s->r,s->g,s->b,a);
        SDL_RenderDrawPoint(g_ren,(int)s->x,  (int)s->y);
        SDL_RenderDrawPoint(g_ren,(int)s->x+1,(int)s->y);
        SDL_RenderDrawPoint(g_ren,(int)s->x,  (int)s->y+1);
        SDL_RenderDrawPoint(g_ren,(int)s->x+1,(int)s->y+1);
    }
}

/* ─────────────────────────────────────────────────────────────────────────
   DUST
   ───────────────────────────────────────────────────────────────────────── */
static Dust dust[MAX_DUST];
static void initDust(void){
    for(int i=0;i<MAX_DUST;i++){
        dust[i].x    =(float)(rand()%WIN_W);
        dust[i].y    =(float)(rand()%WIN_H);
        dust[i].spd  =0.1f+((float)(rand()%20))/40.0f;
        dust[i].phase=(float)(rand()%628)/100.0f;
        dust[i].size =(float)(1+rand()%2);
    }
}
static void drawDust(Uint32 now){
    for(int i=0;i<MAX_DUST;i++){
        Dust *d=&dust[i];
        d->y-=d->spd; if(d->y<0)d->y=(float)WIN_H;
        d->x+=sinf(d->phase+(float)now/2000.0f)*0.3f;
        float bright=0.3f+0.5f*sinf(d->phase+(float)now/1500.0f);
        SC(0,200,255,(Uint8)(bright*48));
        SDL_RenderDrawPoint(g_ren,(int)d->x,(int)d->y);
    }
}

/* ─────────────────────────────────────────────────────────────────────────
   CIRCUITS
   ───────────────────────────────────────────────────────────────────────── */
static void drawCircuits(Uint32 now){
    int pts[][4]={
        {12,340,240,340},{240,340,240,420},{240,420,290,420},
        {12,380,120,380},{120,380,120,460},{12,460,60,460},{60,410,60,460},
        {260,490,260,540},{260,540,310,540},
        {840,490,840,540},{840,540,790,540},
    };
    float pulse=0.3f+0.3f*sinf((float)now/1800.0f);
    SC(0,180,255,(Uint8)(pulse*42));
    for(int i=0;i<(int)(sizeof(pts)/sizeof(pts[0]));i++)
        SDL_RenderDrawLine(g_ren,pts[i][0],pts[i][1],pts[i][2],pts[i][3]);
    SC(0,220,255,(Uint8)(pulse*110));
    int dots[][2]={{240,340},{240,420},{120,380},{60,460},{260,540},{840,540}};
    for(int i=0;i<(int)(sizeof(dots)/sizeof(dots[0]));i++){
        for(int dy=0;dy<2;dy++) for(int dx=0;dx<2;dx++)
            SDL_RenderDrawPoint(g_ren,dots[i][0]+dx,dots[i][1]+dy);
    }
}

/* ─────────────────────────────────────────────────────────────────────────
   SCREEN SHAKE
   ───────────────────────────────────────────────────────────────────────── */
static void shakeUpdate(ScreenShake *sh,float dt){
    if(sh->timeLeft<=0.0f){sh->ox=sh->oy=0;return;}
    sh->timeLeft-=dt;
    float mag=sh->intensity*(sh->timeLeft>0?sh->timeLeft/0.45f:0);
    sh->ox=(int)(((float)(rand()%2001)-1000)/1000.0f*mag);
    sh->oy=(int)(((float)(rand()%2001)-1000)/1000.0f*mag*0.6f);
}
static void shakeTrigger(ScreenShake *sh){sh->intensity=14.0f;sh->timeLeft=0.45f;}

/* ─────────────────────────────────────────────────────────────────────────
   SLICE HELPERS
   ───────────────────────────────────────────────────────────────────────── */
static int          g_puzzW=0,g_puzzH=0;
static SDL_Texture *g_puzzTex=NULL;

static SDL_Rect sliceFor(int slot){
    return (SDL_Rect){
        (slot%GCOLS)*(g_puzzW/GCOLS),(slot/GCOLS)*(g_puzzH/GROWS),
        g_puzzW/GCOLS,g_puzzH/GROWS};
}
static void drawSlice(int pieceIdx,SDL_Rect dst,Uint8 alpha){
    if(!g_puzzTex)return;
    SDL_Rect src=sliceFor(pieceIdx);
    SDL_SetTextureAlphaMod(g_puzzTex,alpha);
    SDL_RenderCopy(g_ren,g_puzzTex,&src,&dst);
    SDL_SetTextureAlphaMod(g_puzzTex,255);
}
static SDL_Texture *makePuzzlePlaceholder(int puzzleIdx){
    g_puzzW=390; g_puzzH=390;
    SDL_Texture *t=SDL_CreateTexture(g_ren,SDL_PIXELFORMAT_RGBA8888,
                                     SDL_TEXTUREACCESS_TARGET,g_puzzW,g_puzzH);
    SDL_SetRenderTarget(g_ren,t);
    SDL_SetRenderDrawColor(g_ren,0,14,34,255); SDL_RenderClear(g_ren);
    Uint8 tints[4][3]={{0,110,170},{20,130,60},{90,40,150},{170,90,0}};
    int cw=g_puzzW/3,ch=g_puzzH/3;
    for(int i=0;i<9;i++){
        SDL_Rect r={(i%3)*cw,(i/3)*ch,cw,ch};
        Uint8 base=40+(i*18);
        SDL_SetRenderDrawColor(g_ren,
            tints[puzzleIdx][0]+base,tints[puzzleIdx][1]+base,
            tints[puzzleIdx][2]+base,255);
        SDL_RenderFillRect(g_ren,&r);
        SDL_SetRenderDrawColor(g_ren,255,255,255,14);
        for(int d=-ch;d<cw;d+=14) SDL_RenderDrawLine(g_ren,r.x+d,r.y,r.x+d+ch,r.y+ch);
        SDL_SetRenderDrawColor(g_ren,0,220,255,55);
        SDL_RenderDrawRect(g_ren,&r);
    }
    SDL_SetRenderDrawColor(g_ren,0,220,255,90);
    SDL_RenderDrawLine(g_ren,cw,0,cw,g_puzzH); SDL_RenderDrawLine(g_ren,2*cw,0,2*cw,g_puzzH);
    SDL_RenderDrawLine(g_ren,0,ch,g_puzzW,ch); SDL_RenderDrawLine(g_ren,0,2*ch,g_puzzW,2*ch);
    SDL_SetRenderTarget(g_ren,NULL);
    return t;
}

/* ─────────────────────────────────────────────────────────────────────────
   SHUFFLE / VALIDATE
   ───────────────────────────────────────────────────────────────────────── */
static void shuffle(int *a,int n){
    for(int i=n-1;i>0;i--){int j=rand()%(i+1),tmp=a[i];a[i]=a[j];a[j]=tmp;}
}
static int countCorrect(Piece *pieces,int *slotOcc){
    int ok=0;
    for(int s=0;s<GPIECES;s++){int pi=slotOcc[s]; if(pi>=0&&pieces[pi].idx==s)ok++;}
    return ok;
}

/* ─────────────────────────────────────────────────────────────────────────
   HINT SYSTEM
   ───────────────────────────────────────────────────────────────────────── */
static char g_hintCipher[32]="";
static int  g_hintSlot=-1,g_hintPiece=-1,g_hintGlow=0;

static void triggerHint(Piece *pieces,int *slotOcc,int *hintsLeft){
    if(*hintsLeft<=0)return;
    (*hintsLeft)--; g_hintGlow=280;
    int candidates[GPIECES],nc=0;
    for(int s=0;s<GPIECES;s++){
        int pi=slotOcc[s];
        if(pi<0||pieces[pi].idx!=s)candidates[nc++]=s;
    }
    if(!nc)return;
    g_hintSlot=candidates[rand()%nc]; g_hintPiece=-1;
    for(int i=0;i<GPIECES;i++) if(pieces[i].idx==g_hintSlot){g_hintPiece=i;break;}
    snprintf(g_hintCipher,sizeof(g_hintCipher),"NODE_%02d",g_hintSlot+1);
}
static void drawHintCipher(Uint32 now,SDL_Rect area){
    (void)now;
    if(g_hintGlow<=0)return;
    float t=(float)g_hintGlow/280.0f;
    char display[32]; int len=(int)strlen(g_hintCipher);
    for(int i=0;i<len;i++)
        display[i]=(((float)(rand()%100))/100.0f<t*0.6f&&g_hintCipher[i]!='_')?
                   'A'+rand()%26:g_hintCipher[i];
    display[len]=0;
    Uint8 alpha=(Uint8)(t*255);
    SDL_Color col={CY_R,CY_G,CY_B,alpha};
    SDL_Surface *su=TTF_RenderText_Blended(g_fntMid,display,col); if(!su)return;
    SDL_Texture *tx=SDL_CreateTextureFromSurface(g_ren,su); SDL_FreeSurface(su); if(!tx)return;
    SDL_SetTextureAlphaMod(tx,alpha);
    int tw,th; SDL_QueryTexture(tx,NULL,NULL,&tw,&th);
    SDL_Rect d={area.x+(area.w-tw)/2,area.y+(area.h-th)/2,tw,th};
    SDL_RenderCopy(g_ren,tx,NULL,&d); SDL_DestroyTexture(tx);
}

/* ─────────────────────────────────────────────────────────────────────────
   HINT ICON BUTTON
   ─────────────────────────────────────────────────────────────────────────
   A round-cornered amber button with a drawn lightbulb glyph.
   When active it pulses, glows and shows charge crystals below.
   When depleted it dims and shows a crossed-out circle.
   ───────────────────────────────────────────────────────────────────────── */
static void renderHintButton(Uint32 now,int hintsLeft,SDL_Rect btn,int mx,int my){
    int hov=(mx>=btn.x&&mx<=btn.x+btn.w&&my>=btn.y&&my<=btn.y+btn.h);
    int active=(hintsLeft>0);
    float pulse=0.5f+0.5f*sinf((float)now/380.0f);

    /* Outer glow when active */
    if(active) GL(btn,AM_R,AM_G,AM_B,hov?1.2f:pulse*0.7f);

    /* Button face */
    Uint8 faceR=active?(Uint8)(20+hov*20):8;
    Uint8 faceG=active?(Uint8)(14+hov*10):8;
    FR(btn,faceR,faceG,0,230);

    /* Border — thick amber when active, dim grey when spent */
    if(active) SR(btn,AM_R,AM_G,AM_B,(Uint8)(180+pulse*75),2);
    else        SR(btn,60,60,60,140,1);
    CO(btn,AM_R,AM_G,active?(Uint8)(pulse*120):0,active?200:80,10);

    /* ── Lightbulb glyph ── */
    int cx=btn.x+btn.w/2, cy=btn.y+btn.h/2-6;
    Uint8 glR=active?(Uint8)(200+pulse*55):70;
    Uint8 glG=active?(Uint8)(140+pulse*40):70;

    if(active){
        /* Warm fill inside bulb */
        FR((SDL_Rect){cx-10,cy-12,20,20},glR,glG,0,(Uint8)(pulse*80));
    }
    /* Bulb circle outline (8 line segments) */
    SC(glR,glG,0,active?220:100);
    int br=11; /* radius */
    for(int seg=0;seg<12;seg++){
        float a1=(float)seg/12*6.283f, a2=(float)(seg+1)/12*6.283f;
        SDL_RenderDrawLine(g_ren,
            cx+(int)(cosf(a1)*br), cy+(int)(sinf(a1)*br),
            cx+(int)(cosf(a2)*br), cy+(int)(sinf(a2)*br));
    }
    /* Bulb base / neck */
    SC(glR,glG,0,active?200:80);
    SDL_RenderDrawLine(g_ren,cx-6,cy+10,cx-6,cy+16);
    SDL_RenderDrawLine(g_ren,cx+6,cy+10,cx+6,cy+16);
    SDL_RenderDrawLine(g_ren,cx-6,cy+16,cx+6,cy+16);
    SDL_RenderDrawLine(g_ren,cx-5,cy+19,cx+5,cy+19);
    SDL_RenderDrawLine(g_ren,cx-4,cy+22,cx+4,cy+22);
    /* Filament spark (only when active) */
    if(active){
        SC(255,220,80,(Uint8)(pulse*220));
        SDL_RenderDrawLine(g_ren,cx-4,cy-4,cx,cy);
        SDL_RenderDrawLine(g_ren,cx,cy,cx+4,cy-4);
        SDL_RenderDrawLine(g_ren,cx-2,cy+4,cx+2,cy-2);
    }
    /* Crossed-out circle when spent */
    if(!active){
        SC(RD_R,RD_G,RD_B,120);
        SDL_RenderDrawLine(g_ren,btn.x+8,btn.y+8,btn.x+btn.w-8,btn.y+btn.h-8);
        SDL_RenderDrawLine(g_ren,btn.x+btn.w-8,btn.y+8,btn.x+8,btn.y+btn.h-8);
    }

    /* "H" shortcut label below the bulb */
    TL("[H]",btn.x+btn.w/2-8,btn.y+btn.h-13,
       active?AM_R:50, active?AM_G:50, 0, g_fntTiny);

    /* ── Charge crystals under button ── */
    int startX=btn.x+(btn.w-MAX_HINTS*22)/2;
    for(int i=0;i<MAX_HINTS;i++){
        int onoff=(i<hintsLeft);
        float cp=0.4f+0.4f*sinf((float)now/400.0f+i*1.1f);
        SDL_Rect crystal={startX+i*24,btn.y+btn.h+8,18,10};
        if(onoff){GL(crystal,PU_R,PU_G,PU_B,cp*0.7f); FR(crystal,40,0,80,210);}
        else       FR(crystal,10,10,20,180);
        SR(crystal,onoff?PU_R:38,onoff?PU_G:18,onoff?PU_B:38,onoff?155:55,1);
        if(onoff){
            SC(PU_R,PU_G,PU_B,(Uint8)(cp*170));
            SDL_RenderDrawLine(g_ren,crystal.x+9,crystal.y+1, crystal.x+17,crystal.y+5);
            SDL_RenderDrawLine(g_ren,crystal.x+17,crystal.y+5,crystal.x+9, crystal.y+9);
            SDL_RenderDrawLine(g_ren,crystal.x+9, crystal.y+9,crystal.x+1, crystal.y+5);
            SDL_RenderDrawLine(g_ren,crystal.x+1, crystal.y+5,crystal.x+9, crystal.y+1);
        }
    }
}

/* ─────────────────────────────────────────────────────────────────────────
   WIN / LOSE OVERLAY
   ───────────────────────────────────────────────────────────────────────── */
static void renderEndOverlay(GameState st,float prog,Uint32 ticks,
                              int *wantRestart,int *wantBack,
                              int mx,int my,int clicked)
{
    Uint8 veil=(Uint8)(prog*210);
    if(st==ST_WIN) FR((SDL_Rect){0,0,WIN_W,WIN_H},0,40,10,veil);
    else           FR((SDL_Rect){0,0,WIN_W,WIN_H},40,0,0,veil);
    if(prog<0.3f)return;

    float pp=(prog-0.3f)/0.7f; if(pp>1)pp=1;
    int panH=260,panY=(int)(WIN_H/2-panH/2-(1-pp)*(WIN_H/2+panH));
    SDL_Rect panel={WIN_W/2-280,panY,560,panH};

    if(st==ST_WIN){
        GL(panel,GR_R,GR_G,GR_B,pp*1.8f); FR(panel,0,10,22,245);
        SR(panel,GR_R,GR_G,GR_B,pp>0.8f?230:170,3); CO(panel,GR_R,GR_G,GR_B,255,22);
    } else {
        GL(panel,RD_R,RD_G,RD_B,pp*1.8f); FR(panel,0,10,22,245);
        SR(panel,RD_R,RD_G,RD_B,pp>0.8f?230:170,3); CO(panel,RD_R,RD_G,RD_B,255,22);
    }
    SC(st==ST_WIN?0:255,st==ST_WIN?255:55,st==ST_WIN?120:55,80);
    SDL_RenderDrawLine(g_ren,panel.x+22,panY+2,panel.x+panel.w-22,panY+2);
    float bp=0.5f+0.5f*sinf((float)ticks/280.0f); Uint8 ba=(Uint8)(160+bp*95);
    if(st==ST_WIN) BAT(WIN_W/2,panY+38,28,GR_R,GR_G,GR_B,ba);
    else           BAT(WIN_W/2,panY+38,28,RD_R,RD_G,RD_B,ba);

    SDL_Rect tr={panel.x,panY+60,panel.w,46};
    if(st==ST_WIN) TC("DECRYPTION SUCCESS",tr,GR_R,GR_G,GR_B,g_fntBig);
    else           TC("DECRYPTION FAILED", tr,RD_R,RD_G,RD_B,g_fntBig);
    SDL_Rect sr2={panel.x,panY+110,panel.w,28};
    if(st==ST_WIN) TC("IMAGE RECONSTRUCTED — FILE UNLOCKED",sr2,0,200,110,g_fntSm);
    else           TC("NEURAL LINK SEVERED — ACCESS DENIED", sr2,210,70,70, g_fntSm);

    SC(CY_R,CY_G,CY_B,55);
    SDL_RenderDrawLine(g_ren,panel.x+30,panY+152,panel.x+panel.w-30,panY+152);

    SDL_Rect btnR={panel.x+54, panY+168,196,52};
    SDL_Rect btnB={panel.x+310,panY+168,196,52};
    int hR=(mx>=btnR.x&&mx<=btnR.x+btnR.w&&my>=btnR.y&&my<=btnR.y+btnR.h);
    int hB=(mx>=btnB.x&&mx<=btnB.x+btnB.w&&my>=btnB.y&&my<=btnB.y+btnB.h);
    if(hR){GL(btnR,CY_R,CY_G,CY_B,1.0f);FR(btnR,0,45,65,245);}
    else   FR(btnR,0,18,38,215);
    SR(btnR,CY_R,CY_G,CY_B,hR?235:115,hR?2:1); CO(btnR,CY_R,CY_G,CY_B,hR?255:135,14);
    TC("START OVER",btnR,CY_R,CY_G,CY_B,g_fntMid);
    if(hB){GL(btnB,0,200,60,1.0f);FR(btnB,0,40,18,245);}
    else   FR(btnB,0,18,10,215);
    SR(btnB,0,hB?225:105,hB?85:40,hB?225:105,hB?2:1); CO(btnB,0,hB?255:125,44,hB?255:125,14);
    TC("GO BACK",btnB,0,hB?255:175,65,g_fntMid);
    if(clicked){if(hR)*wantRestart=1; if(hB)*wantBack=1;}
}

/* ─────────────────────────────────────────────────────────────────────────
   TOP BAR  — timer only, no subtitle text
   ───────────────────────────────────────────────────────────────────────── */
static void renderTopBar(Uint32 now,float timerFrac,GameState state,
                          Uint32 elapsed,Uint8 tR,Uint8 tG)
{
    FR((SDL_Rect){0,0,WIN_W,44},0,6,18,238);
    SC(CY_R,CY_G,CY_B,60); SDL_RenderDrawLine(g_ren,0,44,WIN_W,44);
    SC(CY_R,CY_G,CY_B,20); SDL_RenderDrawLine(g_ren,0,46,WIN_W,46);

    SDL_Rect tbar={60,12,880,16};
    FR(tbar,0,18,38,205); SR(tbar,CY_R,CY_G,CY_B,85,1);
    FR((SDL_Rect){tbar.x,tbar.y,(int)(880*timerFrac),tbar.h},tR,tG,40,230);

    if(timerFrac<0.25f&&state==ST_PLAY){
        float p=0.5f+0.5f*sinf((float)now/180.0f);
        FR(tbar,255,60,0,(Uint8)(p*70));
        SC(255,80,0,(Uint8)(p*180));
        SDL_RenderDrawLine(g_ren,
            tbar.x+(int)(880*timerFrac),tbar.y,
            tbar.x+(int)(880*timerFrac),tbar.y+tbar.h);
    }
    CO(tbar,CY_R,CY_G,CY_B,140,9);


    {float bp=0.5f+0.5f*sinf((float)now/550.0f); BAT(1048,22,14,CY_R,CY_G,CY_B,(Uint8)(80+bp*80));}
}

/* ─────────────────────────────────────────────────────────────────────────
   LEFT REFERENCE PANEL
   ───────────────────────────────────────────────────────────────────────── */
static void renderRefPanel(Uint32 now,int correctCount){
    SDL_Rect lp={8,50,228,280};
    FR(lp,PNL_R,PNL_G,PNL_B,210); SR(lp,CY_R,CY_G,CY_B,88,1); CO(lp,CY_R,CY_G,CY_B,165,12);
    SC(MG_R,MG_G,MG_B,55); SDL_RenderDrawLine(g_ren,9,51,235,51);
    FR((SDL_Rect){8,50,228,20},0,28,62,230);
    TL("REFERENCE",14,52,CY_R,CY_G,CY_B,g_fntSm);

    SDL_Rect refImg={12,72,220,196};
    if(g_puzzTex) SDL_RenderCopy(g_ren,g_puzzTex,NULL,&refImg);
    SL(refImg,18); SR(refImg,CY_R,CY_G,CY_B,70,1);
    {
        float sw=fmodf((float)now/1600.0f,1.0f);
        int sy=refImg.y+(int)(sw*refImg.h);
        SC(CY_R,CY_G,CY_B,38); SDL_RenderDrawLine(g_ren,refImg.x,sy,refImg.x+refImg.w,sy);
        SC(CY_R,CY_G,CY_B,14); SDL_RenderDrawLine(g_ren,refImg.x,sy-1,refImg.x+refImg.w,sy-1);
    }
    SC(CY_R,CY_G,CY_B,45);
    for(int c=1;c<GCOLS;c++)
        SDL_RenderDrawLine(g_ren,refImg.x+c*refImg.w/GCOLS,refImg.y,
                                 refImg.x+c*refImg.w/GCOLS,refImg.y+refImg.h);
    for(int r=1;r<GROWS;r++)
        SDL_RenderDrawLine(g_ren,refImg.x,refImg.y+r*refImg.h/GROWS,
                                 refImg.x+refImg.w,refImg.y+r*refImg.h/GROWS);
    if(g_hintGlow>0&&g_hintSlot>=0){
        float ht=(float)g_hintGlow/280.0f;
        int hx2=refImg.x+(g_hintSlot%GCOLS)*refImg.w/GCOLS;
        int hy2=refImg.y+(g_hintSlot/GCOLS)*refImg.h/GROWS;
        float ap=0.5f+0.5f*sinf((float)now/200.0f);
        FR((SDL_Rect){hx2,hy2,refImg.w/GCOLS,refImg.h/GROWS},AM_R,AM_G,AM_B,(Uint8)(ap*ht*100));
        SR((SDL_Rect){hx2,hy2,refImg.w/GCOLS,refImg.h/GROWS},AM_R,AM_G,AM_B,(Uint8)(ht*220),2);
    }
    {
        char pg[32]; snprintf(pg,32,"%d / %d",correctCount,GPIECES);
        SDL_Rect pl={8,274,228,32};
        FR(pl,0,18,42,220);
        SR(pl,correctCount==GPIECES?GR_R:CY_R,correctCount==GPIECES?GR_G:CY_G,
              correctCount==GPIECES?GR_B:CY_B,85,1);
        if(correctCount>0){
            SDL_Rect fill={pl.x+1,pl.y+1,(pl.w-2)*correctCount/GPIECES,pl.h-2};
            FR(fill,correctCount==GPIECES?GR_R:CY_R,correctCount==GPIECES?GR_G:CY_G,
                    correctCount==GPIECES?GR_B:CY_B,38);
        }
        TC(pg,pl,correctCount==GPIECES?GR_R:CY_R,correctCount==GPIECES?GR_G:CY_G,
                  correctCount==GPIECES?GR_B:CY_B,g_fntMid);
    }
}

/* ─────────────────────────────────────────────────────────────────────────
   CENTRE GRID PANEL  — no header label
   ───────────────────────────────────────────────────────────────────────── */
static void renderGridPanel(Uint32 now,Piece *pieces,int *slotOcc,
                             SDL_Rect *gridSlot,GameState state,float winWaveT,
                             int dragging)
{
    int gpX=GRID_X-16,gpY=GRID_Y-16,gpW=GCOLS*CELL+32,gpH=GROWS*CELL+32;
    FR((SDL_Rect){gpX,gpY,gpW,gpH},PNL_R,PNL_G,PNL_B,192);
    SR((SDL_Rect){gpX,gpY,gpW,gpH},CY_R,CY_G,CY_B,105,2);
    CO((SDL_Rect){gpX,gpY,gpW,gpH},CY_R,CY_G,CY_B,190,16);
    /* No header strip — just corner accents */

    int dash=9,off=(int)(now/50)%(dash*2);
    for(int s=0;s<GPIECES;s++){
        SDL_Rect sr=gridSlot[s];
        if(slotOcc[s]>=0){
            int pi=slotOcc[s];
            SDL_Rect dst={sr.x+1,sr.y+1,CELL-2,CELL-2};
            Uint8 alpha=255;
            if(state==ST_WIN){
                float wave=sinf(winWaveT*4.0f-(float)s*0.5f);
                alpha=(Uint8)(200+wave*55);
                if(wave>0.5f) GL(sr,GR_R,GR_G,GR_B,wave*0.8f);
            }
            drawSlice(pieces[pi].idx,dst,alpha);
            if(pieces[pi].flashT>0){
                Uint8 fa=(Uint8)(pieces[pi].flashT/0.8f*140);
                FR(dst,255,255,255,fa);
            }
            if(state==ST_WIN&&pieces[pi].idx==s){
                float wg=0.3f+0.3f*sinf(winWaveT*3.0f-(float)s*0.6f);
                GL(sr,GR_R,GR_G,GR_B,wg); SR(sr,GR_R,GR_G,GR_B,165,1);
            } else {
                SR(sr,CY_R,CY_G,CY_B,58,1);
            }
        } else {
            drawSlice(s,(SDL_Rect){sr.x+2,sr.y+2,CELL-4,CELL-4},18);
            FR((SDL_Rect){sr.x+1,sr.y+1,CELL-2,CELL-2},0,35,65,42);
            SC(CY_R,CY_G,CY_B,105);
            for(int dx=sr.x+off;dx<sr.x+CELL;dx+=dash*2)
                SDL_RenderDrawLine(g_ren,dx,sr.y,SDL_min(dx+dash,sr.x+CELL),sr.y);
            for(int dx=sr.x+off;dx<sr.x+CELL;dx+=dash*2)
                SDL_RenderDrawLine(g_ren,dx,sr.y+CELL,SDL_min(dx+dash,sr.x+CELL),sr.y+CELL);
            for(int dy=sr.y+off;dy<sr.y+CELL;dy+=dash*2)
                SDL_RenderDrawLine(g_ren,sr.x,dy,sr.x,SDL_min(dy+dash,sr.y+CELL));
            for(int dy=sr.y+off;dy<sr.y+CELL;dy+=dash*2)
                SDL_RenderDrawLine(g_ren,sr.x+CELL,dy,sr.x+CELL,SDL_min(dy+dash,sr.y+CELL));
            {char sn[4]; snprintf(sn,4,"%d",s+1); TC(sn,sr,CY_R,CY_G,CY_B,g_fntTiny);}
            if(g_hintGlow>0&&g_hintSlot==s){
                float ht=(float)g_hintGlow/280.0f;
                float ap=0.5f+0.5f*sinf((float)now/150.0f);
                FR((SDL_Rect){sr.x+1,sr.y+1,CELL-2,CELL-2},AM_R,AM_G,AM_B,(Uint8)(ap*ht*85));
                SR(sr,AM_R,AM_G,AM_B,(Uint8)(ht*240),2);
                GL(sr,AM_R,AM_G,AM_B,ht*1.3f);
            }
        }
    }
    (void)dragging;
}

/* ─────────────────────────────────────────────────────────────────────────
   RIGHT SYSTEM PANEL  — hint button replaced by icon button
   ───────────────────────────────────────────────────────────────────────── */
static void renderSysPanel(Uint32 now,float timerFrac,int correctCount,
                            int hintsLeft,SDL_Rect hintBtn,int mx,int my,
                            Uint8 tR,Uint8 tG)
{
    SDL_Rect rp={920,50,172,490};
    FR(rp,PNL_R,PNL_G,PNL_B,196); SR(rp,CY_R,CY_G,CY_B,84,1); CO(rp,CY_R,CY_G,CY_B,155,12);
    SC(PU_R,PU_G,PU_B,48); SDL_RenderDrawLine(g_ren,921,51,1091,51);
    FR((SDL_Rect){920,50,172,22},0,28,62,230);
    TL("SYS LOG",926,53,CY_R,CY_G,CY_B,g_fntSm);

    /* Integrity bar */
    TL("INTEGRITY",928,78,GR_R,GR_G,GR_B,g_fntTiny);
    {SDL_Rect bg={928,90,150,10},fg={928,90,(int)(150*correctCount/GPIECES),10};
     FR(bg,0,18,38,165); FR(fg,GR_R,GR_G,GR_B,215); SR(bg,CY_R,CY_G,CY_B,52,1);}

    /* Timer bar */
    TL("TIMER",928,108,AM_R,AM_G,AM_B,g_fntTiny);
    {SDL_Rect bg={928,120,150,10},fg={928,120,(int)(150*timerFrac),10};
     FR(bg,0,18,38,165); FR(fg,tR,tG,40,205); SR(bg,CY_R,CY_G,CY_B,52,1);}

    SC(CY_R,CY_G,CY_B,35); SDL_RenderDrawLine(g_ren,928,138,1084,138);

    /* ── Icon hint button ── */
    renderHintButton(now,hintsLeft,hintBtn,mx,my);

    SC(CY_R,CY_G,CY_B,32); SDL_RenderDrawLine(g_ren,928,290,1084,290);

    /* Cipher display */
    TL("CIPHER:",928,296,CY_R,CY_G,CY_B,g_fntTiny);
    SDL_Rect hintDisplay={924,308,156,32};
    FR(hintDisplay,0,14,30,205); SR(hintDisplay,CY_R,CY_G,CY_B,58,1);
    if(g_hintGlow>0){
        drawHintCipher(now,hintDisplay);
        TL("ANALYZING...",hintDisplay.x+5,hintDisplay.y+hintDisplay.h+4,255,160,0,g_fntTiny);
    } else {
        TC("— AWAITING —",hintDisplay,CY_R,CY_G,CY_B,g_fntTiny);
    }

    SC(CY_R,CY_G,CY_B,30); SDL_RenderDrawLine(g_ren,928,360,1076,360);
    TL("PLACEMENT:",928,366,CY_R,CY_G,CY_B,g_fntTiny);
    {
        char log1[32],log2[32];
        snprintf(log1,32,"OK:  %d",correctCount);
        snprintf(log2,32,"LEFT:%d",GPIECES-correctCount);
        TL(log1,928,380,GR_R,GR_G,GR_B,g_fntTiny);
        TL(log2,928,394,AM_R,AM_G,AM_B,g_fntTiny);
    }

    SC(CY_R,CY_G,CY_B,28); SDL_RenderDrawLine(g_ren,928,412,1076,412);
    TL("AUDIO:",928,418,CY_R,CY_G,CY_B,g_fntTiny);
    {
        float mp=0.5f+0.5f*sinf((float)now/400.0f);
        for(int b=0;b<5;b++){
            int bh=3+(int)(mp*(2+b)*3);
            FR((SDL_Rect){972+b*10,434-bh,7,bh},
               CY_R,(Uint8)(CY_G*mp),(Uint8)(CY_B*0.8f),(Uint8)(120+mp*135));
        }
    }
    {float bp=0.5f+0.5f*sinf((float)now/700.0f); BAT(1004,455,12,CY_R,CY_G,CY_B,(Uint8)(42+bp*55));}
    {float bp=0.5f+0.5f*sinf((float)now/900.0f+0.5f); BAT(986,475,9,PU_R,PU_G,PU_B,(Uint8)(32+bp*44));}
    {float bp=0.5f+0.5f*sinf((float)now/600.0f+1.0f); BAT(1022,475,9,PU_R,PU_G,PU_B,(Uint8)(32+bp*44));}
}

/* ─────────────────────────────────────────────────────────────────────────
   REPO STRIP  — no bottom label
   ───────────────────────────────────────────────────────────────────────── */
static void renderRepo(Uint32 now,Piece *pieces,int dragging,int repoStartX){
    int totalW=GPIECES*(RPW+RGAP)-RGAP;
    SDL_Rect repoBg={repoStartX-14,REPO_Y-14,totalW+28,RPH+28};
    FR(repoBg,PNL_R,PNL_G,PNL_B,205);
    SR(repoBg,CY_R,CY_G,CY_B,105,2);
    CO(repoBg,CY_R,CY_G,CY_B,185,12);
    SC(PU_R,PU_G,PU_B,42);
    SDL_RenderDrawLine(g_ren,repoBg.x+1,repoBg.y+1,repoBg.x+repoBg.w-1,repoBg.y+1);
    /* No bottom label */

    for(int i=0;i<GPIECES;i++){
        if(pieces[i].slot>=0||i==dragging)continue;
        SDL_Rect r=pieces[i].cur; r.x+=(int)pieces[i].shakeX;
        int isHinted=(g_hintGlow>0&&i==g_hintPiece);
        if(isHinted){
            float ht=(float)g_hintGlow/280.0f;
            float ap=0.5f+0.5f*sinf((float)now/160.0f);
            GL(r,AM_R,AM_G,AM_B,ap*ht*2.7f);
            SR(r,AM_R,AM_G,AM_B,(Uint8)(ht*245),2);
            CO(r,AM_R,AM_G,AM_B,(Uint8)(ht*255),9);
        } else {
            SR(r,CY_R,CY_G,CY_B,68,1);
        }
        FR(r,0,8,22,215); drawSlice(pieces[i].idx,r,252); SL(r,14);
        SDL_Rect nt={r.x+r.w/2-7,r.y-5,14,5};
        SDL_Rect nb={r.x+r.w/2-7,r.y+r.h,14,5};
        SDL_Rect nl={r.x-5,r.y+r.h/2-7,5,14};
        SDL_Rect nr2={r.x+r.w,r.y+r.h/2-7,5,14};
        FR(nt,0,12,30,185); SR(nt,CY_R,CY_G,CY_B,58,1);
        FR(nb,0,12,30,185); SR(nb,CY_R,CY_G,CY_B,58,1);
        FR(nl,0,12,30,185); SR(nl,CY_R,CY_G,CY_B,58,1);
        FR(nr2,0,12,30,185); SR(nr2,CY_R,CY_G,CY_B,58,1);
        if(pieces[i].shakeX!=0)
            FR(r,255,0,0,(Uint8)(fabsf(pieces[i].shakeX)*6));
    }
}

/* ─────────────────────────────────────────────────────────────────────────
   BACK BUTTON
   ───────────────────────────────────────────────────────────────────────── */
static void renderBackBtn(int hov){
    SDL_Rect r={10,558,86,32};
    if(hov){GL(r,GR_R,GR_G,GR_B,0.75f);FR(r,0,34,14,238);}
    else    FR(r,0,16,10,210);
    SR(r,0,hov?225:115,42,hov?225:115,hov?2:1);
    CO(r,0,hov?255:134,50,hov?255:134,7);
    BAT(r.x-13,r.y+r.h/2,9,0,180,60,190);
    TC("BACK",r,0,hov?255:185,62,g_fntMid);
}

/* ─────────────────────────────────────────────────────────────────────────
   RED FLASH
   ───────────────────────────────────────────────────────────────────────── */
static void renderWrongFlash(float t){
    if(t<=0)return; if(t>1)t=1;
    int border=(int)(t*32);
    for(int i=0;i<border;i++){
        Uint8 ia=(Uint8)(t*200*(border-i)/border);
        SC(255,0,0,ia);
        SDL_RenderDrawLine(g_ren,i,0,i,WIN_H);
        SDL_RenderDrawLine(g_ren,WIN_W-1-i,0,WIN_W-1-i,WIN_H);
        SDL_RenderDrawLine(g_ren,0,i,WIN_W,i);
        SDL_RenderDrawLine(g_ren,0,WIN_H-1-i,WIN_W,WIN_H-1-i);
    }
    FR((SDL_Rect){0,0,WIN_W,WIN_H},255,0,0,(Uint8)(t*50));
}

/* ─────────────────────────────────────────────────────────────────────────
   RUN PUZZLE
   ───────────────────────────────────────────────────────────────────────── */
int run_puzzle(SDL_Window *win,int puzzleIdx){
    (void)win;

    SDL_Texture *bgTex=IMG_LoadTexture(g_ren,"assets/bg.png");
    if(!bgTex){
        bgTex=SDL_CreateTexture(g_ren,SDL_PIXELFORMAT_RGBA8888,SDL_TEXTUREACCESS_TARGET,WIN_W,WIN_H);
        SDL_SetRenderTarget(g_ren,bgTex);
        SDL_SetRenderDrawColor(g_ren,0,4,12,255); SDL_RenderClear(g_ren);
        for(int y=0;y<WIN_H;y++){
            float t2=(float)y/WIN_H; Uint8 b=(Uint8)(4+t2*22);
            SDL_SetRenderDrawColor(g_ren,0,b/3,b,255);
            SDL_RenderDrawLine(g_ren,0,y,WIN_W,y);
        }
        SDL_SetRenderDrawColor(g_ren,0,50,80,38);
        for(int y=0;y<WIN_H;y+=32) for(int x=0;x<WIN_W;x+=32)
            SDL_RenderDrawPoint(g_ren,x,y);
        SDL_SetRenderTarget(g_ren,NULL);
    }

    g_puzzTex=IMG_LoadTexture(g_ren,PUZZLE_IMAGES[puzzleIdx]);
    if(g_puzzTex) SDL_QueryTexture(g_puzzTex,NULL,NULL,&g_puzzW,&g_puzzH);
    else           g_puzzTex=makePuzzlePlaceholder(puzzleIdx);

    initDust(); sparkCount=0;

    SDL_Rect gridSlot[GPIECES];
    for(int i=0;i<GPIECES;i++)
        gridSlot[i]=(SDL_Rect){GRID_X+(i%GCOLS)*CELL,GRID_Y+(i/GCOLS)*CELL,CELL,CELL};
    int slotOcc[GPIECES];
    for(int i=0;i<GPIECES;i++) slotOcc[i]=-1;

    int order[GPIECES];
    for(int i=0;i<GPIECES;i++) order[i]=i;
    shuffle(order,GPIECES);

    int totalW=GPIECES*(RPW+RGAP)-RGAP;
    int repoStartX=(WIN_W-totalW)/2;

    Piece pieces[GPIECES];
    for(int i=0;i<GPIECES;i++){
        int rx=repoStartX+i*(RPW+RGAP);
        pieces[i].idx=order[i]; pieces[i].slot=-1;
        pieces[i].home=(SDL_Rect){rx,REPO_Y,RPW,RPH};
        pieces[i].cur=pieces[i].home;
        pieces[i].shakeX=0; pieces[i].shakeTmr=0;
        pieces[i].flashT=0; pieces[i].flashOk=0;
    }

    GameState   state       =ST_PLAY;
    Uint32      startT      =SDL_GetTicks();
    int         hintsLeft   =MAX_HINTS;
    int         dragging    =-1,dragOX=0,dragOY=0;
    int         correctCount=0;
    float       endProgress =0.0f,winWaveT=0.0f;
    int         wantRestart =0,wantBack=0,result=0;
    ScreenShake shake       ={0,0,0,0};

    g_hintGlow=0; g_hintSlot=-1; g_hintPiece=-1; g_hintCipher[0]=0;

    /* Hint button — square icon style in the right panel */
    SDL_Rect hintBtn={934,148,152,120};
    SDL_Rect backBtn={10,558,86,32};

    SDL_Event ev;
    Uint32 lastTick=SDL_GetTicks();
    int running=1;

    while(running){
        Uint32 now  =SDL_GetTicks();
        Uint32 dtMs =(Uint32)(now-lastTick);
        float  dt   =(float)dtMs/1000.0f; if(dt>0.05f){dt=0.05f;dtMs=50;}
        lastTick=now;

        int mouseClicked=0,mx,my;
        SDL_GetMouseState(&mx,&my);

        while(SDL_PollEvent(&ev)){
            if(ev.type==SDL_QUIT){result=-1;running=0;}
            if(ev.type==SDL_KEYDOWN){
                if(ev.key.keysym.sym==SDLK_ESCAPE){result=0;running=0;}
                if(ev.key.keysym.sym==SDLK_r&&state!=ST_PLAY)wantRestart=1;
                if(ev.key.keysym.sym==SDLK_h&&state==ST_PLAY)
                    triggerHint(pieces,slotOcc,&hintsLeft);
            }
            if(ev.type==SDL_MOUSEBUTTONDOWN&&ev.button.button==SDL_BUTTON_LEFT){
                mouseClicked=1;
                int bx=ev.button.x,by=ev.button.y;
                if(state==ST_PLAY&&bx>=hintBtn.x&&bx<=hintBtn.x+hintBtn.w&&
                   by>=hintBtn.y&&by<=hintBtn.y+hintBtn.h)
                    triggerHint(pieces,slotOcc,&hintsLeft);
                if(state==ST_PLAY&&dragging<0){
                    for(int i=0;i<GPIECES;i++){
                        if(pieces[i].slot>=0)continue;
                        SDL_Rect r=pieces[i].cur;
                        if(bx>=r.x&&bx<=r.x+r.w&&by>=r.y&&by<=r.y+r.h){
                            dragging=i;dragOX=bx-r.x;dragOY=by-r.y;break;
                        }
                    }
                    if(dragging<0){
                        for(int s=0;s<GPIECES;s++){
                            int pi=slotOcc[s]; if(pi<0)continue;
                            SDL_Rect r=pieces[pi].cur;
                            if(bx>=r.x&&bx<=r.x+r.w&&by>=r.y&&by<=r.y+r.h){
                                slotOcc[s]=-1;pieces[pi].slot=-1;
                                dragging=pi;dragOX=bx-r.x;dragOY=by-r.y;
                                correctCount=countCorrect(pieces,slotOcc);break;
                            }
                        }
                    }
                }
            }
            if(ev.type==SDL_MOUSEMOTION&&dragging>=0){
                pieces[dragging].cur.x=ev.motion.x-dragOX;
                pieces[dragging].cur.y=ev.motion.y-dragOY;
            }
            if(ev.type==SDL_MOUSEBUTTONUP&&ev.button.button==SDL_BUTTON_LEFT&&dragging>=0){
                int pcx=pieces[dragging].cur.x+RPW/2;
                int pcy=pieces[dragging].cur.y+RPH/2;
                int best=-1; float bestD=1e9f;
                for(int s=0;s<GPIECES;s++){
                    if(slotOcc[s]>=0)continue;
                    float dx2=(float)(pcx-(gridSlot[s].x+CELL/2));
                    float dy2=(float)(pcy-(gridSlot[s].y+CELL/2));
                    float d=dx2*dx2+dy2*dy2;
                    if(d<(float)(CELL*CELL)*0.55f&&d<bestD){bestD=d;best=s;}
                }
                if(best>=0){
                    slotOcc[best]=dragging;
                    pieces[dragging].slot=best;
                    pieces[dragging].cur=(SDL_Rect){gridSlot[best].x+1,gridSlot[best].y+1,CELL-2,CELL-2};
                    int wasCorrect=(pieces[dragging].idx==best);
                    pieces[dragging].flashT=0.8f; pieces[dragging].flashOk=wasCorrect;
                    spawnSparks(gridSlot[best].x+CELL/2,gridSlot[best].y+CELL/2,wasCorrect);
                    correctCount=countCorrect(pieces,slotOcc);
                    if(correctCount==GPIECES){state=ST_WIN;winWaveT=0;}
                    if(!wasCorrect) shakeTrigger(&shake);
                    printf("[BACKEND] Piece %d → slot %d — %s (%d/9)\n",
                           pieces[dragging].idx,best,wasCorrect?"OK":"WRONG",correctCount);
                } else {
                    pieces[dragging].cur=pieces[dragging].home;
                }
                dragging=-1;
            }
        }

        if(wantRestart){
            state=ST_PLAY; startT=SDL_GetTicks(); hintsLeft=MAX_HINTS;
            g_hintGlow=0;g_hintSlot=-1;g_hintPiece=-1;g_hintCipher[0]=0;
            dragging=-1;correctCount=0;endProgress=0;winWaveT=0;
            wantRestart=0;wantBack=0;sparkCount=0;
            shake.timeLeft=0;shake.ox=shake.oy=0;
            for(int i=0;i<GPIECES;i++)slotOcc[i]=-1;
            shuffle(order,GPIECES);
            for(int i=0;i<GPIECES;i++){
                int rx=repoStartX+i*(RPW+RGAP);
                pieces[i].idx=order[i];pieces[i].slot=-1;
                pieces[i].home=(SDL_Rect){rx,REPO_Y,RPW,RPH};
                pieces[i].cur=pieces[i].home;
                pieces[i].shakeX=0;pieces[i].shakeTmr=0;pieces[i].flashT=0;
            }
        }
        if(wantBack){result=0;running=0;}

        Uint32 elapsed=(Uint32)(now-startT);
        float timerFrac=(state==ST_PLAY)?1.0f-(float)elapsed/TIMER_MS
                                        :(correctCount==GPIECES?1.0f:0.0f);
        if(timerFrac<0)timerFrac=0;
        if(state==ST_PLAY&&elapsed>=TIMER_MS)state=ST_LOSE;
        if(state!=ST_PLAY&&endProgress<1.0f)endProgress+=dt*0.65f;
        if(endProgress>1.0f)endProgress=1.0f;
        if(state==ST_WIN)winWaveT+=dt;

        Uint8 tR=(Uint8)(255*(1-timerFrac));
        Uint8 tG=(Uint8)(210*timerFrac);

        for(int i=0;i<GPIECES;i++){
            if(pieces[i].shakeTmr>0){
                pieces[i].shakeTmr-=dt*60;
                pieces[i].shakeX=sinf(pieces[i].shakeTmr*1.4f)*7.0f*(pieces[i].shakeTmr/22.0f);
            } else pieces[i].shakeX=0;
            if(pieces[i].flashT>0)pieces[i].flashT-=dt;
        }
        if(g_hintGlow>0)g_hintGlow--;
        shakeUpdate(&shake,dt);
        updateSparks(dt);

        /* ══ RENDER ══ */
        SDL_SetRenderDrawColor(g_ren,0,0,0,255);
        SDL_RenderClear(g_ren);

        SDL_Rect vp={shake.ox,shake.oy,WIN_W,WIN_H};
        SDL_RenderSetViewport(g_ren,&vp);

        SDL_RenderCopy(g_ren,bgTex,NULL,&(SDL_Rect){0,0,WIN_W,WIN_H});
        FR((SDL_Rect){0,0,WIN_W,WIN_H},0,6,16,105);

        drawCircuits(now);
        drawDust(now);
        renderTopBar(now,timerFrac,state,elapsed,tR,tG);
        renderRefPanel(now,correctCount);
        renderGridPanel(now,pieces,slotOcc,gridSlot,state,winWaveT,dragging);
        renderSysPanel(now,timerFrac,correctCount,hintsLeft,hintBtn,mx,my,tR,tG);
        renderRepo(now,pieces,dragging,repoStartX);

        if(dragging>=0){
            SDL_Rect r=pieces[dragging].cur;
            GL(r,CY_R,CY_G,CY_B,1.8f); FR(r,0,28,55,215);
            drawSlice(pieces[dragging].idx,r,250);
            SL(r,10); SR(r,CY_R,CY_G,CY_B,255,2); CO(r,CY_R,CY_G,CY_B,255,12);
            if(rand()%3==0&&sparkCount<MAX_SPARKS){
                Spark *sp=&sparks[sparkCount++];
                sp->x=(float)(r.x+rand()%r.w); sp->y=(float)(r.y+rand()%r.h);
                sp->vx=((float)(rand()%100)-50)/80.0f; sp->vy=-((float)(rand()%100))/80.0f;
                sp->life=sp->maxLife=0.3f; sp->r=0;sp->g=200;sp->b=255;
            }
        }
        drawSparks();

        if(shake.timeLeft>0) renderWrongFlash(shake.timeLeft/0.45f);

        {
            int backHov=(mx>=backBtn.x&&mx<=backBtn.x+backBtn.w&&
                         my>=backBtn.y&&my<=backBtn.y+backBtn.h);
            renderBackBtn(backHov);
            if(backHov&&mouseClicked){result=0;running=0;}
        }

        FR((SDL_Rect){0,WIN_H-20,WIN_W,20},0,6,18,230);
        SC(CY_R,CY_G,CY_B,45); SDL_RenderDrawLine(g_ren,0,WIN_H-20,WIN_W,WIN_H-20);
        SC(PU_R,PU_G,PU_B,24); SDL_RenderDrawLine(g_ren,0,WIN_H-21,WIN_W,WIN_H-21);

        SDL_RenderSetViewport(g_ren,NULL);

        if(state!=ST_PLAY)
            renderEndOverlay(state,endProgress,now,&wantRestart,&wantBack,mx,my,mouseClicked);

        SDL_RenderPresent(g_ren);
        SDL_Delay(8);
    }

    SDL_DestroyTexture(bgTex);
    if(g_puzzTex){SDL_DestroyTexture(g_puzzTex);g_puzzTex=NULL;}
    return result;
}
