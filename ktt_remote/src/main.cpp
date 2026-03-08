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


// ─── PERIPHERALS ───────────────────────────────────────────────────────────────
TFT_eSPI tft = TFT_eSPI();

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
void onReceive(const uint8_t mac[6], const uint8_t* buf, size_t count, void* arg);
void sendToDevice(int deviceId, int tempo);
void drawSendPage();
void drawNoConnectionPage();
bool pressed(uint8_t pin);
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

  tft.begin();
  tft.setRotation(3);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.print("Root MAC: ");
  Serial.println(WiFi.macAddress());

  if (!WifiEspNow.begin()) {
    Serial.println("ERROR: WifiEspNow.begin() failed");
    while (1);
  }
  Serial.println("WifiEspNow initialized");

  WifiEspNow.onReceive(onReceive, nullptr);

  for (int i = 0; i < TOTAL_TRAINERS; i++) {
    WifiEspNow.addPeer(TRAINER_MACS[i]);
    Serial.printf("Added Trainer %d as peer\n", i + 1);
  }

  uint8_t broadcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  WifiEspNow.addPeer(broadcastAddr);

  // drawNoConnectionPage();
}

// ═══════════════════════════════════════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════════════════════════════════════
void loop() {
  #ifdef OTA_SSID
    while (otaEnable) {
      ArduinoOTA.handle();
    }
  #endif

  if (connectedCount() == 0 && currentPage != NO_CONNECT_PAGE) {
    // drawNoConnectionPage();
  }

  if (currentPage == SEND_PAGE) {
    if (pressed(DPAD_UP) || pressed(DPAD_DOWN)) {
      focusIndex = 1 - focusIndex;
      drawSendPage();

    } else if (pressed(DPAD_LEFT)) {
      if (focusIndex == 0) {
        selectedDevice = (selectedDevice + TOTAL_TRAINERS) % (TOTAL_TRAINERS + 1);
      } else {
        if (selectedValue > 20) selectedValue--;
      }
      drawSendPage();

    } else if (pressed(DPAD_RIGHT)) {
      if (focusIndex == 0) {
        selectedDevice = (selectedDevice + 1) % (TOTAL_TRAINERS + 1);
      } else {
        if (selectedValue < 300) selectedValue++;
      }
      drawSendPage();

    } else if (pressed(ENTER_BUTTON)) {
      sendToDevice(selectedDevice, selectedValue);
      tft.fillScreen(TFT_BLACK);
      tft.setTextSize(2);
      tft.setTextColor(TFT_GREEN);
      tft.setCursor(20, tft.height() / 2 - 10);
      tft.print("Sent!");
      delay(800);
      drawSendPage();

    } else if (pressed(BACK_BUTTON)) {
      focusIndex = 0;
      drawSendPage();
    }
  }
  outgoingData.messageType  = 1;
  outgoingData.tempo        = 0;
  outgoingData.timestamp    = millis();
  outgoingData.targetDevice = 1;
  outgoingData.senderId     = 0;
  WifiEspNow.send(broadcastAddr, (const uint8_t*)&testData, sizeof(testData));

  delay(1000);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  RECEIVE CALLBACK
// ═══════════════════════════════════════════════════════════════════════════════
void onReceive(const uint8_t mac[6], const uint8_t* buf, size_t count, void* arg) {
  tft.println("Receieved data!!!");
  if (count != sizeof(struct_message)) return;

  memcpy(&incomingData, buf, sizeof(incomingData));

  if (incomingData.messageType == 2) {
    uint8_t id = incomingData.senderId;
    if (id >= 1 && id <= TOTAL_TRAINERS) {
      if (!trainerConnected[id - 1]) {
        trainerConnected[id - 1] = true;
        Serial.printf("✓ Trainer %d connected\n", id);
        if (currentPage == NO_CONNECT_PAGE) drawSendPage();
      }
    }
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  SEND TO DEVICE
// ═══════════════════════════════════════════════════════════════════════════════
void sendToDevice(int deviceId, int tempo) {
  outgoingData.tempo     = tempo;
  outgoingData.timestamp = millis();
  outgoingData.senderId  = 0;

  if (deviceId == 0) {
    outgoingData.messageType  = 0;
    outgoingData.targetDevice = 0;
    uint8_t broadcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    WifiEspNow.send(broadcastAddr, (uint8_t*)&outgoingData, sizeof(outgoingData));
    Serial.printf("📡 Broadcast %d BPM to all\n", tempo);
  } else {
    outgoingData.messageType  = 1;
    outgoingData.targetDevice = deviceId;
    WifiEspNow.send(TRAINER_MACS[deviceId - 1], (uint8_t*)&outgoingData, sizeof(outgoingData));
    Serial.printf("📡 Sent %d BPM to Trainer %d\n", tempo, deviceId);
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  DRAW SEND PAGE
// ═══════════════════════════════════════════════════════════════════════════════
void drawSendPage() {
  currentPage = SEND_PAGE;

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);

  int16_t w     = tft.width();
  int16_t y0    = 40;
  int16_t lineH = 40;

  if (focusIndex == 0) {
    tft.fillRect(10, y0 - 18, w - 20, lineH, TFT_DARKGREY);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  } else {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
  }
  tft.setCursor(20, y0);
  tft.print("Device: ");
  if (selectedDevice == 0) {
    tft.print("ALL");
  } else {
    tft.print(selectedDevice);
  }
  tft.printf(" (%d connected)", connectedCount());

  int16_t y1 = y0 + lineH;
  if (focusIndex == 1) {
    tft.fillRect(10, y1 - 18, w - 20, lineH, TFT_DARKGREY);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  } else {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
  }
  tft.setCursor(20, y1);
  tft.print("Tempo:  ");
  tft.print(selectedValue);
  tft.print(" BPM");

  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(10, tft.height() - 20);
  tft.print("<U/D> switch   <L/R> change   [Enter] send");
}

// ═══════════════════════════════════════════════════════════════════════════════
//  DRAW NO CONNECTION PAGE
// ═══════════════════════════════════════════════════════════════════════════════
void drawNoConnectionPage() {
  currentPage = NO_CONNECT_PAGE;

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(40, SCREEN_HEIGHT / 2);
  tft.setTextSize(2);
  tft.setTextColor(TFT_RED);
  tft.print("No Trainer Connected!");
}

// ═══════════════════════════════════════════════════════════════════════════════
//  HELPERS
// ═══════════════════════════════════════════════════════════════════════════════
bool pressed(uint8_t pin) {
  if (digitalRead(pin) == LOW && millis() - lastDebounce > DEBOUNCE_DELAY) {
    lastDebounce = millis();
    return true;
  }
  return false;
}

int connectedCount() {
  int count = 0;
  for (int i = 0; i < TOTAL_TRAINERS; i++) {
    if (trainerConnected[i]) count++;
  }
  return count;
}