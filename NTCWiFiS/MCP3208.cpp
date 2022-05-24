/***************************************************************************//**
 *   @file   AD7793.c
 *   @brief  Implementation of AD7793 Driver.
 *   @author Bancisor MIhai
********************************************************************************
 * Copyright 2012(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
********************************************************************************
 *   SVN Revision: 500
*******************************************************************************/

/******************************************************************************/
/* Include Files                                                              */
/******************************************************************************/
#include "MCP3208.h"				

iMCP3208::iMCP3208(uint8_t csPin, uint32_t spiFrequency)
{
  _cs = csPin;
  //Should be MODE3
  spiSettings = SPISettings(spiFrequency,MSBFIRST,SPI_MODE3); 
}

iMCP3208::iMCP3208(uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin,uint8_t csPin, uint32_t spiFrequency)
{
  _cs = csPin;
  _sck = sckPin;
  _miso = misoPin;
  _mosi = mosiPin;
  //Should be MODE3
  spiSettings = SPISettings(spiFrequency,MSBFIRST,SPI_MODE3); 
}

void iMCP3208::begin()
{
  /* Initialize CS pin and set high level */
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs,HIGH);

  SPI.begin(_sck,_miso,_mosi,_cs);
  SPI.beginTransaction(spiSettings);
  /* Must waiting about 500us before accessing any of the on-chip registers. */
  delayMicroseconds(500);
}
uint16_t iMCP3208::read(iMCP3208_CH ch)
{
  uint16_t retVal;
  uint8_t pByte1 = 0, pByte2 = 0;
  uint8_t rByte1, rbyte2;
  pByte1 = (ch>>2)|0x04;
  pByte2 = ch<<6;
//  pByte1 = 0x07;
//  pByte2 = 0x80;
  digitalWrite(_cs, LOW);
  spiWriteByte(pByte1);
  rByte1 = spiByteTransfer(pByte2);
  rbyte2 = spiByteTransfer(0xFF);
  digitalWrite(_cs, HIGH);
  delayMicroseconds(1);
  retVal = ((uint16_t)(rByte1&0x0F)<<8)|(uint16_t)rbyte2;
  return retVal;
}
void iMCP3208::spiWriteByte(unsigned char oneByte)
{
  SPI.transfer(oneByte);
}

void iMCP3208::spiWriteByte(unsigned char *nByte, uint8_t size)
{
  SPI.transfer(nByte,size);
}

uint8_t iMCP3208::spiReadByte()
{
  uint8_t retByte;
  digitalWrite(_cs, LOW);
  // Send a dumy byte
  retByte = SPI.transfer(0xFF);
  digitalWrite(_cs, HIGH);

  return retByte;
}
uint8_t iMCP3208::spiByteTransfer(unsigned char oneByte)
{
  uint8_t retByte;
  // Send a dumy byte
  retByte = SPI.transfer(oneByte);
  return retByte;
}


uint8_t iMCP3208::spiReadByte(uint8_t *retData, uint8_t numByte)
{
  uint8_t retByte;
  digitalWrite(_cs, LOW);
  for(int i=0; i<numByte; i++)
  {
    // Send a dumy byte
    *(retData + i)= SPI.transfer(0xFF);
  }
  digitalWrite(_cs, HIGH);

  return retByte;
}
