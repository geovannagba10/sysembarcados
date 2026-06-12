#include "GamepadController.h"
#include "../config/Pinos.h"
GamepadController::GamepadController()
  : joystick2(
      Pins::JOYSTICK_2_X,
      Pins::JOYSTICK_2_Y),
    buttons(
      Pins::BUTTON_PIN_1,
      Pins::BUTTON_PIN_2,
      Pins::BUTTON_PIN_3,
      Pins::BUTTON_PIN_4),
    imu(),
    vibrationMotor(Pins::VIBRATION_MOTOR),
    bluetoothGamepad(),
    bluetoothReady(false),
    inputQueue(nullptr),
    reportMailbox(nullptr),
    imuMailbox(nullptr),
    joystickTaskHandle(nullptr),
    buttonsTaskHandle(nullptr),
    imuTaskHandle(nullptr),
    commTaskHandle(nullptr),
    processInputsTaskHandle(nullptr) {
}

void GamepadController::runCommTask() {
  TickType_t lastWakeTime = xTaskGetTickCount();
  TickType_t lastDebugTime = xTaskGetTickCount();

  bool previousConnectionState = false;

  for (;;) {
    GamepadReport report{};
    bool hasReport =
      readLatestReport(
        report,
        0
      );

    if (!bluetoothReady) {
      vTaskDelayUntil(
        &lastWakeTime,
        pdMS_TO_TICKS(COMM_PERIOD_MS)
      );
      continue;
    }

    bool connected =
      bluetoothGamepad.isConnected();

    /*
     * Imprime somente quando o estado da conexão muda.
     */
    if (connected != previousConnectionState) {
      if (connected) {
        Serial.println(
          "[BLE] Computador conectado."
        );
      } else {
        Serial.println(
          "[BLE] Computador desconectado."
        );
      }

      previousConnectionState = connected;
    }

    /*
     * Mantém o HID com descritor mínimo por estabilidade,
     * mas já usa o relatório consolidado do projeto.
     */
    if (connected && hasReport) {
      bluetoothGamepad.sendReport(report);
    }

    TickType_t currentTime =
      xTaskGetTickCount();

    if (
      currentTime - lastDebugTime >=
      pdMS_TO_TICKS(DEBUG_PERIOD_MS)
    ) {
      Serial.printf(
        "BLE: %s | Joystick: X=%4d Y=%4d | "
        "Botoes: 0x%04X",
        connected ? "ON " : "OFF",
        report.joystick2X,
        report.joystick2Y,
        report.buttons
      );

      if (report.imu.valid) {
        Serial.printf(
          " | IMU: A=[%6d,%6d,%6d] "
          "G=[%6d,%6d,%6d]\n",
          report.imu.accelX,
          report.imu.accelY,
          report.imu.accelZ,
          report.imu.gyroX,
          report.imu.gyroY,
          report.imu.gyroZ
        );
      } else {
        Serial.println(" | IMU invalido");
      }

      lastDebugTime = currentTime;
    }

    if (Serial.available()) {
      String command =
        Serial.readStringUntil('\n');

      command.trim();

      if (
        command.equalsIgnoreCase("VIB ON") ||
        command.equalsIgnoreCase("VIB:1") ||
        command == "1"
      ) {
        setVibration(true);
        Serial.println("Vibration: ON");
      } else if (
        command.equalsIgnoreCase("VIB OFF") ||
        command.equalsIgnoreCase("VIB:0") ||
        command == "0"
      ) {
        setVibration(false);
        Serial.println("Vibration: OFF");
      } else if (command.startsWith("VIB:")) {
        int requestedIntensity =
          command.substring(4).toInt();

        requestedIntensity =
          constrain(
            requestedIntensity,
            0,
            255
          );

        setVibrationIntensity(
          static_cast<uint8_t>(requestedIntensity)
        );

        Serial.printf(
          "Vibration intensity: %d\n",
          requestedIntensity
        );
      }
    }

    vTaskDelayUntil(
      &lastWakeTime,
      pdMS_TO_TICKS(COMM_PERIOD_MS)
    );
  }
}

bool GamepadController::begin() {
  analogReadResolution(12);
  joystick2.begin();
  buttons.begin();

  if (!imu.begin()) {
    Serial.println("Erro: IMU nao foi inicializada.");
    return false;
  }

  vibrationMotor.begin();
  joystick2.calibrate();

  inputQueue = xQueueCreate(
    INPUT_QUEUE_LENGTH,
    sizeof(InputUpdate));

  reportMailbox = xQueueCreate(
    1,
    sizeof(GamepadReport));

  imuMailbox = xQueueCreate(
    1,
    sizeof(IMUReport));

  if (inputQueue == nullptr || reportMailbox == nullptr || imuMailbox == nullptr) {
    if (inputQueue != nullptr) {
      vQueueDelete(inputQueue);
      inputQueue = nullptr;
    }

    if (reportMailbox != nullptr) {
      vQueueDelete(reportMailbox);
      reportMailbox = nullptr;
    }

    if (imuMailbox != nullptr) {
      vQueueDelete(imuMailbox);
      imuMailbox = nullptr;
    }

    return false;
  }

  if (!createTasks()) {
    deleteTasks();

    if (inputQueue != nullptr) {
      vQueueDelete(inputQueue);
    }

    if (reportMailbox != nullptr) {
      vQueueDelete(reportMailbox);
    }

    if (imuMailbox != nullptr) {
      vQueueDelete(imuMailbox);
    }

    inputQueue = nullptr;
    reportMailbox = nullptr;
    imuMailbox = nullptr;

    return false;
  }

  Serial.println("[DEBUG] Inicializando Bluetooth...");
  bluetoothGamepad.begin();
  bluetoothReady = true;

  return true;
}

bool GamepadController::createTasks() {
  BaseType_t result;
  result = xTaskCreate(
    joystickTaskEntry,
    "JoystickTask",
    INPUT_TASK_STACK_SIZE,
    this,
    2,
    &joystickTaskHandle);

  if (result != pdPASS) {
    return false;
  }

  result = xTaskCreate(
    buttonsTaskEntry,
    "ButtonsTask",
    INPUT_TASK_STACK_SIZE,
    this,
    2,
    &buttonsTaskHandle);

  if (result != pdPASS) {
    return false;
  }

  result = xTaskCreate(
    imuTaskEntry,
    "IMUTask",
    INPUT_TASK_STACK_SIZE,
    this,
    2,
    &imuTaskHandle);

  if (result != pdPASS) {
    return false;
  }

  result = xTaskCreate(
    processInputsTaskEntry,
    "ProcessInputsTask",
    INPUT_TASK_STACK_SIZE,
    this,
    1,
    &processInputsTaskHandle);

  if (result != pdPASS) {
    return false;
  }

  result = xTaskCreate(
    commTaskEntry,
    "CommTask",
    COMM_TASK_STACK_SIZE,
    this,
    1,
    &commTaskHandle);

  if (result != pdPASS) {
    return false;
  }

  return true;
}

void GamepadController::deleteTasks() {
  if (joystickTaskHandle != nullptr) {
    vTaskDelete(joystickTaskHandle);
    joystickTaskHandle = nullptr;
  }

  if (buttonsTaskHandle != nullptr) {
    vTaskDelete(buttonsTaskHandle);
    buttonsTaskHandle = nullptr;
  }

  if (imuTaskHandle != nullptr) {
    vTaskDelete(imuTaskHandle);
    imuTaskHandle = nullptr;
  }

  if (processInputsTaskHandle != nullptr) {
    vTaskDelete(processInputsTaskHandle);
    processInputsTaskHandle = nullptr;
  }

  if (commTaskHandle != nullptr) {
    vTaskDelete(commTaskHandle);
    commTaskHandle = nullptr;
  }
}

void GamepadController::joystickTaskEntry(void *parameter) {
  GamepadController *controller =
    static_cast<GamepadController *>(parameter);

  controller->runJoystickTask();
}

void GamepadController::buttonsTaskEntry(void *parameter) {
  GamepadController *controller =
    static_cast<GamepadController *>(parameter);

  controller->runButtonsTask();
}

void GamepadController::imuTaskEntry(void *parameter) {
  GamepadController *controller =
    static_cast<GamepadController *>(parameter);

  controller->runIMUTask();
}

void GamepadController::commTaskEntry(void *parameter) {
  GamepadController *controller =
    static_cast<GamepadController *>(parameter);

  controller->runCommTask();
}

void GamepadController::processInputsTaskEntry(void *parameter) {
  GamepadController *controller =
    static_cast<GamepadController *>(parameter);

  controller->runProcessInputsTask();
}

void GamepadController::runJoystickTask() {
  TickType_t lastWakeTime = xTaskGetTickCount();

  for (;;) {
    Axis2D reading2 = joystick2.read();

    InputUpdate update2{
      InputSource::JOYSTICK_2,
      reading2.x,
      reading2.y,
      0
    };

    xQueueSend(
      inputQueue,
      &update2,
      0);

    vTaskDelayUntil(
      &lastWakeTime,
      pdMS_TO_TICKS(JOYSTICK_PERIOD_MS));
  }
}

void GamepadController::runButtonsTask() {
  TickType_t lastWakeTime = xTaskGetTickCount();

  for (;;) {
    buttons.update();

    InputUpdate update{
      InputSource::BUTTONS,
      0,
      0,
      buttons.getButtonMask()
    };

    xQueueSend(
      inputQueue,
      &update,
      0);

    vTaskDelayUntil(
      &lastWakeTime,
      pdMS_TO_TICKS(BUTTONS_PERIOD_MS));
  }
}

void GamepadController::runIMUTask() {
  TickType_t lastWakeTime = xTaskGetTickCount();

  for (;;) {
    IMUReport reading{};

    /*
     * Faz a leitura do acelerômetro e do giroscópio.
     * O método imu.read() altera reading.valid para true
     * quando a comunicação com o MPU6050 funciona.
     */
    if (!imu.read(reading)) {
      reading.valid = false;
    }

   /*
     * Mantém somente a leitura mais recente.
     * Caso a mailbox já esteja ocupada, o valor antigo
     * é substituído pelo novo.
     */
    if (imuMailbox != nullptr) {
      xQueueOverwrite(
        imuMailbox,
        &reading);
    }

    vTaskDelayUntil(
      &lastWakeTime,
      pdMS_TO_TICKS(IMU_PERIOD_MS));
  }
}

void GamepadController::runProcessInputsTask() {
  GamepadReport report{};
  InputUpdate update{};

  for (;;) {
    /*
     * A tarefa permanece bloqueada enquanto não chegam dados.
     * Portanto, não desperdiça processamento.
     */
    if (
      xQueueReceive(
        inputQueue,
        &update,
        portMAX_DELAY)
      == pdPASS) {
      switch (update.source) {
        case InputSource::JOYSTICK_2:
          report.joystick2X =
            static_cast<int8_t>(update.value1);

          report.joystick2Y =
            static_cast<int8_t>(update.value2);

          break;

        case InputSource::BUTTONS:
          report.buttons = update.buttons;
          break;
      }

      /*
       * Mantém somente o relatório mais recente.
       */
      xQueueOverwrite(
        reportMailbox,
        &report);
    }
  }
}

bool GamepadController::readLatestReport(
  GamepadReport &report,
  TickType_t waitTicks) const {
  if (reportMailbox == nullptr) {
    return false;
  }

  /*
   * Copia os joysticks e os botões consolidados.
   */
  if (
    xQueuePeek(
      reportMailbox,
      &report,
      waitTicks)
    != pdPASS
  ) {
    return false;
  }

  /*
   * Acrescenta ao relatório a leitura mais recente do IMU.
   */
  if (imuMailbox != nullptr) {
    IMUReport latestIMUReport{};

    if (
      xQueuePeek(
        imuMailbox,
        &latestIMUReport,
        0)
      == pdPASS
    ) {
      report.imu = latestIMUReport;
    }
  }

  return true;
}

void GamepadController::setVibration(bool enabled) {
  vibrationMotor.setEnabled(enabled);
}

void GamepadController::setVibrationIntensity(uint8_t intensity) {
  vibrationMotor.setIntensity(intensity);
}
