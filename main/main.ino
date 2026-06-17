#include <KeyboardBLE.h> // Handles the secure LE pairing engine natively

// --- GSPRO ARCADE BUTTON PIN CONNECTIONS ---
// Expanded to 18 inputs total (Adds GPIO 6 and 7 at the end)
const int buttonPins[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 26, 27, 28, 6, 7};
const int numButtons   = 18;

// --- NATIVE USB SCAN CODE KEY MAP ---
const char keyMap[]    = {
  KEY_UP_ARROW,    // Pin 10 - Joystick Up (Fly Forward)
  KEY_DOWN_ARROW,  // Pin 11 - Joystick Down (Fly Backward)
  KEY_LEFT_ARROW,  // Pin 12 - Joystick Left (Fly Left)
  KEY_RIGHT_ARROW, // Pin 13 - Joystick Right (Fly Right)
  ' ',             // Pin 14 - Placeholder for Mulligan Macro
  KEY_RETURN,      // Pin 15 - Enter / Select
  KEY_ESC,         // Pin 16 - Escape / Menu
  'i',             // Pin 17 - Club Up
  'k',             // Pin 18 - Club Down
  'j',             // Pin 19 - Shot Cam
  'o',             // Pin 20 - Flyover
  'u',             // Pin 21 - Putt Toggle
  'c',             // Pin 22 - Tee Left
  'v',             // Pin 26 - Tee Right
  'a',             // Pin 27 - Reset Aim
  KEY_F5,          // Pin 28 - Free Look Camera Toggle
  'q',             // Pin 6  - Camera Altitude UP
  'e'              // Pin 7  - Camera Altitude DOWN
};

// Tracking array to handle fast mechanical button debouncing safely
unsigned long lastDebounceTime[numButtons] = {0};
const unsigned long debounceDelay = 50; // 50ms filtering window

void setup() {
  KeyboardBLE.begin("GSPro Box");
  
  for (int i = 0; i < numButtons; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
}

void loop() {
  for (int i = 0; i < numButtons; i++) {
    int currentPin = buttonPins[i];
    int reading = digitalRead(currentPin);
    
    if (reading == LOW && (millis() - lastDebounceTime[i]) > debounceDelay) {
      
      // Mulligan Macro
      if (currentPin == 14) {
        KeyboardBLE.press(KEY_LEFT_CTRL);
        delay(10);
        KeyboardBLE.press('m');
        delay(50);
        KeyboardBLE.releaseAll();
      } 
      // Standard Keys
      else {
        KeyboardBLE.press(keyMap[i]);
      }
      
      while(digitalRead(currentPin) == LOW) {
        delay(5); 
      }
      
      if (currentPin != 14) {
        KeyboardBLE.release(keyMap[i]);
      }
      
      lastDebounceTime[i] = millis();
    }
  }
  delay(5);
}