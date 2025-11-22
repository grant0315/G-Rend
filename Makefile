CFLAGS  = -Wall -Wextra -g -Iinclude
LDFLAGS = -L/ucrt64/lib -lSDL2 -lSDL2_ttf
# Note: cJSON is optional - if not installed, save/load will use fallback
# To enable full save/load: pacman -S mingw-w64-ucrt-x86_64-cjson
# Then uncomment the line below:
LDFLAGS += -lcjson

SRC     = src/main.c src/map.c src/game.c src/renderer.c src/data.c src/save.c src/enemy.c src/item.c
OBJ     = $(SRC:src/%.c=build/%.o)
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

