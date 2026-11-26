#include "assets.c"
#include "assets.h"
#include "games/pong.c"
#include "games/pong.h"
#include "games/roadFigthers.c"
#include "games/roadFigthers.h"
#include "include/hub75.h"
#include <stdint.h>

#define GAMES_COUNT 6
#define BTN_COUNT 4
#define QSIZE 16
#define GAME_TICK_MS 20

int bg_menu = 0;
static uint8_t game_inited = 0;


static const GameOps games[GAMES_COUNT] = {
    [ROADF] = {roadF_init, roadF_input, 0, roadF_render, roadF_mode_menu, 0, 0},
    [TETRIS] = {0, 0, 0, 0, 0, 0, 0},
    [BREAKOUT] = {0, 0, 0, 0, 0, 0, 0},
    [SNAKE] = {0, 0, 0, 0, 0, 0, 0},
    [SPACE_INVADERS] = {0, 0, 0, 0, 0, 0, 0},
    [PONG] = {pong_init, pong_input, pong_tick, pong_render, pong_mode_menu,
              pong_pause, 0},
};

int8_t gameIndex = 0;
int8_t modeIndex = 0;
GameMode selectedMode = SINGLE;
uint8_t dirty = 1;

static volatile uint8_t key_queue[QSIZE];
static volatile uint8_t q_head = 0;
static volatile uint8_t q_tail = 0;

static const uint8_t bit_to_key[BTN_COUNT] = { BTN_A,    BTN_B,
                                              BTN_RIGHT, BTN_LEFT};

void __attribute__((interrupt)) isr(void) {
  unsigned int p = IO_IN(IO_BUTTONS);
  for (int i = 0; i < BTN_COUNT; i++) {
    if (p & (1u << i)) {
      uint8_t next = (q_head + 1) & (QSIZE - 1);
      if (next != q_tail) {
        key_queue[q_head] = bit_to_key[i];
        q_head = next;
      }
    }
  }
}

static int key_pop(void) {
  if (q_tail == q_head)
    return 0;
  int key = key_queue[q_tail];
  q_tail = (q_tail + 1) & (QSIZE - 1);
  return key;
}

static void enter_screen(enum SCREEN next) {
  clearLayers();
  screen = next;
  bg_menu = 0;
  dirty = 1;

  if (next == GAME && !game_inited) {
    if (gameIndex >= 0 && gameIndex < GAMES_COUNT && games[gameIndex].init) {
      games[gameIndex].init(selectedMode);
    }
    game_inited = 1;
  }
}

static void apply_game_status(int status) {
  switch (status) {
  case GAME_EXIT_MODE:
    game_inited = 0;
    enter_screen(MODE_SELECTOR);
    break;
  case GAME_REQ_PAUSE:
    enter_screen(PAUSE);
    break;
  case GAME_REQ_OVER:
    game_inited = 0;
    enter_screen(GAME_OVER);
    break;
  case GAME_DIRTY:
    dirty = 1;
    break;
  case GAME_OK:
  default:
    break;
  }
}

void update_index_pos(void) {
  const int selector_x = 18;
  const int selector_y = 18;
  const int selector_w = 28;
  const int selector_h = 28;
  static int8_t old_sel_x = 18;
  static int8_t old_sel_y = 18;

  if (gameIndex < 0)
    gameIndex = 0;
  if (gameIndex >= GAMES_COUNT)
    gameIndex = GAMES_COUNT - 1;

  move_sprite(old_sel_x, old_sel_y, selector_x, selector_y, selector_w,
              selector_h, selector28x28);
  old_sel_x = selector_x;
  old_sel_y = selector_y;
}

static void handle_key(int key) {
  switch (screen) {
  case WATING:
    if (key == BTN_A) {
      enter_screen(GAMES_MENU);
    }
    break;

  case GAMES_MENU:
    switch (key) {
    case BTN_RIGHT:
      gameIndex = (gameIndex + 1) % GAMES_COUNT;
      cleanLayer(LAYER_BG);
      dirty = 1;
      break;
    case BTN_LEFT:
      gameIndex = (gameIndex + GAMES_COUNT - 1) % GAMES_COUNT;
      cleanLayer(LAYER_BG);
      dirty = 1;
      break;
    case BTN_B:
      enter_screen(WATING);
      break;
    case BTN_A:
      modeIndex = 0;
      game_inited = 0;
      enter_screen(MODE_SELECTOR);
      break;
    }
    break;

  case MODE_SELECTOR:
    switch (key) {
    case BTN_UP:
      modeIndex = (modeIndex + NUM_MODES - 1) % NUM_MODES;
      dirty = 1;
      break;
    case BTN_DOWN:
      modeIndex = (modeIndex + 1) % NUM_MODES;
      dirty = 1;
      break;
    case BTN_A:
      selectedMode = (GameMode)modeIndex;
      game_inited = 0;
      enter_screen(GAME);
      break;
    case BTN_B:
      enter_screen(GAMES_MENU);
      break;
    }
    break;

  case GAME:
    if (gameIndex >= 0 && gameIndex < GAMES_COUNT && games[gameIndex].input) {
      int res = games[gameIndex].input(key);
      dirty = 1;
      apply_game_status(res);
    } else if (key == BTN_B) {
      game_inited = 0;
      enter_screen(MODE_SELECTOR);
    }
    break;

  case PAUSE:
    switch (key) {
    case BTN_A:
      enter_screen(GAME);
      break;
    case BTN_B:
      enter_screen(MODE_SELECTOR);
      break;
    }
    break;

  case GAME_OVER:
    switch (key) {
    case BTN_A:
      game_inited = 0;
      enter_screen(GAME);
      break;
    case BTN_B:
      game_inited = 0;
      enter_screen(MODE_SELECTOR);
      break;
    }
    break;

  default:
    break;
  }
}

static void handle_time(void) {
  static uint64_t next_ms = 0;

  if (screen != GAME || gameIndex < 0 || gameIndex >= GAMES_COUNT ||
      !games[gameIndex].tick)
    return;

  uint64_t now = milliseconds();
  if (now < next_ms)
    return;
  next_ms = now + GAME_TICK_MS;

  dirty = 1;
  apply_game_status(games[gameIndex].tick());
}

void render(void) {
  if (!dirty)
    return;

  switch (screen) {
  case WATING:
    draw_background(waitingScreen);
    break;

  case GAMES_MENU: {
    int item_w = 26;
    int step_x = 28;
    int item_center_x = gameIndex * step_x + item_w / 2;
    int menu_x = 32 - item_center_x;

    draw_asset(LAYER_BG, menu_x, 20, 166, 26, LateralMenuGamer166x26);
    update_index_pos();
    break;
  }

  case MODE_SELECTOR: {
    const int row_y[NUM_MODES] = {32, 41, 50};
    static int8_t old_arrow_y = 32;

    if (bg_menu == 0) {
      if (games[gameIndex].draw_mode_bg) {
        games[gameIndex].draw_mode_bg();
      } else {
        cleanLayer(LAYER_BG);
        draw_background(underCostruction);
      }
      old_arrow_y = row_y[modeIndex];
      bg_menu = 1;
    }

    move_sprite(5, old_arrow_y, 5, row_y[modeIndex], 3, 5, selectArrow);
    old_arrow_y = row_y[modeIndex];
    break;
  }

  case GAME:
    if (gameIndex >= 0 && gameIndex < GAMES_COUNT && games[gameIndex].render) {
      games[gameIndex].render();
    }
    break;

  case PAUSE:
    if (gameIndex >= 0 && gameIndex < GAMES_COUNT &&
        games[gameIndex].draw_pause) {
      games[gameIndex].draw_pause();
    }
    break;

  case GAME_OVER:
    if (gameIndex >= 0 && gameIndex < GAMES_COUNT &&
        games[gameIndex].draw_over) {
      games[gameIndex].draw_over();
    }
    break;

  default:
    break;
  }
  dirty = 0;
}

int main(void) {
  asm volatile("csrw mtvec, %0" ::"r"(isr));
  asm volatile("csrsi mstatus, 8");

  clearLayers();
  render();

  for (;;) {
    int key = key_pop();
    if (key) {
      handle_key(key);
    } else {
      handle_time();
    }
    render();
  }
}
