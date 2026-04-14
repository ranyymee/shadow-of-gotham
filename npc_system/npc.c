#include "npc.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

// ========== FONCTION DE CHARGEMENT DE TEXTURE ==========

static SDL_Texture* loadTexture(SDL_Renderer *renderer, const char *path) {
    SDL_Texture *texture = IMG_LoadTexture(renderer, path);
    if (!texture) {
        printf("Note: Texture non chargée: %s\n", path);
    } else {
        printf("OK: Texture chargée: %s\n", path);
    }
    return texture;
}

// ========== FONCTIONS D'INVINCIBILITÉ ==========

bool isPlayerInvincible(NPCManager *manager) {
    Uint32 currentTime = SDL_GetTicks();
    return (currentTime < manager->playerInvincibilityEndTime);
}

void setPlayerInvincible(NPCManager *manager) {
    Uint32 currentTime = SDL_GetTicks();
    manager->playerInvincibilityEndTime = currentTime + manager->invincibilityDuration;
}

// ========== INITIALISATION ==========

bool initNPCManager(NPCManager *manager, SDL_Renderer *renderer) {
    memset(manager, 0, sizeof(NPCManager));
    manager->renderer = renderer;
    manager->playerHealth = NULL;
    manager->playerScore = NULL;
    manager->backgroundTexture = NULL;
    
    // Configuration de l'invincibilité (0.9 secondes après chaque attaque)
    manager->playerInvincibilityEndTime = 0;
    manager->invincibilityDuration = 900;  // 900 ms = 0.9secondes
    
    srand((unsigned int)time(NULL));
    return true;
}

bool loadEnemyTexture(NPC *npc, SDL_Renderer *renderer, const char *path) {
    npc->texture = loadTexture(renderer, path);
    return (npc->texture != NULL);
}

bool loadBackgroundTexture(NPCManager *manager, const char *path) {
    manager->backgroundTexture = loadTexture(manager->renderer, path);
    return (manager->backgroundTexture != NULL);
}

void initHealthBar(NPC *npc, int maxHealth, int width, int height) {
    npc->healthBar.maxHealth = maxHealth;
    npc->healthBar.currentHealth = maxHealth;
    npc->healthBar.width = width;
    npc->healthBar.height = height;
    npc->healthState = HEALTH_ALIVE;
}

void initLevel1NPC(NPCManager *manager, SDL_Renderer *renderer) {
    manager->currentLevel = 1;
    manager->enemyCount = 0;
    manager->esCount = 0;
    
    // ========== JOKER - Ennemi Level 1 ==========
    NPC joker;
    memset(&joker, 0, sizeof(NPC));
    joker.type = NPC_TYPE_ENEMY;
    joker.enemyType = ENEMY_TYPE_JOKER;
    joker.id = 0;
    joker.x = 500;
    joker.y = 450;
    joker.width = 48;
    joker.height = 48;
    joker.rect = (SDL_Rect){joker.x, joker.y, joker.width, joker.height};
    joker.speed = 2;
    joker.trajectory = TRAJECTORY_HORIZONTAL;
    joker.directionX = 1;
    joker.patrolMinX = 350;
    joker.patrolMaxX = 750;
    joker.damage = 15;
    joker.scoreValue = 100;
    joker.active = true;
    joker.canAttack = true;
    joker.isAggro = true;
    joker.aggroRange = 150;
    joker.attackRange = 40;
    joker.attackCooldown = 1000;  // 1 seconde entre les attaques
    initHealthBar(&joker, 60, 48, 6);
    loadEnemyTexture(&joker, renderer, "assets/joker.png");
    manager->enemies[manager->enemyCount++] = joker;
    
    // ========== HARLEY QUINN - Ennemi Level 1 ==========
    NPC harley;
    memset(&harley, 0, sizeof(NPC));
    harley.type = NPC_TYPE_ENEMY;
    harley.enemyType = ENEMY_TYPE_HARLEY;
    harley.id = 1;
    harley.x = 700;
    harley.y = 400;
    harley.width = 45;
    harley.height = 45;
    harley.rect = (SDL_Rect){harley.x, harley.y, harley.width, harley.height};
    harley.speed = 3;
    harley.trajectory = TRAJECTORY_VERTICAL;
    harley.directionY = 1;
    harley.patrolMinY = 300;
    harley.patrolMaxY = 500;
    harley.damage = 12;
    harley.scoreValue = 80;
    harley.active = true;
    harley.canAttack = true;
    harley.isAggro = true;
    harley.aggroRange = 130;
    harley.attackRange = 38;
    harley.attackCooldown = 900;  // 0.9 seconde entre les attaques
    initHealthBar(&harley, 50, 45, 5);
    loadEnemyTexture(&harley, renderer, "assets/harley.png");
    manager->enemies[manager->enemyCount++] = harley;
    
    // ========== ES (Power-ups) Level 1 ==========
    
    // ES Soin
    NPC esHealth;
    memset(&esHealth, 0, sizeof(NPC));
    esHealth.type = NPC_TYPE_ES;
    esHealth.id = 0;
    esHealth.x = 300;
    esHealth.y = 500;
    esHealth.width = 30;
    esHealth.height = 30;
    esHealth.rect = (SDL_Rect){esHealth.x, esHealth.y, esHealth.width, esHealth.height};
    esHealth.speed = 1;
    esHealth.trajectory = TRAJECTORY_HORIZONTAL;
    esHealth.directionX = 1;
    esHealth.patrolMinX = 250;
    esHealth.patrolMaxX = 450;
    esHealth.damage = -20;
    esHealth.scoreValue = 50;
    esHealth.active = true;
    manager->es[manager->esCount++] = esHealth;
    
    // ES Points
    NPC esScore;
    memset(&esScore, 0, sizeof(NPC));
    esScore.type = NPC_TYPE_ES;
    esScore.id = 1;
    esScore.x = 800;
    esScore.y = 550;
    esScore.width = 30;
    esScore.height = 30;
    esScore.rect = (SDL_Rect){esScore.x, esScore.y, esScore.width, esScore.height};
    esScore.speed = 2;
    esScore.trajectory = TRAJECTORY_DIAGONAL;
    esScore.directionX = -1;
    esScore.directionY = -1;
    esScore.patrolMinX = 700;
    esScore.patrolMaxX = 900;
    esScore.patrolMinY = 500;
    esScore.patrolMaxY = 600;
    esScore.damage = 0;
    esScore.scoreValue = 100;
    esScore.active = true;
    manager->es[manager->esCount++] = esScore;
    
    printf("[NIVEAU 1] %d ennemis, %d ES\n", manager->enemyCount, manager->esCount);
}

void initLevel2NPC(NPCManager *manager, SDL_Renderer *renderer) {
    manager->currentLevel = 2;
    manager->enemyCount = 0;
    manager->esCount = 0;
    
    // ========== JOKER - Plus fort Level 2 ==========
    NPC joker;
    memset(&joker, 0, sizeof(NPC));
    joker.type = NPC_TYPE_ENEMY;
    joker.enemyType = ENEMY_TYPE_JOKER;
    joker.id = 0;
    joker.x = 400;
    joker.y = 450;
    joker.width = 52;
    joker.height = 52;
    joker.rect = (SDL_Rect){joker.x, joker.y, joker.width, joker.height};
    joker.speed = 3;
    joker.trajectory = TRAJECTORY_DIAGONAL;
    joker.directionX = 1;
    joker.directionY = 1;
    joker.patrolMinX = 300;
    joker.patrolMaxX = 700;
    joker.patrolMinY = 400;
    joker.patrolMaxY = 550;
    joker.damage = 20;
    joker.scoreValue = 150;
    joker.active = true;
    joker.canAttack = true;
    joker.isAggro = true;
    joker.aggroRange = 180;
    joker.attackRange = 45;
    joker.attackCooldown = 800;
    initHealthBar(&joker, 80, 52, 6);
    loadEnemyTexture(&joker, renderer, "assets/joker.png");
    manager->enemies[manager->enemyCount++] = joker;
    
    // ========== HARLEY QUINN - Plus forte Level 2 ==========
    NPC harley;
    memset(&harley, 0, sizeof(NPC));
    harley.type = NPC_TYPE_ENEMY;
    harley.enemyType = ENEMY_TYPE_HARLEY;
    harley.id = 1;
    harley.x = 650;
    harley.y = 350;
    harley.width = 48;
    harley.height = 48;
    harley.rect = (SDL_Rect){harley.x, harley.y, harley.width, harley.height};
    harley.speed = 4;
    harley.trajectory = TRAJECTORY_HORIZONTAL;
    harley.directionX = -1;
    harley.patrolMinX = 500;
    harley.patrolMaxX = 850;
    harley.damage = 18;
    harley.scoreValue = 120;
    harley.active = true;
    harley.canAttack = true;
    harley.isAggro = true;
    harley.aggroRange = 160;
    harley.attackRange = 42;
    harley.attackCooldown = 750;
    initHealthBar(&harley, 70, 48, 5);
    loadEnemyTexture(&harley, renderer, "assets/harley.png");
    manager->enemies[manager->enemyCount++] = harley;
    
    // ========== DEUXIÈME JOKER Level 2 ==========
    NPC joker2;
    memset(&joker2, 0, sizeof(NPC));
    joker2.type = NPC_TYPE_ENEMY;
    joker2.enemyType = ENEMY_TYPE_JOKER;
    joker2.id = 2;
    joker2.x = 900;
    joker2.y = 500;
    joker2.width = 50;
    joker2.height = 50;
    joker2.rect = (SDL_Rect){joker2.x, joker2.y, joker2.width, joker2.height};
    joker2.speed = 3;
    joker2.trajectory = TRAJECTORY_VERTICAL;
    joker2.directionY = -1;
    joker2.patrolMinY = 400;
    joker2.patrolMaxY = 600;
    joker2.damage = 22;
    joker2.scoreValue = 180;
    joker2.active = true;
    joker2.canAttack = true;
    joker2.isAggro = true;
    joker2.aggroRange = 170;
    joker2.attackRange = 45;
    joker2.attackCooldown = 850;
    initHealthBar(&joker2, 85, 50, 6);
    loadEnemyTexture(&joker2, renderer, "assets/joker.png");
    manager->enemies[manager->enemyCount++] = joker2;
    
    // ========== ES Power-ups Level 2 ==========
    
    // ES Soin puissant
    NPC esHealth;
    memset(&esHealth, 0, sizeof(NPC));
    esHealth.type = NPC_TYPE_ES;
    esHealth.id = 0;
    esHealth.x = 500;
    esHealth.y = 300;
    esHealth.width = 35;
    esHealth.height = 35;
    esHealth.rect = (SDL_Rect){esHealth.x, esHealth.y, esHealth.width, esHealth.height};
    esHealth.speed = 2;
    esHealth.trajectory = TRAJECTORY_HORIZONTAL;
    esHealth.directionX = 1;
    esHealth.patrolMinX = 400;
    esHealth.patrolMaxX = 700;
    esHealth.damage = -40;
    esHealth.scoreValue = 80;
    esHealth.active = true;
    manager->es[manager->esCount++] = esHealth;
    
    // ES Points bonus
    NPC esScore;
    memset(&esScore, 0, sizeof(NPC));
    esScore.type = NPC_TYPE_ES;
    esScore.id = 1;
    esScore.x = 750;
    esScore.y = 550;
    esScore.width = 32;
    esScore.height = 32;
    esScore.rect = (SDL_Rect){esScore.x, esScore.y, esScore.width, esScore.height};
    esScore.speed = 3;
    esScore.trajectory = TRAJECTORY_DIAGONAL;
    esScore.directionX = -1;
    esScore.directionY = -1;
    esScore.patrolMinX = 650;
    esScore.patrolMaxX = 850;
    esScore.patrolMinY = 500;
    esScore.patrolMaxY = 600;
    esScore.damage = 0;
    esScore.scoreValue = 200;
    esScore.active = true;
    manager->es[manager->esCount++] = esScore;
    
    // ES Power-up spécial
    NPC esPower;
    memset(&esPower, 0, sizeof(NPC));
    esPower.type = NPC_TYPE_ES;
    esPower.id = 2;
    esPower.x = 300;
    esPower.y = 200;
    esPower.width = 32;
    esPower.height = 32;
    esPower.rect = (SDL_Rect){esPower.x, esPower.y, esPower.width, esPower.height};
    esPower.speed = 2;
    esPower.trajectory = TRAJECTORY_VERTICAL;
    esPower.directionY = 1;
    esPower.patrolMinY = 150;
    esPower.patrolMaxY = 350;
    esPower.damage = 0;
    esPower.scoreValue = 150;
    esPower.active = true;
    manager->es[manager->esCount++] = esPower;
    
    printf("[NIVEAU 2] %d ennemis, %d ES\n", manager->enemyCount, manager->esCount);
}

// ========== DÉPLACEMENT ==========

void updateRandomMovement(NPC *npc) {
    if (!npc->active) return;
    
    switch (npc->trajectory) {
        case TRAJECTORY_HORIZONTAL:
            npc->x += npc->speed * npc->directionX;
            if (npc->x <= npc->patrolMinX) {
                npc->x = npc->patrolMinX;
                npc->directionX = 1;
            } else if (npc->x >= npc->patrolMaxX) {
                npc->x = npc->patrolMaxX;
                npc->directionX = -1;
            }
            break;
            
        case TRAJECTORY_VERTICAL:
            npc->y += npc->speed * npc->directionY;
            if (npc->y <= npc->patrolMinY) {
                npc->y = npc->patrolMinY;
                npc->directionY = 1;
            } else if (npc->y >= npc->patrolMaxY) {
                npc->y = npc->patrolMaxY;
                npc->directionY = -1;
            }
            break;
            
        case TRAJECTORY_DIAGONAL:
            npc->x += npc->speed * npc->directionX;
            npc->y += npc->speed * npc->directionY;
            
            if (npc->x <= npc->patrolMinX || npc->x >= npc->patrolMaxX) {
                npc->directionX = -npc->directionX;
            }
            if (npc->y <= npc->patrolMinY || npc->y >= npc->patrolMaxY) {
                npc->directionY = -npc->directionY;
            }
            break;
    }
    
    npc->rect.x = npc->x;
    npc->rect.y = npc->y;
}

void updateAIPursuit(NPC *npc, SDL_Rect *playerRect) {
    if (!npc->active || npc->type != NPC_TYPE_ENEMY) return;
    if (!npc->isAggro) return;
    if (npc->healthState == HEALTH_NEUTRALIZED) return;
    
    int dx = playerRect->x - npc->x;
    int dy = playerRect->y - npc->y;
    int distance = (int)sqrt(dx * dx + dy * dy);
    
    // Poursuite si le joueur est dans la zone d'agressivité
    if (distance <= npc->aggroRange) {
        // Déplacement vers le joueur
        if (abs(dx) > abs(dy)) {
            npc->x += (dx > 0 ? npc->speed : -npc->speed);
        } else {
            npc->y += (dy > 0 ? npc->speed : -npc->speed);
        }
    } else {
        // Déplacement aléatoire si hors zone
        updateRandomMovement(npc);
    }
    
    npc->rect.x = npc->x;
    npc->rect.y = npc->y;
}

void updateNPCs(NPCManager *manager, SDL_Rect *playerRect, int cameraX, int cameraY) {
    (void)cameraX;
    (void)cameraY;
    
    // Mise à jour des ennemis
    for (int i = 0; i < manager->enemyCount; i++) {
        NPC *npc = &manager->enemies[i];
        if (!npc->active) continue;
        updateAIPursuit(npc, playerRect);
    }
    
    // Mise à jour des ES
    for (int i = 0; i < manager->esCount; i++) {
        NPC *npc = &manager->es[i];
        if (!npc->active) continue;
        updateRandomMovement(npc);
    }
}

// ========== COLLISION ==========

bool checkCollisionBB(SDL_Rect *a, SDL_Rect *b) {
    return SDL_HasIntersection(a, b);
}

// UNE SEULE FONCTION pour gérer Joueur/Ennemi ET Joueur/ES
bool handleCollisionAndReact(NPCManager *manager, NPC *npc, SDL_Rect *playerRect, int *playerHealth, int *playerScore) {
    if (!npc->active) return false;
    if (!checkCollisionBB(&npc->rect, playerRect)) return false;
    
    if (npc->type == NPC_TYPE_ENEMY) {
        // Vérifier si le joueur est invincible
        if (isPlayerInvincible(manager)) {
            return true;  // Pas de dégâts
        }
        
        // Collision avec ennemi : infliger des dégâts
        if (npc->damage > 0 && playerHealth) {
            *playerHealth -= npc->damage;
            const char *name = (npc->enemyType == ENEMY_TYPE_JOKER) ? "Joker" : "Harley";
            printf("💀 %s attaque! Dégâts: %d, Vie: %d\n", name, npc->damage, *playerHealth);
            
            // Activer l'invincibilité
            setPlayerInvincible(manager);
        }
        return true;
    } 
    else if (npc->type == NPC_TYPE_ES) {
        // Collision avec ES : gagner score et/ou vie
        if (npc->damage < 0 && playerHealth) {
            *playerHealth -= npc->damage;  // damage négatif = soin
            printf("❤️ ES ramassé! Soin: %d, Vie: %d\n", -npc->damage, *playerHealth);
        }
        if (playerScore) {
            *playerScore += npc->scoreValue;
            printf("⭐ ES ramassé! +%d points, Score: %d\n", npc->scoreValue, *playerScore);
        }
        npc->active = false;  // L'ES disparaît après ramassage
        return true;
    }
    
    return false;
}

// Appelle handleCollisionAndReact pour tous les NPC (ennemis ET ES)
void checkAllCollisions(NPCManager *manager, SDL_Rect *playerRect, int *playerHealth, int *playerScore) {
    // 1er appel: collision avec les ennemis
    for (int i = 0; i < manager->enemyCount; i++) {
        handleCollisionAndReact(manager, &manager->enemies[i], playerRect, playerHealth, playerScore);
    }
    
    // 2ème appel: collision avec les ES (même fonction)
    for (int i = 0; i < manager->esCount; i++) {
        handleCollisionAndReact(manager, &manager->es[i], playerRect, playerHealth, playerScore);
    }
}

// ========== SANTÉ ==========

void updateHealthState(NPC *npc) {
    if (npc->type != NPC_TYPE_ENEMY) return;
    
    if (npc->healthBar.currentHealth <= 0) {
        npc->healthState = HEALTH_NEUTRALIZED;
        npc->active = false;
        const char *name = (npc->enemyType == ENEMY_TYPE_JOKER) ? "Joker" : "Harley";
        printf("💀 %s vaincu! +%d points\n", name, npc->scoreValue);
    } 
    else if ((float)npc->healthBar.currentHealth / npc->healthBar.maxHealth <= 0.5f) {
        npc->healthState = HEALTH_INJURED;
    } 
    else {
        npc->healthState = HEALTH_ALIVE;
    }
}

void damageNPC(NPC *npc, int damage) {
    if (npc->type != NPC_TYPE_ENEMY) return;
    if (!npc->active) return;
    
    npc->healthBar.currentHealth -= damage;
    if (npc->healthBar.currentHealth < 0) {
        npc->healthBar.currentHealth = 0;
    }
    
    updateHealthState(npc);
    
    const char *name = (npc->enemyType == ENEMY_TYPE_JOKER) ? "Joker" : "Harley";
    printf("⚔️ %s blessé! Vie: %d/%d\n", name, npc->healthBar.currentHealth, npc->healthBar.maxHealth);
}

void renderHealthBar(NPCManager *manager, NPC *npc, int cameraX, int cameraY) {
    if (npc->type != NPC_TYPE_ENEMY) return;
    if (npc->healthState == HEALTH_NEUTRALIZED) return;
    if (!manager->renderer) return;
    
    int barX = npc->x - cameraX;
    int barY = npc->y - cameraY - 10;
    int barWidth = npc->healthBar.width;
    int barHeight = npc->healthBar.height;
    
    float healthPercent = (float)npc->healthBar.currentHealth / npc->healthBar.maxHealth;
    int currentWidth = (int)(barWidth * healthPercent);
    if (currentWidth < 0) currentWidth = 0;
    
    // Fond de la barre (gris)
    SDL_Rect bgRect = {barX, barY, barWidth, barHeight};
    SDL_SetRenderDrawColor(manager->renderer, 60, 60, 60, 255);
    SDL_RenderFillRect(manager->renderer, &bgRect);
    
    // Barre de vie (couleur selon état)
    SDL_Rect fgRect = {barX, barY, currentWidth, barHeight};
    if (npc->healthState == HEALTH_INJURED) {
        SDL_SetRenderDrawColor(manager->renderer, 255, 150, 0, 255);  // Orange
    } else {
        SDL_SetRenderDrawColor(manager->renderer, 0, 200, 0, 255);    // Vert
    }
    SDL_RenderFillRect(manager->renderer, &fgRect);
    
    // Bordure
    SDL_SetRenderDrawColor(manager->renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(manager->renderer, &bgRect);
}

// ========== RENDU ==========

void renderBackground(NPCManager *manager, int cameraX, int cameraY) {
    if (!manager->renderer) return;
    
    if (manager->backgroundTexture) {
        SDL_Rect bgRect = {-cameraX, -cameraY, 2000, 1500};
        SDL_RenderCopy(manager->renderer, manager->backgroundTexture, NULL, &bgRect);
    } else {
        SDL_SetRenderDrawColor(manager->renderer, 20, 20, 40, 255);
        SDL_RenderFillRect(manager->renderer, NULL);
    }
}

void renderNPC(SDL_Renderer *renderer, NPC *npc, int cameraX, int cameraY) {
    if (!npc->active) return;
    
    SDL_Rect renderRect = {
        npc->x - cameraX,
        npc->y - cameraY,
        npc->width,
        npc->height
    };
    
    // Ennemis: utiliser l'image PNG
    if (npc->type == NPC_TYPE_ENEMY && npc->texture) {
        SDL_RenderCopy(renderer, npc->texture, NULL, &renderRect);
    } 
    // ES: rectangles verts
    else if (npc->type == NPC_TYPE_ES) {
        SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
        SDL_RenderFillRect(renderer, &renderRect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &renderRect);
    }
    else {
        // Fallback
        if (npc->type == NPC_TYPE_ENEMY) {
            if (npc->enemyType == ENEMY_TYPE_JOKER) {
                SDL_SetRenderDrawColor(renderer, 150, 0, 150, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 200, 100, 200, 255);
            }
            SDL_RenderFillRect(renderer, &renderRect);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &renderRect);
        }
    }
}

void renderAllNPCs(NPCManager *manager, int cameraX, int cameraY) {
    // Afficher les ennemis
    for (int i = 0; i < manager->enemyCount; i++) {
        if (manager->enemies[i].active) {
            renderNPC(manager->renderer, &manager->enemies[i], cameraX, cameraY);
            renderHealthBar(manager, &manager->enemies[i], cameraX, cameraY);
        }
    }
    
    // Afficher les ES (rectangles verts)
    for (int i = 0; i < manager->esCount; i++) {
        if (manager->es[i].active) {
            SDL_Rect renderRect = {
                manager->es[i].x - cameraX,
                manager->es[i].y - cameraY,
                manager->es[i].width,
                manager->es[i].height
            };
            SDL_SetRenderDrawColor(manager->renderer, 0, 200, 0, 255);
            SDL_RenderFillRect(manager->renderer, &renderRect);
            SDL_SetRenderDrawColor(manager->renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(manager->renderer, &renderRect);
        }
    }
}

void cleanupNPCs(NPCManager *manager) {
    // Libérer les textures des ennemis
    for (int i = 0; i < manager->enemyCount; i++) {
        if (manager->enemies[i].texture) {
            SDL_DestroyTexture(manager->enemies[i].texture);
        }
    }
    
    // Libérer la texture du background
    if (manager->backgroundTexture) {
        SDL_DestroyTexture(manager->backgroundTexture);
    }
    
    manager->enemyCount = 0;
    manager->esCount = 0;
}
