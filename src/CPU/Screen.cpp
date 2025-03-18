#include "Screen.hpp"

Screen::Screen() {

  pixels = new u32[TOTAL_PIXELS];

  // initalize all black 
  for (int i = 0; i < TOTAL_PIXELS; i++) {
    pixels[i]  = BLACK;
  }

  window = SDL_CreateWindow("Space Invaders Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,  window_w, window_h, 0);

  // shift the window to appropriate position
  int window_x;
  int window_y;
  SDL_GetWindowPosition(window, &window_x, &window_y);
  SDL_SetWindowPosition(window, window_x * 1.5, window_y);
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