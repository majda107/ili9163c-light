#include "ILI9163_TFT.h"

#define __CS  10
#define __RS  8
#define __DC  9

ILI9163_TFT tft = ILI9163_TFT(__CS, __RS, __DC);

void setup() {
  tft.start();
}

void loop() {

  tft.fill_screen(RED);
  //tft.fill_screen(BLUE);

  for(int x = 0; x < 20; x++)
    for(int y = 0; y < 20; y++)
      tft.set_pixel(x + 100, y + 100, WHITE);

  delay(14);
}
