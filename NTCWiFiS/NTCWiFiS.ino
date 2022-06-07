extern "C"{
  #include "circular_buffer.h"
}

#include "BoardCFG.h"
#include "MCP3208.h"
#include "Crc16.h"

#include <WiFi.h>
#include <driver/adc.h>
#include <esp_WiFi.h>
#include <esp_now.h>
#include <EEPROM.h>

#define NSS 26
#define FLASH_MEMORY_SIZE 512

/*ADC Variables*/
static const int spiClk = 1000000;   /*SPI clock 1 MHz*/
iMCP3208 ADCi(14,12,13,NSS,spiClk);
const float _Vref = 3300;            /*Vref mV*/
const float _ADC_MAX = 4095;         /* ADC 12bit */
const float _R1 = 10000.0;
const float _R25 = 100000.0;
const float _R50 = 35899.9;
const float _R100 = 6710.0;
const float _R150 = 1770.0;
const float _R300 = 105.6;

const float BETA25_50 = 3948.06;
const float BETA50_100 = 4044.69;
const float BETA100_150 = 4208.37;
const float BETA150_300 = 4504.0;

const float Comfilter = 0.999;

NTC_Typedef NTC1;
NTC_Typedef NTC2;
NTC_Typedef NTC3;
NTC_Typedef NTC4;

PS_Typedef PS1;
PS_Typedef PS2;

float TC_K_Temperature;
uint32_t smps,c_smps;
uint32_t micro;
/********************************************************************************/
/*CRC Variables*/
ifacCRC CRC;
uint16_t CRC16_Val;
/********************************************************************************/
/*ESP NOW Variables*/
esp_now_peer_info_t peerInfo;
uint8_t Broadcast_MAC[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
uint8_t Master_MAC[]={0x24,0x62,0xAB,0xE4,0x06,0x20};
uint8_t MAC_Dest[]={0x24,0x00,0x00,0x00,0x00,0x00};

const uint8_t outcomData_maxlen=250;
uint8_t outcomData[outcomData_maxlen];
uint8_t incomData[250];
uint8_t ESPNOW_Data_Received;
uint8_t ESPNOW_Data_len;
uint8_t MACS_Received = 0;
/********************************************************************************/
/*UART Variables*/
CIRC_BUFF_INIT(Uart_Circ_Buff1, 2047);
uint8_t uart_rx1;
uint8_t Uart_Receive_Cmd_Buff1[255],Uart_Receive_Cmd_Buff_Index1;
uint8_t Uart_buff_cmd_status1;
uint8_t Uart_rx1,Uart_Start_Frame_Flag1;
uint8_t lucaEndByte1[2] = {'\r','\n'};
uint8_t lucCountEndByte1 = 0;
uint8_t Luc_Ret1;
/********************************************************************************/
void InitESPNow(void);
void ADC_InitValue(void);
float map_value(float x, float in_min, float in_max, float out_min, float out_max);

void ADC_Process(void * parameter)
{
  vTaskDelay(1000 / portTICK_PERIOD_MS); /*Delay 1000ms*/
  UART_Debug.printf("ADC_Process is running on CPU %d\n", xPortGetCoreID());
  for(;;)
  {
    NTC1.ADC_Raw = ADCi.read(SINGLE_7);
    NTC1.ADC_Value = Comfilter*NTC1.ADC_Value + (1-Comfilter)*NTC1.ADC_Raw;
    NTC1.Resistance =((float)(NTC1.ADC_Value)*_ADC_MAX*_R1)/(_ADC_MAX*(_ADC_MAX-(float)NTC1.ADC_Value));
    NTC1.Temperature_C = R_To_Temperature(NTC1.Resistance);

    NTC2.ADC_Raw = ADCi.read(SINGLE_6);
    NTC2.ADC_Value = Comfilter*NTC2.ADC_Value + (1-Comfilter)*NTC2.ADC_Raw;
    NTC2.Resistance =((float)(NTC2.ADC_Value)*_ADC_MAX*_R1)/(_ADC_MAX*(_ADC_MAX-(float)NTC2.ADC_Value));
    NTC2.Temperature_C = R_To_Temperature(NTC2.Resistance);

    NTC3.ADC_Raw = ADCi.read(SINGLE_5);
    NTC3.ADC_Value = Comfilter*NTC3.ADC_Value + (1-Comfilter)*NTC3.ADC_Raw;
    NTC3.Resistance =((float)(NTC3.ADC_Value)*_ADC_MAX*_R1)/(_ADC_MAX*(_ADC_MAX-(float)NTC3.ADC_Value));
    NTC3.Temperature_C = R_To_Temperature(NTC3.Resistance);

    NTC4.ADC_Raw = ADCi.read(SINGLE_4);
    NTC4.ADC_Value = Comfilter*NTC4.ADC_Value + (1-Comfilter)*NTC4.ADC_Raw;
    NTC4.Resistance =((float)(NTC4.ADC_Value)*_ADC_MAX*_R1)/(_ADC_MAX*(_ADC_MAX-(float)NTC4.ADC_Value));
    NTC4.Temperature_C = R_To_Temperature(NTC4.Resistance);

    PS1.ADC_Raw = ADCi.read(SINGLE_0);
    PS1.ADC_Value = Comfilter*PS1.ADC_Value + (1-Comfilter)*PS1.ADC_Raw;
    PS1.Voltage = PS1.ADC_Value*_Vref/_ADC_MAX;
    PS1.Current = PS1.Voltage / PS1.Resistance;
    
    PS2.ADC_Raw = ADCi.read(SINGLE_1);
    PS2.ADC_Value = Comfilter*PS2.ADC_Value + (1-Comfilter)*PS2.ADC_Raw;
    PS2.Voltage = PS2.ADC_Value*_Vref/_ADC_MAX;
    PS2.Current = PS2.Voltage / PS2.Resistance;

    TC_K_Temperature = map_value(PS1.Current,4.0,20.0,0,800);
    c_smps++;
    if(millis() - micro >= 1000)
    {
      micro = millis();
      smps = c_smps;
      c_smps = 0;
    }
    
//    vTaskDelay(1 / portTICK_PERIOD_MS); /*Delay 1000ms*/
  }
}
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
//  Serial.print("Last Packet Sent to: "); Serial.println(macStr);
//  Serial.print("Last Packet Send Status: "); Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
void data_receive(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
    memset(&incomData[0],0,250);
    memcpy(&incomData[0],incomingData,len);
    ESPNOW_Data_len = len;
    ESPNOW_Data_Received = 1;
    if(incomData[0]=='M' && incomData[1]=='A' && incomData[2]=='C'&& incomData[3]=='S'&& incomData[4]==':')
    {
      memcpy(MAC_Dest,&incomData[5],6);
      EEPROM.write(0,MAC_Dest[0]);
      EEPROM.write(1,MAC_Dest[1]);
      EEPROM.write(2,MAC_Dest[2]);
      EEPROM.write(3,MAC_Dest[3]);
      EEPROM.write(4,MAC_Dest[4]);
      EEPROM.write(5,MAC_Dest[5]);
      EEPROM.commit();
      MACS_Received = 1;
    }
}
void setup() {
  // put your setup code here, to run once:
  UART_Debug.begin(115200);
  UART1.begin(115200, SERIAL_8N1,33,32);
  delay(1000);
  UART_Debug.println("Initialize UART_debug: UART0 115200 8N1");
  UART_Debug.println(WiFi.macAddress());
  UART_Debug.printf("Setup is running on CPU %d\n", xPortGetCoreID());
  
  EEPROM.begin(FLASH_MEMORY_SIZE);
  MAC_Dest[0] = EEPROM.readByte(0);
  MAC_Dest[1] = EEPROM.readByte(1);
  MAC_Dest[2] = EEPROM.readByte(2);
  MAC_Dest[3] = EEPROM.readByte(3);
  MAC_Dest[4] = EEPROM.readByte(4);
  MAC_Dest[5] = EEPROM.readByte(5);
  
  UART_Debug.print("MACDest: ");
  UART_Debug.print(MAC_Dest[5],HEX);
  UART_Debug.print(":");
  UART_Debug.print(MAC_Dest[4],HEX);
  UART_Debug.print(":");
  UART_Debug.print(MAC_Dest[3],HEX);
  UART_Debug.print(":");
  UART_Debug.print(MAC_Dest[2],HEX);
  UART_Debug.print(":");
  UART_Debug.print(MAC_Dest[1],HEX);
  UART_Debug.print(":");
  UART_Debug.println(MAC_Dest[0],HEX);
  
  ADCi.begin();
  esp_wifi_set_ps(WIFI_PS_NONE);    /* No power save */
  PS1.Resistance = 150.0;
  PS2.Resistance = 150.0;
  
  InitESPNow();
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(data_receive);
  memcpy(peerInfo.peer_addr, MAC_Dest, 6);
  peerInfo.channel = 1;  
  peerInfo.encrypt = false;     
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    UART_Debug.println("Failed to add peer");
    return;
  }
  ADC_InitValue();
  xTaskCreatePinnedToCore(
    ADC_Process,
    "ADC_Process",  // Task name
    5000,             // Stack size (bytes)
    NULL,             // Parameter
    1,                // Task priority
    NULL,             // Task handle
    1    // CPU ID
  );
  xTaskCreatePinnedToCore(
    Serial_Process,
    "Serial_Process",  // Task name
    5000,             // Stack size (bytes)
    NULL,             // Parameter
    1,                // Task priority
    NULL,             // Task handle
    1    // CPU ID
  );
}

void loop() {
  // put your main code here, to run repeatedly:
  static uint32_t tick_espnow = 0;
  if(millis() - tick_espnow >= 1000)
  {
    memset(outcomData,0,outcomData_maxlen);
    sprintf((char*)outcomData,"04NTCPS: %08.2f %08.2f %08.2f %08.2f %08.4f %08.4f %08.2f ",NTC1.Temperature_C,NTC2.Temperature_C
                ,NTC3.Temperature_C,NTC4.Temperature_C,PS1.Current,PS2.Current,TC_K_Temperature);
    uint16_t len_tmp = strlen((char*)outcomData);      
    CRC16_Val = CRC.CRC16_Modbus(outcomData,len_tmp);
    sprintf((char*)&outcomData[len_tmp],"%05d\r\n",CRC16_Val);
//    UART_Debug.println((char*)outcomData);
    esp_err_t result = esp_now_send(MAC_Dest, (uint8_t *) &outcomData[0], strlen((char*)outcomData));
    if (result != ESP_OK)
    {
      UART_Debug.print("Send Status: ");
      if (result == ESP_ERR_ESPNOW_NOT_INIT) {
        // How did we get so far!!
        UART_Debug.println("ESPNOW not Init.");
      } else if (result == ESP_ERR_ESPNOW_ARG) {
        UART_Debug.println("Invalid Argument");
      } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
        UART_Debug.println("Internal Error");
      } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
        UART_Debug.println("ESP_ERR_ESPNOW_NO_MEM");
      } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
        UART_Debug.println("Peer not found.");
      } else {
        UART_Debug.println("Not sure what happened");
      }
    }
    tick_espnow = millis();
  }
//  UART_Debug.printf("ADC  = %d %d\n",NTC1.ADC_Raw,(int)NTC1.ADC_Value);
//  UART_Debug.printf("T = %f   %f   %f   %f    %f\n", NTC1.Temperature_C, NTC2.Temperature_C, NTC3.Temperature_C, NTC4.Temperature_C, TC_K_Temperature);
//  UART_Debug.printf("ADC  = %f %f   %f %f\n",PS1.Voltage,PS1.Current,PS2.Voltage,PS2.Current );
  vTaskDelay(100 / portTICK_PERIOD_MS); /*Delay 1000ms*/
}

float map_value(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
float R_To_Temperature(float Resistance)
{
  float Tb,Rb,Beta;
  float retVal;
  if(Resistance <= _R150) /*150 300*/
  {
    Tb = 150;
    Rb = _R150;
    Beta = BETA150_300;
  }
  else if(Resistance <= _R100) /*100 150*/
  {
    Tb = 100;
    Rb = _R100;
    Beta = BETA100_150;
  }
  else if(Resistance <= _R50) /*50 100*/
  {
    Tb = 50;
    Rb = _R50;
    Beta = BETA50_100;
  }
  else if(Resistance <= _R25) /*25 50*/
  {
    Tb = 25;
    Rb = _R25;
    Beta = BETA25_50;
  }
  else if(Resistance > _R25) /*25 50*/
  {
    Tb = 25;
    Rb = _R25;
    Beta = BETA25_50;
  }
  retVal = (Beta*(273.15 + Tb))/(Beta+((273.15+ Tb)*log(Resistance/Rb)))-273.15;
  return retVal;
}
void InitESPNow(void) 
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    UART_Debug.println("ESPNow Init Success");
  }
  else {
    UART_Debug.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
  esp_wifi_set_channel(1,WIFI_SECOND_CHAN_NONE);
}

void Process_Data_Serial1(void)
{
  if (strstr((char*)Uart_Receive_Cmd_Buff1,"R FF 00000000") != NULL)
  {
    
  }
}
void Serial_Process(void * parameter)
{
     // ARTISAN_WS_Serial.print("//Serial_Process running on core ");
     // ARTISAN_WS_Serial.println(xPortGetCoreID());
  for(;;)
  {
    /*****************Receive uart and push to circular buffer******************/
    if(UART1.available())
    {
      uart_rx1 = UART1.read();
      Circ_Buff_Push(&Uart_Circ_Buff1, uart_rx1);
    }
    /***************************************************************************/
    /*******************Process data form circular buffer***********************/
    Luc_Ret1 = 0;
    Uart_buff_cmd_status1 = Circ_Buff_Pop(&Uart_Circ_Buff1,&Uart_rx1);
    while(Uart_buff_cmd_status1 != CIRC_BUFF_EMPTY)
    {
      /* Check start byte */
      if(Uart_rx1 == '>')
      {
        memset(Uart_Receive_Cmd_Buff1, '\0', 255);
        Uart_Start_Frame_Flag1 = 1;
        Uart_Receive_Cmd_Buff_Index1 = 0;
      }
      else
      {
        /* No action */
      }
      /* Get valid byte cmd and check frame */
      if((Uart_Start_Frame_Flag1 == 1) && (Uart_rx1 != '>'))
      {
        /* Check end byte and flush */
        if(Uart_rx1 == lucaEndByte1[lucCountEndByte1])
        {
          lucCountEndByte1++;
        }
        /* Check 2 byte lien tiep */
        else if(lucCountEndByte1 == 1)
        {
          /* Reset counter end byte */
          lucCountEndByte1 = 0;
        }
        else
        {
  
        }
  
        /* Get valid byte cmd into buff */
        Uart_Receive_Cmd_Buff1[Uart_Receive_Cmd_Buff_Index1] = Uart_rx1;
        Uart_Receive_Cmd_Buff_Index1++;
  
        /* Check out of range of buffer */
        if(Uart_Receive_Cmd_Buff_Index1 >= 255)
        {
          Uart_Receive_Cmd_Buff_Index1 = 0;
          Uart_Start_Frame_Flag1 = 0;
          Luc_Ret1 = 0;
        }
  
        /* Check enough 2 byte end and stop frame data */
        if(lucCountEndByte1 == 2)
        {
          /* Clear 2 end byte of frame received data */
          memset(Uart_Receive_Cmd_Buff1 + Uart_Receive_Cmd_Buff_Index1 - lucCountEndByte1, '\0', lucCountEndByte1);
  
          Uart_Start_Frame_Flag1 = 0;
          Uart_Receive_Cmd_Buff_Index1 = 0;
          Luc_Ret1 = 1;
  
          /* Break out loop */
          Uart_buff_cmd_status1 = CIRC_BUFF_EMPTY;
  
          /* Reset counter end byte */
          lucCountEndByte1 = 0;
        }
        else
        {
          /* No action */
        }
      }
      else
      {
        /* No action */
      }
  
      /* Check empty condition break-out loop above */
      if(Uart_buff_cmd_status1 != CIRC_BUFF_EMPTY)
      {
        Uart_buff_cmd_status1 = Circ_Buff_Pop(&Uart_Circ_Buff1, &Uart_rx1);
      }
    }
    if(Luc_Ret1 == 1)
    {
      /*Received buffer cmd successfully*/
      Process_Data_Serial1();
    }
  }
}
void ADC_InitValue(void)
{
    NTC1.ADC_Raw = ADCi.read(SINGLE_7);
    NTC1.ADC_Value = NTC1.ADC_Raw;
    NTC1.Resistance =((float)(NTC1.ADC_Value)*_ADC_MAX*_R1)/(_ADC_MAX*(_ADC_MAX-(float)NTC1.ADC_Value));
    NTC1.Temperature_C = R_To_Temperature(NTC1.Resistance);

    NTC2.ADC_Raw = ADCi.read(SINGLE_6);
    NTC2.ADC_Value = NTC2.ADC_Raw;
    NTC2.Resistance =((float)(NTC2.ADC_Value)*_ADC_MAX*_R1)/(_ADC_MAX*(_ADC_MAX-(float)NTC2.ADC_Value));
    NTC2.Temperature_C = R_To_Temperature(NTC2.Resistance);

    NTC3.ADC_Raw = ADCi.read(SINGLE_5);
    NTC3.ADC_Value = NTC3.ADC_Raw;
    NTC3.Resistance =((float)(NTC3.ADC_Value)*_ADC_MAX*_R1)/(_ADC_MAX*(_ADC_MAX-(float)NTC3.ADC_Value));
    NTC3.Temperature_C = R_To_Temperature(NTC3.Resistance);

    NTC4.ADC_Raw = ADCi.read(SINGLE_4);
    NTC4.ADC_Value = NTC4.ADC_Raw;
    NTC4.Resistance =((float)(NTC4.ADC_Value)*_ADC_MAX*_R1)/(_ADC_MAX*(_ADC_MAX-(float)NTC4.ADC_Value));
    NTC4.Temperature_C = R_To_Temperature(NTC4.Resistance);

    PS1.ADC_Raw = ADCi.read(SINGLE_0);
    PS1.ADC_Value = PS1.ADC_Raw;
    PS1.Voltage = PS1.ADC_Value*_Vref/_ADC_MAX;
    PS1.Current = PS1.Voltage / PS1.Resistance;
    
    PS2.ADC_Raw = ADCi.read(SINGLE_1);
    PS2.ADC_Value = PS2.ADC_Raw;
    PS2.Voltage = PS2.ADC_Value*_Vref/_ADC_MAX;
    PS2.Current = PS2.Voltage / PS2.Resistance;

    TC_K_Temperature = map_value(PS1.Current,4.0,20.0,0,800);
}
