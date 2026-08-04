#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "pet.h"
#include "sprites.h"

// Wiring (ESP32 <-> ST7735S 128x160 SPI):
//   TFT_CS   -> GPIO 27
//   TFT_DC   -> GPIO 14 A0
//   TFT_RST  -> GPIO 4
//   TFT_SCLK -> GPIO 18 (VSPI SCK)
//   TFT_MOSI -> GPIO 23 (VSPI MOSI) SDA
//   VCC -> 3.3V, GND -> GND
//
// Note: CS and DC were moved off GPIO5/GPIO2 - those are ESP32 "strapping"
// pins that affect boot mode. Having the display hold them in a certain
// state during reset/upload can prevent the chip from talking to its own
// flash memory during flashing. GPIO27/14 avoid that entirely, so you
// shouldn't need to unplug the display before uploading anymore.
//
// Note: ST7735 boards come with different internal "tabs" (color offsets).
// If colors look shifted/cropped on first boot, try INITR_GREENTAB or
// INITR_144GREENTAB instead of INITR_BLACKTAB in Display::begin().

class Display {
  public:
    void begin();
    // Draws the character sprite (from uploaded PNG art) for the given
    // mood, plus the 5-icon menu bar with `selected` highlighted.
    void render(PetMood mood, MenuItem selected, const PetState &state, uint8_t nowSongIndex = 0);
    // Redraws ONLY the sprite area (not the whole screen) - call this
    // every loop() so character animations keep cycling frames smoothly,
    // without the full-screen flicker a complete redraw would cause.
    void updateSprite(PetMood mood);

  private:
    Adafruit_ST7735 _tft{27, 14, 4}; // CS, DC, RST

    // Animation state - which frame of the current mood's sequence we're on,
    // and when to advance to the next one.
    PetMood _lastAnimMood = PetMood::IDLE;
    uint8_t _frameIndex = 0;
    unsigned long _lastFrameMs = 0;

    void drawSprite(PetMood mood);
    void drawMenuBar(MenuItem selected);
    void drawStatBars(const PetState &state);
    void drawMusicOverlay(uint8_t songIndex);
};

#endif