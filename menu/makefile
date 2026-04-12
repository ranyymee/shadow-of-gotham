# ─────────────────────────────────────────────────────────────────────────
#  Makefile — TACTICAL DECRYPTION ENGINE
# ─────────────────────────────────────────────────────────────────────────

CC      = gcc
TARGET  = tde

SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LIBS   := $(shell sdl2-config --libs)

CFLAGS  = -Wall -Wextra -O2 $(SDL_CFLAGS)
LIBS    = $(SDL_LIBS) -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm

SRCS    = main.c puzzle.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)
	@echo "  LINK  $(TARGET)  ✓"

%.o: %.c header.h
	@echo "  CC    $<"
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: all
	./$(TARGET)

.PHONY: all clean run
