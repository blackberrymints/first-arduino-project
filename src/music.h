#ifndef MUSIC_H
#define MUSIC_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// Wiring (adjust if you use different UART pins):
//   ESP32 TX2 (GPIO17) -> DFPlayer RX
//   ESP32 RX2 (GPIO16) -> DFPlayer TX (through a ~1k resistor is recommended)
//   DFPlayer SPK+/SPK- -> small 8ohm speaker
//   DFPlayer VCC -> 5V, GND -> GND
//   MicroSD card in DFPlayer, files named 0001.mp3 ... 0010.mp3 in the root
//   (DFPlayer indexes files by the numeric prefix, 1-10 here)

static const uint8_t TOTAL_SONGS = 10;

class MusicPlayer {
  public:
    bool begin(HardwareSerial &serial);
    void playRandomSong();     // picks 1-10 at random and plays it
    void stop();
    bool hasFinished();        // true when DFPlayer reports the track ended
    uint8_t lastSongIndex() const { return _lastIndex; }

  private:
    DFRobotDFPlayerMini _player;
    uint8_t _lastIndex = 0;
    bool _ready = false;
};

#endif
