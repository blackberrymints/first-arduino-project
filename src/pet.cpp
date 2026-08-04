#include "pet.h"
#include <Preferences.h>

static Preferences prefs;

// --- Tunable timing/rates ---
static const unsigned long DECAY_INTERVAL_MS   = 60UL * 1000UL; // check once a minute
static const unsigned long EATING_ANIMATION_MS = 22UL * 150UL;  // one pass through all eat frames
static const unsigned long PETTING_ANIMATION_MS = 3UL * 1000UL;

void Pet::begin() {
  load();
  _state.lastDecayTick = millis();
}

void Pet::clampStats() {
  auto clamp = [](uint8_t &v, int delta) {
    int n = (int)v + delta;
    if (n < 0) n = 0;
    if (n > 100) n = 100;
    v = (uint8_t)n;
  };
  // no-op helper kept for callers; individual actions clamp inline below
  (void)clamp;
}

void Pet::update() {
  unsigned long now = millis();

  // Return to the happiness-based face after a one-shot action animation.
  if (_state.moodUntil != 0 && now >= _state.moodUntil) {
    _state.moodUntil = 0;
    refreshBaseMood();
  }

  // Stat decay, once per DECAY_INTERVAL_MS
  if (now - _state.lastDecayTick >= DECAY_INTERVAL_MS) {
    _state.lastDecayTick = now;
    _state.ageMinutes++;

    if (_state.sleeping) {
      // Resting: energy climbs, other stats barely move
      if (_state.energy < 100) _state.energy = min(100, _state.energy + 4);
      if (_state.hunger > 0)   _state.hunger = max(0, _state.hunger - 1);
    } else {
      if (_state.hunger > 0)     _state.hunger--;
      if (_state.happiness > 0)  _state.happiness--;
      if (_state.energy > 0)     _state.energy--;
      // Discipline drifts back toward neutral (30) slowly
      if (_state.discipline > 30) _state.discipline--;
      else if (_state.discipline < 30) _state.discipline++;
    }

    // Auto-save every 15 minutes of pet-time to limit flash wear
    if (_state.ageMinutes % 15 == 0) {
      save();
    }

    if (_state.moodUntil == 0 && _state.currentMood != PetMood::LISTENING_MUSIC) {
      refreshBaseMood();
    }
  }
}

void Pet::setTemporaryMood(PetMood m, unsigned long durationMs) {
  _state.currentMood = m;
  _state.moodUntil = millis() + durationMs;
}

PetMood Pet::baseMood() const {
  if (_state.sleeping) return PetMood::SLEEPING;
  if (_state.happiness < 20) return PetMood::SAD;
  if (_state.happiness > 85) return PetMood::HAPPY;
  return PetMood::IDLE;
}

void Pet::refreshBaseMood() {
  _state.currentMood = baseMood();
}

void Pet::feed() {
  if (_state.sleeping) return; // no feeding while asleep
  _state.hunger = min(100, _state.hunger + 30);
  _state.happiness = min(100, _state.happiness + 5);
  setTemporaryMood(PetMood::EATING, EATING_ANIMATION_MS);
}

void Pet::pet() {
  if (_state.sleeping) return;
  _state.happiness = min(100, _state.happiness + 15);
  setTemporaryMood(PetMood::PETTING, PETTING_ANIMATION_MS);
}

void Pet::scold() {
  if (_state.sleeping) return;
  _state.discipline = min(100, _state.discipline + 15);
  _state.happiness = (_state.happiness >= 10) ? _state.happiness - 10 : 0;
  refreshBaseMood();
}

void Pet::toggleSleep() {
  _state.sleeping = !_state.sleeping;
  _state.moodUntil = 0;
  refreshBaseMood();
}

void Pet::onMusicStart() {
  if (_state.sleeping) return;
  _state.currentMood = PetMood::LISTENING_MUSIC;
  _state.moodUntil = 0; // held until onMusicEnd()/onMusicStop()
}

void Pet::onMusicEnd() {
  _state.happiness = min(100, _state.happiness + 10);
  _state.moodUntil = 0;
  refreshBaseMood();
}

void Pet::onMusicStop() {
  _state.moodUntil = 0;
  refreshBaseMood();
}

PetMood Pet::mood() const {
  return _state.currentMood;
}

void Pet::save() {
  prefs.begin("tamagotchi", false);
  prefs.putUChar("hunger", _state.hunger);
  prefs.putUChar("happy", _state.happiness);
  prefs.putUChar("energy", _state.energy);
  prefs.putUChar("disc", _state.discipline);
  prefs.putUInt("age", _state.ageMinutes);
  prefs.putBool("sleep", _state.sleeping);
  prefs.end();
}

void Pet::load() {
  prefs.begin("tamagotchi", true);
  _state.hunger     = prefs.getUChar("hunger", 70);
  _state.happiness  = prefs.getUChar("happy", 70);
  _state.energy     = prefs.getUChar("energy", 70);
  _state.discipline = prefs.getUChar("disc", 30);
  _state.ageMinutes = prefs.getUInt("age", 0);
  _state.sleeping   = prefs.getBool("sleep", false);
  prefs.end();
  refreshBaseMood();
}
