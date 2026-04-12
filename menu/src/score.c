#include "header.h"
#include "integrated.h"
#include "enigme.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FRAME_DELAY 100

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

    menu->background_frames[0] = loadTexture("assets/back/score1.jpg", renderer);
    menu->background_frames[1] = loadTexture("assets/back/score2.jpg", renderer);

    menu->button_validate_texture = loadTexture("assets/button/valider.png", renderer);
    menu->button_return_texture   = loadTexture("assets/button/retour.png", renderer);
    menu->button_quit_texture     = loadTexture("assets/image/bouton quitter 1.png", renderer);

    menu->button_validate_hover_texture = loadTexture("assets/button/valider_hover.png", renderer);

    
    menu->pos_button_validate = (SDL_Rect){515, 370, 250, 70};
    menu->pos_button_return   = (SDL_Rect){160, 590, 200, 65};
    menu->pos_button_quit     = (SDL_Rect){920, 590, 200, 65};

    menu->zone_input = (SDL_Rect){310, 300, 660, 55};

    strcpy(menu->player_name, "");

    menu->font = TTF_OpenFont("assets/font/font.ttf", 20);
    menu->textColor = (SDL_Color){255,255,255,255};

    menu->click_sound = Mix_LoadWAV("assets/audio/click.wav");
    menu->validation_music = Mix_LoadMUS("assets/audio/sound.mp3");
}

/* ================= CLEAN ================= */
void cleanupScoreMenu(ScoreMenu *menu)
{
    for (int i = 0; i < MAX_FRAMES; i++)
        if (menu->background_frames[i])
            SDL_DestroyTexture(menu->background_frames[i]);

    SDL_DestroyTexture(menu->button_validate_texture);
    SDL_DestroyTexture(menu->button_validate_hover_texture);
    SDL_DestroyTexture(menu->button_return_texture);
    SDL_DestroyTexture(menu->button_return_hover_texture);
    SDL_DestroyTexture(menu->button_quit_texture);
    SDL_DestroyTexture(menu->button_quit_hover_texture);

    if (menu->font) TTF_CloseFont(menu->font);

    Mix_FreeChunk(menu->click_sound);
    Mix_FreeMusic(menu->validation_music);
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
    PlayerScore scores[MAX_SCORES];
    int count = 0;

    FILE *f = fopen("scores.txt", "r");
    if (f) {
        while (count < MAX_SCORES &&
               fscanf(f, "%99s %d", scores[count].name, &scores[count].score) == 2)
            count++;
        fclose(f);
    }

    /* sort */
    for (int i = 0; i < count - 1; i++)
        for (int j = i + 1; j < count; j++)
            if (scores[j].score > scores[i].score) {
                PlayerScore tmp = scores[i];
                scores[i] = scores[j];
                scores[j] = tmp;
            }

    for (int i = 0; i < count && i < 3; i++) {
        char line[100];
        snprintf(line, sizeof(line), "#%d %.50s %d",
         i+1, scores[i].name, scores[i].score);
        SDL_Surface *s = TTF_RenderText_Blended(menu->font, line, menu->textColor);
        SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);

        SDL_Rect r = {200, 260 + i * 80, s->w, s->h};
        SDL_RenderCopy(renderer, t, NULL, &r);

        SDL_FreeSurface(s);
        SDL_DestroyTexture(t);
    }
}

/* ================= DISPLAY ================= */
void display(SDL_Renderer *renderer, ScoreMenu *menu,
             ScoreMenuState state, int score)
{
    (void)score;
    SDL_RenderClear(renderer);

    int bg = (state == MENU_INPUT) ? 0 : 1;
    if (menu->background_frames[bg])
        SDL_RenderCopy(renderer, menu->background_frames[bg], NULL, NULL);

    /* ===== INPUT ===== */
    if (state == MENU_INPUT) {

        if (strlen(menu->player_name) > 0) {
            SDL_Surface *s = TTF_RenderText_Blended(menu->font,
                                menu->player_name, menu->textColor);
            SDL_Texture *t = SDL_CreateTextureFromSurface(renderer, s);

            SDL_Rect r = {menu->zone_input.x + 10,
                          menu->zone_input.y + 10,
                          s->w, s->h};

            SDL_RenderCopy(renderer, t, NULL, &r);

            SDL_FreeSurface(s);
            SDL_DestroyTexture(t);
        }

        SDL_Texture *vTex = menu->hovered_validate ?
            menu->button_validate_hover_texture :
            menu->button_validate_texture;

        SDL_RenderCopy(renderer, vTex, NULL, &menu->pos_button_validate);
    }

    /* ===== SCORES ===== */
    else if (state == MENU_SCORES_DISPLAY) {

        displayScores(renderer, menu);

        SDL_Texture *rTex = menu->hovered_return ?
            menu->button_return_hover_texture :
            menu->button_return_texture;

        SDL_Texture *qTex = menu->hovered_quit ?
            menu->button_quit_hover_texture :
            menu->button_quit_texture;

        SDL_RenderCopy(renderer, rTex, NULL, &menu->pos_button_return);
        SDL_RenderCopy(renderer, qTex, NULL, &menu->pos_button_quit);
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
        menu->hovered_validate = isHovered(mx,my,menu->pos_button_validate);
        menu->hovered_return   = isHovered(mx,my,menu->pos_button_return);
        menu->hovered_quit     = isHovered(mx,my,menu->pos_button_quit);
    }

    if (event.type == SDL_MOUSEBUTTONDOWN) {
        int mx = event.button.x, my = event.button.y;

        if (*state == MENU_INPUT &&
            isHovered(mx,my,menu->pos_button_validate)) {

            saveScore(menu->player_name, score);
            *state = MENU_SCORES_DISPLAY;
        }
        else if (*state == MENU_SCORES_DISPLAY) {

            if (isHovered(mx,my,menu->pos_button_return)) {
                *go_main_menu = 1;
                *quit = 1;
            }
            if (isHovered(mx,my,menu->pos_button_quit)) {
                *quit = 1;
            }
        }
    }

    /* FIX TEXT */
    if (event.type == SDL_TEXTINPUT && *state == MENU_INPUT) {
        if (strlen(menu->player_name) < MAX_NAME - 1) {
            strcat(menu->player_name, event.text.text);
        }
    }

    if (event.type == SDL_KEYDOWN) {

        if (event.key.keysym.sym == SDLK_BACKSPACE &&
            strlen(menu->player_name) > 0) {
            menu->player_name[strlen(menu->player_name)-1] = '\0';
        }

        if (event.key.keysym.sym == SDLK_RETURN &&
            *state == MENU_INPUT) {
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

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

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
