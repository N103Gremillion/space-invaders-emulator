#include <iostream>
#include <SDL2/SDL.h>
#include "8080.hpp"
#include "screen.hpp"

int main() {
  Screen* screen = new Screen();
  run();
  return 0;
}