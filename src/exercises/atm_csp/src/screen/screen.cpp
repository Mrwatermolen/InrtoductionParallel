#include "screen/screen.h"

#include "screen/screen_state.h"

void Screen::init() {
  transition(std::make_unique<ScreenStateWaiting>(incomer()));
}
