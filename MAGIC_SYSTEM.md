# G-Rend Magic System Documentation

## Overview
The G-Rend roguelike features a sophisticated, aimable magic system with visual previews, diverse spell types, and a comprehensive spell codex.

## Spell System Features

### 1. **Spell Codex (Press C or B)**
- View all available spells in the game
- Detailed spell information including:
  - MP cost and level requirements
  - Range and radius (AoE)
  - Targeting type (Self, Single, Area, Cone, Line)
  - Description and effect type
- Bind spells to hotbar slots 1-9 by selecting a spell and pressing the number key
- Cast spells directly from codex with ENTER

### 2. **Spell Hotbar**
- 9 quick-cast slots visible in the right panel
- Press 1-9 to cast bound spells
- Customize your loadout from the spell codex

### 3. **Targeting System**
- **Aimable Spells**: Most spells require targeting
- **Visual Preview**: 
  - Semi-transparent overlay shows affected area
  - Color-coded: Green = valid target, Red = invalid
  - Epicenter highlighted with white border
- **Mouse Tracking**: Preview updates in real-time as you move mouse
- **Click to Cast**: Left-click confirms spell casting
- **ESC to Cancel**: Cancel targeting at any time

### 4. **Spell Types**

#### **Targeting Types:**
- **SELF**: Instant cast on yourself (no targeting needed)
- **SINGLE**: Target a single tile/enemy
- **AREA**: Circular area of effect around target point
- **CONE**: Cone emanating from player toward target
- **LINE**: Straight line from player to target
- **BEAM**: Directional beam (reserved for future use)

#### **Effect Types:**
- **DAMAGE**: Deals damage to enemies in area
- **HEAL**: Restores player HP
- **BUFF**: Temporary stat increases
- **DEBUFF**: Enemy stat decreases
- **TELEPORT**: Move player to target location
- **LIGHT**: Increase vision radius
- **SUMMON**: Spawn creatures (future)
- **KNOCK_BACK**: Push enemies away (future)

### 5. **Default Spell Roster**

#### **1. Fireball** [Area Damage]
- MP: 8 | Range: 8 | Radius: 2
- Explosive fire damage in circular area
- Great for groups of enemies

#### **2. Heal** [Self Heal]
- MP: 5 | Range: Self
- Instantly restore health
- No targeting required

#### **3. Lightning Bolt** [Line Damage]
- MP: 6 | Range: 10
- Strike all enemies in a line
- Penetrates through multiple targets

#### **4. Flame Cone** [Cone AoE]
- MP: 7 | Range: 5
- Cone of fire spreading from you
- Effective for enemies at mid-range

#### **5. Blink** [Teleport]
- MP: 4 | Range: 6
- Instantly teleport to target location
- Must target walkable floor tiles

#### **6. Ice Nova** [Self AoE]
- MP: 10 | Range: Self | Radius: 3
- Freeze all enemies around you
- Powerful defensive spell

#### **7. Magic Missile** [Single Target]
- MP: 3 | Range: 12
- Unerring single-target damage
- Efficient for focused damage

#### **8. Light** [Vision Buff]
- MP: 2 | Range: Self
- Illuminate darkness, increase vision
- Lasts 50 turns

## Controls

### Spell Casting Flow:
1. **Open Spell Codex**: Press `C` or `B`
2. **Browse Spells**: Use arrow keys
3. **Bind to Hotbar**: Press `1-9` to assign to slot
4. **Cast Spell**: Press `1-9` in game
5. **Aim Spell**: Move mouse to target
6. **Confirm Cast**: Left-click when green
7. **Cancel**: Press `ESC`

### Quick Reference:
- `C` or `B`: Open/close Spell Codex
- `1-9`: Cast spell from hotbar
- `Mouse Move`: Aim spell while targeting
- `Left Click`: Confirm spell cast
- `ESC`: Cancel targeting
- `TAB` or `I`: Inventory
- `R`: Rest to restore stamina

## Spell Design Principles

### Visual Feedback
- **Color-coded overlays** show spell effects before casting
- **Real-time preview** shows exactly what will be affected
- **Invalid targets** clearly indicated with red overlay
- **Spell colors** match their elemental/magical type

### Strategic Depth
- **MP management**: Choose spells wisely
- **Range planning**: Position matters
- **AoE positioning**: Maximize damage/minimize waste
- **Spell combos**: Mix damage, utility, and mobility

### Customization
- **Flexible hotbar**: Arrange spells to suit your playstyle
- **8 diverse spells**: Different situations require different tools
- **Quick access**: Muscle memory for 1-9 keys

## Technical Details

### Spell Data Structure
Each spell contains:
- Name and description
- MP cost and level requirement
- Target type and effect type
- Range and radius
- Power and duration
- Pattern array (for complex AoE shapes)
- Visual properties (color, glyph)

### Pattern System
Spells can have custom patterns defined as 2D arrays:
- **Circle patterns**: Radial AoE
- **Cone patterns**: Directional spreading effect
- **Line patterns**: Penetrating attacks
- Extensible for future custom shapes

## Future Enhancements
- More spell types and effects
- Spell progression/upgrades
- Combination spells
- Charged/channeled spells
- Spell cooldowns
- Spell scrolls and discovery
- Elemental interactions

