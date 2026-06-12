#pragma once

#include <Arduino.h>
#include "Button.h"

/**
 * Cada botão corresponde a um bit do relatório enviado ao computador.
 */
enum class GamepadButton : uint8_t {
  ACTION_1 = 0,
  ACTION_2 = 1,
  ACTION_3 = 2,
  ACTION_4 = 3
};

/**
 * Gerencia todos os botões digitais do controle.
 *
 * Responsabilidades:
 * - inicializar os botões;
 * - atualizar o debounce de cada botão;
 * - reunir os estados em uma máscara de bits.
 */
class ButtonManager {
  private:
    Button button1;
    Button button2;
    Button button3;
    Button button4;

    uint16_t buttonMask;

    void updateMask();

  public:
    ButtonManager(
      uint8_t pinButton1,
      uint8_t pinButton2,
      uint8_t pinButton3,
      uint8_t pinButton4,
      uint16_t debounceMs = 30
    );

    void begin();

    /**
     * Atualiza o estado de todos os botões.
     *
     * Deve ser chamado periodicamente pela tarefa do FreeRTOS.
     */
    void update();

    /**
     * Retorna uma máscara de bits representando os botões pressionados.
     */
    uint16_t getButtonMask() const;
};
