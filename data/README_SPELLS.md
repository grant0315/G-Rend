# Spell Data Format

## Field Reference

### Basic Fields
- **name**: Display name of the spell
- **description**: Short description shown in codex
- **mp_cost**: Mana points required to cast
- **level_required**: Minimum player level to use (1 = available from start)

### Targeting Fields
- **target_type**: How the spell is aimed
  - `0` = SELF (cast on yourself, no aiming)
  - `1` = SINGLE (target one tile/enemy)
  - `2` = AREA (circular area of effect)
  - `3` = CONE (spreading cone from player)
  - `4` = LINE (straight line attack)
  - `5` = BEAM (directional beam, reserved)

- **effect_type**: What the spell does
  - `0` = DAMAGE (hurts enemies)
  - `1` = HEAL (restores HP)
  - `2` = BUFF (stat increase)
  - `3` = DEBUFF (enemy stat decrease)
  - `4` = SUMMON (spawn creature)
  - `5` = TELEPORT (move player)
  - `6` = LIGHT (increase vision)
  - `7` = KNOCK_BACK (push enemies)

### Power and Range
- **range**: Maximum distance in tiles (0 = self-cast only)
- **radius**: Area of effect radius in tiles (0 = single tile)
- **power**: Base damage/healing amount
- **duration**: Effect duration in turns (for buffs/debuffs)

### Visual
- **color_r**: Red component (0-255)
- **color_g**: Green component (0-255)
- **color_b**: Blue component (0-255)
- **glyph**: Single character symbol (e.g., "*", "+", "|")

## Example Spells

### Area Damage Spell
```json
{
  "name": "Fireball",
  "target_type": 2,  // AREA
  "effect_type": 0,  // DAMAGE
  "range": 8,
  "radius": 2,
  "power": 30
}
```

### Self-Cast Heal
```json
{
  "name": "Heal",
  "target_type": 0,  // SELF
  "effect_type": 1,  // HEAL
  "range": 0,
  "radius": 0,
  "power": 40
}
```

### Line Attack
```json
{
  "name": "Lightning Bolt",
  "target_type": 4,  // LINE
  "effect_type": 0,  // DAMAGE
  "range": 10,
  "radius": 0,
  "power": 25
}
```

## Tips for Creating New Spells

1. **Balance MP cost** - More powerful spells should cost more MP
2. **Range affects power** - Longer range spells can be slightly weaker
3. **AoE spells** - Should have lower power per target than single-target spells
4. **Color coding** - Use red/orange for fire, blue for ice, white for lightning, etc.
5. **Glyphs** - Choose symbols that represent the spell visually

## Pattern System

Spells with `target_type` AREA, CONE, or LINE automatically generate patterns:
- **AREA**: Circular pattern based on radius
- **CONE**: Expanding cone in direction of target
- **LINE**: Straight line to target

Custom patterns can be defined in code for more complex shapes.

