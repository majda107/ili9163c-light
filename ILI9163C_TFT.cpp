#include "ILI9163C_TFT.h"




ILI9163C_TFT::ILI9163C_TFT(uint8_t cspin, uint8_t rspin, uint8_t dcpin)
{
  this->m_cspin = cspin;
  this->m_rspin = rspin;
  this->m_dcpin = dcpin;
}

void ILI9163C_TFT::start()
{
  pinMode(this->m_cspin, OUTPUT);
  pinMode(this->m_rspin, OUTPUT);

  
  #if defined ESP8266
  m_ILI9163CSPI = SPISettings(80000000, MSBFIRST, SPI_MODE0); //80MHz
  
  // begin SPI
  SPI.begin();
  SPI.setClockDivider(4);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  
  GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pin_reg(this->m_cspin));
  #else
  this->m_csport = portOutputRegister(digitalPinToPort(this->m_cspin));
  this->m_rsport = portOutputRegister(digitalPinToPort(this->m_rspin));
  
  this->m_csmask = digitalPinToBitMask(this->m_cspin);
  this->m_rsmask = digitalPinToBitMask(this->m_rspin);
  #endif
  
  
  
  this->m_en_data();

  // reset display power
  pinMode(this->m_dcpin, OUTPUT);
  digitalWrite(this->m_dcpin, HIGH);
  delay(500);
  digitalWrite(this->m_dcpin, LOW);
  delay(500);
  digitalWrite(this->m_dcpin, HIGH);
  delay(500);

  this->m_chip();
}


void ILI9163C_TFT::m_chip()
{
  this->m_trans();

  this->m_com(CMD_SWRESET);
  delay(500);

  this->m_com(CMD_SLPOUT); //exit sleep
  delay(5);

  this->m_com(CMD_PIXFMT); //Set Color Format 16bit
  this->m_data(0x05);
  delay(5);


  this->m_com(CMD_FRMCTR1); //Frame Rate Control (In normal mode/Full colors)
  this->m_data(0x08);       //0x0C//0x08
  this->m_data(0x02);       //0x14//0x08
  delay(1);

  this->m_com(CMD_DINVCTR); //display inversion
  this->m_data(0x07);
  delay(1);

  this->m_com(CMD_PWCTR1); //Set VRH1[4:0] & VC[2:0] for VCI1 & GVDD
  this->m_data(0x0A);      //4.30 - 0x0A
  this->m_data(0x02);      //0x05
  delay(1);

  this->m_com(CMD_PWCTR2); //Set BT[2:0] for AVDD & VCL & VGH & VGL
  this->m_data(0x02);
  delay(1);

  this->m_com(CMD_VCOMCTR1); //Set VMH[6:0] & VML[6:0] for VOMH & VCOML
  this->m_data(0x50);      //0x50
  this->m_data(99);        //0x5b
  delay(1);

  this->m_com(CMD_VCOMOFFS);
  this->m_data(0); //0x40
  delay(1);

  this->m_com(CMD_CLMADRS); //Set Column Address
  this->m_data16(0x00);
  this->m_data16(128);

  this->m_com(CMD_PGEADRS); //Set Page Address
  this->m_data16(0X00);
  this->m_data16(180);


  this->m_com(0x34); //tearing

  // set color mode
  auto Madctl = 0b00001000;
  bitSet(Madctl,3);
  bitSet(Madctl,4);
  this->m_com(CMD_MADCTL);
  this->m_data(Madctl);
  

  
  

  this->m_com(CMD_DISPON); //display ON
  delay(1);
  this->m_com(CMD_RAMWR); //Memory Write

  delay(1);
  
  this->m_end_trans();

  // clear memory
  this->fill_screen(0x00);
}




void ILI9163C_TFT::fill_screen(uint16_t color) 
{
  this->m_trans();

  this->set_address(0, 0, this->WIDTH, this->HEIGHT);

  this->m_en_data();
  
  for(uint16_t p = 0; p < this->WIDTH * this->HEIGHT; p++)
    this->m_data16(color);

  this->m_dis_cs();
  
  this->m_end_trans();
}


void ILI9163C_TFT::set_pixel(uint16_t x, uint16_t y, uint16_t color)
{
  if(x < 0 || x > this->WIDTH) return;
  if(y < 0 || y > this->HEIGHT) return;
  
  this->m_trans();
  this->set_address(x, y, x+1, y+1);
  
  this->m_en_data();
  this->m_data16(color);
  this->m_dis_cs();
  
  this->m_end_trans();
}

void ILI9163C_TFT::draw_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  if (y0 > y1) {
    short tmp_x = x0;
    short tmp_y = y0;

    x0 = x1;
    y0 = y1;

    x1 = tmp_x;
    y1 = tmp_y;
  }
  if (y1 > y2) {
    short tmp_x = x1;
    short tmp_y = y1;

    x1 = x2;
    y1 = y2;

    x2 = tmp_x;
    y2 = tmp_y;
  }
  if (y0 > y1) {
    short tmp_x = x0;
    short tmp_y = y0;

    x0 = x1;
    y0 = y1;

    x1 = tmp_x;
    y1 = tmp_y;
  }

  /*
  float dx1 = (y1 - y0) >= 0 ? (x1 - x0) / (float)(y1 - y0) : 0;
  float dx2 = (y2 - y0) >= 0 ? (x2 - x0) / (float)(y2 - y0) : 0;
  float dx3 = (y2 - y1) >= 0 ? (x2 - x1) / (float)(y2 - y1) : 0; 

  float sx = x0, ex = x0;
  float sy = y0, ey = y0;
  
  if(dx1 > dx2) 
  {
    for(;sy <= y1; sy++, ey++, sx += dx2, ex += dx1)
    {
      this->fast_hline(sx, ex, sy, color);
    }

    ex = x1;
    ey = y1;
    
    for(;sy <= y2; sy++, ey++, sx += dx2, ex += dx3)
    {
      this->fast_hline(sx, ex, sy, color);
    }
      
  } else 
  {
    for(;sy <= y1; sy++, ey++, sx += dx1, ex += dx2)
    {
      this->fast_hline(sx, ex, sy, color);
    }

    sx = x1;
    sy = y1;
    
    for(;sy <= y2; sy++, ey++, sx += dx3, ex += dx2)
    {
      this->fast_hline(sx, ex, sy, color);
    }
  }
  */
  int16_t a, b, y, last;

  if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if (x1 < a)
      a = x1;
    else if (x1 > b)
      b = x1;
    if (x2 < a)
      a = x2;
    else if (x2 > b)
      b = x2;
    this->fast_hline(a, b + 1, y0, color);
    return;
  }

  int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
          dx12 = x2 - x1, dy12 = y2 - y1;
  int32_t sa = 0, sb = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if (y1 == y2)
    last = y1; // Include y1 scanline
  else
    last = y1 - 1; // Skip it

  for (y = y0; y <= last; y++) {
    a = x0 + sa / dy01;
    b = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if (a > b)
    {
      int16_t tmp_a = a;
      a = b;
      b = tmp_a;
    }
      
    this->fast_hline(a, b + 1, y, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = (int32_t)dx12 * (y - y1);
  sb = (int32_t)dx02 * (y - y0);
  for (; y <= y2; y++) {
    a = x1 + sa / dy12;
    b = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if (a > b)
    {
      int16_t tmp_a = a;
      a = b;
      b = tmp_a;
    }
      
    this->fast_hline(a, b + 1, y, color);
  }
}


void ILI9163C_TFT::draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  short dx = abs(x1 - x0);
  short sx = x0 < x1 ? 1 : -1;

  short dy = -abs(y1 - y0);
  short sy = y0 < y1 ? 1 : -1;

  short err = dx + dy;
  short e2;

  while (true)
  {
    this->set_pixel(x0, y0, color );
    if (x0 == x1 && y0 == y1)
      break;

    e2 = 2 * err;
    if (e2 >= dy)
    {
      err += dy;
      x0 += sx;
    }

    if (e2 <= dx)
    {
      err += dx;
      y0 += sy;
    }
  }
}

void ILI9163C_TFT::fast_hline(int16_t x0, int16_t x1, int16_t y, uint16_t color)
{
  if(x0 < 0) x0 = 0;
  if(x1 > this->WIDTH) x1 = this->WIDTH;
  
  if(y < 0 || y > this->HEIGHT) return;
  
  this->m_trans();
  this->set_address(x0, y, x1+1, y+1);
  
  this->m_en_data();
  for(; x0 < x1; x0++)
    this->m_data16(color);
  
  this->m_dis_cs();
  
  this->m_end_trans();
}


void ILI9163C_TFT::fast_vline(int16_t y0, int16_t y1, int16_t x, uint16_t color)
{
  this->m_trans();
  this->set_address(x, y0, x+1, y1+1);
  
  this->m_en_data();
  for(; y0 < y1; y0++)
    this->m_data16(color);
  
  this->m_dis_cs();
  
  this->m_end_trans();
}



void ILI9163C_TFT::set_address(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  this->m_com(CMD_CLMADRS);
  this->m_data16(x0 + this->OFFSET_X);
  this->m_data16(x1 + this->OFFSET_X);

  this->m_com(CMD_PGEADRS);
  this->m_data16(y0 + this->OFFSET_Y);
  this->m_data16(y1 + this->OFFSET_Y);

  this->m_com(CMD_RAMWR);
}



#if defined(ESP8266)
void ILI9163C_TFT::m_trans() 
{
	SPI.beginTransaction(m_ILI9163CSPI);
	GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pin_reg(this->m_cspin));
}

void ILI9163C_TFT::m_en_data() 
{
	GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pin_reg(this->m_rspin));
}

void ILI9163C_TFT::m_en_com() 
{
	GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pin_reg(this->m_rspin));
}

void ILI9163C_TFT::m_dis_cs() 
{
	GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pin_reg(this->m_cspin));
}

void ILI9163C_TFT::m_end_trans() 
{
	SPI.endTransaction();
}
#else
void ILI9163C_TFT::m_trans() 
{
  *(this->m_csport) &= ~(this->m_csmask);
}

void ILI9163C_TFT::m_en_data() 
{
  *(this->m_rsport) |= this->m_rsmask;
}

void ILI9163C_TFT::m_en_com() 
{
  *(this->m_rsport) &= ~(this->m_rsmask);
}

void ILI9163C_TFT::m_dis_cs() 
{
  *(this->m_csport) |= this->m_csmask;
}

void ILI9163C_TFT::m_end_trans() 
{
	// nothing
}
#endif




#if defined(ESP8266)
void ILI9163C_TFT::m_spi(uint16_t c)
{
  SPI.write(c);
}
#else
void ILI9163C_TFT::m_spi(uint16_t c)
{
  SPDR = c;
  while (!(SPSR & _BV(SPIF)));
}
#endif




void ILI9163C_TFT::m_com(uint16_t c) 
{
  this->m_en_com();
  this->m_spi(c);
}

void ILI9163C_TFT::m_data(uint8_t d) 
{
  this->m_en_data();
  this->m_spi(d);
}



#if defined(ESP8266)
void ILI9163C_TFT::m_data16(uint16_t d) 
{
  this->m_en_data();
  SPI.write16(d);
}
#else
void ILI9163C_TFT::m_data16(uint16_t d) 
{
  this->m_en_data();
  this->m_spi(d >> 8);
  this->m_spi(d);
}
#endif
