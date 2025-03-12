#ifndef SCREEN_HPP
#define SCREEN_HPP

#include <SDL2/SDL.h>

typedef struct Screen {
  SDL_Window* window;
  int window_w = 1024;
  int window_h = 896;
  Screen() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Space Invaders Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,  window_w, window_h, SDL_WINDOW_RESIZABLE);
  }
} Screen;

#endif