#ifndef MAP_H
#define MAP_H

#include <stdio.h>

#define MAP_WIDTH  200
#define MAP_HEIGHT 100

// Tile types used by both the game and the image gen
typedef enum {
	TILE_WALL = 0,
	TILE_FLOOR = 1,
	TILE_STAIRS_UP = 2, 
	TILE_STAIRS_DOWN = 3
} Tile;

// Generate a new level (random walk + stairs)
void generate_level(Tile map[MAP_HEIGHT][MAP_WIDTH]);

// Optional: convert map to RGB image and write to PPM (for testing)
void map_to_image(Tile map[MAP_HEIGHT][MAP_WIDTH],
		unsigned char image[MAP_HEIGHT][MAP_WIDTH][3]);

void write_image(const char *filename,
		unsigned char image[MAP_HEIGHT][MAP_WIDTH][3]);

#endif // MAP_H
