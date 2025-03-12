#include "keys.hpp"


bool keys[116] = {false};

void handle_key_press(int keycode) {
  switch (keycode) {
    case ESC:
      keys[ESC] = true;
      break;
    case SPACE:
      keys[SPACE] = true;
      break;
    case _1:
      keys[_1] = true;
      break;
    case _2:
      keys[_2] = true;
      break;
    case A:
      keys[A] = true;
      break;
    case C:
      keys[C] = true;
      break;
    case D:
      keys[D] = true;
      break;
    case P:
      keys[P] = true;
      break;
    case S:
      keys[S] = true;
      break;
    case LEFT_ARROW:
      keys[LEFT_ARROW_INDEX] = true;
      break;
    case RIGHT_ARROW:
      keys[RIGHT_ARROW_INDEX] = true;
      break;
    default:
      break;
  }
}

void handle_key_release(int keycode) {
  switch (keycode) {
    case ESC:
      keys[ESC] = false;
      break;
    case SPACE:
      keys[SPACE] = false;
      break;
    case _1:
      keys[_1] = false;
      break;
    case _2:
      keys[_2] = false;
      break;
    case A:
      keys[A] = false;
      break;
    case C:
      keys[C] = false;
      break;
    case D:
      keys[D] = false;
      break;
    case P:
      keys[P] = false;
      break;
    case S:
      keys[S] = false;
      break;
    case LEFT_ARROW:
      keys[LEFT_ARROW_INDEX] = false;
      break;
    case RIGHT_ARROW:
      keys[RIGHT_ARROW_INDEX] = false;
      break;
    default:
      break;
  }
}
