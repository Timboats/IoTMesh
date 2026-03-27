#include <Arduino.h>
#include <WifiEspNow.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <ArduinoOTA.h>
#include "config.h"

// ─── TRAINER MAC ADDRESSES ─────────────────────────────────────────────────────
uint8_t TRAINER_MACS[2][6] = {
  {0x9C, 0x13, 0x9E, 0x7A, 0x1D, 0xD4},  // Trainer 1
  {0x9C, 0x13, 0x9E, 0x7A, 0x24, 0xBC}   // Trainer 2
};
#define TOTAL_TRAINERS 2

// ─── DATA STRUCTURE ────────────────────────────────────────────────────────────
typedef struct struct_message {
  uint8_t  messageType;   // 0 = broadcast, 1 = individual, 2 = callback
  int      tempo;
  uint32_t timestamp;
  uint8_t  targetDevice;  // 0 = all, 1-2 = specific trainer
  uint8_t  senderId;      // 0 = root
} struct_message;

struct_message outgoingData;
struct_message incomingData;
struct_message testData;



// ─── GLOBAL STATE ──────────────────────────────────────────────────────────────
bool    otaEnable      = false;
int     selectedDevice = 0;
int     selectedValue  = 120;
uint8_t focusIndex     = 0;
uint8_t currentPage    = NO_CONNECT_PAGE;
bool    trainerConnected[TOTAL_TRAINERS] = {false, false};

unsigned long lastDebounce = 0;
const unsigned long DEBOUNCE_DELAY = 200;

// ─── FUNCTION DECLARATIONS ─────────────────────────────────────────────────────
int  connectedCount();
uint8_t broadcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ═══════════════════════════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(TFT_CS,       OUTPUT);
  pinMode(TFT_DC,       OUTPUT);
  pinMode(TFT_RST,      OUTPUT);
  pinMode(DPAD_UP,      INPUT);
  pinMode(DPAD_DOWN,    INPUT);
  pinMode(DPAD_LEFT,    INPUT);
  pinMode(DPAD_RIGHT,   INPUT);
  pinMode(ENTER_BUTTON, INPUT);
  pinMode(BACK_BUTTON,  INPUT);

  #ifdef OTA_SSID
    if (!digitalRead(DPAD_UP) && !digitalRead(DPAD_DOWN)) {
      otaEnable = true;
      Serial.println("Entering OTA Update mode");
    }

    if (otaEnable) {
      WiFi.mode(WIFI_AP);
      WiFi.softAP(OTA_SSID, OTA_PSK);
      IPAddress ip = WiFi.softAPIP();
      Serial.print("IP addr: ");
      Serial.println(ip);

      ArduinoOTA.setHostname(OTA_host);
      ArduinoOTA
        .onStart([]() {
          String type = ArduinoOTA.getCommand() == U_FLASH ? "sketch" : "filesystem";
          Serial.println("Start updating " + type);
        })
        .onEnd([]() { Serial.println("\nEnd"); })
        .onProgress([](unsigned int progress, unsigned int total) {
          Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
          Serial.printf("Error[%u]: ", error);
          if (error == OTA_AUTH_ERROR)         Serial.println("Auth Failed");
          else if (error == OTA_BEGIN_ERROR)   Serial.println("Begin Failed");
          else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
          else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
          else if (error == OTA_END_ERROR)     Serial.println("End Failed");
        });

      ArduinoOTA.begin();
      return;
    }
  #endif


}

// ═══════════════════════════════════════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════════════════════════════════════
void loop()
{
  #ifdef OTA_SSID
    while (otaEnable) {
      ArduinoOTA.handle();
    }
  #endif

  delay(10);
}






