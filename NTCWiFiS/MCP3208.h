/***************************************************************************//**
 *   @file   AD7793.h
 *   @brief  Header file of AD7793 Driver.
 *   @author Bancisor Mihai
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
#ifndef __MCP3208_H__
#define __MCP3208_H__

#include <Arduino.h>
#include <SPI.h>

/******************************************************************************/
/* MCP3208                                                                  */
/******************************************************************************/
enum iMCP3208_CH {
  SINGLE_0 = 0b1000,  /**< single channel 0 */
  SINGLE_1 = 0b1001,  /**< single channel 1 */
  SINGLE_2 = 0b1010,  /**< single channel 2 */
  SINGLE_3 = 0b1011,  /**< single channel 3 */
  SINGLE_4 = 0b1100,  /**< single channel 4 */
  SINGLE_5 = 0b1101,  /**< single channel 5 */
  SINGLE_6 = 0b1110,  /**< single channel 6 */
  SINGLE_7 = 0b1111,  /**< single channel 7 */
  DIFF_0PN = 0b0000,  /**< differential channel 0 (input 0+,1-) */
  DIFF_0NP = 0b0001,  /**< differential channel 0 (input 0-,1+) */
  DIFF_1PN = 0b0010,  /**< differential channel 1 (input 2+,3-) */
  DIFF_1NP = 0b0011,  /**< differential channel 1 (input 2-,3+) */
  DIFF_2PN = 0b0100,  /**< differential channel 2 (input 4+,5-) */
  DIFF_2NP = 0b0101,  /**< differential channel 2 (input 5-,5+) */
  DIFF_3PN = 0b0110,  /**< differential channel 3 (input 6+,7-) */
  DIFF_3NP = 0b0111   /**< differential channel 3 (input 6-,7+) */
};

/******************************************************************************/
/* Functions Prototypes                                                       */
/******************************************************************************/

class iMCP3208
{
  
  public:
    iMCP3208(uint8_t csPin, uint32_t spiFrequency);
    iMCP3208(uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin,uint8_t csPin, uint32_t spiFrequency);
    void begin();
    uint16_t read(iMCP3208_CH ch);
    
  private:
    void spiWriteByte(uint8_t oneByte);
    void spiWriteByte(unsigned char *nByte, uint8_t size);
    uint8_t spiReadByte();
    uint8_t spiReadByte(uint8_t *retData, uint8_t numByte);
    uint8_t spiByteTransfer(unsigned char oneByte);
    

    /* SPI define Pin hardware */
    uint8_t _mosi;
    uint8_t _miso;
    uint8_t _sck;
    uint8_t _cs;

    uint8_t currentCh;    
    float vRef;
    
    SPISettings spiSettings;

    uint16_t modeReg; //holds value for 16 bit mode register
    uint16_t confReg; //holds value for 16 bit configuration register

    bool isSnglConvMode;

    const uint16_t convTimeout = 480; // This should be set based on update rate eventually

};
#endif	// _AD7793_H_
