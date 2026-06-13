#pragma once

#include <Arduino.h>
#include <BleGamepad.h>

#include "../model/ControllerTypes.h"

/**
 * Gerencia a comunicação Bluetooth HID com o computador.
 *
 * Responsabilidades:
 * - iniciar o dispositivo BLE;
 * - informar se existe um host conectado;
 * - enviar joystick, botões e IMU.
 */
class BluetoothGamepad {
  private:
    static constexpr uint8_t OUTPUT_REPORT_LENGTH = 1;

    BleGamepad bleGamepad;
    BleGamepadConfiguration configuration;

    /**
     * Atualiza o estado dos quatro botões no relatório HID.
     */
    void updateButtons(uint16_t buttonMask);

  public:
    BluetoothGamepad();

    /**
     * Inicializa o Bluetooth e começa o advertising.
     */
    void begin();

    /**
     * Retorna true quando existe um computador conectado.
     */
    bool isConnected();

    /**
     * Envia ao computador o estado mais recente do controle.
     */
    void sendReport(const GamepadReport &report);

    /**
     * Lê a intensidade de rumble enviada pelo host via Output Report.
     *
     * 0 desliga o motor; 1..255 ajustam a intensidade PWM.
     */
    bool readRumbleIntensity(uint8_t &intensity);
};
