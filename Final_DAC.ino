#include "Arduino.h"

// --- 16-Step Sine Table for 4-bit DAC (values 0 to 15) ---
const uint8_t SineWave[16] = {
  8, 10, 12, 14, 15, 15, 14, 12,
  8, 6, 4, 2, 0, 0, 2, 4
};

// --- Global Variables ---
volatile uint8_t Index = 0;              // Current index into the sine table
volatile bool waveformActive = false;    // Indicates whether the tone should be output

// --- DAC Output Pins (simulate 4-bit DAC) ---
const int DAC_PINS[4] = {0, 1, 2, 4};

// --- Piano Key Input Pins (active-high) ---
const int KEY_PINS[4] = {D5, D8, D9, D10}; // Corresponding to notes: C, D, E, G

// --- Timer Periods (alarm values in microseconds) for each note ---
// These values determine the timer period for each note's frequency.
const uint32_t timerPeriods[4] = {120, 106, 95, 80}; // Approximate periods for C, D, E, G

// --- Hardware Timer Pointer ---
hw_timer_t *timer = NULL;

// --- Timer Interrupt Service Routine ---
void IRAM_ATTR onTimer(){
  if (waveformActive) {
    Index = (Index + 1) & 0x0F;  // Increment index and wrap around after 15 (16 samples)
    uint8_t outValue = SineWave[Index];
    // Output each bit of the value to the corresponding DAC pin
    for (int i = 0; i < 4; i++) {
      digitalWrite(DAC_PINS[i], (outValue >> i) & 0x01);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize DAC pins as outputs and set them LOW
  for (int i = 0; i < 4; i++) {
    pinMode(DAC_PINS[i], OUTPUT);
    digitalWrite(DAC_PINS[i], LOW);
  }
  
  // Initialize key input pins
  for (int i = 0; i < 4; i++) {
    pinMode(KEY_PINS[i], INPUT);
  }
  
  // --- Setup the Hardware Timer ---
  // With the new API, timerBegin is called with the tick rate.
  // Here, 1,000,000 ticks per second means 1 tick = 1 microsecond.
  timer = timerBegin(1000000);
  
  // Attach the timer ISR; note that no extra parameter is required.
  timerAttachInterrupt(timer, &onTimer);
  
  // Set the initial alarm period corresponding to the default note (Note C)
  // The new timerAlarm function signature requires:
  // timerAlarm(timer, alarm_val, autoreload, counter_val)
  timerAlarm(timer, timerPeriods[0], true, 0);
  
  // Start the timer
  timerStart(timer);
  
  Serial.println("4-Key Digital Piano Initialized (New Timer API).");
}

void loop(){
  bool keyPressed = false;

  // Poll each piano key
  for (int i = 0; i < 4; i++) {
    if (digitalRead(KEY_PINS[i]) == HIGH) {  // Key pressed (active-high)
      keyPressed = true;
      
      // Stop the timer before modifying the alarm period
      timerStop(timer);
      
      // Update the timer alarm with the period corresponding to the detected key.
      // Here we set auto-reload to true and reset the counter (0).
      timerAlarm(timer, timerPeriods[i], true, 0);
      
      waveformActive = true;
      
      // Restart the timer
      timerStart(timer);
      
      // Optional: Print which note is being played.
      if (i == 0) Serial.println("Note C");
      else if (i == 1) Serial.println("Note D");
      else if (i == 2) Serial.println("Note E");
      else if (i == 3) Serial.println("Note G");
      
      // Only process the first detected key press.
      break;
    }
  }
  
  // If no key is pressed, disable the waveform and set DAC outputs to 0 (silence)
  if (!keyPressed) {
    waveformActive = false;
    for (int i = 0; i < 4; i++) {
      digitalWrite(DAC_PINS[i], LOW);
    }
  }
}
