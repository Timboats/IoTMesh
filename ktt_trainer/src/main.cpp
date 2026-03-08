#include <Arduino.h>
#include <WifiEspNow.h>
#include <WiFi.h>

// Data Structure
typedef struct struct_message {
  uint8_t  messageType;
  int      tempo;
  uint32_t timestamp;
  uint8_t  targetDevice;
  uint8_t  senderId;
} struct_message;

struct_message outgoingData;

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());

  if (!WifiEspNow.begin()) {
    Serial.println("ERROR: WifiEspNow.begin() failed");
    while (1);
  }
  Serial.println("WifiEspNow initialized");

  // Add broadcast peer
  uint8_t broadcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  WifiEspNow.addPeer(broadcastAddr);

  Serial.println("Setup done, sending every 2 seconds...");
}

void loop() {
  outgoingData.messageType  = 2;
  outgoingData.tempo        = 0;
  outgoingData.timestamp    = millis();
  outgoingData.targetDevice = 0;
  outgoingData.senderId     = 1;

  uint8_t broadcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  bool ok = WifiEspNow.send(broadcastAddr, (uint8_t*)&outgoingData, sizeof(outgoingData));

  Serial.println(ok ? "→ Sent!" : "✗ Send failed");

  delay(2000);
}

