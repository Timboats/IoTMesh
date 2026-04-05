#include <Arduino.h>
#include "config.h"


// LIBRARIES
#include "painlessMesh.h"      // Mesh WiFi
#include <cppQueue.h>          // Queue
#include <StackArray.h>        // stack (https://github.com/oogre/StackArray)
#include "ShiftIn.h"
#include <math.h>
#include "ArduinoOTA.h"
#include <WiFi.h>
#include <TFT_eSPI.h>


// TASK HANDLERS
TaskHandle_t uiHandler;

// MESH CONFIG
Scheduler userScheduler;
painlessMesh mesh;

// TASK FUNCTIONS 
void uiTask(void* params);

// FUNCTION DECLARATIONS
bool pressed(uint8_t pin);
void drawSendPage();
void drawNoConnectionPage();

void sendToDevice(int deviceId, int value);
void receivedCallback(uint32_t from, String &msg);
void initMesh();

// PERIPHERALS
TFT_eSPI tft = TFT_eSPI();

// Global vars
bool otaEnable = false;
int selectedDevice = 1;
int totalDevices = 0; // Mesh should still be stable with 10 devices connected
int selectedValue = 150;
uint8_t focusIndex = 0;
uint8_t currentPage = NO_CONNECT_PAGE;

unsigned long lastDebounce = 0; // for input buttons
const unsigned long DEBOUNCE_DELAY = 200;




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
    if(!digitalRead(DPAD_UP) && !digitalRead(DPAD_DOWN)) // TODO require left and right keys also later
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
  tft.setRotation(3);  // Landscape mode

  initMesh();
  drawNoConnectionPage();
  // initializeTFT();
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

  mesh.update();

  // —— SWITCH FOCUS ——
  if(currentPage == SEND_PAGE)
  {
    if(pressed(DPAD_UP) || pressed(DPAD_DOWN)) 
    {
      focusIndex = 1 - focusIndex;
      drawSendPage();
    }
    else if(pressed(DPAD_LEFT)) 
    {
      if(focusIndex == 0) 
      {
        selectedDevice = (selectedDevice + totalDevices - 2) % totalDevices + 1;
        drawSendPage();
      } 
      else 
      {
        if (selectedValue > 0) selectedValue--;
        drawSendPage();
      }
      //drawSendPage();
    }
    else if(pressed(DPAD_RIGHT)) 
    {
      if(focusIndex == 0) 
      {
        selectedDevice = selectedDevice % totalDevices + 1;
        drawSendPage();
      } 
      else 
      {
        selectedValue++;
        drawSendPage();
      }
      //drawSendPage();
    }
    else if(pressed(ENTER_BUTTON)) 
    {
      sendToDevice(selectedDevice, selectedValue);
      tft.fillScreen(ILI9341_BLACK);
      tft.setTextSize(2);
      tft.setTextColor(ILI9341_GREEN);
      tft.setCursor(20, tft.height()/2 - 10);
      tft.print("Sent!");
      delay(800);
      drawSendPage();
    }
    else if(pressed(BACK_BUTTON)) 
    {
    focusIndex = 0;
    drawSendPage();
    }
  }
  delay(10);
}

bool pressed(uint8_t pin) 
{
  if (digitalRead(pin) == LOW && millis() - lastDebounce > DEBOUNCE_DELAY) {
    lastDebounce = millis();
    return true;
  }
  return false;
}

void drawSendPage() 
{
  currentPage = SEND_PAGE; // sets current page

  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);

  int16_t w     = tft.width();
  int16_t y0    = 40;
  int16_t lineH = 40;

  // Device line
  if (focusIndex == 0) {
    tft.fillRect(10, y0-18, w-20, lineH, ILI9341_DARKGREY);
    tft.setTextColor(ILI9341_WHITE, ILI9341_DARKGREY);
  } else {
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  }
  tft.setCursor(20, y0);
  tft.print("Device: ");
  tft.print(selectedDevice);
  tft.printf("(%d)", totalDevices);

  // Value line
  int16_t y1 = y0 + lineH;
  if (focusIndex == 1) {
    tft.fillRect(10, y1-18, w-20, lineH, ILI9341_DARKGREY);
    tft.setTextColor(ILI9341_WHITE, ILI9341_DARKGREY);
  } else {
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  }
  tft.setCursor(20, y1);
  tft.print("Value:  ");
  tft.print(selectedValue);

  // Footer legend
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(10, tft.height() - 20);
  tft.print("<U/D> switch   <L/R> change   [Select] send");
}

void sendToDevice(int deviceId, int value) 
{
  SimpleList<uint32_t> nodes = mesh.getNodeList();
  int numNodes = nodes.size();
  totalDevices = numNodes;
  
  if (deviceId >= 1 && deviceId <= numNodes) {
    auto it = nodes.begin();
    for (int i = 0; i < deviceId - 1; i++) ++it;
    uint32_t nodeId = *it;

    String msg = String(value);
    mesh.sendSingle(nodeId, msg);
    Serial.printf("📡 Sent %s → Node %u\n", msg.c_str(), nodeId);
  } else {
    Serial.println("⚠️  No such node to send to");
  }
}

void receivedCallback(uint32_t from, String &msg) 
{
  Serial.printf("⬅️  Got `%s` from Node %u\n", msg.c_str(), from);
}

void initMesh() 
{
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection([](uint32_t nodeId){
    Serial.printf("🔗 New connection to Node %u\n", nodeId);

  });
  mesh.onChangedConnections([](){
    Serial.println("🔄 Connection list changed");
    totalDevices = mesh.getNodeList().size();
    if(!totalDevices)
    {
      drawNoConnectionPage();
    }
    else
    {
      drawSendPage();
    }
    
  });
  mesh.onNodeTimeAdjusted([](int32_t offset){
    Serial.printf("⏱ Time adjusted by %d ms\n", offset);
  });

  mesh.onDroppedConnection([](uint32_t nodeId){
    Serial.printf("Lost connection to Node %u\n", nodeId);
  });

  mesh.setContainsRoot();
  mesh.setRoot();
}
void drawNoConnectionPage()
{
  currentPage = NO_CONNECT_PAGE; // sets currentPage

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(40, SCREEN_HEIGHT / 2);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_RED);
  tft.print("No Trainer Connected!");

}