#ifndef SDL_PLATFORM_H
#define SDL_PLATFORM_H

#define SDL_MAIN_HANDLED

#if defined(__has_include)
#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#elif __has_include("sdl_stub.h")
#include "sdl_stub.h"
#else
#error "SDL2/SDL.h is required"
#endif
#else
#include <SDL2/SDL.h>
#endif

#if defined(__has_include)
#if __has_include(<SDL2/SDL_ttf.h>)
#include <SDL2/SDL_ttf.h>
#elif __has_include("sdl_stub.h")
typedef struct _TTF_Font TTF_Font;
int TTF_Init(void);
TTF_Font *TTF_OpenFont(const char *file, int ptsize);
void TTF_CloseFont(TTF_Font *font);
void TTF_Quit(void);
SDL_Surface *TTF_RenderText_Solid(TTF_Font *font, const char *text, SDL_Color fg);
#else
#error "SDL2/SDL_ttf.h is required"
#endif
#else
#include <SDL2/SDL_ttf.h>
#endif

// These are only needed if SDL2 is not available (stub mode)
#if !defined(SDL_h_)
// Define SDL_MouseWheelEvent if not available
typedef struct SDL_MouseWheelEvent {
    uint32_t type;
    uint32_t timestamp;
    uint32_t windowID;
    uint32_t which;
    int32_t x;
    int32_t y;
    uint32_t direction;
} SDL_MouseWheelEvent;

// Declare SDL functions if not available
int SDL_RenderDrawRect(SDL_Renderer *renderer, const SDL_Rect *rect);
int SDL_SetWindowFullscreen(SDL_Window *window, uint32_t flags);
void SDL_GetWindowSize(SDL_Window *window, int *w, int *h);
uint32_t SDL_GetMouseState(int *x, int *y);
#endif

#ifndef SDLK_TAB
#define SDLK_TAB 9
#endif
#ifndef SDLK_RIGHTBRACKET
#define SDLK_RIGHTBRACKET ']'
#endif
#ifndef SDLK_LEFTBRACKET
#define SDLK_LEFTBRACKET '['
#endif
#ifndef SDLK_RETURN
#define SDLK_RETURN 13
#endif
#ifndef SDLK_SPACE
#define SDLK_SPACE ' '
#endif
// Define key codes (SDL uses these values from ASCII/Unicode)
// These match SDL2's standard key definitions
#ifndef SDLK_e
#define SDLK_e 'e'
#endif
#ifndef SDLK_r
#define SDLK_r 'r'
#endif
#ifndef SDLK_1
#define SDLK_1 '1'
#endif
#ifndef SDLK_2
#define SDLK_2 '2'
#endif
#ifndef SDLK_3
#define SDLK_3 '3'
#endif
#ifndef SDLK_4
#define SDLK_4 '4'
#endif
#ifndef SDLK_5
#define SDLK_5 '5'
#endif
#ifndef SDLK_6
#define SDLK_6 '6'
#endif
#ifndef SDLK_7
#define SDLK_7 '7'
#endif
#ifndef SDLK_8
#define SDLK_8 '8'
#endif
#ifndef SDLK_9
#define SDLK_9 '9'
#endif
#ifndef SDLK_b
#define SDLK_b 'b'
#endif
#ifndef SDLK_c
#define SDLK_c 'c'
#endif
#ifndef SDLK_i
#define SDLK_i 'i'
#endif
#ifndef SDLK_EQUALS
#define SDLK_EQUALS '='
#endif
#ifndef SDLK_PLUS
#define SDLK_PLUS '+'
#endif
#ifndef SDL_MOUSEWHEEL
#define SDL_MOUSEWHEEL 0x303
#endif
#ifndef SDL_MOUSEMOTION
#define SDL_MOUSEMOTION 0x400
#endif

#endif /* SDL_PLATFORM_H */

