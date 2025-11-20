// src/map.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "map.h"

static void init_map(Tile map[MAP_HEIGHT][MAP_WIDTH]) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            map[y][x] = TILE_WALL;
        }
    }
}

static void random_walk(int steps, Tile map[MAP_HEIGHT][MAP_WIDTH]) {
    int x = MAP_WIDTH  / 2;
    int y = MAP_HEIGHT / 2;

    map[y][x] = TILE_FLOOR;

    for (int i = 0; i < steps; i++) {
        int dir = rand() % 4;

        if (dir == 0 && y > 0)                 y--;
        if (dir == 1 && y < MAP_HEIGHT - 1)    y++;
        if (dir == 2 && x > 0)                 x--;
        if (dir == 3 && x < MAP_WIDTH  - 1)    x++;

        map[y][x] = TILE_FLOOR;
    }
}

static void place_stairs(Tile map[MAP_HEIGHT][MAP_WIDTH]) {
    int up_x, up_y;
    int down_x, down_y;

    // stairs up
    do {
        up_x = rand() % MAP_WIDTH;
        up_y = rand() % MAP_HEIGHT;
    } while (map[up_y][up_x] != TILE_FLOOR);

    map[up_y][up_x] = TILE_STAIRS_UP;

    // stairs down (different tile)
    do {
        down_x = rand() % MAP_WIDTH;
        down_y = rand() % MAP_HEIGHT;
    } while ((down_x == up_x && down_y == up_y) ||
             map[down_y][down_x] != TILE_FLOOR);

    map[down_y][down_x] = TILE_STAIRS_DOWN;
}

// PUBLIC: generate a full level that main.c can use
void generate_level(Tile map[MAP_HEIGHT][MAP_WIDTH]) {
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned)time(NULL));
        seeded = 1;
    }

    init_map(map);
    random_walk(10000, map);
    place_stairs(map);
}

// Optional helpers if you want to dump the map to an image from somewhere
void write_image(const char *filename,
                 unsigned char image[MAP_HEIGHT][MAP_WIDTH][3]) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Error opening file");
        return;
    }

    fprintf(fp, "P6\n%d %d\n255\n", MAP_WIDTH, MAP_HEIGHT);

    for (int y = 0; y < MAP_HEIGHT; y++) {
        fwrite(image[y], sizeof(unsigned char), MAP_WIDTH * 3, fp);
    }

    fclose(fp);
    printf("Image '%s' successfully created.\n", filename);
}

void map_to_image(Tile map[MAP_HEIGHT][MAP_WIDTH],
                  unsigned char image[MAP_HEIGHT][MAP_WIDTH][3]) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {

            unsigned char r = 0, g = 0, b = 0;

            switch (map[y][x]) {
                case TILE_FLOOR:
                    r = g = b = 255;             // white floor
                    break;
                case TILE_STAIRS_UP:
                    r = 0; g = 255; b = 0;       // green up stairs
                    break;
                case TILE_STAIRS_DOWN:
                    r = 255; g = 0; b = 0;       // red down stairs
                    break;
                case TILE_WALL:
                default:
                    r = g = b = 0;               // black wall
                    break;
            }

            image[y][x][0] = r;
            image[y][x][1] = g;
            image[y][x][2] = b;
        }
    }
}

