#include <stdio.h>
#include <stdbool.h>
#include "menu.h"

int main(int argc, char *argv[]) {
    (void)argc;     // Pour éviter le warning "unused parameter"
    (void)argv;     // Pour éviter le warning "unused parameter"
    
    Menu menu;
    bool quit = false;
    
    // Initialiser le menu
    if (!initMenu(&menu)) {
        printf("Erreur lors de l'initialisation du menu\n");
        return 1;
    }
    
    // Charger les ressources
    if (!loadMenuResources(&menu)) {
        printf("Erreur lors du chargement des ressources\n");
        cleanupMenu(&menu);
        return 1;
    }
    
    // Boucle principale
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            handleMenuEvents(&menu, &event, &quit);
        }
        
        updateMenu(&menu);
        renderMenu(&menu);
        
        SDL_Delay(16); // ~60 FPS
    }
    
    // Nettoyer
    cleanupMenu(&menu);
    
    return 0;
}
