# ─────────────────────────────────────────────────────────────────────────────
# Makefile — Shadow of Gotham (Batman/Catwoman + Énigmes)
# ─────────────────────────────────────────────────────────────────────────────

CC      = gcc
CFLAGS  = -Wall -Wextra -O2 $(shell sdl2-config --cflags)
LIBS    = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm

# ── Cibles ────────────────────────────────────────────────────────────────────

# Jeu principal avec énigmes par collision
TARGET_JEU   = jeu
SRC_JEU      = main_jeu.c enigme_trigger.c enigme.c player.c

# Version mainfarah (menu + quiz + puzzle)  — adaptez si nécessaire
TARGET_MAIN  = shadow_of_gotham
SRC_MAIN     = mainfarah.c enigme.c player.c puzzle.c mainpuzzle.c

# ── Règles ────────────────────────────────────────────────────────────────────

all: $(TARGET_JEU) $(TARGET_MAIN)

$(TARGET_JEU): $(SRC_JEU)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(TARGET_MAIN): $(SRC_MAIN)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

jeu: $(TARGET_JEU)

main: $(TARGET_MAIN)

clean:
	rm -f $(TARGET_JEU) $(TARGET_MAIN) *.o

.PHONY: all jeu main clean
