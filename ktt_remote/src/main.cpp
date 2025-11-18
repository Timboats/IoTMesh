#include <Arduino.h>
#include "config.h"


// LIBRARIES
#include "painlessMesh.h"      // Mesh WiFi
#include <cppQueue.h>          // Queue
#include <StackArray.h>        // stack (https://github.com/oogre/StackArray)
#include <Adafruit_GFX.h>      // For TFT LCD Screen
#include "Adafruit_ILI9341.h"  // For TFT LCD Screen
#include "ShiftIn.h"
#include <math.h>
#include "ArduinoOTA.h"
#include <WiFi.h>


// TASK HANDLERS
TaskHandle_t uiHandler;

// TASK FUNCTIONS 
void uiTask(void* params);

// FUNCTION DECLARATIONS
void initializeTFT();

// PERIPHERALS
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, SPI_MOSI, SPI_SCK, TFT_RST, SPI_MISO); // Bad practice to have globals but for simplicity in this example we will do it

// Global vars
bool otaEnable = false;
void setup()
{
  Serial.begin(115200);
  pinMode(TFT_CS, OUTPUT);
  pinMode(TFT_DC, OUTPUT);
  pinMode(TFT_RST, OUTPUT);
  // digitalWrite(TFT_CS, HIGH); // Deselect TFT

  pinMode(DPAD_DOWN, INPUT);
  pinMode(DPAD_UP, INPUT);
  pinMode(DPAD_LEFT, INPUT);
  pinMode(DPAD_RIGHT, INPUT);
  pinMode(ENTER_BUTTON, INPUT);
  pinMode(BACK_BUTTON, INPUT); 

  // OTA init
  #ifdef OTA_SSID
    if(!digitalRead(DPAD_UP) && !digitalRead(DPAD_DOWN) && !digitalRead(DPAD_LEFT) && !digitalRead(DPAD_RIGHT))
    {
      otaEnable = true;
      Serial.println("Entering OTA Update mode");
    }

    if(otaEnable)
    {

      WiFi.mode(WIFI_AP);
      WiFi.softAP(OTA_SSID, OTA_PSK);

      IPAddress ip = WiFi.softAPIP();
      Serial.printf("IP addr: ");
      Serial.println(ip);

      ArduinoOTA.setHostname(OTA_host);

      ArduinoOTA
        .onStart([]() {
          String type;
          if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
          else // U_SPIFFS
            type = "filesystem";

          // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
          Serial.println("Start updating " + type);
        })
        .onEnd([]() {
          Serial.println("\nEnd");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
          Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
          Serial.printf("Error[%u]: ", error);
          if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
          else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
          else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
          else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
          else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });

      ArduinoOTA.begin();
      return;
    }

  #endif

  tft.begin();
  tft.setRotation(1);  // Landscape mode
  initializeTFT();
  /*
  #the stuff below can be replaced
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(40, 10);
  tft.println("Tempo Trainer");
  */
}

void loop()
{
  #ifdef OTA_SSID
    while(otaEnable)
    {
      ArduinoOTA.handle();
    }
  #endif

}

void initializeTFT(){
  tft.fillScreen(ILI9341_BLACK);  // Clear entire screen

  // Header
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(10, 10);
  tft.print("Welcome Screen");
  /*
  // Navigation
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_CYAN);
  tft.setCursor(10, 40);
  tft.print("B1 to Welcome <-");
  tft.setCursor(10, 60);
  tft.print("B2 to Broadcasting ->");

  // Optional: add dividing line
  tft.drawLine(0, 80, 240, 80, ILI9341_DARKGREY);
  */
}

