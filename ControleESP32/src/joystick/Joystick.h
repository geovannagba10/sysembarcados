#pragma once

#include <Arduino.h>
#include "../model/ControllerTypes.h"

/**
 * Representa um joystick analógico de dois eixos.
 *
 * Responsabilidades:
 * - configurar os pinos ADC;
 * - calibrar a posição central;
 * - ler os dois eixos;
 * - aplicar a zona morta;
 * - normalizar os valores para a escala HID de -127 a 127.
 */
class Joystick {
  private:
    static constexpr int ADC_MAX_VALUE = 4095;

    uint8_t pinX;
    uint8_t pinY;

    int centerX;
    int centerY;

    int deadzone;

    int8_t normalizeAxis(int rawValue, int center) const;

  public:
    Joystick(uint8_t pinX, uint8_t pinY, int deadzone = 180);

    void begin();

    /**
     * Calcula o valor central médio do joystick.
     * O controle deve permanecer parado durante a inicialização.
     */
    void calibrate(uint16_t numberOfSamples = 200);

    /**
     * Retorna os dois eixos já tratados e normalizados.
     */
    Axis2D read() const;
};