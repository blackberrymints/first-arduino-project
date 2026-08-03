#include "display.h"
#include <SPI.h>

// 128x160 screen layout:
//   y   0-88  : character sprite area
//   y  92-132 : stat bars (hunger/happiness/energy/discipline)
//   y 136-160 : menu icon bar (5 icons)

static const uint16_t COL_BG     = 0x0000; // black
static const uint16_t COL_BODY   = 0xFFE0; // yellow (swap for your own color/sprite style)
static const uint16_t COL_ACCENT = 0x07FF; // cyan
static const uint16_t COL_TEXT   = 0xFFFF; // white
static const uint16_t COL_BAD    = 0xF800; // red
static const uint16_t COL_GOOD   = 0x07E0; // green

void Display::begin() {
  // Most generic ST7735S 1.8" boards use the "black tab" variant. If colors
  // look wrong/shifted, try INITR_GREENTAB instead (see note in display.h).
  _tft.initR(INITR_BLACKTAB);
  _tft.setSPISpeed(8000000); // 8MHz - fine now that we only redraw on change
  _tft.setRotation(0); // portrait, 128 wide x 160 tall
  _tft.fillScreen(COL_BG);
}

void Display::drawSprite(PetMood mood) {
  const uint16_t* const* frames;
  uint8_t frameCount;

  switch (mood) {
    case PetMood::SLEEPING:
      frames = SLEEP_FRAMES;
      frameCount = SLEEP_FRAME_COUNT;
      break;
    case PetMood::HAPPY:
      frames = PET_FRAMES;
      frameCount = PET_FRAME_COUNT;
      break;
    // EATING, SAD, and LISTENING_MUSIC don't have dedicated art yet -
    // falls back to the idle frame until more sprites are added.
    case PetMood::IDLE:
    case PetMood::EATING:
    case PetMood::SAD:
    case PetMood::LISTENING_MUSIC:
    default:
      frames = IDLE_FRAMES;
      frameCount = IDLE_FRAME_COUNT;
      break;
  }

  unsigned long now = millis();

  if (mood != _lastAnimMood) {
    // Mood just changed - restart the animation from frame 0
    _frameIndex = 0;
    _lastFrameMs = now;
    _lastAnimMood = mood;
  } else if (frameCount > 1 && now - _lastFrameMs >= 150) {
    // Advance to the next frame, looping back to 0 at the end
    _frameIndex = (_frameIndex + 1) % frameCount;
    _lastFrameMs = now;
  }

  int x = (128 - SPRITE_W) / 2; // centered horizontally
  int y = 8;
  _tft.drawRGBBitmap(x, y, frames[_frameIndex], SPRITE_W, SPRITE_H);
}

void Display::drawStatBars(const PetState &state) {
  struct Row { const char* label; uint8_t val; uint16_t color; };
  Row rows[4] = {
    {"HUN", state.hunger,     COL_GOOD},
    {"HAP", state.happiness,  0xFFE0},
    {"ENR", state.energy,     COL_ACCENT},
    {"DIS", state.discipline, COL_BAD},
  };

  int y = 94;
  for (int i = 0; i < 4; i++) {
    _tft.setTextColor(COL_TEXT);
    _tft.setTextSize(1);
    _tft.setCursor(2, y);
    _tft.print(rows[i].label);

    int barX = 24, barW = 100, barH = 6;
    _tft.drawRect(barX, y - 1, barW, barH, COL_TEXT);
    int fillW = (rows[i].val * (barW - 2)) / 100;
    _tft.fillRect(barX + 1, y, fillW, barH - 2, rows[i].color);

    y += 10;
  }
}

void Display::drawMenuBar(MenuItem selected) {
  const char* labels[5] = {"FEED", "PET", "SCLD", "SLEP", "MUSC"};
  int barY = 136, barH = 24, iconW = 128 / 5;

  for (int i = 0; i < 5; i++) {
    int x = i * iconW;
    bool isSel = ((int)selected == i);
    _tft.fillRect(x, barY, iconW, barH, isSel ? COL_ACCENT : COL_BG);
    _tft.drawRect(x, barY, iconW, barH, COL_TEXT);
    _tft.setTextColor(isSel ? COL_BG : COL_TEXT);
    _tft.setTextSize(1);
    _tft.setCursor(x + 1, barY + barH / 2 - 4);
    _tft.print(labels[i]);
  }
}

void Display::drawMusicOverlay(uint8_t songIndex) {
  _tft.fillRect(4, 84, 120, 16, COL_BG);
  _tft.drawRect(4, 84, 120, 16, COL_ACCENT);
  _tft.setTextColor(COL_TEXT);
  _tft.setTextSize(1);
  _tft.setCursor(8, 89);
  _tft.print("Song ");
  _tft.print(songIndex);
  _tft.print("/10");
}

void Display::updateSprite(PetMood mood) {
  drawSprite(mood); // drawSprite already only touches the 32x32 sprite area
}

void Display::render(PetMood mood, MenuItem selected, const PetState &state, uint8_t nowSongIndex) {
  _tft.fillScreen(COL_BG);
  drawSprite(mood);
  if (mood == PetMood::LISTENING_MUSIC && nowSongIndex > 0) {
    drawMusicOverlay(nowSongIndex);
  }
  drawStatBars(state);
  drawMenuBar(selected);
}
