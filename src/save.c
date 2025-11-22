#include "../include/save.h"
#include "../include/data.h"
#include "../include/map.h"
#include "../include/sdl_platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __has_include
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

static cJSON *save_player(const Player *player) {
  cJSON *json = cJSON_CreateObject();
  cJSON_AddNumberToObject(json, "x", player->x);
  cJSON_AddNumberToObject(json, "y", player->y);
  cJSON_AddNumberToObject(json, "hp", player->hp);
  cJSON_AddNumberToObject(json, "stamina", player->stamina);
  cJSON_AddNumberToObject(json, "mp", player->mp);
  cJSON_AddNumberToObject(json, "xp", player->xp);
  cJSON_AddNumberToObject(json, "level", player->level);
  return json;
}

static cJSON *save_spells(const Spell *spells, int count) {
  cJSON *array = cJSON_CreateArray();
  for (int i = 0; i < count; i++) {
    cJSON *spell = cJSON_CreateObject();
    cJSON_AddStringToObject(spell, "name", spells[i].name);
    cJSON_AddStringToObject(spell, "description", spells[i].description);
    cJSON_AddNumberToObject(spell, "mp_cost", spells[i].mp_cost);
    cJSON_AddItemToArray(array, spell);
  }
  return array;
}

static cJSON *save_equipment(const EquipmentItem *equipment, int count) {
  cJSON *array = cJSON_CreateArray();
  for (int i = 0; i < count; i++) {
    cJSON *item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "name", equipment[i].name);
    cJSON_AddStringToObject(item, "description", equipment[i].description);
    cJSON_AddNumberToObject(item, "slot", equipment[i].slot);
    cJSON_AddItemToArray(array, item);
  }
  return array;
}

static cJSON *save_backpack(const BackpackItem *backpack, int count) {
  cJSON *array = cJSON_CreateArray();
  for (int i = 0; i < count; i++) {
    cJSON *item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "name", backpack[i].name);
    cJSON_AddStringToObject(item, "description", backpack[i].description);
    cJSON_AddNumberToObject(item, "quantity", backpack[i].quantity);
    cJSON_AddItemToArray(array, item);
  }
  return array;
}

int save_game(const GameState *game, const char *filename) {
  cJSON *json = cJSON_CreateObject();
  
  cJSON_AddNumberToObject(json, "state", game->state);
  cJSON_AddNumberToObject(json, "current_level", game->current_level);
  cJSON_AddNumberToObject(json, "cam_x", game->cam_x);
  cJSON_AddNumberToObject(json, "cam_y", game->cam_y);
  
  cJSON *player = save_player(&game->player);
  cJSON_AddItemToObject(json, "player", player);
  
  cJSON *spells = save_spells(game->spells, game->spell_count);
  cJSON_AddItemToObject(json, "spells", spells);
  
  // Save spell hotbar
  cJSON *hotbar = cJSON_CreateIntArray(game->spell_hotbar, MAX_SPELL_HOTBAR);
  cJSON_AddItemToObject(json, "spell_hotbar", hotbar);
  
  cJSON *equipment = save_equipment(game->equipment, game->equipment_count);
  cJSON_AddItemToObject(json, "equipment", equipment);
  
  cJSON *backpack = save_backpack(game->backpack, game->backpack_count);
  cJSON_AddItemToObject(json, "backpack", backpack);
  
  char *json_string = cJSON_Print(json);
  if (!json_string) {
    cJSON_Delete(json);
    return 0;
  }
  
  FILE *f = fopen(filename, "w");
  if (!f) {
    free(json_string);
    cJSON_Delete(json);
    return 0;
  }
  
  fprintf(f, "%s", json_string);
  fclose(f);
  
  free(json_string);
  cJSON_Delete(json);
  return 1;
}

static int load_player(cJSON *json, Player *player) {
  cJSON *item;
  
  item = cJSON_GetObjectItem(json, "x");
  if (cJSON_IsNumber(item)) player->x = cJSON_GetNumberValue(item);
  
  item = cJSON_GetObjectItem(json, "y");
  if (cJSON_IsNumber(item)) player->y = cJSON_GetNumberValue(item);
  
  item = cJSON_GetObjectItem(json, "hp");
  if (cJSON_IsNumber(item)) player->hp = cJSON_GetNumberValue(item);
  
  item = cJSON_GetObjectItem(json, "stamina");
  if (cJSON_IsNumber(item)) player->stamina = cJSON_GetNumberValue(item);
  
  item = cJSON_GetObjectItem(json, "mp");
  if (cJSON_IsNumber(item)) player->mp = cJSON_GetNumberValue(item);
  
  item = cJSON_GetObjectItem(json, "xp");
  if (cJSON_IsNumber(item)) player->xp = cJSON_GetNumberValue(item);
  
  item = cJSON_GetObjectItem(json, "level");
  if (cJSON_IsNumber(item)) player->level = cJSON_GetNumberValue(item);
  
  return 1;
}

static int load_spells(cJSON *array, Spell *spells, int max_count, int *count) {
  if (!cJSON_IsArray(array)) {
    return 0;
  }
  
  int array_size = cJSON_GetArraySize(array);
  *count = (array_size < max_count) ? array_size : max_count;
  
  for (int i = 0; i < *count; i++) {
    cJSON *item = cJSON_GetArrayItem(array, i);
    if (!item) continue;
    
    cJSON *name = cJSON_GetObjectItem(item, "name");
    if (cJSON_IsString(name)) {
      strncpy(spells[i].name, cJSON_GetStringValue(name), MAX_NAME_LENGTH - 1);
      spells[i].name[MAX_NAME_LENGTH - 1] = '\0';
    }
    
    cJSON *desc = cJSON_GetObjectItem(item, "description");
    if (cJSON_IsString(desc)) {
      strncpy(spells[i].description, cJSON_GetStringValue(desc), MAX_DESC_LENGTH - 1);
      spells[i].description[MAX_DESC_LENGTH - 1] = '\0';
    }
    
    cJSON *mp_cost = cJSON_GetObjectItem(item, "mp_cost");
    if (cJSON_IsNumber(mp_cost)) {
      spells[i].mp_cost = cJSON_GetNumberValue(mp_cost);
    }
  }
  
  return 1;
}

static int load_equipment(cJSON *array, EquipmentItem *equipment, int max_count, int *count) {
  if (!cJSON_IsArray(array)) {
    return 0;
  }
  
  int array_size = cJSON_GetArraySize(array);
  *count = (array_size < max_count) ? array_size : max_count;
  
  for (int i = 0; i < *count; i++) {
    cJSON *item = cJSON_GetArrayItem(array, i);
    if (!item) continue;
    
    cJSON *name = cJSON_GetObjectItem(item, "name");
    if (cJSON_IsString(name)) {
      strncpy(equipment[i].name, cJSON_GetStringValue(name), MAX_NAME_LENGTH - 1);
      equipment[i].name[MAX_NAME_LENGTH - 1] = '\0';
    }
    
    cJSON *desc = cJSON_GetObjectItem(item, "description");
    if (cJSON_IsString(desc)) {
      strncpy(equipment[i].description, cJSON_GetStringValue(desc), MAX_DESC_LENGTH - 1);
      equipment[i].description[MAX_DESC_LENGTH - 1] = '\0';
    }
    
    cJSON *slot = cJSON_GetObjectItem(item, "slot");
    if (cJSON_IsNumber(slot)) {
      equipment[i].slot = cJSON_GetNumberValue(slot);
    }
  }
  
  return 1;
}

static int load_backpack(cJSON *array, BackpackItem *backpack, int max_count, int *count) {
  if (!cJSON_IsArray(array)) {
    return 0;
  }
  
  int array_size = cJSON_GetArraySize(array);
  *count = (array_size < max_count) ? array_size : max_count;
  
  for (int i = 0; i < *count; i++) {
    cJSON *item = cJSON_GetArrayItem(array, i);
    if (!item) continue;
    
    cJSON *name = cJSON_GetObjectItem(item, "name");
    if (cJSON_IsString(name)) {
      strncpy(backpack[i].name, cJSON_GetStringValue(name), MAX_NAME_LENGTH - 1);
      backpack[i].name[MAX_NAME_LENGTH - 1] = '\0';
    }
    
    cJSON *desc = cJSON_GetObjectItem(item, "description");
    if (cJSON_IsString(desc)) {
      strncpy(backpack[i].description, cJSON_GetStringValue(desc), MAX_DESC_LENGTH - 1);
      backpack[i].description[MAX_DESC_LENGTH - 1] = '\0';
    }
    
    cJSON *qty = cJSON_GetObjectItem(item, "quantity");
    if (cJSON_IsNumber(qty)) {
      backpack[i].quantity = cJSON_GetNumberValue(qty);
    } else {
      backpack[i].quantity = 1;
    }
  }
  
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

int load_game(GameState *game, const char *filename) {
  char *json_string = read_file(filename);
  if (!json_string) {
    return 0;
  }

  cJSON *json = cJSON_Parse(json_string);
  if (!json) {
    free(json_string);
    return 0;
  }

  cJSON *item;
  
  item = cJSON_GetObjectItem(json, "state");
  if (cJSON_IsNumber(item)) {
    game->state = cJSON_GetNumberValue(item);
  }
  
  item = cJSON_GetObjectItem(json, "current_level");
  if (cJSON_IsNumber(item)) {
    game->current_level = cJSON_GetNumberValue(item);
  }
  
  item = cJSON_GetObjectItem(json, "cam_x");
  if (cJSON_IsNumber(item)) {
    game->cam_x = cJSON_GetNumberValue(item);
  }
  
  item = cJSON_GetObjectItem(json, "cam_y");
  if (cJSON_IsNumber(item)) {
    game->cam_y = cJSON_GetNumberValue(item);
  }
  
  cJSON *player_json = cJSON_GetObjectItem(json, "player");
  if (player_json) {
    load_player(player_json, &game->player);
  }
  
  cJSON *spells_json = cJSON_GetObjectItem(json, "spells");
  if (spells_json) {
    load_spells(spells_json, game->spells, MAX_SPELLS, &game->spell_count);
  }
  
  // Load spell hotbar
  cJSON *hotbar_json = cJSON_GetObjectItem(json, "spell_hotbar");
  if (hotbar_json && cJSON_IsArray(hotbar_json)) {
    int size = cJSON_GetArraySize(hotbar_json);
    for (int i = 0; i < size && i < MAX_SPELL_HOTBAR; i++) {
      cJSON *item = cJSON_GetArrayItem(hotbar_json, i);
      if (cJSON_IsNumber(item)) {
        game->spell_hotbar[i] = item->valueint;
      }
    }
  }
  
  cJSON *equipment_json = cJSON_GetObjectItem(json, "equipment");
  if (equipment_json) {
    load_equipment(equipment_json, game->equipment, MAX_EQUIPMENT, &game->equipment_count);
  }
  
  cJSON *backpack_json = cJSON_GetObjectItem(json, "backpack");
  if (backpack_json) {
    load_backpack(backpack_json, game->backpack, MAX_BACKPACK, &game->backpack_count);
  }
  
  cJSON_Delete(json);
  free(json_string);
  
  if (game->state == GAME_STATE_PLAYING) {
    generate_level(game->map);
    // Ensure player is on walkable tile
    for (int y = 0; y < MAP_HEIGHT; y++) {
      for (int x = 0; x < MAP_WIDTH; x++) {
        Tile tile = game->map[y][x];
        if (tile == TILE_FLOOR || tile == TILE_STAIRS_UP || tile == TILE_STAIRS_DOWN) {
          if (x == game->player.x && y == game->player.y) {
            goto player_ok;
          }
        }
      }
    }
    // Player not on walkable tile, find first walkable tile
    for (int y = 0; y < MAP_HEIGHT; y++) {
      for (int x = 0; x < MAP_WIDTH; x++) {
        Tile tile = game->map[y][x];
        if (tile == TILE_FLOOR || tile == TILE_STAIRS_UP || tile == TILE_STAIRS_DOWN) {
          game->player.x = x;
          game->player.y = y;
          goto player_ok;
        }
      }
    }
    player_ok:;
  }
  
  game->path_length = 0;
  game->path_index = 0;
  game->menu_type = MENU_NONE;
  game->menu_selection = 0;
  
  return 1;
}

#else

int save_game(const GameState *game, const char *filename) {
  (void)game;
  (void)filename;
  return 0;
}

int load_game(GameState *game, const char *filename) {
  (void)game;
  (void)filename;
  return 0;
}

#endif

