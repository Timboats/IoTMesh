// /*
//  * FILENAME: button_tester.cpp
//  * DESCRIPTION: This file is just a simple tester for the button inputs defined in config.h.
//  *
// */

// #include "Arduino.h"
// #include "config.h"

// void setup()
// {
//     Serial.begin(115200);
//     pinMode(DPAD_DOWN, INPUT);
//     pinMode(DPAD_UP, INPUT);
//     pinMode(DPAD_LEFT, INPUT);
//     pinMode(DPAD_RIGHT, INPUT);
//     pinMode(ENTER_BUTTON, INPUT);
//     pinMode(BACK_BUTTON, INPUT);
// }

// void loop()
// {
//     if (digitalRead(DPAD_UP) == LOW) {
//         Serial.println("D-PAD UP pressed");
//     }
//     if (digitalRead(DPAD_DOWN) == LOW) {
//         Serial.println("D-PAD DOWN pressed");
//     }
//     if (digitalRead(DPAD_LEFT) == LOW) {
//         Serial.println("D-PAD LEFT pressed");
//     }
//     if (digitalRead(DPAD_RIGHT) == LOW) {
//         Serial.println("D-PAD RIGHT pressed");
//     }
//     if (digitalRead(ENTER_BUTTON) == LOW) {
//         Serial.println("ENTER button pressed");
//     }
//     if (digitalRead(BACK_BUTTON) == LOW) {
//         Serial.println("BACK button pressed");
//     }
//     delay(200); // Debounce delay
// }