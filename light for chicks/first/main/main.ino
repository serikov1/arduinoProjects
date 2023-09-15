#include <DS1302.h>
#include <EncButton.h>
#include <EEPROM.h>
#include "menu.h"

#define INIT_ADDR 1023
#define INIT_KEY 102

//-----------------//

// pins
const uint8_t TRANSISTOR_PIN = 15;

//-----------------//

// time
const uint16_t LAMP_PWM_TIME = 10; // in minutes
const uint16_t LAMP_PWM_PARTICLE = 2360; // step of counter in mls
uint8_t lampTimeCounter = 0;
uint32_t tmrLampPWM = 0;

uint32_t tmrRTC = 0;
DS1302 rtc(11, 10, 9);
Time t;

//----------------//

// buttons
EncButton<EB_TICK, 2> buttonCenter;
EncButton<EB_TICK, 3> buttonRight;
EncButton<EB_TICK, 4> buttonLeft;
EncButton<EB_TICK, 5> buttonHourUp;
EncButton<EB_TICK, 6> buttonHourDown;
EncButton<EB_TICK, 7> buttonMinUp;
EncButton<EB_TICK, 8> buttonMinDown;

bool backlight_ = false; 

//----------------//

//pages
bool firstPage_ = true;
bool secondPage_ = false;
bool thirdPage_ = false;
bool fourthPage_ = false;
bool fifthPage_ = false;

bool update = false;

bool lampOffSet_ = false;
bool lampOnSet_ = false;

bool doorDriverOnSet_ = false;
bool doorDriverOffSet_ = false;

bool tempSet_ = false;

bool timeSet_ = false;

bool fanRepetionSet_ = false;
bool fanDurationSet_ = false;

//----------------//

// EEPROM struct
struct DataForChange {
  uint8_t lampOnHour = 5;
  uint8_t lampOnMin = 0;
  uint8_t lampOffHour = 20;
  uint8_t lampOffMin = 0;

  uint8_t doorDriverOnHour = 5;
  uint8_t doorDriverOnMin = 0;
  uint8_t doorDriverOffHour = 20;
  uint8_t doorDriverOffMin = 0;

  uint8_t settingsTemperature = 10;

  uint8_t fanRepetions = 5;
  uint8_t fanDuration = 10;
};
DataForChange personalSettings;

//----------------//

//============= SETUP =================//
void setup() {
 Serial.begin(9600); // Setup Serial connection

 //----------------//

  // time
  rtc.halt(false); // Set the clock to run-mode

  // set current time for RTC
  // rtc.writeProtect(false);
  // rtc.setDOW(MONDAY);
  // rtc.setTime(21, 06, 0);
  // rtc.setDate(20, 3, 2023);
  // rtc.writeProtect(true);

  //----------------//

  // display
  lcd.init();
  lcd.setBacklight(0);

  //----------------//

  // EEPROM settings
  if(EEPROM.read(INIT_ADDR) != INIT_KEY){
    EEPROM.write(INIT_ADDR, INIT_KEY);
    EEPROM.put(0, personalSettings);
  }
  EEPROM.get(0, personalSettings);
}

//============= END SETUP =================//


void loop() 
{
  buttonCenter.tick();
  buttonRight.tick();
  buttonLeft.tick();
  buttonHourUp.tick();
  buttonHourDown.tick();
  buttonMinUp.tick();
  buttonMinDown.tick();

  t = rtc.getTime(); // get data from the DS1302

  //================ Lamp PWM control ==================//

  if(t.hour = personalSettings.lampOnHour && t.min >= personalSettings.lampOnMin && t.min < personalSettings.lampOnMin + LAMP_PWM_TIME || t.hour - personalSettings.lampOnHour == 1 && t.min < personalSettings.lampOnMin + LAMP_PWM_TIME - 60)
  {
    if(millis() - tmrLampPWM > LAMP_PWM_PARTICLE)
    {
      uint16_t kvant = lampTimeCounter * LAMP_PWM_PARTICLE;
      if (kvant == 255) analogWrite(TRANSISTOR_PIN, 255);
      else {
        analogWrite(TRANSISTOR_PIN, kvant);
        lampTimeCounter++;
      }
    }
  }

  //================ Door Driver control ==================//

  if(t.hour >= personalSettings.doorDriverOnHour && t.min >= personalSettings.doorDriverOnHour && t)

  //================ FIRST PAGE (Lamp settings)==================//

  if(buttonCenter.click() && firstPage) 
  {
    lcd.backlight();
    update = true;
  }

  if(firstPage_ && update)
  {
    clearDisplay();
    firstPage(personalSettings.lampOnHour, personalSettings.lampOnMin, personalSettings.lampOffHour, personalSettings.lampOffMin, lampOnSet_, lampOffSet_);
    update = false;
  }
  
  // ------lamp on settings-------//

  if(firstPage_ && !lampOnSet_ && !lampOffSet_ && buttonLeft.hasClicks(2))
  {
    setText(0, 1, "*");
    lampOnSet_ = true;
  }

  if(firstPage_ && lampOnSet_ && buttonHourUp.press() && personalSettings.lampOnHour >= 0 && personalSettings.lampOnHour < 24)
  {
    if(personalSettings.lampOnHour == 23) personalSettings.lampOnHour = 0;
    else personalSettings.lampOnHour++;
    update = true;
  }

  if(firstPage_ && lampOnSet_ && buttonHourDown.press()&& personalSettings.lampOnHour >= 0 && personalSettings.lampOnHour < 24)
  {
    if(personalSettings.lampOnHour == 0) personalSettings.lampOnHour = 23;
    else personalSettings.lampOnHour--;
    update = true;
  }

  if(firstPage_ && lampOnSet_ && buttonMinUp.press() && personalSettings.lampOnMin < 60 && personalSettings.lampOnMin >= 0)
  {
    if(personalSettings.lampOnMin == 59) personalSettings.lampOnMin = 0;
    else personalSettings.lampOnMin++;
    update = true;
  }

  if(firstPage_ && lampOnSet_ && buttonMinDown.press() && personalSettings.lampOnMin < 60 && personalSettings.lampOnMin >= 0)
  {
    if(personalSettings.lampOnMin == 0) personalSettings.lampOnMin = 59;
    else personalSettings.lampOnMin--;
    update = true;
  }

  if(firstPage_ && buttonLeft.hasClicks(2) && lampOnSet_)
  {
    setText(0, 1, " ");
    lampOnSet_ = false;
    // EEPROM.put(0, personalSettings);
  }

  // ------lamp off settings-------//

  if(firstPage_ && !lampOffSet_ && !lampOnSet_ && buttonRight.hasClicks(2))
  {
    lampOffSet_ = true;
    setText(15, 1, "*");
  }
  if(firstPage_ && lampOffSet_ && buttonHourUp.press() && personalSettings.lampOffHour >= 0 && personalSettings.lampOffHour < 24)
  {
    if(personalSettings.lampOffHour == 23) personalSettings.lampOffHour = 0;
    else personalSettings.lampOffHour++;
    update = true;
  }

  if(firstPage_ && lampOffSet_ && buttonHourDown.press()&& personalSettings.lampOffHour >= 0 && personalSettings.lampOffHour < 24)
  {
    if(personalSettings.lampOffHour == 0) personalSettings.lampOffHour = 23;
    else personalSettings.lampOffHour--;
    update = true;
  }

  if(firstPage_ && lampOffSet_ && buttonMinUp.press() && personalSettings.lampOffMin < 60 && personalSettings.lampOffMin >= 0)
  {
    if(personalSettings.lampOffMin == 59) personalSettings.lampOffMin = 0;
    else personalSettings.lampOffMin++;
    update = true;
  }

  if(firstPage_ && lampOffSet_ && buttonMinDown.press() && personalSettings.lampOffMin < 60 && personalSettings.lampOffMin >= 0)
  {
    if(personalSettings.lampOffMin == 0) personalSettings.lampOffMin = 59;
    else personalSettings.lampOffMin--;
    update = true;
  }

  if(firstPage_ && lampOffSet_ && buttonRight.hasClicks(2))
  {
    setText(15, 1, " ");
    lampOffSet_ = false;
    // EEPROM.put(0, personalSettings);
  }
  
  // -----------next page settings-------------//
  if(firstPage_ && buttonRight.hasClicks(1) && !lampOffSet_ && !lampOnSet_)
  {
    secondPage_ = true;
    firstPage_ = false;
    update = true;
  }
  if(firstPage_ && buttonLeft.hasClicks(1)&& !lampOffSet_ && !lampOnSet_)
  {
    fifthPage_ = true;
    firstPage_ = false;
    update = true;
  }

  //================ SECOND PAGE (Door driver)==================//

  if(secondPage_ && update)
  {
    clearDisplay();
    secondPage(personalSettings.doorDriverOnHour, personalSettings.doorDriverOnMin, personalSettings.doorDriverOffHour, personalSettings.doorDriverOffMin, doorDriverOnSet_, doorDriverOffSet_);
    update = false;
  }

  // ------doorDriver on settings-------//

  if(secondPage_ && !doorDriverOnSet_ && !doorDriverOffSet_ && buttonLeft.hasClicks(2))
  {
    doorDriverOnSet_ = true;
    setText(0, 1, "*");
  }

  if(secondPage_ && doorDriverOnSet_ && buttonHourUp.press() && personalSettings.doorDriverOnHour >= 0 && personalSettings.doorDriverOnHour < 24)
  {
    if(personalSettings.doorDriverOnHour == 23) personalSettings.doorDriverOnHour = 0;
    else personalSettings.doorDriverOnHour++;
    update = true;
  }

  if(secondPage_ && doorDriverOnSet_ && buttonHourDown.press()&& personalSettings.doorDriverOnHour >= 0 && personalSettings.doorDriverOnHour < 24)
  {
    if(personalSettings.doorDriverOnHour == 0) personalSettings.doorDriverOnHour = 23;
    else personalSettings.doorDriverOnHour--;
    update = true;
  }

  if(secondPage_ && doorDriverOnSet_ && buttonMinUp.press() && personalSettings.doorDriverOnMin < 60 && personalSettings.doorDriverOnMin >= 0)
  {
    if(personalSettings.doorDriverOnMin == 59) personalSettings.doorDriverOnMin = 0;
    else personalSettings.doorDriverOnMin++;
    update = true;
  }

  if(secondPage_ && doorDriverOnSet_ && buttonMinDown.press() && personalSettings.doorDriverOnMin < 60 && personalSettings.doorDriverOnMin >= 0)
  {
    if(personalSettings.doorDriverOnMin == 0) personalSettings.doorDriverOnMin = 59;
    else personalSettings.doorDriverOnMin--;
    update = true;
  }

  if(secondPage_ && buttonLeft.hasClicks(2) && doorDriverOnSet_)
  {
    setText(0, 1, " ");
    doorDriverOnSet_ = false;
    // EEPROM.put(0, personalSettings);
  }

  // ------door driver off settings-------//

  if(secondPage_ && !doorDriverOffSet_ && !doorDriverOnSet_ && buttonRight.hasClicks(2))
  {
    doorDriverOffSet_ = true;
    setText(15, 1, "*");
  }
  if(secondPage_ && doorDriverOffSet_ && buttonHourUp.press() && personalSettings.doorDriverOffHour >= 0 && personalSettings.doorDriverOffHour < 24)
  {
    if(personalSettings.doorDriverOffHour == 23) personalSettings.doorDriverOffHour = 0;
    else personalSettings.doorDriverOffHour++;
    update = true;
  }

  if(secondPage_ && doorDriverOffSet_ && buttonHourDown.press() && personalSettings.doorDriverOffHour >= 0 && personalSettings.doorDriverOffHour < 24)
  {
    if(personalSettings.doorDriverOffHour == 0) personalSettings.doorDriverOffHour = 23;
    else personalSettings.doorDriverOffHour--;
    update = true;
  }

  if(secondPage_ && doorDriverOffSet_ && buttonMinUp.press() && personalSettings.doorDriverOffMin < 60 && personalSettings.doorDriverOffMin >= 0)
  {
    if(personalSettings.doorDriverOffMin == 59) personalSettings.doorDriverOffMin = 0;
    else personalSettings.doorDriverOffMin++;
    update = true;
  }

  if(secondPage_ && doorDriverOffSet_ && buttonMinDown.press() && personalSettings.doorDriverOffMin < 60 && personalSettings.doorDriverOffMin >= 0)
  {
    if(personalSettings.doorDriverOffMin == 0) personalSettings.doorDriverOffMin = 59;
    else personalSettings.doorDriverOffMin--;
    update = true;
  }

  if(secondPage_ && doorDriverOffSet_ && buttonRight.hasClicks(2))
  {
    setText(15, 1, " ");
    doorDriverOffSet_ = false;
    // EEPROM.put(0, personalSettings);
  }

   // -----------next page settings-------------//
  if(secondPage_ && buttonRight.hasClicks(1) && !doorDriverOffSet_ && !doorDriverOnSet_)
  {
    secondPage_ = false;
    thirdPage_ = true;
    update = true;
  }
  if(secondPage_ && buttonLeft.hasClicks(1) && !doorDriverOffSet_ && !doorDriverOnSet_)
  {
    firstPage_ = true;
    secondPage_ = false;
    update = true;
  }

  //================ THIRD PAGE (Temperature settings)==================//

  if(thirdPage_ && update)
  {
    clearDisplay();
    update = false;
    thirdPage(15, personalSettings.settingsTemperature, tempSet_);
  }

  if(thirdPage_ && !tempSet_ &&buttonRight.hasClicks(2))
  {
    setText(15, 1, "*");
    tempSet_ = true;
  }

  if(thirdPage_ && tempSet_ && buttonHourUp.press() && personalSettings.settingsTemperature < 30 && personalSettings.settingsTemperature > 0)
  {
    personalSettings.settingsTemperature++;
    update = true;
  }

  if(thirdPage_ && tempSet_ && buttonHourDown.press() && personalSettings.settingsTemperature > 1 && personalSettings.settingsTemperature <= 30)
  {
    personalSettings.settingsTemperature--;
    update = true;
  }

  if(thirdPage_ && tempSet_ &&buttonRight.hasClicks(2))
  {
    setText(15, 1, " ");
    tempSet_ = false;
    // EEPROM.put(0, personalSettings);
  }

   // -----------next page settings-------------//
  if(thirdPage_ && buttonRight.hasClicks(1) && !tempSet_)
  {
    thirdPage_ = false;
    fourthPage_ = true;
    update = true;
  }

  if(thirdPage_ && buttonLeft.hasClicks(1) && !tempSet_)
  {
    secondPage_ = true;
    thirdPage_ = false;
    update = true;
  }

  //================ FOURTH PAGE (Fan settings)==================//

  if(fourthPage_ && update)
  {
    clearDisplay();
    fourthPage(personalSettings.fanRepetions, personalSettings.fanDuration, fanRepetionSet_, fanDurationSet_);
    update = false;
  }

  // ---------fan duration settings-------------//
  if(fourthPage_ && !fanDurationSet_ && !fanRepetionSet_ && buttonRight.hasClicks(2))
  {
    setText(15, 0, "*");
    fanDurationSet_ = true;
  }

  if(fourthPage_ && fanDurationSet_ && buttonMinUp.press() && personalSettings.fanDuration < 50 && personalSettings.fanDuration >= 0)
  {
    personalSettings.fanDuration++;
    update = true;
  }

  if(fourthPage_ && fanDurationSet_ && buttonMinDown.press() && personalSettings.fanDuration > 0 && personalSettings.fanDuration <= 50)
  {
    personalSettings.fanDuration--;
    update = true;
  }

  if(fourthPage_ && fanDurationSet_ && buttonRight.hasClicks(2))
  {
    setText(15, 0, " ");
    fanDurationSet_ = false;
    // EEPROM.put(0, personalSettings);
  }

  // -------------------fan repetions settings----------------//
  if(fourthPage_ && !fanRepetionSet_ && !fanDurationSet_ && buttonLeft.hasClicks(2))
  {
    setText(0, 0, "*");
    fanRepetionSet_ = true;
  }

  if(fourthPage_ && fanRepetionSet_ && buttonMinUp.press() && personalSettings.fanRepetions < 1440/personalSettings.fanDuration && personalSettings.fanRepetions >= 0)
  {
    personalSettings.fanRepetions++;
    update = true;
  }

  if(fourthPage_ && fanRepetionSet_ && buttonMinDown.press() && personalSettings.fanRepetions > 0 && personalSettings.fanRepetions <= 1440/personalSettings.fanDuration )
  {
    personalSettings.fanRepetions--;
    update = true;
  }

  if(fourthPage_ && fanRepetionSet_ && buttonLeft.hasClicks(2))
  {
    setText(0, 0, " ");
    fanRepetionSet_ = false;
    // EEPROM.put(0, personalSettings);
  }

  // -----------next page settings-------------//
  if(fourthPage_ && buttonRight.hasClicks(1) && !fanDurationSet_ && !fanRepetionSet_)
  {
    fourthPage_ = false;
    fifthPage_ = true;
    update = true;
  }

  if(fourthPage_ && buttonLeft.hasClicks(1) && !fanDurationSet_ && !fanRepetionSet_)
  {
    thirdPage_ = true;
    fourthPage_ = false;
    update = true;
  }

  //================ FIFTH PAGE (Current time settings)==================//

  // if(fifthPage_ && update)
  // {
  //   clearDisplay();
  //   fifthPage(t.hour, t.min, timeSet_);
  //   update = false;
  // }

  // if(fifthPage_ && !timeSet_ && buttonRight.hasClicks(2))
  // {
  //   setText(15, 1, "*");
  //   timeSet_ = true;
  // }

  // if(fifthPage_ && timeSet_ && buttonHourUp.press())
  // {
  //   if(t.hour < 23) t.hour++;
  //   else if(t.hour == 23) {
  //     t.hour = 0;
  //   }
  //   rtc.writeProtect(false);
  //   rtc.setTime(t.hour, t.min, t.sec);
  //   rtc.writeProtect(true);
  //   update = true;
  // }

  // if(fifthPage_ && timeSet_ && buttonHourDown.press())
  // {
  //   t.hour--;
  //   rtc.writeProtect(false);
  //   rtc.setTime(t.hour, t.min, t.sec);
  //   rtc.writeProtect(true);
  //   update = true;
  // }

  // if(fifthPage_ && timeSet_ && buttonMinUp.press())
  // {
  //   t.min++;
  //   rtc.writeProtect(false);
  //   rtc.setTime(t.hour, t.min, t.sec);
  //   rtc.writeProtect(true);
  //   update = true;
  // }

  // if(fifthPage_ && timeSet_ && buttonMinDown.press())
  // {
  //   t.min--;
  //   rtc.writeProtect(false);
  //   rtc.setTime(t.hour, t.min, t.sec);
  //   rtc.writeProtect(true);
  //   update = true;
  // }

  // if(fifthPage_ && timeSet_ && buttonRight.hasClicks(2))
  // {
  //   setText(15, 1, " ");
  //   timeSet_ = false;
  // }

  // // -----------next page settings-------------//
  // if(fifthPage_ && buttonRight.hasClicks(1))
  // {
  //   fifthPage_ = false;
  //   firstPage_ = true;
  //   update = true;
  // }

  // if(fifthPage_ && buttonLeft.hasClicks(1))
  // {
  //   fourthPage_ = true;
  //   fifthPage_ = false;
  //   update = true;
  // }

}
