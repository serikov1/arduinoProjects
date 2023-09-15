#define THERM_PIN 0
#define RELAY_PIN 9
#define SETPOINT 100.0
#define HYSTER 1
#define ntcPin 0
#define ntcResist 100000
#define extraResist 100000
#define baseTemp 25


#include <GyverNTC.h>
GyverNTC therm(ntcPin, ntcResist, 3950, baseTemp, extraResist);// пин, сопротивление, бета-коэффициент
#include "GyverRelay.h"
GyverRelay regulator(REVERSE);
#include "GyverPID.h"
GyverPID reg(220, 50, 0);

// либо GyverRelay regulator(); без указания направления (будет REVERSE)
void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);     // пин реле
  digitalWrite(RELAY_PIN, LOW);
  regulator.k = 10;              // коэффициент обратной связи (подбирается по факту)
  regulator.setpoint = SETPOINT;  // установка (ставим на SETPOINT градусов)
  regulator.hysteresis = HYSTER;  // ширина гистерезиса
  reg.setLimits(0, 20);
  reg.setpoint = SETPOINT;
}
void loop() {
  regul();
  debug();
}
void regul() {
  static uint32_t tmr;
  if (millis() - tmr > 500) {
    tmr = millis();
    float temp = therm.getTempAverage();
    regulator.input = temp; // сообщаем регулятору текущую температуру
    reg.input = temp;
    regulator.k = reg.getResultTimer();
    digitalWrite(RELAY_PIN, !regulator.getResult());   // отправляем на реле (ОС работает по своему таймеру)
  }
}
void debug() {
  static uint32_t tmr;
  if (millis() - tmr > 30) {
    tmr = millis();
    Serial.print(therm.getTempAverage());    // фактическая
    Serial.print(',');
    Serial.print(SETPOINT);  // гистерезис
    Serial.print(',');
    Serial.print(SETPOINT + HYSTER);  // гистерезис
    Serial.print(',');
    Serial.print(SETPOINT - HYSTER);  // гистерезис
    Serial.print(',');
    Serial.println(regulator.output * 2 + 60); // сост. реле
  }
}
