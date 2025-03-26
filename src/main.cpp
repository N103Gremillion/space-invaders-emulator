#include <iostream>
#include <SDL2/SDL.h>
#include "./CPU/8080.hpp"
#include "./CPU/Screen.hpp"

#define ROM_FILE_STRING "./invaders/invaders"
#define INVADERS_H_START 0x0000
#define INVADERS_G_START 0x0800
#define INVADERS_F_START 0x1000
#define INVADERS_E_START 0x1800

// note this is indecated by pc but I have this for debugging
u16 start_address = 0x0000;

int main() {
  _8080* _8080_ = new _8080();
  _8080_->regs->pc = start_address;
  _8080_->load_rom("../invaders/invaders.h", INVADERS_H_START);
  _8080_->load_rom("../invaders/invaders.g", INVADERS_G_START);
  _8080_->load_rom("../invaders/invaders.f", INVADERS_F_START);
  _8080_->load_rom("../invaders/invaders.e", INVADERS_E_START);
  
  _8080_->run();
  return 0;
}