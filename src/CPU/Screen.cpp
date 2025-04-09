#include "Screen.hpp"

Screen::Screen() {

  // initalize the memory for the pixels
  pixels = (u32*) malloc(sizeof(u32) * TOTAL_PIXELS);

  // initalize all black 
  for (int i = 0; i < TOTAL_PIXELS; i++) {
    pixels[i] = BACKGROUND_COLOR;
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

int Screen::determine_pixel_color(int bit, int y) {
  if (bit == 0) {
    return BACKGROUND_COLOR;
  }
  if (y <= SPACESHIP_CUTOFF) {
    return SPACESHIP;
  } else if (y <= SHIELDS_CUTOFF && y < ) {
      return SHIELDS;
  } else if (y <= ENEMIES_CUTOFF) {
      return ENEMIES;
  } else {
      return TOP_SCREEN;
  }
}

// note: pixles are draw from bottom left vertially from VRAM
void Screen::change_pixels(u8* memory) {
  int byte_num = 1;
  u8 cur_byte = 0;
  int cur_column = 0;
  int cur_row = PIXELS_PER_ROW - 1;
  for (uint16_t address = VRAM_START; address <= VRAM_END; address++) {
    cur_byte = memory[address];
    // mirror over each bit
    for (int i = 0; i < 8; i++) {
      int bit = (cur_byte >> 7) & 1;
      cur_byte = cur_byte << 1;
      std::cout << "The row is " << cur_row << std::endl;
      std::cout << "The column is " << cur_column << std::endl;
      std::cout << "The byte number is " << byte_num << std::endl;
      pixels[cur_row][cur_column] = determine_pixel_color(bit, cur_column, cur_row);
      cur_row--;
    }
    if (byte_num % 28 == 0 && byte_num != 0) {
      cur_column++;
      cur_row = PIXELS_PER_ROW - 1;
    }
    byte_num++;
  }
}

void Screen::render_screen(u8* memory) {
  change_pixels(memory);
  SDL_UpdateTexture(texture, NULL, pixels, PIXELS_PER_ROW * sizeof(u32));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

Screen::~Screen() {
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  free(pixels);
  SDL_Quit();
}