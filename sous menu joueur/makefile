# Compilateur et flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_mixer

# Nom de l'exécutable
TARGET = menu_game

# Fichiers sources
SOURCES = main.c button.c menu.c
OBJECTS = $(SOURCES:.c=.o)

# Règle par défaut
all: $(TARGET)

# Compilation de l'exécutable
$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compilation des fichiers .c en .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage
clean:
	rm -f $(OBJECTS) $(TARGET)

# Nettoyage complet
distclean: clean
	rm -f *~

# Installation des dépendances (pour Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev

# PHONY targets
.PHONY: all clean distclean install-deps
