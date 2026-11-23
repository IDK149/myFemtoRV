#include "pong.h"

#define PONG_WIDTH 64
#define PONG_HEIGHT 64
#define PADDLE_WIDTH 2
#define LEFT_PADDLE_HEIGHT 10
#define RIGHT_PADDLE_HEIGHT 9
#define PADDLE_SPEED 2
#define BALL_SIZE 2
#define POINTS_TO_WIN 11
#define MAX_BALL_SPEED 3
#define SERVE_DELAY_FRAMES 20

#define LEFT_PADDLE_X 3
#define RIGHT_PADDLE_X 59
#define SCORE_LEFT_EDGE 27
#define SCORE_RIGHT_X 37
#define SCORE_Y 2
#define SCORE_DIGIT_HEIGHT 9

static int left_y;
static int right_y;
static int old_left_y;
static int old_right_y;
static int ball_x;
static int ball_y;
static int old_ball_x;
static int old_ball_y;
static int ball_dx;
static int ball_dy;
static int ball_speed;
static int rally_hits;
static int serve_delay;
static int left_score;
static int right_score;
static int game_over;
static int needs_full_render;
static int multiplayer;

static int clamp_left_y(int y) {
  if (y < 0)
    return 0;
  if (y > PONG_HEIGHT - LEFT_PADDLE_HEIGHT)
    return PONG_HEIGHT - LEFT_PADDLE_HEIGHT;
  return y;
}

static int clamp_right_y(int y) {
  if (y < 0)
    return 0;
  if (y > PONG_HEIGHT - RIGHT_PADDLE_HEIGHT)
    return PONG_HEIGHT - RIGHT_PADDLE_HEIGHT;
  return y;
}

static void serve_ball(int direction, int inherited_dy) {
  ball_x = (PONG_WIDTH - BALL_SIZE) / 2;
  ball_y = (PONG_HEIGHT - BALL_SIZE) / 2;
  ball_dx = direction;
  ball_dy = inherited_dy;
  ball_speed = 1;
  rally_hits = 0;
  serve_delay = SERVE_DELAY_FRAMES;
}

static void reset_match(void) {
  left_score = 0;
  right_score = 0;
  game_over = 0;
  left_y = (PONG_HEIGHT - LEFT_PADDLE_HEIGHT) / 2;
  right_y = (PONG_HEIGHT - RIGHT_PADDLE_HEIGHT) / 2;
  serve_ball(1, 0);
  old_left_y = left_y;
  old_right_y = right_y;
  old_ball_x = ball_x;
  old_ball_y = ball_y;
  needs_full_render = 1;
}

void pong_set_mode(int two_players) {
  multiplayer = two_players != 0;
}

void pong_init(void) {
  cleanLayer(LAYER_BG);
  cleanLayer(LAYER_SPRITE);
  reset_match();
}

int pong_input(int key) {
  if ((!multiplayer && key == BTN_B) ||
      (multiplayer && key == BTN_SELECT))
    return -1;

  if (game_over && (key == BTN_A || key == BTN_START)) {
    reset_match();
    return 1;
  }

  if (key == BTN_UP) {
    old_left_y = left_y;
    left_y = clamp_left_y(left_y - PADDLE_SPEED);
  } else if (key == BTN_DOWN) {
    old_left_y = left_y;
    left_y = clamp_left_y(left_y + PADDLE_SPEED);
  } else if (multiplayer && key == BTN_A) {
    old_right_y = right_y;
    right_y = clamp_right_y(right_y - PADDLE_SPEED);
  } else if (multiplayer && key == BTN_B) {
    old_right_y = right_y;
    right_y = clamp_right_y(right_y + PADDLE_SPEED);
  } else
    return 0;

  return 1;
}

static void move_cpu(void) {
  int paddle_center = right_y + RIGHT_PADDLE_HEIGHT / 2;
  int ball_center = ball_y + BALL_SIZE / 2;

  if (paddle_center < ball_center)
    right_y = clamp_right_y(right_y + 1);
  else if (paddle_center > ball_center)
    right_y = clamp_right_y(right_y - 1);
}

static int overlaps_vertical(int y1, int h1, int y2, int h2) {
  return y1 < y2 + h2 && y1 + h1 > y2;
}

static int bounce_angle(int paddle_y, int paddle_height) {
  static const int angle_by_zone[8] = {-2, -1, -1, 0, 1, 1, 2, 2};
  int relative_y = (ball_y + BALL_SIZE / 2) - paddle_y;
  int zone = (relative_y * 8) / paddle_height;

  if (zone < 0)
    zone = 0;
  if (zone > 7)
    zone = 7;
  return angle_by_zone[zone];
}

static void accelerate_rally(void) {
  rally_hits++;
  if (rally_hits % 3 == 0 && ball_speed < MAX_BALL_SPEED)
    ball_speed++;
}

static void score_point(int direction) {
  int inherited_dy = ball_dy;

  if (direction < 0)
    left_score++;
  else
    right_score++;

  /* Every point starts the next rally at the slowest speed. */
  ball_speed = 1;
  rally_hits = 0;

  if (left_score >= POINTS_TO_WIN || right_score >= POINTS_TO_WIN) {
    game_over = 1;
    return;
  }

  serve_ball(direction, inherited_dy);
}

void pong_update(void) {
  if (game_over)
    return;

  if (serve_delay > 0) {
    serve_delay--;
    return;
  }

  old_right_y = right_y;
  if (!multiplayer)
    move_cpu();

  old_ball_x = ball_x;
  old_ball_y = ball_y;

  ball_x += ball_dx * ball_speed;
  ball_y += ball_dy;

  if (ball_y <= 0) {
    ball_y = 0;
    if (ball_dy < 0)
      ball_dy = -ball_dy;
  } else if (ball_y >= PONG_HEIGHT - BALL_SIZE) {
    ball_y = PONG_HEIGHT - BALL_SIZE;
    if (ball_dy > 0)
      ball_dy = -ball_dy;
  }

  if (ball_dx < 0 && ball_x <= LEFT_PADDLE_X + PADDLE_WIDTH &&
      ball_x + BALL_SIZE >= LEFT_PADDLE_X &&
      overlaps_vertical(ball_y, BALL_SIZE, left_y, LEFT_PADDLE_HEIGHT)) {
    ball_x = LEFT_PADDLE_X + PADDLE_WIDTH;
    ball_dx = 1;
    ball_dy = bounce_angle(left_y, LEFT_PADDLE_HEIGHT);
    accelerate_rally();
  }

  if (ball_dx > 0 && ball_x + BALL_SIZE >= RIGHT_PADDLE_X &&
      ball_x <= RIGHT_PADDLE_X + PADDLE_WIDTH &&
      overlaps_vertical(ball_y, BALL_SIZE, right_y, RIGHT_PADDLE_HEIGHT)) {
    ball_x = RIGHT_PADDLE_X - BALL_SIZE;
    ball_dx = -1;
    ball_dy = bounce_angle(right_y, RIGHT_PADDLE_HEIGHT);
    accelerate_rally();
  }

  if (ball_x < -BALL_SIZE)
    score_point(1);
  else if (ball_x >= PONG_WIDTH)
    score_point(-1);
}

static const uint16_t * const score_digits[10] = {
    Num0, Num1, Num2, Num3, Num4,
    Num5, Num6, Num7, Num8, Num9,
};

static const int score_widths[10] = {6, 1, 6, 6, 6, 6, 6, 6, 6, 6};

static int score_number_width(int score) {
  int units = score % 10;

  if (score >= 10)
    return score_widths[score / 10] + 1 + score_widths[units];
  return score_widths[units];
}

static void draw_score_number(int score, int x) {
  int tens = score / 10;
  int units = score % 10;

  if (score >= 10) {
    draw_asset(LAYER_SPRITE, x, SCORE_Y, score_widths[tens],
               SCORE_DIGIT_HEIGHT, score_digits[tens]);
    x += score_widths[tens] + 1;
  }

  draw_asset(LAYER_SPRITE, x, SCORE_Y, score_widths[units],
             SCORE_DIGIT_HEIGHT, score_digits[units]);
}

static void draw_score(void) {
  int left_x = SCORE_LEFT_EDGE - score_number_width(left_score);

  fill_rect(LAYER_SPRITE, 15, SCORE_Y, 34, SCORE_DIGIT_HEIGHT,
            TRANSPARENT);
  draw_score_number(left_score, left_x);
  draw_score_number(right_score, SCORE_RIGHT_X);
}

void pong_render(void) {
  if (needs_full_render) {
    cleanLayer(LAYER_BG);
    cleanLayer(LAYER_SPRITE);
    draw_background(PongScenario);
    draw_asset(LAYER_SPRITE, LEFT_PADDLE_X, left_y,
               PADDLE_WIDTH, LEFT_PADDLE_HEIGHT, PongPlayer1);
    draw_asset(LAYER_SPRITE, RIGHT_PADDLE_X, right_y,
               PADDLE_WIDTH, RIGHT_PADDLE_HEIGHT, PongPlayer2);
    draw_asset(LAYER_SPRITE, ball_x, ball_y,
               BALL_SIZE, BALL_SIZE, PongBall);
    needs_full_render = 0;
  } else {
    move_sprite(LEFT_PADDLE_X, old_left_y, LEFT_PADDLE_X, left_y,
                PADDLE_WIDTH, LEFT_PADDLE_HEIGHT, PongPlayer1);
    move_sprite(RIGHT_PADDLE_X, old_right_y, RIGHT_PADDLE_X, right_y,
                PADDLE_WIDTH, RIGHT_PADDLE_HEIGHT, PongPlayer2);
    move_sprite(old_ball_x, old_ball_y, ball_x, ball_y,
                BALL_SIZE, BALL_SIZE, PongBall);
  }
  draw_score();
}
