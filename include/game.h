#ifndef GAME_H
#define GAME_H

#include <stdint.h>

#include "map.h"

// Forward declarations to avoid circular dependency
typedef struct Enemy Enemy;
typedef struct GroundItem GroundItem;

enum {
  DEFAULT_TILE_SIZE = 16,
  MIN_TILE_SIZE = 8,
  MAX_TILE_SIZE = 48,
  TITLE_BAR_HEIGHT = 30,
  STATUS_BAR_HEIGHT = 30,
  SIDE_PANEL_WIDTH = 340,
  MOVE_INTERVAL_MS = 120,
  PLAYER_MAX_HP = 100,
  PLAYER_MAX_STAMINA = 100,
  PLAYER_MAX_MP = 50,
  XP_PER_LEVEL = 100,
  MAX_SPELLS = 10,
  MAX_SPELL_HOTBAR = 9,
  MAX_EQUIPMENT = 8,
  MAX_BACKPACK = 20,
  MAX_ENEMIES = 200,
  MAX_GROUND_ITEMS = 100,
  DEFAULT_VISION_RADIUS = 8
};

typedef enum {
  GAME_STATE_START_SCREEN,
  GAME_STATE_PLAYING,
  GAME_STATE_QUIT
} GameStateType;

typedef enum {
  EQUIP_SLOT_WEAPON = 0,
  EQUIP_SLOT_ARMOR = 1,
  EQUIP_SLOT_ACCESSORY = 2,
  EQUIP_SLOT_BOOTS = 3
} EquipmentSlot;

typedef enum {
  EQUIP_RARITY_COMMON = 0,
  EQUIP_RARITY_UNCOMMON = 1,
  EQUIP_RARITY_RARE = 2,
  EQUIP_RARITY_EPIC = 3,
  EQUIP_RARITY_LEGENDARY = 4
} EquipmentRarity;

typedef struct {
  int x;
  int y;
  int hp;
  int stamina;
  int mp;
  int xp;
  int level;
  int gold;
  // Base stats (improved by leveling)
  int base_attack;
  int base_defense;
  int base_dexterity;
  int base_speed;
  // Equipped item IDs (-1 = nothing equipped)
  int equipped_weapon;
  int equipped_armor;
  int equipped_accessory;
  int equipped_boots;
} Player;

typedef enum {
  MENU_NONE = 0,
  MENU_INVENTORY,
  MENU_ITEM_PICKUP,
  MENU_SPELL_CODEX
} MenuType;

typedef enum {
  TARGETING_NONE = 0,
  TARGETING_SPELL
} TargetingMode;

typedef enum {
  GAME_INPUT_NONE = 0,
  GAME_INPUT_MOVE_UP,
  GAME_INPUT_MOVE_DOWN,
  GAME_INPUT_MOVE_LEFT,
  GAME_INPUT_MOVE_RIGHT,
  GAME_INPUT_MOVE_TO_POS,
  GAME_INPUT_START_GAME,
  GAME_INPUT_QUIT,
  GAME_INPUT_MENU_TOGGLE,
  GAME_INPUT_MENU_SELECT,
  GAME_INPUT_MENU_BACK,
  GAME_INPUT_SAVE,
  GAME_INPUT_LOAD,
  GAME_INPUT_ATTACK_ENEMY,
  GAME_INPUT_EQUIP_ITEM,
  GAME_INPUT_UNEQUIP_ITEM,
  GAME_INPUT_USE_ITEM,
  GAME_INPUT_REST,
  GAME_INPUT_ZOOM_IN,
  GAME_INPUT_ZOOM_OUT,
  GAME_INPUT_PICKUP_ALL,
  GAME_INPUT_PICKUP_ITEM_1,
  GAME_INPUT_PICKUP_ITEM_2,
  GAME_INPUT_PICKUP_ITEM_3,
  GAME_INPUT_PICKUP_ITEM_4,
  GAME_INPUT_PICKUP_ITEM_5,
  GAME_INPUT_CAST_SPELL_1,
  GAME_INPUT_CAST_SPELL_2,
  GAME_INPUT_CAST_SPELL_3,
  GAME_INPUT_CAST_SPELL_4,
  GAME_INPUT_CAST_SPELL_5,
  GAME_INPUT_CAST_SPELL_6,
  GAME_INPUT_CAST_SPELL_7,
  GAME_INPUT_CAST_SPELL_8,
  GAME_INPUT_CAST_SPELL_9,
  GAME_INPUT_DROP_ITEM,
  GAME_INPUT_INVENTORY,
  GAME_INPUT_SPELL_CODEX,
  GAME_INPUT_CANCEL_TARGETING,
  GAME_INPUT_INTERACT
} GameInput;

#define PATH_MAX_LENGTH 500

typedef struct {
  int x;
  int y;
} PathNode;

#define MAX_NAME_LENGTH 64
#define MAX_DESC_LENGTH 128
#define MAX_SPELL_PATTERN_SIZE 25  // 5x5 grid max for AoE patterns

typedef enum {
  SPELL_TARGET_SELF,        // Cast on self, no targeting needed
  SPELL_TARGET_SINGLE,      // Single tile target
  SPELL_TARGET_AREA,        // Area effect with epicenter
  SPELL_TARGET_CONE,        // Cone from player
  SPELL_TARGET_LINE,        // Line from player to target
  SPELL_TARGET_BEAM         // Beam in direction
} SpellTargetType;

typedef enum {
  SPELL_EFFECT_DAMAGE,      // Deal damage
  SPELL_EFFECT_HEAL,        // Heal HP
  SPELL_EFFECT_BUFF,        // Increase stats temporarily
  SPELL_EFFECT_DEBUFF,      // Decrease enemy stats
  SPELL_EFFECT_SUMMON,      // Summon creature
  SPELL_EFFECT_TELEPORT,    // Teleport player
  SPELL_EFFECT_LIGHT,       // Increase vision
  SPELL_EFFECT_KNOCK_BACK   // Push enemies
} SpellEffectType;

typedef struct {
  char name[MAX_NAME_LENGTH];
  char description[MAX_DESC_LENGTH];
  int mp_cost;
  int level_required;       // Player level required to use
  SpellTargetType target_type;
  SpellEffectType effect_type;
  int range;                // Max range in tiles
  int radius;               // AoE radius (0 = single target)
  int power;                // Base damage/heal amount
  int duration;             // Duration in turns for buffs/debuffs
  // Pattern defines which tiles are affected (relative to center)
  // 1 = affected, 0 = not affected
  int pattern[MAX_SPELL_PATTERN_SIZE][MAX_SPELL_PATTERN_SIZE];
  int pattern_width;
  int pattern_height;
  int color_r, color_g, color_b;  // Spell visual color
  char glyph;               // Visual representation
} Spell;

typedef struct {
  char name[MAX_NAME_LENGTH];
  char description[MAX_DESC_LENGTH];
  EquipmentSlot slot;
  EquipmentRarity rarity;
  int attack_bonus;
  int defense_bonus;
  int dexterity_bonus;
  int speed_bonus;
  int stamina_cost;  // Stamina cost per attack (0 for non-weapons)
  int value;
  int color_r;
  int color_g;
  int color_b;
} EquipmentItem;

typedef struct {
  char name[MAX_NAME_LENGTH];
  char description[MAX_DESC_LENGTH];
  int quantity;
} BackpackItem;

// Include enemy.h and item.h here, after our own forward declarations are done
#include "enemy.h"
#include "item.h"

// Fog of war tile states
typedef enum {
  FOG_UNEXPLORED = 0,  // Never seen
  FOG_EXPLORED = 1,     // Seen before but not currently visible
  FOG_VISIBLE = 2       // Currently visible
} FogState;

typedef struct GameState {
  GameStateType state;
  Tile map[MAP_HEIGHT][MAP_WIDTH];
  FogState fog[MAP_HEIGHT][MAP_WIDTH];  // Fog of war tracking
  Player player;
  int cam_x;
  int cam_y;
  uint32_t last_move_time;
  int current_level;
  PathNode path[PATH_MAX_LENGTH];
  int path_length;
  int path_index;
  MenuType menu_type;
  int menu_selection;
  Spell spells[MAX_SPELLS];
  int spell_count;
  int spell_hotbar[MAX_SPELL_HOTBAR];  // Indices into spells array (-1 = empty slot)
  TargetingMode targeting_mode;
  int targeting_spell_idx;  // Index of spell being targeted
  int targeting_x, targeting_y;  // Current mouse target position
  int targeting_valid;      // Whether current target is valid
  EquipmentItem equipment[MAX_EQUIPMENT];
  int equipment_count;
  BackpackItem backpack[MAX_BACKPACK];
  int backpack_count;
  Enemy enemies[MAX_ENEMIES];
  GroundItem ground_items[MAX_GROUND_ITEMS];
  char status_message[128];
  uint32_t status_message_time;
  int is_resting;           // Flag for rest state
  int rest_turns_remaining;  // Turns left to rest
  int rest_stamina_per_turn; // Stamina regained per rest turn
  int vision_radius;        // How far player can see
  int zoom_level;           // Current zoom level (tile size)
  int is_on_item_tile;      // Flag when player is standing on items
  int items_at_player[5];   // Indices of ground items at player position (max 5)
  int item_count_at_player; // Number of items at player position
} GameState;

void game_init(GameState *game);
void game_start(GameState *game);
void game_compute_view(const GameState *game, int window_w, int window_h,
                       int *tiles_w, int *tiles_h);
void game_update_camera(GameState *game, int tiles_w, int tiles_h);
int game_handle_input(GameState *game, GameInput input, int target_x, int target_y, uint32_t now);
void game_update(GameState *game, uint32_t now);
void game_change_level(GameState *game, int direction);
void game_update_fog_of_war(GameState *game);  // Update visible tiles
int game_get_tile_size(const GameState *game);  // Get current tile size based on zoom
int game_menu_open(GameState *game);
int game_menu_close(GameState *game);
int game_menu_selected(const GameState *game);
int game_menu_count(const GameState *game);
const char *game_menu_item_name(const GameState *game, int index);
int game_menu_click(GameState *game, int mouse_x, int mouse_y, int window_w, int window_h);
void game_spawn_enemies(GameState *game);
void game_set_status_message(GameState *game, const char *fmt, ...);

// Spell system functions
void spell_init_pattern_circle(Spell *spell, int radius);
void spell_init_pattern_cone(Spell *spell);
void spell_init_pattern_line(Spell *spell);
int spell_can_target(const GameState *game, const Spell *spell, int target_x, int target_y);
void spell_get_affected_tiles(const GameState *game, const Spell *spell, int target_x, int target_y,
                               int *tiles_x, int *tiles_y, int *tile_count, int max_tiles);
void spell_cast(GameState *game, int spell_idx, int target_x, int target_y);
void game_start_spell_targeting(GameState *game, int spell_idx);
void game_update_spell_targeting(GameState *game, int mouse_x, int mouse_y, int window_w, int window_h);
void game_cancel_spell_targeting(GameState *game);
void game_confirm_spell_cast(GameState *game);
void game_spell_codex_toggle(GameState *game);

// Equipment functions
int game_get_total_attack(const GameState *game);
int game_get_total_defense(const GameState *game);
int game_get_total_dexterity(const GameState *game);
int game_get_total_speed(const GameState *game);
void game_equip_item(GameState *game, int equipment_index);
void game_unequip_slot(GameState *game, EquipmentSlot slot);
const EquipmentItem *game_get_equipped_item(const GameState *game, EquipmentSlot slot);

#endif /* GAME_H */
