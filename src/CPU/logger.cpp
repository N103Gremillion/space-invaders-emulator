#include "logger.hpp"

void log_key_press(int keycode) {
   cout << "Key Pressed: " << SDL_GetKeyName(keycode) << " with keycode " << keycode << endl;
}