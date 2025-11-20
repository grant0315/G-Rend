CC      = gcc
CFLAGS  = -Wall -Wextra -g -Iinclude
LDFLAGS = -lncurses

SRC     = src/main.c src/map.c
OBJ     = build/main.o build/map.o
BIN     = build/roguelike

all: $(BIN)

build:
	mkdir -p build

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LDFLAGS)

clean:
	rm -rf build

.PHONY: all clean

