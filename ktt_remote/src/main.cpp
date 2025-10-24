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

// TASK HANDLERS
TaskHandle_t uiHandler;

// TASK FUNCTIONS 
void uiTask(void* params);

// FUNCTION DECLARATIONS
void initializeTFT();

// PERIPHERALS
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST); // Bad practice to have globals but for simplicity in this example we will do it


//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
void setup()
{
  /*
  tft.begin();
  tft.setRotation(1);  // Landscape mode
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(40, 10);
  tft.println("Tempo Trainer");
  */
}

void loop()
{

}


