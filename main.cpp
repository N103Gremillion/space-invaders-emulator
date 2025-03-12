#include <iostream>
#include <SDL2/SDL.h>
#include "./CPU/8080.hpp"
#include "./CPU/screen.hpp"

int main() {
  Screen* screen = new Screen();
  run();
  return 0;
}