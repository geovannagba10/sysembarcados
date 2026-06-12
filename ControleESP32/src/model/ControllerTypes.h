#pragma once

#include <stdint.h>

/**
 * Valor normalizado de um joystick com dois eixos.
 *
 * Cada eixo varia de -127 a 127.
 */
struct Axis2D {
  int8_t x;
  int8_t y;
};

/**
 * Estado de leitura do IMU.
 */
struct IMUReport {
  int16_t accelX = 0;
  int16_t accelY = 0;
  int16_t accelZ = 0;

  int16_t gyroX = 0;
  int16_t gyroY = 0;
  int16_t gyroZ = 0;

  bool valid = false;
};

/**
 * Identifica qual componente produziu uma atualização.
 */
enum class InputSource : uint8_t {
  JOYSTICK_2,
  BUTTONS
};

/**
 * Mensagem interna enviada pelas tarefas de leitura.
 */
struct InputUpdate {
  InputSource source;

  int16_t value1;
  int16_t value2;

  uint16_t buttons;
};

/**
 * Estado completo do controle.
 */
struct GamepadReport {
  int8_t joystick2X = 0;
  int8_t joystick2Y = 0;

  uint16_t buttons = 0;

  IMUReport imu{};

};