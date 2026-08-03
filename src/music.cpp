#include "music.h"

bool MusicPlayer::begin(HardwareSerial &serial) {
  // DFPlayer Mini talks at 9600 baud
  serial.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17 on ESP32
  _ready = _player.begin(serial, /*isACK=*/true, /*doReset=*/true);
  if (_ready) {
    _player.volume(22); // 0-30, tune to your speaker
  }
  return _ready;
}

void MusicPlayer::playRandomSong() {
  if (!_ready) return;
  // random(1, 11) gives 1..10 inclusive
  uint8_t track = (uint8_t)random(1, TOTAL_SONGS + 1);
  _lastIndex = track;
  _player.play(track); // plays /00track.mp3 by numeric index
}

void MusicPlayer::stop() {
  if (!_ready) return;
  _player.stop();
}

bool MusicPlayer::isPlaying() {
  if (!_ready) return false;
  // DFRobotDFPlayerMini exposes readState()/available() for status polling;
  // simplest robust check is CurrentState == playing after a play() call.
  return _player.readState() == 1; // 1 = playing on most DFPlayer firmware
}
