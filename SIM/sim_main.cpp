#include "HUB75.h"
#include "VfemtoRV32_bench.h"
#include "verilated.h"
#include <GLFW/glfw3.h>
#include <fenv.h>
#include <xmmintrin.h>
#include <cstdint>
#include <memory>

namespace {

struct KeyBinding {
  int glfw_key;
  uint32_t btn_bit;
};

constexpr KeyBinding kBindings[] = {
    {GLFW_KEY_Z, 0u},      // BTN_A
    {GLFW_KEY_X, 1u},      // BTN_B
    {GLFW_KEY_RIGHT, 2u},  // BTN_RIGHT
    {GLFW_KEY_LEFT, 3u},   // BTN_LEFT
    {GLFW_KEY_UP, 4u},     // BTN_UP (optional / if used)
    {GLFW_KEY_DOWN, 5u},   // BTN_DOWN (optional / if used)
};

} // namespace

int main(int argc, char **argv, char **env) {
  (void)env;
  Verilated::commandArgs(argc, argv);

  fesetround(FE_TOWARDZERO);
  _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

  auto top = std::make_unique<VfemtoRV32_bench>();
  HUB75 screen(
      top->r0, top->g0, top->b0,
      top->r1, top->g1, top->b1,
      top->addr, top->clk_out, top->latch, top->oe);

  top->pclk = 0;
  top->BUTTONS = ~0u; // Active-low: all buttons unpressed
  top->eval();

  uint32_t prev_pressed_mask = 0;

  while (!Verilated::gotFinish() && !screen.shouldClose()) {
    glfwPollEvents();

    // Read current GLFW key states into a bitmask (1 = key currently held)
    uint32_t curr_pressed_mask = 0;
    for (const auto &b : kBindings) {
      if (screen.isKeyPressed(b.glfw_key)) {
        curr_pressed_mask |= (1u << b.btn_bit);
      }
    }

    // Pulse each newly-pressed key for a few simulation cycles so Buttons.v
    // sees a clean transition without flooding the queue while held.
    // (Adjust pulse_hold_cycles if your clk vs pclk ratio differs.)
    static int pulse_ticks_left[8] = {0};
    uint32_t newly_pressed = curr_pressed_mask & ~prev_pressed_mask;

    for (const auto &b : kBindings) {
      if (newly_pressed & (1u << b.btn_bit)) {
        pulse_ticks_left[b.btn_bit] = 8;
      }
    }

    uint32_t active_low_buttons = ~0u;
    for (const auto &b : kBindings) {
      if (pulse_ticks_left[b.btn_bit] > 0) {
        active_low_buttons &= ~(1u << b.btn_bit);
        pulse_ticks_left[b.btn_bit]--;
      }
    }

    top->BUTTONS = active_low_buttons;
    prev_pressed_mask = curr_pressed_mask;

    screen.eval();
    top->pclk = !top->pclk;
    top->eval();
  }

  top->final();
  return 0;
}
