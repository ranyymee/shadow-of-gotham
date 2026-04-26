CC      = gcc
TARGET  = batman_quiz
CFLAGS  = -Wall -Wextra -g $(shell sdl2-config --cflags) -I h/
LIBS    = $(shell sdl2-config --libs) -lSDL2_ttf -lSDL2_image -lSDL2_mixer -lm

all: $(TARGET)

$(TARGET): mainenigme.o enigme.o puzzle.o
	$(CC) mainenigme.o enigme.o puzzle.o -o $(TARGET) $(LIBS)

mainenigme.o: c/mainenigme.c
	$(CC) $(CFLAGS) -c c/mainenigme.c -o mainenigme.o

enigme.o: s/enigme.c
	$(CC) $(CFLAGS) -c s/enigme.c -o enigme.o

puzzle.o: s/puzzle.c
	$(CC) $(CFLAGS) -c s/puzzle.c -o puzzle.o

clean:
	rm -f mainenigme.o enigme.o puzzle.o $(TARGET)

.PHONY: all clean 
