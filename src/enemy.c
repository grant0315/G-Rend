#include "../include/enemy.h"
#include "../include/game.h"
#include "../include/item.h"
#include "../include/map.h"
#include "../include/data.h"
#include "../include/sdl_platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Global enemy types (loaded once at startup)
static EnemyType g_enemy_types[MAX_ENEMY_TYPES];
static int g_enemy_type_count = 0;

// Helper: Check if position is walkable for enemies
static int is_walkable_for_enemy(const GameState *game, int x, int y) {
  if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
    return 0;
  }
  Tile tile = game->map[y][x];
  if (tile != TILE_FLOOR && tile != TILE_STAIRS_UP && tile != TILE_STAIRS_DOWN) {
    return 0;
  }
  
  // Check if another enemy is there
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (game->enemies[i].active && game->enemies[i].x == x && game->enemies[i].y == y) {
      return 0;
    }
  }
  
  // Check if player is there
  if (game->player.x == x && game->player.y == y) {
    return 0;
  }
  
  return 1;
}

// Helper: Calculate distance between two points
static int distance(int x1, int y1, int x2, int y2) {
  int dx = x2 - x1;
  int dy = y2 - y1;
  return (int)sqrt(dx * dx + dy * dy);
}

// move_toward_player removed - enemy AI is now handled in game.c's process_enemy_turns

void enemy_init_types(void) {
  g_enemy_type_count = 0;
  
  // Load from JSON, or use defaults
  if (!enemy_load_types_from_json(g_enemy_types, MAX_ENEMY_TYPES)) {
    // Hardcoded fallback enemy types
    g_enemy_type_count = 4;
    
    // Rat
    strncpy(g_enemy_types[0].name, "Rat", MAX_ENEMY_NAME_LENGTH - 1);
    strncpy(g_enemy_types[0].description, "A small rodent", MAX_ENEMY_DESC_LENGTH - 1);
    g_enemy_types[0].glyph = 'r';
    g_enemy_types[0].max_hp = 5;
    g_enemy_types[0].attack_power = 2;
    g_enemy_types[0].defense = 0;
    g_enemy_types[0].xp_value = 5;
    g_enemy_types[0].aggro_range = 3;
    g_enemy_types[0].color_r = 150;
    g_enemy_types[0].color_g = 100;
    g_enemy_types[0].color_b = 50;
    
    // Goblin
    strncpy(g_enemy_types[1].name, "Goblin", MAX_ENEMY_NAME_LENGTH - 1);
    strncpy(g_enemy_types[1].description, "A small green humanoid", MAX_ENEMY_DESC_LENGTH - 1);
    g_enemy_types[1].glyph = 'g';
    g_enemy_types[1].max_hp = 15;
    g_enemy_types[1].attack_power = 5;
    g_enemy_types[1].defense = 1;
    g_enemy_types[1].xp_value = 15;
    g_enemy_types[1].aggro_range = 5;
    g_enemy_types[1].color_r = 50;
    g_enemy_types[1].color_g = 200;
    g_enemy_types[1].color_b = 50;
    
    // Orc
    strncpy(g_enemy_types[2].name, "Orc", MAX_ENEMY_NAME_LENGTH - 1);
    strncpy(g_enemy_types[2].description, "A fierce warrior", MAX_ENEMY_DESC_LENGTH - 1);
    g_enemy_types[2].glyph = 'o';
    g_enemy_types[2].max_hp = 30;
    g_enemy_types[2].attack_power = 8;
    g_enemy_types[2].defense = 3;
    g_enemy_types[2].xp_value = 30;
    g_enemy_types[2].aggro_range = 6;
    g_enemy_types[2].color_r = 100;
    g_enemy_types[2].color_g = 150;
    g_enemy_types[2].color_b = 100;
    
    // Dragon
    strncpy(g_enemy_types[3].name, "Dragon", MAX_ENEMY_NAME_LENGTH - 1);
    strncpy(g_enemy_types[3].description, "A mighty beast", MAX_ENEMY_DESC_LENGTH - 1);
    g_enemy_types[3].glyph = 'D';
    g_enemy_types[3].max_hp = 100;
    g_enemy_types[3].attack_power = 20;
    g_enemy_types[3].defense = 10;
    g_enemy_types[3].xp_value = 200;
    g_enemy_types[3].aggro_range = 10;
    g_enemy_types[3].color_r = 255;
    g_enemy_types[3].color_g = 50;
    g_enemy_types[3].color_b = 50;
  }
}

int enemy_get_type_count(void) {
  return g_enemy_type_count;
}

const EnemyType *enemy_get_type(int type_id) {
  if (type_id < 0 || type_id >= g_enemy_type_count) {
    return NULL;
  }
  return &g_enemy_types[type_id];
}

void enemy_spawn(GameState *game, int type_id, int x, int y) {
  if (type_id < 0 || type_id >= g_enemy_type_count) {
    return;
  }

  const EnemyType *type = enemy_get_type(type_id);
  if (!type) {
    return;
  }

  // Find empty slot
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (!game->enemies[i].active) {
      game->enemies[i].active = 1;
      game->enemies[i].type_id = type_id;
      game->enemies[i].x = x;
      game->enemies[i].y = y;
  game->enemies[i].hp = type->max_hp;
  game->enemies[i].last_move_time = 0;
  game->enemies[i].is_alerted = 0;
  game->enemies[i].patrol_x = 0;
  game->enemies[i].patrol_y = 0;
  game->enemies[i].patrol_timer = rand() % 20; // Start with random delay
      return;
    }
  }
}

// Legacy function - no longer used with turn-based system
// Enemy turns are now processed in game.c after player takes a turn
void enemy_update_all(GameState *game, uint32_t now) {
  (void)game;
  (void)now;
  // Turn-based: enemies move in process_enemy_turns() in game.c
}

void enemy_remove(Enemy *enemy) {
  enemy->active = 0;
}

Enemy *enemy_get_at_position(GameState *game, int x, int y) {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    if (game->enemies[i].active && game->enemies[i].x == x && game->enemies[i].y == y) {
      return &game->enemies[i];
    }
  }
  return NULL;
}

void enemy_take_damage(GameState *game, Enemy *enemy, int damage) {
  if (!enemy->active) {
    return;
  }

  const EnemyType *type = enemy_get_type(enemy->type_id);
  if (!type) {
    return;
  }

  // Apply defense
  int actual_damage = damage - type->defense;
  if (actual_damage < 1) {
    actual_damage = 1; // Always deal at least 1 damage
  }

  enemy->hp -= actual_damage;
  
  char msg[256];
  snprintf(msg, sizeof(msg), "You hit %s for %d damage!", type->name, actual_damage);
  game_set_status_message(game, "%s", msg);

  if (enemy->hp <= 0) {
    // Enemy dies - drop loot!
    int enemy_x = enemy->x;
    int enemy_y = enemy->y;
    int enemy_type_id = enemy->type_id;
    
    snprintf(msg, sizeof(msg), "You killed %s! (+%d XP)", type->name, type->xp_value);
    game_set_status_message(game, "%s", msg);
    
    game->player.xp += type->xp_value;
    
    // Drop loot at enemy position
    item_drop_loot(game, enemy_type_id, enemy_x, enemy_y);
    
    // Level up check
    int required_xp = game->player.level * XP_PER_LEVEL;
    if (game->player.xp >= required_xp) {
      game->player.level++;
      game->player.xp -= required_xp;
      game->player.hp = PLAYER_MAX_HP; // Restore HP on level up
      game->player.mp = PLAYER_MAX_MP;
      game->player.stamina = PLAYER_MAX_STAMINA;
      snprintf(msg, sizeof(msg), "Level up! You are now level %d!", game->player.level);
      game_set_status_message(game, "%s", msg);
    }
    
    enemy_remove(enemy);
  }
}

void player_attack_enemy(GameState *game, Enemy *enemy) {
  if (!enemy->active) {
    return;
  }

  const EnemyType *type = enemy_get_type(enemy->type_id);
  if (!type) {
    return;
  }

  // Calculate stamina cost based on equipped weapon
  int stamina_cost = 5;  // Base cost for unarmed
  if (game->player.equipped_weapon >= 0 && game->player.equipped_weapon < game->equipment_count) {
    const EquipmentItem *weapon = &game->equipment[game->player.equipped_weapon];
    stamina_cost = weapon->stamina_cost;
  }

  // Check if player has enough stamina
  if (game->player.stamina < stamina_cost) {
    game_set_status_message(game, "Too exhausted to attack! (need %d stamina)", stamina_cost);
    game->path_length = 0;  // Cancel any pathfinding
    return;
  }

  // Consume stamina for the attack
  game->player.stamina -= stamina_cost;
  if (game->player.stamina < 0) {
    game->player.stamina = 0;
  }

  // Calculate number of attacks based on speed
  int speed = game_get_total_speed(game);
  int num_attacks = 1;  // Base attack
  
  // Speed system: 100% speed = guaranteed 2nd attack, 200% = 3rd attack, etc.
  if (speed >= 100) {
    num_attacks++;
    speed -= 100;
  }
  if (speed >= 100) {
    num_attacks++;
    speed -= 100;
  }
  
  // Remaining speed is chance for one more attack
  if (speed > 0 && (rand() % 100) < speed) {
    num_attacks++;
  }

  // Perform each attack
  for (int i = 0; i < num_attacks; i++) {
    if (!enemy->active) break;  // Enemy might die mid-combo
    
    int attack_power = game_get_total_attack(game);
    int dexterity = game_get_total_dexterity(game);
    
    // Critical hit chance based on dexterity (1% per dexterity point)
    int is_critical = (rand() % 100) < dexterity;
    
    int damage = attack_power + (rand() % (attack_power / 2 + 1));
    
    if (is_critical) {
      damage = (damage * 3) / 2;  // 1.5x damage on crit
    }
    
    enemy_take_damage(game, enemy, damage);
    
    if (is_critical) {
      game_set_status_message(game, "CRITICAL HIT! %d damage!", damage);
    }
  }
  
  if (num_attacks > 1) {
    game_set_status_message(game, "You attack %d times!", num_attacks);
  }
}

void enemy_attack_player(GameState *game, Enemy *enemy) {
  if (!enemy->active) {
    return;
  }

  const EnemyType *type = enemy_get_type(enemy->type_id);
  if (!type) {
    return;
  }

  // Calculate damage with player defense
  int base_damage = type->attack_power + (rand() % 3);
  int player_defense = game_get_total_defense(game);
  int damage = base_damage - player_defense;
  
  if (damage < 1) {
    damage = 1;  // Always do at least 1 damage
  }

  game->player.hp -= damage;
  if (game->player.hp < 0) {
    game->player.hp = 0;
  }

  char msg[256];
  if (player_defense > 0) {
    snprintf(msg, sizeof(msg), "%s hits you for %d damage (%d blocked)!", 
             type->name, damage, player_defense);
  } else {
    snprintf(msg, sizeof(msg), "%s hits you for %d damage!", type->name, damage);
  }
  game_set_status_message(game, "%s", msg);

  if (game->player.hp <= 0) {
    snprintf(msg, sizeof(msg), "You died! Game Over.");
    game_set_status_message(game, "%s", msg);
    game->state = GAME_STATE_QUIT;
  }
}

int enemy_load_types_from_json(EnemyType *types, int max_types) {
  int count = 0;
  if (data_load_enemies(types, max_types, &count)) {
    g_enemy_type_count = count;
    return 1;
  }
  return 0;
}

