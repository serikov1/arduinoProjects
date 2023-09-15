#include "Arduino.h"
#include <LiquidCrystal_I2C.h>

#define PREVIEW_DELAY 4000
#define speedModesLength 3

LiquidCrystal_I2C lcd(0x27, 16, 2);

void initLcd(){
  lcd.init();
  lcd.backlight();
  lcd.setCursor(2, 0);
  lcd.print("PetPull v1.0");
  lcd.setCursor(3, 1);
  lcd.print("by Serikov");
}

template<typename T>
void setText(uint8_t col, uint8_t str, T &message){
  lcd.setCursor(col, str);
  lcd.print(message);
}

void firstPageSettings(uint16_t temp, uint16_t setTemp, bool heat, String& speedMode, bool motion){
  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.setCursor(5, 0);
  lcd.print(temp);
  if(temp < 100) lcd.print(" ");
  if(heat){
    lcd.write(126);
    lcd.print(setTemp);
  }
  lcd.setCursor(13, 0);
  if(!motion) lcd.print("OFF");
  else lcd.print("ON ");
  lcd.setCursor(0, 1);
  lcd.print("Speed mode:");
  lcd.setCursor(12, 1);
  lcd.print(speedMode);
}

void secondPageSettings(uint16_t setTemp, bool settingsSaved){
    lcd.setCursor(4, 0);
    lcd.print("Set temp");
    lcd.setCursor(5, 1);
    lcd.write(60);
    lcd.print(setTemp);
    lcd.write(62);
    if(!settingsSaved) setText(0, 1, "*");
    else setText(0, 1, " ");
}

void thirdPageSettings(uint16_t speed, String& speedMode, bool settingsSaved){
  lcd.setCursor(1, 0);
  lcd.print("Set speed: ");
  lcd.print(speedMode);
  lcd.setCursor(4, 1);
  lcd.write(60);
  lcd.print(speed);
  lcd.write(62);
  lcd.setCursor(11, 1);
  lcd.write(223);
  lcd.print("/sec");
  if(!settingsSaved) setText(0, 1, "*");
  else setText(0, 1, " ");
}

template<typename T>
T setNextSpeedModeOrSpeed(T speedModes[], T& speedMode){
  for(int i = 0; i < speedModesLength - 1; i++){
    if(speedModes[i] == speedMode) return speedModes[i+1];
  }
  return speedModes[0];
}

template<typename T>
uint8_t getIndex(T speedModes[], T& speedMode){
  for(uint8_t i = 0; i < speedModesLength; i++){
    if(speedModes[i] == speedMode) return i;
  }
}


















