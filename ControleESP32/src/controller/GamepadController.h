#pragma once

#include <Arduino.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "../joystick/Joystick.h"
#include "../buttons/ButtonManager.h"
#include "../model/ControllerTypes.h"
#include "../imu/IMU.h"
#include "../haptics/VibrationMotor.h"
#include "../bluetooth/BluetoothGamepad.h"

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
    static constexpr uint16_t IMU_PERIOD_MS = 20;
    static constexpr uint16_t COMM_PERIOD_MS = 100;
    static constexpr uint16_t DEBUG_PERIOD_MS = 250;
    static constexpr uint16_t INPUT_TASK_STACK_SIZE = 4096;
    static constexpr uint16_t COMM_TASK_STACK_SIZE = 8192;

    Joystick joystick2;

    ButtonManager buttons;
    IMU imu;
    VibrationMotor vibrationMotor;
    BluetoothGamepad bluetoothGamepad;
    bool bluetoothReady;

    QueueHandle_t inputQueue;
    QueueHandle_t reportMailbox;
    QueueHandle_t imuMailbox;

    TaskHandle_t joystickTaskHandle;
    TaskHandle_t buttonsTaskHandle;
    TaskHandle_t imuTaskHandle;
    TaskHandle_t commTaskHandle;
    TaskHandle_t processInputsTaskHandle;

    bool createTasks();
    void deleteTasks();

    static void joystickTaskEntry(void *parameter);
    static void buttonsTaskEntry(void *parameter);
    static void imuTaskEntry(void *parameter);
    static void commTaskEntry(void *parameter);
    static void processInputsTaskEntry(void *parameter);

    void runJoystickTask();
    void runButtonsTask();
    void runIMUTask();
    void runCommTask();
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
     * Ativa ou desativa o motor de vibração.
     */
    void setVibration(bool enabled);

    /**
     * Ajusta a intensidade do motor de vibração via PWM.
     */
    void setVibrationIntensity(uint8_t intensity);

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
