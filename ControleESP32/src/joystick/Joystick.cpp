#include "Joystick.h"

Joystick::Joystick(uint8_t pinX, uint8_t pinY, int deadzone) {
  this->pinX = pinX;
  this->pinY = pinY;

  this->deadzone = deadzone;

  // Valores provisórios utilizados até a calibração inicial.
  this->centerX = ADC_MAX_VALUE / 2;
  this->centerY = ADC_MAX_VALUE / 2;
}

void Joystick::begin() {
  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);

  /*
   * Permite utilizar uma faixa de tensão maior nos canais ADC. A documentação oficial explica que a atenuação interna é justamente o recurso utilizado para medir tensões maiores que a tensão de referência.
   */
  analogSetPinAttenuation(pinX, ADC_11db);
  analogSetPinAttenuation(pinY, ADC_11db);
}

void Joystick::calibrate(uint16_t numberOfSamples) {
  long sumX = 0;
  long sumY = 0;

  for (uint16_t sample = 0; sample < numberOfSamples; sample++) {
    sumX += analogRead(pinX);
    sumY += analogRead(pinY);

    delay(2);
  }

  centerX = sumX / numberOfSamples;
  centerY = sumY / numberOfSamples;
}

Axis2D Joystick::read() const {
  Axis2D reading;

  reading.x = normalizeAxis(analogRead(pinX), centerX);
  reading.y = normalizeAxis(analogRead(pinY), centerY);

  return reading;
}

int8_t Joystick::normalizeAxis(int rawValue, int center) const {
  int delta = rawValue - center;

  /*
   * Valores próximos ao centro são ignorados.
   */
  if (abs(delta) <= deadzone) {
    return 0;
  }

  int maximumDelta;

  if (delta > 0) {
    maximumDelta = ADC_MAX_VALUE - center;
  } else {
    maximumDelta = center;
  }

  int usableRange = maximumDelta - deadzone;

  if (usableRange <= 0) {
    return 0;
  }

  long normalizedValue =
    static_cast<long>(abs(delta) - deadzone) * 127L / usableRange;

  normalizedValue = constrain(normalizedValue, 0L, 127L);

  if (delta < 0) {
    normalizedValue = -normalizedValue;
  }

  return static_cast<int8_t>(normalizedValue);
}