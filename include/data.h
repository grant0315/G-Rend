#ifndef DATA_H
#define DATA_H

#include "game.h"
#include "enemy.h"
#include "item.h"

int data_load_spells(Spell *spells, int max_count, int *count);
int data_load_equipment_items(EquipmentItem *equipment, int max_count, int *count);
int data_load_backpack(BackpackItem *backpack, int max_count, int *count);
int data_load_enemies(EnemyType *enemies, int max_count, int *count);
int data_load_item_types(ItemType *types, int max_count, int *count);

#endif /* DATA_H */

