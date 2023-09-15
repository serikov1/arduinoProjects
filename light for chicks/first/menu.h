#include "WString.h"
#include <stdint.h>
#define _LCD_TYPE 1
#include <LCD_1602_RUS_ALL.h>

LCD_1602_RUS lcd(0x27, 16, 2);

void setText(uint16_t column, uint16_t row, String message)
{
  lcd.setCursor(column, row);
  lcd.print(message);
}
void clearDisplay()
{
  lcd.clear();
}

void timeSettings(uint8_t hourOn, uint8_t minOn, uint8_t hourOff, uint8_t minOff, bool lampOnSet_, bool lampOffSet_)
{
  if(hourOn <= 9){
    lcd.setCursor(1, 1);
    lcd.print(" ");
    lcd.setCursor(2, 1);
    lcd.print(hourOn, DEC);
  }
  else{
    lcd.setCursor(1, 1);
    lcd.print(hourOn, DEC);
  }
  lcd.print(":");
  if(minOn <= 9) {
    lcd.print("0");
    lcd.print(minOn, DEC);
  }
  else lcd.print(minOn, DEC);

  if(hourOff <= 9)
  {
    lcd.setCursor(9, 1);
    lcd.print(" ");
    lcd.setCursor(10, 1);
    lcd.print(hourOff, DEC);
  }
  else 
  {
    lcd.setCursor(9, 1);
    lcd.print(hourOff, DEC);
  }
  
  lcd.print(":");
  if(minOff <= 9) {
    lcd.print("0");
    lcd.print(minOff, DEC);
  }
  else lcd.print(minOff, DEC);

  if(lampOnSet_) setText(0, 1, "*");
  else setText(0, 1, " ");
  if(lampOffSet_) setText(15, 1, "*");
  else setText(15, 1, " ");
}

// first page settings for lamp
void firstPage(uint8_t hourOn, uint8_t minOn, uint8_t hourOff, uint8_t minOff, bool lampOnSet_, bool lampOffSet_)
{
  lcd.setCursor(5, 0);
  lcd.print("Лампа");
  timeSettings(hourOn, minOn, hourOff, minOff, lampOnSet_, lampOffSet_);
}

// second page settings for door driver
void secondPage(uint8_t hourOn, uint8_t minOn, uint8_t hourOff, uint8_t minOff, bool doorDriverOnSet_, bool doorDriverOffSet_)
{
  lcd.setCursor(5, 0);
  lcd.print("Дверца");
  timeSettings(hourOn, minOn, hourOff, minOff, doorDriverOnSet_, doorDriverOffSet_);
}

// third page settings for temperature
void thirdPage(uint8_t currentTemperature, uint8_t settingsTemperature, bool tempSet_)
{
  lcd.setCursor(4, 0);
  lcd.print("Обогрев");

  lcd.setCursor(0, 1);
  lcd.print("Тек:");
  if(currentTemperature <= 9) lcd.print(" ");
  lcd.print(currentTemperature, DEC);
  lcd.print("°");

  lcd.setCursor(8, 1);
  lcd.print("Уст:");
  if(settingsTemperature <= 9) lcd.print(" ");
  lcd.print(settingsTemperature, DEC);
  lcd.print("°");

  if(tempSet_)
  {
    lcd.setCursor(15, 1);
    lcd.print("*");
  }
  else
  {
    lcd.setCursor(15, 1);
    lcd.print(" ");
  }
}

// fourth page settings for fan settings
fourthPage(uint8_t numOfRepetitons, uint8_t duration, bool fanRepetionSet_, bool fanDurationSet_)
{
  lcd.setCursor(4, 0);
  lcd.print("Bытяжka");

  lcd.setCursor(0, 1);
  lcd.print("Повт:");
  if(numOfRepetitons <= 9)lcd.print(" ");
  lcd.print(numOfRepetitons, DEC);

  lcd.setCursor(8, 1);
  lcd.print("Прод:");
   if(duration <= 9)lcd.print(" ");
  lcd.print(duration, DEC);
  lcd.print("м");

  lcd.setCursor(0, 0);
  if(fanRepetionSet_) lcd.print("*");
  else lcd.print(" ");

  lcd.setCursor(15, 0);
  if(fanDurationSet_) lcd.print("*");
  else lcd.print(" ");
}

// fifth page settings for current time
fifthPage(uint16_t hour, uint16_t min, bool setTime_)
{
  lcd.setCursor(3, 0);
  lcd.print("Тек. Время");

  lcd.setCursor(5, 1);
  if(hour <= 9) lcd.print(" ");
  lcd.print(hour, DEC);
  lcd.print(":");
  if(min <= 9) lcd.print("0");
  lcd.print(min, DEC);

  lcd.setCursor(15, 1);
  if(setTime_) lcd.print("*");
  else lcd.print(" ");
}

