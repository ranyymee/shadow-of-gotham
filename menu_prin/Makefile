CC     = gcc
CFLAGS = -Wall -Wextra -g $(shell sdl2-config --cflags) -Iheader
LIBS   = $(shell sdl2-config --libs) -lSDL2_ttf -lSDL2_image -lSDL2_mixer -lm
TARGET = prog
SRCS = main/Main.c main/mainenigme.c src/sourceMenu.c src/sauvegarde.c src/options.c src/menu.c src/button.c src/score.c src/enigme.c src/puzzle.c
OBJS = $(notdir $(SRCS:.c=.o))
VPATH = src:main:header
all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(OBJS) $(TARGET)
.PHONY: all clean
