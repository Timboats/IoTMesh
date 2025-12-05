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
#define SPI_MOSI 26 // SPI MOSI pin
#define SPI_MISO 27 // SPI MISO pin
#define SPI_SCK 25 // SPI Clock pin

// Other defs

#define MESH_PREFIX "IoTHub" // Mesh WiFI name
#define MESH_PASSWORD "IOAIHTHG" // Mesh password
#define MESH_PORT 5555 // Mesh port

#define SCREEN_WIDTH 320 // LCD display width, in pixels
#define SCREEN_HEIGHT 240 // LCD display height, in pixels

#define NO_CONNECT_PAGE 0 // UI No connection page
#define SEND_PAGE 1 // UI send to trainer page

#define MAX_DISCONNECT_TIMEOUT 5000 // max time til last callback in ms



// OTA defines
#define OTA_SSID "KTTREM" // hotspot ssid
#ifdef OTA_SSID
    #define OTA_PSK "kttrem2018"
    #define OTA_host "kttremotedev"
#endif