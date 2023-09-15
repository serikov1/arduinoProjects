#include <stdint.h>
#include <Arduino.h>

class pidRegulator {
  public:
    pidRegulator(float p, float i, float d, uint16_t dt = 100){
      _p = p;
      _i = i;
      _d = d;
      setDt(dt);
    }

    uint16_t setpoint = 0;
    uint16_t input = 0;
    uint16_t output = 0;
    float _p = 0.0;
    float _i = 0.0;
    float _d = 0.0;


    void setDt(int16_t dt) {
      _dt_s = _dt / 1000.0f;						
      _dt = dt;
    }

    uint16_t getResult() {
      uint16_t error = setpoint - input;			// ошибка регулирования
      uint16_t delta_input = prevInput - input;	// изменение входного сигнала за dt
      prevInput = input;						// запомнили предыдущее

      output = error * _p; 			// пропорциональая составляющая
      output += delta_input * _d / _dt_s; // дифференциальная составляющая
      output += error * _i * _dt_s; 

      return output;
    }

    getResultTimer() {
      if (millis() - pidTimer >= _dt) {
        pidTimer = millis();
        getResult();
      }
      return output;
    }

  private:
    uint16_t _dt = 100;
    float _dt_s = 0.1;
    uint16_t prevInput = 0;
    uint32_t pidTimer = 0;

};
