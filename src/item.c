#include "../include/item.h"
#include "../include/game.h"
#include "../include/enemy.h"
#include "../include/data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global item types
static ItemType g_item_types[MAX_ITEM_TYPES];
static int g_item_type_count = 0;

void item_init_types(void) {
  g_item_type_count = 0;
  
  // Try to load from JSON first
  if (data_load_item_types(g_item_types, MAX_ITEM_TYPES, &g_item_type_count)) {
    return; // Successfully loaded from JSON
  }
  
  // Fallback to hardcoded items
  {
    // Hardcoded fallback items
    g_item_type_count = 8;
    
    // Gold
    strncpy(g_item_types[0].name, "Gold Coin", MAX_ITEM_NAME - 1);
    strncpy(g_item_types[0].description, "Currency for trading", MAX_ITEM_DESC - 1);
    g_item_types[0].glyph = '$';
    g_item_types[0].category = ITEM_CATEGORY_CURRENCY;
    g_item_types[0].rarity = ITEM_RARITY_COMMON;
    g_item_types[0].hp_restore = 0;
    g_item_types[0].stamina_restore = 0;
    g_item_types[0].mp_restore = 0;
    g_item_types[0].value = 1;
    g_item_types[0].color_r = 255;
    g_item_types[0].color_g = 215;
    g_item_types[0].color_b = 0;
    
    // Health Potion
    strncpy(g_item_types[1].name, "Health Potion", MAX_ITEM_NAME - 1);
    strncpy(g_item_types[1].description, "Restores 30 HP", MAX_ITEM_DESC - 1);
    g_item_types[1].glyph = '!';
    g_item_types[1].category = ITEM_CATEGORY_POTION;
    g_item_types[1].rarity = ITEM_RARITY_COMMON;
    g_item_types[1].hp_restore = 30;
    g_item_types[1].stamina_restore = 0;
    g_item_types[1].mp_restore = 0;
    g_item_types[1].value = 50;
    g_item_types[1].color_r = 255;
    g_item_types[1].color_g = 50;
    g_item_types[1].color_b = 50;
    
    // Stamina Potion
    strncpy(g_item_types[2].name, "Stamina Potion", MAX_ITEM_NAME - 1);
    strncpy(g_item_types[2].description, "Restores 50 stamina", MAX_ITEM_DESC - 1);
    g_item_types[2].glyph = '!';
    g_item_types[2].category = ITEM_CATEGORY_POTION;
    g_item_types[2].rarity = ITEM_RARITY_COMMON;
    g_item_types[2].hp_restore = 0;
    g_item_types[2].stamina_restore = 50;
    g_item_types[2].mp_restore = 0;
    g_item_types[2].value = 40;
    g_item_types[2].color_r = 255;
    g_item_types[2].color_g = 255;
    g_item_types[2].color_b = 50;
    
    // Mana Potion
    strncpy(g_item_types[3].name, "Mana Potion", MAX_ITEM_NAME - 1);
    strncpy(g_item_types[3].description, "Restores 20 MP", MAX_ITEM_DESC - 1);
    g_item_types[3].glyph = '!';
    g_item_types[3].category = ITEM_CATEGORY_POTION;
    g_item_types[3].rarity = ITEM_RARITY_UNCOMMON;
    g_item_types[3].hp_restore = 0;
    g_item_types[3].stamina_restore = 0;
    g_item_types[3].mp_restore = 20;
    g_item_types[3].value = 60;
    g_item_types[3].color_r = 100;
    g_item_types[3].color_g = 100;
    g_item_types[3].color_b = 255;
    
    // Bread
    strncpy(g_item_types[4].name, "Bread", MAX_ITEM_NAME - 1);
    strncpy(g_item_types[4].description, "Restores 10 HP and 20 stamina", MAX_ITEM_DESC - 1);
    g_item_types[4].glyph = '%';
    g_item_types[4].category = ITEM_CATEGORY_FOOD;
    g_item_types[4].rarity = ITEM_RARITY_COMMON;
    g_item_types[4].hp_restore = 10;
    g_item_types[4].stamina_restore = 20;
    g_item_types[4].mp_restore = 0;
    g_item_types[4].value = 10;
    g_item_types[4].color_r = 210;
    g_item_types[4].color_g = 180;
    g_item_types[4].color_b = 140;
    
    // Cheese
    strncpy(g_item_types[5].name, "Cheese", MAX_ITEM_NAME - 1);
    strncpy(g_item_types[5].description, "Restores 15 HP and 30 stamina", MAX_ITEM_DESC - 1);
    g_item_types[5].glyph = '%';
    g_item_types[5].category = ITEM_CATEGORY_FOOD;
    g_item_types[5].rarity = ITEM_RARITY_UNCOMMON;
    g_item_types[5].hp_restore = 15;
    g_item_types[5].stamina_restore = 30;
    g_item_types[5].mp_restore = 0;
    g_item_types[5].value = 20;
    g_item_types[5].color_r = 255;
    g_item_types[5].color_g = 255;
    g_item_types[5].color_b = 150;
    
    // Roasted Meat
    strncpy(g_item_types[6].name, "Roasted Meat", MAX_ITEM_NAME - 1);
    strncpy(g_item_types[6].description, "Restores 25 HP and 40 stamina", MAX_ITEM_DESC - 1);
    g_item_types[6].glyph = '%';
    g_item_types[6].category = ITEM_CATEGORY_FOOD;
    g_item_types[6].rarity = ITEM_RARITY_RARE;
    g_item_types[6].hp_restore = 25;
    g_item_types[6].stamina_restore = 40;
    g_item_types[6].mp_restore = 0;
    g_item_types[6].value = 35;
    g_item_types[6].color_r = 180;
    g_item_types[6].color_g = 100;
    g_item_types[6].color_b = 70;
    
    // Elixir of Power
    strncpy(g_item_types[7].name, "Elixir of Power", MAX_ITEM_NAME - 1);
    strncpy(g_item_types[7].description, "Fully restores HP, MP, and Stamina", MAX_ITEM_DESC - 1);
    g_item_types[7].glyph = '!';
    g_item_types[7].category = ITEM_CATEGORY_POTION;
    g_item_types[7].rarity = ITEM_RARITY_LEGENDARY;
    g_item_types[7].hp_restore = 9999;
    g_item_types[7].stamina_restore = 9999;
    g_item_types[7].mp_restore = 9999;
    g_item_types[7].value = 500;
    g_item_types[7].color_r = 255;
    g_item_types[7].color_g = 0;
    g_item_types[7].color_b = 255;
  }
}

int item_get_type_count(void) {
  return g_item_type_count;
}

const ItemType *item_get_type(int type_id) {
  if (type_id < 0 || type_id >= g_item_type_count) {
    return NULL;
  }
  return &g_item_types[type_id];
}

void item_spawn_ground(GameState *game, int type_id, int x, int y, int quantity) {
  if (type_id < 0 || type_id >= g_item_type_count) {
    return;
  }

  // Find empty slot
  for (int i = 0; i < MAX_GROUND_ITEMS; i++) {
    if (!game->ground_items[i].active) {
      game->ground_items[i].active = 1;
      game->ground_items[i].type_id = type_id;
      game->ground_items[i].x = x;
      game->ground_items[i].y = y;
      game->ground_items[i].quantity = quantity;
      return;
    }
  }
}

void item_remove_ground(GroundItem *item) {
  item->active = 0;
}

GroundItem *item_get_at_position(GameState *game, int x, int y) {
  for (int i = 0; i < MAX_GROUND_ITEMS; i++) {
    if (game->ground_items[i].active && 
        game->ground_items[i].x == x && 
        game->ground_items[i].y == y) {
      return &game->ground_items[i];
    }
  }
  return NULL;
}

void item_pickup(GameState *game, GroundItem *ground_item) {
  const ItemType *type = item_get_type(ground_item->type_id);
  if (!type) {
    return;
  }

  // Handle currency
  if (type->category == ITEM_CATEGORY_CURRENCY) {
    game->player.gold += ground_item->quantity;
    game_set_status_message(game, "Picked up %d gold!", ground_item->quantity);
    item_remove_ground(ground_item);
    return;
  }

  // Add to backpack (find existing or create new)
  for (int i = 0; i < game->backpack_count; i++) {
    if (strcmp(game->backpack[i].name, type->name) == 0) {
      game->backpack[i].quantity += ground_item->quantity;
      game_set_status_message(game, "Picked up %s (x%d)", type->name, ground_item->quantity);
      item_remove_ground(ground_item);
      return;
    }
  }

  // Create new backpack entry
  if (game->backpack_count < MAX_BACKPACK) {
    strncpy(game->backpack[game->backpack_count].name, type->name, MAX_NAME_LENGTH - 1);
    strncpy(game->backpack[game->backpack_count].description, type->description, MAX_DESC_LENGTH - 1);
    game->backpack[game->backpack_count].quantity = ground_item->quantity;
    game->backpack_count++;
    game_set_status_message(game, "Picked up %s!", type->name);
    item_remove_ground(ground_item);
  } else {
    game_set_status_message(game, "Backpack full!");
  }
}

void item_drop_loot(GameState *game, int enemy_type_id, int x, int y) {
  const EnemyType *enemy_type = enemy_get_type(enemy_type_id);
  if (!enemy_type) {
    return;
  }

  // Calculate loot quality based on enemy difficulty
  int enemy_difficulty = enemy_type->max_hp + enemy_type->attack_power + enemy_type->defense;
  
  // Always drop some gold
  int gold_amount = enemy_type->xp_value / 5 + (rand() % (enemy_type->xp_value / 3 + 1));
  if (gold_amount > 0) {
    item_spawn_ground(game, 0, x, y, gold_amount); // 0 = gold coin type
  }

  // Chance for consumable items
  int loot_roll = rand() % 100;
  int loot_chance = 30 + (enemy_difficulty / 10);
  
  if (loot_roll < loot_chance && g_item_type_count > 1) {
    int item_type = -1;
    
    if (enemy_difficulty < 20) {
      int common_items[] = {1, 2, 4}; // Health potion, stamina potion, bread
      item_type = common_items[rand() % 3];
    } else if (enemy_difficulty < 40) {
      int uncommon_items[] = {1, 2, 3, 5}; // Potions and cheese
      item_type = uncommon_items[rand() % 4];
    } else if (enemy_difficulty < 80) {
      int rare_items[] = {1, 2, 3, 6}; // All potions and roasted meat
      item_type = rare_items[rand() % 4];
    } else {
      item_type = 1 + (rand() % (g_item_type_count - 1));
      if (rand() % 100 < 5) {
        item_type = 7; // Elixir of Power
      }
    }
    
    if (item_type >= 0 && item_type < g_item_type_count) {
      item_spawn_ground(game, item_type, x, y, 1);
    }
  }
  
  // Chance for equipment drops (rarer than consumables)
  int equip_roll = rand() % 100;
  int equip_chance = 5 + (enemy_difficulty / 20); // Base 5% + scaling
  
  if (equip_roll < equip_chance && game->equipment_count < MAX_EQUIPMENT) {
    // Equipment drop message based on enemy difficulty
    // Actual equipment generation is handled by the equipment loading system
    if (enemy_difficulty < 20) {
      game_set_status_message(game, "Enemy dropped common equipment!");
    } else if (enemy_difficulty < 40) {
      game_set_status_message(game, "Enemy dropped uncommon equipment!");
    } else if (enemy_difficulty < 80) {
      game_set_status_message(game, "Enemy dropped rare equipment!");
    } else {
      game_set_status_message(game, "Enemy dropped legendary equipment!");
    }
  }
}

int item_use_consumable(GameState *game, int backpack_index) {
  if (backpack_index < 0 || backpack_index >= game->backpack_count) {
    return 0;
  }

  BackpackItem *item = &game->backpack[backpack_index];
  if (item->quantity <= 0) {
    return 0;
  }

  // Find matching item type
  int type_id = -1;
  for (int i = 0; i < g_item_type_count; i++) {
    if (strcmp(g_item_types[i].name, item->name) == 0) {
      type_id = i;
      break;
    }
  }

  if (type_id < 0) {
    return 0;
  }

  const ItemType *type = item_get_type(type_id);
  if (!type) {
    return 0;
  }

  // Use the item
  int used = 0;
  
  if (type->hp_restore > 0) {
    int old_hp = game->player.hp;
    game->player.hp += type->hp_restore;
    if (game->player.hp > PLAYER_MAX_HP) {
      game->player.hp = PLAYER_MAX_HP;
    }
    if (game->player.hp > old_hp) {
      used = 1;
    }
  }
  
  if (type->stamina_restore > 0) {
    int old_stamina = game->player.stamina;
    game->player.stamina += type->stamina_restore;
    if (game->player.stamina > PLAYER_MAX_STAMINA) {
      game->player.stamina = PLAYER_MAX_STAMINA;
    }
    if (game->player.stamina > old_stamina) {
      used = 1;
    }
  }
  
  if (type->mp_restore > 0) {
    int old_mp = game->player.mp;
    game->player.mp += type->mp_restore;
    if (game->player.mp > PLAYER_MAX_MP) {
      game->player.mp = PLAYER_MAX_MP;
    }
    if (game->player.mp > old_mp) {
      used = 1;
    }
  }

  if (used) {
    item->quantity--;
    game_set_status_message(game, "Used %s!", type->name);
    
    // Remove from backpack if quantity reaches 0
    if (item->quantity <= 0) {
      for (int i = backpack_index; i < game->backpack_count - 1; i++) {
        game->backpack[i] = game->backpack[i + 1];
      }
      game->backpack_count--;
    }
    return 1;
  } else {
    game_set_status_message(game, "Already at full %s!", 
      type->hp_restore > 0 ? "HP" : (type->stamina_restore > 0 ? "stamina" : "MP"));
    return 0;
  }
}


