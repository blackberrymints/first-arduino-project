#ifndef PET_H
#define PET_H

#include <Arduino.h>

// What the display should currently render for the character
enum class PetMood {
  IDLE,
  EATING,
  HAPPY,
  SLEEPING,
  SAD,
  LISTENING_MUSIC
};

enum class MenuItem {
  FEED = 0,
  PET  = 1,
  SCOLD = 2,
  SLEEP = 3,
  MUSIC = 4,
  COUNT = 5
};

struct PetState {
  uint8_t hunger      = 70;  // 0 = starving, 100 = full
  uint8_t happiness   = 70;  // 0 = miserable, 100 = delighted
  uint8_t energy      = 70;  // 0 = exhausted, 100 = fully rested
  uint8_t discipline  = 30;  // 0 = spoiled, 100 = very disciplined
  uint32_t ageMinutes = 0;
  bool sleeping       = false;

  // Timestamps used for decay + mood-flash timing (millis-based)
  unsigned long lastDecayTick = 0;
  unsigned long moodUntil     = 0; // when a temporary mood (EATING/HAPPY/SAD) expires
  PetMood currentMood         = PetMood::IDLE;
};

class Pet {
  public:
    void begin();
    void update();            // call every loop() - handles decay + mood timing
    void feed();
    void pet();
    void scold();
    void toggleSleep();
    void onMusicStart();      // call when music action begins
    void onMusicEnd();        // call when music playback finishes

    PetMood mood() const;
    const PetState& state() const { return _state; }

    void save();               // persist to EEPROM/NVS
    void load();                // restore from EEPROM/NVS

  private:
    PetState _state;
    void setTemporaryMood(PetMood m, unsigned long durationMs);
    void clampStats();
};

#endif
