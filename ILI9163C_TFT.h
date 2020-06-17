#include "Arduino.h"
#include "ILI9163C_Commands.h"
#include "ILI9163C_Colors.h"
#include <SPI.h>

class ILI9163C_TFT {
private:
  uint8_t m_cspin;
  uint8_t m_rspin;
  uint8_t m_dcpin;

  volatile uint8_t* m_csport;
  volatile uint8_t* m_rsport;

  uint8_t m_csmask;
  uint8_t m_rsmask;

  // data flow
  void m_trans();
  void m_en_data();
  void m_en_com();
  void m_dis_cs();

  // communication
  void m_spi(uint16_t c);
  void m_com(uint16_t c);
  void m_data(uint8_t d);
  void m_data16(uint16_t d);


  void m_chip(void);
  
public:
  const uint8_t WIDTH = 128;
  const uint8_t HEIGHT = 128;

  const uint8_t OFFSET_Y = 32;
  const uint8_t OFFSET_X = 0;
  
  ILI9163C_TFT(uint8_t cspin, uint8_t rspin, uint8_t dcpin);

  void start();
  
  void set_address(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

  void fill_screen(uint16_t color);
  void set_pixel(uint16_t x, uint16_t y, uint16_t color);
  void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
};
