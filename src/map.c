// src/map.c
#include "../include/map.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_ROOMS 30
#define MIN_ROOM_SIZE 5
#define MAX_ROOM_SIZE 12

typedef struct {
  int x, y, w, h;
} Room;

static void init_map(Tile map[MAP_HEIGHT][MAP_WIDTH]) {
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      map[y][x] = TILE_WALL;
    }
  }
}

// Create a rectangular room
static void create_room(Tile map[MAP_HEIGHT][MAP_WIDTH], Room *room) {
  for (int y = room->y; y < room->y + room->h; y++) {
    for (int x = room->x; x < room->x + room->w; x++) {
      if (y > 0 && y < MAP_HEIGHT - 1 && x > 0 && x < MAP_WIDTH - 1) {
        map[y][x] = TILE_FLOOR;
      }
    }
  }
}

// Check if two rooms intersect (with buffer)
static int rooms_intersect(Room *r1, Room *r2, int buffer) {
  return !(r1->x + r1->w + buffer <= r2->x ||
           r2->x + r2->w + buffer <= r1->x ||
           r1->y + r1->h + buffer <= r2->y ||
           r2->y + r2->h + buffer <= r1->y);
}

// Create horizontal corridor with 1-tile width
static void create_h_corridor(Tile map[MAP_HEIGHT][MAP_WIDTH], int x1, int x2, int y) {
  int start = x1 < x2 ? x1 : x2;
  int end = x1 < x2 ? x2 : x1;
  
  for (int x = start; x <= end; x++) {
    if (y > 0 && y < MAP_HEIGHT - 1 && x > 0 && x < MAP_WIDTH - 1) {
      map[y][x] = TILE_FLOOR;
    }
  }
}

// Create vertical corridor with 1-tile width
static void create_v_corridor(Tile map[MAP_HEIGHT][MAP_WIDTH], int y1, int y2, int x) {
  int start = y1 < y2 ? y1 : y2;
  int end = y1 < y2 ? y2 : y1;
  
  for (int y = start; y <= end; y++) {
    if (y > 0 && y < MAP_HEIGHT - 1 && x > 0 && x < MAP_WIDTH - 1) {
      map[y][x] = TILE_FLOOR;
    }
  }
}

// Connect two rooms with clean L-shaped corridor
static void connect_rooms(Tile map[MAP_HEIGHT][MAP_WIDTH], Room *r1, Room *r2) {
  // Get center points of each room
  int x1 = r1->x + r1->w / 2;
  int y1 = r1->y + r1->h / 2;
  int x2 = r2->x + r2->w / 2;
  int y2 = r2->y + r2->h / 2;
  
  // Always go horizontal first if rooms are more horizontally separated
  // This creates cleaner, more orthogonal corridors
  if (abs(x2 - x1) > abs(y2 - y1)) {
    create_h_corridor(map, x1, x2, y1);
    create_v_corridor(map, y1, y2, x2);
  } else {
    create_v_corridor(map, y1, y2, x1);
    create_h_corridor(map, x1, x2, y2);
  }
}

// Check if a position is a doorway (floor tile with walls on opposite sides)
static int is_doorway_position(Tile map[MAP_HEIGHT][MAP_WIDTH], int x, int y) {
  if (map[y][x] != TILE_FLOOR) return 0;
  if (x <= 1 || x >= MAP_WIDTH - 2 || y <= 1 || y >= MAP_HEIGHT - 2) return 0;
  
  // Check for horizontal doorway (walls to left/right, floor above/below or vice versa)
  int h_door = (map[y][x-1] == TILE_WALL && map[y][x+1] == TILE_WALL &&
                map[y-1][x] == TILE_FLOOR && map[y+1][x] == TILE_FLOOR);
  
  int v_door = (map[y-1][x] == TILE_WALL && map[y+1][x] == TILE_WALL &&
                map[y][x-1] == TILE_FLOOR && map[y][x+1] == TILE_FLOOR);
  
  return h_door || v_door;
}

// Place doors at room entrances
static void place_doors(Tile map[MAP_HEIGHT][MAP_WIDTH], Room *rooms, int room_count) {
  for (int i = 0; i < room_count; i++) {
    Room *room = &rooms[i];
    
    // Check perimeter of room for doorway positions
    for (int x = room->x - 1; x <= room->x + room->w; x++) {
      // Top edge
      if (is_doorway_position(map, x, room->y - 1)) {
        map[room->y - 1][x] = TILE_DOOR_CLOSED;
      }
      // Bottom edge
      if (is_doorway_position(map, x, room->y + room->h)) {
        map[room->y + room->h][x] = TILE_DOOR_CLOSED;
      }
    }
    
    for (int y = room->y - 1; y <= room->y + room->h; y++) {
      // Left edge
      if (is_doorway_position(map, room->x - 1, y)) {
        map[room->x - 1][y] = TILE_DOOR_CLOSED;
      }
      // Right edge
      if (is_doorway_position(map, room->x + room->w, y)) {
        map[room->x + room->w][y] = TILE_DOOR_CLOSED;
      }
    }
  }
}

static void place_stairs(Tile map[MAP_HEIGHT][MAP_WIDTH], Room *rooms, int room_count) {
  if (room_count < 2) return;
  
  // Place up stairs in first room
  Room *up_room = &rooms[0];
  int up_x = up_room->x + 1 + rand() % (up_room->w - 2);
  int up_y = up_room->y + 1 + rand() % (up_room->h - 2);
  map[up_y][up_x] = TILE_STAIRS_UP;
  
  // Place down stairs in last room
  Room *down_room = &rooms[room_count - 1];
  int down_x = down_room->x + 1 + rand() % (down_room->w - 2);
  int down_y = down_room->y + 1 + rand() % (down_room->h - 2);
  map[down_y][down_x] = TILE_STAIRS_DOWN;
}

// PUBLIC: generate a full level with rooms and corridors
void generate_level(Tile map[MAP_HEIGHT][MAP_WIDTH]) {
  static int seeded = 0;
  if (!seeded) {
    srand((unsigned)time(NULL));
    seeded = 1;
  }

  init_map(map);
  
  Room rooms[MAX_ROOMS];
  int room_count = 0;
  
  // Try to create rooms
  int attempts = 0;
  while (room_count < MAX_ROOMS && attempts < 500) {
    attempts++;
    
    // Generate random room
    int w = MIN_ROOM_SIZE + rand() % (MAX_ROOM_SIZE - MIN_ROOM_SIZE + 1);
    int h = MIN_ROOM_SIZE + rand() % (MAX_ROOM_SIZE - MIN_ROOM_SIZE + 1);
    int x = 2 + rand() % (MAP_WIDTH - w - 4);
    int y = 2 + rand() % (MAP_HEIGHT - h - 4);
    
    Room new_room = {x, y, w, h};
    
    // Check if it intersects with existing rooms
    int intersects = 0;
    for (int i = 0; i < room_count; i++) {
      if (rooms_intersect(&new_room, &rooms[i], 3)) {
        intersects = 1;
        break;
      }
    }
    
    if (!intersects) {
      create_room(map, &new_room);
      
      // Connect to previous room
      if (room_count > 0) {
        connect_rooms(map, &rooms[room_count - 1], &new_room);
      }
      
      rooms[room_count] = new_room;
      room_count++;
    }
  }
  
  // Place doors at room entrances
  place_doors(map, rooms, room_count);
  
  // Place stairs
  place_stairs(map, rooms, room_count);
}

// Optional helpers for image export
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
        r = g = b = 255; // white floor
        break;
      case TILE_STAIRS_UP:
        r = 0;
        g = 255;
        b = 0; // green up stairs
        break;
      case TILE_STAIRS_DOWN:
        r = 255;
        g = 0;
        b = 0; // red down stairs
        break;
      case TILE_DOOR_CLOSED:
        r = 139;
        g = 90;
        b = 43; // brown door
        break;
      case TILE_DOOR_OPEN:
        r = 90;
        g = 60;
        b = 30; // dark brown
        break;
      case TILE_WALL:
      default:
        r = g = b = 0; // black wall
        break;
      }

      image[y][x][0] = r;
      image[y][x][1] = g;
      image[y][x][2] = b;
    }
  }
}
