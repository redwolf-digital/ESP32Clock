/* 
Author : REDWOLF DiGiTAL (C. Woraken)
*/
#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include <TM1637Display.h>

TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;

const char* ssid       = "WIFI_SSID";
const char* password   = "WIFI_PASSs";

#define CLK 18
#define DIO 5

TM1637Display display(CLK, DIO);

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 0;

unsigned int Second, Minute, Hour;
unsigned long TIMEOUT, ClockTick;
bool DOTSHOW = 0;

const uint8_t CONN[] = {
	SEG_G | SEG_E | SEG_D,           // c
	SEG_G | SEG_E | SEG_D | SEG_C,   // o
	SEG_E | SEG_G | SEG_C,           // n
	SEG_E | SEG_G | SEG_C            // n
};

const uint8_t BOOT[] = {
	SEG_F | SEG_E | SEG_G | SEG_D| SEG_C,   // b
	SEG_G | SEG_E | SEG_D | SEG_C,          // o
	SEG_G | SEG_E | SEG_D | SEG_C,          // o
	SEG_F | SEG_E | SEG_G                   // t
};


void GET_NTP_TIME(void *pvParameters) {
  for(;;){
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      //return;
    }
    Second = timeinfo.tm_sec;
    Minute = timeinfo.tm_min;
    Hour = timeinfo.tm_hour;
    delay(1000);
  } 
}


void SegmentDriver(void *pvParameters) {
  for(;;){
    //Show Display
    if((millis()-ClockTick) >= 1000) {
      //flip dot 
      DOTSHOW = !DOTSHOW;
      //Active Display
      Serial.printf("TIME : %d:%d:%d\n", Hour, Minute, Second);

      
      if(DOTSHOW == 0) {
        display.showNumberDec(Hour, true, 2, 0);
      }else {
        display.showNumberDecEx(Hour, 0x40, true, 2, 0);
      }
      display.showNumberDec(Minute, true, 2, 2);
      
      ClockTick = millis();
    }
  }
}


void setup() {
  Serial.begin(115200); 
  display.setBrightness(0x01);
  


  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    for(int k=0; k <= 4; k++) {
      display.showNumberDecEx(0, (0x80 >> k), true);
      delay(500);
	  }

    if((millis() - TIMEOUT) >= 60000) {
      Serial.printf("\nREBOOT\n");
      display.setSegments(BOOT);
      ESP.restart();
    }
  }
  
  Serial.println(" CONNECTED");

  //pins setup
  pinMode(15, INPUT_PULLUP);  // Auto mode
  pinMode(2, INPUT_PULLUP);   // Display Now

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  //pass
  display.setSegments(CONN);
  delay(1000);

  xTaskCreatePinnedToCore(GET_NTP_TIME, "Task1", 10000, NULL, 2, &Task1, 0);
  xTaskCreatePinnedToCore(SegmentDriver, "Task2", 10000, NULL, 1, &Task2, 1);
}

void loop() {
 
}