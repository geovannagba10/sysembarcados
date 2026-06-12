#pragma once

#include <Arduino.h>

/**
 * Representa um botão digital com debounce.
 *
 * Convenção elétrica:
 * - botão solto: GPIO em HIGH;
 * - botão pressionado: GPIO em LOW.
 *
 * O botão deve ser conectado entre o GPIO e o GND.
 */
class Button {
  private:
    uint8_t pin;
    uint16_t debounceMs;

    bool stablePressed;
    bool lastRawPressed;

    unsigned long lastRawChangeTime;

  public:
    Button(uint8_t pin, uint16_t debounceMs = 30);

    void begin();

    /**
     * Atualiza o estado interno considerando o debounce.
     */
    void update(unsigned long currentTime);

    /**
     * Retorna true quando o botão está pressionado de forma estável.
     */
    bool isPressed() const;
};