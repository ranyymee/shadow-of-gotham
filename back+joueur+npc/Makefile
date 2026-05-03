CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm
TARGET  = game

# Sources du jeu principal (sans mback.c qui a son propre main)
SRCS    = Main.c player.c menu.c back.c npc.c minimap.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Cible optionnelle pour tester le background seul
mback: $(MBACK_OBJS)
	$(CC) $(MBACK_OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(MBACK_OBJS) $(TARGET) mback

.PHONY: all clean mback
