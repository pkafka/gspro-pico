#include <KeyboardBLE.h> // Handles the secure LE pairing engine natively

// --- GSPRO ARCADE BUTTON PIN CONNECTIONS ---
// Clean, continuous sequence mapped straight down your breakout terminals
const int buttonPins[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 26, 27};
const int numButtons   = 15;

// --- NATIVE USB SCAN CODE KEY MAP ---
// Pin 14 uses a custom macro block below, so its array slot is a placeholder
const char keyMap[]    = {
  KEY_UP_ARROW,    // Pin 10 - Joystick Up
  KEY_DOWN_ARROW,  // Pin 11 - Joystick Down
  KEY_LEFT_ARROW,  // Pin 12 - Joystick Left
  KEY_RIGHT_ARROW, // Pin 13 - Joystick Right
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
  'a'              // Pin 27 - Reset Aim
};

// Tracking array to handle fast mechanical button debouncing safely
unsigned long lastDebounceTime[numButtons] = {0};
const unsigned long debounceDelay = 50; // 50ms filtering window

void setup() {
  // Initialize the Bluetooth HID service and assign the broadcast name cleanly
  KeyboardBLE.begin("GSPro Box");
  
  // Configure all arcade input pins with internal PULLUP resistors
  for (int i = 0; i < numButtons; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
}

void loop() {
  for (int i = 0; i < numButtons; i++) {
    int currentPin = buttonPins[i];
    int reading = digitalRead(currentPin);
    
    // Check if an arcade switch breaks contact and pulls the pin to GND (LOW)
    if (reading == LOW && (millis() - lastDebounceTime[i]) > debounceDelay) {
      
      // =======================================================
      // CUSTOM MULTI-PRESS FOR PIN 14 (CTRL + M MULLIGAN)
      // =======================================================
      if (currentPin == 14) {
        KeyboardBLE.press(KEY_LEFT_CTRL); // 1. Firmly hold Control down
        delay(10);                        // Tiny hardware stabilization delay
        KeyboardBLE.press('m');           // 2. Tap the 'M' key
        delay(50);                        // Hold both for 50ms so Windows registers the combo
        
        KeyboardBLE.releaseAll();         // 3. Cleanly lift BOTH keys simultaneously
      } 
      
      // =======================================================
      // STANDARD SINGLE KEY PRESSES FOR OTHER PINS
      // =======================================================
      else {
        KeyboardBLE.press(keyMap[i]);
      }
      
      // --- HARDWARE SAFETY DEBOUNCE TRAP ---
      // This holds program execution right here until you physically lift your 
      // finger off the arcade button, preventing accidental double-triggers.
      while(digitalRead(currentPin) == LOW) {
        delay(5); 
      }
      
      // Cleanly release standard single keys on physical button lift
      if (currentPin != 14) {
        KeyboardBLE.release(keyMap[i]);
      }
      
      lastDebounceTime[i] = millis();
    }
  }
  delay(5); // Ultra-fast 5ms system cycle tick (Zero input lag)
}