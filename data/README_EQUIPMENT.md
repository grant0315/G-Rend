# Equipment System Guide

## Overview
Equipment directly affects your combat stats and capabilities in G-Rend Roguelike. The equipment panel is always visible on the right side of the screen.

## Equipment Slots

There are 4 equipment slots:
- **Weapon** (slot 0) - Primary source of attack power and speed
- **Armor** (slot 1) - Main source of defense
- **Accessory** (slot 2) - Rings and amulets with balanced bonuses
- **Boots** (slot 3) - Enhance mobility and dexterity

## Stats Explained

### Attack Power
- Determines base damage dealt to enemies
- Increases with player level
- Boosted primarily by weapons and rings

### Defense
- Reduces incoming damage from enemies
- Even small amounts significantly improve survivability
- Provided mainly by armor, but also boots and accessories

### Dexterity
- Each point = 1% critical hit chance
- Critical hits deal 1.5x damage
- Important for high-damage burst potential

### Speed (Most Complex)
- **100% speed = 1 extra guaranteed attack**
- **200% speed = 2 extra guaranteed attacks**
- **Partial speed = chance for extra attack**

#### Examples:
- 0% speed = 1 attack per turn
- 50% speed = 1 attack + 50% chance for 2nd
- 100% speed = 2 attacks guaranteed
- 150% speed = 2 attacks + 50% chance for 3rd
- 200% speed = 3 attacks guaranteed

## Equipment Rarities

### Common (Gray)
- Basic starting gear
- Low stat bonuses
- Frequently dropped by weak enemies

### Uncommon (Green)
- Modest improvements
- Dropped by medium enemies
- Good early-game upgrades

### Rare (Blue)
- Significant stat boosts
- Dropped by strong enemies
- Notable combat improvements

### Epic (Purple)
- Powerful equipment
- Rare drops from tough enemies
- Game-changing bonuses

### Legendary (Gold)
- Ultimate power
- Extremely rare drops
- Massive stat bonuses
- Often negative trade-offs balanced by huge gains

## Drop System

Equipment drops are based on enemy difficulty:
- **Base chance**: 5% + (enemy_difficulty / 20)
- Harder enemies = better equipment rarity
- Weak enemies: Mostly common items
- Medium enemies: Common/Uncommon
- Strong enemies: Uncommon/Rare/Epic
- Boss-level: Rare/Epic/Legendary

## Equipment Examples

### Weapons
- **Rusty Dagger**: +2 attack, +3 dex, +5% speed (fast weak weapon)
- **Iron Sword**: +5 attack (balanced starter)
- **Rapier of Speed**: +7 attack, +8 dex, +50% speed (combo weapon)
- **Godslayer Blade**: +30 attack, +5 def, +15 dex, +100% speed (legendary)

### Armor
- **Leather Armor**: +3 defense (light protection)
- **Chainmail**: +8 def, -2 dex, -10% speed (trade mobility for protection)
- **Plate Armor**: +15 def, -5 dex, -20% speed (tank build)
- **Aegis of Eternity**: +35 def, +10 dex, +10% speed (legendary - no downsides)

### Boots
- **Worn Boots**: +1 def, +1 dex, +5% speed (minor all-around)
- **Swift Boots**: +2 def, +5 dex, +25% speed (mobility focus)
- **Boots of Haste**: +3 def, +10 dex, +75% speed (near extra attack)
- **Hermes Sandals**: +5 def, +20 dex, +150% speed (legendary speed build)

### Accessories (Rings)
- **Simple Ring**: +1 all stats (starter)
- **Ring of Power**: +4 attack, +2 def, +3 dex, +10% speed (balanced)
- **Ring of the Assassin**: +6 attack, +12 dex, +30% speed (crit build)
- **The One Ring**: +20 attack, +15 def, +25 dex, +100% speed (ultimate power)

## Build Strategies

### Speed Demon
- Focus: Multiple attacks per turn
- Equipment: Rapier, Light Armor, Swift Boots, Speed Ring
- Result: 2-3 attacks per turn, moderate damage per hit

### Tank
- Focus: Maximum survivability
- Equipment: Any Weapon, Plate Armor, Defensive Boots, Titan Ring
- Result: Very high defense, can ignore weak enemies

### Crit Assassin
- Focus: High-damage burst
- Equipment: High-dex weapon, Light Armor, Dex Boots, Assassin Ring
- Result: 50%+ crit chance, massive damage spikes

### Balanced Warrior
- Focus: Well-rounded stats
- Equipment: Longsword, Chainmail, Balanced Boots, Power Ring
- Result: Good at everything, reliable performance

## Tips
1. **Early Game**: Prioritize any equipment over none
2. **Mid Game**: Balance offense and defense based on playstyle
3. **Late Game**: Build synergies (e.g., all speed items for multi-attack)
4. **Heavy Armor**: Great defense but speed penalties hurt DPS
5. **Dexterity**: Very powerful once you stack enough for consistent crits
6. **Speed**: Most complex but potentially highest DPS stat

## Adding Custom Equipment

Edit `data/equipment_items.json`:

```json
{
  "name": "Your Item Name",
  "description": "What it does",
  "slot": 0,
  "rarity": "epic",
  "attack_bonus": 15,
  "defense_bonus": 5,
  "dexterity_bonus": 10,
  "speed_bonus": 50,
  "value": 1000,
  "color_r": 200,
  "color_g": 100,
  "color_b": 255
}
```

**Slot values**: 0=Weapon, 1=Armor, 2=Accessory, 3=Boots
**Rarity values**: "common", "uncommon", "rare", "epic", "legendary"
**Speed bonus**: In percentage points (100 = +100% = 1 extra attack)



