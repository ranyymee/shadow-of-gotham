game: main.o back.o npc.o
	gcc main.o back.o npc.o -o game -lSDL2 -lSDL2_ttf -lSDL2_image -lm

main.o: main.c back.h npc.h
	gcc -c main.c

back.o: back.c back.h
	gcc -c back.c

npc.o: npc.c npc.h
	gcc -c npc.c

clean:
	rm -f *.o game
