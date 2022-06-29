#include "grfx.h"
Grfx::Grfx()
{
}
void Grfx::begin(Adafruit_SSD1306 *_display, bool *_screen_active)
{
  display = _display;
  screen_active = _screen_active;
}

void Grfx::splash(int timeout)
{
  if (*screen_active == false)
  {
    return;
  }
  display->clearDisplay();                                   // Clear display buffer
  display->drawBitmap(0, 0, stadtpuls_logo, 128, 64, WHITE); // Draw Bitmap
  display->display();                                        // Display buffer
  delay(timeout);
}