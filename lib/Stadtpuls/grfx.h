#ifndef GRFX_H
#define GRFX_H
#include <Arduino.h>
#include "images.h"
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"

class Grfx
{
public:
  Adafruit_SSD1306 *display;
  bool *screen_active;
  Grfx();
  void begin(Adafruit_SSD1306 *_display, bool *_screen_active);
  void splash(int timeout);
};
#endif // GRFX_H