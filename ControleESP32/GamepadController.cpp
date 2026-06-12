#include "GamepadController.h"
#include "Pinos.h"

GamepadController::GamepadController()
  : joystick1(
    Pins::JOYSTICK_1_X,
    Pins::JOYSTICK_1_Y),
    joystick2(
      Pins::JOYSTICK_2_X,
      Pins::JOYSTICK_2_Y),
    buttons(
      Pins::BUTTON_1,
      Pins::BUTTON_2,
      Pins::BUTTON_3,
      Pins::BUTTON_4),
    inputQueue(nullptr),
    reportMailbox(nullptr),
    joystickTaskHandle(nullptr),
    buttonsTaskHandle(nullptr),
    processInputsTaskHandle(nullptr) {
}

bool GamepadController::begin() {
  /*
   * Os joysticks são lidos com resolução de 12 bits:
   * valores brutos entre 0 e 4095.
   */
  analogReadResolution(12);

  joystick1.begin();
  joystick2.begin();

  buttons.begin();

  /*
   * Durante a inicialização, os joysticks devem permanecer
   * na posição central.
   */
  joystick1.calibrate();
  joystick2.calibrate();

  /*
   * Fila intermediária:
   * recebe atualizações produzidas pelas tarefas de leitura.
   */
  inputQueue = xQueueCreate(
    INPUT_QUEUE_LENGTH,
    sizeof(InputUpdate));

  /*
   * Mailbox:
   * possui somente uma posição e mantém sempre o relatório
   * mais recente do controle.
   */
  reportMailbox = xQueueCreate(
    1,
    sizeof(GamepadReport));

  if (inputQueue == nullptr || reportMailbox == nullptr) {
    if (inputQueue != nullptr) {
      vQueueDelete(inputQueue);
      inputQueue = nullptr;
    }

    if (reportMailbox != nullptr) {
      vQueueDelete(reportMailbox);
      reportMailbox = nullptr;
    }

    return false;
  }

  /*
   * Disponibiliza um estado inicial zerado.
   */
  GamepadReport initialReport;

  xQueueOverwrite(
    reportMailbox,
    &initialReport);

  if (!createTasks()) {
    deleteTasks();

    vQueueDelete(inputQueue);
    vQueueDelete(reportMailbox);

    inputQueue = nullptr;
    reportMailbox = nullptr;

    return false;
  }

  return true;
}

bool GamepadController::createTasks() {
  BaseType_t result;

  result = xTaskCreate(
    joystickTaskEntry,
    "JoystickTask",
    2048,
    this,
    2,
    &joystickTaskHandle);

  if (result != pdPASS) {
    return false;
  }

  result = xTaskCreate(
    buttonsTaskEntry,
    "ButtonsTask",
    2048,
    this,
    2,
    &buttonsTaskHandle);

  if (result != pdPASS) {
    return false;
  }

  result = xTaskCreate(
    processInputsTaskEntry,
    "ProcessInputsTask",
    3072,
    this,
    3,
    &processInputsTaskHandle);

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

  if (processInputsTaskHandle != nullptr) {
    vTaskDelete(processInputsTaskHandle);
    processInputsTaskHandle = nullptr;
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

void GamepadController::processInputsTaskEntry(void *parameter) {
  GamepadController *controller =
    static_cast<GamepadController *>(parameter);

  controller->runProcessInputsTask();
}

void GamepadController::runJoystickTask() {
  TickType_t lastWakeTime = xTaskGetTickCount();

  for (;;) {
    Axis2D reading1 = joystick1.read();
    Axis2D reading2 = joystick2.read();

    InputUpdate update1{
      InputSource::JOYSTICK_1,
      reading1.x,
      reading1.y,
      0
    };

    InputUpdate update2{
      InputSource::JOYSTICK_2,
      reading2.x,
      reading2.y,
      0
    };

    xQueueSend(
      inputQueue,
      &update1,
      0);

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

void GamepadController::runProcessInputsTask() {
  GamepadReport report;
  InputUpdate update;

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
        case InputSource::JOYSTICK_1:
          report.joystick1X =
            static_cast<int8_t>(update.value1);

          report.joystick1Y =
            static_cast<int8_t>(update.value2);

          break;

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

  return xQueuePeek(
           reportMailbox,
           &report,
           waitTicks)
         == pdPASS;
}