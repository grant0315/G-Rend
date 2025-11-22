#ifndef RENDERER_H
#define RENDERER_H

#include "game.h"
#include "sdl_platform.h"

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  TTF_Font *font;
  TTF_Font *title_font;
  int font_size;
} Renderer;

int renderer_init(Renderer *renderer, int width, int height);
void renderer_shutdown(Renderer *renderer);
void renderer_draw_start_screen(Renderer *renderer, int window_w, int window_h);
void renderer_draw_game(Renderer *renderer, const GameState *game, int window_w,
                       int window_h, int tiles_w, int tiles_h);

#endif /* RENDERER_H */
