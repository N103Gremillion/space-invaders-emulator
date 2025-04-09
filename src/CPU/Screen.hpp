#ifndef SCREEN_HPP
#define SCREEN_HPP

#include <SDL2/SDL.h>
#include <cstdint>
#include <iostream>

#define TOTAL_PIXELS (224 * 256)
#define PIXELS_PER_ROW 224
#define PIXELS_PER_COLUMN 256
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

class Screen {
  public:
    // pixels
    int pixel_w = 1 * SCREEN_SCALER;
    int pixel_h = 1 * SCREEN_SCALER;
    int window_w = PIXELS_PER_ROW * SCREEN_SCALER;
    int window_h = PIXELS_PER_COLUMN * SCREEN_SCALER;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    Screen();
    ~Screen();
    int determine_pixel_color(int bit, int y);
    void change_pixels(u8* memory);
    void render_screen(u8* memory);

  private:
    SDL_Texture* texture = nullptr;  
    u32* pixels = nullptr;

};

#endif