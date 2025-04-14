#include <iostream>
#include <SDL2/SDL.h>
#include "./CPU/8080.hpp"
#include "./CPU/Screen.hpp"

#define ROM_FILE_STRING "./invaders/invaders"
#define INVADERS_H_START 0x0000
#define INVADERS_G_START 0x0800
#define INVADERS_F_START 0x1000
#define INVADERS_E_START 0x1800

#define INVADERS_H_FILE "../invaders/invaders.h"
#define INVADERS_G_FILE "../invaders/invaders.g"
#define INVADERS_F_FILE "../invaders/invaders.f"
#define INVADERS_E_FILE "../invaders/invaders.e"

#define TEST1_FILE "../cpu_tests/8080EXM.COM"
#define TEST2_FILE "../cpu_test/8080EXER.COM"
#define TEST3_FILE "../cpu_test/CPUTEST.COM"
#define TEST4_FILE "../cpu_test/TST8080.COM"

// note this is indicated by pc but I have this for debugging
u16 space_invaders_start_address = 0x0000;

void setup_space_invaders(_8080* _8080_) {
  _8080_->regs->pc = space_invaders_start_address;
  _8080_->load_rom(INVADERS_H_FILE, INVADERS_H_START);
  _8080_->load_rom(INVADERS_G_FILE, INVADERS_G_START);
  _8080_->load_rom(INVADERS_F_FILE, INVADERS_F_START);
  _8080_->load_rom(INVADERS_E_FILE, INVADERS_E_START);
}

void setup_test(_8080* _8080_) {
  // Load the cpudiag.bin file at 0x100
  _8080_->load_rom(TEST1_FILE, 0x100);
  _8080_->regs->pc = 0x0100;
}

int main() {
  _8080* _8080_ = new _8080();
  setup_test(_8080_);
  _8080_->run_test();
  return 0;
}