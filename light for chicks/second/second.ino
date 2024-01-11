// Including the ESP8266 WiFi library
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
// #include <GyverNTC.h>
// #include "PIDtuner.h"
#include <EEPROM.h>

//-----------------//

// Replace with your network details
// const char* ssid = "RT-WiFi-2616";
// const char* password = "9h3HK6aS";

const char* ssid = "TP_Link_Saray";
const char* password = "Tx672Ey9iu"; 

// const char* ssid = "realme GT 5G";
// const char* password = "p3sbs4k3";

// Web Server on port 8888
WiFiServer server(8888);

//-----------------//

// variables
String header;


// int8_t settingsTemp = 10;
// int8_t currentTemp = 0;

// uint8_t fanRepetions = 5;
// uint8_t fanDuration = 10;
// bool fanState = false;

const uint16_t LAMP_PWM_TIME = 60; // in minutes
const uint16_t LAMP_PWM_PARTICLE = 14000; // step of counter in mls
uint16_t lampTimeCounter = 256;
uint32_t tmrLampPWM = 0;

bool doorClosed = false;
bool dayLight = true;
// const uint16_t DOOR_MOVE_TIME = 5000; // in mls

uint8_t currHour = 0;
uint8_t currMin = 0;

#define INIT_ADDR 53
#define INIT_KEY 9

// termistor on А0
// resistence 10к
// coeff 3950
// GyverNTC therm(A0, 10000, 3950);
// PIDtuner tuner;
// float filt = 0; // for filter of temp

uint32_t sixtyTimer = 0;

struct Data {
  uint8_t lampOnHour = 5;
  uint8_t lampOnMin = 0;
  uint8_t lampOffHour = 20;
  uint8_t lampOffMin = 0;

  uint8_t doorOnHour = 6;
  uint8_t doorOnMin = 0;
  uint8_t doorOffHour = 21;
  uint8_t doorOffMin = 0;
};

Data data;
//-----------------//

// pins
#define LAMP_PWM_PIN 15

#define DOOR_PLUS_PIN_1 4
#define DOOR_MINUS_PIN_1 5
#define DOOR_PLUS_PIN_2 2
#define DOOR_MINUS_PIN_2 0

#define DAY_HOUR_ON_LIGHT 16
#define DAY_HOUR_OFF_LIGHT 8
#define DAY_MIN_ON_LIGHT 0
#define DAY_MIN_OFF_LIGHT 0


// #define TEMP_PWM_CONTROL_PIN 5
// #define FAN_CONTROL_PIN 6
// #define TEMP_CONTROL_PIN 7

//-----------------//

//--------------------------------------------------------------------------------------
//uint32_t tmr = 0;
//--------------------------------------------------------------------------------------

//-----------------//

const long utcOffsetInSeconds = 10800;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//-----------------//

void setup() {
  
  // tuner.setParameters(NORMAL, settingsTemp, 5, 5000, 0.08, 15000, 500);

  pinMode(DOOR_PLUS_PIN_1, OUTPUT);
  digitalWrite(DOOR_PLUS_PIN_1, 1);
  pinMode(DOOR_MINUS_PIN_1, OUTPUT);
  digitalWrite(DOOR_PLUS_PIN_2, 1);
  pinMode(DOOR_PLUS_PIN_2, OUTPUT);
  digitalWrite(DOOR_MINUS_PIN_1, 1);
  pinMode(DOOR_MINUS_PIN_2, OUTPUT);
  digitalWrite(DOOR_MINUS_PIN_2, 1);

  // pinMode(FAN_CONTROL_PIN, OUTPUT);
  // pinMode(TEMP_CONTROL_PIN, OUTPUT);
  analogWriteFreq(15000);
  analogWrite(LAMP_PWM_PIN, 0);
  timeClient.begin();
  EEPROM.begin(100);

  Serial.begin(9600);
  delay(10);
  
  // Connecting to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  analogWrite(LAMP_PWM_PIN, 256);
  delay(1000);
  analogWrite(LAMP_PWM_PIN, 0);
  delay(1000);
  analogWrite(LAMP_PWM_PIN, 256);
  delay(1000);
  analogWrite(LAMP_PWM_PIN, 0);
  // Starting the web server
  server.begin();
  Serial.println("Web server running. Waiting for the ESP IP...");
  delay(1000);
  
  // Printing the ESP IP address
  Serial.println(WiFi.localIP());

  if(EEPROM.read(INIT_ADDR) != INIT_KEY){
    EEPROM.put(INIT_ADDR, INIT_KEY);
    EEPROM.commit();
    EEPROM.put(0, data);
    EEPROM.commit();
    Serial.println(EEPROM.read(INIT_ADDR));
  }
  EEPROM.get(0, data);

  timeClient.update();
  
  currHour = timeClient.getHours();
  currMin = timeClient.getMinutes();
  delay(1000);
  if(currHour > data.lampOnHour && currHour < data.lampOffHour)
  {
    lampTimeCounter = 256;
     Serial.println(lampTimeCounter);
  }
  if(currHour < data.lampOnHour || currHour > data.lampOffHour) 
  {
    lampTimeCounter = 0;
    Serial.println(lampTimeCounter);
  }


}

void loop() {

  if(millis() - sixtyTimer >= 60000) 
  {
    if( WiFi.status() == WL_CONNECTED) {
      timeClient.update();
      currHour = timeClient.getHours();
      currMin = timeClient.getMinutes();
      sixtyTimer = millis();
      Serial.println("in loop");
    }
    else {
      WiFi.begin(ssid, password);
      delay(5000);
    }
    // Serial.println(timeClient.getHours());
    // Serial.println(lampTimeCounter);
    // Serial.println(data.lampOnHour);
    // Serial.println(data.lampOffHour);
  }


  lampPWMcontrol(currHour, currMin);
  doorControl(currHour, currMin, doorClosed);
  // tempControl();

  // if(millis() - tmr >= 1000)
  // {
  // timeClient.update();
  // Serial.print(daysOfTheWeek[timeClient.getDay()]);
  // Serial.print(", ");
  // Serial.print(timeClient.getHours());
  // Serial.print(":");
  // Serial.print(timeClient.getMinutes());
  // Serial.print(":");
  // Serial.println(timeClient.getSeconds());
  // tmr = millis();
  // }

  // Listenning for new clients
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New client");
    // boolean to locate when the http request ends
    boolean blank_line = true;
    while (client.connected()) {
      if(millis() - sixtyTimer >= 30000) 
      {
        timeClient.update();
        currHour = timeClient.getHours();
        currMin = timeClient.getMinutes();
        sixtyTimer = millis();
      }
      
      lampPWMcontrol(currHour, currMin);
      doorControl(currHour, currMin, doorClosed);

      if (client.available()) {
        char c = client.read();
        header += c;
  
        if (c == '\n' && blank_line) {

          // checking if header is valid
          // dXNlcjpwYXNz = 'user:pass' (user:pass) base64 encode
    
          Serial.print(header);
          
          // Finding the right credential string
          if(header.indexOf("cGFwYTpwYXBh") >= 0) {
            //successful login
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            // turns the GPIOs on and off
            if(header.indexOf("GET / HTTP/1.1") >= 0) {
                Serial.println("Main Web Page");
            }   

            //#################################################### LAMP

            else if(header.indexOf("GET /lampOnHourPlus HTTP/1.1") >= 0){
              data.lampOnHour = plusHour(data.lampOnHour);
              EEPROM.put(0, data);
              EEPROM.commit();
              Serial.println(data.lampOnHour);
            }
            else if(header.indexOf("GET /lampOnHourMinus HTTP/1.1") >= 0){
              data.lampOnHour = minusHour(data.lampOnHour);
              EEPROM.put(0, data);
              EEPROM.commit();
            }
            else if(header.indexOf("GET /lampOnMinPlus HTTP/1.1") >= 0){
              data.lampOnMin = plusMin(data.lampOnMin);
              EEPROM.put(0, data);
              EEPROM.commit();
            }
            else if(header.indexOf("GET /lampOnMinMinus HTTP/1.1") >= 0){
              data.lampOnMin = minusMin(data.lampOnMin);
              EEPROM.put(0, data);
              EEPROM.commit();
            }

            else if(header.indexOf("GET /lampOffHourPlus HTTP/1.1") >= 0){
              data.lampOffHour = plusHour(data.lampOffHour);
              EEPROM.put(0, data);
              EEPROM.commit();
            }
            else if(header.indexOf("GET /lampOffHourMinus HTTP/1.1") >= 0){
              data.lampOffHour = minusHour(data.lampOffHour);
              EEPROM.put(0, data);
              EEPROM.commit();
            }
            else if(header.indexOf("GET /lampOffMinPlus HTTP/1.1") >= 0){
              data.lampOffMin = plusMin(data.lampOffMin);
              EEPROM.put(0, data);
              EEPROM.commit();
            }
            else if(header.indexOf("GET /lampOffMinMinus HTTP/1.1") >= 0){
              data.lampOffMin = minusMin(data.lampOffMin);
              EEPROM.put(0, data);
              EEPROM.commit();
            }

            //##################################################### DOOR

            else if(header.indexOf("GET /doorOnHourPlus HTTP/1.1") >= 0){
              data.doorOnHour = plusHour(data.doorOnHour);
              EEPROM.put(0, data);
              EEPROM.commit();
            }
            else if(header.indexOf("GET /doorOnHourMinus HTTP/1.1") >= 0){
              data.doorOnHour = minusHour(data.doorOnHour);
              EEPROM.put(0, data);
              EEPROM.commit();
            }
            else if(header.indexOf("GET /doorOnMinPlus HTTP/1.1") >= 0){
              data.doorOnMin = plusMin(data.doorOnMin);
              EEPROM.put(0, data);
              EEPROM.commit();
            }
            else if(header.indexOf("GET /doorOnMinMinus HTTP/1.1") >= 0){
              data.doorOnMin = minusMin(data.doorOnMin);
              EEPROM.put(0, data);
              EEPROM.commit();
            }

            else if(header.indexOf("GET /doorOffHourPlus HTTP/1.1") >= 0){
              data.doorOffHour = plusHour(data.doorOffHour);
              EEPROM.put(0, data);
              EEPROM.commit();
            }
            else if(header.indexOf("GET /doorOffHourMinus HTTP/1.1") >= 0){
              data.doorOffHour = minusHour(data.doorOffHour);
              EEPROM.put(0, data);
              EEPROM.commit();
            }
            else if(header.indexOf("GET /doorOffMinPlus HTTP/1.1") >= 0){
              data.doorOffMin = plusMin(data.doorOffMin);
              EEPROM.put(0, data);
              EEPROM.commit();
            }
            else if(header.indexOf("GET /doorOffMinMinus HTTP/1.1") >= 0){
              data.doorOffMin = minusMin(data.doorOffMin);
              EEPROM.put(0, data);
              EEPROM.commit();
            }

            //######################################################### TEMP
            
            // else if(header.indexOf("GET /TempPlus HTTP/1.1") >= 0){
            //   settingsTemp ++;
            // }
            // else if(header.indexOf("GET /TempMinus HTTP/1.1") >= 0){
            //   settingsTemp --;
            // }

            //########################################################## FAN

            // else if(header.indexOf("GET /fanDurPlus HTTP/1.1") >= 0){
            //   fanDuration ++;
            // }
            // else if(header.indexOf("GET /fanDurMinus HTTP/1.1") >= 0){
            //   fanDuration --;
            // }
            // else if(header.indexOf("GET /fanRepPlus HTTP/1.1") >= 0){
            //   fanRepetions ++;
            // }
            // else if(header.indexOf("GET /fanRepMinus HTTP/1.1") >= 0){
            //   fanRepetions --;
            // }

            // your web page
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");

            client.println("<head>");
              client.println("<meta charset=\"UTF-8\"/>");
              client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
              client.println("<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.4/css/bootstrap.min.css\">");                                                                    
            client.println("</head><div class=\"container\">");

            //client.println("<h1>Умный Курятник</h1>");

            //############################################################### LAMP

            client.println("<h1>Лампа</h1>");

            client.println("<h2>Время включения света - " );
            client.println(data.lampOnHour);
            client.println(":");
            client.println(data.lampOnMin);
            client.println("</h2>");

            client.println("<div class=\"row\">");
            client.println("<div type='button' class=\"col-md-2\"><a href=\"/lampOnHourPlus\" class=\"btn btn-block btn-lg btn-success\" role=\"button\">+ час</a></div>");
            client.println("<div class=\"col-md-2\"><a href=\"/lampOnHourMinus\" class=\"btn btn-block btn-lg btn-danger\" role=\"button\">- час</a></div>");

            client.println("<div class=\"col-md-2\"><a href=\"/lampOnMinPlus\" class=\"btn btn-block btn-lg btn-success\" role=\"button\">+ минута</a></div>");
            client.println("<div class=\"col-md-2\"><a href=\"/lampOnMinMinus\" class=\"btn btn-block btn-lg btn-danger\" role=\"button\">- минута</a></div>");                                   
            client.println("</div>");

            client.println("<h2> Время выключения света - ");
            client.println(data.lampOffHour);
            client.println(":");
            client.println(data.lampOffMin);
            client.println("</h2>");


            client.println("<div class=\"row\">");
            client.println("<div class=\"col-md-2\"><a href=\"/lampOffHourPlus\" class=\"btn btn-block btn-lg btn-success\" role=\"button\">+ час</a></div>");
            client.println("<div class=\"col-md-2\"><a href=\"/lampOffHourMinus\" class=\"btn btn-block btn-lg btn-danger\" role=\"button\">- час</a></div>");

            client.println("<div class=\"col-md-2\"><a href=\"/lampOffMinPlus\" class=\"btn btn-block btn-lg btn-success\" role=\"button\">+ минута</a></div>");
            client.println("<div class=\"col-md-2\"><a href=\"/lampOffMinMinus\" class=\"btn btn-block btn-lg btn-danger\" role=\"button\">- минута</a></div>");                                                                  
            client.println("</div>");

            //############################################################### DOOR

            client.println("<h1>Дверь</h1>");

            client.println("<h2>Время открывания двери - " );
            client.println(data.doorOnHour);
            client.println(":");
            client.println(data.doorOnMin);
            client.println("</h2>");

            client.println("<div class=\"row\">");
            client.println("<div class=\"col-md-2\"><a href=\"/doorOnHourPlus\" class=\"btn btn-block btn-lg btn-success\" role=\"button\">+ час</a></div>");
            client.println("<div class=\"col-md-2\"><a href=\"/doorOnHourMinus\" class=\"btn btn-block btn-lg btn-danger\" role=\"button\">- час</a></div>");

            client.println("<div class=\"col-md-2\"><a href=\"/doorOnMinPlus\" class=\"btn btn-block btn-lg btn-success\" role=\"button\">+ минута</a></div>");
            client.println("<div class=\"col-md-2\"><a href=\"/doorOnMinMinus\" class=\"btn btn-block btn-lg btn-danger\" role=\"button\">- минута</a></div>");                                   
            client.println("</div>");

            client.println("<h2> Время закрывания двери - ");
            client.println(data.doorOffHour);
            client.println(":");
            client.println(data.doorOffMin);
            client.println("</h2>");


            client.println("<div class=\"row\">");
            client.println("<div class=\"col-md-2\"><a href=\"/doorOffHourPlus\" class=\"btn btn-block btn-lg btn-success\" role=\"button\">+ час</a></div>");
            client.println("<div class=\"col-md-2\"><a href=\"/doorOffHourMinus\" class=\"btn btn-block btn-lg btn-danger\" role=\"button\">- час</a></div>");

            client.println("<div class=\"col-md-2\"><a href=\"/doorOffMinPlus\" class=\"btn btn-block btn-lg btn-success\" role=\"button\">+ минута</a></div>");
            client.println("<div class=\"col-md-2\"><a href=\"/doorOffMinMinus\" class=\"btn btn-block btn-lg btn-danger\" role=\"button\">- минута</a></div>");                                                                  
            client.println("</div>");

            //####################################################### TEMP

            // client.println("<h1>Температура</h1>");

            // client.println("<h2>Теккущая температура - " );
            // client.println(currentTemp);
            // client.println("°C");
            // client.println("</h2>");

            // client.println("<h2>Требуемая температура - " );
            // client.println(settingsTemp);
            // client.println("°C");
            // client.println("</h2>");

            // client.println("<div class=\"row\">");
            // client.println("<div class=\"col-md-2\"><a href=\"/TempPlus\" class=\"btn btn-block btn-lg btn-success\" role=\"button\">+ °С</a></div>");
            // client.println("<div class=\"col-md-2\"><a href=\"/TempMinus\" class=\"btn btn-block btn-lg btn-danger\" role=\"button\">- °С</a></div>");
            // client.println("</div>");

            //##################################################### FAN

            // client.println("<h1>Вентилятор</h1>");

            // client.println("<h2> Период работы вентилятора - " );
            // client.println(fanDuration);
            // client.println("мин.</h2>");

            // client.println("<div class=\"row\">");
            // client.println("<div class=\"col-md-2\"><a href=\"/fanDurPlus\" class=\"btn btn-block btn-lg btn-success\" role=\"button\">+ мин.</a></div>");
            // client.println("<div class=\"col-md-2\"><a href=\"/fanDurMinus\" class=\"btn btn-block btn-lg btn-danger\" role=\"button\">- мин.</a></div>");
            // client.println("</div>");

            // client.println("<h2> Количество включений в сутки - ");
            // client.println(fanRepetions);
            // client.println("</h2>");

            // client.println("<div class=\"row\">");
            // client.println("<div class=\"col-md-2\"><a href=\"/fanRepPlus\" class=\"btn btn-block btn-lg btn-success\" role=\"button\">+ повторение</a></div>");
            // client.println("<div class=\"col-md-2\"><a href=\"/fanRepMinus\" class=\"btn btn-block btn-lg btn-danger\" role=\"button\">- повторение</a></div>");                                                                  
            // client.println("</div>");

            //##########################################################

            client.println("<div id=\"footer\">");
            client.println("<h1>      </h1>");
            client.println("</div>");

            client.println("</html>");
          } 
       // wrong user or passm, so http request fails...   
        else {            
           client.println("HTTP/1.1 401 Unauthorized");
           client.println("WWW-Authenticate: Basic realm=\"Secure\"");
           client.println("Content-Type: text/html");
           client.println();
           client.println("<html>Authentication failed</html>");
        }   
        header = "";
        break;
        }
        if (c == '\n') {
          // when starts reading a new line
          blank_line = true;
        }
        else if (c != '\r') {
          // when finds a character on the current line
          blank_line = false;
        }
      }
    }  
    // closing the client connection
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
}

uint8_t plusHour(uint8_t hour)
{
  if(hour < 23) hour++;
  else if(hour == 23) hour = 0;
  return hour;
}

uint8_t plusMin(uint8_t min)
{
  if(min < 59) min++;
  else if(min == 59) min = 0;
  return min;
}

uint8_t minusHour(uint8_t hour)
{
  if(hour > 0) hour--;
  else if(hour == 0) hour = 23;
  return hour;
}

uint8_t minusMin(uint8_t min)
{
  if(min > 0) min--;
  else if(min == 0) min = 59;
  return min;
}

uint8_t* plusForTime(uint8_t currHour, uint8_t currMin, uint8_t extraMin)
{
  if(currMin + extraMin <= 59) 
  {
    uint8_t arr[2] = {currHour, currMin + extraMin};
    return arr;
  }
  else 
  {
    uint8_t extraHour = (currMin + extraMin)/60;
    uint8_t remainderMin = currMin + extraMin - 60 * extraHour;
    uint8_t arr[2] = {currHour + extraHour, remainderMin};
    return arr;
  }
}

void lampPWMcontrol(uint32_t currHour, uint32_t currMin)
{
  if((currHour == data.lampOnHour && currMin >= data.lampOnMin && currMin < data.lampOnMin + LAMP_PWM_TIME) || (currHour - data.lampOnHour == 1 && currMin <= data.lampOnMin + LAMP_PWM_TIME - 60))
  {
    if(millis() - tmrLampPWM > LAMP_PWM_PARTICLE)
    {
      analogWrite(LAMP_PWM_PIN, lampTimeCounter);
      if(lampTimeCounter < 256) lampTimeCounter++;
      tmrLampPWM = millis();
    }
  }

  // if(currHour == DAY_HOUR_OFF_LIGHT && currMin == DAY_MIN_OFF_LIGHT)
  // {
  //   analogWrite(LAMP_PWM_PIN, 0);
  // }

  // if(currHour == DAY_HOUR_ON_LIGHT && currMin == DAY_MIN_ON_LIGHT)
  // {
  //   analogWrite(LAMP_PWM_PIN, 256);
  //   lampTimeCounter = 256;
  // }

  if(dayLight && (currHour > data.lampOnHour || (currHour = data.lampOnHour && currMin >= data.lampOnMin + LAMP_PWM_TIME)) && ( currHour < data.lampOffHour || (currHour =  data.lampOffHour && currMin < data.lampOffMin) )) {
    if(lampTimeCounter != 256) lampTimeCounter = 256;
    // Serial.println("in time");
    analogWrite(LAMP_PWM_PIN, lampTimeCounter);
    dayLight = false;
  }

  if((currHour == data.lampOffHour && currMin >= data.lampOffMin && currMin < data.lampOffMin + LAMP_PWM_TIME) || (currHour - data.lampOffHour == 1 && currMin <= data.lampOffMin + LAMP_PWM_TIME - 60))
  {
    if(millis() - tmrLampPWM > LAMP_PWM_PARTICLE)
    {
      analogWrite(LAMP_PWM_PIN, lampTimeCounter);
      if(lampTimeCounter > 0) lampTimeCounter--;
      tmrLampPWM = millis();
    }
  }
}

void doorControl(uint32_t currHour, uint32_t currMin, bool doorClosed)
{
  if(currHour >= data.doorOnHour && currHour < data.doorOnHour + 1 && currMin == data.doorOnMin)
  {
    digitalWrite(DOOR_PLUS_PIN_1, 0);
    digitalWrite(DOOR_MINUS_PIN_1, 0);
  }
  if(currHour >= data.doorOnHour && currHour < data.doorOnHour + 1 && currMin == data.doorOnMin + 1)
  {
    digitalWrite(DOOR_PLUS_PIN_1, 1);
    digitalWrite(DOOR_MINUS_PIN_1, 1);
  }
  if(currHour >= data.doorOffHour && currHour < data.doorOffHour + 1 && currMin == data.doorOffMin)
  {
    digitalWrite(DOOR_PLUS_PIN_2, 0);
    digitalWrite(DOOR_MINUS_PIN_2, 0);
  }
  if(currHour >= data.doorOffHour && currHour < data.doorOffHour + 1 && currMin == data.doorOffMin + 1)
  {
    digitalWrite(DOOR_PLUS_PIN_2, 1);
    digitalWrite(DOOR_MINUS_PIN_2, 1);
  }

}

// void tempControl()
// {
//   filt += (therm.getTemp() - filt) * 0.1;
//   tuner.setInput(filt);
//   tuner.compute();
//   analogWrite(TEMP_CONTROL_PIN, tuner.getOutput());
// }

// void fanControl(bool fanState)
// {
//   digitalWrite(FAN_CONTROL_PIN, !fanState);
//   fanState = !fanState;
// }

