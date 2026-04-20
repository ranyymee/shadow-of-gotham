CC     = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude $(shell sdl2-config --cflags)
LIBS   = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lm

game: main.o back.o player.o
	$(CC) main.o back.o player.o -o game $(LIBS)

main.o: src/main.c include/back.h include/player.h include/game.h
	$(CC) $(CFLAGS) -c src/main.c -o main.o

back.o: src/back.c include/back.h
	$(CC) $(CFLAGS) -c src/back.c -o back.o

player.o: src/player.c include/player.h
	$(CC) $(CFLAGS) -c src/player.c -o player.o

clean:
	rm -f *.o game

.PHONY: clean
