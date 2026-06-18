#include <KeyboardBLE.h> // Handles the secure LE pairing engine natively

// --- PIN ALLOCATIONS ---
const int buttonPins[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 26, 27, 28, 6, 7};
const int numButtons   = 18;
const int ropeLightPin = 9; // Drives the optocoupler module for the 12V rope light

// --- NATIVE USB SCAN CODE KEY MAP ---
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
  'a',             // Pin 27 - Reset Aim
  KEY_F5,          // Pin 28 - Free Look Camera Toggle
  'q',             // Pin 6  - Camera Altitude UP
  'e'              // Pin 7  - Camera Altitude DOWN
};

unsigned long lastDebounceTime[numButtons] = {0};

// State variable to track camera mode
bool isFreeFlight = false; 

void setup() {
  // Initialize the Bluetooth HID service and assign the broadcast name cleanly
  KeyboardBLE.begin("GSPro Box");
  
  // Configure all arcade input pins with internal PULLUP resistors
  for (int i = 0; i < numButtons; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // Configure the MOSFET module control pin as an active output
  pinMode(ropeLightPin, OUTPUT);
  digitalWrite(ropeLightPin, HIGH); // Start with rope light steady ON
}

void loop() {
  // Base cycle speed changes dynamically based on flight tracking state
  unsigned long systemTickDelay = isFreeFlight ? 5 : 30;

  for (int i = 0; i < numButtons; i++) {
    int currentPin = buttonPins[i];
    int reading = digitalRead(currentPin);
    
    // --- DYNAMIC DEBOUNCE CALCULATOR ---
    // Default standard inputs to a stable 50ms to prevent jumpy/double inputs
    unsigned long currentDelay = 50; 
    
    // If we are actively in Free Flight mode, drop the joystick and altitude pins down to 5ms
    if (isFreeFlight) {
      if ((currentPin >= 10 && currentPin <= 13) || currentPin == 6 || currentPin == 7) {
        currentDelay = 5; // Lightning fast response for flying
      }
    }

    if (reading == LOW && (millis() - lastDebounceTime[i]) > currentDelay) {
      
      // =======================================================
      // CAMERA TOGGLE CONTROLS (PIN 28 - F5)
      // =======================================================
      if (currentPin == 28) {
        isFreeFlight = !isFreeFlight; // Flip the state
        KeyboardBLE.press(KEY_F5);
      }
      
      // =======================================================
      // AUTO-RESET SAFETY CATCH (PIN 16 / PIN 27)
      // =======================================================
      // If Reset Aim or Escape is hit, force the box out of free flight mode
      else if (currentPin == 27 || currentPin == 16) {
        isFreeFlight = false; 
        KeyboardBLE.press(keyMap[i]);
      }

      // =======================================================
      // CUSTOM MULLIGAN MACRO (PIN 14 - CTRL + M + FLASH)
      // =======================================================
      else if (currentPin == 14) {
        KeyboardBLE.press(KEY_LEFT_CTRL);
        delay(10);
        KeyboardBLE.press('m');
        delay(50);
        KeyboardBLE.releaseAll();

        // Perform a dramatic flash-off sequence against the steady-on state
        for (int flash = 0; flash < 3; flash++) {
          digitalWrite(ropeLightPin, LOW);  // Turn OFF
          delay(120);                      
          digitalWrite(ropeLightPin, HIGH); // Turn ON 
          delay(120);                      
        }
      } 
      
      // =======================================================
      // STANDARD SINGLE KEY PRESSES
      // =======================================================
      else {
        KeyboardBLE.press(keyMap[i]);
      }
      
      // Hardware safety debounce trap (Wait until finger/foot lifts off switch)
      while(digitalRead(currentPin) == LOW) {
        delay(5); 
      }
      
      if (currentPin != 14) {
        KeyboardBLE.release(keyMap[i]);
      }
      
      lastDebounceTime[i] = millis();
    }
  }
  delay(systemTickDelay); // Smooth dynamic clock adjustment
}