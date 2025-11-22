#ifndef SDL_STUB_H
#define SDL_STUB_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t Uint8;
typedef uint32_t Uint32;
typedef int32_t SDL_Keycode;

#define SDL_INIT_VIDEO 0x00000020U
#define SDL_INIT_EVENTS 0x00004000U
#define SDL_WINDOWPOS_CENTERED 0
#define SDL_WINDOW_SHOWN 0x00000001U
#define SDL_WINDOW_RESIZABLE 0x00000020U
#define SDL_RENDERER_ACCELERATED 0x00000002U

#define SDL_QUIT 0x100
#define SDL_KEYDOWN 0x300
#define SDL_MOUSEBUTTONDOWN 0x401

#define SDLK_w 'w'
#define SDLK_s 's'
#define SDLK_a 'a'
#define SDLK_d 'd'
#define SDLK_q 'q'
#define SDLK_TAB 9
#define SDLK_RETURN 13
#define SDLK_SPACE ' '
#define SDLK_LEFTBRACKET '['
#define SDLK_RIGHTBRACKET ']'
#define SDLK_UP 273
#define SDLK_DOWN 274
#define SDLK_LEFT 276
#define SDLK_RIGHT 275
#define SDLK_ESCAPE 27
#define SDLK_EQUALS 61
#define SDLK_MINUS 45
#define SDLK_KP_PLUS 0x1000
#define SDLK_KP_MINUS 0x1001
#define SDLK_F5 0x4000003A
#define SDLK_F9 0x4000003E

typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Surface {
  int w;
  int h;
} SDL_Surface;
typedef struct SDL_Texture SDL_Texture;
typedef struct _TTF_Font TTF_Font;

typedef struct {
  Uint32 sym;
} SDL_Keysym;

typedef struct {
  SDL_Keysym keysym;
} SDL_KeyboardEvent;

typedef struct {
  int x;
  int y;
  Uint8 button;
} SDL_MouseButtonEvent;

typedef union SDL_Event {
  Uint32 type;
  SDL_KeyboardEvent key;
  SDL_MouseButtonEvent button;
} SDL_Event;

typedef struct {
  int x;
  int y;
  int w;
  int h;
} SDL_Rect;

typedef struct {
  Uint8 r;
  Uint8 g;
  Uint8 b;
  Uint8 a;
} SDL_Color;

int SDL_Init(Uint32 flags);
void SDL_Quit(void);
SDL_Window *SDL_CreateWindow(const char *title, int x, int y, int w, int h,
                             Uint32 flags);
void SDL_DestroyWindow(SDL_Window *window);
SDL_Renderer *SDL_CreateRenderer(SDL_Window *window, int index, Uint32 flags);
void SDL_DestroyRenderer(SDL_Renderer *renderer);
const char *SDL_GetError(void);
void SDL_Log(const char *fmt, ...);
void SDL_SetWindowTitle(SDL_Window *window, const char *title);
int SDL_PollEvent(SDL_Event *event);
void SDL_SetRenderDrawColor(SDL_Renderer *renderer, Uint8 r, Uint8 g, Uint8 b,
                            Uint8 a);
int SDL_RenderClear(SDL_Renderer *renderer);
int SDL_RenderFillRect(SDL_Renderer *renderer, const SDL_Rect *rect);
void SDL_RenderPresent(SDL_Renderer *renderer);
void SDL_Delay(Uint32 ms);
void SDL_GetWindowSize(SDL_Window *window, int *w, int *h);
Uint32 SDL_GetTicks(void);
int SDL_RenderDrawPoint(SDL_Renderer *renderer, int x, int y);
SDL_Texture *SDL_CreateTextureFromSurface(SDL_Renderer *renderer, SDL_Surface *surface);
int SDL_RenderCopy(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_Rect *dstrect);
void SDL_DestroyTexture(SDL_Texture *texture);

#define SDL_BUTTON_LEFT 1
#define SDL_BUTTON_MIDDLE 2
#define SDL_BUTTON_RIGHT 3

SDL_Surface *TTF_RenderText_Solid(TTF_Font *font, const char *text, SDL_Color fg);
void SDL_FreeSurface(SDL_Surface *surface);

int TTF_Init(void);
TTF_Font *TTF_OpenFont(const char *file, int ptsize);
void TTF_CloseFont(TTF_Font *font);
void TTF_Quit(void);

#endif // SDL_STUB_H

