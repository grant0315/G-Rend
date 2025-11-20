#include <ncurses.h>

#define MAP_WIDTH 20
#define MAP_HEIGHT 10

// Simple static map
const char dungeon_map[MAP_HEIGHT][MAP_WIDTH + 1] = {
    "####################",  // 19 chars + '\0' (MAP_WIDTH should match)
    "#..................#",
    "#..######..........#",
    "#..#....#..........#",
    "#..#....#.....######",
    "#..#....#..........#",
    "#..######....#######",
    "#..................#",
    "#.........##########",
    "####################"
};

typedef struct {
	int x;
	int y;
	int hp;
} Player;

void init_ncurses(void) {
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);

	if (has_colors()) {
		start_color();
		init_pair(1, COLOR_WHITE, COLOR_BLACK); // Default
		init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Player
		init_pair(3, COLOR_CYAN, COLOR_BLACK); // Floor
		init_pair(4, COLOR_RED, COLOR_BLACK); // Walls
		init_pair(5, COLOR_BLACK, COLOR_WHITE); // Status Bar
	}
}

void shutdown_ncurses(void) {
	endwin();
}

void draw_map(void) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            char tile = dungeon_map[y][x];

            if (has_colors()) {
                if (tile == '#') {
                    attron(COLOR_PAIR(4));
                    mvaddch(y, x, tile);
                    attroff(COLOR_PAIR(4));
                } else if (tile == '.') {
                    attron(COLOR_PAIR(3));
                    mvaddch(y, x, tile);
                    attroff(COLOR_PAIR(3));
                } else {
                    mvaddch(y, x, tile);
                }
            } else {
                mvaddch(y, x, tile);
            }
        }
    }
}

void draw_status_bar(const Player *p) {
    	int screen_width = COLS;
    	int y = MAP_HEIGHT + 1;

    	// Draw a simple status line
    	if (has_colors()) {
        	attron(COLOR_PAIR(5));
        	mvhline(y, 0, ' ', screen_width); // clear line with background
    	} else {
        	mvhline(y, 0, ' ', screen_width);
    	}

    	char status[128];
    	snprintf(status, sizeof(status),
             "HP: %d   Move: Arrows/WASD   Quit: q", p->hp);

    	if (has_colors()) {
        	mvprintw(y, 1, "%s", status);
        	attroff(COLOR_PAIR(5));
    	} else {
        	mvprintw(y, 1, "%s", status);
	}
}

void draw_player(const Player *p) {
    if (has_colors()) {
        attron(COLOR_PAIR(2));
        mvaddch(p->y, p->x, '@');
        attroff(COLOR_PAIR(2));
    } else {
        mvaddch(p->y, p->x, '@');
    }
}

// Check if a tile is walkable (not a wall)
int is_walkable(int x, int y) {
	if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
		return 0;
	}
	char tile = dungeon_map[y][x];
	return (tile == '.'); // Only '.' is walkable
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
		default:
			break;
	}

	if (is_walkable(new_x, new_y)) {
		p->x = new_x;
		p->y = new_y;
	}
}

int main(void) {
	Player player = { .x = 2, .y = 2, .hp = 10 };

	init_ncurses();

	// Main game loop
	int ch;
	while (1) {
		clear(); // Clear screen

		draw_map(); // Draw the dungeon
		draw_player(&player); // Draw the player
		draw_status_bar(&player); // Draw status bar

		refresh(); // Show everything

		ch = getch(); // Wait for keypress

		if (ch == 'q' || ch == 'Q') {
			break; // QUIT
		}

		handle_input(ch, &player);
	}

	shutdown_ncurses();
	return 0;
}












