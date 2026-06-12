#pragma once
#include <Arduino.h>

namespace Pins {

  // Joystick 1
  constexpr uint8_t JOYSTICK_1_X = 32;
  constexpr uint8_t JOYSTICK_1_Y = 33;

  // Joystick 2
  constexpr uint8_t JOYSTICK_2_X = 34;
  constexpr uint8_t JOYSTICK_2_Y = 35;

  // Sensor inercial por I2C
  constexpr uint8_t IMU_SDA = 21;
  constexpr uint8_t IMU_SCL = 22;

  // Botões indicados na imagem
  constexpr uint8_t BUTTON_1 = 12;
  constexpr uint8_t BUTTON_2 = 13;
  constexpr uint8_t BUTTON_3 = 14;
  constexpr uint8_t BUTTON_4 = 27;
}