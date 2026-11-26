#include "pong.h"

#define WHITE_RGB444 0x0FFF
#define BALL_W 2
#define BALL_H 2

#define PADDLE_X 61
#define PADDLE_H 10
#define PADDLE_STEP 2

static int ball_x = 8;
static int ball_y = 31;
static int old_ball_x = 8;
static int old_ball_y = 31;
static int vx = 1;
static uint32_t frame_div = 0;

static int paddle_y = 27;
static int old_paddle_y = 27;

static const uint16_t px_white[1] = {WHITE_RGB444};
static const uint16_t px_clear[1] = {0x0000};

static const uint16_t test_ball[BALL_W * BALL_H] = {
    WHITE_RGB444,
    WHITE_RGB444,
    WHITE_RGB444,
    WHITE_RGB444,
};

static void draw_paddle_line(int y, const uint16_t *color_px) {
  for (int dy = 0; dy < PADDLE_H; dy++) {
    draw_asset(LAYER_SPRITE, PADDLE_X, y + dy, 1, 1, color_px);
  }
}

void pong_mode_menu(void) { draw_background(pongMenu); }

void pong_init(GameMode mode) {
  (void)mode;
  ball_x = 8;
  ball_y = 31;
  old_ball_x = ball_x;
  old_ball_y = ball_y;
  vx = 1;
  frame_div = 0;

  paddle_y = (64 - PADDLE_H) / 2;
  old_paddle_y = paddle_y;

  clearLayers();
  draw_paddle_line(paddle_y, px_white);
}

int pong_input(int key) {
  if (key == BTN_B) {
    return -1;
  }

  if (key == BTN_RIGHT) {
    paddle_y -= PADDLE_STEP;
    if (paddle_y < 0) {
      paddle_y = 0;
    }
    return GAME_DIRTY;
  }

  if (key == BTN_LEFT) {
    paddle_y += PADDLE_STEP;
    if (paddle_y > 64 - PADDLE_H) {
      paddle_y = 64 - PADDLE_H;
    }
    return GAME_DIRTY;
  }

  if (key == BTN_A) {
    return GAME_REQ_PAUSE;
  }
  return GAME_OK;
}

int pong_tick(void) {
  const uint32_t ticks_per_step = 4000;
  frame_div++;
  if (frame_div < ticks_per_step) {
    return 0;
  }
  frame_div = 0;

  old_ball_x = ball_x;
  old_ball_y = ball_y;

  ball_x += vx;
  if (ball_x <= 2) {
    ball_x = 2;
    vx = 1;
  } else if (ball_x >= 64 - BALL_W - 2) {
    ball_x = 64 - BALL_W - 2;
    vx = -1;
  }

  return 1;
}

void pong_render(void) {
  if (old_paddle_y != paddle_y) {
    draw_paddle_line(old_paddle_y, px_clear);
    draw_paddle_line(paddle_y, px_white);
    old_paddle_y = paddle_y;
  }

  move_sprite(old_ball_x, old_ball_y, ball_x, ball_y, BALL_W, BALL_H,
              test_ball);
  old_ball_x = ball_x;
  old_ball_y = ball_y;
}

void pong_pause() { draw_background(pongPause); }
