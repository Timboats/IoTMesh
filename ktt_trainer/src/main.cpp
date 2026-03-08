#include <Arduino.h>
#include <WifiEspNow.h>
#include <WiFi.h>

// pin defs
#define LED_PIN 4   // Main tempo flash LED

// device config
// 
// Trainer 1 = 1, Trainer 2 = 2
uint8_t MY_DEVICE_ID = 1;

// data structure
typedef struct struct_message {
  uint8_t  messageType;   // 0 = broadcast, 1 = individual, 2 = callback
  int      tempo;         // BPM (-1 = reset, -2 = idle, >0 = BPM)
  uint32_t timestamp;     // millis() when sent
  uint8_t  targetDevice;  // which client (0 = all)
  uint8_t  senderId;      // 0 = root, 1-2 = clients
} struct_message;

struct_message incomingData;
struct_message outgoingData;

// global states
uint8_t rootAddress[6]   = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
bool    rootAddressKnown = false;

int           currentTempo        = 0;
unsigned long lastBlinkTime       = 0;
unsigned long lastMessageReceived = 0;
unsigned long lastHelloTime       = 0;
bool          isConnected         = false;

// ─── FUNCTION DECLARATIONS ─────────────────────────────────────────────────────
void onReceive(const uint8_t mac[6], const uint8_t* buf, size_t count, void* arg);
void sendHello();
void sendCallback();
void blinkLED();
void checkConnection();

// ═══════════════════════════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);

  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║   TEMPO TRAINER - CLIENT DEVICE       ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.print("Device ID: ");
  Serial.println(MY_DEVICE_ID);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());

  if (!WifiEspNow.begin()) {
    Serial.println("ERROR: WifiEspNow.begin() failed");
    while (1) {
      digitalWrite(LED_PIN, HIGH); delay(100);
      digitalWrite(LED_PIN, LOW);  delay(100);
    }
  }
  Serial.println("WifiEspNow initialized");

  WifiEspNow.onReceive(onReceive, nullptr);

  // Add broadcast peer so we can send and receive broadcast messages
  uint8_t broadcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  WifiEspNow.addPeer(broadcastAddr);

  Serial.println("Waiting for tempo from root...\n");

  // Flash 3 times to show device is ready
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH); delay(100);
    digitalWrite(LED_PIN, LOW);  delay(100);
  }

  // Send hello so remote knows we are here
  sendHello();
}

// main loop
void loop() {
  blinkLED();
  checkConnection();

  // Keep sending hello every 3 seconds until remote responds
  if (!isConnected && millis() - lastHelloTime > 3000) {
    sendHello();
  }

  delay(1);
}

// send hello test 
void sendHello() {
  outgoingData.messageType  = 2;
  outgoingData.tempo        = 0;
  outgoingData.timestamp    = millis();
  outgoingData.targetDevice = 0;
  outgoingData.senderId     = MY_DEVICE_ID;

  uint8_t broadcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  WifiEspNow.send(broadcastAddr, (uint8_t*)&outgoingData, sizeof(outgoingData));

  lastHelloTime = millis();
  Serial.println("→ Sent hello to remote");
}

// ═══════════════════════════════════════════════════════════════════════════════
//  RECEIVE CALLBACK
// ═══════════════════════════════════════════════════════════════════════════════
void onReceive(const uint8_t mac[6], const uint8_t* buf, size_t count, void* arg) {
  if (count != sizeof(struct_message)) {
    Serial.println("← Unexpected packet size, ignoring");
    return;
  }

  memcpy(&incomingData, buf, sizeof(incomingData));
  lastMessageReceived = millis();
  isConnected = true;

  if (!rootAddressKnown) {
    memcpy(rootAddress, mac, 6);
    rootAddressKnown = true;
    WifiEspNow.addPeer(rootAddress);
    Serial.printf("Root found: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  }

  if (incomingData.messageType == 1 && incomingData.targetDevice != MY_DEVICE_ID) {
    Serial.printf("Message for Device %d — ignoring\n", incomingData.targetDevice);
    return;
  }

  Serial.printf("← Received | Type: %d | Tempo: %d\n",
                incomingData.messageType, incomingData.tempo);

  if (incomingData.tempo != currentTempo) {
    currentTempo = incomingData.tempo;

    if (currentTempo == -1 || currentTempo == -2) {
      Serial.println("  → Stopped");
      digitalWrite(LED_PIN, LOW);
    } else if (currentTempo > 0) {
      Serial.printf("  → Tempo: %d BPM\n", currentTempo);
    }
  }

  sendCallback();
}

// send callback to root
void sendCallback() {
  if (!rootAddressKnown) return;

  outgoingData.messageType  = 2;
  outgoingData.tempo        = currentTempo;
  outgoingData.timestamp    = millis();
  outgoingData.targetDevice = 0;
  outgoingData.senderId     = MY_DEVICE_ID;

  bool ok = WifiEspNow.send(rootAddress, (uint8_t*)&outgoingData, sizeof(outgoingData));
  Serial.println(ok ? "  → Callback sent" : "  ✗ Callback failed");
}
//blink led
void blinkLED() {
  if (currentTempo <= 0) {
    digitalWrite(LED_PIN, LOW);
    return;
  }

  unsigned long now          = millis();
  unsigned long beatInterval = 60000UL / currentTempo;

  if (now - lastBlinkTime >= beatInterval) {
    digitalWrite(LED_PIN, HIGH);
    lastBlinkTime = now;
  } else if (now - lastBlinkTime >= 50) {
    digitalWrite(LED_PIN, LOW);
  }
}

// connection status
void checkConnection() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck < 100) return;
  lastCheck = millis();

  if (millis() - lastMessageReceived < 5000) {
    if (!isConnected) {
      isConnected = true;
      Serial.println("✓ Connected to root");
    }
  } else {
    if (isConnected) {
      isConnected = false;
      Serial.println("✗ Connection lost — waiting for root...");
    }
  }
}