#ifndef NPC_H
#define NPC_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

// ========== ÉNUMÉRATIONS ==========

typedef enum {
    NPC_TYPE_ENEMY,
    NPC_TYPE_ES
} NPCType;

typedef enum {
    ENEMY_TYPE_JOKER,
    ENEMY_TYPE_HARLEY
} EnemyType;

typedef enum {
    HEALTH_ALIVE,
    HEALTH_INJURED,
    HEALTH_NEUTRALIZED
} HealthState;

typedef enum {
    TRAJECTORY_HORIZONTAL,
    TRAJECTORY_VERTICAL,
    TRAJECTORY_DIAGONAL
} TrajectoryType;

// ========== STRUCTURES ==========

typedef struct {
    int maxHealth;
    int currentHealth;
    int width;
    int height;
} HealthBar;

typedef struct {
    NPCType type;
    EnemyType enemyType;
    int id;
    
    // Position
    SDL_Rect rect;
    int x, y;
    int width, height;
    int speed;
    
    // Déplacement
    TrajectoryType trajectory;
    int directionX;
    int directionY;
    int patrolMinX, patrolMaxX;
    int patrolMinY, patrolMaxY;
    
    // Santé
    HealthState healthState;
    HealthBar healthBar;
    int damage;
    int scoreValue;
    
    // Texture
    SDL_Texture *texture;
    
    // IA
    bool isAggro;
    int aggroRange;
    int attackRange;
    Uint32 lastAttackTime;
    int attackCooldown;
    
    // État
    bool active;
    bool canAttack;
} NPC;

typedef struct {
    NPC enemies[50];
    NPC es[50];
    int enemyCount;
    int esCount;
    int currentLevel;
    
    SDL_Renderer *renderer;
    int *playerHealth;
    int *playerScore;
    
    // Protection du joueur (invincibilité)
    Uint32 playerInvincibilityEndTime;
    int invincibilityDuration;  // Durée en millisecondes
    
    // Texture pour le background
    SDL_Texture *backgroundTexture;
} NPCManager;

// ========== FONCTIONS D'INITIALISATION ==========

bool initNPCManager(NPCManager *manager, SDL_Renderer *renderer);
void initLevel1NPC(NPCManager *manager, SDL_Renderer *renderer);
void initLevel2NPC(NPCManager *manager, SDL_Renderer *renderer);
bool loadEnemyTexture(NPC *npc, SDL_Renderer *renderer, const char *path);
bool loadBackgroundTexture(NPCManager *manager, const char *path);

// ========== FONCTIONS DE DÉPLACEMENT ==========

void updateNPCs(NPCManager *manager, SDL_Rect *playerRect, int cameraX, int cameraY);
void updateRandomMovement(NPC *npc);
void updateAIPursuit(NPC *npc, SDL_Rect *playerRect);

// ========== FONCTIONS DE COLLISION ==========

bool checkCollisionBB(SDL_Rect *a, SDL_Rect *b);
bool handleCollisionAndReact(NPCManager *manager, NPC *npc, SDL_Rect *playerRect, int *playerHealth, int *playerScore);
void checkAllCollisions(NPCManager *manager, SDL_Rect *playerRect, int *playerHealth, int *playerScore);

// ========== FONCTIONS DE SANTÉ ==========

void initHealthBar(NPC *npc, int maxHealth, int width, int height);
void updateHealthState(NPC *npc);
void damageNPC(NPC *npc, int damage);
void renderHealthBar(NPCManager *manager, NPC *npc, int cameraX, int cameraY);
bool isPlayerInvincible(NPCManager *manager);
void setPlayerInvincible(NPCManager *manager);

// ========== FONCTIONS D'AFFICHAGE ==========

void renderNPC(SDL_Renderer *renderer, NPC *npc, int cameraX, int cameraY);
void renderAllNPCs(NPCManager *manager, int cameraX, int cameraY);
void renderBackground(NPCManager *manager, int cameraX, int cameraY);

// ========== FONCTIONS DE NETTOYAGE ==========

void cleanupNPCs(NPCManager *manager);

#endif
