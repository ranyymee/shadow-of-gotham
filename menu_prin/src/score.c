/*
 * score.c  — Gestion du menu scores
 * Utilise : header.h, integrated.h, enigme.h
 */
#include "header.h"
#include "integrated.h"
#include "enigme.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FRAME_DELAY 100

/* ============================================================
   Helpers internes
   ============================================================ */
static int isHovered(int mx, int my, SDL_Rect r)
{
    return (mx >= r.x && mx <= r.x + r.w &&
            my >= r.y && my <= r.y + r.h);
}

static void renderButton(SDL_Renderer *renderer, TTF_Font *font, SDL_Color color,
                         SDL_Texture *tex_normal, SDL_Texture *tex_hover,
                         SDL_Rect pos, int hovered, const char *fallback_text)
{
    SDL_Rect draw_pos = pos;
    if (hovered) {
        int dw = 20, dh = 10;
        draw_pos.x -= dw / 2;
        draw_pos.y -= dh / 2;
        draw_pos.w += dw;
        draw_pos.h += dh;
    }
    SDL_Texture *tex = (hovered && tex_hover) ? tex_hover
                     : tex_normal             ? tex_normal
                     : NULL;
    if (tex) {
        SDL_RenderCopy(renderer, tex, NULL, &draw_pos);
    } else if (font) {
        SDL_Color c = hovered ? (SDL_Color){255, 220, 50, 255} : color;
        SDL_Surface *s = TTF_RenderText_Solid(font, fallback_text, c);
        if (s) {
            SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_RenderCopy(renderer, t, NULL, &draw_pos);
            SDL_FreeSurface(s);
            SDL_DestroyTexture(t);
        }
    }
}

/* ============================================================
   initScoreMenu
   ============================================================ */
void initScoreMenu(ScoreMenu *menu, SDL_Renderer *renderer)
{
    memset(menu, 0, sizeof(ScoreMenu));
    menu->frame_count     = 26;
    menu->current_frame   = 0;
    menu->frame_direction = 1;
    menu->last_frame_time = SDL_GetTicks();

    char path[256];
    for (int i = 1; i <= menu->frame_count; i++) {
        sprintf(path, "assets/back/ezgif-frame-%03d.png", i);
        menu->background_frames[i-1] = loadTexture(path, renderer);
    }

    menu->button_validate_texture       = loadTexture("assets/button/valider.png",       renderer);
    menu->button_return_texture         = loadTexture("assets/button/retour.png",         renderer);
    menu->button_quit_texture           = loadTexture("assets/button/quitter.png",        renderer);
    menu->button_validate_hover_texture = loadTexture("assets/button/valider_hover.png",  renderer);
    menu->button_return_hover_texture   = loadTexture("assets/button/retour_hover.png",   renderer);
    menu->button_quit_hover_texture     = loadTexture("assets/button/quitter_hover.png",  renderer);

    menu->pos_button_validate = (SDL_Rect){320, 350, 250, 70};
    menu->pos_button_return   = (SDL_Rect){210, 420, 200, 65};
    menu->pos_button_quit     = (SDL_Rect){780, 420, 200, 65};
    menu->zone_input          = (SDL_Rect){310,210, 520, 55};

    strcpy(menu->player_name, "");
    menu->font      = TTF_OpenFont("assets/font/font.ttf", 34);
    menu->textColor = (SDL_Color){255, 255, 255, 255};
    if (!menu->font) printf("[WARN] font: %s\n", TTF_GetError());

    menu->click_sound      = Mix_LoadWAV("assets/audio/click.wav");
    menu->validation_music = Mix_LoadMUS("assets/audio/sound.mp3");

    menu->panel = loadTexture("assets/back/zina.jpg",  renderer);
    menu->label = loadTexture("assets/back/label.png", renderer);
    if (!menu->panel) printf("[WARN] zina.jpg introuvable\n");
    if (!menu->label) printf("[WARN] label.png introuvable\n");
}

/* ============================================================
   cleanupScoreMenu
   ============================================================ */
void cleanupScoreMenu(ScoreMenu *menu)
{
    for (int i = 0; i < menu->frame_count; i++)
        if (menu->background_frames[i])
            SDL_DestroyTexture(menu->background_frames[i]);

    #define DTEX(t) if (t) { SDL_DestroyTexture(t); t = NULL; }
    DTEX(menu->button_validate_texture)
    DTEX(menu->button_return_texture)
    DTEX(menu->button_quit_texture)
    DTEX(menu->button_validate_hover_texture)
    DTEX(menu->button_return_hover_texture)
    DTEX(menu->button_quit_hover_texture)
    DTEX(menu->panel)
    DTEX(menu->label)
    #undef DTEX

    if (menu->click_sound)      { Mix_FreeChunk(menu->click_sound);      menu->click_sound      = NULL; }
    if (menu->validation_music) { Mix_FreeMusic(menu->validation_music);  menu->validation_music = NULL; }
    if (menu->font)             { TTF_CloseFont(menu->font);              menu->font             = NULL; }
}

/* ============================================================
   saveScore
   ============================================================ */
void saveScore(const char *player_name, int score)
{
    PlayerScore scores[MAX_SCORES];
    int nb = 0;

    FILE *file = fopen("scores.txt", "r");
    if (file) {
        char line[256];
        while (nb < MAX_SCORES && fgets(line, sizeof(line), file)) {
            char name[MAX_NAME]; int sc;
            if (sscanf(line, "%99s %d", name, &sc) == 2) {
                int is_num = 1;
                for (int k = 0; name[k]; k++)
                    if (name[k] < '0' || name[k] > '9') { is_num = 0; break; }
                if (!is_num) {
                    strncpy(scores[nb].name, name, MAX_NAME - 1);
                    scores[nb].score = sc;
                    nb++;
                }
            }
        }
        fclose(file);
    }

    strncpy(scores[nb].name, strlen(player_name) ? player_name : "Anonyme", MAX_NAME - 1);
    scores[nb].score = score;
    nb++;

    for (int i = 0; i < nb - 1; i++)
        for (int j = 0; j < nb - i - 1; j++)
            if (scores[j].score < scores[j+1].score) {
                PlayerScore tmp = scores[j];
                scores[j]       = scores[j+1];
                scores[j+1]     = tmp;
            }

    file = fopen("scores.txt", "w");
    if (file) {
        for (int i = 0; i < nb && i < MAX_SCORES; i++)
            fprintf(file, "%s %d\n", scores[i].name, scores[i].score);
        fclose(file);
    }
}

/* ============================================================
   runEnigmeLoop
   ============================================================ */
static void runEnigmeLoop(SDL_Renderer *renderer, TTF_Font *font)
{
    Enigme e;
    initEnigme(&e, renderer);
    TTF_Font *fontSmall = TTF_OpenFont("assets/font/font.ttf", 24);
    if (!fontSmall) fontSmall = font;
    SDL_Event ev;
    int done = 0;
    while (!done) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { done = 1; break; }
            if (ev.type == SDL_KEYDOWN &&
                ev.key.keysym.sym == SDLK_ESCAPE) { done = 1; break; }
            handleEnigmeEvent(&e, &ev);
        }
        updateEnigme(&e);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        renderEnigme(&e, renderer, font, fontSmall);
        SDL_RenderPresent(renderer);
        if (e.questionIndex >= NB_QUESTIONS) { SDL_Delay(1500); done = 1; }
        SDL_Delay(16);
    }
    freeEnigme(&e);
    if (fontSmall && fontSmall != font) TTF_CloseFont(fontSmall);
}

/* ============================================================
   displayScores
   ============================================================ */
void displayScores(SDL_Renderer *renderer, ScoreMenu *menu)
{
    FILE *file = fopen("scores.txt", "r");
    if (!file) return;

    PlayerScore scores[MAX_SCORES];
    int nb = 0;
    char line[256];
    while (nb < MAX_SCORES && fgets(line, sizeof(line), file)) {
        char name[MAX_NAME]; int sc;
        if (sscanf(line, "%99s %d", name, &sc) == 2) {
            int is_num = 1;
            for (int k = 0; name[k]; k++)
                if (name[k] < '0' || name[k] > '9') { is_num = 0; break; }
            if (!is_num) {
                strncpy(scores[nb].name, name, MAX_NAME - 1);
                scores[nb].score = sc;
                nb++;
            }
        }
    }
    fclose(file);

    SDL_Texture *star_tex = loadTexture("assets/star.png", renderer);
    /* Panel scores: {220, 90, 780, 420}
       Layout Image 2: etoiles a gauche (petites), empilees par rang
       chaque ligne: [★★★]  NOM : SCORE
       Panel interne commence x=240, titre y=100 */
    int panel_x  = 210;
    int star_size = 35;
    int star_gap  = 6;
    int y = 235;
    char buf[128];

    /* Titre TOP 3 SCORES */
    if (menu->font) {
        SDL_Surface *s = TTF_RenderText_Solid(menu->font, "TOP 3 SCORES", menu->textColor);
        if (s) {
            SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
            SDL_Rect r = {panel_x, 165, s->w, s->h};
            SDL_RenderCopy(renderer, t, NULL, &r);
            SDL_FreeSurface(s); SDL_DestroyTexture(t);
        }
    }

    int max_display = nb < 3 ? nb : 3;
    int stars_count[3] = {3, 2, 1};
    /* Largeur fixe reservee aux etoiles: 3 etoiles max */
    int stars_zone_w = 3 * (star_size + star_gap);

    for (int i = 0; i < max_display; i++) {
        /* Etoiles sur la ligne */
        if (star_tex) {
            for (int s = 0; s < stars_count[i]; s++) {
                SDL_Rect sr = {panel_x + s * (star_size + star_gap),
                               y + (star_size/2 - star_size/2), star_size, star_size};
                SDL_RenderCopy(renderer, star_tex, NULL, &sr);
            }
        }
        /* Nom et score — juste apres la zone etoiles, meme ligne, centre vertical */
        snprintf(buf, 128, "%s : %d", scores[i].name, scores[i].score);
        if (menu->font) {
            SDL_Surface *s = TTF_RenderText_Solid(menu->font, buf, menu->textColor);
            if (s) {
                SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
                int text_y = y + (star_size - s->h) / 2;
                SDL_Rect r = {panel_x + stars_zone_w + 15, text_y, s->w, s->h};
                SDL_RenderCopy(renderer, t, NULL, &r);
                SDL_FreeSurface(s); SDL_DestroyTexture(t);
            }
        }
        y += star_size + 30;  /* espacement vertical entre lignes */
    }
    if (star_tex) SDL_DestroyTexture(star_tex);
}

/* ============================================================
   handleEvents
   ============================================================ */
void handleEvents(SDL_Event event, ScoreMenu *menu, ScoreMenuState *state,
                  int *quit, int score, SDL_Renderer *renderer, int *go_main_menu)
{
    ScoreMenuState old_state = *state;

    switch (event.type) {
    case SDL_QUIT:
        *quit = 1;
        break;

    case SDL_MOUSEMOTION: {
        int x = event.motion.x, y = event.motion.y;
        menu->hovered_validate = isHovered(x, y, menu->pos_button_validate);
        menu->hovered_quit     = isHovered(x, y, menu->pos_button_quit);
        menu->hovered_return   = isHovered(x, y, menu->pos_button_return);
        break;
    }

    case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_LEFT) {
            int x = event.button.x, y = event.button.y;
            if (*state == MENU_INPUT) {
                if (isHovered(x, y, menu->pos_button_validate)) {
                    saveScore(menu->player_name, score);
                    *state = MENU_SCORES_DISPLAY;
                    if (menu->click_sound) Mix_PlayChannel(-1, menu->click_sound, 0);
                    if (menu->validation_music) {
                        Mix_HaltMusic();
                        Mix_PlayMusic(menu->validation_music, 0);
                    }
                }
            }
            else if (*state == MENU_SCORES_DISPLAY) {
                if (isHovered(x, y, menu->pos_button_quit)) {
                    if (menu->click_sound) Mix_PlayChannel(-1, menu->click_sound, 0);
                    *quit = 1;
                }
                if (isHovered(x, y, menu->pos_button_return)) {
                    if (menu->click_sound) Mix_PlayChannel(-1, menu->click_sound, 0);
                    Mix_HaltMusic();
                    *go_main_menu = 1;
                    *quit = 1;
                }
            }
        }
        break;

    case SDL_TEXTINPUT:
        if (*state == MENU_INPUT) {
            if (strlen(menu->player_name) + strlen(event.text.text) < MAX_NAME)
                strcat(menu->player_name, event.text.text);
        }
        break;

    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE) { *quit = 1; break; }
        if (event.key.keysym.sym == SDLK_r) {
            if (menu->click_sound) Mix_PlayChannel(-1, menu->click_sound, 0);
            Mix_HaltMusic();
            *go_main_menu = 1;
            *quit = 1;
            break;
        }
        /* Touche E → lancer enigme */
        if (event.key.keysym.sym == SDLK_e) {
            if (menu->click_sound) Mix_PlayChannel(-1, menu->click_sound, 0);
            Mix_HaltMusic();
            SDL_StopTextInput();
            runEnigmeLoop(renderer, menu->font);
            return;
        }
        if (*state == MENU_INPUT) {
            if (event.key.keysym.sym == SDLK_RETURN) {
                saveScore(menu->player_name, score);
                *state = MENU_SCORES_DISPLAY;
                if (menu->validation_music) {
                    Mix_HaltMusic();
                    Mix_PlayMusic(menu->validation_music, 0);
                }
            }
            if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(menu->player_name) > 0)
                menu->player_name[strlen(menu->player_name) - 1] = '\0';
        }
        break;
    }

    if (*state != old_state) {
        if (*state == MENU_INPUT) SDL_StartTextInput();
        else                      SDL_StopTextInput();
    }
}

/* ============================================================
   display
   ============================================================ */
void display(SDL_Renderer *renderer, ScoreMenu *menu,
             ScoreMenuState state, int score)
{
    (void)score;
    if (state == MENU_ENIGME) return;

    SDL_RenderClear(renderer);

    /* Background animé */
    Uint32 now = SDL_GetTicks();
    if (now - menu->last_frame_time > FRAME_DELAY) {
        menu->current_frame += menu->frame_direction;
        if (menu->current_frame >= menu->frame_count - 1) {
            menu->current_frame   = menu->frame_count - 1;
            menu->frame_direction = -1;
        } else if (menu->current_frame <= 0) {
            menu->current_frame   = 0;
            menu->frame_direction = 1;
        }
        menu->last_frame_time = now;
    }
    if (menu->background_frames[menu->current_frame])
        SDL_RenderCopy(renderer, menu->background_frames[menu->current_frame], NULL, NULL);

    /* Panel zina.jpg semi-transparent avec coins arrondis — taille réduite */
    {
        SDL_Rect panelRect = (state == MENU_SCORES_DISPLAY)
            ? (SDL_Rect){180, 155, 820, 340}   /* top scores : sous le bat */
            : (SDL_Rect){290, 160, 680, 320};  /* saisir nom : sous le bat */
        int radius = 30;

        if (menu->panel) {
            /* Render zina.jpg dans une texture intermédiaire avec coins arrondis */
            SDL_Texture *mask_tex = SDL_CreateTexture(renderer,
                SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                panelRect.w, panelRect.h);
            if (mask_tex) {
                SDL_SetTextureBlendMode(mask_tex, SDL_BLENDMODE_BLEND);
                SDL_SetRenderTarget(renderer, mask_tex);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                SDL_RenderClear(renderer);

                SDL_SetTextureAlphaMod(menu->panel, 150);
                SDL_SetTextureBlendMode(menu->panel, SDL_BLENDMODE_BLEND);
                SDL_Rect src = {0, 0, panelRect.w, panelRect.h};
                SDL_RenderCopy(renderer, menu->panel, NULL, &src);

                /* Effacer les 4 coins pour faire l'arrondi */
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                for (int dy = 0; dy < radius; dy++)
                    for (int dx = 0; dx < radius; dx++)
                        if ((dx-radius)*(dx-radius)+(dy-radius)*(dy-radius) > radius*radius)
                            SDL_RenderDrawPoint(renderer, dx, dy);
                for (int dy = 0; dy < radius; dy++)
                    for (int dx = panelRect.w-radius; dx < panelRect.w; dx++)
                        if ((dx-(panelRect.w-radius))*(dx-(panelRect.w-radius))+(dy-radius)*(dy-radius) > radius*radius)
                            SDL_RenderDrawPoint(renderer, dx, dy);
                for (int dy = panelRect.h-radius; dy < panelRect.h; dy++)
                    for (int dx = 0; dx < radius; dx++)
                        if ((dx-radius)*(dx-radius)+(dy-(panelRect.h-radius))*(dy-(panelRect.h-radius)) > radius*radius)
                            SDL_RenderDrawPoint(renderer, dx, dy);
                for (int dy = panelRect.h-radius; dy < panelRect.h; dy++)
                    for (int dx = panelRect.w-radius; dx < panelRect.w; dx++)
                        if ((dx-(panelRect.w-radius))*(dx-(panelRect.w-radius))+(dy-(panelRect.h-radius))*(dy-(panelRect.h-radius)) > radius*radius)
                            SDL_RenderDrawPoint(renderer, dx, dy);

                SDL_SetRenderTarget(renderer, NULL);
                SDL_RenderCopy(renderer, mask_tex, NULL, &panelRect);
                SDL_DestroyTexture(mask_tex);
            }
        }
    }

    if (state == MENU_INPUT) {
        /* Titre */
        if (menu->font) {
            SDL_Color blanc = {255, 255, 255, 255};
            SDL_Surface *s = TTF_RenderText_Solid(menu->font, "SAISIR VOTRE NOM", blanc);
            if (s) {
                SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
                SDL_Rect r = {640 - s->w/2, 175, s->w, s->h};
                SDL_RenderCopy(renderer, t, NULL, &r);
                SDL_FreeSurface(s); SDL_DestroyTexture(t);
            }
        }
        /* label.png barre de saisie */
        if (menu->label)
            SDL_RenderCopy(renderer, menu->label, NULL, &menu->zone_input);
        else {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &menu->zone_input);
        }
        /* Texte saisi */
        if (strlen(menu->player_name) > 0 && menu->font) {
            SDL_Color blanc = {255, 255, 255, 255};
            SDL_Surface *ns = TTF_RenderText_Solid(menu->font, menu->player_name, blanc);
            if (ns) {
                SDL_Texture *nt = SDL_CreateTextureFromSurface(renderer, ns);
                SDL_Rect nr = {menu->zone_input.x + 15, menu->zone_input.y + 12, ns->w, ns->h};
                SDL_RenderCopy(renderer, nt, NULL, &nr);
                SDL_FreeSurface(ns); SDL_DestroyTexture(nt);
            }
        }
        /* Bouton Valider */
        renderButton(renderer, menu->font, menu->textColor,
                     menu->button_validate_texture, menu->button_validate_hover_texture,
                     menu->pos_button_validate, menu->hovered_validate, "Valider");
    }
    else if (state == MENU_SCORES_DISPLAY) {
        displayScores(renderer, menu);
        renderButton(renderer, menu->font, menu->textColor,
                     menu->button_quit_texture, menu->button_quit_hover_texture,
                     menu->pos_button_quit, menu->hovered_quit, "Quitter");
        renderButton(renderer, menu->font, menu->textColor,
                     menu->button_return_texture, menu->button_return_hover_texture,
                     menu->pos_button_return, menu->hovered_return, "Retour");
    }

    SDL_RenderPresent(renderer);
}

/* ============================================================
   scoreMenuLoop
   ============================================================ */
int scoreMenuLoop(SDL_Window *window, SDL_Renderer *renderer, int final_score)
{
    (void)window;
    ScoreMenu menu;
    initScoreMenu(&menu, renderer);

    ScoreMenuState state = MENU_INPUT;
    SDL_StartTextInput();

    int quit = 0, go_main_menu = 0;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event))
            handleEvents(event, &menu, &state, &quit, final_score, renderer, &go_main_menu);
        display(renderer, &menu, state, final_score);
        SDL_Delay(16);
    }

    SDL_StopTextInput();
    cleanupScoreMenu(&menu);
    return go_main_menu ? 2 : 0;
}
