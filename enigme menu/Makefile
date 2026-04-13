
CC      = gcc
TARGET  = batman_quiz
SRCS    = mainfarah.c enigme.c puzzle.c main.c
OBJS    = $(SRCS:.c=.o)
 
CFLAGS  = -Wall -Wextra -g $(shell sdl2-config --cflags)
LIBS    = $(shell sdl2-config --libs) -lSDL2_ttf -lSDL2_image -lSDL2_mixer -lm
 
all: $(TARGET)
 
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)
 
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
 
clean:
	rm -f $(OBJS) $(TARGET)
 
.PHONY: all clean
 
