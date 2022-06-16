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
#include <otadrive_esp.h>
#include "ifacSetting.h"
#define NSS 26
#define FLASH_MEMORY_SIZE 512

/*ADC Variables*/
static const int spiClk = 1000000;   /*SPI clock 1 MHz*/
iMCP3208 ADCi(14,12,13,NSS,spiClk);
const float _Vref = 3300;            /*Vref mV*/
const float _ADC_MAX = 4095;         /* ADC 12bit */
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
/*OTA Variable*/
uint8_t MAIN_FW_Update_Flag = 0; /*ESP32 FW update Flag, Received from HMI command, Saved and Load from NVS Flash, self clear when update successfully*/
/*CRC Variables*/
ifacCRC CRC;
uint16_t CRC16_Val;
uint16_t CRC16_Val_CCIT;
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
char outcomData_uart1[100];
uint8_t Uart1_Transceiving = 0;
uint32_t Uart1_Transceiving_tick = 0;
/********************************************************************************/
void InitESPNow(void);
void ADC_InitValue(void);
float map_value(float x, float in_min, float in_max, float out_min, float out_max);
void OTADRIVE_UPDATE(String Version,int OTA_WIFI_timeout);
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
void onUpdateProgress(int progress, int totalt);
void ADC_Process(void * parameter)
{
  vTaskDelay(1000 / portTICK_PERIOD_MS); /*Delay 1000ms*/
  UART_Debug.printf("ADC_Process is running on CPU %d\n", xPortGetCoreID());
  for(;;)
  {
    NTC1.ADC_Raw = ADCi.read(SINGLE_7);
    NTC1.ADC_Value = Comfilter*NTC1.ADC_Value + (1-Comfilter)*NTC1.ADC_Raw;
    NTC1.Resistance =((float)(NTC1.ADC_Value)*_ADC_MAX*NTC1.RS)/(_ADC_MAX*(_ADC_MAX-(float)NTC1.ADC_Value));
    NTC1.Temperature_C = R_To_Temperature(NTC1.Resistance, NTC100k_3950);

    NTC2.ADC_Raw = ADCi.read(SINGLE_6);
    NTC2.ADC_Value = Comfilter*NTC2.ADC_Value + (1-Comfilter)*NTC2.ADC_Raw;
    NTC2.Resistance =((float)(NTC2.ADC_Value)*_ADC_MAX*NTC2.RS)/(_ADC_MAX*(_ADC_MAX-(float)NTC2.ADC_Value));
    NTC2.Temperature_C = R_To_Temperature(NTC2.Resistance, NTC100k_3950);

    NTC3.ADC_Raw = ADCi.read(SINGLE_5);
    NTC3.ADC_Value = Comfilter*NTC3.ADC_Value + (1-Comfilter)*NTC3.ADC_Raw;
    NTC3.Resistance =((float)(NTC3.ADC_Value)*_ADC_MAX*NTC3.RS)/(_ADC_MAX*(_ADC_MAX-(float)NTC3.ADC_Value));
    NTC3.Temperature_C = R_To_Temperature(NTC3.Resistance, NTC100k_3950);

    NTC4.ADC_Raw = ADCi.read(SINGLE_4);
    NTC4.ADC_Value = Comfilter*NTC4.ADC_Value + (1-Comfilter)*NTC4.ADC_Raw;
    NTC4.Resistance =((float)(NTC4.ADC_Value)*_ADC_MAX*NTC4.RS)/(_ADC_MAX*(_ADC_MAX-(float)NTC4.ADC_Value));
    NTC4.Temperature_C = R_To_Temperature(NTC4.Resistance, NTC100k_3950);

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
  UART1.println("Initialize UART1: UART1 115200 8N1");
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
  /*Read ESP32 FW update flag from NVS Flash*/
  MAIN_FW_Update_Flag = EEPROM.read(101);
  UART_Debug.print("MAIN FW UPDATE FLAG:");
  UART_Debug.println((int)MAIN_FW_Update_Flag);
  
  if(MAIN_FW_Update_Flag == 1)
  {
    OTADRIVE_UPDATE(SENSOR_FIRMWARE_VERSION,30);
    MAIN_FW_Update_Flag = 0;
    EEPROM.write(101,0);
    EEPROM.commit();
  }
  
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
    UART_Debug.println((char*)outcomData);
    if(Uart1_Transceiving == 0)
    {
      UART1.println("Send ESP now");
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
    }
    if(millis() - Uart1_Transceiving_tick >= 5000)
    {
      Uart1_Transceiving = 0;
      Uart1_Transceiving_tick = millis();
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
float R_To_Temperature(float Resistance, NTCType_Typedef NTC_Type)
{
  float Tb,Rb,Beta;
  float retVal;
  if(Resistance <= NTC_Type.R150) /*150 300*/
  {
    Tb = 150;
    Rb = NTC_Type.R150;
    Beta = NTC_Type.BETA150_300;
  }
  else if(Resistance <= NTC_Type.R100) /*100 150*/
  {
    Tb = 100;
    Rb = NTC_Type.R100;
    Beta = NTC_Type.BETA100_150;
  }
  else if(Resistance <= NTC_Type.R50) /*50 100*/
  {
    Tb = 50;
    Rb = NTC_Type.R50;
    Beta = NTC_Type.BETA50_100;
  }
  else if(Resistance <= NTC_Type.R25) /*25 50*/
  {
    Tb = 25;
    Rb = NTC_Type.R25;
    Beta = NTC_Type.BETA25_50;
  }
  else if(Resistance > NTC_Type.R25) /*25 50*/
  {
    Tb = 25;
    Rb = NTC_Type.R25;
    Beta = NTC_Type.BETA25_50;
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
  static uint8_t repSeq;
  memset(outcomData_uart1,0,100);
  repSeq = 1;
  if (strstr((char*)Uart_Receive_Cmd_Buff1,"R FF 00000000") != NULL)
  {
    UART1.printf("> R FF 00000000");
  }
  else if (strstr((char*)Uart_Receive_Cmd_Buff1,"R T1 00000000") != NULL)
  {
    sprintf(outcomData_uart1,"> R T1 %08.3f ",NTC1.Temperature_C);
  }
  else if (strstr((char*)Uart_Receive_Cmd_Buff1,"R T2 00000000") != NULL)
  {
    sprintf(outcomData_uart1,"> R T2 %08.3f ",NTC2.Temperature_C);
  }
  else if (strstr((char*)Uart_Receive_Cmd_Buff1,"R T3 00000000") != NULL)
  {
    sprintf(outcomData_uart1,"> R T3 %08.3f ",TC_K_Temperature);
  }
  else if (strstr((char*)Uart_Receive_Cmd_Buff1,"R T4 00000000") != NULL)
  {
    sprintf(outcomData_uart1,"> R T4 %08.3f ",NTC4.Temperature_C);
  }
  else if (strstr((char*)Uart_Receive_Cmd_Buff1,"R I1 00000000") != NULL)
  {
    sprintf(outcomData_uart1,"> R I1 %08.5f ",PS1.Current);
  }
  else if (strstr((char*)Uart_Receive_Cmd_Buff1,"R I2 00000000") != NULL)
  {
    sprintf(outcomData_uart1,"> R I2 %08.5f ",PS2.Current);
  }
  else if (strstr((char*)Uart_Receive_Cmd_Buff1,"R N1 00000000") != NULL)
  {
    sprintf(outcomData_uart1,"> R N1 %08.3f ",NTC1.Temperature_C);
  }
  else if (strstr((char*)Uart_Receive_Cmd_Buff1,"R N2 00000000") != NULL)
  {
    sprintf(outcomData_uart1,"> R N2 %08.3f ",NTC2.Temperature_C);
  }
  else if (strstr((char*)Uart_Receive_Cmd_Buff1,"R N3 00000000") != NULL)
  {
    sprintf(outcomData_uart1,"> R N3 %08.3f ",NTC3.Temperature_C);
  }
  else if (strstr((char*)Uart_Receive_Cmd_Buff1,"R N4 00000000") != NULL)
  {
    sprintf(outcomData_uart1,"> R N4 %08.3f ",NTC4.Temperature_C);
  }
  else if (strstr((char*)Uart_Receive_Cmd_Buff1,"R P1 00000000") != NULL)
  {
    sprintf(outcomData_uart1,"> R P1 %08.5f ",PS1.Pressure);
  }
  else if (strstr((char*)Uart_Receive_Cmd_Buff1,"R P2 00000000") != NULL)
  {
    sprintf(outcomData_uart1,"> R P2 %08.5f ",PS2.Pressure);
  }
  else
  {
    repSeq = 0;
  }
  if(repSeq == 1)
  {
    Uart1_Transceiving = 1;
    Uart1_Transceiving_tick = millis();
    repSeq = 0;
    CRC16_Val_CCIT = CRC.CRC16_CCIT((uint8_t*)outcomData_uart1,16);
    sprintf(&outcomData_uart1[16],"%05d\r\n",CRC16_Val_CCIT);
    UART1.print(outcomData_uart1);
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
        if(lucCountEndByte1 == 2 || Uart_rx1 == 0x0A)
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
  NTC1.RS = 10000.0;
  NTC2.RS = 10000.0;
  NTC3.RS = 10000.0;
  NTC4.RS = 10000.0;
  NTC1.ADC_Raw = ADCi.read(SINGLE_7);
  NTC1.ADC_Value = NTC1.ADC_Raw;
  NTC1.Resistance =((float)(NTC1.ADC_Value)*_ADC_MAX*NTC1.RS)/(_ADC_MAX*(_ADC_MAX-(float)NTC1.ADC_Value));
  NTC1.Temperature_C = R_To_Temperature(NTC1.Resistance, NTC100k_3950);

  NTC2.ADC_Raw = ADCi.read(SINGLE_6);
  NTC2.ADC_Value = NTC2.ADC_Raw;
  NTC2.Resistance =((float)(NTC2.ADC_Value)*_ADC_MAX*NTC2.RS)/(_ADC_MAX*(_ADC_MAX-(float)NTC2.ADC_Value));
  NTC2.Temperature_C = R_To_Temperature(NTC2.Resistance, NTC100k_3950);

  NTC3.ADC_Raw = ADCi.read(SINGLE_5);
  NTC3.ADC_Value = NTC3.ADC_Raw;
  NTC3.Resistance =((float)(NTC3.ADC_Value)*_ADC_MAX*NTC3.RS)/(_ADC_MAX*(_ADC_MAX-(float)NTC3.ADC_Value));
  NTC3.Temperature_C = R_To_Temperature(NTC3.Resistance, NTC100k_3950);

  NTC4.ADC_Raw = ADCi.read(SINGLE_4);
  NTC4.ADC_Value = NTC4.ADC_Raw;
  NTC4.Resistance =((float)(NTC4.ADC_Value)*_ADC_MAX*NTC4.RS)/(_ADC_MAX*(_ADC_MAX-(float)NTC4.ADC_Value));
  NTC4.Temperature_C = R_To_Temperature(NTC4.Resistance, NTC100k_3950);

  PS1.ADC_Raw = ADCi.read(SINGLE_0);
  PS1.ADC_Value = PS1.ADC_Raw;
  PS1.Voltage = PS1.ADC_Value*_Vref/_ADC_MAX;
  PS1.Current = PS1.Voltage / PS1.Resistance;
  PS1.Pressure = map_value(PS1.Current,4.0,20.0,0.0,10.0);
  
  PS2.ADC_Raw = ADCi.read(SINGLE_1);
  PS2.ADC_Value = PS2.ADC_Raw;
  PS2.Voltage = PS2.ADC_Value*_Vref/_ADC_MAX;
  PS2.Current = PS2.Voltage / PS2.Resistance;
  PS2.Pressure = map_value(PS2.Current,4.0,20.0,0.0,10.0);
  
  TC_K_Temperature = map_value(PS1.Current,4.0,20.0,0,800);
}
void OTADRIVE_UPDATE(String Version,int OTA_WIFI_timeout)
{
  int time_out=0;
  // Wifi IFACTORY connectting
  WiFi.begin("ifactory", "ifactory");
  Serial.println("Connect to OTA wifi - ifactory - then check new FirmWare ");
//  UART_HMI.print("t_stt.txt=\"Connect to OTA wifi - ifactory\"");
//  UART_HMI.write(0xFF);UART_HMI.write(0xFF);UART_HMI.write(0xFF);
  while ((WiFi.status() != WL_CONNECTED) && (time_out<OTA_WIFI_timeout))
    {
      
      Serial.print(".");
//      digitalWrite(LED1, HIGH);
      delay(100);
//      digitalWrite(LED1, LOW);
      delay(200);
      time_out++;
    }
  // Wifi IFACTORY not found !
  if (time_out == OTA_WIFI_timeout) { 
    Serial.println("");
    Serial.print("Connect to OTA wifi - ifactory - failed ! ");
    Serial.print("Return to next code !");
    Serial.println("");
//    UART_HMI.print("t_stt.txt=\"Connect to OTA wifi - failed\"");
//    UART_HMI.write(0xFF);UART_HMI.write(0xFF);UART_HMI.write(0xFF);
    return; // Exit OTAUPDATE_DRIVE
    }



  // Wifi IFACTORY connected !
    Serial.print("Wifi OTA connected : ");
    Serial.println(WiFi.localIP());
//    UART_HMI.print("t_stt.txt=\"Wifi OTA connected\"");
//    UART_HMI.write(0xFF);UART_HMI.write(0xFF);UART_HMI.write(0xFF);
  // initialize FileSystem
              OTADRIVE.setFileSystem(&FILESYS);
            #ifdef ESP8266
              if (!LittleFS.begin())
              {
                Serial.println("LittleFS Mount Failed");
                LittleFS.format();
                return;
              }
            #elif defined(ESP32)
              if (!SPIFFS.begin(true))
              {
                Serial.println("SPIFFS Mount Failed");
                return;
              }
            #endif
              Serial.println("File system Mounted");

    //**********************************************************************************************************************************
    //
    //                                                                OTA DRIVE 
    
    
    OTADRIVE.setInfo(OTA_APIKEY, Version);

    OTADRIVE.onUpdateFirmwareProgress(onUpdateProgress);


  // retrive firmware info from OTA drive server


        updateInfo inf = OTADRIVE.updateFirmwareInfo();
        Serial.printf("\nFirmware info: %s ,%dBytes\n%s\n",
                      inf.version.c_str(), inf.size, inf.available ? "New version available" : "No newer version");
        // update firmware if newer available
        if (inf.available)
          OTADRIVE.updateFirmware();
        // sync local files with OTAdrive server
        OTADRIVE.syncResources();
        // list local files to serial port
        listDir(FILESYS, "/", 0);
        WiFi.disconnect();

  //**************************************************************************************************************************************
}
void onUpdateProgress(int progress, int totalt)
{
  static int last = 0;
  int progressPercent = (100 * progress) / totalt;
  Serial.print("*");
//  UART_HMI.printf("t_pc.txt=\"%d%%\"",progressPercent);
//  UART_HMI.write(0xFF);UART_HMI.write(0xFF);UART_HMI.write(0xFF);
//  UART_HMI.printf("j0.val=%d",progressPercent);
//  UART_HMI.write(0xFF);UART_HMI.write(0xFF);UART_HMI.write(0xFF);
  if (last != progressPercent && progressPercent % 10 == 0)
  {
    //print every 10%
    Serial.printf("%d", progressPercent);
  }
  if(progressPercent == 100)
  {
//    UART_HMI.print("vis b_ok,1");UART_HMI.write(0xFF);UART_HMI.write(0xFF);UART_HMI.write(0xFF);
//    UART_HMI.print("vis b_exit,1");UART_HMI.write(0xFF);UART_HMI.write(0xFF);UART_HMI.write(0xFF);
  }
  last = progressPercent;
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname, "r");
  if (!root)
  {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}
