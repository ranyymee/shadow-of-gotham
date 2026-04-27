quiz: mainfarah.o enigme.o puzzle.o main.o
	gcc mainfarah.o enigme.o puzzle.o main.o -o quiz -lSDL2 -lSDL2_ttf -lSDL2_image -lSDL2_mixer -lm

mainfarah.o: mainfarah.c enigme.h header.h
	gcc -Wall -Wextra -g -c mainfarah.c

enigme.o: enigme.c enigme.h
	gcc -Wall -Wextra -g -c enigme.c

puzzle.o: puzzle.c header.h
	gcc -Wall -Wextra -g -c puzzle.c

main.o: main.c header.h
	gcc -Wall -Wextra -g -c main.c


