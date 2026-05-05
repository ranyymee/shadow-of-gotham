# Makefile — Shadow Of Gotham
CC     = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iheader $(shell sdl2-config --cflags)
LIBS   = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm
TARGET = prog

OBJS = Main.o mainenigme.o sourceMenu.o score.o enigme.o puzzle.o \
       menu.o button.o sauvegarde.o options.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

Main.o:        main/Main.c
	$(CC) $(CFLAGS) -c -o $@ $<

mainenigme.o:  main/mainenigme.c
	$(CC) $(CFLAGS) -c -o $@ $<

sourceMenu.o:  src/sourceMenu.c
	$(CC) $(CFLAGS) -c -o $@ $<

score.o:       src/score.c
	$(CC) $(CFLAGS) -c -o $@ $<

enigme.o:      src/enigme.c
	$(CC) $(CFLAGS) -c -o $@ $<

puzzle.o:      src/puzzle.c
	$(CC) $(CFLAGS) -c -o $@ $<

menu.o:        src/menu.c
	$(CC) $(CFLAGS) -c -o $@ $<

button.o:      src/button.c
	$(CC) $(CFLAGS) -c -o $@ $<

sauvegarde.o:  src/sauvegarde.c
	$(CC) $(CFLAGS) -c -o $@ $<

options.o:     src/options.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
