#include <KeyboardBLE.h> // Handles the secure LE pairing engine natively
#include <Adafruit_NeoPixel.h>

// --- PIN ALLOCATIONS ---
const int buttonPins[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 26, 27, 28, 6, 7};
const int numButtons   = 18;
const int LED_PIN      = 9;  // Drives the 5V WS2812B Data Line

// --- LED STRIP CONFIGURATION ---
#define NUM_LEDS 30          // Change this to the exact number of LEDs on your strip
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

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

// --- HELPER FUNCTION FOR STRIP COLORS ---
void setStripColor(uint8_t r, uint8_t g, uint8_t b) {
  for(int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

void setup() {
  // Initialize the Bluetooth HID service and assign the broadcast name cleanly
  KeyboardBLE.begin("GSPro Box");
  
  // Configure all arcade input pins with internal PULLUP resistors
  for (int i = 0; i < numButtons; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // Initialize NeoPixel Strip
  strip.begin();
  strip.setBrightness(150); // Set global brightness (0 to 255) to manage draw/heat
  setStripColor(0, 180, 40); // Initial State: Solid Golf Green
}

void loop() {
  // Base cycle speed changes dynamically based on flight tracking state
  unsigned long systemTickDelay = isFreeFlight ? 5 : 30;

  for (int i = 0; i < numButtons; i++) {
    int currentPin = buttonPins[i];
    int reading = digitalRead(currentPin);
    
    // --- DYNAMIC DEBOUNCE CALCULATOR ---
    unsigned long currentDelay = 50; 
    
    // Drop latency down if actively flying
    if (isFreeFlight) {
      if ((currentPin >= 10 && currentPin <= 13) || currentPin == 6 || currentPin == 7) {
        currentDelay = 5; 
      }
    }

    if (reading == LOW && (millis() - lastDebounceTime[i]) > currentDelay) {
      
      // =======================================================
      // CAMERA TOGGLE CONTROLS (PIN 28 - F5)
      // =======================================================
      if (currentPin == 28) {
        isFreeFlight = !isFreeFlight; // Flip the state
        KeyboardBLE.press(KEY_F5);
        
        // Shift light profile instantly based on camera state
        if (isFreeFlight) {
          setStripColor(0, 100, 255); // Blue tint for flight mode
        } else {
          setStripColor(0, 180, 40);  // Return to Green for normal play
        }
      }
      
      // =======================================================
      // AUTO-RESET SAFETY CATCH (PIN 16 / PIN 27)
      // =======================================================
      else if (currentPin == 27 || currentPin == 16) {
        isFreeFlight = false; 
        setStripColor(0, 180, 40); // Restore normal green profile
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

        // Perform a quick triple-pulse flash sequence natively down the data line
        for (int flash = 0; flash < 3; flash++) {
          setStripColor(0, 0, 0);       // Blackout
          delay(120);                      
          setStripColor(255, 180, 0);   // High visibility amber alert pulse
          delay(120);                      
        }
        
        // Return to standard play lighting state
        setStripColor(0, 180, 40);
      } 
      
      // =======================================================
      // STANDARD SINGLE KEY PRESSES
      // =======================================================
      else {
        KeyboardBLE.press(keyMap[i]);
      }
      
      // Hardware safety debounce trap (Wait until foot/finger completely clears switch)
      while(digitalRead(currentPin) == LOW) {
        delay(5); 
      }
      
      if (currentPin != 14) {
        KeyboardBLE.release(keyMap[i]);
      }
      
      lastDebounceTime[i] = millis();
    }
  }
  delay(systemTickDelay); 
}