#include "LEDMatrix8.h"

const unsigned numDisplays = 5;
const unsigned speedMillis = 22;

//LEDMatrix8 lc = LEDMatrix8(data, clock, cs, numDisplays);
LEDMatrix8 matrix = LEDMatrix8(0, 1, 2, numDisplays);

void setup()
{
  matrix.setIntensity(2);
}

void loop()
{
  simpleScroller("      Arduino LED Matrix Scroller with LEDMatrix8 library");
}

void simpleScroller(const char * txt)
{
  static uint32_t tm = 0;
  static unsigned x  = 0;
  uint32_t t = millis();

  if (t - tm > speedMillis) {
    tm = t;
    if (++x >= strlen(txt) * 8) x = 0;
    matrix.clearDisplay();
    matrix.setCursor(-x, 0);
    matrix.println(txt);
    matrix.update();
  }
}
