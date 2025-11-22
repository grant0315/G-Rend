# Debug Instructions

## To rebuild and test:

In your UCRT64/MSYS2 terminal:

```bash
cd /c/Users/grant/onedrive/documents/github/G-Rend
make clean
make
./build/roguelike.exe
```

## What to look for in the terminal output:

1. **Font loading messages** - Will show which fonts succeeded/failed:
   - "Trying: fonts/PressStart2P-Regular.ttf - Failed"
   - "Trying: C:/Windows/Fonts/consola.ttf - Failed"
   - etc.

2. **Game state** - Should show "Game state: 0" (START_SCREEN)

## What you should see on screen:

- **If fonts load**: Beautiful start screen with "G-REND ROGUELIKE" title
- **If NO fonts load**: Red square in center (visual indicator)
- **If partially working**: Colored boxes but no text

## Quick Fix if fonts fail:

The game will still run! Just press **ENTER** to start playing. You'll see colored blocks for the game world even without text.

## To add a proper font:

1. Download a TTF font (like Arial or Consolas)
2. Create a `fonts/` folder in your G-Rend directory
3. Copy any .ttf file there and rename it to `PressStart2P-Regular.ttf`



