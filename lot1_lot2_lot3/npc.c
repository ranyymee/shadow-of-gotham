#include "npc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

SDL_Texture* loadTex(SDL_Renderer *r, const char *p) {
    SDL_Texture *t = IMG_LoadTexture(r, p);
    if (!t) printf("Manque: %s\n", p);
    return t;
}

void NPC_init(GameNPC *n, SDL_Renderer *r, const char *bgPath) {
    memset(n, 0, sizeof(GameNPC));
    n->renderer = r;
    n->bg = loadTex(r, bgPath);
    n->itemTex = loadTex(r, "assets/es.png");
    srand(time(NULL));
}

void NPC_loadLevel(GameNPC *n, SDL_Renderer *r, int lvl) {
    char path[128];
    n->level = lvl;
    n->enemyCnt = 0;
    n->itemCnt = 0;

    int nbEnemies = (lvl == 1) ? 2 : 3;
    int pos[3][2] = {{500,450}, {700,400}, {900,500}};
    int hpVals[3] = {60,50,85};
    int dmgVals[3] = {15,12,22};
    int ptsVals[3] = {100,80,180};

    for (int i = 0; i < nbEnemies; i++) {
        NPC e; memset(&e, 0, sizeof(NPC));
        e.type = ENEMY;
        e.enemyType = (i % 2 == 0) ? JOKER : HARLEY;
        e.x = pos[i][0] - (lvl==2 ? 50 : 0);
        e.y = pos[i][1];
        e.w = (lvl==1) ? 70 : 80;
        e.h = e.w;
        e.rect = (SDL_Rect){e.x, e.y, e.w, e.h};
        e.speed = (lvl==1) ? 2 : 3;
        e.traj = i % 3;
        e.dx = (e.traj == 0 || e.traj == 2) ? 1 : 0;
        e.dy = (e.traj == 1 || e.traj == 2) ? 1 : 0;
        e.minX = e.x - 150; e.maxX = e.x + 150;
        e.minY = e.y - 100; e.maxY = e.y + 100;
        e.damage = dmgVals[i];
        e.score = ptsVals[i];
        e.maxHealth = hpVals[i];
        e.health = hpVals[i];
        e.healthState = ALIVE;
        e.active = 1;
        const char *name = (e.enemyType == JOKER) ? "joker" : "harley";
        for (int f = 0; f < ANIM_FRAMES; f++) {
            sprintf(path, "assets/%s_run%d.png", name, f+1);
            e.run[f] = loadTex(r, path);
            sprintf(path, "assets/%s_attack%d.png", name, f+1);
            e.attack[f] = loadTex(r, path);
            sprintf(path, "assets/%s_death%d.png", name, f+1);
            e.death[f] = loadTex(r, path);
        }
        e.tex = e.run[0];
        n->enemies[n->enemyCnt++] = e;
    }

    NPC it1; memset(&it1, 0, sizeof(NPC));
    it1.type = POWERUP;
    it1.x = 300; it1.y = 500; it1.w = 30; it1.h = 30;
    it1.rect = (SDL_Rect){it1.x, it1.y, it1.w, it1.h};
    it1.speed = 1; it1.traj = HORIZONTAL; it1.dx = 1;
    it1.minX = 250; it1.maxX = 450;
    it1.damage = -20; it1.score = 50; it1.active = 1;
    n->items[n->itemCnt++] = it1;

    NPC it2; memset(&it2, 0, sizeof(NPC));
    it2.type = POWERUP;
    it2.x = 800; it2.y = 550; it2.w = 30; it2.h = 30;
    it2.rect = (SDL_Rect){it2.x, it2.y, it2.w, it2.h};
    it2.speed = 2; it2.traj = DIAGONAL; it2.dx = -1; it2.dy = -1;
    it2.minX = 700; it2.maxX = 900;
    it2.minY = 500; it2.maxY = 600;
    it2.damage = 0; it2.score = 100; it2.active = 1;
    n->items[n->itemCnt++] = it2;

    if (lvl == 2) {
        NPC it3; memset(&it3, 0, sizeof(NPC));
        it3.type = POWERUP;
        it3.x = 500; it3.y = 300; it3.w = 35; it3.h = 35;
        it3.rect = (SDL_Rect){it3.x, it3.y, it3.w, it3.h};
        it3.speed = 2; it3.traj = HORIZONTAL; it3.dx = 1;
        it3.minX = 400; it3.maxX = 700;
        it3.damage = -40; it3.score = 80; it3.active = 1;
        n->items[n->itemCnt++] = it3;
    }
}

void NPC_update(GameNPC *n, SDL_Rect *player, int *health, int *score, int cx, int cy) {
    for (int i = 0; i < n->enemyCnt; i++) {
        NPC *e = &n->enemies[i];
        if (!e->active && !e->dying) continue;

        if (e->active && !e->dying && !e->attacking) {
            if (e->traj == HORIZONTAL) {
                e->x += e->speed * e->dx;
                if (e->x <= e->minX) { e->x = e->minX; e->dx = 1; }
                if (e->x >= e->maxX) { e->x = e->maxX; e->dx = -1; }
            }
            else if (e->traj == VERTICAL) {
                e->y += e->speed * e->dy;
                if (e->y <= e->minY) { e->y = e->minY; e->dy = 1; }
                if (e->y >= e->maxY) { e->y = e->maxY; e->dy = -1; }
            }
            else if (e->traj == DIAGONAL) {
                e->x += e->speed * e->dx;
                e->y += e->speed * e->dy;
                if (e->x <= e->minX || e->x >= e->maxX) e->dx = -e->dx;
                if (e->y <= e->minY || e->y >= e->maxY) e->dy = -e->dy;
            }
            e->rect.x = e->x; e->rect.y = e->y;
        }

        e->step++;
        int delay = (e->dying) ? 3 : (e->attacking) ? 4 : 6;
        if (e->step >= delay) {
            e->step = 0;
            e->frame++;
            if (e->dying) {
                if (e->frame >= ANIM_FRAMES) { e->active = 0; e->dying = 0; continue; }
                e->tex = e->death[e->frame];
            }
            else if (e->attacking) {
                if (e->frame >= ANIM_FRAMES) {
                    e->attacking = 0;
                    e->frame = 0;
                    e->tex = e->run[0];
                } else {
                    e->tex = e->attack[e->frame];
                }
            }
            else {
                if (e->frame >= ANIM_FRAMES) e->frame = 0;
                e->tex = e->run[e->frame];
            }
        }

        if (!e->active) continue;
        if (SDL_HasIntersection(&e->rect, player)) {
            if (SDL_GetTicks() > n->invincibleEnd) {
                *health -= e->damage;
                n->invincibleEnd = SDL_GetTicks() + 1500;
                e->attacking = 1;
                e->frame = 0;
                e->step = 0;
                e->tex = e->attack[0];
                for (int j = 0; j < 20; j++) {
                    for (int k = 0; k < MAX_BLOOD; k++) {
                        if (!n->blood[k].active) {
                            n->blood[k].x = (e->x - cx) + (rand() % 60) - 30;
                            n->blood[k].y = (e->y - cy) + (rand() % 60) - 30;
                            n->blood[k].life = 30;
                            n->blood[k].active = 1;
                            break;
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < n->itemCnt; i++) {
        NPC *it = &n->items[i];
        if (!it->active) continue;
        if (it->traj == HORIZONTAL) {
            it->x += it->speed * it->dx;
            if (it->x <= it->minX) { it->x = it->minX; it->dx = 1; }
            if (it->x >= it->maxX) { it->x = it->maxX; it->dx = -1; }
        }
        else if (it->traj == VERTICAL) {
            it->y += it->speed * it->dy;
            if (it->y <= it->minY) { it->y = it->minY; it->dy = 1; }
            if (it->y >= it->maxY) { it->y = it->maxY; it->dy = -1; }
        }
        else if (it->traj == DIAGONAL) {
            it->x += it->speed * it->dx;
            it->y += it->speed * it->dy;
            if (it->x <= it->minX || it->x >= it->maxX) it->dx = -it->dx;
            if (it->y <= it->minY || it->y >= it->maxY) it->dy = -it->dy;
        }
        it->rect.x = it->x; it->rect.y = it->y;
        if (SDL_HasIntersection(&it->rect, player)) {
            *health -= it->damage;
            *score += it->score;
            it->active = 0;
        }
    }

    for (int i = 0; i < MAX_BLOOD; i++) {
        if (n->blood[i].active) {
            n->blood[i].life--;
            if (n->blood[i].life <= 0) n->blood[i].active = 0;
        }
    }
}

void NPC_draw(GameNPC *n, int cx, int cy) {
    for (int i = 0; i < n->enemyCnt; i++) {
        NPC *e = &n->enemies[i];
        if (!e->active && !e->dying) continue;
        SDL_Rect r = {e->x - cx, e->y - cy, e->w, e->h};
        if (e->tex) SDL_RenderCopy(n->renderer, e->tex, NULL, &r);
        if (e->healthState != DEAD && !e->dying) {
            int bx = e->x - cx, by = e->y - cy - 12;
            int percent = (e->health * 100) / e->maxHealth;
            int cw = (e->w * percent) / 100;
            SDL_SetRenderDrawColor(n->renderer, 60,60,60,255);
            SDL_Rect bg = {bx, by, e->w, 6};
            SDL_RenderFillRect(n->renderer, &bg);
            SDL_SetRenderDrawColor(n->renderer, percent>50?0:255, percent>50?200:150, 0, 255);
            SDL_Rect fg = {bx, by, cw, 6};
            SDL_RenderFillRect(n->renderer, &fg);
            SDL_SetRenderDrawColor(n->renderer, 255,255,255,255);
            SDL_RenderDrawRect(n->renderer, &bg);
        }
    }

    for (int i = 0; i < n->itemCnt; i++) {
        NPC *it = &n->items[i];
        if (!it->active) continue;
        SDL_Rect r = {it->x - cx, it->y - cy, it->w, it->h};
        if (n->itemTex) SDL_RenderCopy(n->renderer, n->itemTex, NULL, &r);
    }

    for (int i = 0; i < MAX_BLOOD; i++) {
        if (n->blood[i].active) {
            int alpha = (n->blood[i].life * 255) / 30;
            SDL_SetRenderDrawColor(n->renderer, 255, 0, 0, alpha);
            SDL_Rect spot = {n->blood[i].x, n->blood[i].y, 3, 3};
            SDL_RenderFillRect(n->renderer, &spot);
        }
    }
}

void NPC_clean(GameNPC *n) {
    for (int i = 0; i < n->enemyCnt; i++) {
        for (int f = 0; f < ANIM_FRAMES; f++) {
            if (n->enemies[i].run[f]) SDL_DestroyTexture(n->enemies[i].run[f]);
            if (n->enemies[i].attack[f]) SDL_DestroyTexture(n->enemies[i].attack[f]);
            if (n->enemies[i].death[f]) SDL_DestroyTexture(n->enemies[i].death[f]);
        }
    }
    if (n->bg) SDL_DestroyTexture(n->bg);
    if (n->itemTex) SDL_DestroyTexture(n->itemTex);
}
