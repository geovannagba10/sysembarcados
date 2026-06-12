#include "ButtonManager.h"

ButtonManager::ButtonManager(
  uint8_t pinButton1,
  uint8_t pinButton2,
  uint8_t pinButton3,
  uint8_t pinButton4,
  uint16_t debounceMs
) :
  button1(pinButton1, debounceMs),
  button2(pinButton2, debounceMs),
  button3(pinButton3, debounceMs),
  button4(pinButton4, debounceMs),
  buttonMask(0) {
}

void ButtonManager::begin() {
  button1.begin();
  button2.begin();
  button3.begin();
  button4.begin();

  updateMask();
}

void ButtonManager::update() {
  unsigned long currentTime = millis();

  button1.update(currentTime);
  button2.update(currentTime);
  button3.update(currentTime);
  button4.update(currentTime);

  updateMask();
}

uint16_t ButtonManager::getButtonMask() const {
  return buttonMask;
}

void ButtonManager::updateMask() {
  buttonMask = 0;

  if (button1.isPressed()) {
    buttonMask |= 1U << static_cast<uint8_t>(GamepadButton::BUTTON_1);
  }

  if (button2.isPressed()) {
    buttonMask |= 1U << static_cast<uint8_t>(GamepadButton::BUTTON_2);
  }

  if (button3.isPressed()) {
    buttonMask |= 1U << static_cast<uint8_t>(GamepadButton::BUTTON_3);
  }

  if (button4.isPressed()) {
    buttonMask |= 1U << static_cast<uint8_t>(GamepadButton::BUTTON_4);
  }
}