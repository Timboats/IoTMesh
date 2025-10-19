#include <Arduino.h>

// LIBRARIES
#include "painlessMesh.h"      // Mesh WiFi
#include <cppQueue.h>          // Queue
#include <StackArray.h>        // stack (https://github.com/oogre/StackArray)
#include <Adafruit_GFX.h>      // For TFT LCD Screen
#include "Adafruit_ILI9341.h"  // For TFT LCD Screen
#include "ShiftIn.h"
#include <math.h>

// CONSTANTS
#define MESH_PREFIX "IoTHub"     // Mesh WiFI name
#define MESH_PASSWORD "IOAIHTHG"   // Mesh password
#define MESH_PORT 5555         // Mesh port
#define SCREEN_WIDTH 128             // OLED display width, in pixels
#define SCREEN_HEIGHT 64             // OLED display height, in pixels


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


