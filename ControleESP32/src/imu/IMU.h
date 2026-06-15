#pragma once

#include <Arduino.h>
#include "../model/ControllerTypes.h"
#include "../config/Pinos.h"

class IMU {
  private:
    static constexpr uint8_t MPU6050_ADDRESS = 0x68;
    static constexpr uint8_t PWR_MGMT_1 = 0x6B;
    static constexpr uint8_t ACCEL_CONFIG = 0x1C;
    static constexpr uint8_t GYRO_CONFIG = 0x1B;
    static constexpr uint8_t ACCEL_XOUT_H = 0x3B;

    bool writeRegister(uint8_t reg, uint8_t value) const;

  public:
    IMU() = default;

    bool begin();
    bool read(IMUReport &report) const;
};
