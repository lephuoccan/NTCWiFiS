/*
 * Crc16.h
 *
 *  Created on: 2 thg 8, 2021
 *      Author: iFactory
 */

#ifndef INC_CRC16_H_
#define INC_CRC16_H_
#include <Arduino.h>

#define CRC16_START			0xFFFF
#define CRC16_DNP			0x3D65		// DNP, IEC 870, M-BUS, wM-BUS, ...
#define CRC16_CCITT			0x1021		// X.25, V.41, HDLC FCS, Bluetooth, ...


#define CRC16_IBM			0x8005		// ModBus, USB, Bisync, CRC-16, CRC-16-ANSI, ...
#define CRC16_T10_DIF		0x8BB7		// SCSI DIF
#define CRC16_DECT			0x0589		// Cordeless Telephones
#define CRC16_ARINC			0xA02B		// ACARS Aplications

#define POLYNOM					CRC16_CCITT   // Define the used polynom from one of the aboves

class ifacCRC
{
  public:
    uint16_t CRC16_Modbus(uint8_t *uc_pBuffer, uint16_t ul_Length);
    
  private:
    uint16_t crc16_str(char *ptr, uint8_t len);
    uint16_t crc16_byte(uint16_t crcValue, uint8_t newByte);
};


#endif /* INC_CRC16_H_ */
