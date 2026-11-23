#include "assets.h"
#include "assets.c"
#include "games/pong.c"
#include "games/pong.h"
#include "../LIBFEMTORV32/keyboard.h"
#include "include/hub75.h"
#include <stdint.h>

#define MAX_GAMES 6

enum SCREEN { WATING, GAMES_MENU, MODE_SELECTOR, PAUSE, GAME_OVER, GAME };
enum SCREEN screen = WATING;

enum GAME { ROADF, TETRIS, BREAKOUT, SNAKE, SPACE_INVADERS, PONG };

typedef enum {
  SINGLE = 0,
  MULTI,
  LOCAL,
  NUM_MODES
} GameMode;

typedef struct {
  void (*init)(void);
  int (*input)(int key);
  void (*update)(void);
  void (*render)(void);
} GameOps;

static uint8_t game_inited = 0;
static uint8_t mode_bg_drawn = 0;

static const GameOps games[MAX_GAMES] = {
    [ROADF]          = {0, 0, 0, 0},
    [TETRIS]         = {0, 0, 0, 0},
    [BREAKOUT]       = {0, 0, 0, 0},
    [SNAKE]          = {0, 0, 0, 0},
    [SPACE_INVADERS] = {0, 0, 0, 0},
    [PONG]           = {pong_init, pong_input, pong_update, pong_render},
};

static const int SELECTOR_X = 18;
static const int SELECTOR_Y = 18;
static const int SELECTOR_W = 28;
static const int SELECTOR_H = 28;
int8_t old_sel_x = 18;
int8_t old_sel_y = 18;

static const int ARROW_X = 5;
static const int ARROW_W = 3;
static const int ARROW_H = 5;
static const int row_y[NUM_MODES] = {32, 38, 45};
int8_t old_arrow_x = 5;
int8_t old_arrow_y = 32;

int8_t gameIndex = 0;
int8_t modeIndex = 0;
GameMode selectedMode = SINGLE;
uint8_t dirty = 1;

void update_index_pos(void) {
  if (gameIndex < 0)
    gameIndex = 0;
  if (gameIndex >= MAX_GAMES)
    gameIndex = MAX_GAMES - 1;

  move_sprite(old_sel_x, old_sel_y, SELECTOR_X, SELECTOR_Y,
              SELECTOR_W, SELECTOR_H, selector28x28);
  old_sel_x = SELECTOR_X;
  old_sel_y = SELECTOR_Y;
}

int update_input(void) {
  int key = UART_pollkey();
  if (!key)
    return 0;

  switch (screen) {
  case WATING:
    if (key == BTN_A || key == BTN_START) {
      screen = GAMES_MENU;
      dirty = 1;
    }
    break;

  case GAMES_MENU:
    switch (key) {
    case BTN_RIGHT:
      gameIndex = (gameIndex + 1) % MAX_GAMES;
      dirty = 1;
      break;
    case BTN_LEFT:
      gameIndex = (gameIndex + MAX_GAMES - 1) % MAX_GAMES;
      dirty = 1;
      break;
    case BTN_B:
      screen = WATING;
      dirty = 1;
      break;
    case BTN_A:
    case BTN_START:
      modeIndex = 0;
      mode_bg_drawn = 0;
      screen = MODE_SELECTOR;
      game_inited = 0;
      dirty = 1;
      break;
    }
    break;

  case MODE_SELECTOR:
    switch (key) {
    case BTN_UP:
    case BTN_RIGHT:
      modeIndex = (modeIndex + 1) % NUM_MODES;
      dirty = 1;
      break;
    case BTN_DOWN:
    case BTN_LEFT:
      modeIndex = (modeIndex + NUM_MODES - 1) % NUM_MODES;
      dirty = 1;
      break;
    case BTN_A:
    case BTN_START:
      selectedMode = (GameMode)modeIndex;
      if (gameIndex == PONG)
        /* LOCAL currently uses the same Pong path; FPGA link comes later. */
        pong_set_mode(selectedMode != SINGLE);
      mode_bg_drawn = 0;
      game_inited = 0;
      screen = GAME;
      dirty = 1;
      break;
    case BTN_B:
      mode_bg_drawn = 0;
      screen = GAMES_MENU;
      dirty = 1;
      break;
    }
    break;

  case GAME: {
    if (gameIndex >= 0 && gameIndex < MAX_GAMES && games[gameIndex].input) {
      int res = games[gameIndex].input(key);
      if (res < 0) {
        mode_bg_drawn = 0;
        screen = MODE_SELECTOR;
        game_inited = 0;
        dirty = 1;
      } else if (res > 0) {
        dirty = 1;
      }
    } else if (key == BTN_B) {
      mode_bg_drawn = 0;
      screen = MODE_SELECTOR;
      game_inited = 0;
      dirty = 1;
    }
    break;
  }

  default:
    break;
  }
  return 1;
}

static void draw_mode_selector(void) {
  if (!mode_bg_drawn) {
    cleanLayer(LAYER_BG);
    cleanLayer(LAYER_SPRITE);
    draw_background(pongMenu);
    old_arrow_x = ARROW_X;
    old_arrow_y = row_y[modeIndex];
    mode_bg_drawn = 1;
  }

  move_sprite(old_arrow_x, old_arrow_y, ARROW_X, row_y[modeIndex],
              ARROW_W, ARROW_H, selectArrow);
  old_arrow_x = ARROW_X;
  old_arrow_y = row_y[modeIndex];
}

void render(void) {
  if (!dirty)
    return;

  switch (screen) {
  case WATING:
    cleanLayer(LAYER_BG);
    cleanLayer(LAYER_SPRITE);
    draw_background(waitingScreen);
    break;

  case GAMES_MENU: {
    int item_w = 26;
    int step_x = 28;
    int item_center_x = gameIndex * step_x + item_w / 2;
    int menu_x = 32 - item_center_x;

    cleanLayer(LAYER_BG);
    cleanLayer(LAYER_SPRITE);
    draw_asset(LAYER_BG, menu_x, 20, 166, 26, LateralMenuGamer166x26);
    update_index_pos();
    break;
  }

  case MODE_SELECTOR:
    draw_mode_selector();
    break;

  case GAME:
    if (gameIndex >= 0 && gameIndex < MAX_GAMES) {
      if (!game_inited) {
        if (games[gameIndex].init) {
          games[gameIndex].init();
        }
        game_inited = 1;
      }
      if (games[gameIndex].update) {
        games[gameIndex].update();
        dirty = 1;
      }
      if (games[gameIndex].render) {
        games[gameIndex].render();
      }
    }
    break;

  default:
    break;
  }
  dirty = 0;
}

int main(void) {
  cleanLayer(LAYER_BG);
  cleanLayer(LAYER_SPRITE);
  render();

  for (;;) {
    update_input();
    render();
    if (screen == GAME) {
      milliwait(16);
    }
  }
}
