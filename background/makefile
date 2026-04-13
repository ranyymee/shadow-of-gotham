CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -O2 \
          $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs) \
          -lSDL2_image -lSDL2_ttf -lm

TARGET  = game
SRCS    = main.c back.c
OBJS    = $(SRCS:.c=.o)

# ── Règles principales ─────────────────────────────────────────

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c back.h
	$(CC) $(CFLAGS) -c $< -o $@

# ── Utilitaires ────────────────────────────────────────────────

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(TARGET)

re: fclean all

run: all
	./$(TARGET)

.PHONY: all clean fclean re run
