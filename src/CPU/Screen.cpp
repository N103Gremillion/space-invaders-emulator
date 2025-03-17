#include "Screen.hpp"

Screen::Screen() {
  SDL_Init(SDL_INIT_VIDEO);

  pixels = new u32[TOTAL_PIXELS];

  // initalize all black 
  for (int i = 0; i < TOTAL_PIXELS; i++) {
    pixels[i]  = BLACK;
  }

  window = SDL_CreateWindow("Space Invaders Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,  window_w, window_h, SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, PIXELS_PER_ROW, PIXELS_PER_COLUMN);
}

void Screen::render_screen() {
  SDL_UpdateTexture(texture, NULL, pixels, PIXELS_PER_ROW * sizeof(u32));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

Screen::~Screen() {
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  delete[] pixels;
  SDL_Quit();
}