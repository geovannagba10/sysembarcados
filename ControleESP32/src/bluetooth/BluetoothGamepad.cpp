#include "BluetoothGamepad.h"

BluetoothGamepad::BluetoothGamepad()
  : bleGamepad(
      "ESP32 Gamepad",
      "PMR3402 Grupo G",
      100
    ),
    configuration() {
}

void BluetoothGamepad::begin() {
  /*
   * Envio manual:
   * atualizamos todos os campos e enviamos um único pacote completo.
   */
  configuration.setAutoReport(false);

  /*
   * O protótipo atual possui quatro botões físicos.
   */
  configuration.setButtonCount(4);

  /*
   * Não utilizamos D-pad nesta etapa.
   */
  configuration.setHatSwitchCount(0);

  /*
   * Um único joystick:
   * - X habilitado;
   * - Y habilitado;
   * - demais eixos desabilitados.
   */
  configuration.setWhichAxes(
    true,   // X
    true,   // Y
    false,  // Z
    false,  // RX
    false,  // RY
    false,  // RZ
    false,  // Slider 1
    false   // Slider 2
  );

  /*
   * Sua classe Joystick normaliza cada eixo entre -127 e 127.
   */
  configuration.setAxesMin(-127);
  configuration.setAxesMax(127);

  /*
   * Reintroduz a IMU no descritor HID mantendo o restante
   * do controle no formato minimo que se mostrou estavel.
   */
  configuration.setIncludeAccelerometer(true);
  configuration.setIncludeGyroscope(true);
  configuration.setMotionMin(-32767);
  configuration.setMotionMax(32767);

  /*
   * Um byte de Output Report para o rumble remoto:
   * 0 desliga; 1..255 ajustam a intensidade.
   */
  configuration.setEnableOutputReport(true);
  configuration.setOutputReportLength(OUTPUT_REPORT_LENGTH);

  bleGamepad.begin(&configuration);
}

bool BluetoothGamepad::isConnected() {
  return bleGamepad.isConnected();
}

void BluetoothGamepad::updateButtons(uint16_t buttonMask) {
  for (uint8_t index = 0; index < 4; index++) {
    uint16_t mask = 1U << index;

    /*
     * A biblioteca enumera os botões a partir de BUTTON_1.
     */
    uint8_t bluetoothButton =
      static_cast<uint8_t>(BUTTON_1 + index);

    if (buttonMask & mask) {
      bleGamepad.press(bluetoothButton);
    } else {
      bleGamepad.release(bluetoothButton);
    }
  }
}

void BluetoothGamepad::sendReport(
  const GamepadReport &report
) {
  if (!isConnected()) {
    return;
  }

  /*
   * Único joystick enviado como analógico esquerdo.
   *
   * Mantivemos os nomes joystick2X e joystick2Y no projeto
   * porque este é fisicamente o antigo Joystick 2.
   */
  bleGamepad.setLeftThumb(
    report.joystick2X,
    report.joystick2Y
  );

  updateButtons(report.buttons);

  if (report.imu.valid) {
    bleGamepad.setMotionControls(
      report.imu.gyroX,
      report.imu.gyroY,
      report.imu.gyroZ,
      report.imu.accelX,
      report.imu.accelY,
      report.imu.accelZ
    );
  }

  /*
   * Envia um pacote HID consolidado.
   */
  bleGamepad.sendReport();
}

bool BluetoothGamepad::readRumbleIntensity(
  uint8_t &intensity
) {
  if (bleGamepad.isOutputReceived()) {
    uint8_t *outputBuffer =
      bleGamepad.getOutputBuffer();

    if (outputBuffer != nullptr) {
      intensity = outputBuffer[0];
      return true;
    }
  }

  return false;
}
