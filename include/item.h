#ifndef ITEM_H
#define ITEM_H

#include <stdint.h>

// Forward declaration
typedef struct GameState GameState;

// MAX_GROUND_ITEMS is defined in game.h enum
#define MAX_ITEM_TYPES 50
#define MAX_ITEM_NAME 64
#define MAX_ITEM_DESC 128

// Item categories
typedef enum {
  ITEM_CATEGORY_CONSUMABLE,
  ITEM_CATEGORY_CURRENCY,
  ITEM_CATEGORY_FOOD,
  ITEM_CATEGORY_POTION,
  ITEM_CATEGORY_MISC
} ItemCategory;

// Item rarity affects drop rates and value
typedef enum {
  ITEM_RARITY_COMMON = 0,
  ITEM_RARITY_UNCOMMON = 1,
  ITEM_RARITY_RARE = 2,
  ITEM_RARITY_EPIC = 3,
  ITEM_RARITY_LEGENDARY = 4
} ItemRarity;

// Item type definition (template loaded from JSON or hardcoded)
typedef struct {
  char name[MAX_ITEM_NAME];
  char description[MAX_ITEM_DESC];
  char glyph;
  ItemCategory category;
  ItemRarity rarity;
  int hp_restore;      // Amount of HP restored (0 if none)
  int stamina_restore; // Amount of stamina restored
  int mp_restore;      // Amount of MP restored
  int value;           // Gold value
  int color_r;
  int color_g;
  int color_b;
} ItemType;

// Item instance on the ground
typedef struct GroundItem {
  int active;
  int type_id;  // Index into item types
  int x;
  int y;
  int quantity; // Stack size for currency/consumables
} GroundItem;

// Item system functions
void item_init_types(void);
int item_get_type_count(void);
const ItemType *item_get_type(int type_id);
int item_load_types_from_json(ItemType *types, int max_types);

// Ground item management
void item_spawn_ground(GameState *game, int type_id, int x, int y, int quantity);
void item_remove_ground(GroundItem *item);
GroundItem *item_get_at_position(GameState *game, int x, int y);
void item_pickup(GameState *game, GroundItem *item);

// Item drops from enemies
void item_drop_loot(GameState *game, int enemy_type_id, int x, int y);

// Item usage
int item_use_consumable(GameState *game, int type_id);

#endif // ITEM_H

