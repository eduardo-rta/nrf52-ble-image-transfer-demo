/*
  ArduCAM.cpp - Arduino library support for CMOS Image Sensor
  Copyright (C)2011-2015 ArduCAM.com. All right reserved

  Basic functionality of this library are based on the demo-code provided by
  ArduCAM.com. You can find the latest version of the library at
  http://www.ArduCAM.com

  Now supported controllers:
    - OV7670
    - MT9D111
    - OV7675
    - OV2640
    - OV3640
    - OV5642
    - OV7660
    - OV7725
    - MT9M112
    - MT9V111
    - OV5640
    - MT9M001
    - MT9T112
    - MT9D112

  We will add support for many other sensors in next release.

  Supported MCU platform
    - Theoretically support all Arduino families
      - Arduino UNO R3      (Tested)
      - Arduino MEGA2560 R3   (Tested)
      - Arduino Leonardo R3   (Tested)
      - Arduino Nano      (Tested)
      - Arduino DUE       (Tested)
      - Arduino Yun       (Tested)
      - Raspberry Pi      (Tested)
      - ESP8266-12        (Tested)

  If you make any modifications or improvements to the code, I would appreciate
  that you share the code with me so that I might include it in the next release.
  I can be contacted through http://www.ArduCAM.com

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*------------------------------------
  Revision History:
  2012/09/20  V1.0.0  by Lee  first release
  2012/10/23  V1.0.1  by Lee  Resolved some timing issue for the Read/Write Register
  2012/11/29  V1.1.0  by Lee  Add support for MT9D111 sensor
  2012/12/13  V1.2.0  by Lee  Add support for OV7675 sensor
  2012/12/28  V1.3.0  by Lee  Add support for OV2640,OV3640,OV5642 sensors
  2013/02/26  V2.0.0  by Lee  New Rev.B shield hardware, add support for FIFO control
                              and support Mega1280/2560 boards
  2013/05/28  V2.1.0  by Lee  Add support all drawing functions derived from UTFT library
  2013/08/24  V3.0.0  by Lee  Support ArudCAM shield Rev.C hardware, features SPI interface and low power mode.
                Support almost all series of Arduino boards including DUE.
  2014/02/06  V3.0.1  by Lee  Minor change to the library, fixed some bugs, add self test code to the sketches for easy debugging.
  2014/03/09  V3.1.0  by Lee  Add the more impressive example sketches.
                Optimise the OV5642 settings, improve image quality.
                Add live preview before JPEG capture.
                Add play back photos one by one after BMP capture.
  2014/05/01  V3.1.1  by Lee  Minor changes to add support Arduino IDE for linux distributions.
  2014/09/30  V3.2.0  by Lee  Improvement on OV5642 camera dirver.
  2014/10/06  V3.3.0  by Lee  Add OV7660,OV7725 camera support.
  2015/02/27  V3.4.0  by Lee  Add the support for Arduino Yun board, update the latest UTFT library for ArduCAM.
  2015/06/09  V3.4.1  by Lee  Minor changes and add some comments
  2015/06/19  V3.4.2  by Lee  Add support for MT9M112 camera.
  2015/06/20  V3.4.3  by Lee  Add support for MT9V111 camera.
  2015/06/22  V3.4.4  by Lee  Add support for OV5640 camera.
  2015/06/22  V3.4.5  by Lee  Add support for MT9M001 camera.
  2015/08/05  V3.4.6  by Lee  Add support for MT9T112 camera.
  2015/08/08  V3.4.7  by Lee  Add support for MT9D112 camera.
  2015/09/20  V3.4.8  by Lee  Add support for ESP8266 processor.
  2016/02/03  V3.4.9  by Lee  Add support for Arduino ZERO board.
  2016/06/07  V3.5.0  by Lee  Add support for OV5642_CAM_BIT_ROTATION_FIXED.
  2016/06/14  V3.5.1  by Lee  Add support for ArduCAM-Mini-5MP-Plus OV5640_CAM.
  2016/09/29  V3.5.2  by Lee  Optimize the OV5642 register settings
	2016/10/05	V4.0.0	by Lee	Add support for second generation hardware platforms like ArduCAM shield V2, ArduCAM-Mini-5MP-Plus(OV5642/OV5640).	  
	2016/10/17	V4.0.1	by Lee	Add support for Arduino Genuino 101 board	
  --------------------------------------*/
//#include "Arduino.h"
#include "ArduCAM.h"
#include "memorysaver.h"
#include "nrf_delay.h"

//Assert CS signal
void ArduCAM::CS_LOW(void)
{
  mSpiMaster.chipSelect();
}

//Disable CS signal
void ArduCAM::CS_HIGH(void)
{
  mSpiMaster.chipDeselect();
}

uint8_t ArduCAM::spiWrite(uint8_t dataByte)
{
    return mSpiMaster.put(dataByte);
}

void ArduCAM::spiReadMulti(uint8_t *rxBuf, uint32_t length)
{
    mSpiMaster.receive(rxBuf, length);
}

//Set corresponding bit
void ArduCAM::set_bit(uint8_t addr, uint8_t bit)
{
  uint8_t temp;
  temp = read_reg(addr);
  write_reg(addr, temp | bit);
}

//Clear corresponding bit
void ArduCAM::clear_bit(uint8_t addr, uint8_t bit)
{
  uint8_t temp;
  temp = read_reg(addr);
  write_reg(addr, temp & (~bit));
}

//Get corresponding bit status
uint8_t ArduCAM::get_bit(uint8_t addr, uint8_t bit)
{
  uint8_t temp;
  temp = read_reg(addr);
  temp = temp & bit;
  return temp;
}

//Set ArduCAM working mode
//MCU2LCD_MODE: MCU writes the LCD screen GRAM
//CAM2LCD_MODE: Camera takes control of the LCD screen
//LCD2MCU_MODE: MCU read the LCD screen GRAM
void ArduCAM::set_mode(uint8_t mode)
{
  switch (mode)
  {
    case MCU2LCD_MODE:
      write_reg(ARDUCHIP_MODE, MCU2LCD_MODE);
      break;
    case CAM2LCD_MODE:
      write_reg(ARDUCHIP_MODE, CAM2LCD_MODE);
      break;
    case LCD2MCU_MODE:
      write_reg(ARDUCHIP_MODE, LCD2MCU_MODE);
      break;
    default:
      write_reg(ARDUCHIP_MODE, MCU2LCD_MODE);
      break;
  }
}

//Low level SPI write operation
int ArduCAM::bus_write(int address, int value) 
{
    static uint8_t txCommand[2];
    spiEnable(true);
    txCommand[0] = address;
    txCommand[1] = value;
    mSpiMaster.transmit(txCommand, 2);
    while(mSpiMaster.isBusy());
    return 1;
}

//Low level SPI read operation
uint8_t ArduCAM::bus_read(int address) 
{
    uint8_t txBuf[2] = {address, 0}, rxBuf[2];
    uint8_t value = 0;
    spiEnable(true);
    mSpiMaster.transfer(txBuf, rxBuf, 2);
    while(mSpiMaster.isBusy());
    value = rxBuf[1];
    return value;
}

//Write ArduChip internal registers
void ArduCAM::write_reg(uint8_t addr, uint8_t data)
{
  bus_write(addr | 0x80, data);
}

//Read ArduChip internal registers
uint8_t ArduCAM::read_reg(uint8_t addr)
{
  uint8_t data;
  data = bus_read(addr & 0x7F);
  return data;
}

ArduCAM::ArduCAM(uint8_t model, uint32_t scl, uint32_t sda, uint32_t csn, uint32_t mosi, uint32_t miso, uint32_t sck)
{ 
    pinScl = scl;
    pinSda = sda;
    pinCsn = csn;
    pinMosi = mosi;
    pinMiso = miso;
    pinSck = sck;    
    spiTwiInit();
    sensor_model = model;
    switch (sensor_model)
    {
        case OV2640:
            mTwiMaster.address = 0x60 >> 1;
            break;
        default:
            mTwiMaster.address = 0x42;
            break;
    }
}

void ArduCAM::spiTwiInit()
{
    mSpiMaster.pinCsn       = pinCsn;
    mSpiMaster.pinMosi      = pinMosi;
    mSpiMaster.pinMiso      = pinMiso;
    mSpiMaster.pinSck       = pinSck;
    mSpiMaster.frequency    = SPIM_FREQUENCY_FREQUENCY_M4;
    mSpiMaster.blocking     = false;
    mSpiMaster.open();    
    mSpiMaster.close();
    mTwiMaster.pinSda       = pinSda;
    mTwiMaster.pinScl       = pinScl;
    mTwiMaster.open();    
}

void ArduCAM::spiEnable(bool spiEnable)
{
    if(mSpiMaster.isOpen() == spiEnable) return;
    if(spiEnable)
    {
        mTwiMaster.close();
        mSpiMaster.open();
    }
    else
    {
        mSpiMaster.close();
        mTwiMaster.open();
    }
}

//Reset the FIFO pointer to ZERO
void ArduCAM::flush_fifo(void)
{
  write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
}

//Send capture command
void ArduCAM::start_capture(void)
{
  write_reg(ARDUCHIP_FIFO, FIFO_START_MASK);
}

//Clear FIFO Complete flag
void ArduCAM::clear_fifo_flag(void)
{
  write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
}

//Read FIFO single
uint8_t ArduCAM::read_fifo(void)
{
  uint8_t data;
  data = bus_read(SINGLE_FIFO_READ);
  return data;
}

//Read Write FIFO length
//Support ArduCAM Mini only
uint32_t ArduCAM::read_fifo_length(void)
{
  uint32_t len1, len2, len3, length = 0;
  len1 = read_reg(FIFO_SIZE1);
  len2 = read_reg(FIFO_SIZE2);
  len3 = read_reg(FIFO_SIZE3) & 0x7f;
  length = ((len3 << 16) | (len2 << 8) | len1) & 0x07fffff;
  return length;
}

//Send read fifo burst command
//Support ArduCAM Mini only
void ArduCAM::set_fifo_burst()
{
    mSpiMaster.put(BURST_FIFO_READ);
}

//I2C Write 8bit address, 8bit data
uint8_t ArduCAM::wrSensorReg8_8(int regID, int regDat)
{
    static uint8_t twiBuf[2];
    spiEnable(false);
    twiBuf[0] = regID & 0x00FF;
    twiBuf[1] = regDat & 0x00FF;
    mTwiMaster.tx(twiBuf, 2);
    mTwiMaster.completeOperation();
    nrf_delay_ms(1);
    return (1);
}

//I2C Read 8bit address, 8bit data
uint8_t ArduCAM::rdSensorReg8_8(uint8_t regID, uint8_t* regDat)
{
    spiEnable(false);
    mTwiMaster.txRx(&regID, 1, regDat, 1);
    mTwiMaster.completeOperation();
    nrf_delay_ms(1);
    return (1);
}

//I2C Write 8bit address, 16bit data
uint8_t ArduCAM::wrSensorReg8_16(int regID, int regDat)
{
    uint8_t txBuf[3] = {regID & 0x00FF, regDat >> 8, regDat & 0x00FF};
    spiEnable(false);
    mTwiMaster.tx(txBuf, 3);
    mTwiMaster.completeOperation();
    nrf_delay_ms(1);
    return (1);
}

//I2C Read 8bit address, 16bit data
uint8_t ArduCAM::rdSensorReg8_16(uint8_t regID, uint16_t* regDat)
{
    uint8_t rxBuf[2];
    spiEnable(false);
    mTwiMaster.txRx(&regID, 1, rxBuf, 2);
    mTwiMaster.completeOperation();
    *regDat = rxBuf[0] << 8 | rxBuf[1];
    nrf_delay_ms(1);
    return (1);
}

//I2C Write 16bit address, 8bit data
uint8_t ArduCAM::wrSensorReg16_8(int regID, int regDat)
{
    uint8_t txBuf[3] = {regID >> 8, regID & 0x00FF, regDat & 0x00FF};
    spiEnable(false);
    mTwiMaster.tx(txBuf, 3);
    mTwiMaster.completeOperation();   
    nrf_delay_ms(1);
    return (1);
}

//I2C Read 16bit address, 8bit data
uint8_t ArduCAM::rdSensorReg16_8(uint16_t regID, uint8_t* regDat)
{
    uint8_t txBuf[2] = {regID >> 8, regID & 0x00FF};
    spiEnable(false);
    mTwiMaster.txRx(txBuf, 2, regDat, 1);
    mTwiMaster.completeOperation();
    nrf_delay_ms(1);
    return (1);
}

//I2C Write 16bit address, 16bit data
uint8_t ArduCAM::wrSensorReg16_16(int regID, int regDat)
{
    uint8_t txBuf[4] = {regID >> 8, regID & 0x00FF, regDat >> 8, regDat & 0x00FF};
    spiEnable(false);
    mTwiMaster.tx(txBuf, 4);
    mTwiMaster.completeOperation();
    nrf_delay_ms(1);
    return (1);
}

//I2C Read 16bit address, 16bit data
uint8_t ArduCAM::rdSensorReg16_16(uint16_t regID, uint16_t* regDat)
{
    uint8_t txBuf[2] = {regID >> 8, regID & 0x00FF};
    uint8_t rxBuf[2];
    spiEnable(false);
    mTwiMaster.txRx(txBuf, 2, rxBuf, 2);
    mTwiMaster.completeOperation();
    *regDat = rxBuf[0] << 8 | rxBuf[1];
    nrf_delay_ms(1);
    return (1);
}

//I2C Array Write 8bit address, 8bit data
int ArduCAM::wrSensorRegs8_8(const struct sensor_reg reglist[])
{
  uint16_t reg_addr = 0;
  uint16_t reg_val = 0;
  const struct sensor_reg *next = reglist;
  while ((reg_addr != 0xff) | (reg_val != 0xff))
  {
    reg_addr = pgm_read_word(&next->reg);
    reg_val = pgm_read_word(&next->val);
    wrSensorReg8_8(reg_addr, reg_val);
    next++;
#if defined(ESP8266)
    yield();
#endif
  }

  return 1;
}

//I2C Array Write 8bit address, 16bit data
int ArduCAM::wrSensorRegs8_16(const struct sensor_reg reglist[])
{
  unsigned int reg_addr =0, reg_val = 0;
  const struct sensor_reg *next = reglist;

  while ((reg_addr != 0xff) | (reg_val != 0xffff))
  {
    reg_addr = pgm_read_word(&next->reg);
    reg_val = pgm_read_word(&next->val);
    wrSensorReg8_16(reg_addr, reg_val);
    //  if (!err)
    //return err;
    next++;
#if defined(ESP8266)
    yield();
#endif
  }

  return 1;
}

//I2C Array Write 16bit address, 8bit data
int ArduCAM::wrSensorRegs16_8(const struct sensor_reg reglist[])
{
  unsigned int reg_addr = 0;
  unsigned char reg_val = 0;
  const struct sensor_reg *next = reglist;

  while ((reg_addr != 0xffff) | (reg_val != 0xff))
  {
    reg_addr = pgm_read_word(&next->reg);
    reg_val = pgm_read_word(&next->val);
    wrSensorReg16_8(reg_addr, reg_val);
    //if (!err)
    //return err;
    next++;
#if defined(ESP8266)
    yield();
#endif
  }

  return 1;
}

//I2C Array Write 16bit address, 16bit data
int ArduCAM::wrSensorRegs16_16(const struct sensor_reg reglist[])
{

  unsigned int reg_addr, reg_val;
  const struct sensor_reg *next = reglist;
  reg_addr = pgm_read_word(&next->reg);
  reg_val = pgm_read_word(&next->val);
  while ((reg_addr != 0xffff) | (reg_val != 0xffff))
  {
    wrSensorReg16_16(reg_addr, reg_val);
    //if (!err)
    //   return err;
    next++;
    reg_addr = pgm_read_word(&next->reg);
    reg_val = pgm_read_word(&next->val);
#if defined(ESP8266)
    yield();
#endif
  }


  return 1;
}

void ArduCAM::OV2640_set_JPEG_size(uint8_t size)
{

  #if (defined (OV2640_CAM)||defined (OV2640_MINI_2MP))
  switch (size)
  {  
    case OV2640_160x120:
      wrSensorRegs8_8(OV2640_160x120_JPEG);
      break;
    case OV2640_176x144:
      wrSensorRegs8_8(OV2640_176x144_JPEG);
      break;
    case OV2640_320x240:
      wrSensorRegs8_8(OV2640_320x240_JPEG);
      break;
    case OV2640_352x288:
      wrSensorRegs8_8(OV2640_352x288_JPEG);
      break;
    case OV2640_640x480:
      wrSensorRegs8_8(OV2640_640x480_JPEG);
      break;
    case OV2640_800x600:
      wrSensorRegs8_8(OV2640_800x600_JPEG);
      break;
    case OV2640_1024x768:
      wrSensorRegs8_8(OV2640_1024x768_JPEG);
      break;
    case OV2640_1280x1024:
      wrSensorRegs8_8(OV2640_1280x1024_JPEG);
      break;
    case OV2640_1600x1200:
      wrSensorRegs8_8(OV2640_1600x1200_JPEG);
      break;
    default:
      wrSensorRegs8_8(OV2640_320x240_JPEG);
      break;
  }
#endif
}

void ArduCAM::set_format(uint8_t fmt)
{
  if (fmt == BMP)
    m_fmt = BMP;
  else
    m_fmt = JPEG;
}

void ArduCAM::InitCAM()
{
#if !(defined(__CPU_ARC__))
 #endif
  switch (sensor_model)
  {
    case OV2640:
      {
#if (defined(OV2640_CAM) || defined(OV2640_MINI_2MP))
        wrSensorReg8_8(0xff, 0x01);
        wrSensorReg8_8(0x12, 0x80);
        nrf_delay_ms(100);
        if (m_fmt == JPEG)
        {
          wrSensorRegs8_8(OV2640_JPEG_INIT);
          wrSensorRegs8_8(OV2640_YUV422);
          wrSensorRegs8_8(OV2640_JPEG);
          wrSensorReg8_8(0xff, 0x01);
          wrSensorReg8_8(0x15, 0x00);
          wrSensorRegs8_8(OV2640_320x240_JPEG);
          //wrSensorReg8_8(0xff, 0x00);
          //wrSensorReg8_8(0x44, 0x32);
        }
        else
        {
          wrSensorRegs8_8(OV2640_QVGA);
        }
#endif
        break;
      }
    default:

      break;
  }
}
