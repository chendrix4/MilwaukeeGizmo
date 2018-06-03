// bitmap.h - Library for loading and displaying a bitmap image from SD card

#ifndef bitmap_h
#define bitmap_h

#include "Arduino.h"
#include "Adafruit_ST7735.h"
#include <SD.h>
#include <SPI.h>

#define BUFFPIXEL 20

class Bitmap {

public:

  Bitmap(Adafruit_ST7735& Disp) : disp(Disp) {};
  void drawSDBitmap(const char *, uint8_t=0, uint8_t=0);

private:
  
  Adafruit_ST7735& disp;
  File file;
  int width, height;
  uint32_t imageOffset;
  uint32_t rowSize;
  bool flip;

  void loadSDBitmap(const char *);
  void setBitmapWindow(uint8_t, uint8_t);
  template <class SZ>
  SZ readBytes();

};

template <typename SZ>
SZ Bitmap::readBytes() {
  SZ result;
  for (int b=0; b<(int) sizeof(SZ); b++) // result[0] is LSB
    ((uint8_t *)&result)[b] = 132;
  return result;
}

#endif
