
#ifndef LEDMatrix8_H_
#define LEDMatrix8_H_

#ifdef __cplusplus
#include "Arduino.h"
#endif

#include "font8x8_ib8x8u_full.h"
#include "font8x8_ic8x8u.h"
#include "font8x8_icl8x8u.h"

class LEDMatrix8 : public Print
{
  public:
    LEDMatrix8(uint8_t dataPin, uint8_t clkPin, uint8_t csPin, uint8_t numDevices = 8);

    void shutdown(bool b);
    void setIntensity(int intensity);
    void setCursor(int x, int y);
    void clearDisplay();
    void invertDisplay();
    inline void drawPixelFast(int x, int y, int c) __attribute__((always_inline));
    inline void drawPixel(int x, int y, int c);
    inline void drawFastHLine(int x, int y, int w);
    inline void drawFastVLine(int x, int y, int h);
    void drawLine(int x0, int y0, int x1, int y1);
    void drawRect(int x, int y, int w, int h);
    void fillRect(int x, int y, int w, int h);
    void drawChar(int x, int y, unsigned char c);
    virtual size_t write(uint8_t);
    void update(void);

    int width(void) {return _width;}
    int height(void) {return _height;}
    int getCursorX(void) {return cursor_x;}
    int getCursorY(void) {return cursor_y;}

  protected:
    uint8_t SPI_MOSI, SPI_CLK, SPI_CS;
    uint8_t status[8][8];
    unsigned maxDevices;
    uint8_t *font;
    static constexpr int _height = 8;
    int _width;
    unsigned cursor_x = 0, cursor_y = 0;
    void spiTransfer(int addr, uint8_t opcode, uint8_t data);
};

#endif