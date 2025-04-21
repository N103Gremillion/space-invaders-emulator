#include "keys.hpp"


bool keys[116] = {false};
bool inputs[NUM_INPUTS] = {false};

void handle_key_press(int keycode) {
  switch (keycode) {
    case ESC:
      keys[ESC] = true;
      break;
    case SPACE:
      keys[SPACE] = true;
      inputs[SPACE_KEY] = true;  // Shooting action
      break;
    case _1:
      keys[_1] = true;
      break;
    case _2:
      keys[_2] = true;
      break;
    case A:
      keys[A] = true;
      inputs[A_KEY] = true;  // Move left
      break;
    case C:
      keys[C] = true;
      break;
    case D:
      keys[D] = true;
      inputs[D_KEY] = true;  // Move right
      break;
    case I:
      keys[I] = true;
      inputs[INSERT_COIN] = true;
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
      inputs[SPACE_KEY] = false;  // Stop shooting
      break;
    case _1:
      keys[_1] = false;
      break;
    case _2:
      keys[_2] = false;
      break;
    case A:
      keys[A] = false;
      inputs[A_KEY] = false;  // Stop moving left
      break;
    case C:
      keys[C] = false;
      break;
    case D:
      keys[D] = false;
      inputs[D_KEY] = false;  // Stop moving right
      break;
    case I:
      keys[I] = false;
      inputs[INSERT_COIN] = false;
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

