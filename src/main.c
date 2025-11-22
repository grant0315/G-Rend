#include "../include/game.h"
#include "../include/renderer.h"
#include "../include/save.h"
#include "../include/map.h"
#include "../include/enemy.h"
#include "../include/item.h"

#include <stdlib.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

static int is_walkable(const GameState *game, int x, int y) {
  if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
    return 0;
  }
  Tile tile = game->map[y][x];
  return (tile == TILE_FLOOR || tile == TILE_STAIRS_UP || tile == TILE_STAIRS_DOWN);
}

static void ensure_player_on_floor(GameState *game) {
  if (is_walkable(game, game->player.x, game->player.y)) {
    return;
  }
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      if (is_walkable(game, x, y)) {
        game->player.x = x;
        game->player.y = y;
        return;
      }
    }
  }
}

static GameInput translate_key(SDL_Keycode sym, const GameState *game) {
  switch (sym) {
  case SDLK_w:
  case SDLK_UP:
    return GAME_INPUT_MOVE_UP;
  case SDLK_s:
  case SDLK_DOWN:
    return GAME_INPUT_MOVE_DOWN;
  case SDLK_LEFT:
    return GAME_INPUT_MOVE_LEFT;
  case SDLK_a:
    // 'a' only for movement when not in inventory menu
    if (game->menu_type != MENU_INVENTORY) {
      return GAME_INPUT_MOVE_LEFT;
    }
    return GAME_INPUT_NONE;
  case SDLK_RIGHT:
    return GAME_INPUT_MOVE_RIGHT;
  case SDLK_TAB:
    if (game->state == GAME_STATE_PLAYING) {
      return GAME_INPUT_INVENTORY;
    }
    return GAME_INPUT_NONE;
  case SDLK_RETURN:
    if (game->state == GAME_STATE_START_SCREEN) {
      return GAME_INPUT_START_GAME;
    }
    return GAME_INPUT_MENU_SELECT;
  case SDLK_SPACE:
    if (game->state == GAME_STATE_START_SCREEN) {
      return GAME_INPUT_START_GAME;
    }
    return GAME_INPUT_NONE;
  case SDLK_ESCAPE:
    if (game->state == GAME_STATE_PLAYING) {
      // Cancel targeting first, then menus, then quit
      if (game->targeting_mode != TARGETING_NONE) {
        return GAME_INPUT_CANCEL_TARGETING;
      }
      if (game->menu_type != MENU_NONE) {
        return GAME_INPUT_MENU_BACK;
      }
    }
    return GAME_INPUT_QUIT;
  case SDLK_q:
    return GAME_INPUT_QUIT;
  case SDLK_F5:
    return GAME_INPUT_SAVE;
  case SDLK_F9:
    return GAME_INPUT_LOAD;
  case SDLK_r:
    if (game->state == GAME_STATE_PLAYING) {
      return GAME_INPUT_REST;
    }
    return GAME_INPUT_NONE;
  case SDLK_f:
  case SDLK_o:
    if (game->state == GAME_STATE_PLAYING) {
      return GAME_INPUT_INTERACT;
    }
    return GAME_INPUT_NONE;
  case SDLK_e:
    if (game->state == GAME_STATE_PLAYING) {
      return GAME_INPUT_PICKUP_ALL;
    }
    return GAME_INPUT_NONE;
  case SDLK_i:
    if (game->state == GAME_STATE_PLAYING) {
      return GAME_INPUT_INVENTORY;
    }
    return GAME_INPUT_NONE;
  case SDLK_c:
  case SDLK_b:
    if (game->state == GAME_STATE_PLAYING) {
      return GAME_INPUT_SPELL_CODEX;
    }
    return GAME_INPUT_NONE;
  case SDLK_1:
    if (game->state == GAME_STATE_PLAYING) {
      // In pickup menu, pick item; otherwise cast spell
      if (game->menu_type == MENU_ITEM_PICKUP) {
        return GAME_INPUT_PICKUP_ITEM_1;
      }
      return GAME_INPUT_CAST_SPELL_1;
    }
    return GAME_INPUT_NONE;
  case SDLK_2:
    if (game->state == GAME_STATE_PLAYING) {
      if (game->menu_type == MENU_ITEM_PICKUP) {
        return GAME_INPUT_PICKUP_ITEM_2;
      }
      return GAME_INPUT_CAST_SPELL_2;
    }
    return GAME_INPUT_NONE;
  case SDLK_3:
    if (game->state == GAME_STATE_PLAYING) {
      if (game->menu_type == MENU_ITEM_PICKUP) {
        return GAME_INPUT_PICKUP_ITEM_3;
      }
      return GAME_INPUT_CAST_SPELL_3;
    }
    return GAME_INPUT_NONE;
  case SDLK_4:
    if (game->state == GAME_STATE_PLAYING) {
      if (game->menu_type == MENU_ITEM_PICKUP) {
        return GAME_INPUT_PICKUP_ITEM_4;
      }
      return GAME_INPUT_CAST_SPELL_4;
    }
    return GAME_INPUT_NONE;
  case SDLK_5:
    if (game->state == GAME_STATE_PLAYING) {
      if (game->menu_type == MENU_ITEM_PICKUP) {
        return GAME_INPUT_PICKUP_ITEM_5;
      }
      return GAME_INPUT_CAST_SPELL_5;
    }
    return GAME_INPUT_NONE;
  case SDLK_6:
    if (game->state == GAME_STATE_PLAYING) {
      return GAME_INPUT_CAST_SPELL_6;
    }
    return GAME_INPUT_NONE;
  case SDLK_7:
    if (game->state == GAME_STATE_PLAYING) {
      return GAME_INPUT_CAST_SPELL_7;
    }
    return GAME_INPUT_NONE;
  case SDLK_8:
    if (game->state == GAME_STATE_PLAYING) {
      return GAME_INPUT_CAST_SPELL_8;
    }
    return GAME_INPUT_NONE;
  case SDLK_9:
    if (game->state == GAME_STATE_PLAYING) {
      return GAME_INPUT_CAST_SPELL_9;
    }
    return GAME_INPUT_NONE;
  case SDLK_d:
    if (game->state == GAME_STATE_PLAYING && game->menu_type == MENU_INVENTORY) {
      return GAME_INPUT_DROP_ITEM;
    }
    return GAME_INPUT_NONE;
  // Note: Zoom is handled by mouse wheel only, not keyboard
  default:
    return GAME_INPUT_NONE;
  }
}

static void handle_mouse_click(GameState *game, int mouse_x, int mouse_y,
                                int window_w, int window_h, int tiles_w, int tiles_h) {
  if (game->state != GAME_STATE_PLAYING) {
    return;
  }

  int map_area_y = TITLE_BAR_HEIGHT;
  int map_area_h = window_h - TITLE_BAR_HEIGHT - STATUS_BAR_HEIGHT;
  int map_area_w = window_w - SIDE_PANEL_WIDTH;

  if (mouse_y < map_area_y || mouse_y >= map_area_y + map_area_h ||
      mouse_x < 0 || mouse_x >= map_area_w) {
    return;
  }

  int tile_size = game_get_tile_size(game);
  int tile_x = mouse_x / tile_size;
  int tile_y = (mouse_y - map_area_y) / tile_size;

  if (tile_x < 0 || tile_x >= tiles_w || tile_y < 0 || tile_y >= tiles_h) {
    return;
  }

  int map_x = game->cam_x + tile_x;
  int map_y = game->cam_y + tile_y;

  if (map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT) {
    return;
  }
  
  // Check if clicking on an enemy (for direct attack if adjacent)
  Enemy *enemy = enemy_get_at_position(game, map_x, map_y);
  if (enemy) {
    // Check if player is adjacent
    int dx = abs(game->player.x - map_x);
    int dy = abs(game->player.y - map_y);
    if ((dx == 1 && dy == 0) || (dx == 0 && dy == 1)) {
      // Adjacent, attack directly
      player_attack_enemy(game, enemy);
      return;
    }
    // Not adjacent, path to position next to enemy
  }

  game_handle_input(game, GAME_INPUT_MOVE_TO_POS, map_x, map_y,
                    SDL_GetTicks());
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  SDL_Log("=== G-Rend Roguelike Starting ===");
  
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return 1;
  }
  SDL_Log("SDL initialized successfully");

  Renderer renderer = {0};
  SDL_Log("Initializing renderer...");
  if (renderer_init(&renderer, WINDOW_WIDTH, WINDOW_HEIGHT) != 0) {
    SDL_Quit();
    return 1;
  }
  SDL_Log("Renderer initialized");

  GameState game;
  SDL_Log("Initializing game state...");
  game_init(&game);
  SDL_Log("Game state: %d (0=START, 1=PLAYING, 2=QUIT)", game.state);

  int running = 1;
  while (running) {
    int window_w = 0;
    int window_h = 0;
    SDL_GetWindowSize(renderer.window, &window_w, &window_h);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = 0;
        break;
      }
      
      // Mouse motion events are now handled in main loop for smoother tracking

      if (event.type == SDL_KEYDOWN) {
        SDL_Keycode key = event.key.keysym.sym;
        GameInput input = translate_key(key, &game);
        
        // Debug: Log any key that produces a non-NONE input
        if (input != GAME_INPUT_NONE) {
          SDL_Log("Key %d ('%c') -> GameInput %d", key, (char)key, input);
        }
        
        if (input == GAME_INPUT_LOAD && game.state == GAME_STATE_START_SCREEN) {
          if (load_game(&game, "save.json")) {
            if (game.state == GAME_STATE_PLAYING) {
              generate_level(game.map);
              ensure_player_on_floor(&game);
            }
          }
          continue;
        }
        if (input == GAME_INPUT_NONE) {
          continue;
        }
        if (game_handle_input(&game, input, 0, 0, SDL_GetTicks()) < 0) {
          running = 0;
          break;
        }
      }

      if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
          // Handle spell targeting confirmation
          if (game.state == GAME_STATE_PLAYING && game.targeting_mode == TARGETING_SPELL) {
            game_confirm_spell_cast(&game);
            continue;
          }
          
          if (game.menu_type != MENU_NONE) {
            if (game_menu_click(&game, event.button.x, event.button.y, window_w, window_h)) {
              continue;
            }
          }
          if (game.state == GAME_STATE_PLAYING) {
            int tiles_w = 0;
            int tiles_h = 0;
            game_compute_view(&game, window_w, window_h, &tiles_w, &tiles_h);
            handle_mouse_click(&game, event.button.x, event.button.y,
                               window_w, window_h, tiles_w, tiles_h);
          }
        }
      }

      // Zoom disabled for now due to event handling issues
      /*
      if (event.type == SDL_MOUSEWHEEL) {
        if (game.state == GAME_STATE_PLAYING) {
          SDL_MouseWheelEvent *wheel_event = (SDL_MouseWheelEvent*)&event;
          SDL_Log("MOUSEWHEEL event: y=%d, x=%d, timestamp=%u", 
                  wheel_event->y, wheel_event->x, wheel_event->timestamp);
          
          // Only process significant wheel movements (filter out noise)
          if (wheel_event->y > 0) {
            SDL_Log("Calling game_handle_input with ZOOM_IN");
            game_handle_input(&game, GAME_INPUT_ZOOM_IN, 0, 0, SDL_GetTicks());
          } else if (wheel_event->y < 0) {
            SDL_Log("Calling game_handle_input with ZOOM_OUT");
            game_handle_input(&game, GAME_INPUT_ZOOM_OUT, 0, 0, SDL_GetTicks());
          }
        }
      }
      */
    }

    if (game.state == GAME_STATE_START_SCREEN) {
      renderer_draw_start_screen(&renderer, window_w, window_h);
      SDL_RenderPresent(renderer.renderer);
    } else if (game.state == GAME_STATE_PLAYING) {
      // Update spell targeting continuously with current mouse position
      if (game.targeting_mode == TARGETING_SPELL) {
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        game_update_spell_targeting(&game, mouse_x, mouse_y, window_w, window_h);
      }
      
      game_update(&game, SDL_GetTicks());
      int tiles_w = 0;
      int tiles_h = 0;
      game_compute_view(&game, window_w, window_h, &tiles_w, &tiles_h);
      game_update_camera(&game, tiles_w, tiles_h);
      renderer_draw_game(&renderer, &game, window_w, window_h, tiles_w,
                         tiles_h);
    } else {
      running = 0;
    }

    SDL_Delay(16);
  }

  renderer_shutdown(&renderer);
  SDL_Quit();
  return 0;
}
