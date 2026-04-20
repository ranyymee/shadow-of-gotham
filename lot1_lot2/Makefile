CC     = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LIBS   = -lSDL2 -lSDL2_image -lSDL2_ttf -lm
TARGET = game
SRC    = main.c player.c back.c

all: $(TARGET)

$(TARGET): $(SRC) player.h back.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET) *.o
