#include <Arduino.h>

#include "src/controller/GamepadController.h"

constexpr bool ENABLE_SERIAL_DEBUG = true;
constexpr uint16_t DEBUG_PERIOD_MS = 250;

GamepadController controller;

void setup() {
  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("=== ESP32 iniciou ===");
  Serial.println("[1] Chamando controller.begin()...");

  if (!controller.begin()) {
    Serial.println("[ERRO] Nao foi possivel inicializar o controle.");

    for (;;) {
      delay(1000);
    }
  }

  Serial.println("[OK] Controle inicializado com sucesso.");
}

void loop() {
  // Main loop idle: communications task handles serial I/O and printing.
  vTaskDelay(
    pdMS_TO_TICKS(DEBUG_PERIOD_MS)
  );
}