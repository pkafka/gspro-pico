#include <KeyboardBLE.h> // Handles the secure LE pairing engine natively
#include <Adafruit_NeoPixel.h>

// --- PIN ALLOCATIONS ---
const int buttonPins[] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 26, 27, 28, 6, 7};
const int numButtons   = 18;
const int LED_PIN      = 9;  // Drives the 5V WS2812B Data Line through the level shifter

// --- LED STRIP CONFIGURATION ---
#define NUM_LEDS 30          // TODO: Change this to the exact number of LEDs on your final strip
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
uint16_t rainbowHue = 0; // Tracks the color wheel position smoothly

// State variable to track camera mode
bool isFreeFlight = false; 

// --- HELPER FUNCTION FOR SOLID COLORS ---
void setStripColor(uint8_t r, uint8_t g, uint8_t b) {
  for(int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

// --- HELPER FUNCTION FOR RAINBOW ANIMATION FRAME ---
void cycleRainbowFrame() {
  for (int i = 0; i < strip.numPixels(); i++) {
    int pixelHue = rainbowHue + (i * 65536L / strip.numPixels());
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue, 255, 150))); // Brighter for production build
  }
  strip.show();
  rainbowHue += 1200; // Speed of the rainbow cycle color shift
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
  strip.setBrightness(150); // High global brightness for final enclosure visibility
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
        
        // Instant static color shifts for mode awareness
        if (isFreeFlight) {
          setStripColor(0, 100, 255); // Flight Blue
        } else {
          setStripColor(0, 180, 40);  // Standard Golf Green
        }
      }
      
      // =======================================================
      // AUTO-RESET SAFETY CATCH (PIN 16 / PIN 27)
      // =======================================================
      else if (currentPin == 27 || currentPin == 16) {
        isFreeFlight = false; 
        setStripColor(0, 180, 40); // Force back to green baseline
        KeyboardBLE.press(keyMap[i]);
      }

      // =======================================================
      // CUSTOM MULLIGAN MACRO (PIN 14 - CTRL + M + AMBER STROBE)
      // =======================================================
      else if (currentPin == 14) {
        KeyboardBLE.press(KEY_LEFT_CTRL);
        delay(10);
        KeyboardBLE.press('m');
        delay(50);
        KeyboardBLE.releaseAll();

        // High visibility triple amber alert pulse sequence
        for (int flash = 0; flash < 3; flash++) {
          setStripColor(0, 0, 0);       // Dark
          delay(120);                      
          setStripColor(255, 130, 0);   // Punchy Deep Amber
          delay(120);                      
        }
        
        // Revert cleanly back to whatever state the box is currently in
        if (isFreeFlight) setStripColor(0, 100, 255);
        else setStripColor(0, 180, 40);
      } 
      
      // =======================================================
      // STANDARD BUTTON PRESSED -> RAINBOW ANIMATION RUN
      // =======================================================
      else {
        KeyboardBLE.press(keyMap[i]);
        
        // Run the rainbow wave loop as long as the button is down (minimum 800ms)
        unsigned long startTime = millis();
        while(digitalRead(currentPin) == LOW || (millis() - startTime < 800)) {
          cycleRainbowFrame();
          delay(20); // Silky smooth 50 FPS refresh rate
        }
        
        KeyboardBLE.release(keyMap[i]);
        
        // Revert clean to baseline state upon finger/foot lift
        if (isFreeFlight) setStripColor(0, 100, 255);
        else setStripColor(0, 180, 40);
      }
      
      // Hardware safety debounce lock
      while(digitalRead(currentPin) == LOW) {
        delay(5); 
      }
      
      lastDebounceTime[i] = millis();
    }
  }
  delay(systemTickDelay); 
}