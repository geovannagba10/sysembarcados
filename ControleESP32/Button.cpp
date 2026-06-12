#include "Button.h"

Button::Button(uint8_t pin, uint16_t debounceMs) {
  this->pin = pin;
  this->debounceMs = debounceMs;

  this->stablePressed = false;
  this->lastRawPressed = false;
  this->lastRawChangeTime = 0;
}

void Button::begin() {
  pinMode(pin, INPUT_PULLUP);

  /*
   * Como utilizamos INPUT_PULLUP:
   * - LOW  significa botão pressionado;
   * - HIGH significa botão solto.
   */
  bool initialPressed = digitalRead(pin) == LOW;

  stablePressed = initialPressed;
  lastRawPressed = initialPressed;
  lastRawChangeTime = millis();
}

void Button::update(unsigned long currentTime) {
  bool rawPressed = digitalRead(pin) == LOW;

  /*
   * Caso a leitura bruta mude, iniciamos novamente a contagem
   * do tempo necessário para considerar a mudança estável.
   */
  if (rawPressed != lastRawPressed) {
    lastRawPressed = rawPressed;
    lastRawChangeTime = currentTime;
  }

  /*
   * A mudança somente é aceita após permanecer constante
   * durante o intervalo de debounce.
   */
  if (
    currentTime - lastRawChangeTime >= debounceMs &&
    stablePressed != lastRawPressed
  ) {
    stablePressed = lastRawPressed;
  }
}

bool Button::isPressed() const {
  return stablePressed;
}