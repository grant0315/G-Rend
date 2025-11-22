# Dungeon Generation System

## Overview
G-Rend uses a room-based dungeon generation algorithm that creates interconnected rooms with corridors and interactive doors.

## Algorithm

### 1. **Room Generation**
- Creates 20-30 rectangular rooms
- Room size: 5x12 tiles (random within range)
- Rooms placed with 3-tile buffer to prevent overlap
- Maximum 500 placement attempts to find valid positions

### 2. **Room Connection**
- Each room connects to the previous room via L-shaped corridors
- Randomly chooses horizontal-first or vertical-first path
- Guarantees all rooms are accessible
- Creates interesting navigation paths

### 3. **Door Placement**
- Automatically detects room entrance points
- Places doors where corridors meet room walls
- Doors use `+` glyph when closed, `/` when open
- Wooden brown color scheme

### 4. **Stairs Placement**
- Up stairs: First room (green `<`)
- Down stairs: Last room (red `>`)
- Always placed inside rooms, never in corridors

## Tile Types

| Tile | Symbol | Color | Description |
|------|--------|-------|-------------|
| Wall | `#` | Dark Gray | Impassable |
| Floor | `.` | Very Dark | Walkable |
| Door (Closed) | `+` | Brown | Blocks enemies, requires interaction |
| Door (Open) | `/` | Dark Brown | Walkable by all |
| Stairs Up | `<` | Green | Ascend level |
| Stairs Down | `>` | Red | Descend level |

## Door Mechanics

### Opening Doors
- **Automatic**: Walk into a closed door - it opens automatically
- **Manual**: Press `F` or `O` while adjacent to a door
- Opening a door **takes a turn** (enemies move after)
- Once opened, doors stay open permanently

### Door Properties
- **Block enemies**: Enemies cannot pass through closed doors
- **Strategic depth**: Use doors to control enemy flow
- **Safe zones**: Close enemies in/out of rooms
- **Tactical retreats**: Open escape routes selectively

## Map Characteristics

### Dimensions
- **Width**: 200 tiles
- **Height**: 100 tiles
- Much larger than visible viewport (encourages exploration)

### Room Distribution
- Rooms scattered throughout map
- Natural clustering with open spaces
- Corridors create interesting paths
- Multiple routes between areas

### Connectivity
- **Guaranteed**: All rooms accessible from start
- **Linear**: Rooms connect in sequence
- **No dead ends**: Every room has entrance/exit
- **Stairs accessible**: Always reachable from spawn

## Gameplay Impact

### Exploration
- Clear room structure encourages systematic exploration
- Doors provide natural waypoints
- Larger map rewards thorough searching

### Combat
- Rooms provide tactical arenas
- Doors control enemy access
- Corridors create chokepoints
- Kiting opportunities in open spaces

### Strategy
- Plan escape routes before engaging
- Use doors to funnel enemies
- Clear rooms methodically
- Doors provide breathing room

## Future Enhancements

Potential improvements to the system:

1. **Special Rooms**
   - Treasure rooms with better loot
   - Monster lairs with tougher enemies
   - Safe rooms with healing fountains

2. **Advanced Door Types**
   - Locked doors requiring keys
   - Secret doors (hidden passages)
   - One-way doors
   - Trapped doors

3. **Room Themes**
   - Libraries, armories, crypts
   - Visual variety
   - Themed loot and enemies

4. **Alternative Algorithms**
   - BSP (Binary Space Partitioning) for more structured dungeons
   - Cellular automata for cave-like areas
   - Prefab room templates
   - Mixed generation styles per level

5. **Environmental Features**
   - Pillars and obstacles in rooms
   - Furniture and decorations
   - Water, lava, chasms
   - Interactive elements

## Technical Details

### Room Structure
```c
typedef struct {
  int x, y;      // Top-left corner
  int w, h;      // Width and height
} Room;
```

### Generation Steps
1. Initialize map with walls
2. Generate random rooms (validate no overlap)
3. Connect rooms with L-shaped corridors
4. Detect and place doors at entrances
5. Place stairs in first/last rooms

### Door Detection
Doors placed where:
- Tile is floor
- Opposite sides are walls
- Adjacent sides are floors
- Within 1 tile of room boundary

This ensures doors appear only at actual entrances, not in corridors.

## Performance
- Generation time: < 10ms typical
- Deterministic with same seed
- Scales well with larger maps
- Minimal memory overhead

## Testing the System
Generate multiple levels to see variety:
1. Use stairs to go down/up
2. Each level generates fresh
3. Room count and layout varies
4. Door placement adapts to structure

