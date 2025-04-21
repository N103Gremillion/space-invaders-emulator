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
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, NUM_OF_COLUMNS, NUM_OF_ROWS);
}

int Screen::determine_pixel_color(int bit, int y) {
  if (bit == 0) {
    return BACKGROUND_COLOR;
  } else {
    if (y <= TOP_CUTOFF) {
      return TOP_SCREEN;
    } else if (y > TOP_CUTOFF and y <= ENEMIES_CUTOFF) {
      return ENEMIES;
    } else if (y > ENEMIES_CUTOFF and y <= SHIELDS_CUTOFF) {
      return SHIELDS;
    } else {
      return SPACESHIP;
    }
  }
}

// note: pixles are draw from bottom left vertially from VRAM
void Screen::change_pixels(_8080* cpu) {
  int byte_num = 1;
  u8 cur_byte = 0;
  int cur_column = 0;
  int cur_row = NUM_OF_ROWS - 1;
  for (uint16_t address = VRAM_START; address <= VRAM_END; address++) {
    cur_byte = cpu->memory[address];
    for (int i = 0; i < 8; i++) {
      int bit = (cur_byte >> i) & 1;
      int row = cur_row - i;
      pixels[((cur_row - i) * NUM_OF_COLUMNS) + cur_column] = determine_pixel_color(bit, cur_row);
    }
    cur_row-=8;
    if (byte_num % BYTES_PER_COLMN == 0 && byte_num != 0) {
      cur_column++;
      cur_row = NUM_OF_ROWS - 1;
    }
    byte_num++;
  }
}

void Screen::render_screen(_8080* cpu) {
  SDL_RenderClear(renderer);
  change_pixels(cpu);
  SDL_UpdateTexture(texture, NULL, pixels, NUM_OF_COLUMNS * sizeof(u32));
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