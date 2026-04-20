CC     = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude $(shell sdl2-config --cflags)
LIBS   = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lm

game: main.o player.o menu.o
	$(CC) main.o player.o menu.o -o game $(LIBS)

main.o: src/main.c include/player.h include/game.h
	$(CC) $(CFLAGS) -c src/main.c -o main.o

player.o: src/player.c include/player.h
	$(CC) $(CFLAGS) -c src/player.c -o player.o

menu.o: src/menu.c include/menu.h include/player.h
	$(CC) $(CFLAGS) -c src/menu.c -o menu.o

clean:
	rm -f *.o game

.PHONY: clean
