#include <Arduino.h>
#include "soc/soc.h"
#include "soc/timer_group_struct.h"
#include "soc/interrupts.h"
#include "esp_intr_alloc.h"
#include "esp_sleep.h"
// THIS IS NODE 3944100465

// LIBRARIES
#include "painlessMesh.h" // For mesh wifi

// CONSTANTS
#define   MESH_PREFIX     "IoTHub"     // Mesh WiFI name
#define   MESH_PASSWORD   "IOAIHTHG"   // Mesh password
#define   MESH_PORT       5555         // Mesh port
#define WAKEUP_TIME_SEC 60

// FUNCTION DECLARATIONS(PLATIO MIGRATION ADDITION)
void receivedCallback( uint32_t from, String &msg );
uint32_t getRootId(painlessmesh::protocol::NodeTree nodeTree);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);
void initConnectionTimer();
void IRAM_ATTR connectionTimerISR(void *arg);


// GLOBAL VARIABLES
// PINS
int ledPin = 5;
bool ledState = false;
uint32_t remoteId;
uint32_t initConnectionAlarmVal = 150000;


  // TIMES
  unsigned long timeSinceOn = millis();
  unsigned long timeSinceUpdate = millis();
  unsigned long timeSinceMsgReceived = millis();
  unsigned long timeSinceDisplayUpdate = millis();
  unsigned long lastBlinkTime = millis();

  // WiFi COMMUNICATION MESSAGE
  int lastValue = 0;
  String receivedMsg;

// INITIALIZING OBJECTS
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
void sendMessage() ; // Prototype so PlatformIO doesn't complain
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void setUpMesh() 
{
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  //mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  // mesh.setDebugMsgTypes( CONNECTION | SYNC );

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  // Tells nodes that there is a root and to connect to it
  mesh.setContainsRoot(false);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) 
  {
    Serial.println("WAKEY WAKEY!!!");
    initConnectionAlarmVal = 18750; // bring down to 15 sec interval after first check
  } 
  else 
  {
    Serial.println("Running start up blink");
    long currTime = millis();

    while(millis() - currTime < 6000)
    {
      digitalWrite(ledPin, HIGH);
      delay(200);
      digitalWrite(ledPin, LOW);
      delay(200);
    }
  
    Serial.println("Finished start up test");
  }
  
  setUpMesh();
  initConnectionTimer();
}

// Send message to root
void sendMessage() {
  String msg = "Callback:" + receivedMsg;
  uint32_t rootId = getRootId(mesh.asNodeTree());
  // Serial.printf("sending %s to %u\n", msg, rootId);
  mesh.sendSingle(rootId, msg);
  if(atoi(receivedMsg.c_str()) == -2) {
    taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 1.5)); 
  } else {
    taskSendMessage.setInterval( random( TASK_SECOND * 0.1, TASK_SECOND * 0.2 ));
  }
}

// Recieve message
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  receivedMsg = msg.c_str();
  timeSinceMsgReceived = millis();
  if(atoi(msg.c_str()) != 0) {
    if(atoi(msg.c_str()) == -1) {
      lastValue = 0;  
    } else if(atoi(msg.c_str()) != -2) {
      lastValue = atoi(msg.c_str());
    }
  }
}

uint32_t getRootId(painlessmesh::protocol::NodeTree nodeTree) {
  if (nodeTree.root) return nodeTree.nodeId;
  for (auto&& s : nodeTree.subs) {
    auto id = getRootId(s);
    if (id != 0) return id;
  }
  return 0;
}

void newConnectionCallback(uint32_t nodeId) {
    // disable counter
    //TODO renable conunters on disconnect
    TIMERG0.hw_timer[0].config.tx_alarm_en = 0;
    TIMERG0.hw_timer[0].config.tx_en = 0;
    TIMERG0.hw_timer[0].load.tx_load = 0;
    TIMERG0.hw_timer[0].alarmlo.val = 150000;
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
    remoteId = getRootId(mesh.asNodeTree());
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
  if(mesh.isConnected(remoteId))
  {
    // make sure timer is disabled
  }
  else
  {
    // make sure timer is enabled
    Serial.println("Lost connection to remote");
    TIMERG0.hw_timer[0].config.tx_alarm_en = 1;
    TIMERG0.hw_timer[0].config.tx_en = 1;

  }
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

unsigned long blinkTempo(int tempo, unsigned long lastBlinkTime) {
  unsigned long newBlinkTime = lastBlinkTime;
  //Serial.printf("millis() - tempo/1024 * 1000 > lastBlinkTime: %d - %d > %d\n", millis(), tempo, lastBlinkTime);
  float bpm = (60000.0 / tempo) * 4;
  if (millis() * 1000 - bpm * 1000 > lastBlinkTime * 1000) {
    Serial.printf("ledState %d\n", ledState);
    digitalWrite(ledPin, ledState);
    ledState = !ledState;
    newBlinkTime = millis();
  }

  return newBlinkTime;
}

void initConnectionTimer()
{
  // TODO there may be an issue that comes up when the connection interrupt triggers exactly when theres a new connection and puts it to sleep mode
  // Timer base clk should be 80MHz APB clk
  TIMERG0.hw_timer[0].config.tx_increase = 1;
  TIMERG0.hw_timer[0].config.tx_autoreload = 1; // enables auto reload so when alarm fires timer resets back to zero 
  TIMERG0.hw_timer[0].config.tx_divider = 64000; // prescaler to bring timer clk to 1.25kHZ

  // alarm value should be set to 150,000 to trigger after 120sec
  TIMERG0.hw_timer[0].config.tx_alarm_en = 0; // make sure timer is disabled
  TIMERG0.hw_timer[0].alarmlo.val = initConnectionAlarmVal; // TODO set this to the proper value depending on if it woke up from deep sleep

  TIMERG0.hw_timer[0].config.tx_en = 0; // Enable this on disconnect

  // interrupt setup
  TIMERG0.int_clr_timers.val = 1; // clrs only the timer0 interrupt
  TIMERG0.int_ena_timers.val |= 1; // enables timer0 interrupt

  esp_intr_alloc(ETS_TG0_T0_LEVEL_INTR_SOURCE, ESP_INTR_FLAG_IRAM, connectionTimerISR, NULL, NULL);

  TIMERG0.hw_timer[0].config.tx_alarm_en = 1;
  TIMERG0.hw_timer[0].config.tx_en = 1;
  Serial.println("Setup alarm irq");
}

void IRAM_ATTR connectionTimerISR(void *arg)
{
  TIMERG0.int_clr_timers.val = 1; // clrs only the timer0 interrupt
  Serial.println("Been disconnected for 120 or 15 seconds since last check");

  esp_sleep_enable_timer_wakeup(WAKEUP_TIME_SEC * 1000000);
  Serial.println("Trainer sleep timer set to 10 sec");

  Serial.println("Night night...");

  Serial.flush();

  esp_deep_sleep_start();
}
void loop() 
{
    mesh.update();
    //analogWrite(ledPin, lastValue / 4);
    //Serial.printf("output value: %d\n", lastValue / 4);
    lastBlinkTime = blinkTempo(lastValue, lastBlinkTime);
}