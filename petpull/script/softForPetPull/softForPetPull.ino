#define stepPin 8
#define dirPin 7
#define countStep 200
#define INIT_ADDR 1023
#define INIT_KEY 54
#define ntcPin 0
#define ntcResist 100000
#define extraResist 100000
#define baseTemp 25
#define relayPin 9
#define HYSTER 1

#include "lsd.h"
#include "button.h"
#include <GyverStepper.h>
#include <GyverNTC.h>
#include <EEPROM.h>
#include <GyverRelay.h>
#include "GyverPID.h"

GStepper<STEPPER2WIRE> stepper(countStep, stepPin, dirPin);
GyverNTC therm(ntcPin, ntcResist, 3950, baseTemp, extraResist);
GyverRelay regulator(REVERSE);
GyverPID reg(220, 50, 0);

button btnUp(3);
button btnDown(5);
button btnLeft(6);
button btnRight(4);

struct DataForChange {
  uint16_t speeds[3] = {720, 1080, 1440}; 
  uint16_t setTemp = 100;
};

DataForChange personalSettings;

uint32_t tmrPreview;
uint32_t tmrTemp;

bool preview = true;
bool newPage = false;
bool update = false;
bool changeTemp = false;
bool tempSaved = true;
bool speedSaved = true;
bool firstPage = false;
bool secondPage = false;
bool thirdPage = false;

uint16_t temp = 0;
bool heat = false;

String speedModes[] = {"slow", "mid", "fast"};
String speedMode = speedModes[0];
uint16_t speed = 0;
bool motion = false;

void setup() {
  Serial.begin(9600);
  initLcd();
  stepper.setRunMode(KEEP_SPEED);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  regulator.k = 10;
  regulator.setpoint = personalSettings.setTemp;
  regulator.hysteresis = HYSTER;
  reg.setLimits(0, 20);
  reg.setpoint = personalSettings.setTemp;

  if(EEPROM.read(INIT_ADDR) != INIT_KEY){
    EEPROM.write(INIT_ADDR, INIT_KEY);
    EEPROM.put(0, personalSettings);
  }
  EEPROM.get(0, personalSettings);
  speed =  personalSettings.speeds[0];
}

void loop() {
  
  stepper.tick();

  if(newPage) {
    lcd.clear();
    newPage = false;
    update = true;
  }
  
  //================= PREVIEW ===================

  if(preview && millis() - tmrPreview >= PREVIEW_DELAY){
    preview = false;
    firstPage = true;
    newPage = true;
    update = true;
  }
  
  //================ FIRST PAGE ==================

  if(millis() - tmrTemp >= 2000){
    tmrTemp = millis();
    temp = therm.getTemp();
    changeTemp = true;
  }

  if(changeTemp && firstPage){
    setText(5, 0, temp);
    if(temp < 100) setText(7, 0, " ");
    changeTemp = false;
  }

  if(firstPage && update) {
    firstPageSettings(temp, personalSettings.setTemp, heat, speedMode, motion);
    update = false;
  }
  
  if(firstPage && btnRight.click()){
    firstPage = false;
    secondPage = true;
    newPage = true;
    update = true;
  }

  if(firstPage && btnDown.click() && !motion){
    speedMode = setNextSpeedModeOrSpeed(speedModes, speedMode);
    speed = setNextSpeedModeOrSpeed(personalSettings.speeds, speed);
    newPage = true;
    update = true;
  }

  if(firstPage && btnUp.click()) {
    if(heat) digitalWrite(relayPin, HIGH);
    else digitalWrite(relayPin, LOW);
    heat = !heat;
    newPage = true;
  }

  if(therm.getTemp() >= personalSettings.setTemp - 10 && heat ){
    static uint32_t tmr;
    if (millis() - tmr > 1000) {
      tmr = millis();
      uint16_t temp = therm.getTempAverage();
      regulator.input = temp;
      reg.input = temp;
      regulator.k = reg.getResultTimer();
      digitalWrite(relayPin, !regulator.getResult());
    }
  }

  if(firstPage && btnLeft.click()){
    update = true;
    if(motion){
      stepper.brake();
      motion = false;
      return ;
    }
    if(!motion){
      stepper.setSpeedDeg(speed);
      motion = true;
    }
  }
  
  //================ SECOND PAGE ==================

  if(secondPage && update) {
    secondPageSettings(personalSettings.setTemp, tempSaved);
    update = false;
  }

  if(secondPage && btnRight.click()){
    secondPage = false;
    thirdPage = true;
    newPage = true;
    update = true;
  }
  if(secondPage && btnUp.click()) {
    if(personalSettings.setTemp < 300 && personalSettings.setTemp >= 100) {
      personalSettings.setTemp++;
      tempSaved = false;
      update = true;
    }
  }

  if(secondPage && btnDown.click()) {
    if(personalSettings.setTemp <= 300 && personalSettings.setTemp > 100){
      personalSettings.setTemp--;
      tempSaved = false;
      update = true;
    }
  }

  if(secondPage && btnLeft.click() && !tempSaved){
    EEPROM.put(0, personalSettings);
    tempSaved = true;
    update = true;
  }

  //================ THIRD PAGE ===================

  if(thirdPage && update) {
    thirdPageSettings(speed, speedMode, speedSaved);
    update = false;
  }

  if(thirdPage && btnRight.click()){
    thirdPage = false;
    firstPage = true;
    newPage = true;
    update = true;
  }

  if(thirdPage && btnUp.click()) {
    if(speed < 2000 && speed >= 180 ){
      uint8_t ind = getIndex(personalSettings.speeds, speed);
      speed++;
      personalSettings.speeds[ind] = speed;
      stepper.setSpeedDeg(speed);
      speedSaved = false;
      update = true;
    }
  }

  if(thirdPage && btnDown.click()) {
    if(speed <= 2000 && speed > 180 ){
      uint8_t ind = getIndex(personalSettings.speeds, speed);
      speed--;
      if(speed == 999) newPage = true;
      personalSettings.speeds[ind] = speed;
      stepper.setSpeedDeg(speed);
      speedSaved = false;
      update = true;
    }
  }

  if(thirdPage && btnLeft.click() && !speedSaved){
    EEPROM.put(0, personalSettings);
    speedSaved = true;
    update = true;
  }

}
