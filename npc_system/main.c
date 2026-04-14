/**
 * Programme de test pour le système NPC
 * - Ennemis: images PNG (Joker et Harley Quinn)
 * - Joueur: rectangle bleu
 * - ES: rectangles verts
 * - Background: image personnalisée
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "npc.h"

// Constantes
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MAP_WIDTH 2000
#define MAP_HEIGHT 1500

// Structure du joueur pour le test (rectangle bleu)
typedef struct {
    SDL_Rect rect;
    int health;
    int score;
    int speed;
} TestPlayer;

// Structure principale du test
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    bool quit;
    
    TestPlayer player;
    NPCManager npcManager;
    
    SDL_Rect camera;
    int mapWidth, mapHeight;
} TestGame;

// Prototypes
bool initTest(TestGame *game);
void handleInput(TestGame *game);
void updateTest(TestGame *game);
void renderTest(TestGame *game);
void cleanupTest(TestGame *game);
void updateCamera(TestGame *game);
void renderTextCentered(SDL_Renderer *renderer, TTF_Font *font, const char *text, 
                        SDL_Color color, int x, int y, int w, int h);

// Point d'entrée
int main(void) {
    TestGame game;
    srand((unsigned int)time(NULL));
    
    if (!initTest(&game)) {
        printf("Erreur d'initialisation\n");
        return 1;
    }
    
    // Boucle principale
    while (!game.quit) {
        Uint32 start = SDL_GetTicks();
        
        handleInput(&game);
        updateTest(&game);
        renderTest(&game);
        
        // Limitation FPS
        Uint32 elapsed = SDL_GetTicks() - start;
        if (elapsed < 16) {
            SDL_Delay(16 - elapsed);
        }
    }
    
    cleanupTest(&game);
    return 0;
}

bool initTest(TestGame *game) {
    // Initialisation SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Erreur SDL_Init: %s\n", SDL_GetError());
        return false;
    }
    
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        printf("Erreur IMG_Init: %s\n", IMG_GetError());
        return false;
    }
    
    if (TTF_Init() == -1) {
        printf("Erreur TTF_Init: %s\n", TTF_GetError());
        return false;
    }
    
    // Création fenêtre
    game->window = SDL_CreateWindow("NPC System Test - Joker vs Harley Quinn",
                                     SDL_WINDOWPOS_CENTERED,
                                     SDL_WINDOWPOS_CENTERED,
                                     SCREEN_WIDTH, SCREEN_HEIGHT,
                                     SDL_WINDOW_SHOWN);
    if (!game->window) {
        printf("Erreur création fenêtre: %s\n", SDL_GetError());
        return false;
    }
    
    game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED);
    if (!game->renderer) {
        printf("Erreur création renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(game->window);
        return false;
    }
    
    // Police (optionnelle)
    game->font = TTF_OpenFont("assets/font.ttf", 20);
    if (!game->font) {
        printf("Note: Police non chargée\n");
    }
    
    // Initialisation du joueur (rectangle bleu)
    game->player.rect = (SDL_Rect){400, 500, 40, 40};
    game->player.health = 100;
    game->player.score = 0;
    game->player.speed = 5;
    
    // Initialisation de la caméra
    game->camera = (SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    game->mapWidth = MAP_WIDTH;
    game->mapHeight = MAP_HEIGHT;
    
    game->quit = false;
    
    // Initialisation du gestionnaire NPC
    if (!initNPCManager(&game->npcManager, game->renderer)) {
        printf("Erreur initialisation NPC\n");
        return false;
    }
    
    // Charger le background
    loadBackgroundTexture(&game->npcManager, "assets/background.jpg");
    
    // Initialisation du niveau 1
    initLevel1NPC(&game->npcManager, game->renderer);
    
    // Lier les stats du joueur aux NPC
    game->npcManager.playerHealth = &game->player.health;
    game->npcManager.playerScore = &game->player.score;
    
    printf("=== TEST NPC SYSTEM ===\n");
    printf("- Ennemis: Images PNG (Joker et Harley Quinn)\n");
    printf("- Joueur: Rectangle bleu\n");
    printf("- ES: Rectangles verts (power-ups)\n");
    printf("- Background: Image personnalisée\n\n");
    printf("Controles:\n");
    printf("  - Z/Q/S/D ou Fleches: Deplacer le joueur\n");
    printf("  - ESPACE: Attaquer (inflige 25 degats aux ennemis proches)\n");
    printf("  - 1: Changer pour niveau 1\n");
    printf("  - 2: Changer pour niveau 2\n");
    printf("  - ESC: Quitter\n");
    printf("\n");
    
    return true;
}

void handleInput(TestGame *game) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            game->quit = true;
        }
        
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    game->quit = true;
                    break;
                    
                // Déplacement du joueur
                case SDLK_LEFT:
                case SDLK_q:
                    game->player.rect.x -= game->player.speed;
                    break;
                case SDLK_RIGHT:
                case SDLK_d:
                    game->player.rect.x += game->player.speed;
                    break;
                case SDLK_UP:
                case SDLK_z:
                    game->player.rect.y -= game->player.speed;
                    break;
                case SDLK_DOWN:
                case SDLK_s:
                    game->player.rect.y += game->player.speed;
                    break;
                    
                // Attaque
                case SDLK_SPACE:
                    for (int i = 0; i < game->npcManager.enemyCount; i++) {
                        NPC *enemy = &game->npcManager.enemies[i];
                        if (enemy->active) {
                            int dx = enemy->x - game->player.rect.x;
                            int dy = enemy->y - game->player.rect.y;
                            int dist = (int)sqrt(dx*dx + dy*dy);
                            if (dist < 60) {
                                damageNPC(enemy, 25);
                                printf("🗡️ Attaque!\n");
                                break;
                            }
                        }
                    }
                    break;
                    
                // Changement de niveau
                case SDLK_1:
                    initLevel1NPC(&game->npcManager, game->renderer);
                    printf("--- Passage au NIVEAU 1 ---\n");
                    break;
                case SDLK_2:
                    initLevel2NPC(&game->npcManager, game->renderer);
                    printf("--- Passage au NIVEAU 2 ---\n");
                    break;
                    
                default:
                    break;
            }
        }
    }
    
    // Limites du joueur
    if (game->player.rect.x < 0) game->player.rect.x = 0;
    if (game->player.rect.y < 0) game->player.rect.y = 0;
    if (game->player.rect.x > game->mapWidth - game->player.rect.w)
        game->player.rect.x = game->mapWidth - game->player.rect.w;
    if (game->player.rect.y > game->mapHeight - game->player.rect.h)
        game->player.rect.y = game->mapHeight - game->player.rect.h;
}

void updateTest(TestGame *game) {
    // Mise à jour des NPC
    updateNPCs(&game->npcManager, &game->player.rect, game->camera.x, game->camera.y);
    
    // Vérification des collisions
    checkAllCollisions(&game->npcManager, &game->player.rect, 
                       &game->player.health, &game->player.score);
    
    // Mise à jour de la caméra
    updateCamera(game);
    
    // Vérification game over
    if (game->player.health <= 0) {
        printf("\n💀 GAME OVER! Score final: %d 💀\n", game->player.score);
        printf("Appuyez sur 1 ou 2 pour recommencer\n\n");
        
        // Réinitialiser le joueur
        game->player.health = 100;
        game->player.score = 0;
        game->player.rect.x = 400;
        game->player.rect.y = 500;
        
        // Recharger le niveau actuel
        if (game->npcManager.currentLevel == 1) {
            initLevel1NPC(&game->npcManager, game->renderer);
        } else {
            initLevel2NPC(&game->npcManager, game->renderer);
        }
    }
    
    // Limite de vie max
    if (game->player.health > 100) game->player.health = 100;
}

void updateCamera(TestGame *game) {
    game->camera.x = game->player.rect.x + (game->player.rect.w / 2) - (SCREEN_WIDTH / 2);
    game->camera.y = game->player.rect.y + (game->player.rect.h / 2) - (SCREEN_HEIGHT / 2);
    
    if (game->camera.x < 0) game->camera.x = 0;
    if (game->camera.y < 0) game->camera.y = 0;
    if (game->camera.x > game->mapWidth - SCREEN_WIDTH)
        game->camera.x = game->mapWidth - SCREEN_WIDTH;
    if (game->camera.y > game->mapHeight - SCREEN_HEIGHT)
        game->camera.y = game->mapHeight - SCREEN_HEIGHT;
}
void renderTest(TestGame *game) {
    SDL_RenderClear(game->renderer);
    
    // Afficher le background
    renderBackground(&game->npcManager, game->camera.x, game->camera.y);
    
    // Afficher tous les NPC (ennemis en images, ES en rectangles verts)
    renderAllNPCs(&game->npcManager, game->camera.x, game->camera.y);
    
    // Afficher le joueur (rectangle bleu)
    SDL_Rect playerRect = {
        game->player.rect.x - game->camera.x,
        game->player.rect.y - game->camera.y,
        game->player.rect.w,
        game->player.rect.h
    };
    
    // Si le joueur est invincible, afficher un contour jaune clignotant
    if (isPlayerInvincible(&game->npcManager)) {
        // Effet de clignotement (alternance toutes les 100ms)
        Uint32 currentTime = SDL_GetTicks();
        if ((currentTime / 100) % 2 == 0) {
            // Joueur normal
            SDL_SetRenderDrawColor(game->renderer, 0, 100, 255, 255);
            SDL_RenderFillRect(game->renderer, &playerRect);
        } else {
            // Joueur clignotant (blanc/jaune)
            SDL_SetRenderDrawColor(game->renderer, 255, 255, 100, 255);
            SDL_RenderFillRect(game->renderer, &playerRect);
        }
    } else {
        // Joueur normal
        SDL_SetRenderDrawColor(game->renderer, 0, 100, 255, 255);
        SDL_RenderFillRect(game->renderer, &playerRect);
    }
    
    // Contour du joueur (toujours blanc)
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(game->renderer, &playerRect);
    
    // Affichage du temps d'invincibilité restant
    if (isPlayerInvincible(&game->npcManager)) {
        Uint32 currentTime = SDL_GetTicks();
        Uint32 remaining = (game->npcManager.playerInvincibilityEndTime - currentTime);
        float remainingSec = remaining / 1000.0f;
        
        // Afficher un texte au-dessus du joueur
        char invText[32];
        sprintf(invText, "INVINCIBLE: %.1fs", remainingSec);
        SDL_Color yellow = {255, 255, 0, 255};
        
        // Afficher le texte centré au-dessus du joueur
        int textX = playerRect.x + (playerRect.w / 2) - 60;
        int textY = playerRect.y - 25;
        renderTextCentered(game->renderer, game->font, invText, yellow, textX, textY, 120, 20);
        
        // Afficher un contour jaune autour du joueur
        SDL_Rect invincRect = {playerRect.x - 3, playerRect.y - 3, 
                                playerRect.w + 6, playerRect.h + 6};
        SDL_SetRenderDrawColor(game->renderer, 255, 255, 0, 255);
        SDL_RenderDrawRect(game->renderer, &invincRect);
    }
    
    // Barre de vie du joueur
    int barWidth = 100, barHeight = 10;
    int barX = 10, barY = 10;
    int currentWidth = (int)(barWidth * (game->player.health / 100.0f));
    
    // Fond de la barre
    SDL_SetRenderDrawColor(game->renderer, 60, 60, 60, 255);
    SDL_RenderFillRect(game->renderer, &(SDL_Rect){barX, barY, barWidth, barHeight});
    
    // Barre de vie (couleur selon santé)
    if (game->player.health > 50)
        SDL_SetRenderDrawColor(game->renderer, 0, 200, 0, 255);
    else if (game->player.health > 25)
        SDL_SetRenderDrawColor(game->renderer, 255, 150, 0, 255);
    else
        SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(game->renderer, &(SDL_Rect){barX, barY, currentWidth, barHeight});
    
    // Bordure de la barre
    SDL_SetRenderDrawColor(game->renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(game->renderer, &(SDL_Rect){barX, barY, barWidth, barHeight});
    
    // Afficher la valeur de la vie en texte
    char healthText[16];
    sprintf(healthText, "%d / 100", game->player.health);
    SDL_Color white = {255, 255, 255, 255};
    renderTextCentered(game->renderer, game->font, healthText, white, barX + barWidth + 5, barY, 60, barHeight);
    
    // Score
    char scoreText[32];
    sprintf(scoreText, "Score: %d", game->player.score);
    renderTextCentered(game->renderer, game->font, scoreText, white, 120, 10, 150, 30);
    
    // Niveau actuel
    char levelText[32];
    sprintf(levelText, "Niveau: %d", game->npcManager.currentLevel);
    renderTextCentered(game->renderer, game->font, levelText, white, SCREEN_WIDTH - 100, 10, 90, 30);
    
    // Instructions
    char helpText[] = "Z/Q/S/D: Move | SPACE: Attack | 1/2: Level | ESC: Quit";
    renderTextCentered(game->renderer, game->font, helpText, white, 10, SCREEN_HEIGHT - 30, SCREEN_WIDTH - 20, 25);
    
    // Compteur d'ennemis
    char enemyCountText[64];
    int activeEnemies = 0;
    for (int i = 0; i < game->npcManager.enemyCount; i++) {
        if (game->npcManager.enemies[i].active) activeEnemies++;
    }
    sprintf(enemyCountText, "Ennemis: %d / %d", activeEnemies, game->npcManager.enemyCount);
    renderTextCentered(game->renderer, game->font, enemyCountText, white, SCREEN_WIDTH - 140, 45, 130, 25);
    
    // Afficher le temps d'invincibilité global (en haut à droite)
    if (isPlayerInvincible(&game->npcManager)) {
        Uint32 remaining = (game->npcManager.playerInvincibilityEndTime - SDL_GetTicks());
        float remainingSec = remaining / 1000.0f;
        char invTimeText[32];
        sprintf(invTimeText, "Invincible: %.1fs", remainingSec);
        renderTextCentered(game->renderer, game->font, invTimeText, (SDL_Color){255,255,0,255}, 
                           SCREEN_WIDTH - 120, 75, 110, 20);
    }
    
    SDL_RenderPresent(game->renderer);
}

void renderTextCentered(SDL_Renderer *renderer, TTF_Font *font, const char *text, 
                        SDL_Color color, int x, int y, int w, int h) {
    if (!text) return;
    
    if (!font) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        SDL_Rect rect = {x, y, w, h};
        SDL_RenderDrawRect(renderer, &rect);
        return;
    }
    
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) return;
    
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_Rect rect = {
        x + (w - surface->w) / 2,
        y + (h - surface->h) / 2,
        surface->w,
        surface->h
    };
    
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void cleanupTest(TestGame *game) {
    cleanupNPCs(&game->npcManager);
    if (game->font) TTF_CloseFont(game->font);
    if (game->renderer) SDL_DestroyRenderer(game->renderer);
    if (game->window) SDL_DestroyWindow(game->window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
