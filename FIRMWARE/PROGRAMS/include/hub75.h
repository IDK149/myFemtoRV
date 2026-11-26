#ifndef HUB75_H
#define HUB75_H

#include "../../LIBFEMTORV32/femtorv32.h"
#include <stdint.h>

typedef enum {
  BTN_A = 1,
  BTN_B,
  BTN_RIGHT,
  BTN_LEFT,
  BTN_UP,
  BTN_DOWN
} Buttons;

#define LAYER_BG 0U
#define LAYER_SPRITE 1U

#define COLOR_BLACK 0x000
#define COLOR_WHITE 0xFFF
#define COLOR_RED 0xF00
#define COLOR_GREEN 0x0F0
#define COLOR_BLUE 0x00F
#define COLOR_YELLOW 0xFF0
#define COLOR_CYAN 0x0FF
#define TRANSPARENT 0xF0F

enum SCREEN { WATING, GAMES_MENU, MODE_SELECTOR, PAUSE, GAME_OVER, GAME };
enum SCREEN screen = WATING;

enum GAME { ROADF, TETRIS, BREAKOUT, SNAKE, SPACE_INVADERS, PONG };

typedef enum { SINGLE = 0, MULTI, LOCAL, NUM_MODES } GameMode;

enum GAME_STATUS {
  GAME_OK = 0,
  GAME_DIRTY = 1,
  GAME_EXIT_MODE = -1,
  GAME_REQ_PAUSE = 2,
  GAME_REQ_OVER = 3
};

typedef struct {
  void (*init)(GameMode mode);
  int (*input)(int key);
  int (*tick)(void);
  void (*render)(void);
  void (*draw_mode_bg)(void);
  void (*draw_pause)(void);
  void (*draw_over)(void);
} GameOps;

static inline void hub75_draw_pixel(uint32_t layer, uint8_t x, uint8_t y,
                                    uint32_t color) {
  if (x >= 64 || y >= 64)
    return;
  uint32_t word =
      (layer << 30) | ((uint32_t)y << 24) | ((uint32_t)x << 18) | color;
  IO_IN(IO_HUB75) = word;
}

static inline void fill_rect(uint32_t layer, int x0, int y0, int w, int h,
                             uint32_t color) {
  for (int y = y0; y < y0 + h; y++)
    for (int x = x0; x < x0 + w; x++)
      hub75_draw_pixel(layer, x, y, color);
}

static inline void cleanLayer(uint32_t layer) {
  fill_rect(layer, 0, 0, 64, 64, 0xF0F);
}

static void clearLayers(void) {
  cleanLayer(LAYER_BG);
  cleanLayer(LAYER_SPRITE);
}

static inline void draw_asset(uint32_t layer, int x0, int y0, int w, int h,
                              const uint16_t *asset) {
  int index = 0;
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      uint32_t px_x = x0 + x;
      uint32_t px_y = y0 + y;
      if (px_x < 64 && px_y < 64) {
        hub75_draw_pixel(layer, px_x, px_y, asset[y * w + x]);
      }
      index++;
    }
  }
}

static inline void draw_background(const uint16_t *asset) {
  draw_asset(LAYER_BG, 0, 0, 64, 64, asset);
}

static inline void move_sprite(int x_prev, int y_prev, int x_new, int y_new,
                               int w, int h, const uint16_t *asset) {
  fill_rect(LAYER_SPRITE, x_prev, y_prev, w, h, 0xF0F);
  draw_asset(LAYER_SPRITE, x_new, y_new, w, h, asset);
}

#endif
