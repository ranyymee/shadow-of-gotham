game: main.o back.o
	gcc main.o back.o -o game -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer
main.o: main.c back.h
	gcc -c main.c
back.o: back.c back.h
	gcc -c back.c
clean:
	rm -f *.o game*
