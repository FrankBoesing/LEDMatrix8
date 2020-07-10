#include "LEDMatrix8.h"

//the opcodes for the MAX7221 and MAX7219
#define OP_NOOP   0
#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15

LEDMatrix8::LEDMatrix8(uint8_t dataPin, uint8_t clkPin, uint8_t csPin, uint8_t numDevices) {

  SPI_MOSI = dataPin;
  SPI_CLK = clkPin;
  SPI_CS = csPin;

  pinMode(SPI_CS, OUTPUT);
  digitalWrite(SPI_CS, HIGH);
  pinMode(SPI_CLK, OUTPUT);
  pinMode(SPI_MOSI, OUTPUT);
  digitalWrite(SPI_CLK, LOW);
  digitalWrite(SPI_MOSI, LOW);

  if (numDevices <= 0 || numDevices > 8 ) numDevices = 8;
  maxDevices = numDevices;

  _width = 8 * numDevices;
  font = (uint8_t*)font8x8_icl8x8u;

  for (unsigned i = 0; i < maxDevices; i++) {
    spiTransfer(i, OP_DISPLAYTEST, 0);
    spiTransfer(i, OP_SCANLIMIT, 7);
    spiTransfer(i, OP_DECODEMODE, 0);
    spiTransfer(i, OP_INTENSITY, 7);
  }
  clearDisplay();
  update();
  shutdown(false);
}

void LEDMatrix8::shutdown(bool b) {
  for (unsigned i = 0; i < maxDevices; i++)
    spiTransfer(i, OP_SHUTDOWN, b ? 0 : 1);
}

void LEDMatrix8::setIntensity(int intensity) {
  if (intensity >= 0 && intensity < 16)
    for (unsigned i = 0; i < maxDevices; i++)
      spiTransfer(i, OP_INTENSITY, intensity);
}


void LEDMatrix8::clearDisplay() {
  memset(status, 0, sizeof(status));
}


void LEDMatrix8::drawPixelFast(int x, int y, int c) {
  if (c)
    status[x & 7][x >> 3] |= 1 << y;
  else
    status[x & 7][x >> 3] &= ~(1 << y);
}

void LEDMatrix8::drawPixel(int x, int y, int c) {
  if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height))
    drawPixelFast(x, y, c);
#if 1
  else {
    Serial.print("x:");
    Serial.print(x);
    Serial.print(" y:");
    Serial.println(y);
  }
#endif
}

void LEDMatrix8::drawFastHLine(int x, int y, int w)
{  
  w += x;
  while(--w >= x) {
    drawPixel(w, y, 1);
  } 
}

void LEDMatrix8::drawFastVLine(int x, int y, int h)
{  
  h += y;
  while(--h >= y) {
    drawPixel(x, h, 1);
  } 
}

void LEDMatrix8::drawRect(int x, int y, int w, int h)
{
  drawFastHLine(x+1, y, w-2);
  drawFastHLine(x+1, y+h-1, w-2);
  drawFastVLine(x, y, h);
  drawFastVLine(x+w-1, y, h);
}

void LEDMatrix8::fillRect(int x, int y, int w, int h)
{
  while (h-->0) {
    drawFastHLine(x, y, w);    
    y++;
  }
}

#ifndef swap
#define swap(a, b) { typeof(a) t = a; a = b; b = t; }
#endif

// Bresenham's algorithm - thx wikpedia
void LEDMatrix8::drawLine(int x0, int y0, int x1, int y1)
{
 
  if (y0 == y1) {
    if (x1 > x0) {
      drawFastHLine(x0, y0, x1 - x0 + 1);
    } else if (x1 < x0) {
      drawFastHLine(x1, y0, x0 - x1 + 1);
    } else {
      drawPixel(x0, y0, 1);
    }
    return;
  } else if (x0 == x1) {
    if (y1 > y0) {
      drawFastVLine(x0, y0, y1 - y0 + 1);
    } else {
      drawFastVLine(x0, y1, y0 - y1 + 1);
    }
    return;
  }

  bool steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }
  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  int16_t xbegin = x0;
  if (steep) {
    for (; x0<=x1; x0++) {
      err -= dy;
      if (err < 0) {
        int16_t len = x0 - xbegin;
        if (len) {
          drawFastVLine(y0, xbegin, len + 1);
        } else {
          drawPixel(y0, x0, 1);
        }
        xbegin = x0 + 1;
        y0 += ystep;
        err += dx;
      }
    }
    if (x0 > xbegin + 1) {
      drawFastVLine(y0, xbegin, x0 - xbegin);
    }

  } else {
    for (; x0<=x1; x0++) {
      err -= dy;
      if (err < 0) {
        int16_t len = x0 - xbegin;
        if (len) {
          drawFastHLine(xbegin, y0, len + 1);
        } else {
          drawPixel(x0, y0, 1);
        }
        xbegin = x0 + 1;
        y0 += ystep;
        err += dx;
      }
    }
    if (x0 > xbegin + 1) {
      drawFastHLine(xbegin, y0, x0 - xbegin);
    }
  }

}

void LEDMatrix8::update(void) {
  for (unsigned i = 0; i < 8; i++) {
    digitalWrite(SPI_CS, LOW);
    for (int n = maxDevices - 1; n >= 0; n--) {
      shiftOut(SPI_MOSI, SPI_CLK, MSBFIRST, i + 1);
      shiftOut(SPI_MOSI, SPI_CLK, MSBFIRST, status[i][n]);
    }
    digitalWrite(SPI_CS, HIGH);
  }
}

void LEDMatrix8::invertDisplay(void) {
  for (unsigned i = 0; i < 8; i++) {
    for (int n = maxDevices - 1; n >= 0; n--) {
      status[i][n] = ~status[i][n];
    }
  }
}

void LEDMatrix8::drawChar(int x, int y, unsigned char c) {  
  if (x <= -8 || x >= _width) return;
  int mx = min(x + 8, _width);
  int sx = max(x, 0);
  int my = min(y, 0) + _height;    
  uint8_t *p = &font[(int)c * 8];
  for (int yy = max(y, 0); yy < my; yy++) {
    uint16_t val =  pgm_read_byte_near(p++);    
    for (int xx = sx; xx < mx; xx++) {
      drawPixelFast(xx, yy, val & (1 << (7 - (xx - x)) ) );          
    }
  }
}

size_t LEDMatrix8::write(uint8_t c)
{
  if (c == '\n') {
    cursor_x  = 0;
  } else if (c == '\r') {
  } else {
    drawChar(cursor_x, cursor_y, c);
    cursor_x += 8;
  }
  return 1;
}

void LEDMatrix8::setCursor(int x, int y) {
  cursor_x = x;
  cursor_y = y;
}

void LEDMatrix8::spiTransfer(int addr, byte opcode, byte data) {
  uint8_t spidata[16] = {0};

  spidata[addr * 2 + 1] = opcode;
  spidata[addr * 2] = data;

  digitalWrite(SPI_CS, LOW);

  for (unsigned i = maxDevices * 2; i > 0; i--)
    shiftOut(SPI_MOSI, SPI_CLK, MSBFIRST, spidata[i - 1]);

  digitalWrite(SPI_CS, HIGH);
}
