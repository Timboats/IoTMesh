ktt_remote/src/main.cpp

@@ -37,8 +37,8 @@ TFT_eSPI tft = TFT_eSPI();

// Global vars
bool otaEnable = false;
const int DEV_COUNT = 4;
int selectedDevice = 1;
int totalDevices = 0; // Mesh should still be stable with 10 devices connected
int selectedValue = 150;
uint8_t focusIndex = 0;
uint8_t currentPage = NO_CONNECT_PAGE;

@@ -46,7 +46,7 @@ uint8_t currentPage = NO_CONNECT_PAGE;
unsigned long lastDebounce = 0; // for input buttons
const unsigned long DEBOUNCE_DELAY = 200;

int totalDevices = 0;



void setup()


@@ -149,14 +149,13 @@ void loop()
    if(pressed(DPAD_UP) || pressed(DPAD_DOWN)) 
    {
      focusIndex = 1 - focusIndex;
      //updateDeviceLine();
      drawSendPage();
    }
    else if(pressed(DPAD_LEFT)) 
    {
      if(focusIndex == 0) 
      {
        selectedDevice = (selectedDevice + DEV_COUNT - 2) % DEV_COUNT + 1;
        selectedDevice = (selectedDevice + totalDevices - 2) % totalDevices + 1;
        drawSendPage();
      } 
      else 

@@ -170,7 +169,7 @@ void loop()
    {
      if(focusIndex == 0) 
      {
        selectedDevice = selectedDevice % DEV_COUNT + 1;
        selectedDevice = selectedDevice % totalDevices + 1;
        drawSendPage();
      } 
      else 