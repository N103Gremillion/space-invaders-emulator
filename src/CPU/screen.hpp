#ifndef SCREEN_HPP
#define SCREEN_HPP

#include <SDL2/SDL.h>
#include <cstdint>

#define TOTAL_PIXELS (256 * 224)
#define PIXELS_PER_ROW 256
#define PIXELS_PER_COLUMN 224
#define SCREEN_SCALER 4
#define GREEN 0x19FF19
#define WHITE 0xFFFFFF
#define BLACK 0x000000
#define RED 0xFF0000

using u32 = std::uint32_t;

class Screen {
  public:
    // pixels
    int pixel_w = 4;
    int pixel_h = 4;
    int window_w = PIXELS_PER_ROW * SCREEN_SCALER;
    int window_h = PIXELS_PER_COLUMN * SCREEN_SCALER;
    Screen();
    ~Screen();
    void render_screen();

  private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;  
    u32* pixels = nullptr;

};

#endif