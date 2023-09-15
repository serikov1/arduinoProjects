#include <stdint.h>
#include<Arduino.h>

bool regul(uint16_t temp, uint16_t setTemp, uint16_t hysteresis){
  if(temp < (setTemp - hysteresis)) return false;
  else if(temp > (setTemp + hysteresis)) return true;
}

uint16_t prevInput = 0; 
bool getRelayDt(float dt, uint8_t k, uint16_t input, uint16_t setpoint, uint16_t hysteresis){
  uint16_t signal;
  if(dt != 0 && k != 0){
    signal = input + (input - prevInput) * k / dt;
    prevInput = input;
  } else signal = input;

  return regul(signal, setpoint, hysteresis);
}

uint32_t prevTime = 0;
bool getRelay(uint8_t k, uint16_t input, uint16_t setpoint, uint16_t hysteresis){
  bool out = getRelayDt((millis() - prevTime)/1000.0f, k, input, setpoint, hysteresis);
  prevTime = millis();
  return out;
}

