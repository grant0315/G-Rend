# G-Rend Roguelike - Architecture Documentation

## Overview
G-Rend is a turn-based roguelike game built in C with SDL2. The codebase is modular and designed for easy maintenance and extensibility.

## Core Design Principles

### 1. Turn-Based System
- **Player actions** (move, attack, use stairs) consume a turn
- **Enemy actions** happen immediately after player takes a turn
- No real-time movement - everything is discrete and fair
- UI rate limiting (MOVE_INTERVAL_MS) is separate from game logic

### 2. Modularity
The codebase is organized into clear, single-responsibility modules:

```
src/
├── main.c          # Entry point, SDL setup, event loop
├── game.c          # Core game logic, turn system, state management
├── map.c           # Map generation (random walk algorithm)
├── enemy.c         # Enemy types, spawning, combat
├── renderer.c      # All rendering logic (SDL2)
├── data.c          # JSON data loading (items, spells, enemies)
└── save.c          # Save/load game state

include/
├── game.h          # Game state, player, turns
├── map.h           # Map types and generation
├── enemy.h         # Enemy types and functions
├── renderer.h      # Rendering interface
├── data.h          # Data loading interface
└── save.h          # Save/load interface
```

## Module Responsibilities

### `main.c`
- SDL initialization and cleanup
- Main game loop
- Input translation (SDL events → game inputs)
- Mouse click handling
- Window management

### `game.c` - Core Game Logic
- **Turn System**: Manages player and enemy turns
- **State Management**: Start screen, playing, quit states
- **Player Actions**: Movement, combat, stairs
- **Camera**: Follows player, handles viewport
- **Pathfinding**: A* algorithm for mouse movement
- **Menu System**: Spells, Equipment, Backpack
- **Turn Processing**: `try_move_player()` returns if turn was taken
- **Enemy Turn Processing**: `process_enemy_turns()` executes after player

### `map.c` - Map Generation
- Random walk algorithm for organic dungeons
- Tile types: Floor, Wall, Stairs Up/Down
- Map size: 200x100 tiles
- Generates connected walkable areas

### `enemy.c` - Enemy System
- **Enemy Types**: Loaded from JSON or hardcoded fallback
- **Spawning**: Level-appropriate enemy selection
- **Combat**: Damage calculation with defense
- **AI**: Currently in `game.c` for turn-based logic
- **Stats**: HP, attack, defense, XP value, aggro range

### `renderer.c` - Rendering
- SDL2 + SDL_ttf for text rendering
- Layered rendering: Map → Enemies → Player → UI
- **UI Elements**: Title bar, status bar, menu panel, start screen
- Pixelated font support
- Color-coded enemy glyphs

### `data.c` - Data Loading
- JSON parsing via cJSON library
- Loads: Spells, Equipment, Items, Enemies
- Graceful fallback to hardcoded data
- Extensible for adding new data types

### `save.c` - Persistence
- JSON-based save files
- Saves: Player stats, inventory, level, XP
- **Note**: Map is regenerated, not saved
- Full game state serialization

## Data Flow

### Turn System Flow
```
1. Player Input (keyboard/mouse)
   ↓
2. try_move_player() - validates and executes action
   ↓ (returns 1 if turn taken)
3. process_enemy_turns() - each enemy takes one action
   ↓
4. Render updated game state
```

### Enemy Turn Logic
```
For each active enemy:
  1. Check aggro range (detect player)
  2. If adjacent: Attack player
  3. Else if alerted: Move toward player
  4. Simple pathfinding (prefer axis with greater distance)
```

## Key Data Structures

### `GameState`
Central game state containing:
- Player stats and position
- Map (2D tile array)
- Camera position
- Enemies array
- Inventory (spells, equipment, backpack)
- Menu state
- Pathfinding data
- Status messages

### `Enemy`
Instance data:
- Active flag
- Type ID (references EnemyType)
- Position
- Current HP
- Alerted state

### `EnemyType`
Template data (loaded from JSON):
- Name, description, glyph
- Max HP, attack, defense
- XP value
- Aggro range
- Color (RGB)

## Extension Points

### Adding New Enemy Types
1. Add entry to `data/enemies.json`
2. That's it! Auto-loaded on startup

### Adding New Items/Spells
1. Add to respective JSON file
2. Implement use logic in `game.c` (if needed)

### Adding New Game Mechanics
- **New tile types**: Extend `Tile` enum in `map.h`
- **New player actions**: Add to `GameInput` enum in `game.h`
- **New UI elements**: Add rendering function in `renderer.c`

### Improving Enemy AI
- Current: Simple "move toward player" in `game.c`
- Future: Extract to `enemy.c` as `enemy_take_turn()`
- Can add: Pathfinding, abilities, fleeing behavior

## Important Constants

```c
MOVE_INTERVAL_MS = 120      // UI rate limit (not turn speed)
PLAYER_MAX_HP = 100
PLAYER_MAX_STAMINA = 100
PLAYER_MAX_MP = 50
XP_PER_LEVEL = 100
MAX_ENEMIES = 200
MAP_WIDTH = 200
MAP_HEIGHT = 100
TILE_SIZE = 16              // Pixels per tile
```

## Build System

- **Makefile**: Simple, clear dependencies
- **Libraries**: SDL2, SDL2_ttf, cJSON (optional)
- **Fallbacks**: Hardcoded data if JSON unavailable
- **Platform**: MSYS2/MinGW on Windows, adaptable to Linux

## Future Improvements

### Code Quality
- [ ] Extract enemy AI from `game.c` to `enemy.c`
- [ ] Add unit tests for core systems
- [ ] Document all public functions

### Features
- [ ] Enemy special abilities
- [ ] Item use functionality
- [ ] Spell casting system
- [ ] Equipment stats application
- [ ] Permadeath and high scores
- [ ] More map generation algorithms

### Performance
- [ ] Spatial hash for enemy lookups
- [ ] Dirty rectangles for rendering
- [ ] Optimize pathfinding (JPS or similar)

## Debugging Tips

1. **Status messages**: Use `game_set_status_message()` for debugging
2. **Logging**: Add `SDL_Log()` calls in critical paths
3. **Enemy count**: Check with: `grep -c "active.*1" in enemies array`
4. **Turn tracking**: Add counter in `process_enemy_turns()`

## Common Patterns

### Adding a New Input
```c
// 1. Add to GameInput enum (game.h)
GAME_INPUT_NEW_ACTION,

// 2. Map key in translate_key() (main.c)
case SDLK_X:
  return GAME_INPUT_NEW_ACTION;

// 3. Handle in game_handle_input() (game.c)
case GAME_INPUT_NEW_ACTION:
  perform_new_action(game);
  process_enemy_turns(game); // If action takes a turn
  return 0;
```

### Adding a New Render Layer
```c
// Add function in renderer.c
static void draw_new_layer(Renderer *renderer, const GameState *game, ...) {
  // Rendering code
}

// Call in renderer_draw_game() in correct order
draw_map_layer(...);
draw_enemies(...);
draw_new_layer(...);  // Add here
draw_player(...);
```

## Maintainability Checklist

When adding features, ask:
- [ ] Is it in the right module?
- [ ] Does it respect turn-based logic?
- [ ] Are new types in headers?
- [ ] Is data JSON-loadable (not hardcoded)?
- [ ] Does it maintain single responsibility?
- [ ] Is the save system updated if needed?

---

This architecture ensures the codebase remains clean, extensible, and easy to understand for future development.

