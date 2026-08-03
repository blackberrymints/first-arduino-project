#include <Arduino.h>
#include "pet.h"
#include "display.h"
#include "music.h"

// Buttons (adjust pins to your wiring). Wired to GND with INPUT_PULLUP,
// so pressed = LOW.
static const int PIN_LEFT   = 32;
static const int PIN_SELECT = 33;
static const int PIN_RIGHT  = 25;

static const unsigned long DEBOUNCE_MS = 180;

Pet pet;
Display display;
MusicPlayer music;
HardwareSerial dfSerial(2); // UART2 on ESP32

static int selectedIndex = 0;         // 0..4, maps to MenuItem
static bool musicActive = false;      // true while music screen is showing/playing
static unsigned long lastButtonMs = 0;

// Tracks what was last drawn, so we only redraw the screen when something
// actually changed - avoids clearing+redrawing 20x/sec, which caused visible
// flicker over slower breadboard SPI wiring.
static PetMood lastMood = PetMood::IDLE;
static int lastSelected = -1; // -1 forces the very first draw
static uint8_t lastHunger = 255, lastHappy = 255, lastEnergy = 255, lastDiscipline = 255;
static uint8_t lastSongIndex = 0;

bool pressed(int pin) {
  return digitalRead(pin) == LOW;
}

void handleSelect() {
  MenuItem item = (MenuItem)selectedIndex;

  switch (item) {
    case MenuItem::FEED:
      pet.feed();
      break;
    case MenuItem::PET:
      pet.pet();
      break;
    case MenuItem::SCOLD:
      pet.scold();
      break;
    case MenuItem::SLEEP:
      pet.toggleSleep();
      break;
    case MenuItem::MUSIC:
      if (!musicActive) {
        pet.onMusicStart();
        music.playRandomSong();
        musicActive = true;
      } else {
        // pressing select again while music plays skips to a new random song
        music.playRandomSong();
      }
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_SELECT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);

  pet.begin();
  display.begin();

  if (!music.begin(dfSerial)) {
    Serial.println("DFPlayer Mini not detected - check wiring/SD card");
  }
}

void loop() {
  unsigned long now = millis();

  if (now - lastButtonMs > DEBOUNCE_MS) {
    if (pressed(PIN_LEFT)) {
      selectedIndex = (selectedIndex + 4) % 5; // move left, wrap
      lastButtonMs = now;
    } else if (pressed(PIN_RIGHT)) {
      selectedIndex = (selectedIndex + 1) % 5; // move right, wrap
      lastButtonMs = now;
    } else if (pressed(PIN_SELECT)) {
      handleSelect();
      lastButtonMs = now;
    }
  }

  // If a song finished playing, return the pet to idle/sleeping and
  // clear the "now playing" overlay.
  if (musicActive && !music.isPlaying()) {
    pet.onMusicEnd();
    musicActive = false;
  }

  pet.update();

  // Cheap, small-area redraw every loop - keeps sleep/pet animations
  // cycling frames smoothly without the full-screen flicker.
  display.updateSprite(pet.mood());

  const PetState &st = pet.state();
  uint8_t songNow = musicActive ? music.lastSongIndex() : 0;
  bool changed =
      pet.mood() != lastMood ||
      selectedIndex != lastSelected ||
      st.hunger != lastHunger ||
      st.happiness != lastHappy ||
      st.energy != lastEnergy ||
      st.discipline != lastDiscipline ||
      songNow != lastSongIndex;

  if (changed) {
    display.render(pet.mood(), (MenuItem)selectedIndex, st, songNow);
    lastMood = pet.mood();
    lastSelected = selectedIndex;
    lastHunger = st.hunger;
    lastHappy = st.happiness;
    lastEnergy = st.energy;
    lastDiscipline = st.discipline;
    lastSongIndex = songNow;
  }

  delay(50); // simple frame pacing; not time-critical for this UI
}
