# NAME
TempPressureNTCWiFi firmware
# Prerequisites
- [Arduino IDE 1.8.19](https://www.arduino.cc/en/software)
- [ESP32 in Arduino IDE](https://www.instructables.com/Installing-the-ESP32-Board-in-Arduino-IDE-Windows-/)
- [OTA Drive](https://otadrive.com/)

## Feature
- Read 4 channels external thermistor 100k B25/50 = 3950.
- Read 2 channels 4-20mA.
- Transfer data through ESPNOW or UART1(High-Priority).
- MAC Address destination setable for ESP NOW.

## Usage
1. Set MAC Address Destination
Send ESPNOW package data following frame:
```bash
MACS:<MAC Destination>
	with <MAC Destination>: 6 bytes of destination mac address

example: Destination Mac Address 12:34:56:78:90:AB
	Frame need to send:
		MACS:[0x12][0x34][0x56][0x78][0x90][0xAB]
```

2. ESPNOW package respone
```bash
	04NTCPS: <NTC1> <NTC2> <NTC3> <NTC4> <I1> <I2> <CRC>
		04NTCPS: Header
		<NTC1-4>: 8bytes float value NTC1 temperature in *C, ASCII format
		<I1-2>: 8bytes float value curent of channel 4-20ma, ASCII format
		<CRC>: 5Bytes CRC 16 Modbus value 00000-65535, ASCII format
Example: 
	04NTCPS: 0023.456 0065.432 0-45.678 0567.890 04.00000 20.00000 34567
		NTC1 = 23.456*C
		NTC2 = 65.432*C
		NTC3 = -45.678*C
		NTC4 = 567.89*C
		I1 = 4.0 mA
		I2 = 20.0 mA
		CRC = 34567 
```
3. UART Transceiver
Package need to send (ASCII mode)
```bash
	> R <Register> 00000000<CR><LF>
```
Data respone
```bash
	> R <Register> <Value> <CRC><CR><LF>
		with <Value> : 8bytes data
		<CRC>: 5Bytes CRC 16 CCIT value 00000-65535, ASCII format
```
Registers table:
|          Registers            |                                 Description                                 |
|:-----------------------------:|:---------------------------------------------------------------------------:|
| T1           		            |  Temperature channel 1 for old version  (NTC1)                              |
| T2       				        |  Temperature channel 2 for old version  (NTC2)                              |
| T3       	                    |  Temperature channel 3 for old version  (TC_K 4-20mA)                       |
| T4            		        |  Temperature channel 4                                                      |
| I1                            |  Currrent 4-20mA channel 1                                                  |
| I2                            |  Currrent 4-20mA channel 2                                                  |
| N1                            |  NTC Temperature channel 1                                                  |
| N2                            |  NTC Temperature channel 2                                                  |
| N3                            |  NTC Temperature channel 3                                                  |
| N4                            |  NTC Temperature channel 4                                                  |

Examples:
```bash
	Ask CMD:
		> R T1 00000000<CR><LF>
		> R T2 00000000<CR><LF>
		> R T3 00000000<CR><LF>
		> R T4 00000000<CR><LF>
		> R I1 00000000<CR><LF>
		> R I2 00000000<CR><LF>
		> R N1 00000000<CR><LF>
		> R N2 00000000<CR><LF>
		> R N3 00000000<CR><LF>
		> R N4 00000000<CR><LF>
	Respone CMD:
		> R T1 0123.456 12345<CR><LF>
		> R T2 -234.567 01234<CR><LF>
		> R T3 0999.999	65535<CR><LF>
		> R T4 0253.452 02345<CR><LF>
		> R I1 04.00000 45678<CR><LF>
		> R I2 20.00000 65530<CR><LF>
		> R N1 00000000	00001<CR><LF>
		> R N2 00000000 21453<CR><LF>
		> R N3 00000000 14523<CR><LF>
		> R N4 00000000 12563<CR><LF>
```
## Authors and acknowledgment


## License
Copyright (C) 2017 iFactory Joint Stock Company

