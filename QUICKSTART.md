# G-Rend Roguelike - Quick Start Guide

## Building and Running

```bash
cd /c/Users/grant/OneDrive/documents/github/G-Rend
make clean && make
./build/roguelike
```

## Core Controls

### Movement
- **WASD** or **Arrow Keys**: Move player
- **Click on map**: Move to location (pathfinding)
- Movement automatically opens doors you walk into

### Doors
- **F** or **O**: Manually open adjacent door
- **Walk into door**: Automatically opens it
- **Closed doors** (`+`): Block enemies, require opening
- **Open doors** (`/`): Fully walkable

### Combat
- **Click enemy**: Attack if adjacent, otherwise pathfind to them
- **1-9**: Cast spell from hotbar
- **R**: Rest to restore stamina

### Magic System
1. **C** or **B**: Open Spell Codex
2. **1-9** (in codex): Bind spell to hotbar slot
3. **1-9** (in game): Cast spell
4. **Move mouse**: Aim spell (preview shows affected area)
5. **Left Click**: Confirm and cast spell
6. **ESC**: Cancel targeting

### Inventory
- **TAB** or **I**: Open inventory
- **E**: Pick up items (auto-opens modal when on items)
- **1-5** (in pickup modal): Take specific item
- **D** (in inventory): Drop selected item

### Other
- **ESC**: Cancel/close menus
- **Q**: Quit game
- **F5**: Save game
- **F9**: Load game

## Dungeon Features

### Rooms
- 20-30 rectangular rooms per level
- Connected by L-shaped corridors
- Doors at room entrances

### Doors
- Brown `+` symbol when closed
- Block enemies completely
- Open permanently when used
- Tactical tool for enemy control

### Stairs
- Green `<`: Stairs up (previous level)
- Red `>`: Stairs down (next level)
- Always in rooms, never corridors

## Spell System

### Default Hotbar
- **1**: Fireball (Area damage, 8 tiles, radius 2)
- **2**: Heal (Self-cast, restores HP)
- **3**: Lightning Bolt (Line attack, 10 tiles)

### All Spells
1. **Fireball**: AoE explosion
2. **Heal**: Restore health
3. **Lightning Bolt**: Line damage
4. **Flame Cone**: Cone AoE
5. **Blink**: Teleport
6. **Ice Nova**: Self AoE (radius 3)
7. **Magic Missile**: Single target
8. **Light**: Increase vision

### Spell Casting
- Every spell shows targeting preview
- Green = valid target
- Red = invalid/out of range
- Click to confirm cast
- Spells take MP and a turn

## Tips

### Combat Strategy
- Use doors to control enemy flow
- Kite enemies through corridors
- Cast AoE spells to hit groups
- Rest in safe areas behind doors

### Exploration
- Check each room systematically
- Open all doors for escape routes
- Use Blink to teleport over obstacles
- Light spell reveals more map

### Resource Management
- MP regenerates slowly - use spells wisely
- Stamina no longer costs on movement
- Rest when safe to recover stamina
- Pick up consumables for healing

## Advanced Features

### Tactical Door Usage
- Pull enemies through doorways (chokepoint)
- Retreat and close door behind you (future: closable doors)
- Clear rooms one at a time
- Use open doors as waypoints

### Spell Combos
- Blink away, then Fireball pursuers
- Ice Nova when surrounded
- Lightning Bolt down corridors
- Heal between rooms

### Map Exploration
- Map is 200x100 tiles (much larger than viewport)
- Fog of war shows explored areas
- Vision radius: 8 tiles (12 with Light spell)
- Camera follows player automatically

## Status Bar Info
- **HP**: Health points (die at 0)
- **Stamina**: Combat resource (attacks cost stamina)
- **MP**: Mana points (spells cost MP)
- **Level**: Player level (gain from XP)
- **Gold**: Currency from enemies

## Visual Legend
- `@`: Player (gold)
- `+`: Closed door (brown)
- `/`: Open door (dark brown)
- `#`: Wall (dark gray)
- `.`: Floor (very dark)
- `<`: Stairs up (green)
- `>`: Stairs down (red)
- Letters: Enemies (colored by type)
- Symbols: Items and spells

## Troubleshooting

### Spell Won't Cast
- Check MP (top right panel)
- Verify spell is bound (check hotbar display)
- Ensure target is in range (green preview)
- Open Spell Codex (C) to rebind

### Can't Move
- Check for closed door (`+`) - press F to open
- Enemy blocking path? Click to attack
- In targeting mode? Press ESC to cancel

### Door Won't Open
- Must be adjacent (one tile away)
- Press F or O
- Or just walk into it
- Already open? Look for `/` symbol

## Files and Customization

### Spell Data
- `data/spells.json`: Define custom spells
- `data/README_SPELLS.md`: Spell creation guide
- Modify MP costs, ranges, effects

### Items
- `data/items.json`: Item types
- `data/consumables.json`: Potions and consumables
- `data/equipment.json`: Weapons and armor

### Save Files
- `save.json`: Game state (F5 to save, F9 to load)
- Saves: player stats, inventory, level, position
- Reloads generate new dungeon layout

## Credits
G-Rend Roguelike
- Dungeon generation: Rooms + Corridors algorithm
- Magic system: Targeting with real-time preview
- Interactive doors: Enemy control mechanics

