// src/main.c
#include <ncurses.h>
#include <stdio.h>
#include "map.h"

typedef struct {
    int x;
    int y;
    int hp;
} Player;

// Global map used by the game
static Tile g_map[MAP_HEIGHT][MAP_WIDTH];

static int cam_x = 0;
static int cam_y = 0;

static int zoom = 1;
#define ZOOM_MIN 1 // 1 char per tile
#define ZOOM_MAX 3 // FUTURE: could be higher in the future

void init_ncurses(void) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE,  COLOR_BLACK); // Default
        init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Player
        init_pair(3, COLOR_CYAN,   COLOR_BLACK); // Floor
        init_pair(4, COLOR_RED,    COLOR_WHITE); // Walls
        init_pair(5, COLOR_BLACK,  COLOR_WHITE); // Status Bar
        init_pair(6, COLOR_GREEN,  COLOR_BLACK); // Stairs up
        init_pair(7, COLOR_MAGENTA,COLOR_BLACK); // Stairs down
    }
}

// Center camera on player and clamp to map bounds
void update_camera(const Player *p, int view_w, int view_h) {
	cam_x = p->x - view_w / 2;
	cam_y = p->y - view_h / 2;

	if (cam_x < 0) cam_x = 0;
	if (cam_y < 0) cam_y = 0;

	if (cam_x > MAP_WIDTH - view_w)  cam_x = MAP_WIDTH - view_w;
	if (cam_y > MAP_HEIGHT - view_h) cam_y = MAP_HEIGHT - view_h;

	if (cam_x < 0) cam_x = 0;
	if (cam_y < 0) cam_y = 0;
}

void shutdown_ncurses(void) {
    endwin();
}

void draw_map(int tiles_w, int tiles_h) {
    for (int ty = 0; ty < tiles_h; ty++) {
        for (int tx = 0; tx < tiles_w; tx++) {
            int mx = cam_x + tx; // map x (tile)
            int my = cam_y + ty; // map y (tile)

            Tile tile = g_map[my][mx];
            char ch   = '#';
            int color_pair = 1; // default

            switch (tile) {
                case TILE_WALL:
                    ch = ' ';
                    color_pair = 4;
                    break;
                case TILE_FLOOR:
                    ch = ' ';
                    color_pair = 3;
                    break;
                case TILE_STAIRS_UP:
                    ch = '<';
                    color_pair = 6;
                    break;
                case TILE_STAIRS_DOWN:
                    ch = '>';
                    color_pair = 7;
                    break;
                default:
                    ch = '?';
                    break;
            }

            // top-left of this tile on screen
            int screen_y0 = ty * zoom;
            int screen_x0 = tx * zoom;

            // Fill a zoom x zoom block (zoomed tile)
            for (int oy = 0; oy < zoom && screen_y0 + oy < LINES - 1; oy++) {
                for (int ox = 0; ox < zoom && screen_x0 + ox < COLS; ox++) {
                    if (has_colors()) {
                        attron(COLOR_PAIR(color_pair));
                        mvaddch(screen_y0 + oy, screen_x0 + ox, ch);
                        attroff(COLOR_PAIR(color_pair));
                    } else {
                        mvaddch(screen_y0 + oy, screen_x0 + ox, ch);
                    }
                }
            }
        }
    }
}

void draw_status_bar(const Player *p, int view_w, int view_h) {
    int screen_width = COLS;
    int y = LINES - 1; // bottom line

    if (has_colors()) {
        attron(COLOR_PAIR(5));
        mvhline(y, 0, ' ', screen_width);
    } else {
        mvhline(y, 0, ' ', screen_width);
    }

    char status[128];
    snprintf(status, sizeof(status),
             "HP: %d   Move: Arrows/WASD   Zoom: +/-   Quit: q", p->hp);

    if (has_colors()) {
        mvprintw(y, 1, "%s", status);
        attroff(COLOR_PAIR(5));
    } else {
        mvprintw(y, 1, "%s", status);
    }
}


// Check if a tile is walkable (not a wall)
int is_walkable(int x, int y) {
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
        return 0;
    }

    Tile tile = g_map[y][x];
    // Floors and stairs are walkable
    return (tile == TILE_FLOOR ||
            tile == TILE_STAIRS_UP ||
            tile == TILE_STAIRS_DOWN);
}

void draw_player(const Player *p, int tiles_w, int tiles_h) {
    int tile_sx = p->x - cam_x;
    int tile_sy = p->y - cam_y;

    if (tile_sx < 0 || tile_sx >= tiles_w || tile_sy < 0 || tile_sy >= tiles_h) {
        return;
    }

    int screen_x0 = tile_sx * zoom;
    int screen_y0 = tile_sy * zoom;

    for (int oy = 0; oy < zoom && screen_y0 + oy < LINES - 1; oy++) {
        for (int ox = 0; ox < zoom && screen_x0 + ox < COLS; ox++) {
            if (has_colors()) {
                attron(COLOR_PAIR(2));
                mvaddch(screen_y0 + oy, screen_x0 + ox, '@');
                attroff(COLOR_PAIR(2));
            } else {
                mvaddch(screen_y0 + oy, screen_x0 + ox, '@');
            }
        }
    }
}


void handle_input(int ch, Player *p) {
    int new_x = p->x;
    int new_y = p->y;

    switch (ch) {
        case KEY_UP:
        case 'w':
        case 'W':
            new_y--;
            break;
        case KEY_DOWN:
        case 's':
        case 'S':
            new_y++;
            break;
        case KEY_LEFT:
        case 'a':
        case 'A':
            new_x--;
            break;
        case KEY_RIGHT:
        case 'd':
        case 'D':
            new_x++;
            break;
				case '+':
				case '=': // Shift + =
						if (zoom < ZOOM_MAX) zoom++; // Zoom IN: bigger tiles, fewer visiable
						break;
				case '-':
				case '_':
						if (zoom > ZOOM_MIN) zoom--; // Zoom OUT: smaller tiles, more visiable
						break;

				default:
            break;
    }

    if (is_walkable(new_x, new_y)) {
        p->x = new_x;
        p->y = new_y;
    }
}

int main(void) {
    // Generate the map using your map module
    generate_level(g_map);

    // Start the player roughly in the center
    Player player = { .x = MAP_WIDTH / 2, .y = MAP_HEIGHT / 2, .hp = 10 };

    init_ncurses();

    int ch;
		while(1) {
			clear();

			// How many tiles can we show, given the zoom?
			int tiles_w = COLS / zoom;
			int tiles_h = (LINES - 2) / zoom; // leave room for status bar

			if (tiles_w < 1) tiles_w = 1;
			if (tiles_h < 1) tiles_h = 1;
			if (tiles_w > MAP_WIDTH)  tiles_w = MAP_WIDTH;
			if (tiles_h > MAP_HEIGHT) tiles_h = MAP_HEIGHT;

			// Update camera in TILE space
			update_camera(&player, tiles_w, tiles_h);

			// Draw everything in TILE space
			draw_map(tiles_w, tiles_h);
			draw_player(&player, tiles_w, tiles_h);
			draw_status_bar(&player, tiles_w, tiles_h);

			refresh();

			ch = getch();
			if (ch == 'q' || ch == 'Q') {
					break;
			}

			handle_input(ch, &player);
		}

    shutdown_ncurses();
    return 0;
}
