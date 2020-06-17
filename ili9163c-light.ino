#include "ILI9163C_TFT.h"

#define __CS  10
#define __RS  8
#define __DC  9

ILI9163C_TFT tft = ILI9163C_TFT(__CS, __RS, __DC);

void setup() {
  tft.start();
}

int mov = 100;

void loop() {

  tft.fill_screen(RED);
  //tft.fill_screen(BLUE);

  //for(int x = 0; x < 20; x++)
  //  for(int y = 0; y < 20; y++)
  //    tft.set_pixel(x + 80, y + 20, WHITE);

  mov += 2;
  if(mov > 128)
    mov = 100;

  tft.draw_line(0, 0, 70, mov, WHITE);
  tft.draw_line(0, 0, mov, 70, WHITE);

  delay(30);
}
