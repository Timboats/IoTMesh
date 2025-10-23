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

void setup()
{

}

void loop()
{

}


