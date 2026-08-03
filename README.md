# Custom ESP32 Tamagotchi

A from-scratch Tamagotchi-style pet — no ROM emulation, no copyright ROM file
needed. Fully custom pet logic, custom sprite drawing, big color screen, and
real MP3 music playback.

## Menu

Cycle with LEFT/RIGHT, confirm with SELECT:
1. **Feed** – raises hunger, small happiness boost
2. **Pet** – raises happiness
3. **Scold** – raises discipline, costs some happiness
4. **Sleep** – toggle sleep mode; energy regenerates while asleep
5. **Music** – plays a random song (1 of 10) from the SD card over the DFPlayer Mini.
   Pressing SELECT again while a song plays skips to a new random track.

## Parts list

- ESP32 DevKit (any WROOM-32 board)
- ST7735S 128x160 SPI color TFT display
- DFPlayer Mini MP3 module
- Small 8-ohm speaker
- MicroSD card (formatted FAT32), with `0001.mp3` ... `0010.mp3` in the root
- 3x momentary push buttons
- Breadboard/wires, 3.3V-friendly wiring (ESP32 is NOT 5V tolerant on most pins)

## Wiring

**ST7735 display:**
| Display pin | ESP32 pin |
|---|---|
| CS   | GPIO 5 |
| DC   | GPIO 2 |
| RST  | GPIO 4 |
| SCLK | GPIO 18 |
| MOSI | GPIO 23 |
| VCC  | 3.3V |
| GND  | GND |

**DFPlayer Mini:**
| DFPlayer pin | ESP32 pin |
|---|---|
| RX  | GPIO 17 (through ~1k resistor recommended) |
| TX  | GPIO 16 |
| VCC | 5V |
| GND | GND |
| SPK+/SPK- | to speaker |

**Buttons** (each to GND, using internal pull-ups):
| Button | ESP32 pin |
|---|---|
| Left   | GPIO 32 |
| Select | GPIO 33 |
| Right  | GPIO 25 |

## Building

This is a PlatformIO project.

1. Install [PlatformIO](https://platformio.org/) (VSCode extension or CLI)
2. Open this folder as a PlatformIO project
3. Copy 10 MP3 files onto a microSD card, named `0001.mp3` through `0010.mp3`, insert into the DFPlayer Mini
4. Run the `Upload` task targeting `esp32dev`

## Project structure

```
platformio.ini       - build config, ESP32 target + library deps
include/pet.h         - pet state + mood enum
include/display.h     - screen rendering interface
include/music.h       - DFPlayer control interface
src/pet.cpp           - stat decay + 5 actions (feed/pet/scold/sleep/music)
src/display.cpp       - procedurally-drawn sprite per mood + menu/stat UI
src/music.cpp         - DFPlayer Mini driver (random song selection)
src/main.cpp          - button handling, wires pet+display+music together
```

## Notes on the sprite art

The character is currently drawn **procedurally** (circles/lines via
Adafruit_GFX) rather than from fixed bitmap files — this was a deliberate
choice to keep the code short and easy to read/tweak. `Display::drawFace()`
in `src/display.cpp` is where each mood's face is drawn; swap in your own
`drawBitmap()` calls there if you'd rather use hand-drawn sprite art (e.g.
exported as XBM/RGB565 arrays from a tool like image2cpp).

## Tuning

- Stat decay rate: `DECAY_INTERVAL_MS` in `src/pet.cpp`
- Action strength (how much feed/pet/scold move stats): inside each method
  in `src/pet.cpp`
- DFPlayer volume: `_player.volume(22)` in `src/music.cpp` (0-30)
- Screen colors: the `COL_*` constants at the top of `src/display.cpp`
