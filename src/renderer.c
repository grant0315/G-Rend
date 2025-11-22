#include "../include/renderer.h"
#include "../include/game.h"
#include "../include/enemy.h"
#include "../include/item.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static SDL_Color get_tile_color(Tile tile) {
  switch (tile) {
  case TILE_WALL:
    return (SDL_Color){60, 60, 60, 255};
  case TILE_FLOOR:
    return (SDL_Color){40, 40, 40, 255};
  case TILE_STAIRS_UP:
    return (SDL_Color){100, 200, 100, 255};
  case TILE_STAIRS_DOWN:
    return (SDL_Color){200, 100, 100, 255};
  case TILE_DOOR_CLOSED:
    return (SDL_Color){139, 90, 43, 255}; // Brown/wood color
  case TILE_DOOR_OPEN:
    return (SDL_Color){90, 60, 30, 255}; // Darker brown
  default:
    return (SDL_Color){80, 80, 80, 255};
  }
}

static char get_tile_glyph(Tile tile) {
  switch (tile) {
  case TILE_WALL:
    return '#';
  case TILE_FLOOR:
    return '.';
  case TILE_STAIRS_UP:
    return '<';
  case TILE_STAIRS_DOWN:
    return '>';
  case TILE_DOOR_CLOSED:
    return '+';
  case TILE_DOOR_OPEN:
    return '/';
  default:
    return '?';
  }
}

static void render_text(Renderer *renderer, int x, int y, const char *text,
                        SDL_Color color) {
  if (!renderer->font) {
    return;
  }

  SDL_Surface *surface = TTF_RenderText_Solid(renderer->font, text, color);
  if (!surface) {
    return;
  }

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer->renderer, surface);
  if (!texture) {
    SDL_FreeSurface(surface);
    return;
  }

  SDL_Rect dst = {x, y, surface->w, surface->h};
  SDL_RenderCopy(renderer->renderer, texture, NULL, &dst);

  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
}

static void render_text_centered(Renderer *renderer, int center_x, int y,
                                  const char *text, SDL_Color color) {
  if (!renderer->font) {
    return;
  }

  SDL_Surface *surface = TTF_RenderText_Solid(renderer->font, text, color);
  if (!surface) {
    return;
  }

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer->renderer, surface);
  if (!texture) {
    SDL_FreeSurface(surface);
    return;
  }

  SDL_Rect dst = {center_x - surface->w / 2, y, surface->w, surface->h};
  SDL_RenderCopy(renderer->renderer, texture, NULL, &dst);

  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
}

static void render_title_text(Renderer *renderer, int center_x, int y,
                               const char *text, SDL_Color color) {
  if (!renderer->title_font) {
    render_text_centered(renderer, center_x, y, text, color);
    return;
  }

  SDL_Surface *surface = TTF_RenderText_Solid(renderer->title_font, text, color);
  if (!surface) {
    return;
  }

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer->renderer, surface);
  if (!texture) {
    SDL_FreeSurface(surface);
    return;
  }

  SDL_Rect dst = {center_x - surface->w / 2, y, surface->w, surface->h};
  SDL_RenderCopy(renderer->renderer, texture, NULL, &dst);

  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);
}

static void draw_map_layer(Renderer *renderer, const GameState *game,
                           int tiles_w, int tiles_h, int tile_size) {
  for (int ty = 0; ty < tiles_h; ty++) {
    for (int tx = 0; tx < tiles_w; tx++) {
      int mx = game->cam_x + tx;
      int my = game->cam_y + ty;

      if (mx < 0 || mx >= MAP_WIDTH || my < 0 || my >= MAP_HEIGHT) {
        continue;
      }

      FogState fog = game->fog[my][mx];
      
      // Skip unexplored tiles
      if (fog == FOG_UNEXPLORED) {
        SDL_SetRenderDrawColor(renderer->renderer, 0, 0, 0, 255);
        int px = tx * tile_size;
        int py = ty * tile_size + TITLE_BAR_HEIGHT;
        SDL_Rect dst = {px, py, tile_size, tile_size};
        SDL_RenderFillRect(renderer->renderer, &dst);
        continue;
      }

      Tile tile = game->map[my][mx];
      SDL_Color bg_color = get_tile_color(tile);
      
      // Dim explored but not visible tiles
      if (fog == FOG_EXPLORED) {
        bg_color.r = bg_color.r / 3;
        bg_color.g = bg_color.g / 3;
        bg_color.b = bg_color.b / 3;
      }
      
      SDL_SetRenderDrawColor(renderer->renderer, bg_color.r, bg_color.g,
                             bg_color.b, 255);

      int px = tx * tile_size;
      int py = ty * tile_size + TITLE_BAR_HEIGHT;
      SDL_Rect dst = {px, py, tile_size, tile_size};
      SDL_RenderFillRect(renderer->renderer, &dst);

      // Draw glyphs for special tiles on visible tiles
      if (fog == FOG_VISIBLE && 
          (tile == TILE_STAIRS_UP || tile == TILE_STAIRS_DOWN || 
           tile == TILE_DOOR_CLOSED || tile == TILE_DOOR_OPEN)) {
        char glyph = get_tile_glyph(tile);
        char glyph_str[2] = {glyph, '\0'};
        SDL_Color fg_color = {220, 220, 220, 255};
        if (tile == TILE_DOOR_CLOSED) {
          fg_color = (SDL_Color){255, 200, 100, 255}; // Bright for closed door
        }
        int text_offset = tile_size / 8;
        render_text(renderer, px + text_offset, py + text_offset, glyph_str, fg_color);
      }
    }
  }
}

static void draw_enemies(Renderer *renderer, const GameState *game, int tiles_w,
                         int tiles_h, int tile_size) {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    const Enemy *enemy = &game->enemies[i];
    if (!enemy->active) {
      continue;
    }

    int rel_x = enemy->x - game->cam_x;
    int rel_y = enemy->y - game->cam_y;

    if (rel_x < 0 || rel_x >= tiles_w || rel_y < 0 || rel_y >= tiles_h) {
      continue;
    }
    
    // Only show if visible in fog of war
    FogState fog = game->fog[enemy->y][enemy->x];
    if (fog != FOG_VISIBLE) {
      continue;
    }

    const EnemyType *type = enemy_get_type(enemy->type_id);
    if (!type) {
      continue;
    }

    int px = rel_x * tile_size;
    int py = rel_y * tile_size + TITLE_BAR_HEIGHT;
    SDL_Color enemy_color = {type->color_r, type->color_g, type->color_b, 255};
    
    char glyph_str[2] = {type->glyph, '\0'};
    int text_offset = tile_size / 8;
    render_text(renderer, px + text_offset, py + text_offset, glyph_str, enemy_color);
  }
}

static void draw_ground_items(Renderer *renderer, const GameState *game, int tiles_w,
                               int tiles_h, int tile_size) {
  for (int i = 0; i < MAX_GROUND_ITEMS; i++) {
    const GroundItem *item = &game->ground_items[i];
    if (!item->active) {
      continue;
    }

    int rel_x = item->x - game->cam_x;
    int rel_y = item->y - game->cam_y;

    if (rel_x < 0 || rel_x >= tiles_w || rel_y < 0 || rel_y >= tiles_h) {
      continue;
    }
    
    // Only show if visible in fog of war
    FogState fog = game->fog[item->y][item->x];
    if (fog != FOG_VISIBLE) {
      continue;
    }

    const ItemType *type = item_get_type(item->type_id);
    if (!type) {
      continue;
    }

    int px = rel_x * tile_size;
    int py = rel_y * tile_size + TITLE_BAR_HEIGHT;
    SDL_Color item_color = {type->color_r, type->color_g, type->color_b, 255};
    
    char glyph_str[2] = {type->glyph, '\0'};
    int text_offset = tile_size / 8;
    render_text(renderer, px + text_offset, py + text_offset, glyph_str, item_color);
  }
}

static void draw_player(Renderer *renderer, const GameState *game, int tiles_w,
                        int tiles_h, int tile_size) {
  int rel_x = game->player.x - game->cam_x;
  int rel_y = game->player.y - game->cam_y;

  if (rel_x < 0 || rel_x >= tiles_w || rel_y < 0 || rel_y >= tiles_h) {
    return;
  }

  int px = rel_x * tile_size;
  int py = rel_y * tile_size + TITLE_BAR_HEIGHT;

  SDL_SetRenderDrawColor(renderer->renderer, 255, 215, 0, 255);
  SDL_Rect rect = {px, py, tile_size, tile_size};
  SDL_RenderFillRect(renderer->renderer, &rect);

  SDL_Color fg_color = {0, 0, 0, 255};
  int text_offset = tile_size / 4;
  render_text(renderer, px + text_offset, py + text_offset / 2, "@", fg_color);
}

static void draw_minimap(Renderer *renderer, const GameState *game, int window_w, int window_h) {
  (void)window_h; // Unused
  
  // Mini-map dimensions and position
  int minimap_size = 200; // 200x200 pixel minimap
  int minimap_x = 10;
  int minimap_y = TITLE_BAR_HEIGHT + 10;
  
  // Calculate scale: how many map tiles fit in minimap
  float scale_x = (float)minimap_size / MAP_WIDTH;
  float scale_y = (float)minimap_size / MAP_HEIGHT;
  
  // Background
  SDL_SetRenderDrawBlendMode(renderer->renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer->renderer, 0, 0, 0, 180);
  SDL_Rect bg = {minimap_x - 2, minimap_y - 2, minimap_size + 4, minimap_size + 4};
  SDL_RenderFillRect(renderer->renderer, &bg);
  
  // Border
  SDL_SetRenderDrawColor(renderer->renderer, 100, 100, 120, 255);
  SDL_RenderDrawRect(renderer->renderer, &bg);
  
  // Draw explored tiles
  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      FogState fog = game->fog[y][x];
      
      // Skip unexplored tiles
      if (fog == FOG_UNEXPLORED) {
        continue;
      }
      
      Tile tile = game->map[y][x];
      int px = minimap_x + (int)(x * scale_x);
      int py = minimap_y + (int)(y * scale_y);
      int pw = (int)(scale_x) + 1;
      int ph = (int)(scale_y) + 1;
      
      SDL_Color color;
      if (fog == FOG_EXPLORED) {
        // Dimmed for explored but not visible
        if (tile == TILE_WALL) {
          color = (SDL_Color){40, 40, 40, 255};
        } else if (tile == TILE_DOOR_CLOSED) {
          color = (SDL_Color){70, 45, 20, 255};
        } else if (tile == TILE_DOOR_OPEN) {
          color = (SDL_Color){50, 30, 15, 255};
        } else {
          color = (SDL_Color){60, 60, 60, 255}; // Floor
        }
      } else {
        // Bright for currently visible
        if (tile == TILE_WALL) {
          color = (SDL_Color){80, 80, 80, 255};
        } else if (tile == TILE_DOOR_CLOSED) {
          color = (SDL_Color){139, 90, 43, 255};
        } else if (tile == TILE_DOOR_OPEN) {
          color = (SDL_Color){90, 60, 30, 255};
        } else if (tile == TILE_STAIRS_UP) {
          color = (SDL_Color){100, 200, 100, 255};
        } else if (tile == TILE_STAIRS_DOWN) {
          color = (SDL_Color){200, 100, 100, 255};
        } else {
          color = (SDL_Color){120, 120, 120, 255}; // Floor
        }
      }
      
      SDL_SetRenderDrawColor(renderer->renderer, color.r, color.g, color.b, 255);
      SDL_Rect tile_rect = {px, py, pw, ph};
      SDL_RenderFillRect(renderer->renderer, &tile_rect);
    }
  }
  
  // Draw player position (bright marker)
  int player_x = minimap_x + (int)(game->player.x * scale_x);
  int player_y = minimap_y + (int)(game->player.y * scale_y);
  int player_size = 3;
  
  SDL_SetRenderDrawColor(renderer->renderer, 255, 215, 0, 255); // Gold
  SDL_Rect player_rect = {player_x - player_size/2, player_y - player_size/2, player_size, player_size};
  SDL_RenderFillRect(renderer->renderer, &player_rect);
  
  SDL_SetRenderDrawBlendMode(renderer->renderer, SDL_BLENDMODE_NONE);
}

static void draw_title_bar(Renderer *renderer, const GameState *game,
                           int window_w) {
  SDL_Rect bar = {0, 0, window_w, TITLE_BAR_HEIGHT};
  SDL_SetRenderDrawColor(renderer->renderer, 20, 20, 30, 255);
  SDL_RenderFillRect(renderer->renderer, &bar);

  SDL_Color text_color = {255, 255, 255, 255};
  SDL_Color gold_color = {255, 215, 0, 255};
  char buf[128];
  snprintf(buf, sizeof(buf), "Dungeon %d | XP: %d", game->current_level,
           game->player.xp);
  render_text(renderer, 8, 4, buf, text_color);
  
  // Show gold in top right
  snprintf(buf, sizeof(buf), "Gold: %d", game->player.gold);
  render_text(renderer, window_w - 150, 4, buf, gold_color);
}

static void draw_status_bar(Renderer *renderer, const GameState *game,
                            int window_w, int window_h) {
  int bar_y = window_h - STATUS_BAR_HEIGHT;
  SDL_Rect bar = {0, bar_y, window_w, STATUS_BAR_HEIGHT};
  SDL_SetRenderDrawColor(renderer->renderer, 20, 20, 30, 255);
  SDL_RenderFillRect(renderer->renderer, &bar);

  // Status messages or targeting hints
  if (game->targeting_mode == TARGETING_SPELL) {
    // Show prominent targeting instruction
    SDL_Color target_color = {255, 215, 0, 255};
    const char *hint = ">>> TARGETING MODE: Move mouse to aim, LEFT CLICK to cast, ESC to cancel <<<";
    render_text(renderer, 10, bar_y + 6, hint, target_color);
  } else if (game->status_message[0] != '\0' && 
      (SDL_GetTicks() - game->status_message_time) < 3000) {
    SDL_Color msg_color = {100, 255, 100, 255};
    render_text(renderer, 10, bar_y + 6, game->status_message, msg_color);
  } else {
    // Show helpful hints when no message
    SDL_Color hint_color = {120, 120, 140, 255};
    const char *hint = "C: Codex | TAB: Inventory | F: Open Door | 1-9: Spells | R: Rest";
    render_text(renderer, 10, bar_y + 6, hint, hint_color);
  }
}

void renderer_draw_start_screen(Renderer *renderer, int window_w,
                                 int window_h) {
  SDL_SetRenderDrawColor(renderer->renderer, 15, 15, 25, 255);
  SDL_RenderClear(renderer->renderer);
  
  // If no font loaded, show a simple visual indicator
  if (!renderer->font && !renderer->title_font) {
    SDL_SetRenderDrawColor(renderer->renderer, 255, 0, 0, 255);
    SDL_Rect indicator = {window_w / 2 - 50, window_h / 2 - 50, 100, 100};
    SDL_RenderFillRect(renderer->renderer, &indicator);
    SDL_Log("ERROR: No fonts loaded! Press ENTER to start anyway.");
    return;
  }

  int center_x = window_w / 2;
  int center_y = window_h / 2;

  SDL_Color title_color = {255, 215, 0, 255};
  SDL_Color subtitle_color = {180, 180, 200, 255};
  SDL_Color text_color = {200, 200, 220, 255};
  SDL_Color accent_color = {100, 150, 255, 255};
  SDL_Color border_color = {60, 60, 80, 255};

  int box_width = 500;
  int box_height = 400;
  int box_x = center_x - box_width / 2;
  int box_y = center_y - box_height / 2;

  SDL_Rect main_box = {box_x, box_y, box_width, box_height};
  SDL_SetRenderDrawColor(renderer->renderer, 25, 25, 35, 255);
  SDL_RenderFillRect(renderer->renderer, &main_box);

  SDL_SetRenderDrawColor(renderer->renderer, border_color.r, border_color.g,
                         border_color.b, 255);
  SDL_Rect border_rect = {box_x - 2, box_y - 2, box_width + 4, box_height + 4};
  SDL_RenderDrawRect(renderer->renderer, &border_rect);

  int inner_padding = 40;
  int y = box_y + inner_padding;

  render_title_text(renderer, center_x, y, "G-REND", title_color);
  y += 50;
  render_title_text(renderer, center_x, y, "ROGUELIKE", title_color);
  y += 80;

  SDL_SetRenderDrawColor(renderer->renderer, accent_color.r, accent_color.g,
                         accent_color.b, 100);
  SDL_Rect divider = {box_x + 50, y, box_width - 100, 2};
  SDL_RenderFillRect(renderer->renderer, &divider);
  y += 40;

  render_text_centered(renderer, center_x, y, "CONTROLS", subtitle_color);
  y += 40;

  render_text_centered(renderer, center_x, y, "Movement: WASD / Arrow Keys", text_color);
  y += 28;
  render_text_centered(renderer, center_x, y, "Movement: Click on map", text_color);
  y += 28;
  render_text_centered(renderer, center_x, y, "Stairs: < Up  > Down", text_color);
  y += 50;

  SDL_SetRenderDrawColor(renderer->renderer, accent_color.r, accent_color.g,
                         accent_color.b, 100);
  SDL_Rect divider2 = {box_x + 50, y, box_width - 100, 2};
  SDL_RenderFillRect(renderer->renderer, &divider2);
  y += 40;

  SDL_Color start_color = {100, 255, 100, 255};
  render_text_centered(renderer, center_x, y, "PRESS ENTER TO START", start_color);
  y += 35;
  SDL_Color load_color = {150, 200, 255, 255};
  render_text_centered(renderer, center_x, y, "Press F9 to Load Game", load_color);
  y += 35;
  SDL_Color quit_color = {255, 150, 150, 255};
  render_text_centered(renderer, center_x, y, "Press Q or ESC to Quit", quit_color);
}


static void draw_resource_bar(Renderer *renderer, int x, int y, int width, int height,
                               int current, int max, SDL_Color bar_color, SDL_Color bg_color) {
  // Background
  SDL_SetRenderDrawColor(renderer->renderer, bg_color.r, bg_color.g, bg_color.b, 255);
  SDL_Rect bg = {x, y, width, height};
  SDL_RenderFillRect(renderer->renderer, &bg);
  
  // Foreground (filled portion)
  if (current > 0 && max > 0) {
    int filled_width = (width * current) / max;
    if (filled_width > width) filled_width = width;
    SDL_SetRenderDrawColor(renderer->renderer, bar_color.r, bar_color.g, bar_color.b, 255);
    SDL_Rect fg = {x, y, filled_width, height};
    SDL_RenderFillRect(renderer->renderer, &fg);
  }
  
  // Border
  SDL_SetRenderDrawColor(renderer->renderer, 80, 80, 90, 255);
  SDL_RenderDrawRect(renderer->renderer, &bg);
}

static void draw_side_panel(Renderer *renderer, const GameState *game,
                             int window_w, int window_h) {
  int panel_x = window_w - SIDE_PANEL_WIDTH;
  int panel_y = TITLE_BAR_HEIGHT;
  int panel_h = window_h - TITLE_BAR_HEIGHT - STATUS_BAR_HEIGHT;

  SDL_Rect panel = {panel_x, panel_y, SIDE_PANEL_WIDTH, panel_h};
  SDL_SetRenderDrawColor(renderer->renderer, 25, 25, 35, 255);
  SDL_RenderFillRect(renderer->renderer, &panel);

  SDL_SetRenderDrawColor(renderer->renderer, 60, 60, 80, 255);
  SDL_Rect border = {panel_x - 2, panel_y, 2, panel_h};
  SDL_RenderFillRect(renderer->renderer, &border);

  SDL_Color title_color = {255, 215, 0, 255};
  SDL_Color text_color = {220, 220, 230, 255};
  SDL_Color stat_color = {100, 255, 100, 255};
  SDL_Color empty_color = {100, 100, 100, 255};
  SDL_Color hp_color = {255, 50, 50, 255};
  SDL_Color stamina_color = {50, 255, 50, 255};
  SDL_Color mp_color = {50, 150, 255, 255};
  SDL_Color bar_bg = {40, 40, 50, 255};

  int padding = 12;
  int y = panel_y + padding;
  int line_height = 20;
  int bar_height = 16;
  int bar_width = SIDE_PANEL_WIDTH - padding * 2;

  // ====== STATS SECTION ======
  render_text(renderer, panel_x + padding, y, "PLAYER STATS", title_color);
  y += line_height + 6;

  // HP Bar
  char buf[64];
  snprintf(buf, sizeof(buf), "HP: %d/%d", game->player.hp, PLAYER_MAX_HP);
  render_text(renderer, panel_x + padding, y, buf, text_color);
  y += line_height;
  draw_resource_bar(renderer, panel_x + padding, y, bar_width, bar_height,
                    game->player.hp, PLAYER_MAX_HP, hp_color, bar_bg);
  y += bar_height + 8;

  // Stamina Bar
  snprintf(buf, sizeof(buf), "Stamina: %d/%d", game->player.stamina, PLAYER_MAX_STAMINA);
  render_text(renderer, panel_x + padding, y, buf, text_color);
  y += line_height;
  draw_resource_bar(renderer, panel_x + padding, y, bar_width, bar_height,
                    game->player.stamina, PLAYER_MAX_STAMINA, stamina_color, bar_bg);
  y += bar_height + 8;

  // MP Bar
  snprintf(buf, sizeof(buf), "MP: %d/%d", game->player.mp, PLAYER_MAX_MP);
  render_text(renderer, panel_x + padding, y, buf, text_color);
  y += line_height;
  draw_resource_bar(renderer, panel_x + padding, y, bar_width, bar_height,
                    game->player.mp, PLAYER_MAX_MP, mp_color, bar_bg);
  y += bar_height + 10;

  // Level and XP
  snprintf(buf, sizeof(buf), "Level %d | Gold: %d", game->player.level, game->player.gold);
  render_text(renderer, panel_x + padding, y, buf, stat_color);
  y += line_height + 4;
  
  snprintf(buf, sizeof(buf), "XP: %d/%d", game->player.xp, XP_PER_LEVEL);
  render_text(renderer, panel_x + padding, y, buf, text_color);
  y += line_height + 2;
  SDL_Color xp_color = {255, 215, 0, 255};
  draw_resource_bar(renderer, panel_x + padding, y, bar_width, 12,
                    game->player.xp, XP_PER_LEVEL, xp_color, bar_bg);
  y += 12 + 12;

  // Divider
  SDL_SetRenderDrawColor(renderer->renderer, 60, 60, 80, 255);
  SDL_Rect divider = {panel_x + padding, y, SIDE_PANEL_WIDTH - padding * 2, 2};
  SDL_RenderFillRect(renderer->renderer, &divider);
  y += 12;

  // ====== EQUIPMENT SECTION ======
  render_text(renderer, panel_x + padding, y, "EQUIPMENT", title_color);
  y += line_height + 4;

  const char *slot_names[] = {"Weapon:", "Armor:", "Boots:", "Accessory:"};
  EquipmentSlot slots[] = {EQUIP_SLOT_WEAPON, EQUIP_SLOT_ARMOR, EQUIP_SLOT_BOOTS, EQUIP_SLOT_ACCESSORY};

  for (int i = 0; i < 4; i++) {
    render_text(renderer, panel_x + padding, y, slot_names[i], text_color);
    y += 16;

    const EquipmentItem *item = game_get_equipped_item(game, slots[i]);
    if (item) {
      SDL_Color item_color = {item->color_r, item->color_g, item->color_b, 255};
      render_text(renderer, panel_x + padding + 10, y, item->name, item_color);
    } else {
      render_text(renderer, panel_x + padding + 10, y, "(empty)", empty_color);
    }
    y += 18;
  }

  y += 6;
  SDL_SetRenderDrawColor(renderer->renderer, 60, 60, 80, 255);
  SDL_Rect divider2 = {panel_x + padding, y, SIDE_PANEL_WIDTH - padding * 2, 2};
  SDL_RenderFillRect(renderer->renderer, &divider2);
  y += 12;

  // ====== COMBAT STATS ======
  render_text(renderer, panel_x + padding, y, "COMBAT STATS", title_color);
  y += line_height + 2;

  snprintf(buf, sizeof(buf), "Attack: %d", game_get_total_attack(game));
  render_text(renderer, panel_x + padding, y, buf, stat_color);
  y += 18;

  snprintf(buf, sizeof(buf), "Defense: %d", game_get_total_defense(game));
  render_text(renderer, panel_x + padding, y, buf, stat_color);
  y += 18;

  snprintf(buf, sizeof(buf), "Dex: %d (%d%% crit)", 
           game_get_total_dexterity(game), 
           game_get_total_dexterity(game));
  render_text(renderer, panel_x + padding, y, buf, stat_color);
  y += 18;

  int speed = game_get_total_speed(game);
  snprintf(buf, sizeof(buf), "Speed: %d", speed);
  render_text(renderer, panel_x + padding, y, buf, stat_color);
  y += 24;

  // ====== SPELL HOTBAR SECTION ======
  SDL_SetRenderDrawColor(renderer->renderer, 60, 60, 80, 255);
  SDL_Rect divider3 = {panel_x + padding, y, SIDE_PANEL_WIDTH - padding * 2, 2};
  SDL_RenderFillRect(renderer->renderer, &divider3);
  y += 12;

  render_text(renderer, panel_x + padding, y, "SPELLS (1-9)", title_color);
  y += line_height + 6;

  SDL_Color spell_color = {150, 200, 255, 255};
  SDL_Color key_color = {255, 215, 0, 255};
  
  for (int i = 0; i < MAX_SPELL_HOTBAR && i < 9; i++) {
    int spell_idx = game->spell_hotbar[i];
    
    snprintf(buf, sizeof(buf), "%d.", i + 1);
    render_text(renderer, panel_x + padding, y, buf, key_color);
    
    if (spell_idx >= 0 && spell_idx < game->spell_count) {
      const Spell *spell = &game->spells[spell_idx];
      snprintf(buf, sizeof(buf), "%s (%d MP)", spell->name, spell->mp_cost);
      render_text(renderer, panel_x + padding + 25, y, buf, spell_color);
    } else {
      render_text(renderer, panel_x + padding + 25, y, "(empty)", empty_color);
    }
    
    y += 18;
  }
}

static void draw_item_pickup_panel(Renderer *renderer, const GameState *game,
                                   int window_w, int window_h) {
  if (game->menu_type != MENU_ITEM_PICKUP || !game->is_on_item_tile || game->item_count_at_player == 0) {
    return;
  }

  int panel_width = 450;
  int panel_height = 80 + game->item_count_at_player * 28 + 40;
  int panel_x = (window_w - panel_width) / 2;
  int panel_y = (window_h - panel_height) / 2;

  // Semi-transparent overlay
  SDL_SetRenderDrawColor(renderer->renderer, 0, 0, 0, 150);
  SDL_Rect overlay = {0, 0, window_w, window_h};
  SDL_RenderFillRect(renderer->renderer, &overlay);

  // Draw panel background
  SDL_SetRenderDrawColor(renderer->renderer, 30, 30, 40, 255);
  SDL_Rect panel = {panel_x, panel_y, panel_width, panel_height};
  SDL_RenderFillRect(renderer->renderer, &panel);

  // Draw border
  SDL_SetRenderDrawColor(renderer->renderer, 100, 255, 100, 255);
  SDL_Rect border = {panel_x - 3, panel_y - 3, panel_width + 6, panel_height + 6};
  SDL_RenderDrawRect(renderer->renderer, &border);

  // Draw title
  SDL_Color title_color = {100, 255, 100, 255};
  SDL_Color text_color = {220, 220, 230, 255};
  SDL_Color key_color = {255, 215, 0, 255};
  SDL_Color hint_color = {180, 180, 200, 255};
  
  int padding = 16;
  int y = panel_y + padding;
  
  render_text(renderer, panel_x + padding, y, "ITEMS ON GROUND", title_color);
  y += 28;
  
  // Divider
  SDL_SetRenderDrawColor(renderer->renderer, 60, 60, 80, 255);
  SDL_Rect divider = {panel_x + padding, y, panel_width - padding * 2, 2};
  SDL_RenderFillRect(renderer->renderer, &divider);
  y += 16;
  
  // Draw items
  for (int i = 0; i < game->item_count_at_player; i++) {
    int item_idx = game->items_at_player[i];
    if (item_idx >= 0 && item_idx < MAX_GROUND_ITEMS && game->ground_items[item_idx].active) {
      const GroundItem *g_item = &game->ground_items[item_idx];
      const ItemType *type = item_get_type(g_item->type_id);
      if (type) {
        char buf[64];
        snprintf(buf, sizeof(buf), "[%d]", i + 1);
        render_text(renderer, panel_x + padding, y, buf, key_color);
        
        SDL_Color item_color = {type->color_r, type->color_g, type->color_b, 255};
        snprintf(buf, sizeof(buf), "%c %s", type->glyph, type->name);
        render_text(renderer, panel_x + padding + 40, y, buf, item_color);
        
        if (g_item->quantity > 1) {
          snprintf(buf, sizeof(buf), "x%d", g_item->quantity);
          render_text(renderer, panel_x + panel_width - padding - 60, y, buf, text_color);
        }
      }
    }
    y += 28;
  }

  // Instructions
  y += 16;
  SDL_SetRenderDrawColor(renderer->renderer, 60, 60, 80, 255);
  SDL_Rect divider2 = {panel_x + padding, y, panel_width - padding * 2, 2};
  SDL_RenderFillRect(renderer->renderer, &divider2);
  y += 12;
  
  render_text(renderer, panel_x + padding, y, "E: Take All    1-5: Take Specific", hint_color);
  y += 22;
  render_text(renderer, panel_x + padding, y, "ESC: Close", hint_color);
}

static void draw_inventory_menu(Renderer *renderer, const GameState *game,
                                 int window_w, int window_h) {
  if (game->menu_type != MENU_INVENTORY) {
    return;
  }

  int menu_width = 500;
  int menu_height = 450;
  int menu_x = (window_w - menu_width) / 2;
  int menu_y = (window_h - menu_height) / 2;

  // Semi-transparent overlay
  SDL_SetRenderDrawColor(renderer->renderer, 0, 0, 0, 180);
  SDL_Rect overlay = {0, 0, window_w, window_h};
  SDL_RenderFillRect(renderer->renderer, &overlay);

  // Menu background
  SDL_SetRenderDrawColor(renderer->renderer, 30, 30, 40, 255);
  SDL_Rect menu = {menu_x, menu_y, menu_width, menu_height};
  SDL_RenderFillRect(renderer->renderer, &menu);

  // Border
  SDL_SetRenderDrawColor(renderer->renderer, 100, 150, 255, 255);
  SDL_Rect border = {menu_x - 2, menu_y - 2, menu_width + 4, menu_height + 4};
  SDL_RenderDrawRect(renderer->renderer, &border);

  SDL_Color title_color = {255, 215, 0, 255};
  SDL_Color text_color = {220, 220, 230, 255};
  SDL_Color hint_color = {150, 150, 170, 255};
  SDL_Color selected_bg = {60, 80, 120, 255};

  int padding = 20;
  int y = menu_y + padding;
  int line_height = 24;

  // Title
  render_text(renderer, menu_x + padding, y, "INVENTORY", title_color);
  y += line_height + 4;

  // Storage info
  char buf[64];
  snprintf(buf, sizeof(buf), "Items: %d/%d", game->backpack_count, MAX_BACKPACK);
  render_text(renderer, menu_x + padding, y, buf, text_color);
  y += line_height + 8;

  // Divider
  SDL_SetRenderDrawColor(renderer->renderer, 60, 60, 80, 255);
  SDL_Rect divider = {menu_x + padding, y, menu_width - padding * 2, 2};
  SDL_RenderFillRect(renderer->renderer, &divider);
  y += 16;

  // Items list
  for (int i = 0; i < game->backpack_count; i++) {
    // Highlight selected item
    if (i == game->menu_selection) {
      SDL_SetRenderDrawColor(renderer->renderer, selected_bg.r, selected_bg.g, selected_bg.b, 255);
      SDL_Rect highlight = {menu_x + padding - 4, y - 2, menu_width - padding * 2 + 8, line_height};
      SDL_RenderFillRect(renderer->renderer, &highlight);
    }

    const BackpackItem *item = &game->backpack[i];
    snprintf(buf, sizeof(buf), "%s", item->name);
    render_text(renderer, menu_x + padding, y, buf, text_color);
    
    if (item->quantity > 1) {
      snprintf(buf, sizeof(buf), "x%d", item->quantity);
      render_text(renderer, menu_x + menu_width - padding - 60, y, buf, hint_color);
    }
    
    y += line_height;
  }

  if (game->backpack_count == 0) {
    render_text(renderer, menu_x + padding, y, "(Empty)", hint_color);
  }

  // Instructions at bottom
  y = menu_y + menu_height - padding - line_height * 3;
  SDL_SetRenderDrawColor(renderer->renderer, 60, 60, 80, 255);
  SDL_Rect divider2 = {menu_x + padding, y, menu_width - padding * 2, 2};
  SDL_RenderFillRect(renderer->renderer, &divider2);
  y += 12;

  render_text(renderer, menu_x + padding, y, "ENTER: Use Item", hint_color);
  y += line_height;
  render_text(renderer, menu_x + padding, y, "D: Drop Item", hint_color);
  y += line_height;
  render_text(renderer, menu_x + padding, y, "TAB/ESC: Close", hint_color);
}

static void draw_spell_preview_overlay(Renderer *renderer, const GameState *game,
                                        int window_w, int window_h, int tiles_w, int tiles_h, int tile_size) {
  if (game->targeting_mode != TARGETING_SPELL) {
    return;
  }
  
  const Spell *spell = &game->spells[game->targeting_spell_idx];
  
  // Get affected tiles
  int tiles_x[100], tiles_y[100], tile_count;
  spell_get_affected_tiles(game, spell, game->targeting_x, game->targeting_y, 
                           tiles_x, tiles_y, &tile_count, 100);
  
  // Determine color based on validity
  SDL_Color preview_color;
  if (game->targeting_valid) {
    preview_color = (SDL_Color){spell->color_r, spell->color_g, spell->color_b, 120};
  } else {
    preview_color = (SDL_Color){180, 50, 50, 120};
  }
  
  // Draw affected tiles
  for (int i = 0; i < tile_count; i++) {
    int map_x = tiles_x[i];
    int map_y = tiles_y[i];
    
    // Check if tile is visible
    int rel_x = map_x - game->cam_x;
    int rel_y = map_y - game->cam_y;
    
    if (rel_x < 0 || rel_x >= tiles_w || rel_y < 0 || rel_y >= tiles_h) {
      continue;
    }
    
    int px = rel_x * tile_size;
    int py = rel_y * tile_size + TITLE_BAR_HEIGHT;
    
    // Draw semi-transparent overlay
    SDL_SetRenderDrawBlendMode(renderer->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer->renderer, preview_color.r, preview_color.g, 
                           preview_color.b, preview_color.a);
    SDL_Rect rect = {px, py, tile_size, tile_size};
    SDL_RenderFillRect(renderer->renderer, &rect);
    
    // Draw border for epicenter or important tiles
    if (map_x == game->targeting_x && map_y == game->targeting_y) {
      SDL_SetRenderDrawColor(renderer->renderer, 255, 255, 255, 220);
      SDL_Rect inner = {px + 1, py + 1, tile_size - 2, tile_size - 2};
      SDL_RenderDrawRect(renderer->renderer, &rect);
      SDL_RenderDrawRect(renderer->renderer, &inner);
    } else {
      // Subtle border for other affected tiles
      SDL_SetRenderDrawColor(renderer->renderer, 255, 255, 255, 80);
      SDL_RenderDrawRect(renderer->renderer, &rect);
    }
    
    SDL_SetRenderDrawBlendMode(renderer->renderer, SDL_BLENDMODE_NONE);
  }
  
  // Draw targeting info box
  int info_x = window_w / 2 - 200;
  int info_y = TITLE_BAR_HEIGHT + 10;
  int info_w = 400;
  int info_h = 90;
  
  SDL_SetRenderDrawBlendMode(renderer->renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer->renderer, 30, 30, 40, 220);
  SDL_Rect info_box = {info_x, info_y, info_w, info_h};
  SDL_RenderFillRect(renderer->renderer, &info_box);
  
  SDL_Color border_color = game->targeting_valid ? 
    (SDL_Color){100, 255, 100, 255} : (SDL_Color){255, 100, 100, 255};
  SDL_SetRenderDrawColor(renderer->renderer, border_color.r, border_color.g, border_color.b, 255);
  SDL_RenderDrawRect(renderer->renderer, &info_box);
  SDL_SetRenderDrawBlendMode(renderer->renderer, SDL_BLENDMODE_NONE);
  
  // Draw spell info
  SDL_Color title_color = {255, 215, 0, 255};
  SDL_Color text_color = {220, 220, 230, 255};
  SDL_Color hint_color = {180, 180, 200, 255};
  
  int text_y = info_y + 10;
  render_text(renderer, info_x + 10, text_y, spell->name, title_color);
  text_y += 22;
  
  char buf[128];
  snprintf(buf, sizeof(buf), "MP Cost: %d | Range: %d tiles", spell->mp_cost, spell->range);
  render_text(renderer, info_x + 10, text_y, buf, text_color);
  text_y += 20;
  
  // Show target type hint
  const char *target_hint = "";
  if (spell->target_type == SPELL_TARGET_SELF) {
    target_hint = "Self-cast (click anywhere to confirm)";
  } else if (spell->target_type == SPELL_TARGET_AREA) {
    target_hint = "Move mouse to aim area effect";
  } else if (spell->target_type == SPELL_TARGET_LINE) {
    target_hint = "Move mouse to aim line attack";
  } else if (spell->target_type == SPELL_TARGET_CONE) {
    target_hint = "Move mouse to aim cone direction";
  } else {
    target_hint = "Move mouse to select target";
  }
  render_text(renderer, info_x + 10, text_y, target_hint, hint_color);
  text_y += 20;
  
  const char *status = game->targeting_valid ? "LEFT CLICK to cast | ESC to cancel" : "INVALID TARGET - Move mouse";
  SDL_Color status_color = game->targeting_valid ? 
    (SDL_Color){100, 255, 100, 255} : (SDL_Color){255, 100, 100, 255};
  render_text(renderer, info_x + 10, text_y, status, status_color);
}

static void draw_spell_codex(Renderer *renderer, const GameState *game,
                              int window_w, int window_h) {
  if (game->menu_type != MENU_SPELL_CODEX) {
    return;
  }

  int menu_width = 650;
  int menu_height = 550;
  int menu_x = (window_w - menu_width) / 2;
  int menu_y = (window_h - menu_height) / 2;

  // Semi-transparent overlay
  SDL_SetRenderDrawBlendMode(renderer->renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer->renderer, 0, 0, 0, 180);
  SDL_Rect overlay = {0, 0, window_w, window_h};
  SDL_RenderFillRect(renderer->renderer, &overlay);

  // Menu background
  SDL_SetRenderDrawColor(renderer->renderer, 25, 25, 35, 255);
  SDL_Rect menu = {menu_x, menu_y, menu_width, menu_height};
  SDL_RenderFillRect(renderer->renderer, &menu);

  // Border
  SDL_SetRenderDrawColor(renderer->renderer, 150, 100, 255, 255);
  SDL_Rect border = {menu_x - 3, menu_y - 3, menu_width + 6, menu_height + 6};
  SDL_RenderDrawRect(renderer->renderer, &border);
  SDL_SetRenderDrawBlendMode(renderer->renderer, SDL_BLENDMODE_NONE);

  SDL_Color title_color = {255, 215, 0, 255};
  SDL_Color text_color = {220, 220, 230, 255};
  SDL_Color hint_color = {150, 150, 170, 255};
  SDL_Color selected_bg = {60, 50, 100, 255};
  SDL_Color hotbar_color = {255, 215, 0, 255};

  int padding = 20;
  int y = menu_y + padding;
  int line_height = 24;
  int col1_x = menu_x + padding;
  int col2_x = menu_x + menu_width / 2 + 10;

  // Title
  render_text(renderer, col1_x, y, "SPELL CODEX", title_color);
  y += line_height + 8;

  // Divider
  SDL_SetRenderDrawColor(renderer->renderer, 60, 60, 80, 255);
  SDL_Rect divider = {menu_x + padding, y, menu_width - padding * 2, 2};
  SDL_RenderFillRect(renderer->renderer, &divider);
  y += 16;

  int list_start_y = y;
  int list_height = menu_height - (y - menu_y) - 80;

  // Left column: Spell list
  for (int i = 0; i < game->spell_count; i++) {
    int item_y = list_start_y + i * line_height;
    
    if (item_y + line_height > list_start_y + list_height) {
      break; // Don't draw outside bounds
    }
    
    // Highlight selected spell
    if (i == game->menu_selection) {
      SDL_SetRenderDrawColor(renderer->renderer, selected_bg.r, selected_bg.g, selected_bg.b, 255);
      SDL_Rect highlight = {col1_x - 4, item_y - 2, menu_width / 2 - 20, line_height};
      SDL_RenderFillRect(renderer->renderer, &highlight);
    }

    const Spell *spell = &game->spells[i];
    SDL_Color spell_color = {spell->color_r, spell->color_g, spell->color_b, 255};
    
    char buf[64];
    snprintf(buf, sizeof(buf), "%c %s", spell->glyph, spell->name);
    render_text(renderer, col1_x, item_y, buf, spell_color);
    
    // Show which hotbar slot it's in
    for (int slot = 0; slot < MAX_SPELL_HOTBAR; slot++) {
      if (game->spell_hotbar[slot] == i) {
        snprintf(buf, sizeof(buf), "[%d]", slot + 1);
        render_text(renderer, col1_x + 200, item_y, buf, hotbar_color);
        break;
      }
    }
  }

  // Right column: Selected spell details
  if (game->menu_selection >= 0 && game->menu_selection < game->spell_count) {
    const Spell *spell = &game->spells[game->menu_selection];
    SDL_Color spell_color = {spell->color_r, spell->color_g, spell->color_b, 255};
    
    int detail_y = list_start_y;
    
    render_text(renderer, col2_x, detail_y, spell->name, spell_color);
    detail_y += line_height + 4;
    
    char buf[128];
    snprintf(buf, sizeof(buf), "MP Cost: %d", spell->mp_cost);
    render_text(renderer, col2_x, detail_y, buf, text_color);
    detail_y += 20;
    
    snprintf(buf, sizeof(buf), "Level Req: %d", spell->level_required);
    render_text(renderer, col2_x, detail_y, buf, text_color);
    detail_y += 20;
    
    if (spell->range > 0) {
      snprintf(buf, sizeof(buf), "Range: %d tiles", spell->range);
      render_text(renderer, col2_x, detail_y, buf, text_color);
      detail_y += 20;
    }
    
    if (spell->radius > 0) {
      snprintf(buf, sizeof(buf), "Radius: %d tiles", spell->radius);
      render_text(renderer, col2_x, detail_y, buf, text_color);
      detail_y += 20;
    }
    
    // Target type
    const char *target_type = "Unknown";
    switch (spell->target_type) {
      case SPELL_TARGET_SELF: target_type = "Self"; break;
      case SPELL_TARGET_SINGLE: target_type = "Single Target"; break;
      case SPELL_TARGET_AREA: target_type = "Area (Circle)"; break;
      case SPELL_TARGET_CONE: target_type = "Cone"; break;
      case SPELL_TARGET_LINE: target_type = "Line"; break;
      case SPELL_TARGET_BEAM: target_type = "Beam"; break;
    }
    snprintf(buf, sizeof(buf), "Type: %s", target_type);
    render_text(renderer, col2_x, detail_y, buf, text_color);
    detail_y += 24;
    
    // Description (word wrap)
    render_text(renderer, col2_x, detail_y, spell->description, hint_color);
  }

  // Instructions at bottom
  y = menu_y + menu_height - padding - line_height * 4;
  SDL_SetRenderDrawColor(renderer->renderer, 60, 60, 80, 255);
  SDL_Rect divider2 = {menu_x + padding, y, menu_width - padding * 2, 2};
  SDL_RenderFillRect(renderer->renderer, &divider2);
  y += 12;

  render_text(renderer, col1_x, y, "UP/DOWN: Select Spell", hint_color);
  y += line_height;
  render_text(renderer, col1_x, y, "ENTER: Cast Spell", hint_color);
  y += line_height;
  render_text(renderer, col1_x, y, "1-9: Bind to Hotbar", hint_color);
  y += line_height;
  render_text(renderer, col1_x, y, "C/B/ESC: Close", hint_color);
}

void renderer_draw_game(Renderer *renderer, const GameState *game,
                        int window_w, int window_h, int tiles_w, int tiles_h) {
  SDL_SetRenderDrawColor(renderer->renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer->renderer);

  int tile_size = game_get_tile_size(game);

  draw_title_bar(renderer, game, window_w);
  draw_map_layer(renderer, game, tiles_w, tiles_h, tile_size);
  draw_ground_items(renderer, game, tiles_w, tiles_h, tile_size);
  draw_enemies(renderer, game, tiles_w, tiles_h, tile_size);
  draw_player(renderer, game, tiles_w, tiles_h, tile_size);
  draw_spell_preview_overlay(renderer, game, window_w, window_h, tiles_w, tiles_h, tile_size);
  draw_minimap(renderer, game, window_w, window_h);
  draw_status_bar(renderer, game, window_w, window_h);
  draw_side_panel(renderer, game, window_w, window_h);
  draw_item_pickup_panel(renderer, game, window_w, window_h);
  draw_inventory_menu(renderer, game, window_w, window_h);
  draw_spell_codex(renderer, game, window_w, window_h);

  SDL_RenderPresent(renderer->renderer);
}

int renderer_init(Renderer *renderer, int width, int height) {
  memset(renderer, 0, sizeof(Renderer));

  if (TTF_Init() != 0) {
    SDL_Log("TTF_Init failed: %s", SDL_GetError());
    return -1;
  }

  renderer->font_size = 14;
  renderer->font = TTF_OpenFont("fonts/PressStart2P-Regular.ttf", renderer->font_size);
  if (!renderer->font) {
    SDL_Log("Trying: fonts/PressStart2P-Regular.ttf - Failed");
    renderer->font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf", renderer->font_size);
  } else {
    SDL_Log("Loaded: fonts/PressStart2P-Regular.ttf");
  }
  if (!renderer->font) {
    SDL_Log("Trying: Liberation Mono - Failed");
    renderer->font = TTF_OpenFont("C:/Windows/Fonts/consola.ttf", renderer->font_size);
  }
  if (!renderer->font) {
    SDL_Log("Trying: C:/Windows/Fonts/consola.ttf - Failed");
    renderer->font = TTF_OpenFont("C:\\Windows\\Fonts\\consola.ttf", renderer->font_size);
  }
  if (!renderer->font) {
    SDL_Log("Trying: C:\\Windows\\Fonts\\consola.ttf - Failed");
    renderer->font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", renderer->font_size);
  }
  if (!renderer->font) {
    SDL_Log("WARNING: Could not load any font! Text rendering disabled.");
  } else {
    SDL_Log("Font loaded successfully!");
  }

  int title_font_size = 32;
  renderer->title_font = TTF_OpenFont("fonts/PressStart2P-Regular.ttf", title_font_size);
  if (!renderer->title_font) {
    renderer->title_font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf", title_font_size);
  }
  if (!renderer->title_font) {
    renderer->title_font = TTF_OpenFont("C:/Windows/Fonts/consola.ttf", title_font_size);
  }
  if (!renderer->title_font) {
    renderer->title_font = TTF_OpenFont("C:\\Windows\\Fonts\\consola.ttf", title_font_size);
  }
  if (!renderer->title_font) {
    renderer->title_font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", title_font_size);
  }
  if (!renderer->title_font && renderer->font) {
    renderer->title_font = renderer->font;
  }
  if (!renderer->title_font) {
    SDL_Log("WARNING: Could not load title font!");
  }

  // Create window without fullscreen or maximized flags
  renderer->window = SDL_CreateWindow(
      "G-Rend Roguelike", 
      SDL_WINDOWPOS_CENTERED, 
      SDL_WINDOWPOS_CENTERED,
      width, 
      height, 
      SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  
  if (!renderer->window) {
    SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
    TTF_Quit();
    return -1;
  }
  
  // Explicitly ensure window is NOT fullscreen or maximized
  SDL_SetWindowFullscreen(renderer->window, 0);
  
  // Get actual window size after creation
  int actual_w, actual_h;
  SDL_GetWindowSize(renderer->window, &actual_w, &actual_h);
  SDL_Log("Window created: requested %dx%d, actual %dx%d", width, height, actual_w, actual_h);

  renderer->renderer =
      SDL_CreateRenderer(renderer->window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer->renderer) {
    SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
    SDL_DestroyWindow(renderer->window);
    TTF_Quit();
    return -1;
  }

  return 0;
}

void renderer_shutdown(Renderer *renderer) {
  if (renderer->font) {
    TTF_CloseFont(renderer->font);
    renderer->font = NULL;
  }
  if (renderer->title_font && renderer->title_font != renderer->font) {
    TTF_CloseFont(renderer->title_font);
    renderer->title_font = NULL;
  }
  if (renderer->renderer) {
    SDL_DestroyRenderer(renderer->renderer);
    renderer->renderer = NULL;
  }
  if (renderer->window) {
    SDL_DestroyWindow(renderer->window);
    renderer->window = NULL;
  }
  TTF_Quit();
}
