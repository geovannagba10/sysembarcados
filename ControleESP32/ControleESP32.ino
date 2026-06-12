#include <Arduino.h>

#include "GamepadController.h"

constexpr bool ENABLE_SERIAL_DEBUG = true;
constexpr uint16_t DEBUG_PERIOD_MS = 250;

GamepadController controller;

void setup() {
  Serial.begin(115200);

  if (!controller.begin()) {
    Serial.println(
      "Erro: nao foi possivel inicializar o controle."
    );

    for (;;) {
      delay(1000);
    }
  }

  Serial.println("Controle inicializado com sucesso.");
}

void loop() {
  if (ENABLE_SERIAL_DEBUG) {
    GamepadReport report;

    if (controller.readLatestReport(report)) {
      Serial.printf(
        "J1: X=%4d Y=%4d | "
        "J2: X=%4d Y=%4d | "
        "Botoes: 0x%04X\n",
        report.joystick1X,
        report.joystick1Y,
        report.joystick2X,
        report.joystick2Y,
        report.buttons
      );
    }
  }

  vTaskDelay(
    pdMS_TO_TICKS(DEBUG_PERIOD_MS)
  );
}