#include "../include/game.h"
#include "../include/enemy.h"
#include "../include/item.h"
#include "../include/data.h"
#include "../include/save.h"
#include "../include/sdl_platform.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

// Forward declarations for door functions
static int can_interact_door(const GameState *game, int x, int y);
static void open_door(GameState *game, int x, int y);
static void process_enemy_turns(GameState *game);

static int is_walkable(const GameState *game, int x, int y) {
  if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
    return 0;
  }

  Tile tile = game->map[y][x];
  return (tile == TILE_FLOOR || tile == TILE_STAIRS_UP || tile == TILE_STAIRS_DOWN || tile == TILE_DOOR_OPEN);
}

// Check if a door can be opened/interacted with
static int can_interact_door(const GameState *game, int x, int y) {
  if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
    return 0;
  }
  return game->map[y][x] == TILE_DOOR_CLOSED;
}

// Open a door
static void open_door(GameState *game, int x, int y) {
  if (game->map[y][x] == TILE_DOOR_CLOSED) {
    game->map[y][x] = TILE_DOOR_OPEN;
    game_set_status_message(game, "You open the door.");
  }
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

void game_init(GameState *game) {
  memset(game, 0, sizeof(GameState));
  game->state = GAME_STATE_START_SCREEN;
  game->player.hp = PLAYER_MAX_HP;
  game->player.stamina = PLAYER_MAX_STAMINA;
  game->player.mp = PLAYER_MAX_MP;
  game->player.xp = 0;
  game->player.level = 1;
  game->player.gold = 0;
  game->player.base_attack = 5;
  game->player.base_defense = 2;
  game->player.base_dexterity = 10;
  game->player.base_speed = 0;
  game->player.equipped_weapon = -1;
  game->player.equipped_armor = -1;
  game->player.equipped_accessory = -1;
  game->player.equipped_boots = -1;
  game->current_level = 1;
  game->last_move_time = 0;
  game->path_length = 0;
  game->path_index = 0;
  game->menu_type = MENU_NONE;
  game->menu_selection = 0;
  
  // Initialize spell hotbar (empty by default)
  for (int i = 0; i < MAX_SPELL_HOTBAR; i++) {
    game->spell_hotbar[i] = -1;
  }
  
  // Initialize targeting mode
  game->targeting_mode = TARGETING_NONE;
  game->targeting_spell_idx = -1;
  game->targeting_x = 0;
  game->targeting_y = 0;
  game->targeting_valid = 0;
  
  game->is_resting = 0;
  game->rest_turns_remaining = 0;
  game->rest_stamina_per_turn = 10;  // Regain 10 stamina per rest turn
  game->vision_radius = DEFAULT_VISION_RADIUS;
  game->zoom_level = DEFAULT_TILE_SIZE;
  game->is_on_item_tile = 0;
  game->item_count_at_player = 0;
  memset(game->status_message, 0, sizeof(game->status_message));
  game->status_message_time = 0;
  
  // Initialize fog of war (all unexplored)
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      game->fog[y][x] = FOG_UNEXPLORED;
    }
  }
  
  // Initialize enemy and item systems
  enemy_init_types();
  item_init_types();
  
  if (!data_load_spells(game->spells, MAX_SPELLS, &game->spell_count)) {
    // Create diverse default spell set
    game->spell_count = 8;
    
    // Zero out all spells first
    memset(game->spells, 0, sizeof(game->spells));
    
    // 1. Fireball - Area damage
    strncpy(game->spells[0].name, "Fireball", MAX_NAME_LENGTH - 1);
    strncpy(game->spells[0].description, "Explosive fire damage in an area", MAX_DESC_LENGTH - 1);
    game->spells[0].mp_cost = 8;
    game->spells[0].level_required = 1;
    game->spells[0].target_type = SPELL_TARGET_AREA;
    game->spells[0].effect_type = SPELL_EFFECT_DAMAGE;
    game->spells[0].range = 8;
    game->spells[0].radius = 2;
    game->spells[0].power = 30;
    game->spells[0].color_r = 255; game->spells[0].color_g = 100; game->spells[0].color_b = 50;
    game->spells[0].glyph = '*';
    spell_init_pattern_circle(&game->spells[0], 2);
    
    // 2. Heal - Self cast
    strncpy(game->spells[1].name, "Heal", MAX_NAME_LENGTH - 1);
    strncpy(game->spells[1].description, "Restore your health", MAX_DESC_LENGTH - 1);
    game->spells[1].mp_cost = 5;
    game->spells[1].level_required = 1;
    game->spells[1].target_type = SPELL_TARGET_SELF;
    game->spells[1].effect_type = SPELL_EFFECT_HEAL;
    game->spells[1].range = 0;
    game->spells[1].radius = 0;
    game->spells[1].power = 40;
    game->spells[1].color_r = 100; game->spells[1].color_g = 255; game->spells[1].color_b = 100;
    game->spells[1].glyph = '+';
    
    // 3. Lightning Bolt - Line damage
    strncpy(game->spells[2].name, "Lightning Bolt", MAX_NAME_LENGTH - 1);
    strncpy(game->spells[2].description, "Strike enemies in a line", MAX_DESC_LENGTH - 1);
    game->spells[2].mp_cost = 6;
    game->spells[2].level_required = 2;
    game->spells[2].target_type = SPELL_TARGET_LINE;
    game->spells[2].effect_type = SPELL_EFFECT_DAMAGE;
    game->spells[2].range = 10;
    game->spells[2].radius = 0;
    game->spells[2].power = 25;
    game->spells[2].color_r = 200; game->spells[2].color_g = 200; game->spells[2].color_b = 255;
    game->spells[2].glyph = '|';
    spell_init_pattern_line(&game->spells[2]);
    
    // 4. Flame Cone - Cone AoE
    strncpy(game->spells[3].name, "Flame Cone", MAX_NAME_LENGTH - 1);
    strncpy(game->spells[3].description, "Cone of fire damage", MAX_DESC_LENGTH - 1);
    game->spells[3].mp_cost = 7;
    game->spells[3].level_required = 3;
    game->spells[3].target_type = SPELL_TARGET_CONE;
    game->spells[3].effect_type = SPELL_EFFECT_DAMAGE;
    game->spells[3].range = 5;
    game->spells[3].radius = 0;
    game->spells[3].power = 20;
    game->spells[3].color_r = 255; game->spells[3].color_g = 150; game->spells[3].color_b = 0;
    game->spells[3].glyph = '^';
    spell_init_pattern_cone(&game->spells[3]);
    
    // 5. Blink - Teleport
    strncpy(game->spells[4].name, "Blink", MAX_NAME_LENGTH - 1);
    strncpy(game->spells[4].description, "Teleport to target location", MAX_DESC_LENGTH - 1);
    game->spells[4].mp_cost = 4;
    game->spells[4].level_required = 2;
    game->spells[4].target_type = SPELL_TARGET_SINGLE;
    game->spells[4].effect_type = SPELL_EFFECT_TELEPORT;
    game->spells[4].range = 6;
    game->spells[4].radius = 0;
    game->spells[4].power = 0;
    game->spells[4].color_r = 150; game->spells[4].color_g = 100; game->spells[4].color_b = 255;
    game->spells[4].glyph = '@';
    
    // 6. Ice Nova - Large AoE around self
    strncpy(game->spells[5].name, "Ice Nova", MAX_NAME_LENGTH - 1);
    strncpy(game->spells[5].description, "Freeze enemies around you", MAX_DESC_LENGTH - 1);
    game->spells[5].mp_cost = 10;
    game->spells[5].level_required = 4;
    game->spells[5].target_type = SPELL_TARGET_SELF;
    game->spells[5].effect_type = SPELL_EFFECT_DAMAGE;
    game->spells[5].range = 0;
    game->spells[5].radius = 3;
    game->spells[5].power = 18;
    game->spells[5].color_r = 100; game->spells[5].color_g = 200; game->spells[5].color_b = 255;
    game->spells[5].glyph = 'o';
    spell_init_pattern_circle(&game->spells[5], 3);
    
    // 7. Magic Missile - Single target
    strncpy(game->spells[6].name, "Magic Missile", MAX_NAME_LENGTH - 1);
    strncpy(game->spells[6].description, "Unerring magical projectile", MAX_DESC_LENGTH - 1);
    game->spells[6].mp_cost = 3;
    game->spells[6].level_required = 1;
    game->spells[6].target_type = SPELL_TARGET_SINGLE;
    game->spells[6].effect_type = SPELL_EFFECT_DAMAGE;
    game->spells[6].range = 12;
    game->spells[6].radius = 0;
    game->spells[6].power = 15;
    game->spells[6].color_r = 255; game->spells[6].color_g = 100; game->spells[6].color_b = 255;
    game->spells[6].glyph = '.';
    
    // 8. Light - Vision buff
    strncpy(game->spells[7].name, "Light", MAX_NAME_LENGTH - 1);
    strncpy(game->spells[7].description, "Illuminate the darkness", MAX_DESC_LENGTH - 1);
    game->spells[7].mp_cost = 2;
    game->spells[7].level_required = 1;
    game->spells[7].target_type = SPELL_TARGET_SELF;
    game->spells[7].effect_type = SPELL_EFFECT_LIGHT;
    game->spells[7].range = 0;
    game->spells[7].radius = 0;
    game->spells[7].power = 0;
    game->spells[7].duration = 50;
    game->spells[7].color_r = 255; game->spells[7].color_g = 255; game->spells[7].color_b = 150;
    game->spells[7].glyph = '*';
  }
  
  // Auto-bind first 3 spells to hotbar slots 1-3
  for (int i = 0; i < 3 && i < game->spell_count; i++) {
    game->spell_hotbar[i] = i;
  }
  
  // Debug log to verify spell initialization
  SDL_Log("Initialized %d spells:", game->spell_count);
  for (int i = 0; i < game->spell_count && i < 3; i++) {
    SDL_Log("  Spell %d: %s, target_type=%d, range=%d", 
            i, game->spells[i].name, game->spells[i].target_type, game->spells[i].range);
  }
  
  if (!data_load_equipment_items(game->equipment, MAX_EQUIPMENT, &game->equipment_count)) {
    game->equipment_count = 4;
    strncpy(game->equipment[0].name, "Sword", MAX_NAME_LENGTH - 1);
    strncpy(game->equipment[0].description, "Basic weapon", MAX_DESC_LENGTH - 1);
    game->equipment[0].slot = EQUIP_SLOT_WEAPON;
    game->equipment[0].attack_bonus = 5;
    game->equipment[0].stamina_cost = 5;
    game->equipment[0].color_r = 150; game->equipment[0].color_g = 150; game->equipment[0].color_b = 150;
    
    strncpy(game->equipment[1].name, "Leather Armor", MAX_NAME_LENGTH - 1);
    strncpy(game->equipment[1].description, "Light protection", MAX_DESC_LENGTH - 1);
    game->equipment[1].slot = EQUIP_SLOT_ARMOR;
    game->equipment[1].defense_bonus = 3;
    game->equipment[1].stamina_cost = 0;
    game->equipment[1].color_r = 130; game->equipment[1].color_g = 100; game->equipment[1].color_b = 70;
    
    strncpy(game->equipment[2].name, "Boots", MAX_NAME_LENGTH - 1);
    strncpy(game->equipment[2].description, "Movement speed", MAX_DESC_LENGTH - 1);
    game->equipment[2].slot = EQUIP_SLOT_BOOTS;
    game->equipment[2].speed_bonus = 10;
    game->equipment[2].stamina_cost = 0;
    game->equipment[2].color_r = 100; game->equipment[2].color_g = 80; game->equipment[2].color_b = 60;
    
    strncpy(game->equipment[3].name, "Ring", MAX_NAME_LENGTH - 1);
    strncpy(game->equipment[3].description, "Magic boost", MAX_DESC_LENGTH - 1);
    game->equipment[3].slot = EQUIP_SLOT_ACCESSORY;
    game->equipment[3].stamina_cost = 0;
    game->equipment[3].color_r = 200; game->equipment[3].color_g = 180; game->equipment[3].color_b = 50;
  }
  
  if (!data_load_backpack(game->backpack, MAX_BACKPACK, &game->backpack_count)) {
    game->backpack_count = 5;
    strncpy(game->backpack[0].name, "Health Potion", MAX_NAME_LENGTH - 1);
    strncpy(game->backpack[0].description, "Restores HP", MAX_DESC_LENGTH - 1);
    game->backpack[0].quantity = 3;
    strncpy(game->backpack[1].name, "Mana Potion", MAX_NAME_LENGTH - 1);
    strncpy(game->backpack[1].description, "Restores MP", MAX_DESC_LENGTH - 1);
    game->backpack[1].quantity = 2;
    strncpy(game->backpack[2].name, "Rations", MAX_NAME_LENGTH - 1);
    strncpy(game->backpack[2].description, "Food for journey", MAX_DESC_LENGTH - 1);
    game->backpack[2].quantity = 5;
    strncpy(game->backpack[3].name, "Torch", MAX_NAME_LENGTH - 1);
    strncpy(game->backpack[3].description, "Light source", MAX_DESC_LENGTH - 1);
    game->backpack[3].quantity = 1;
    strncpy(game->backpack[4].name, "Rope", MAX_NAME_LENGTH - 1);
    strncpy(game->backpack[4].description, "Climbing tool", MAX_DESC_LENGTH - 1);
    game->backpack[4].quantity = 1;
  }
}

void game_start(GameState *game) {
  game->state = GAME_STATE_PLAYING;
  game->current_level = 1;
  generate_level(game->map);
  game->player.x = MAP_WIDTH / 2;
  game->player.y = MAP_HEIGHT / 2;
  ensure_player_on_floor(game);
  game->cam_x = 0;
  game->cam_y = 0;
  game->path_length = 0;
  game->path_index = 0;
  
  // Reset fog of war for new game
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      game->fog[y][x] = FOG_UNEXPLORED;
    }
  }
  
  // Spawn initial enemies
  game_spawn_enemies(game);
  
  // Update initial fog of war
  game_update_fog_of_war(game);
}

int game_get_tile_size(const GameState *game) {
  return game->zoom_level;
}

void game_compute_view(const GameState *game, int window_w, int window_h,
                       int *tiles_w, int *tiles_h) {
  int usable_h = window_h - TITLE_BAR_HEIGHT - STATUS_BAR_HEIGHT;
  // Side panel is always visible
  int usable_w = window_w - SIDE_PANEL_WIDTH;
  
  int tile_size = game_get_tile_size(game);
  int w = usable_w / tile_size;
  int h = usable_h / tile_size;

  if (w < 1)
    w = 1;
  if (h < 1)
    h = 1;
  if (w > MAP_WIDTH)
    w = MAP_WIDTH;
  if (h > MAP_HEIGHT)
    h = MAP_HEIGHT;

  if (tiles_w)
    *tiles_w = w;
  if (tiles_h)
    *tiles_h = h;
}

void game_update_camera(GameState *game, int tiles_w, int tiles_h) {
  game->cam_x = game->player.x - tiles_w / 2;
  game->cam_y = game->player.y - tiles_h / 2;

  if (game->cam_x < 0)
    game->cam_x = 0;
  if (game->cam_y < 0)
    game->cam_y = 0;

  if (game->cam_x > MAP_WIDTH - tiles_w)
    game->cam_x = MAP_WIDTH - tiles_w;
  if (game->cam_y > MAP_HEIGHT - tiles_h)
    game->cam_y = MAP_HEIGHT - tiles_h;

  if (game->cam_x < 0)
    game->cam_x = 0;
  if (game->cam_y < 0)
    game->cam_y = 0;
}

// Process all enemy turns (turn-based system)
static void process_enemy_turns(GameState *game) {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    Enemy *enemy = &game->enemies[i];
    if (!enemy->active) {
      continue;
    }

    const EnemyType *type = enemy_get_type(enemy->type_id);
    if (!type) {
      continue;
    }

    // Check if enemy can see player
    int dx = game->player.x - enemy->x;
    int dy = game->player.y - enemy->y;
    int dist = abs(dx) + abs(dy); // Manhattan distance

    // If within aggro range, alert the enemy
    if (dist <= type->aggro_range) {
      enemy->is_alerted = 1;
    } else if (dist > type->aggro_range * 2) {
      enemy->is_alerted = 0;
      continue;
    }

    // If not alerted, patrol randomly
    if (!enemy->is_alerted) {
      // Only move occasionally to simulate patrolling
      if (enemy->patrol_timer > 0) {
        enemy->patrol_timer--;
        continue;
      }
      
      // Choose new patrol point occasionally
      if (enemy->patrol_x == 0 && enemy->patrol_y == 0) {
        // Pick a random nearby point to patrol to
        int range = 5;
        enemy->patrol_x = enemy->x + (rand() % (range * 2 + 1)) - range;
        enemy->patrol_y = enemy->y + (rand() % (range * 2 + 1)) - range;
        enemy->patrol_timer = 10 + rand() % 20; // Wait 10-30 turns
      }
      
      // Move towards patrol point
      int dx = enemy->patrol_x - enemy->x;
      int dy = enemy->patrol_y - enemy->y;
      
      // Reached patrol point? Pick new one
      if (abs(dx) <= 1 && abs(dy) <= 1) {
        enemy->patrol_x = 0;
        enemy->patrol_y = 0;
        enemy->patrol_timer = 20 + rand() % 30; // Wait longer
        continue;
      }
      
      // Move one step towards patrol point
      int new_x = enemy->x;
      int new_y = enemy->y;
      
      if (abs(dx) > abs(dy) && dx != 0) {
        new_x += (dx > 0) ? 1 : -1;
      } else if (dy != 0) {
        new_y += (dy > 0) ? 1 : -1;
      }
      
      // Check if valid move
      if (new_x >= 0 && new_x < MAP_WIDTH && new_y >= 0 && new_y < MAP_HEIGHT) {
        Tile tile = game->map[new_y][new_x];
        if ((tile == TILE_FLOOR || tile == TILE_STAIRS_UP || tile == TILE_STAIRS_DOWN || tile == TILE_DOOR_OPEN) &&
            !enemy_get_at_position(game, new_x, new_y) &&
            !(game->player.x == new_x && game->player.y == new_y)) {
          enemy->x = new_x;
          enemy->y = new_y;
        } else {
          // Blocked, pick new patrol point
          enemy->patrol_x = 0;
          enemy->patrol_y = 0;
        }
      }
      
      continue;
    }

    // If adjacent to player, attack
    if (dist == 1) {
      enemy_attack_player(game, enemy);
      continue;
    }

    // Otherwise, move toward player (simple AI)
    int new_x = enemy->x;
    int new_y = enemy->y;

    if (abs(dx) > abs(dy)) {
      new_x += (dx > 0) ? 1 : -1;
    } else if (dy != 0) {
      new_y += (dy > 0) ? 1 : -1;
    }

    // Check if the new position is walkable and empty
    if (new_x >= 0 && new_x < MAP_WIDTH && new_y >= 0 && new_y < MAP_HEIGHT) {
      Tile tile = game->map[new_y][new_x];
      // Enemies can't walk through closed doors
      if ((tile == TILE_FLOOR || tile == TILE_STAIRS_UP || tile == TILE_STAIRS_DOWN || tile == TILE_DOOR_OPEN) &&
          !enemy_get_at_position(game, new_x, new_y) &&
          !(game->player.x == new_x && game->player.y == new_y)) {
        enemy->x = new_x;
        enemy->y = new_y;
      } else {
        // Try alternate direction if blocked
        if (abs(dx) > abs(dy)) {
          new_x = enemy->x;
          new_y = enemy->y + ((dy > 0) ? 1 : -1);
        } else {
          new_x = enemy->x + ((dx > 0) ? 1 : -1);
          new_y = enemy->y;
        }
        if (new_x >= 0 && new_x < MAP_WIDTH && new_y >= 0 && new_y < MAP_HEIGHT) {
          tile = game->map[new_y][new_x];
          if ((tile == TILE_FLOOR || tile == TILE_STAIRS_UP || tile == TILE_STAIRS_DOWN || tile == TILE_DOOR_OPEN) &&
              !enemy_get_at_position(game, new_x, new_y) &&
              !(game->player.x == new_x && game->player.y == new_y)) {
            enemy->x = new_x;
            enemy->y = new_y;
          }
        }
      }
    }
  }
}

// Process a player turn (move or attack)
// Returns 1 if turn was taken, 0 otherwise
static int try_move_player(GameState *game, int new_x, int new_y, uint32_t now) {
  // Rate limiting for UI responsiveness (not for turn logic)
  if (now - game->last_move_time < MOVE_INTERVAL_MS) {
    return 0;
  }
  
  // Check if there's an enemy at target position - attacking is a turn
  Enemy *enemy = enemy_get_at_position(game, new_x, new_y);
  if (enemy) {
    player_attack_enemy(game, enemy);
    game->last_move_time = now;
    return 1; // Turn taken
  }

  Tile target_tile = game->map[new_y][new_x];
  
  // Stairs - takes a turn
  if (target_tile == TILE_STAIRS_DOWN) {
    game_change_level(game, 1);
    game_set_status_message(game, "You descend to level %d.", game->current_level);
    game->last_move_time = now;
    return 1; // Turn taken
  }
  
  if (target_tile == TILE_STAIRS_UP && game->current_level > 1) {
    game_change_level(game, -1);
    game_set_status_message(game, "You ascend to level %d.", game->current_level);
    game->last_move_time = now;
    return 1; // Turn taken
  }

  // Check if trying to walk into a closed door
  if (target_tile == TILE_DOOR_CLOSED) {
    open_door(game, new_x, new_y);
    game->last_move_time = now;
    return 1; // Turn taken
  }
  
  // Movement - takes a turn, but NO stamina cost anymore
  if (is_walkable(game, new_x, new_y)) {
    game->player.x = new_x;
    game->player.y = new_y;
    game->last_move_time = now;
    
    // Check for items at new position
    game->is_on_item_tile = 0;
    game->item_count_at_player = 0;
    for (int i = 0; i < MAX_GROUND_ITEMS && game->item_count_at_player < 5; i++) {
      if (game->ground_items[i].active && 
          game->ground_items[i].x == new_x && 
          game->ground_items[i].y == new_y) {
        game->items_at_player[game->item_count_at_player++] = i;
        game->is_on_item_tile = 1;
      }
    }
    
    if (game->is_on_item_tile) {
      // Auto-open pickup modal when stepping on items
      game->menu_type = MENU_ITEM_PICKUP;
      game_set_status_message(game, "Items here! Press E to pick up all, or 1-%d for specific", 
                             game->item_count_at_player);
    }
    
    return 1; // Turn taken
  }
  
  return 0; // No turn taken
}

static int find_path(GameState *game, int start_x, int start_y, int end_x, int end_y) {
  if (start_x == end_x && start_y == end_y) {
    game->path_length = 0;
    return 0;
  }

  if (!is_walkable(game, end_x, end_y)) {
    return 0;
  }

  typedef struct {
    int x, y;
    int g, h, f;
    int parent;
  } Node;

  Node nodes[MAP_WIDTH * MAP_HEIGHT];
  int node_count = 0;
  int open_list[MAP_WIDTH * MAP_HEIGHT];
  int open_count = 0;
  int closed[MAP_WIDTH][MAP_HEIGHT];
  memset(closed, 0, sizeof(closed));

  int start_idx = node_count++;
  nodes[start_idx] = (Node){start_x, start_y, 0, 0, 0, -1};
  open_list[open_count++] = start_idx;

  static const int dx[] = {0, 1, 0, -1, 1, 1, -1, -1};
  static const int dy[] = {-1, 0, 1, 0, -1, 1, 1, -1};

  while (open_count > 0) {
    int best_idx = 0;
    for (int i = 1; i < open_count; i++) {
      if (nodes[open_list[i]].f < nodes[open_list[best_idx]].f) {
        best_idx = i;
      }
    }
    int current_idx = open_list[best_idx];
    Node *current = &nodes[current_idx];
    open_list[best_idx] = open_list[--open_count];

    if (current->x == end_x && current->y == end_y) {
      int path_len = 0;
      int idx = current_idx;
      while (idx != -1) {
        path_len++;
        idx = nodes[idx].parent;
      }
      if (path_len > PATH_MAX_LENGTH) {
        return 0;
      }
      game->path_length = path_len - 1;
      idx = current_idx;
      int path_idx = game->path_length - 1;
      while (idx != -1 && path_idx >= 0) {
        game->path[path_idx].x = nodes[idx].x;
        game->path[path_idx].y = nodes[idx].y;
        path_idx--;
        idx = nodes[idx].parent;
      }
      game->path_index = 0;
      return 1;
    }

    closed[current->y][current->x] = 1;

    for (int i = 0; i < 4; i++) {
      int nx = current->x + dx[i];
      int ny = current->y + dy[i];

      if (nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT) {
        continue;
      }
      if (closed[ny][nx]) {
        continue;
      }
      if (!is_walkable(game, nx, ny)) {
        continue;
      }

      int g = current->g + 1;
      int h = abs(nx - end_x) + abs(ny - end_y);
      int f = g + h;

      int found = 0;
      int existing_idx = -1;
      for (int j = 0; j < node_count; j++) {
        if (nodes[j].x == nx && nodes[j].y == ny) {
          existing_idx = j;
          found = 1;
          break;
        }
      }

      if (!found) {
        if (node_count >= MAP_WIDTH * MAP_HEIGHT) {
          continue;
        }
        existing_idx = node_count++;
        nodes[existing_idx] = (Node){nx, ny, g, h, f, current_idx};
        open_list[open_count++] = existing_idx;
      } else if (g < nodes[existing_idx].g) {
        nodes[existing_idx].g = g;
        nodes[existing_idx].f = f;
        nodes[existing_idx].parent = current_idx;
      }
    }
  }

  return 0;
}

void game_change_level(GameState *game, int direction) {
  game->current_level += direction;
  if (game->current_level < 1)
    game->current_level = 1;
  
  generate_level(game->map);
  game->player.x = MAP_WIDTH / 2;
  game->player.y = MAP_HEIGHT / 2;
  ensure_player_on_floor(game);
  game->cam_x = 0;
  game->cam_y = 0;
  
  game->player.stamina = PLAYER_MAX_STAMINA;
  game->path_length = 0;
  game->path_index = 0;
  
  // Reset fog of war for new level
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      game->fog[y][x] = FOG_UNEXPLORED;
    }
  }
  
  // Spawn new enemies for this level
  game_spawn_enemies(game);
  
  // Update fog of war
  game_update_fog_of_war(game);
}

int game_handle_input(GameState *game, GameInput input, int target_x, int target_y, uint32_t now) {
  SDL_Log("game_handle_input called with input=%d", input);
  
  if (game->state == GAME_STATE_START_SCREEN) {
    if (input == GAME_INPUT_START_GAME) {
      game_start(game);
      return 0;
    }
    if (input == GAME_INPUT_QUIT) {
      game->state = GAME_STATE_QUIT;
      return -1;
    }
    return 0;
  }

  if (game->state != GAME_STATE_PLAYING) {
    return 0;
  }

  // Cancel targeting
  if (input == GAME_INPUT_CANCEL_TARGETING) {
    if (game->targeting_mode != TARGETING_NONE) {
      game_cancel_spell_targeting(game);
      return 0;
    }
  }
  
  // Toggle inventory menu
  if (input == GAME_INPUT_INVENTORY) {
    if (game->menu_type == MENU_INVENTORY) {
      game->menu_type = MENU_NONE;
    } else {
      game->menu_type = MENU_INVENTORY;
      game->menu_selection = 0;
    }
    return 0;
  }
  
  // Toggle spell codex
  if (input == GAME_INPUT_SPELL_CODEX) {
    game_spell_codex_toggle(game);
    return 0;
  }
  
  // Handle spell codex menu
  if (game->menu_type == MENU_SPELL_CODEX) {
    switch (input) {
    case GAME_INPUT_MENU_BACK:
      game->menu_type = MENU_NONE;
      return 0;
    case GAME_INPUT_MOVE_UP:
      if (game->menu_selection > 0) {
        game->menu_selection--;
      }
      return 0;
    case GAME_INPUT_MOVE_DOWN:
      if (game->menu_selection < game->spell_count - 1) {
        game->menu_selection++;
      }
      return 0;
    case GAME_INPUT_MENU_SELECT:
      // Cast selected spell
      game->menu_type = MENU_NONE;
      game_start_spell_targeting(game, game->menu_selection);
      return 0;
    case GAME_INPUT_CAST_SPELL_1:
    case GAME_INPUT_CAST_SPELL_2:
    case GAME_INPUT_CAST_SPELL_3:
    case GAME_INPUT_CAST_SPELL_4:
    case GAME_INPUT_CAST_SPELL_5:
    case GAME_INPUT_CAST_SPELL_6:
    case GAME_INPUT_CAST_SPELL_7:
    case GAME_INPUT_CAST_SPELL_8:
    case GAME_INPUT_CAST_SPELL_9:
      {
        // Assign selected spell to hotbar slot
        int slot = input - GAME_INPUT_CAST_SPELL_1;
        game->spell_hotbar[slot] = game->menu_selection;
        game_set_status_message(game, "Bound %s to slot %d", 
                               game->spells[game->menu_selection].name, slot + 1);
      }
      return 0;
    case GAME_INPUT_QUIT:
      game->state = GAME_STATE_QUIT;
      return -1;
    default:
      return 0;
    }
  }
  
  // Handle inventory menu
  if (game->menu_type == MENU_INVENTORY) {
    switch (input) {
    case GAME_INPUT_MENU_BACK:
      game->menu_type = MENU_NONE;
      return 0;
    case GAME_INPUT_MOVE_UP:
      if (game->menu_selection > 0) {
        game->menu_selection--;
      }
      return 0;
    case GAME_INPUT_MOVE_DOWN:
      if (game->menu_selection < game->backpack_count - 1) {
        game->menu_selection++;
      }
      return 0;
    case GAME_INPUT_MENU_SELECT:
      // Use selected consumable
      if (item_use_consumable(game, game->menu_selection)) {
        process_enemy_turns(game);
      }
      return 0;
    case GAME_INPUT_DROP_ITEM:
      // Drop selected item
      if (game->menu_selection >= 0 && game->menu_selection < game->backpack_count) {
        game_set_status_message(game, "Dropped %s", game->backpack[game->menu_selection].name);
        // Remove from backpack
        for (int i = game->menu_selection; i < game->backpack_count - 1; i++) {
          game->backpack[i] = game->backpack[i + 1];
        }
        game->backpack_count--;
        if (game->menu_selection >= game->backpack_count && game->menu_selection > 0) {
          game->menu_selection--;
        }
      }
      return 0;
    case GAME_INPUT_QUIT:
      game->state = GAME_STATE_QUIT;
      return -1;
    default:
      return 0;
    }
  }
  
  // Handle item pickup modal
  if (game->menu_type == MENU_ITEM_PICKUP) {
    if (input == GAME_INPUT_MENU_BACK) {
      game->menu_type = MENU_NONE;
      return 0;
    }
  }

  if (input == GAME_INPUT_SAVE) {
    if (save_game(game, "save.json")) {
      snprintf(game->status_message, sizeof(game->status_message), "Game saved!");
      game->status_message_time = SDL_GetTicks();
    } else {
      snprintf(game->status_message, sizeof(game->status_message), "Save failed!");
      game->status_message_time = SDL_GetTicks();
    }
    return 0;
  }

  if (input == GAME_INPUT_LOAD) {
    if (load_game(game, "save.json")) {
      snprintf(game->status_message, sizeof(game->status_message), "Game loaded!");
      game->status_message_time = SDL_GetTicks();
      if (game->state == GAME_STATE_PLAYING) {
        generate_level(game->map);
        ensure_player_on_floor(game);
      }
    } else {
      snprintf(game->status_message, sizeof(game->status_message), "Load failed!");
      game->status_message_time = SDL_GetTicks();
    }
    return 0;
  }

  if (input == GAME_INPUT_REST) {
    // Start resting
    if (game->player.stamina >= PLAYER_MAX_STAMINA) {
      game_set_status_message(game, "You don't need to rest.");
      return 0;
    }
    
    // Calculate turns needed to fully rest
    int stamina_needed = PLAYER_MAX_STAMINA - game->player.stamina;
    int turns_needed = (stamina_needed + game->rest_stamina_per_turn - 1) / game->rest_stamina_per_turn;
    
    game->is_resting = 1;
    game->rest_turns_remaining = turns_needed;
    game_set_status_message(game, "Resting for %d turns...", turns_needed);
    
    // Immediately process one rest turn
    game->player.stamina += game->rest_stamina_per_turn;
    if (game->player.stamina > PLAYER_MAX_STAMINA) {
      game->player.stamina = PLAYER_MAX_STAMINA;
    }
    game->rest_turns_remaining--;
    game->last_move_time = now;
    
    // Enemies act during rest
    process_enemy_turns(game);
    
    if (game->rest_turns_remaining <= 0) {
      game->is_resting = 0;
      game_set_status_message(game, "Finished resting. Stamina restored!");
    }
    return 0;
  }

  if (input == GAME_INPUT_ZOOM_IN) {
    SDL_Log("ZOOM_IN triggered! zoom_level=%d", game->zoom_level);
    if (game->zoom_level < MAX_TILE_SIZE) {
      game->zoom_level += 4;
      if (game->zoom_level > MAX_TILE_SIZE) {
        game->zoom_level = MAX_TILE_SIZE;
      }
      game_set_status_message(game, "Zoom: %d%%", (game->zoom_level * 100) / DEFAULT_TILE_SIZE);
    }
    return 0;
  }

  if (input == GAME_INPUT_ZOOM_OUT) {
    SDL_Log("ZOOM_OUT triggered! zoom_level=%d", game->zoom_level);
    if (game->zoom_level > MIN_TILE_SIZE) {
      game->zoom_level -= 4;
      if (game->zoom_level < MIN_TILE_SIZE) {
        game->zoom_level = MIN_TILE_SIZE;
      }
      game_set_status_message(game, "Zoom: %d%%", (game->zoom_level * 100) / DEFAULT_TILE_SIZE);
    }
    return 0;
  }

  // Handle door interaction (F or O key)
  if (input == GAME_INPUT_INTERACT) {
    // Check adjacent tiles for closed doors
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};
    int door_found = 0;
    
    for (int i = 0; i < 4; i++) {
      int nx = game->player.x + dx[i];
      int ny = game->player.y + dy[i];
      
      if (can_interact_door(game, nx, ny)) {
        open_door(game, nx, ny);
        door_found = 1;
        // Opening a door takes a turn
        process_enemy_turns(game);
        break;
      }
    }
    
    if (!door_found) {
      game_set_status_message(game, "No door nearby to open.");
    }
    return 0;
  }
  
  // Handle item pickup (E key opens modal if items present)
  if (input == GAME_INPUT_PICKUP_ALL) {
    SDL_Log("PICKUP_ALL: is_on_item=%d, count=%d", game->is_on_item_tile, game->item_count_at_player);
    if (game->is_on_item_tile && game->item_count_at_player > 0) {
      int picked_up = 0;
      // Pick up all items at player position (iterate backwards to avoid index issues)
      for (int i = game->item_count_at_player - 1; i >= 0; i--) {
        int item_idx = game->items_at_player[i];
        if (item_idx >= 0 && item_idx < MAX_GROUND_ITEMS && game->ground_items[item_idx].active) {
          item_pickup(game, &game->ground_items[item_idx]);
          picked_up++;
        }
      }
      game->is_on_item_tile = 0;
      game->item_count_at_player = 0;
      game->menu_type = MENU_NONE;
      if (picked_up > 0) {
        game_set_status_message(game, "Picked up %d item(s)!", picked_up);
      }
      SDL_Log("Picked up %d items, returning early", picked_up);
    } else {
      game_set_status_message(game, "No items here to pick up.");
      SDL_Log("No items to pick up, returning early");
    }
    return 0;
  }

  // Handle specific item pickup (1-5) - only when pickup modal is open
  if (input >= GAME_INPUT_PICKUP_ITEM_1 && input <= GAME_INPUT_PICKUP_ITEM_5) {
    SDL_Log("PICKUP_ITEM_%d triggered", input - GAME_INPUT_PICKUP_ITEM_1 + 1);
    int slot = input - GAME_INPUT_PICKUP_ITEM_1;
    if (game->is_on_item_tile && slot < game->item_count_at_player) {
      int item_idx = game->items_at_player[slot];
      if (item_idx >= 0 && item_idx < MAX_GROUND_ITEMS && game->ground_items[item_idx].active) {
        item_pickup(game, &game->ground_items[item_idx]);
        // Update the items_at_player list
        for (int i = slot; i < game->item_count_at_player - 1; i++) {
          game->items_at_player[i] = game->items_at_player[i + 1];
        }
        game->item_count_at_player--;
        if (game->item_count_at_player == 0) {
          game->is_on_item_tile = 0;
          game->menu_type = MENU_NONE;
        }
        SDL_Log("Picked up item from slot %d, returning early", slot);
      }
    } else {
      game_set_status_message(game, "No item in that slot.");
      SDL_Log("No item in slot %d, returning early", slot);
    }
    return 0;
  }
  
  // Handle spell casting (1-9 keys) - starts targeting
  if (input >= GAME_INPUT_CAST_SPELL_1 && input <= GAME_INPUT_CAST_SPELL_9) {
    int slot = input - GAME_INPUT_CAST_SPELL_1;
    int spell_idx = game->spell_hotbar[slot];
    
    if (spell_idx >= 0 && spell_idx < game->spell_count) {
      game_start_spell_targeting(game, spell_idx);
    } else {
      game_set_status_message(game, "No spell in slot %d", slot + 1);
    }
    return 0;
  }

  int new_x = game->player.x;
  int new_y = game->player.y;

  switch (input) {
  case GAME_INPUT_MOVE_UP:
    game->path_length = 0;
    game->is_resting = 0;  // Cancel rest on movement
    new_y--;
    break;
  case GAME_INPUT_MOVE_DOWN:
    game->path_length = 0;
    game->is_resting = 0;
    new_y++;
    break;
  case GAME_INPUT_MOVE_LEFT:
    game->path_length = 0;
    game->is_resting = 0;
    new_x--;
    break;
  case GAME_INPUT_MOVE_RIGHT:
    game->path_length = 0;
    game->is_resting = 0;
    new_x++;
    break;
  case GAME_INPUT_MOVE_TO_POS:
    if (find_path(game, game->player.x, game->player.y, target_x, target_y)) {
      return 0;
    }
    game->path_length = 0;
    break;
  case GAME_INPUT_QUIT:
    game->state = GAME_STATE_QUIT;
    return -1;
  default:
    return 0;
  }

  // Try to move/attack and process enemy turns if player acted
  int turn_taken = try_move_player(game, new_x, new_y, now);
  if (turn_taken) {
    game_update_fog_of_war(game);  // Update vision after player moves
    process_enemy_turns(game);
  }
  return 0;
}

// Check if there's a clear line of sight from (x0,y0) to (x1,y1)
// Returns 0 if blocked by wall, 1 if clear
static int has_line_of_sight(const GameState *game, int x0, int y0, int x1, int y1) {
  // Bresenham's line algorithm to check for walls
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = x0 < x1 ? 1 : -1;
  int sy = y0 < y1 ? 1 : -1;
  int err = dx - dy;
  
  int x = x0;
  int y = y0;
  
  while (1) {
    // Check if current tile blocks vision (but allow destination)
    if (x != x1 || y != y1) {
      if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
        Tile tile = game->map[y][x];
        // Walls and closed doors block vision
        if (tile == TILE_WALL || tile == TILE_DOOR_CLOSED) {
          return 0;
        }
      }
    }
    
    // Reached destination
    if (x == x1 && y == y1) {
      break;
    }
    
    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x += sx;
    }
    if (e2 < dx) {
      err += dx;
      y += sy;
    }
  }
  
  return 1;
}

// Update fog of war based on player position with line-of-sight
void game_update_fog_of_war(GameState *game) {
  // First, mark all visible tiles as explored
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      if (game->fog[y][x] == FOG_VISIBLE) {
        game->fog[y][x] = FOG_EXPLORED;
      }
    }
  }
  
  // Calculate visible tiles using circular vision WITH line-of-sight
  int px = game->player.x;
  int py = game->player.y;
  int radius = game->vision_radius;
  
  for (int dy = -radius; dy <= radius; dy++) {
    for (int dx = -radius; dx <= radius; dx++) {
      int x = px + dx;
      int y = py + dy;
      
      // Check if tile is in map bounds
      if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
        continue;
      }
      
      // Use circular vision (Euclidean distance)
      int dist_sq = dx * dx + dy * dy;
      if (dist_sq <= radius * radius) {
        // Check line of sight from player to this tile
        if (has_line_of_sight(game, px, py, x, y)) {
          game->fog[y][x] = FOG_VISIBLE;
        }
      }
    }
  }
}

void game_update(GameState *game, uint32_t now) {
  if (game->state != GAME_STATE_PLAYING) {
    return;
  }

  // Process resting
  if (game->is_resting && game->rest_turns_remaining > 0) {
    uint32_t time_since_last_rest = now - game->last_move_time;
    if (time_since_last_rest >= MOVE_INTERVAL_MS) {
      game->player.stamina += game->rest_stamina_per_turn;
      if (game->player.stamina > PLAYER_MAX_STAMINA) {
        game->player.stamina = PLAYER_MAX_STAMINA;
      }
      game->rest_turns_remaining--;
      game->last_move_time = now;
      
      // Enemies act during rest
      process_enemy_turns(game);
      
      if (game->rest_turns_remaining <= 0) {
        game->is_resting = 0;
        game_set_status_message(game, "Finished resting. Stamina restored!");
      } else {
        game_set_status_message(game, "Resting... %d turns left", game->rest_turns_remaining);
      }
    }
    return;  // Don't process pathfinding while resting
  }

  // Update pathfinding movement (turn-based)
  if (game->path_index < game->path_length) {
    PathNode *next = &game->path[game->path_index];
    if (next->x == game->player.x && next->y == game->player.y) {
      game->path_index++;
    } else {
      int turn_taken = try_move_player(game, next->x, next->y, now);
      if (turn_taken) {
        game_update_fog_of_war(game);  // Update fog after each pathfinding step
        process_enemy_turns(game);
        if (next->x == game->player.x && next->y == game->player.y) {
          game->path_index++;
        }
      }
    }
  }
  
  // Note: Enemy turns are processed in try_move_player after the player acts
  // This ensures turn-based gameplay rather than real-time
}

int game_menu_open(GameState *game) {
  return game->menu_type != MENU_NONE;
}

int game_menu_close(GameState *game) {
  game->menu_type = MENU_NONE;
  game->menu_selection = 0;
  return 0;
}

int game_menu_selected(const GameState *game) {
  return game->menu_selection;
}

int game_menu_count(const GameState *game) {
  switch (game->menu_type) {
  case MENU_INVENTORY:
    return game->backpack_count;
  case MENU_ITEM_PICKUP:
    return game->item_count_at_player;
  default:
    return 0;
  }
}

const char *game_menu_item_name(const GameState *game, int index) {
  switch (game->menu_type) {
  case MENU_INVENTORY:
    if (index >= 0 && index < game->backpack_count) {
      return game->backpack[index].name;
    }
    return "";
  case MENU_ITEM_PICKUP:
    if (index >= 0 && index < game->item_count_at_player) {
      int item_idx = game->items_at_player[index];
      if (item_idx >= 0 && item_idx < MAX_GROUND_ITEMS && game->ground_items[item_idx].active) {
        const ItemType *type = item_get_type(game->ground_items[item_idx].type_id);
        if (type) {
          return type->name;
        }
      }
    }
    return "";
  default:
    return "";
  }
}

int game_menu_click(GameState *game, int mouse_x, int mouse_y, int window_w, int window_h) {
  if (game->menu_type == MENU_NONE) {
    return 0;
  }

  int panel_x = window_w - SIDE_PANEL_WIDTH;
  int panel_y = TITLE_BAR_HEIGHT;
  
  if (mouse_x < panel_x || mouse_x >= window_w) {
    return 0;
  }
  if (mouse_y < panel_y || mouse_y >= window_h - STATUS_BAR_HEIGHT) {
    return 0;
  }

  int padding = 12;
  int line_height = 22;
  int y = panel_y + padding;
  
  y += line_height + 8; // Skip title
  y += 12; // Skip divider
  
  int item_count = game_menu_count(game);
  for (int i = 0; i < item_count; i++) {
    // Match the highlight rect bounds: y - 2 to y + line_height
    if (mouse_y >= y - 2 && mouse_y < y + line_height) {
      game->menu_selection = i;
      game_handle_input(game, GAME_INPUT_MENU_SELECT, 0, 0, SDL_GetTicks());
      return 1;
    }
    y += line_height;
  }
  
  return 0;
}

void game_spawn_enemies(GameState *game) {
  // Clear existing enemies
  for (int i = 0; i < MAX_ENEMIES; i++) {
    game->enemies[i].active = 0;
  }
  
  int enemy_type_count = enemy_get_type_count();
  if (enemy_type_count == 0) {
    return;
  }
  
  // Spawn enemies based on level (increased density)
  int enemies_to_spawn = 10 + game->current_level * 5;
  if (enemies_to_spawn > MAX_ENEMIES / 2) {
    enemies_to_spawn = MAX_ENEMIES / 2;
  }
  
  int spawned = 0;
  int attempts = 0;
  while (spawned < enemies_to_spawn && attempts < 1000) {
    attempts++;
    
    int x = rand() % MAP_WIDTH;
    int y = rand() % MAP_HEIGHT;
    
    // Check if position is walkable and not too close to player
    if (game->map[y][x] != TILE_FLOOR) {
      continue;
    }
    
    int dx = x - game->player.x;
    int dy = y - game->player.y;
    int dist_sq = dx * dx + dy * dy;
    if (dist_sq < 100) { // At least 10 tiles away
      continue;
    }
    
    // Check if another enemy is already there
    if (enemy_get_at_position(game, x, y)) {
      continue;
    }
    
    // Choose enemy type based on level (harder enemies on deeper levels)
    int type_id;
    if (game->current_level <= 2) {
      type_id = rand() % (enemy_type_count < 2 ? enemy_type_count : 2); // Rats and goblins
    } else if (game->current_level <= 5) {
      type_id = rand() % (enemy_type_count < 3 ? enemy_type_count : 3); // Rats, goblins, orcs
    } else {
      type_id = rand() % enemy_type_count; // All types
    }
    
    enemy_spawn(game, type_id, x, y);
    spawned++;
  }
}

void game_set_status_message(GameState *game, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vsnprintf(game->status_message, sizeof(game->status_message), fmt, args);
  va_end(args);
  game->status_message_time = SDL_GetTicks();
}

int game_get_total_attack(const GameState *game) {
  int total = game->player.base_attack + game->player.level * 2;
  if (game->player.equipped_weapon >= 0 && game->player.equipped_weapon < game->equipment_count) {
    total += game->equipment[game->player.equipped_weapon].attack_bonus;
  }
  if (game->player.equipped_accessory >= 0 && game->player.equipped_accessory < game->equipment_count) {
    total += game->equipment[game->player.equipped_accessory].attack_bonus;
  }
  return total;
}

int game_get_total_defense(const GameState *game) {
  int total = game->player.base_defense;
  if (game->player.equipped_armor >= 0 && game->player.equipped_armor < game->equipment_count) {
    total += game->equipment[game->player.equipped_armor].defense_bonus;
  }
  if (game->player.equipped_boots >= 0 && game->player.equipped_boots < game->equipment_count) {
    total += game->equipment[game->player.equipped_boots].defense_bonus;
  }
  if (game->player.equipped_accessory >= 0 && game->player.equipped_accessory < game->equipment_count) {
    total += game->equipment[game->player.equipped_accessory].defense_bonus;
  }
  return total;
}

int game_get_total_dexterity(const GameState *game) {
  int total = game->player.base_dexterity + game->player.level;
  if (game->player.equipped_weapon >= 0 && game->player.equipped_weapon < game->equipment_count) {
    total += game->equipment[game->player.equipped_weapon].dexterity_bonus;
  }
  if (game->player.equipped_boots >= 0 && game->player.equipped_boots < game->equipment_count) {
    total += game->equipment[game->player.equipped_boots].dexterity_bonus;
  }
  if (game->player.equipped_accessory >= 0 && game->player.equipped_accessory < game->equipment_count) {
    total += game->equipment[game->player.equipped_accessory].dexterity_bonus;
  }
  return total;
}

int game_get_total_speed(const GameState *game) {
  int total = game->player.base_speed;
  if (game->player.equipped_weapon >= 0 && game->player.equipped_weapon < game->equipment_count) {
    total += game->equipment[game->player.equipped_weapon].speed_bonus;
  }
  if (game->player.equipped_boots >= 0 && game->player.equipped_boots < game->equipment_count) {
    total += game->equipment[game->player.equipped_boots].speed_bonus;
  }
  if (game->player.equipped_accessory >= 0 && game->player.equipped_accessory < game->equipment_count) {
    total += game->equipment[game->player.equipped_accessory].speed_bonus;
  }
  return total;
}

void game_equip_item(GameState *game, int equipment_index) {
  if (equipment_index < 0 || equipment_index >= game->equipment_count) {
    return;
  }
  
  EquipmentItem *item = &game->equipment[equipment_index];
  
  // Unequip any item in the same slot first
  switch (item->slot) {
    case EQUIP_SLOT_WEAPON:
      game->player.equipped_weapon = equipment_index;
      break;
    case EQUIP_SLOT_ARMOR:
      game->player.equipped_armor = equipment_index;
      break;
    case EQUIP_SLOT_ACCESSORY:
      game->player.equipped_accessory = equipment_index;
      break;
    case EQUIP_SLOT_BOOTS:
      game->player.equipped_boots = equipment_index;
      break;
  }
  
  game_set_status_message(game, "Equipped %s!", item->name);
}

void game_unequip_slot(GameState *game, EquipmentSlot slot) {
  const char *slot_name = "item";
  switch (slot) {
    case EQUIP_SLOT_WEAPON:
      game->player.equipped_weapon = -1;
      slot_name = "weapon";
      break;
    case EQUIP_SLOT_ARMOR:
      game->player.equipped_armor = -1;
      slot_name = "armor";
      break;
    case EQUIP_SLOT_ACCESSORY:
      game->player.equipped_accessory = -1;
      slot_name = "accessory";
      break;
    case EQUIP_SLOT_BOOTS:
      game->player.equipped_boots = -1;
      slot_name = "boots";
      break;
  }
  game_set_status_message(game, "Unequipped %s.", slot_name);
}

const EquipmentItem *game_get_equipped_item(const GameState *game, EquipmentSlot slot) {
  int index = -1;
  switch (slot) {
    case EQUIP_SLOT_WEAPON:
      index = game->player.equipped_weapon;
      break;
    case EQUIP_SLOT_ARMOR:
      index = game->player.equipped_armor;
      break;
    case EQUIP_SLOT_ACCESSORY:
      index = game->player.equipped_accessory;
      break;
    case EQUIP_SLOT_BOOTS:
      index = game->player.equipped_boots;
      break;
  }
  
  if (index >= 0 && index < game->equipment_count) {
    return &game->equipment[index];
  }
  return NULL;
}

// ============================================================================
// SPELL SYSTEM IMPLEMENTATION
// ============================================================================

void spell_init_pattern_circle(Spell *spell, int radius) {
  memset(spell->pattern, 0, sizeof(spell->pattern));
  spell->pattern_width = radius * 2 + 1;
  spell->pattern_height = radius * 2 + 1;
  
  int center = radius;
  for (int dy = -radius; dy <= radius; dy++) {
    for (int dx = -radius; dx <= radius; dx++) {
      // Use circular distance
      if (dx * dx + dy * dy <= radius * radius) {
        spell->pattern[center + dy][center + dx] = 1;
      }
    }
  }
}

void spell_init_pattern_cone(Spell *spell) {
  memset(spell->pattern, 0, sizeof(spell->pattern));
  // Cone pattern pointing up (can be rotated based on target)
  spell->pattern_width = 5;
  spell->pattern_height = 5;
  
  // Simple cone shape
  spell->pattern[4][2] = 1; // Base
  spell->pattern[3][1] = 1; spell->pattern[3][2] = 1; spell->pattern[3][3] = 1;
  spell->pattern[2][1] = 1; spell->pattern[2][2] = 1; spell->pattern[2][3] = 1;
  spell->pattern[1][0] = 1; spell->pattern[1][1] = 1; spell->pattern[1][2] = 1; spell->pattern[1][3] = 1; spell->pattern[1][4] = 1;
  spell->pattern[0][0] = 1; spell->pattern[0][1] = 1; spell->pattern[0][2] = 1; spell->pattern[0][3] = 1; spell->pattern[0][4] = 1;
}

void spell_init_pattern_line(Spell *spell) {
  memset(spell->pattern, 0, sizeof(spell->pattern));
  spell->pattern_width = 1;
  spell->pattern_height = spell->range;
}

int spell_can_target(const GameState *game, const Spell *spell, int target_x, int target_y) {
  // Self-cast spells are always valid at player position
  if (spell->target_type == SPELL_TARGET_SELF) {
    return 1;
  }
  
  // Check if target is on map
  if (target_x < 0 || target_x >= MAP_WIDTH || target_y < 0 || target_y >= MAP_HEIGHT) {
    return 0;
  }
  
  // Check if target is in range (use Euclidean distance for better feel)
  int dx = target_x - game->player.x;
  int dy = target_y - game->player.y;
  int dist_squared = dx * dx + dy * dy;
  int range_squared = spell->range * spell->range;
  
  if (spell->range > 0 && dist_squared > range_squared) {
    return 0;
  }
  
  // For teleport spells, must be walkable
  if (spell->effect_type == SPELL_EFFECT_TELEPORT) {
    Tile tile = game->map[target_y][target_x];
    if (tile != TILE_FLOOR && tile != TILE_STAIRS_UP && tile != TILE_STAIRS_DOWN) {
      return 0;
    }
    // Can't teleport onto enemies
    if (enemy_get_at_position((GameState*)game, target_x, target_y)) {
      return 0;
    }
  }
  
  return 1;
}

void spell_get_affected_tiles(const GameState *game, const Spell *spell, int target_x, int target_y,
                               int *tiles_x, int *tiles_y, int *tile_count, int max_tiles) {
  *tile_count = 0;
  
  if (spell->target_type == SPELL_TARGET_SELF) {
    // Self or area around self
    if (spell->radius > 0) {
      // AoE around player
      for (int dy = -spell->radius; dy <= spell->radius; dy++) {
        for (int dx = -spell->radius; dx <= spell->radius; dx++) {
          if (dx * dx + dy * dy <= spell->radius * spell->radius) {
            int tx = game->player.x + dx;
            int ty = game->player.y + dy;
            if (tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT && *tile_count < max_tiles) {
              tiles_x[*tile_count] = tx;
              tiles_y[*tile_count] = ty;
              (*tile_count)++;
            }
          }
        }
      }
    } else {
      // Just self
      tiles_x[0] = game->player.x;
      tiles_y[0] = game->player.y;
      *tile_count = 1;
    }
  } else if (spell->target_type == SPELL_TARGET_SINGLE) {
    // Single tile
    tiles_x[0] = target_x;
    tiles_y[0] = target_y;
    *tile_count = 1;
  } else if (spell->target_type == SPELL_TARGET_AREA) {
    // Circular AoE around target
    for (int dy = -spell->radius; dy <= spell->radius; dy++) {
      for (int dx = -spell->radius; dx <= spell->radius; dx++) {
        if (dx * dx + dy * dy <= spell->radius * spell->radius) {
          int tx = target_x + dx;
          int ty = target_y + dy;
          if (tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT && *tile_count < max_tiles) {
            tiles_x[*tile_count] = tx;
            tiles_y[*tile_count] = ty;
            (*tile_count)++;
          }
        }
      }
    }
  } else if (spell->target_type == SPELL_TARGET_LINE) {
    // Line from player to target
    int dx = target_x - game->player.x;
    int dy = target_y - game->player.y;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
    
    for (int i = 1; i <= steps && *tile_count < max_tiles; i++) {
      int tx = game->player.x + (dx * i) / steps;
      int ty = game->player.y + (dy * i) / steps;
      if (tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT) {
        tiles_x[*tile_count] = tx;
        tiles_y[*tile_count] = ty;
        (*tile_count)++;
      }
    }
  } else if (spell->target_type == SPELL_TARGET_CONE) {
    // Cone from player towards target
    int dx = target_x - game->player.x;
    int dy = target_y - game->player.y;
    
    // Determine direction
    int dir_x = dx == 0 ? 0 : (dx > 0 ? 1 : -1);
    int dir_y = dy == 0 ? 0 : (dy > 0 ? 1 : -1);
    
    // Simple cone approximation
    for (int dist = 1; dist <= spell->range; dist++) {
      int width = dist / 2 + 1;
      for (int w = -width; w <= width; w++) {
        int tx, ty;
        if (abs(dx) > abs(dy)) {
          // Horizontal cone
          tx = game->player.x + dir_x * dist;
          ty = game->player.y + w;
        } else {
          // Vertical cone
          tx = game->player.x + w;
          ty = game->player.y + dir_y * dist;
        }
        
        if (tx >= 0 && tx < MAP_WIDTH && ty >= 0 && ty < MAP_HEIGHT && *tile_count < max_tiles) {
          tiles_x[*tile_count] = tx;
          tiles_y[*tile_count] = ty;
          (*tile_count)++;
        }
      }
    }
  }
}

void spell_cast(GameState *game, int spell_idx, int target_x, int target_y) {
  if (spell_idx < 0 || spell_idx >= game->spell_count) {
    return;
  }
  
  const Spell *spell = &game->spells[spell_idx];
  
  // Get affected tiles
  int tiles_x[100], tiles_y[100], tile_count;
  spell_get_affected_tiles(game, spell, target_x, target_y, tiles_x, tiles_y, &tile_count, 100);
  
  // Apply spell effects
  switch (spell->effect_type) {
    case SPELL_EFFECT_DAMAGE:
      {
        int total_damage = 0;
        for (int i = 0; i < tile_count; i++) {
          Enemy *enemy = enemy_get_at_position(game, tiles_x[i], tiles_y[i]);
          if (enemy) {
            int damage = spell->power + game->player.level * 2;
            enemy->hp -= damage;
            total_damage += damage;
            if (enemy->hp <= 0) {
              enemy->active = 0;
              const EnemyType *type = enemy_get_type(enemy->type_id);
              if (type) {
                game->player.xp += type->xp_value;
                // Gold can be added later when EnemyType has gold_value field
              }
            }
          }
        }
        if (total_damage > 0) {
          game_set_status_message(game, "Cast %s! Dealt %d damage!", spell->name, total_damage);
        } else {
          game_set_status_message(game, "Cast %s!", spell->name);
        }
      }
      break;
      
    case SPELL_EFFECT_HEAL:
      {
        int heal_amount = spell->power + game->player.level * 5;
        game->player.hp += heal_amount;
        if (game->player.hp > PLAYER_MAX_HP) {
          game->player.hp = PLAYER_MAX_HP;
        }
        game_set_status_message(game, "Cast %s! Restored %d HP!", spell->name, heal_amount);
      }
      break;
      
    case SPELL_EFFECT_TELEPORT:
      game->player.x = target_x;
      game->player.y = target_y;
      game_set_status_message(game, "Cast %s! Teleported!", spell->name);
      game_update_fog_of_war(game);
      break;
      
    case SPELL_EFFECT_LIGHT:
      game->vision_radius = DEFAULT_VISION_RADIUS + 4;
      game_set_status_message(game, "Cast %s! Vision increased!", spell->name);
      game_update_fog_of_war(game);
      break;
      
    default:
      game_set_status_message(game, "Cast %s!", spell->name);
      break;
  }
}

void game_start_spell_targeting(GameState *game, int spell_idx) {
  if (spell_idx < 0 || spell_idx >= game->spell_count) {
    SDL_Log("ERROR: Invalid spell_idx %d", spell_idx);
    return;
  }
  
  const Spell *spell = &game->spells[spell_idx];
  
  SDL_Log("Starting targeting for spell: %s (target_type=%d, range=%d)", 
          spell->name, spell->target_type, spell->range);
  
  // Check MP cost
  if (game->player.mp < spell->mp_cost) {
    game_set_status_message(game, "Not enough MP! Need %d MP", spell->mp_cost);
    return;
  }
  
  // Enter targeting mode for ALL spells (even self-cast needs confirmation)
  game->targeting_mode = TARGETING_SPELL;
  game->targeting_spell_idx = spell_idx;
  
  // For self-cast spells, target defaults to player position
  if (spell->target_type == SPELL_TARGET_SELF) {
    SDL_Log("Self-cast spell detected");
    game->targeting_x = game->player.x;
    game->targeting_y = game->player.y;
    game->targeting_valid = 1; // Always valid for self
    game_set_status_message(game, "%s ready! Click to cast or ESC to cancel", spell->name);
  } else {
    SDL_Log("Targeted spell - starting at player pos, range=%d", spell->range);
    // For targeted spells, start at player position
    game->targeting_x = game->player.x;
    game->targeting_y = game->player.y;
    game->targeting_valid = spell_can_target(game, spell, game->player.x, game->player.y);
    game_set_status_message(game, "Targeting %s... Move mouse to aim, click to cast", spell->name);
  }
}

void game_update_spell_targeting(GameState *game, int mouse_x, int mouse_y, int window_w, int window_h) {
  if (game->targeting_mode != TARGETING_SPELL) {
    return;
  }
  
  const Spell *spell = &game->spells[game->targeting_spell_idx];
  
  // Self-cast spells don't track mouse - they're always centered on player
  if (spell->target_type == SPELL_TARGET_SELF) {
    game->targeting_x = game->player.x;
    game->targeting_y = game->player.y;
    game->targeting_valid = 1;
    return;
  }
  
  // Convert mouse position to map coordinates
  int map_area_y = TITLE_BAR_HEIGHT;
  int map_area_h = window_h - TITLE_BAR_HEIGHT - STATUS_BAR_HEIGHT;
  int map_area_w = window_w - SIDE_PANEL_WIDTH;
  
  if (mouse_y < map_area_y || mouse_y >= map_area_y + map_area_h ||
      mouse_x < 0 || mouse_x >= map_area_w) {
    // Mouse outside game area - keep last valid position
    return;
  }
  
  int tile_size = game_get_tile_size(game);
  int tile_x = mouse_x / tile_size;
  int tile_y = (mouse_y - map_area_y) / tile_size;
  
  int tiles_w = map_area_w / tile_size;
  int tiles_h = map_area_h / tile_size;
  
  if (tile_x < 0 || tile_x >= tiles_w || tile_y < 0 || tile_y >= tiles_h) {
    // Out of visible tiles - keep last valid position
    return;
  }
  
  int map_x = game->cam_x + tile_x;
  int map_y = game->cam_y + tile_y;
  
  if (map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT) {
    game->targeting_valid = 0;
    return;
  }
  
  // Update targeting position
  game->targeting_x = map_x;
  game->targeting_y = map_y;
  
  // Check if target is valid
  game->targeting_valid = spell_can_target(game, spell, map_x, map_y);
}

void game_cancel_spell_targeting(GameState *game) {
  game->targeting_mode = TARGETING_NONE;
  game->targeting_spell_idx = -1;
  game_set_status_message(game, "Cancelled targeting");
}

void game_confirm_spell_cast(GameState *game) {
  if (game->targeting_mode != TARGETING_SPELL) {
    return;
  }
  
  if (!game->targeting_valid) {
    game_set_status_message(game, "Invalid target!");
    return;
  }
  
  const Spell *spell = &game->spells[game->targeting_spell_idx];
  game->player.mp -= spell->mp_cost;
  spell_cast(game, game->targeting_spell_idx, game->targeting_x, game->targeting_y);
  game->targeting_mode = TARGETING_NONE;
  
  // Enemy turns after spell cast
  process_enemy_turns(game);
}

void game_spell_codex_toggle(GameState *game) {
  if (game->menu_type == MENU_SPELL_CODEX) {
    game->menu_type = MENU_NONE;
  } else {
    game->menu_type = MENU_SPELL_CODEX;
    game->menu_selection = 0;
  }
}
