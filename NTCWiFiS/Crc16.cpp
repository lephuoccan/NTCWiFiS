/*
 * Crc16.c
 *
 *  Created on: 2 thg 8, 2021
 *      Author: iFactory
 */
#include "Crc16.h"

uint16_t ifacCRC::crc16_str(char *ptr, uint8_t len)
{
	uint16_t crc =CRC16_START;
	while(len--)
	{
		crc = crc16_byte(crc , *ptr++);
	}
	return crc;
}
uint16_t ifacCRC::crc16_byte(uint16_t crcValue, uint8_t newByte)
{
	unsigned char i;

	for (i = 0; i < 8; i++) {

		if (((crcValue & 0x8000) >> 8) ^ (newByte & 0x80)){
			crcValue = (crcValue << 1)  ^ POLYNOM;
		}else{
			crcValue = (crcValue << 1);
		}

		newByte <<= 1;
	}

	return crcValue;
}
/**
  * @brief  Calculate CRC16 Modbus.
  * @note   Polynomial calculations 0x8005.
  * @param  uc_pBuffer pointer buffer to check CRC.
  * @retval ul_Length Length of string is needs checking.
  */
uint16_t ifacCRC::CRC16_Modbus(uint8_t *uc_pBuffer, uint16_t ul_Length)
{
    uint8_t Luc_Index_Buff;
    uint8_t Luc_Index_Bit;
    uint16_t Lus_Tmp_Bit;
    uint8_t Luc_Tmp_Val;
    uint16_t Lul_CRC_Val;
    /* Initialize value for Crc16 Modbus */
    Lul_CRC_Val = 0xFFFF;

    for (Luc_Index_Buff = 0; Luc_Index_Buff < ul_Length; Luc_Index_Buff++)
    {
        Luc_Tmp_Val = (uint8_t)*uc_pBuffer++;
        Lul_CRC_Val ^= Luc_Tmp_Val;
        for (Luc_Index_Bit = 0; Luc_Index_Bit < 8; Luc_Index_Bit++ )
        {
            Lus_Tmp_Bit = Lul_CRC_Val & 0x0001;
            Lul_CRC_Val >>= 1;
            if ( Lus_Tmp_Bit != 0 )
            {
                Lul_CRC_Val ^= 0xA001;
            }
        }
    }
    return Lul_CRC_Val;
}
