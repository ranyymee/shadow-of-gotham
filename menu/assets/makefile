# ─────────────────────────────────────────────────────────────────────────
# Makefile — TACTICAL DECRYPTION ENGINE
# ─────────────────────────────────────────────────────────────────────────
# Usage:
#   make          — build the 'tactical' binary
#   make clean    — remove object files and binary
#   make run      — build then run
# ─────────────────────────────────────────────────────────────────────────

TARGET  := tactical

CC      := gcc
CFLAGS  := -Wall -Wextra -O2 $(shell sdl2-config --cflags)
LDFLAGS := $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lm

SRCS    := main.c puzzle.c
OBJS    := $(SRCS:.c=.o)
HEADERS := header.h

# ── Default target ────────────────────────────────────────────────────────
.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Build complete → ./$(TARGET)"

# Compile each .c, depending on the shared header
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# ── Clean ─────────────────────────────────────────────────────────────────
clean:
	rm -f $(OBJS) $(TARGET)
	@echo "Cleaned."

# ── Run ───────────────────────────────────────────────────────────────────
run: all
	./$(TARGET)
