/* score.c — corrected */
#include "integrated.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FRAME_DELAY 100
#define VISIBLE_SCORES 5   /* how many top scores to show */

/* ================= HOVER ================= */
static int isHovered(int mx, int my, SDL_Rect r)
{
    return (mx >= r.x && mx <= r.x + r.w &&
            my >= r.y && my <= r.y + r.h);
}

/* ================= INIT ================= */
void initScoreMenu(ScoreMenu *menu, SDL_Renderer *renderer)
{
    memset(menu, 0, sizeof(ScoreMenu));

    /* Load background frames and track how many loaded successfully */
    const char *frame_paths[] = {
        "assets/back/score1.jpg",
        "assets/back/score2.jpg"
    };
    menu->frame_count = 0;
    for (int i = 0; i < (int)(sizeof(frame_paths)/sizeof(frame_paths[0])); i++) {
        SDL_Texture *t = loadTexture(frame_paths[i], renderer);
        if (t) menu->background_frames[menu->frame_count++] = t;
    }

    /* Normal button textures */
    menu->button_validate_texture = loadTexture("assets/button/valider.png",    renderer);
    menu->button_return_texture   = loadTexture("assets/button/retour.png",     renderer);
    menu->button_quit_texture     = loadTexture("assets/image/bouton quitter 1.png", renderer);

    /* Hover button textures — all four loaded consistently */
    menu->button_validate_hover_texture = loadTexture("assets/button/valider_hover.png", renderer);
    menu->button_return_hover_texture   = loadTexture("assets/button/retour_hover.png",  renderer);
    menu->button_quit_hover_texture     = loadTexture("assets/image/bouton quitter 1_hover.png", renderer);

    /* Layout (window 1280x800):
       Title       ~ y=180
       Subtitle    ~ y=260
       Input bar   ~ y=350  (below subtitle, clear gap)
       Validate    ~ y=450  (below input bar)
       Return/Quit ~ y=650  (bottom of screen, side by side)
    */
    menu->zone_input          = (SDL_Rect){310,  420, 660,  55};
    menu->pos_button_validate = (SDL_Rect){515,  530, 250,  70};
    menu->pos_button_return   = (SDL_Rect){200,  680, 200,  65};
    menu->pos_button_quit     = (SDL_Rect){880,  680, 200,  65};

    strcpy(menu->player_name, "");

    menu->font      = TTF_OpenFont("assets/font/font.ttf", 20);
    menu->textColor = (SDL_Color){255, 255, 255, 255};

    menu->click_sound      = Mix_LoadWAV("assets/audio/click.wav");
    menu->validation_music = Mix_LoadMUS("assets/audio/sound.mp3");
}

/* ================= CLEAN ================= */
void cleanupScoreMenu(ScoreMenu *menu)
{
    /* Only destroy textures that were actually loaded */
    for (int i = 0; i < menu->frame_count; i++)
        if (menu->background_frames[i])
            SDL_DestroyTexture(menu->background_frames[i]);

    if (menu->button_validate_texture)       SDL_DestroyTexture(menu->button_validate_texture);
    if (menu->button_validate_hover_texture) SDL_DestroyTexture(menu->button_validate_hover_texture);
    if (menu->button_return_texture)         SDL_DestroyTexture(menu->button_return_texture);
    if (menu->button_return_hover_texture)   SDL_DestroyTexture(menu->button_return_hover_texture);
    if (menu->button_quit_texture)           SDL_DestroyTexture(menu->button_quit_texture);
    if (menu->button_quit_hover_texture)     SDL_DestroyTexture(menu->button_quit_hover_texture);

    if (menu->font) TTF_CloseFont(menu->font);

    if (menu->click_sound)      Mix_FreeChunk(menu->click_sound);
    if (menu->validation_music) Mix_FreeMusic(menu->validation_music);
}

/* ================= SAVE ================= */
void saveScore(const char *player_name, int score)
{
    FILE *file = fopen("scores.txt", "a");
    if (file) {
        fprintf(file, "%s %d\n", player_name, score);
        fclose(file);
    }
}

/* ================= DISPLAY SCORES ================= */
void displayScores(SDL_Renderer *renderer, ScoreMenu *menu)
{
    if (!menu->font) return;

    PlayerScore scores[MAX_SCORES];
    int count = 0;

    FILE *f = fopen("scores.txt", "r");
    if (f) {
        while (count < MAX_SCORES &&
               fscanf(f, "%99s %d", scores[count].name, &scores[count].score) == 2)
            count++;
        fclose(f);
    }

    /* Bubble sort descending by score */
    for (int i = 0; i < count - 1; i++)
        for (int j = i + 1; j < count; j++)
            if (scores[j].score > scores[i].score) {
                PlayerScore tmp = scores[i];
                scores[i] = scores[j];
                scores[j] = tmp;
            }

    /* Show up to VISIBLE_SCORES entries, not just 3 */
    int display_count = (count < VISIBLE_SCORES) ? count : VISIBLE_SCORES;
    for (int i = 0; i < display_count; i++) {
        char line[128];
        snprintf(line, sizeof(line), "#%d  %-30.30s  %d",
                 i + 1, scores[i].name, scores[i].score);

        SDL_Surface *s = TTF_RenderText_Blended(menu->font, line, menu->textColor);
        if (!s) continue;
        SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
        if (!t) { SDL_FreeSurface(s); continue; }

        SDL_Rect r = {310, 300 + i * 60, s->w, s->h};
        SDL_RenderCopy(renderer, t, NULL, &r);

        SDL_FreeSurface(s);
        SDL_DestroyTexture(t);
    }
}

/* ================= DISPLAY ================= */
void display(SDL_Renderer *renderer, ScoreMenu *menu,
             ScoreMenuState state, int score)
{
    SDL_RenderClear(renderer);

    /* Pick background frame: frame 0 for input, frame 1 for scores */
    int bg = (state == MENU_INPUT) ? 0 : 1;
    if (bg < menu->frame_count && menu->background_frames[bg])
        SDL_RenderCopy(renderer, menu->background_frames[bg], NULL, NULL);

    /* ===== INPUT STATE ===== */
    if (state == MENU_INPUT) {

        /* Show the current score so the player knows what they're saving */
        if (menu->font) {
            char score_line[64];
            snprintf(score_line, sizeof(score_line), "Score: %d", score);
            SDL_Surface *ss = TTF_RenderText_Blended(menu->font, score_line, menu->textColor);
            if (ss) {
                SDL_Texture *st = SDL_CreateTextureFromSurface(renderer, ss);
                if (st) {
                    SDL_Rect sr = {menu->zone_input.x, menu->zone_input.y - 50,
                                   ss->w, ss->h};
                    SDL_RenderCopy(renderer, st, NULL, &sr);
                    SDL_DestroyTexture(st);
                }
                SDL_FreeSurface(ss);
            }
        }

        /* Typed player name */
        if (menu->font && strlen(menu->player_name) > 0) {
            SDL_Surface *s = TTF_RenderText_Blended(menu->font,
                                menu->player_name, menu->textColor);
            if (s) {
                SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);
                if (t) {
                    SDL_Rect r = {menu->zone_input.x + 10,
                                  menu->zone_input.y + 10,
                                  s->w, s->h};
                    SDL_RenderCopy(renderer, t, NULL, &r);
                    SDL_DestroyTexture(t);
                }
                SDL_FreeSurface(s);
            }
        }

        /* Validate button (with hover) */
        SDL_Texture *vTex = (menu->hovered_validate && menu->button_validate_hover_texture)
                            ? menu->button_validate_hover_texture
                            : menu->button_validate_texture;
        if (vTex)
            SDL_RenderCopy(renderer, vTex, NULL, &menu->pos_button_validate);
    }

    /* ===== SCORES STATE ===== */
    else if (state == MENU_SCORES_DISPLAY) {

        displayScores(renderer, menu);

        SDL_Texture *rTex = (menu->hovered_return && menu->button_return_hover_texture)
                            ? menu->button_return_hover_texture
                            : menu->button_return_texture;
        SDL_Texture *qTex = (menu->hovered_quit && menu->button_quit_hover_texture)
                            ? menu->button_quit_hover_texture
                            : menu->button_quit_texture;

        if (rTex) SDL_RenderCopy(renderer, rTex, NULL, &menu->pos_button_return);
        if (qTex) SDL_RenderCopy(renderer, qTex, NULL, &menu->pos_button_quit);
    }

    SDL_RenderPresent(renderer);
}

/* ================= EVENTS ================= */
void handleEvents(SDL_Event event, ScoreMenu *menu, ScoreMenuState *state,
                  int *quit, int score, SDL_Renderer *renderer, int *go_main_menu)
{
    (void)renderer;

    if (event.type == SDL_MOUSEMOTION) {
        int mx = event.motion.x, my = event.motion.y;
        menu->hovered_validate = isHovered(mx, my, menu->pos_button_validate);
        menu->hovered_return   = isHovered(mx, my, menu->pos_button_return);
        menu->hovered_quit     = isHovered(mx, my, menu->pos_button_quit);
    }

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        int mx = event.button.x, my = event.button.y;

        if (*state == MENU_INPUT) {
            if (isHovered(mx, my, menu->pos_button_validate)) {
                if (menu->click_sound) Mix_PlayChannel(-1, menu->click_sound, 0);
                saveScore(menu->player_name, score);
                *state = MENU_SCORES_DISPLAY;
            }
        }
        else if (*state == MENU_SCORES_DISPLAY) {
            if (isHovered(mx, my, menu->pos_button_return)) {
                if (menu->click_sound) Mix_PlayChannel(-1, menu->click_sound, 0);
                *go_main_menu = 1;
                *quit = 1;
            }
            if (isHovered(mx, my, menu->pos_button_quit)) {
                if (menu->click_sound) Mix_PlayChannel(-1, menu->click_sound, 0);
                *quit = 1;
            }
        }
    }

    if (event.type == SDL_TEXTINPUT && *state == MENU_INPUT) {
        if (strlen(menu->player_name) < MAX_NAME - 1) {
            strncat(menu->player_name, event.text.text,
                    MAX_NAME - 1 - strlen(menu->player_name));
        }
    }

    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_BACKSPACE &&
            strlen(menu->player_name) > 0) {
            menu->player_name[strlen(menu->player_name) - 1] = '\0';
        }
        if (event.key.keysym.sym == SDLK_RETURN && *state == MENU_INPUT) {
            saveScore(menu->player_name, score);
            *state = MENU_SCORES_DISPLAY;
        }
        if (event.key.keysym.sym == SDLK_ESCAPE)
            *quit = 1;
    }

    if (event.type == SDL_QUIT)
        *quit = 1;
}

/* ================= LOOP ================= */
int scoreMenuLoop(SDL_Window *window, SDL_Renderer *renderer, int final_score)
{
    (void)window;

    /* Open audio only if not already open */
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        printf("Mix_OpenAudio warning: %s\n", Mix_GetError());

    ScoreMenu menu;
    initScoreMenu(&menu, renderer);

    ScoreMenuState state = MENU_INPUT;
    SDL_StartTextInput();

    int quit = 0, go_main_menu = 0;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event))
            handleEvents(event, &menu, &state, &quit,
                         final_score, renderer, &go_main_menu);

        display(renderer, &menu, state, final_score);
        SDL_Delay(16);
    }

    SDL_StopTextInput();
    cleanupScoreMenu(&menu);
    Mix_CloseAudio();

    return go_main_menu;
}
