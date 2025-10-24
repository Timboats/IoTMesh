/*
 * FILENAME: config.h
 * DESCRIPTION: This file stores all the pin and overall operation configurations of the program
 *
*/

// Pin defs

#define TFT_CS 13 // LCD Chip-Select pin
#define TFT_DC 33 // LCD Data/Command pin
#define TFT_RST 32 // LCD Reset pin
#define DPAD_UP 5 // D-PAD Up pin
#define DPAD_DOWN 17 // D-PAD Down pin
#define DPAD_LEFT 4 // D-PAD Left pin
#define DPAD_RIGHT 16 // D-PAD Right pin
#define ENTER_BUTTON 15 // Enter button pin
#define BACK_BUTTON 14 // Back button pin

// Other defs

#define MESH_PREFIX "IoTHub" // Mesh WiFI name
#define MESH_PASSWORD "IOAIHTHG" // Mesh password
#define MESH_PORT 5555 // Mesh port
#define SCREEN_WIDTH 128 // LCD display width, in pixels
#define SCREEN_HEIGHT 64 // LCD display height, in pixels
//TODO update screen width and height for TFT/LCD