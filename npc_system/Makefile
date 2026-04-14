CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lm

TARGET = npc_test
SOURCES = main.c npc.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
