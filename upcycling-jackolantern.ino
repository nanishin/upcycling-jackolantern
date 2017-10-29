#include "Adafruit_CircuitPlayground.h"
#include <Wire.h>
#include <SPI.h>

// Adjust this number for the sensitivity of the 'click' force
// this strongly depend on the range! for 16G, try 5-10
// for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80
#define CLICKTHRESHHOLD 120

// A few music note frequencies as defined in this tone example:
//   https://www.arduino.cc/en/Tutorial/toneMelody
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988

// Define note durations.  You only need to adjust the whole note
// time and other notes will be subdivided from it directly.
#define WHOLE         2200       // Length of time in milliseconds of a whole note (i.e. a full bar).
#define HALF          WHOLE/2
#define QUARTER       HALF/2
#define EIGHTH        QUARTER/2
#define EIGHTH_TRIPLE QUARTER/3
#define SIXTEENTH     EIGHTH/2

void setup() {
  CircuitPlayground.begin();
  CircuitPlayground.setBrightness(255); // LEDs full blast!
    
  CircuitPlayground.setAccelRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G!
  
  // 0 = turn off click detection & interrupt
  // 1 = single click only interrupt output
  // 2 = double click only interrupt output, detect single click
  // Adjust threshhold, higher numbers are less sensitive
  CircuitPlayground.setAccelTap(1, CLICKTHRESHHOLD);
  
  // have a procedure called when a tap is detected
  attachInterrupt(digitalPinToInterrupt(CPLAY_LIS3DH_INTERRUPT), tapTime, FALLING);
}

uint8_t prev = 128;              // Start brightness in middle

uint8_t tap_detected = 0;

void tapTime(void) {
  // do something :)
  tap_detected=1;
   Serial.print("Tap! ");
   Serial.println(millis()); // the time
}

void loop() {
  uint8_t lvl = random(64, 192); // End brightness at 128±64
  split(prev, lvl, 32);          // Start subdividing, ±32 at midpoint
  prev = lvl;                    // Assign end brightness to next start

  if (tap_detected) {
    celebrateSong();
    tap_detected=0;
  }
}

void split(uint8_t y1, uint8_t y2, uint8_t offset) {
  if(offset) { // Split further into sub-segments w/midpoint at ±offset
    uint8_t mid = (y1 + y2 + 1) / 2 + random(-offset, offset);
    split(y1 , mid, offset / 2); // First segment (offset is halved)
    split(mid, y2 , offset / 2); // Second segment (ditto)
  } else { // No further subdivision - y1 determines LED brightness
    uint32_t c = (((int)(pow((float)y1 / 255.0, 2.7) * 255.0 + 0.5) // Gamma
                 * 0x1004004) >> 8) & 0xFF3F03; // Expand to 32-bit RGB color
    for(uint8_t i=0; i<10; i++) CircuitPlayground.strip.setPixelColor(i, c);
    CircuitPlayground.strip.show();
    delay(4);
  }
}

void playNote(int frequency, int duration, bool hold=false, bool measure=true) {
  if (hold) {
    // For a note that's held play it a little longer than the specified duration
    // so it blends into the next tone (but there's still a small delay to 
    // hear the next note).
    CircuitPlayground.playTone(frequency, duration + duration/32, false);
  }
  else {
    // For a note that isn't held just play it for the specified duration.
    CircuitPlayground.playTone(frequency, duration, false);
  }

  delay(duration + duration/16);
}

// Song to play when the candles are blown out.
void celebrateSong() {
  // Play a little charge melody, from:
  //  https://en.wikipedia.org/wiki/Charge_(fanfare)
  // Note the explicit boolean parameters in particular the measure=false
  // at the end.  This means the notes will play without any breath measurement
  // logic.  Without this false value playNote will try to keep waiting for candles
  // to blow out during the celebration song!
  playNote(NOTE_G4, EIGHTH_TRIPLE, true, false);
  playNote(NOTE_C5, EIGHTH_TRIPLE, true, false);
  playNote(NOTE_E5, EIGHTH_TRIPLE, false, false);
  playNote(NOTE_G5, EIGHTH, true, false);
  playNote(NOTE_E5, SIXTEENTH, false);
  playNote(NOTE_G5, HALF, false);
}
