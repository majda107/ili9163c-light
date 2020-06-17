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

  this->m_csport = portOutputRegister(digitalPinToPort(this->m_cspin));
  this->m_rsport = portOutputRegister(digitalPinToPort(this->m_rspin));

  this->m_csmask = digitalPinToBitMask(this->m_cspin);
  this->m_rsmask = digitalPinToBitMask(this->m_rspin);

  // begin SPI
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);

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

  // set color mode
  auto Mactrl = 0b00001000;
  bitSet(Mactrl,3);
  this->m_com(CMD_MADCTL);
  this->m_data(Mactrl);
  

  this->m_com(CMD_DISPON); //display ON
  delay(1);
  this->m_com(CMD_RAMWR); //Memory Write

  delay(1);

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
}


void ILI9163C_TFT::set_pixel(uint16_t x, uint16_t y, uint16_t color)
{
  this->m_trans();
  this->set_address(x, y, x+1, y+1);
  
  this->m_en_data();
  this->m_data16(color);
  this->m_dis_cs();
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





void ILI9163C_TFT::m_spi(uint16_t c)
{
  SPDR = c;
  while (!(SPSR & _BV(SPIF)));
}

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

void ILI9163C_TFT::m_data16(uint16_t d) 
{
  this->m_en_data();
  this->m_spi(d >> 8);
  this->m_spi(d);
}
