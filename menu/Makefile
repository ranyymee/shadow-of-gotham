<<<<<<< HEAD
# Makefile — Shadow Of Gotham
CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iheader $(shell sdl2-config --cflags)
LIBS   = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm
TARGET = prog

# Liste des objets (AJOUT DE menu.o ET button.o)
OBJS = Main.o sourceMenu.o score.o enigme.o menu.o button.o sauvegarde.o options.o

=======
CC     = gcc
CFLAGS = -Wall -Wextra -g $(shell sdl2-config --cflags) -Iheader
LIBS   = $(shell sdl2-config --libs) -lSDL2_ttf -lSDL2_image -lSDL2_mixer -lm
TARGET = prog
SRCS = main/Main.c main/mainenigme.c src/sourceMenu.c src/sauvegarde.c src/options.c src/menu.c src/button.c src/score.c src/enigme.c src/puzzle.c
OBJS = $(notdir $(SRCS:.c=.o))
VPATH = src:main:header
>>>>>>> 75f7f12 (add bck folder)
all: $(TARGET)
$(TARGET): $(OBJS)
<<<<<<< HEAD
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

Main.o: main/Main.c
	$(CC) $(CFLAGS) -c -o $@ main/Main.c

sourceMenu.o: src/sourceMenu.c
	$(CC) $(CFLAGS) -c -o $@ src/sourceMenu.c

score.o: src/score.c
	$(CC) $(CFLAGS) -c -o $@ src/score.c

enigme.o: src/enigme.c
	$(CC) $(CFLAGS) -c -o $@ src/enigme.c

# NOUVELLES RÈGLES POUR LE MENU JOUEUR
menu.o: src/menu.c
	$(CC) $(CFLAGS) -c -o $@ src/menu.c

button.o: src/button.c
	$(CC) $(CFLAGS) -c -o $@ src/button.c
# RULE FOR YOUR SAUVEGARDE MENU
sauvegarde.o: src/sauvegarde.c
	$(CC) $(CFLAGS) -c -o $@ src/sauvegarde.c

# RÈGLE POUR LE MENU OPTIONS (projet collègue)
options.o: src/options.c
	$(CC) $(CFLAGS) -c -o $@ src/options.c

clean:
	rm -f $(OBJS) $(TARGET)

=======
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(OBJS) $(TARGET)
>>>>>>> 75f7f12 (add bck folder)
.PHONY: all clean
