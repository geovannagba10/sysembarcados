#pragma once

#include <Arduino.h>

class VibrationMotor {
  private:
    static constexpr uint32_t PWM_FREQUENCY_HZ = 5000;
    static constexpr uint8_t PWM_RESOLUTION_BITS = 8;

    uint8_t pin;
    uint8_t pwmChannel;
    bool enabled;
    uint8_t intensity;

    void writeDuty(uint8_t duty);

  public:
    explicit VibrationMotor(uint8_t pin, uint8_t pwmChannel = 0);

    void begin();
    void setEnabled(bool enabled);
    void setIntensity(uint8_t intensity);
    void on();
    void off();
    bool isEnabled() const;
    uint8_t getIntensity() const;
};
