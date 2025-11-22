# Game Data Files

This directory contains JSON files that define the game's spells, equipment, and items.

## Files

- **spells.json** - Defines available spells
- **equipment.json** - Defines equipment items
- **items.json** - Defines backpack/consumable items

## JSON Format

### spells.json
```json
{
  "spells": [
    {
      "name": "Spell Name",
      "description": "What the spell does",
      "mp_cost": 5
    }
  ]
}
```

### equipment.json
```json
{
  "equipment": [
    {
      "name": "Item Name",
      "description": "Item description",
      "slot": 0
    }
  ]
}
```

### items.json
```json
{
  "items": [
    {
      "name": "Item Name",
      "description": "Item description",
      "quantity": 1
    }
  ]
}
```

## Adding/Editing Items

Simply edit the JSON files with any text editor. The game will automatically load the data when it starts.

### Field Limits
- **name**: Maximum 63 characters
- **description**: Maximum 127 characters
- **mp_cost**: Integer (for spells)
- **slot**: Integer (for equipment)
- **quantity**: Integer (for items, defaults to 1 if not specified)

## Installation

To use JSON data loading, you need to install cJSON:

```bash
pacman -S mingw-w64-ucrt-x86_64-cjson
```

If cJSON is not available, the game will fall back to hardcoded default data.

