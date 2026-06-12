#include "IMU.h"
#include <Wire.h>
#include "../config/Pinos.h"

bool IMU::writeRegister(uint8_t reg, uint8_t value) const {
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool IMU::begin() {
  // Initialize Wire with explicit SDA/SCL pins for ESP32
  Wire.begin(Pins::IMU_SDA, Pins::IMU_SCL);

  Wire.beginTransmission(MPU6050_ADDRESS);
  if (Wire.endTransmission() != 0) {
    return false;
  }

  if (!writeRegister(PWR_MGMT_1, 0x00)) {
    return false;
  }

  if (!writeRegister(ACCEL_CONFIG, 0x00)) {
    return false;
  }

  if (!writeRegister(GYRO_CONFIG, 0x00)) {
    return false;
  }

  return true;
}

bool IMU::read(IMUReport &report) const {
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(ACCEL_XOUT_H);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  constexpr uint8_t kBytesToRead = 14;
  Wire.requestFrom(MPU6050_ADDRESS, kBytesToRead);

  if (Wire.available() < kBytesToRead) {
    return false;
  }

  report.accelX = static_cast<int16_t>(Wire.read() << 8 | Wire.read());
  report.accelY = static_cast<int16_t>(Wire.read() << 8 | Wire.read());
  report.accelZ = static_cast<int16_t>(Wire.read() << 8 | Wire.read());

  Wire.read();
  Wire.read();

  report.gyroX = static_cast<int16_t>(Wire.read() << 8 | Wire.read());
  report.gyroY = static_cast<int16_t>(Wire.read() << 8 | Wire.read());
  report.gyroZ = static_cast<int16_t>(Wire.read() << 8 | Wire.read());

  report.valid = true;
  return true;
}
