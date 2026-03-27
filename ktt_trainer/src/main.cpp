#include <Arduino.h>
#include <WifiEspNow.h>
#include <WiFi.h>

// data structure
typedef struct struct_message {
  uint8_t  messageType; // type of message    
  int      tempo; // what tempo recieved from remote?
  uint32_t timestamp;  // time stamp of when tempo was sent in millis
  uint8_t  targetDevice; // target device sent from rmote 0 = everyone
  uint8_t  senderId; // sender id of trainer 
} struct_message;

struct_message outgoingData;

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // disconnect wifi to ensure no connection from external network
  Serial.print("MAC Address: "); //print mac to verify correct tempo trainer device
  Serial.println(WiFi.macAddress());

  // Condition to print if esp failed
  if (!WifiEspNow.begin()) {
    Serial.println("ERROR: WifiEspNow.begin() failed");
    while (1);
  }
  Serial.println("WifiEspNow initialized");

  // Add broadcast peer
  uint8_t broadcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  WifiEspNow.addPeer(broadcastAddr);

  Serial.println("Setup done, will send every 2 seconds...");
}

void loop() {
  outgoingData.messageType  = 2;
  outgoingData.tempo        = 0;
  outgoingData.timestamp    = millis();
  outgoingData.targetDevice = 0;
  outgoingData.senderId     = 1;

  uint8_t broadcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  bool ok = WifiEspNow.send(broadcastAddr, (uint8_t*)&outgoingData, sizeof(outgoingData));

  Serial.println(ok ? " Sent!" : "Send failed");

  delay(2000);
}

