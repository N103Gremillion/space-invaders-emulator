#ifndef SCREEN_HPP
#define SCREEN_HPP

#include "8080.hpp"
#include <SDL2/SDL.h>
#include <cstdint>
#include <iostream>

#define TOTAL_PIXELS (256 * 224)
#define NUM_OF_COLUMNS 224
#define NUM_OF_ROWS 256
#define BYTES_PER_COLMN 32
#define BYTES_PER_ROW 28
#define SCREEN_SCALER 2
#define VRAM_START 0x2400
#define VRAM_END 0x3FFF

#define BACKGROUND_COLOR  0xFF000000  // Black
#define SPACESHIP         0xFF42E9F4  // Cyan / Player
#define SHIELDS           0xFF62DE6D  // Green
#define ENEMIES           0xFFF83B3A  // Red
#define TOP_SCREEN        0xFFDB55DD  // Magenta / UI

#define SPACESHIP_CUTOFF 256
#define SHIELDS_CUTOFF 225
#define ENEMIES_CUTOFF 155
#define TOP_CUTOFF 55

using u8 = std::uint8_t;
using u32 = std::uint32_t;

class _8080;

class Screen {
  public:
    // pixels
    int pixel_w = 1 * SCREEN_SCALER;
    int pixel_h = 1 * SCREEN_SCALER;
    int window_w = NUM_OF_COLUMNS* SCREEN_SCALER;
    int window_h = NUM_OF_ROWS * SCREEN_SCALER;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    Screen();
    ~Screen();
    int determine_pixel_color(int bit, int y);
    void change_pixels(_8080* cpu);
    void render_screen(_8080* cpu);

  private:
    SDL_Texture* texture = nullptr;  
    u32* pixels = nullptr;

};

#endif