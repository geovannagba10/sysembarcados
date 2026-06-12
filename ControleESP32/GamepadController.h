#pragma once

#include <Arduino.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "Joystick.h"
#include "ButtonManager.h"
#include "ControllerTypes.h"

/**
 * Coordena os componentes físicos e as tarefas do controle.
 *
 * Responsabilidades:
 * - inicializar os dispositivos;
 * - criar as filas do FreeRTOS;
 * - criar as tarefas periódicas;
 * - consolidar os dados dos joysticks e botões;
 * - disponibilizar o relatório mais recente.
 */
class GamepadController {
  private:
    static constexpr uint8_t INPUT_QUEUE_LENGTH = 8;

    static constexpr uint16_t JOYSTICK_PERIOD_MS = 10;
    static constexpr uint16_t BUTTONS_PERIOD_MS = 10;

    Joystick joystick1;
    Joystick joystick2;

    ButtonManager buttons;

    QueueHandle_t inputQueue;
    QueueHandle_t reportMailbox;

    TaskHandle_t joystickTaskHandle;
    TaskHandle_t buttonsTaskHandle;
    TaskHandle_t processInputsTaskHandle;

    bool createTasks();
    void deleteTasks();

    static void joystickTaskEntry(void *parameter);
    static void buttonsTaskEntry(void *parameter);
    static void processInputsTaskEntry(void *parameter);

    void runJoystickTask();
    void runButtonsTask();
    void runProcessInputsTask();

  public:
    GamepadController();

    /**
     * Inicializa componentes, filas e tarefas.
     *
     * Retorna false se algum recurso não puder ser criado.
     */
    bool begin();

    /**
     * Copia o relatório mais recente para a variável recebida.
     *
     * A tarefa Bluetooth utilizará esse método posteriormente.
     */
    bool readLatestReport(
      GamepadReport &report,
      TickType_t waitTicks = 0
    ) const;
};