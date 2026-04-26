# Makefile — Shadow Of Gotham
# FIX: all .c files are in the same folder (flat layout)
# FIX: two separate targets:
#   make          -> builds the integrated game (Main.c)
#   make enigme   -> builds the standalone enigme+puzzle (mainfarah.c)

CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 $(shell sdl2-config --cflags)
LIBS   = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm

# ── Integrated game ──────────────────────────────────────────────────────
TARGET = prog

OBJS = Main.o sourceMenu.o score.o enigme.o menu.o button.o sauvegarde.o options.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

Main.o: Main.c
	$(CC) $(CFLAGS) -c -o $@ Main.c

sourceMenu.o: sourceMenu.c
	$(CC) $(CFLAGS) -c -o $@ sourceMenu.c

score.o: score.c
	$(CC) $(CFLAGS) -c -o $@ score.c

enigme.o: enigme.c
	$(CC) $(CFLAGS) -c -o $@ enigme.c

menu.o: menu.c
	$(CC) $(CFLAGS) -c -o $@ menu.c

button.o: button.c
	$(CC) $(CFLAGS) -c -o $@ button.c

sauvegarde.o: sauvegarde.c
	$(CC) $(CFLAGS) -c -o $@ sauvegarde.c

options.o: options.c
	$(CC) $(CFLAGS) -c -o $@ options.c

# ── Standalone enigme + puzzle ────────────────────────────────────────────
ENIGME_TARGET = enigme_standalone

ENIGME_OBJS = mainfarah.o mainp.o enigme_s.o puzzle.o

enigme: $(ENIGME_TARGET)

$(ENIGME_TARGET): $(ENIGME_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

mainfarah.o: mainfarah.c
	$(CC) $(CFLAGS) -c -o $@ mainfarah.c

mainp.o: mainp.c
	$(CC) $(CFLAGS) -c -o $@ mainp.c

enigme_s.o: enigme.c
	$(CC) $(CFLAGS) -c -o $@ enigme.c

puzzle.o: puzzle.c
	$(CC) $(CFLAGS) -c -o $@ puzzle.c

# ── Clean ─────────────────────────────────────────────────────────────────
clean:
	rm -f $(OBJS) $(ENIGME_OBJS) $(TARGET) $(ENIGME_TARGET)

.PHONY: all enigme clean
