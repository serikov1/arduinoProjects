#include "Arduino.h"

class Potentiometr() {
public:
  Potentiometr(uint8_t potPin) {
    _pin = potPin;
  }

  uint16_t getValue(uint16_t begin, uint16_t end) {
    return (analogRead(_pin) / 1024) * (end - begin) + begin;
  }

private:
  uint8_t _pin;
};
