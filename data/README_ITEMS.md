# Consumables and Items System

## Overview
The game uses a loot system where enemies drop items based on their difficulty. Items are automatically picked up when the player walks over them.

## File: `consumables.json`

This file defines all consumable items in the game including:
- **Currency** (Gold Coins)
- **Potions** (Health, Stamina, Mana)
- **Food** (Bread, Cheese, Meat, etc.)

### Item Properties

Each item has the following properties:

```json
{
  "name": "Item Name",
  "description": "What the item does",
  "glyph": "!",
  "category": "potion",
  "rarity": "common",
  "hp_restore": 30,
  "stamina_restore": 0,
  "mp_restore": 0,
  "value": 50,
  "color_r": 255,
  "color_g": 50,
  "color_b": 50
}
```

### Field Descriptions

- **name**: Display name of the item
- **description**: Brief description shown in menus
- **glyph**: Single character displayed on the map (typically `!` for potions, `%` for food, `$` for currency)
- **category**: Type of item - options:
  - `"currency"` - Gold and money
  - `"potion"` - Magical potions with instant effects
  - `"food"` - Edible items that restore stats
  - `"consumable"` - General consumable items
  - `"misc"` - Miscellaneous items
- **rarity**: Affects drop rates - options:
  - `"common"` - Frequently dropped
  - `"uncommon"` - Occasionally dropped
  - `"rare"` - Rarely dropped
  - `"epic"` - Very rarely dropped
  - `"legendary"` - Extremely rare drops
- **hp_restore**: Amount of HP restored when used (0 = none)
- **stamina_restore**: Amount of stamina restored (0 = none)
- **mp_restore**: Amount of MP restored (0 = none)
- **value**: Gold value of the item
- **color_r**, **color_g**, **color_b**: RGB color values (0-255) for rendering

## Loot Drop System

### How Enemies Drop Loot

1. **Gold**: Always drops based on enemy XP value
   - Amount: `xp_value / 5 + random(xp_value / 3)`

2. **Items**: Chance-based on enemy difficulty
   - Weak enemies (difficulty < 20): Drop common items (bread, basic potions)
   - Medium enemies (20-40): Drop uncommon items (cheese, better potions)
   - Strong enemies (40-80): Drop rare items (meat, greater potions)
   - Boss enemies (80+): Can drop any item including legendary

3. **Drop Chance**: `30% + (enemy_difficulty / 10)%`
   - Harder enemies have better loot chances

### Difficulty Calculation
Enemy difficulty = `max_hp + attack_power + defense`

## Usage in Game

### Picking Up Items
- Walk over items on the ground to automatically pick them up
- Gold is added to your gold total
- Other items go to your backpack

### Using Items
1. Press **TAB** to open the menu
2. Navigate to **Backpack**
3. Select an item and press **ENTER** to use it
4. Using an item takes one turn (enemies will act)

### Item Effects
- **Health Potions**: Restore HP (won't overheal)
- **Stamina Potions**: Restore stamina for movement
- **Mana Potions**: Restore MP for spells
- **Food**: Often restores multiple stats at once
- Items won't be consumed if you're already at maximum

## Balancing Tips

### For Easier Gameplay
- Increase `hp_restore`, `stamina_restore`, and `mp_restore` values
- Change rare items to `"common"` rarity
- Increase drop chances in the code

### For Harder Gameplay
- Decrease restoration values
- Make powerful items more rare
- Reduce drop chances
- Increase item values (making them more precious)

### Adding New Items
1. Copy an existing item entry in `consumables.json`
2. Change the name and description
3. Adjust restoration values
4. Set appropriate rarity and category
5. Choose a glyph and color
6. Save the file and restart the game

## Example Custom Item

```json
{
  "name": "Dragon Blood Elixir",
  "description": "Grants immense power",
  "glyph": "!",
  "category": "potion",
  "rarity": "legendary",
  "hp_restore": 100,
  "stamina_restore": 100,
  "mp_restore": 50,
  "value": 1000,
  "color_r": 200,
  "color_g": 0,
  "color_b": 0
}
```

This would create a powerful legendary potion that fully restores all stats!



