#include "enigme.h"

void jouerSon(Enigme *e, int id)
{
    if (!e) return;
    if (e->currentSound == id && Mix_PlayingMusic()) return;
    Mix_HaltMusic();
    e->currentSound = id;
    if (id == SND_BAT && e->soundBat)
        { Mix_VolumeMusic(70); Mix_PlayMusic(e->soundBat, -1); }
    else if (id == SND_SUSPENSE && e->soundSuspense)
        { Mix_VolumeMusic(MIX_MAX_VOLUME); Mix_PlayMusic(e->soundSuspense, 0); }
    else if (id == SND_CORRECT && e->soundCorrect)
        { Mix_VolumeMusic(MIX_MAX_VOLUME); Mix_PlayMusic(e->soundCorrect, 0); }
    else if (id == SND_WRONG && e->soundWrong)
        { Mix_VolumeMusic(MIX_MAX_VOLUME); Mix_PlayMusic(e->soundWrong, 0); }
}

void initEnigme(Enigme *e, SDL_Renderer *renderer, int scrW, int scrH)
{
    int i, nb, n, bonus, len, count;
    int pool[MAX_QUESTIONS], tmp, j;
    FILE *f;
    char line[512], buf[512], *tok[5];
    SDL_Surface *s;

    if (!e || !renderer) return;
    srand((unsigned)time(NULL));
    memset(e, 0, sizeof(Enigme));
    e->scrW = scrW;
    e->scrH = scrH;

    f = fopen("assets/assets/questions.txt", "r");
    count = 0;
    if (f) {
        while (fgets(line, sizeof(line), f) && count < MAX_QUESTIONS) {
            len = (int)strlen(line);
            while (len > 0 && (line[len-1]=='\n'||line[len-1]=='\r')) line[--len]='\0';
            if (len == 0) continue;
            strncpy(buf, line, sizeof(buf)-1); buf[sizeof(buf)-1]='\0';
            tok[0]=strtok(buf,"|"); tok[1]=strtok(NULL,"|");
            tok[2]=strtok(NULL,"|"); tok[3]=strtok(NULL,"|"); tok[4]=strtok(NULL,"|");
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
    }
    e->nbQuestionsPool = count;

    nb = 0;
    for (i=0; i<e->nbQuestionsPool; i++) { e->questions[i].deja_vu=0; pool[nb++]=i; }
    for (i=nb-1; i>0; i--) { j=rand()%(i+1); tmp=pool[i]; pool[i]=pool[j]; pool[j]=tmp; }
    n = (nb < NB_QUESTIONS) ? nb : NB_QUESTIONS;
    for (i=0; i<n; i++) { e->ordreJeu[i]=pool[i]; e->questions[pool[i]].deja_vu=1; }

    e->questionIndex=0; e->score=0; e->niveau=1;
    e->selected=-1; e->hovered=-1;
    e->chronoSecondes=CHRONO_SECS; e->chronoLastTick=SDL_GetTicks();
    e->chronoRatio=1.0f; e->currentSound=SND_NONE;

    s=IMG_Load("assets/assets/background.png");
    e->bgTexture=s?SDL_CreateTextureFromSurface(renderer,s):NULL; if(s)SDL_FreeSurface(s);
    s=IMG_Load("assets/assets/carte.png");
    e->cardTexture=s?SDL_CreateTextureFromSurface(renderer,s):NULL; if(s)SDL_FreeSurface(s);
    for (i=0; i<NB_PROPS; i++) {
        s=IMG_Load("assets/assets/carte.png");
        e->propTexture[i]=s?SDL_CreateTextureFromSurface(renderer,s):NULL; if(s)SDL_FreeSurface(s);
    }
    s=IMG_Load("assets/assets/BatLogo.png");
    e->batLogoTex=s?SDL_CreateTextureFromSurface(renderer,s):NULL; if(s)SDL_FreeSurface(s);

    e->soundBat      = Mix_LoadMUS("assets/assets/bat.mp3");
    e->soundSuspense = Mix_LoadMUS("assets/assets/suspince.mp3");
    e->soundCorrect  = Mix_LoadMUS("assets/assets/correct.mp3");
    e->soundWrong    = Mix_LoadMUS("assets/assets/ghalet.mp3");

    {
        int W=scrW, H=scrH;
        int hudH=H*8/100; if(hudH<100)hudH=100; if(hudH>130)hudH=130;
        e->hudH=hudH;
        int marginX=W*2/100; if(marginX<20)marginX=20;
        int areaX=marginX, areaY=hudH+8, areaW=W-2*marginX, areaH=H-areaY-10;
        int cardW=areaW*28/100, cardH=cardW*3/2;
        if(cardH>areaH*90/100) cardH=areaH*90/100;
        e->cardRect=(SDL_Rect){areaX, areaY+(areaH-cardH)/2, cardW, cardH};
        e->cardAngle=-6.0;
        int gap=areaW*2/100, rightX=areaX+cardW+gap, rightW=areaX+areaW-rightX;
        int propW=rightW*32/100, propH=propW*17/10;
        if(propH>areaH*48/100) propH=areaH*48/100;
        int colGap=rightW-2*propW; if(colGap<10)colGap=10;
        int colL=rightX, colR=rightX+propW+colGap, colC=rightX+(rightW-propW)/2;
        int rowTop=areaY+8, rowBot=areaY+areaH-propH-8;
        e->propRect[0]=(SDL_Rect){colL,rowTop,propW,propH}; e->propAngle[0]=-5.0;
        e->propRect[1]=(SDL_Rect){colR,rowTop,propW,propH}; e->propAngle[1]=+5.0;
        e->propRect[2]=(SDL_Rect){colC,rowBot,propW,propH}; e->propAngle[2]=-3.0;
        int hudRS=W*68/100, hudRW=W-marginX-hudRS;
        int logoSz=hudH*85/100;
        e->batLogoRect=(SDL_Rect){W-marginX-logoSz,(hudH-logoSz)/2,logoSz,logoSz};
        int barH=hudH*25/100; if(barH<16)barH=16; if(barH>38)barH=38;
        int barW=hudRW-logoSz-8; if(barW<40)barW=40;
        e->chronoRect=(SDL_Rect){hudRS,(hudH-barH)/2,barW,barH};
        bonus=(e->niveau-1)*2; e->chronoSecondes=CHRONO_SECS-bonus;
        if(e->chronoSecondes<5)e->chronoSecondes=5;
    }
    jouerSon(e, SND_BAT);
}

void updateEnigme(Enigme *e)
{
    int bonus;
    Uint32 now;
    if (!e || e->questionIndex >= NB_QUESTIONS) return;
    now = SDL_GetTicks();

    if (e->currentSound == SND_BAT && !Mix_PlayingMusic()) jouerSon(e, SND_BAT);

    if (!e->showResult && !e->waitingSuspense && !e->chronoExpire) {
        if (now - e->chronoLastTick >= 1000) {
            e->chronoLastTick = now;
            e->chronoSecondes--;
            if (e->chronoSecondes <= 0) {
                e->chronoSecondes=0; e->chronoExpire=1;
                e->selected=-1; e->correct=0;
                e->waitingSuspense=1; e->suspenseTime=now;
                jouerSon(e, SND_SUSPENSE);
            }
        }
    }
    if (e->waitingSuspense && e->suspenseTime>0 && now-e->suspenseTime>=SUSPENSE_DELAY) {
        e->waitingSuspense=0; e->showResult=1; e->resultTime=now;
        if (e->correct) e->score++;
        jouerSon(e, e->correct ? SND_CORRECT : SND_WRONG);
    }
    if (e->showResult && e->resultTime>0 && now-e->resultTime>=RESULT_DELAY) {
        e->questionIndex++;
        e->selected=-1; e->hovered=-1; e->showResult=0; e->correct=0;
        e->resultTime=0; e->waitingSuspense=0; e->suspenseTime=0; e->chronoExpire=0;
        if (e->questionIndex>0 && e->questionIndex%QUESTIONS_PAR_NIVEAU==0) e->niveau++;
        bonus=(e->niveau-1)*2; e->chronoSecondes=CHRONO_SECS-bonus;
        if(e->chronoSecondes<5)e->chronoSecondes=5;
        e->chronoLastTick=SDL_GetTicks(); e->chronoRatio=1.0f;
        jouerSon(e, SND_BAT);
    }
    e->chronoRatio=(float)e->chronoSecondes/(float)CHRONO_SECS;
    if(e->chronoRatio<0.0f)e->chronoRatio=0.0f;
    if(e->chronoRatio>1.0f)e->chronoRatio=1.0f;
}

void renderEnigme(Enigme *e, SDL_Renderer *renderer,
                  TTF_Font *font, TTF_Font *fontSmall, TTF_Font *fontTiny)
{
    int W, H, hudH, marginX, i, fillW, tw, th;
    float ratio, pulse, zoom2, gp2, lp2, speed, nowF;
    Uint8 tR, tG, mod, r2, gb;
    char buf[64];
    SDL_Rect hud, d, qZone, tZone, barRect, fill, glow, bg2, lr;
    SDL_Surface *s;
    SDL_Texture *t;
    SDL_Color cyan, yellow, black, col;
    QuestionData *q;
    Uint32 now2, nowB;

    if (!e || !renderer || !font) return;
    if (!fontSmall) fontSmall = font;
    if (!fontTiny)  fontTiny  = fontSmall;

    W = e->scrW; H = e->scrH;
    hudH = e->hudH;
    marginX = W*2/100; if(marginX<20)marginX=20;

    if (e->bgTexture) SDL_RenderCopy(renderer, e->bgTexture, NULL, NULL);

    if (e->questionIndex >= NB_QUESTIONS) {
        SDL_Color white={255,255,255,255};
        SDL_Rect r={0,0,W,H};
        int pad=14, wrap=r.w-2*pad; if(wrap<10)wrap=10;
        s=TTF_RenderUTF8_Blended_Wrapped(fontSmall,"Well done! Quiz complete!",white,wrap);
        if(s){t=SDL_CreateTextureFromSurface(renderer,s);
            d=(SDL_Rect){r.x+(r.w-s->w)/2,r.y+(r.h-s->h)/2,s->w,s->h};
            if(t){SDL_RenderCopy(renderer,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(s);}
        return;
    }

    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer,0,0,10,180);
    hud=(SDL_Rect){0,0,W,hudH}; SDL_RenderFillRect(renderer,&hud);
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);

    cyan=(SDL_Color){0,220,255,255};
    yellow=(SDL_Color){255,220,0,255};

    snprintf(buf,sizeof(buf),"SCORE: %d",e->score);
    s=TTF_RenderUTF8_Blended(font,buf,cyan);
    if(s){t=SDL_CreateTextureFromSurface(renderer,s);
        d=(SDL_Rect){marginX,(hudH-s->h)/2,s->w,s->h};
        if(t){SDL_RenderCopy(renderer,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(s);}

    now2=SDL_GetTicks();
    zoom2=1.0f+0.08f*sinf((float)now2/700.0f);
    gp2=0.5f+0.5f*sinf((float)now2/700.0f);
    lp2=0.5f+0.5f*sinf((float)now2/500.0f);
    snprintf(buf,sizeof(buf),"LEVEL %d",e->niveau);
    s=TTF_RenderUTF8_Blended(font,buf,cyan);
    if(s){t=SDL_CreateTextureFromSurface(renderer,s);
        int tw2=(int)(s->w*zoom2),th2=(int)(s->h*zoom2);
        int ty2=(hudH-th2)/2-4; if(ty2<2)ty2=2;
        SDL_Rect d2={W/2-tw2/2,ty2,tw2,th2};
        SDL_Rect glowR2={d2.x-20,d2.y-4,tw2+40,th2+8};
        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer,0,220,255,(Uint8)(gp2*18));
        SDL_RenderFillRect(renderer,&glowR2);
        if(t){SDL_SetTextureAlphaMod(t,(Uint8)(200+gp2*55));SDL_RenderCopy(renderer,t,NULL,&d2);SDL_DestroyTexture(t);}
        SDL_FreeSurface(s);
        int ly2=d2.y+th2/2;
        SDL_SetRenderDrawColor(renderer,0,220,255,(Uint8)(lp2*80));
        SDL_RenderDrawLine(renderer,W/2-tw2/2-40,ly2,W/2-tw2/2-10,ly2);
        SDL_RenderDrawLine(renderer,W/2+tw2/2+10,ly2,W/2+tw2/2+40,ly2);
        SDL_SetRenderDrawColor(renderer,0,220,255,(Uint8)(lp2*160));
        SDL_RenderDrawLine(renderer,W/2-tw2/2-18,ly2-5,W/2-tw2/2-10,ly2);
        SDL_RenderDrawLine(renderer,W/2-tw2/2-10,ly2,W/2-tw2/2-18,ly2+5);
        SDL_RenderDrawLine(renderer,W/2+tw2/2+18,ly2-5,W/2+tw2/2+10,ly2);
        SDL_RenderDrawLine(renderer,W/2+tw2/2+10,ly2,W/2+tw2/2+18,ly2+5);
    }

    snprintf(buf,sizeof(buf),"%02d/%d",e->questionIndex+1,NB_QUESTIONS);
    s=TTF_RenderUTF8_Blended(fontSmall,buf,yellow);
    if(s){t=SDL_CreateTextureFromSurface(renderer,s);
        d=(SDL_Rect){W/2-s->w/2,hudH-s->h-4,s->w,s->h};
        if(t){SDL_RenderCopy(renderer,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(s);}

    if(e->batLogoTex){
        ratio=e->chronoRatio; nowB=SDL_GetTicks();
        int fullSz=e->batLogoRect.w;
        int sz=(int)(fullSz*(0.4f+0.6f*ratio));
        int cx=e->batLogoRect.x+fullSz/2, cy=e->batLogoRect.y+fullSz/2;
        lr=(SDL_Rect){cx-sz/2,cy-sz/2,sz,sz};
        speed=300.0f+(1.0f-ratio)*1200.0f;
        pulse=0.5f+0.5f*sinf((float)nowB/speed);
        if(ratio>0.25f){mod=(Uint8)(180+pulse*75);SDL_SetTextureColorMod(e->batLogoTex,mod,mod,mod);SDL_SetTextureAlphaMod(e->batLogoTex,255);}
        else{r2=(Uint8)(200+pulse*55);gb=(Uint8)(pulse*80);SDL_SetTextureColorMod(e->batLogoTex,r2,gb,gb);SDL_SetTextureAlphaMod(e->batLogoTex,(Uint8)(180+pulse*75));}
        SDL_RenderCopy(renderer,e->batLogoTex,NULL,&lr);
        SDL_SetTextureColorMod(e->batLogoTex,255,255,255);
        SDL_SetTextureAlphaMod(e->batLogoTex,255);
    }

    {
        ratio=(float)e->chronoSecondes/(float)CHRONO_SECS;
        if(ratio<0.0f) ratio=0.0f;
        if(ratio>1.0f) ratio=1.0f;
        barRect=e->chronoRect;
        Uint32 nowC=SDL_GetTicks();
        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer,0,18,38,205); SDL_RenderFillRect(renderer,&barRect);
        SDL_SetRenderDrawColor(renderer,0,220,255,85); SDL_RenderDrawRect(renderer,&barRect);
        fillW=(int)(barRect.w*ratio);
        if(fillW>0){
            if(ratio>0.5f){tR=0;tG=210;}
            else if(ratio>0.25f){tR=255;tG=200;}
            else{tR=220;tG=30;}
            fill=(SDL_Rect){barRect.x,barRect.y,fillW,barRect.h};
            SDL_SetRenderDrawColor(renderer,tR,tG,40,230); SDL_RenderFillRect(renderer,&fill);
            SDL_SetRenderDrawColor(renderer,255,255,255,160);
            glow=(SDL_Rect){barRect.x+fillW-3,barRect.y,3,barRect.h};
            SDL_RenderFillRect(renderer,&glow);
        }
        if(ratio<0.25f){
            nowF=(float)nowC;
            pulse=0.5f+0.5f*sinf(nowF/180.0f);
            SDL_SetRenderDrawColor(renderer,255,60,0,(Uint8)(pulse*70)); SDL_RenderFillRect(renderer,&barRect);
            SDL_SetRenderDrawColor(renderer,255,80,0,(Uint8)(pulse*180));
            SDL_RenderDrawLine(renderer,barRect.x+fillW,barRect.y,barRect.x+fillW,barRect.y+barRect.h);
        }
        int cl=9;
        SDL_SetRenderDrawColor(renderer,0,220,255,140);
        SDL_RenderDrawLine(renderer,barRect.x,barRect.y,barRect.x+cl,barRect.y);
        SDL_RenderDrawLine(renderer,barRect.x,barRect.y,barRect.x,barRect.y+cl);
        SDL_RenderDrawLine(renderer,barRect.x+barRect.w,barRect.y,barRect.x+barRect.w-cl,barRect.y);
        SDL_RenderDrawLine(renderer,barRect.x+barRect.w,barRect.y,barRect.x+barRect.w,barRect.y+cl);
        SDL_RenderDrawLine(renderer,barRect.x,barRect.y+barRect.h,barRect.x+cl,barRect.y+barRect.h);
        SDL_RenderDrawLine(renderer,barRect.x+barRect.w,barRect.y+barRect.h,barRect.x+barRect.w-cl,barRect.y+barRect.h);
        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);
    }

    q=&e->questions[e->ordreJeu[e->questionIndex]];
    black=(SDL_Color){0,0,0,255};

    if(e->cardTexture)
        SDL_RenderCopyEx(renderer,e->cardTexture,NULL,&e->cardRect,e->cardAngle,NULL,SDL_FLIP_NONE);
    else{SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);SDL_SetRenderDrawColor(renderer,255,255,255,200);SDL_RenderFillRect(renderer,&e->cardRect);}

    qZone=(SDL_Rect){e->cardRect.x+e->cardRect.w/8,e->cardRect.y+e->cardRect.h/7,e->cardRect.w*3/5,e->cardRect.h*5/7};
    {int pad=14,wrap=qZone.w-2*pad; if(wrap<10)wrap=10;
    s=TTF_RenderUTF8_Blended_Wrapped(fontTiny,q->question,black,wrap);
    if(s){t=SDL_CreateTextureFromSurface(renderer,s);
        int x=qZone.x+(qZone.w-s->w)/2, y=qZone.y+(qZone.h-s->h)/2;
        if(x<qZone.x+pad) x=qZone.x+pad;
        if(y<qZone.y+pad) y=qZone.y+pad;
        SDL_RenderSetClipRect(renderer,&qZone);
        d=(SDL_Rect){x,y,s->w,s->h};
        if(t){SDL_RenderCopy(renderer,t,NULL,&d);SDL_DestroyTexture(t);}
        SDL_RenderSetClipRect(renderer,NULL);SDL_FreeSurface(s);}}

    for(i=0;i<NB_PROPS;i++){
        if(e->propTexture[i])
            SDL_RenderCopyEx(renderer,e->propTexture[i],NULL,&e->propRect[i],e->propAngle[i],NULL,SDL_FLIP_NONE);
        else{SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);SDL_SetRenderDrawColor(renderer,240,240,255,200);SDL_RenderFillRect(renderer,&e->propRect[i]);}

        SDL_Rect hr={e->propRect[i].x-20,e->propRect[i].y-20,e->propRect[i].w+40,e->propRect[i].h+40};
        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
        if(!e->showResult&&e->hovered==i){SDL_SetRenderDrawColor(renderer,255,200,0,130);SDL_RenderFillRect(renderer,&hr);}
        if(e->showResult&&e->selected==i){
            if(e->correct)SDL_SetRenderDrawColor(renderer,0,200,0,190);
            else SDL_SetRenderDrawColor(renderer,200,0,0,190);
            SDL_RenderFillRect(renderer,&hr);}
        if(e->showResult&&i==q->bonneReponse&&e->selected!=i){SDL_SetRenderDrawColor(renderer,0,200,0,140);SDL_RenderFillRect(renderer,&hr);}
        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);

        tZone=(SDL_Rect){e->propRect[i].x+e->propRect[i].w*15/100,e->propRect[i].y+e->propRect[i].h*2/7,e->propRect[i].w*5/10,e->propRect[i].h*3/7};
        {int pad=14,wrap=tZone.w-2*pad; if(wrap<10)wrap=10;
        s=TTF_RenderUTF8_Blended_Wrapped(fontSmall,q->prop[i],black,wrap);
        if(s){t=SDL_CreateTextureFromSurface(renderer,s);
            int x=tZone.x+(tZone.w-s->w)/2, y=tZone.y+(tZone.h-s->h)/2;
            if(x<tZone.x+pad) x=tZone.x+pad;
            if(y<tZone.y+pad) y=tZone.y+pad;
            SDL_RenderSetClipRect(renderer,&tZone);
            d=(SDL_Rect){x,y,s->w,s->h};
            if(t){SDL_RenderCopy(renderer,t,NULL,&d);SDL_DestroyTexture(t);}
            SDL_RenderSetClipRect(renderer,NULL);SDL_FreeSurface(s);}}
    }

    if(e->showResult||e->chronoExpire){
        const char *msg; SDL_Color mc;
        if(e->chronoExpire&&!e->showResult){msg="Time is up! Next question...";mc=(SDL_Color){255,120,0,255};}
        else if(e->correct){msg="Correct! +1 point";mc=(SDL_Color){0,220,0,255};}
        else{msg="Wrong answer! Next question...";mc=(SDL_Color){220,0,0,255};}
        TTF_SizeUTF8(fontSmall,msg,&tw,&th);
        int bx=(W-tw)/2-20, by=H-th-20;
        bg2=(SDL_Rect){bx,by,tw+40,th+14};
        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer,0,0,0,210); SDL_RenderFillRect(renderer,&bg2);
        SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);
        s=TTF_RenderUTF8_Blended(fontSmall,msg,mc);
        if(s){t=SDL_CreateTextureFromSurface(renderer,s);
            d=(SDL_Rect){(W-s->w)/2,by+7,s->w,s->h};
            if(t){SDL_RenderCopy(renderer,t,NULL,&d);SDL_DestroyTexture(t);}SDL_FreeSurface(s);}
    }
    (void)col;
}

void handleEnigmeEvent(Enigme *e, SDL_Event *event)
{
    int mx, my, choix, i;
    QuestionData *q;
    if (!e || !event || e->questionIndex >= NB_QUESTIONS) return;

    if(event->type==SDL_MOUSEMOTION){
        mx=event->motion.x; my=event->motion.y; e->hovered=-1;
        for(i=0;i<NB_PROPS;i++)
            if(mx>=e->propRect[i].x&&mx<e->propRect[i].x+e->propRect[i].w&&
               my>=e->propRect[i].y&&my<e->propRect[i].y+e->propRect[i].h)
                {e->hovered=i;break;}
    }
    if(e->showResult||e->waitingSuspense) return;
    choix=-1;
    if(event->type==SDL_MOUSEBUTTONDOWN&&event->button.button==SDL_BUTTON_LEFT){
        mx=event->button.x; my=event->button.y;
        for(i=0;i<NB_PROPS;i++)
            if(mx>=e->propRect[i].x&&mx<e->propRect[i].x+e->propRect[i].w&&
               my>=e->propRect[i].y&&my<e->propRect[i].y+e->propRect[i].h)
                {choix=i;break;}
    }
    if(event->type==SDL_KEYDOWN&&!event->key.repeat){
        if(event->key.keysym.sym==SDLK_a||event->key.keysym.sym==SDLK_1) choix=0;
        if(event->key.keysym.sym==SDLK_b||event->key.keysym.sym==SDLK_2) choix=1;
        if(event->key.keysym.sym==SDLK_c||event->key.keysym.sym==SDLK_3) choix=2;
    }
    if(choix>=0&&choix<NB_PROPS){
        q=&e->questions[e->ordreJeu[e->questionIndex]];
        e->selected=choix; e->correct=(choix==q->bonneReponse);
        e->waitingSuspense=1; e->suspenseTime=SDL_GetTicks();
        jouerSon(e,SND_SUSPENSE);
    }
}

void freeEnigme(Enigme *e)
{
    int i;
    if(!e) return;
    Mix_HaltMusic();
    if(e->bgTexture)      {SDL_DestroyTexture(e->bgTexture);      e->bgTexture=NULL;}
    if(e->cardTexture)    {SDL_DestroyTexture(e->cardTexture);     e->cardTexture=NULL;}
    if(e->batLogoTex)     {SDL_DestroyTexture(e->batLogoTex);      e->batLogoTex=NULL;}
    if(e->chronoDesignTex){SDL_DestroyTexture(e->chronoDesignTex); e->chronoDesignTex=NULL;}
    for(i=0;i<NB_PROPS;i++) if(e->propTexture[i]){SDL_DestroyTexture(e->propTexture[i]);e->propTexture[i]=NULL;}
    if(e->soundBat)      {Mix_FreeMusic(e->soundBat);      e->soundBat=NULL;}
    if(e->soundSuspense) {Mix_FreeMusic(e->soundSuspense); e->soundSuspense=NULL;}
    if(e->soundCorrect)  {Mix_FreeMusic(e->soundCorrect);  e->soundCorrect=NULL;}
    if(e->soundWrong)    {Mix_FreeMusic(e->soundWrong);    e->soundWrong=NULL;}
}

SDL_Texture *loadTexture(const char *path, SDL_Renderer *renderer)
{
    SDL_Surface *s = IMG_Load(path);
    if (!s) return NULL;
    SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
    SDL_FreeSurface(s);
    return t;
}
