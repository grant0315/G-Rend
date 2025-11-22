#ifndef ENEMY_H
#define ENEMY_H

#include <stdint.h>

// Forward declare GameState to avoid circular dependency
typedef struct GameState GameState;

#define MAX_ENEMY_TYPES 50
#define MAX_ENEMY_NAME_LENGTH 64
#define MAX_ENEMY_DESC_LENGTH 128

// Enemy type definition (loaded from JSON)
typedef struct {
  char name[MAX_ENEMY_NAME_LENGTH];
  char description[MAX_ENEMY_DESC_LENGTH];
  char glyph;
  int max_hp;
  int attack_power;
  int defense;
  int xp_value;
  int aggro_range; // How far away they can detect the player
  int color_r;
  int color_g;
  int color_b;
} EnemyType;

// Enemy instance in the game world
typedef struct Enemy {
  int active; // 0 = dead/inactive, 1 = alive
  int type_id; // Index into enemy_types array
  int x;
  int y;
  int hp;
  uint32_t last_move_time;
  int is_alerted; // Whether enemy knows player location
  int patrol_x; // Target x for patrol
  int patrol_y; // Target y for patrol
  int patrol_timer; // Time until choosing new patrol point
} Enemy;

// Enemy system functions
void enemy_init_types(void);
int enemy_get_type_count(void);
const EnemyType *enemy_get_type(int type_id);
int enemy_load_types_from_json(EnemyType *types, int max_types);

// Enemy instance management
void enemy_spawn(GameState *game, int type_id, int x, int y);
void enemy_update_all(GameState *game, uint32_t now); // Legacy - kept for compatibility
void enemy_remove(Enemy *enemy);
Enemy *enemy_get_at_position(GameState *game, int x, int y);

// Combat
void enemy_take_damage(GameState *game, Enemy *enemy, int damage);
void player_attack_enemy(GameState *game, Enemy *enemy);
void enemy_attack_player(GameState *game, Enemy *enemy);

#endif // ENEMY_H

