#pragma once
#include <Arduino.h>

namespace Pins {

  // Joystick 2
  constexpr uint8_t JOYSTICK_2_X = 34;
  constexpr uint8_t JOYSTICK_2_Y = 35;

  // Sensor inercial por I2C
  constexpr uint8_t IMU_SDA = 21;
  constexpr uint8_t IMU_SCL = 22;

  // Botões indicados na imagem
  constexpr uint8_t BUTTON_PIN_1 = 12;
  constexpr uint8_t BUTTON_PIN_2 = 13;
  constexpr uint8_t BUTTON_PIN_3 = 14;
  constexpr uint8_t BUTTON_PIN_4 = 27;

  // Motor de vibração (GPIO livre para o motor ou transistor)
  constexpr uint8_t VIBRATION_MOTOR = 4;
}
