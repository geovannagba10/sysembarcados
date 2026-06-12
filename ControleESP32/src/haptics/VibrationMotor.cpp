#include "VibrationMotor.h"

VibrationMotor::VibrationMotor(uint8_t pin, uint8_t pwmChannel)
  : pin(pin),
    pwmChannel(pwmChannel),
    enabled(false),
    intensity(0) {
}

void VibrationMotor::begin() {
  pinMode(pin, OUTPUT);

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(pin, PWM_FREQUENCY_HZ, PWM_RESOLUTION_BITS);
#else
  ledcSetup(pwmChannel, PWM_FREQUENCY_HZ, PWM_RESOLUTION_BITS);
  ledcAttachPin(pin, pwmChannel);
#endif

  off();
}

void VibrationMotor::setEnabled(bool active) {
  setIntensity(active ? 255 : 0);
}

void VibrationMotor::setIntensity(uint8_t newIntensity) {
  intensity = newIntensity;
  enabled = intensity > 0;
  writeDuty(intensity);
}

void VibrationMotor::on() {
  setEnabled(true);
}

void VibrationMotor::off() {
  setEnabled(false);
}

bool VibrationMotor::isEnabled() const {
  return enabled;
}

uint8_t VibrationMotor::getIntensity() const {
  return intensity;
}

void VibrationMotor::writeDuty(uint8_t duty) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(pin, duty);
#else
  ledcWrite(pwmChannel, duty);
#endif
}
