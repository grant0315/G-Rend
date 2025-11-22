#include "../include/data.h"
#include "../include/item.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__has_include)
#if __has_include(<cjson/cJSON.h>)
#include <cjson/cJSON.h>
#elif __has_include(<cJSON.h>)
#include <cJSON.h>
#elif __has_include("cjson/cJSON.h")
#include "cjson/cJSON.h"
#elif __has_include("cJSON.h")
#include "cJSON.h"
#else
#define NO_CJSON
#endif
#else
#define NO_CJSON
#endif

#ifndef NO_CJSON

static int load_string_field(cJSON *item, const char *field, char *dest, int max_len) {
  cJSON *json_field = cJSON_GetObjectItem(item, field);
  if (!json_field || !cJSON_IsString(json_field)) {
    return 0;
  }
  const char *str = cJSON_GetStringValue(json_field);
  strncpy(dest, str, max_len - 1);
  dest[max_len - 1] = '\0';
  return 1;
}

static int load_int_field(cJSON *item, const char *field, int *dest) {
  cJSON *json_field = cJSON_GetObjectItem(item, field);
  if (!json_field || !cJSON_IsNumber(json_field)) {
    return 0;
  }
  *dest = cJSON_GetNumberValue(json_field);
  return 1;
}

static char *read_file(const char *filename) {
  FILE *f = fopen(filename, "rb");
  if (!f) {
    return NULL;
  }

  fseek(f, 0, SEEK_END);
  long length = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *buffer = malloc(length + 1);
  if (!buffer) {
    fclose(f);
    return NULL;
  }

  fread(buffer, 1, length, f);
  buffer[length] = '\0';
  fclose(f);
  return buffer;
}

int data_load_spells(Spell *spells, int max_count, int *count) {
  char *json_string = read_file("data/spells.json");
  if (!json_string) {
    return 0;
  }

  cJSON *json = cJSON_Parse(json_string);
  if (!json) {
    free(json_string);
    return 0;
  }

  cJSON *spells_array = cJSON_GetObjectItem(json, "spells");
  if (!cJSON_IsArray(spells_array)) {
    cJSON_Delete(json);
    free(json_string);
    return 0;
  }

  int array_size = cJSON_GetArraySize(spells_array);
  *count = (array_size < max_count) ? array_size : max_count;

  for (int i = 0; i < *count; i++) {
    cJSON *item = cJSON_GetArrayItem(spells_array, i);
    if (!item) {
      continue;
    }

    memset(&spells[i], 0, sizeof(Spell));
    load_string_field(item, "name", spells[i].name, MAX_NAME_LENGTH);
    load_string_field(item, "description", spells[i].description, MAX_DESC_LENGTH);
    load_int_field(item, "mp_cost", &spells[i].mp_cost);
    load_int_field(item, "level_required", &spells[i].level_required);
    
    int target_type, effect_type;
    if (load_int_field(item, "target_type", &target_type)) {
      spells[i].target_type = (SpellTargetType)target_type;
    }
    if (load_int_field(item, "effect_type", &effect_type)) {
      spells[i].effect_type = (SpellEffectType)effect_type;
    }
    
    load_int_field(item, "range", &spells[i].range);
    load_int_field(item, "radius", &spells[i].radius);
    load_int_field(item, "power", &spells[i].power);
    load_int_field(item, "duration", &spells[i].duration);
    load_int_field(item, "color_r", &spells[i].color_r);
    load_int_field(item, "color_g", &spells[i].color_g);
    load_int_field(item, "color_b", &spells[i].color_b);
    
    // Load glyph (single character)
    cJSON *glyph = cJSON_GetObjectItem(item, "glyph");
    if (glyph && cJSON_IsString(glyph) && glyph->valuestring && strlen(glyph->valuestring) > 0) {
      spells[i].glyph = glyph->valuestring[0];
    } else {
      spells[i].glyph = '*';
    }
    
    // Initialize spell patterns based on target type
    if (spells[i].target_type == SPELL_TARGET_AREA && spells[i].radius > 0) {
      spell_init_pattern_circle(&spells[i], spells[i].radius);
    } else if (spells[i].target_type == SPELL_TARGET_CONE) {
      spell_init_pattern_cone(&spells[i]);
    } else if (spells[i].target_type == SPELL_TARGET_LINE) {
      spell_init_pattern_line(&spells[i]);
    }
  }

  cJSON_Delete(json);
  free(json_string);
  return 1;
}

static EquipmentRarity parse_equipment_rarity(const char *str) {
  if (strcmp(str, "common") == 0) return EQUIP_RARITY_COMMON;
  if (strcmp(str, "uncommon") == 0) return EQUIP_RARITY_UNCOMMON;
  if (strcmp(str, "rare") == 0) return EQUIP_RARITY_RARE;
  if (strcmp(str, "epic") == 0) return EQUIP_RARITY_EPIC;
  if (strcmp(str, "legendary") == 0) return EQUIP_RARITY_LEGENDARY;
  return EQUIP_RARITY_COMMON;
}

int data_load_equipment_items(EquipmentItem *equipment, int max_count, int *count) {
  char *json_string = read_file("data/equipment_items.json");
  if (!json_string) {
    return 0;
  }

  cJSON *json = cJSON_Parse(json_string);
  if (!json) {
    free(json_string);
    return 0;
  }

  cJSON *equipment_array = json;
  if (!cJSON_IsArray(equipment_array)) {
    cJSON_Delete(json);
    free(json_string);
    return 0;
  }

  int array_size = cJSON_GetArraySize(equipment_array);
  *count = (array_size < max_count) ? array_size : max_count;

  for (int i = 0; i < *count; i++) {
    cJSON *item = cJSON_GetArrayItem(equipment_array, i);
    if (!item) {
      continue;
    }

    memset(&equipment[i], 0, sizeof(EquipmentItem));
    load_string_field(item, "name", equipment[i].name, MAX_NAME_LENGTH);
    load_string_field(item, "description", equipment[i].description, MAX_DESC_LENGTH);
    
    int slot_num;
    if (load_int_field(item, "slot", &slot_num)) {
      equipment[i].slot = (EquipmentSlot)slot_num;
    }
    
    cJSON *rarity_field = cJSON_GetObjectItem(item, "rarity");
    if (rarity_field && cJSON_IsString(rarity_field) && rarity_field->valuestring) {
      equipment[i].rarity = parse_equipment_rarity(rarity_field->valuestring);
    } else {
      equipment[i].rarity = EQUIP_RARITY_COMMON;
    }
    
    load_int_field(item, "attack_bonus", &equipment[i].attack_bonus);
    load_int_field(item, "defense_bonus", &equipment[i].defense_bonus);
    load_int_field(item, "dexterity_bonus", &equipment[i].dexterity_bonus);
    load_int_field(item, "speed_bonus", &equipment[i].speed_bonus);
    load_int_field(item, "stamina_cost", &equipment[i].stamina_cost);  // Load stamina cost
    load_int_field(item, "value", &equipment[i].value);
    load_int_field(item, "color_r", &equipment[i].color_r);
    load_int_field(item, "color_g", &equipment[i].color_g);
    load_int_field(item, "color_b", &equipment[i].color_b);
  }

  cJSON_Delete(json);
  free(json_string);
  return 1;
}

int data_load_backpack(BackpackItem *backpack, int max_count, int *count) {
  char *json_string = read_file("data/items.json");
  if (!json_string) {
    return 0;
  }

  cJSON *json = cJSON_Parse(json_string);
  if (!json) {
    free(json_string);
    return 0;
  }

  cJSON *items_array = cJSON_GetObjectItem(json, "items");
  if (!cJSON_IsArray(items_array)) {
    cJSON_Delete(json);
    free(json_string);
    return 0;
  }

  int array_size = cJSON_GetArraySize(items_array);
  *count = (array_size < max_count) ? array_size : max_count;

  for (int i = 0; i < *count; i++) {
    cJSON *item = cJSON_GetArrayItem(items_array, i);
    if (!item) {
      continue;
    }

    memset(&backpack[i], 0, sizeof(BackpackItem));
    load_string_field(item, "name", backpack[i].name, MAX_NAME_LENGTH);
    load_string_field(item, "description", backpack[i].description, MAX_DESC_LENGTH);
    if (!load_int_field(item, "quantity", &backpack[i].quantity)) {
      backpack[i].quantity = 1;
    }
  }

  cJSON_Delete(json);
  free(json_string);
  return 1;
}

int data_load_enemies(EnemyType *enemies, int max_count, int *count) {
  char *json_string = read_file("data/enemies.json");
  if (!json_string) {
    return 0;
  }

  cJSON *json = cJSON_Parse(json_string);
  if (!json) {
    free(json_string);
    return 0;
  }

  cJSON *enemies_array = json;
  if (!cJSON_IsArray(enemies_array)) {
    cJSON_Delete(json);
    free(json_string);
    return 0;
  }

  int array_size = cJSON_GetArraySize(enemies_array);
  *count = (array_size < max_count) ? array_size : max_count;

  for (int i = 0; i < *count; i++) {
    cJSON *enemy = cJSON_GetArrayItem(enemies_array, i);
    if (!enemy) {
      continue;
    }

    memset(&enemies[i], 0, sizeof(EnemyType));
    load_string_field(enemy, "name", enemies[i].name, MAX_ENEMY_NAME_LENGTH);
    load_string_field(enemy, "description", enemies[i].description, MAX_ENEMY_DESC_LENGTH);
    
    cJSON *glyph = cJSON_GetObjectItem(enemy, "glyph");
    if (glyph && cJSON_IsString(glyph) && glyph->valuestring && strlen(glyph->valuestring) > 0) {
      enemies[i].glyph = glyph->valuestring[0];
    } else {
      enemies[i].glyph = '?';
    }
    
    load_int_field(enemy, "max_hp", &enemies[i].max_hp);
    load_int_field(enemy, "attack_power", &enemies[i].attack_power);
    load_int_field(enemy, "defense", &enemies[i].defense);
    load_int_field(enemy, "xp_value", &enemies[i].xp_value);
    load_int_field(enemy, "aggro_range", &enemies[i].aggro_range);
    load_int_field(enemy, "color_r", &enemies[i].color_r);
    load_int_field(enemy, "color_g", &enemies[i].color_g);
    load_int_field(enemy, "color_b", &enemies[i].color_b);
  }

  cJSON_Delete(json);
  free(json_string);
  return 1;
}

static ItemCategory parse_item_category(const char *str) {
  if (strcmp(str, "currency") == 0) return ITEM_CATEGORY_CURRENCY;
  if (strcmp(str, "food") == 0) return ITEM_CATEGORY_FOOD;
  if (strcmp(str, "potion") == 0) return ITEM_CATEGORY_POTION;
  if (strcmp(str, "misc") == 0) return ITEM_CATEGORY_MISC;
  return ITEM_CATEGORY_CONSUMABLE;
}

static ItemRarity parse_item_rarity(const char *str) {
  if (strcmp(str, "common") == 0) return ITEM_RARITY_COMMON;
  if (strcmp(str, "uncommon") == 0) return ITEM_RARITY_UNCOMMON;
  if (strcmp(str, "rare") == 0) return ITEM_RARITY_RARE;
  if (strcmp(str, "epic") == 0) return ITEM_RARITY_EPIC;
  if (strcmp(str, "legendary") == 0) return ITEM_RARITY_LEGENDARY;
  return ITEM_RARITY_COMMON;
}

int data_load_item_types(ItemType *types, int max_count, int *count) {
  char *json_string = read_file("data/consumables.json");
  if (!json_string) {
    return 0;
  }

  cJSON *json = cJSON_Parse(json_string);
  if (!json) {
    fprintf(stderr, "Error: Failed to parse consumables.json\n");
    free(json_string);
    return 0;
  }

  cJSON *items_array = json;
  if (!cJSON_IsArray(items_array)) {
    cJSON_Delete(json);
    free(json_string);
    return 0;
  }

  int array_size = cJSON_GetArraySize(items_array);
  *count = (array_size < max_count) ? array_size : max_count;

  for (int i = 0; i < *count; i++) {
    cJSON *item = cJSON_GetArrayItem(items_array, i);
    if (!item) {
      continue;
    }

    memset(&types[i], 0, sizeof(ItemType));
    load_string_field(item, "name", types[i].name, MAX_ITEM_NAME);
    load_string_field(item, "description", types[i].description, MAX_ITEM_DESC);
    
    // Load glyph (single character)
    cJSON *glyph = cJSON_GetObjectItem(item, "glyph");
    if (glyph && cJSON_IsString(glyph) && glyph->valuestring && strlen(glyph->valuestring) > 0) {
      types[i].glyph = glyph->valuestring[0];
    } else {
      types[i].glyph = '?';
    }
    
    // Load category
    cJSON *category = cJSON_GetObjectItem(item, "category");
    if (category && cJSON_IsString(category) && category->valuestring) {
      types[i].category = parse_item_category(category->valuestring);
    } else {
      types[i].category = ITEM_CATEGORY_CONSUMABLE;
    }
    
    // Load rarity
    cJSON *rarity = cJSON_GetObjectItem(item, "rarity");
    if (rarity && cJSON_IsString(rarity) && rarity->valuestring) {
      types[i].rarity = parse_item_rarity(rarity->valuestring);
    } else {
      types[i].rarity = ITEM_RARITY_COMMON;
    }
    
    load_int_field(item, "hp_restore", &types[i].hp_restore);
    load_int_field(item, "stamina_restore", &types[i].stamina_restore);
    load_int_field(item, "mp_restore", &types[i].mp_restore);
    load_int_field(item, "value", &types[i].value);
    load_int_field(item, "color_r", &types[i].color_r);
    load_int_field(item, "color_g", &types[i].color_g);
    load_int_field(item, "color_b", &types[i].color_b);
  }

  cJSON_Delete(json);
  free(json_string);
  return 1;
}

#else

int data_load_spells(Spell *spells, int max_count, int *count) {
  (void)spells;
  (void)max_count;
  *count = 0;
  return 0;
}

int data_load_equipment_items(EquipmentItem *equipment, int max_count, int *count) {
  (void)equipment;
  (void)max_count;
  *count = 0;
  return 0;
}

int data_load_backpack(BackpackItem *backpack, int max_count, int *count) {
  (void)backpack;
  (void)max_count;
  *count = 0;
  return 0;
}

int data_load_enemies(EnemyType *enemies, int max_count, int *count) {
  (void)enemies;
  (void)max_count;
  *count = 0;
  return 0;
}

int data_load_item_types(ItemType *types, int max_count, int *count) {
  (void)types;
  (void)max_count;
  *count = 0;
  return 0;
}

#endif

