extern "C"{
  #include "circular_buffer.h"
}
#include "BoardCFG.h"
#include "MCP3208.h"
#include <driver/adc.h>

#define NSS 26
static const int spiClk = 1000000; // 1 MHz
iMCP3208 ADCi(14,12,13,NSS,spiClk);
const float _Vref = 3300;  
const float _ADC_MAX = 4095;         /* ADC 12bit */
const float _T0 = 25.0;              /*  */
float _NTC_BETA = 3950.0;

const float _R0 = 100000.0;
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
/*ADC Variables*/


const int MultiSample_num = 64;
int ADC_Sampling_Value[MultiSample_num];
/***************/
int getMostPopularElement(int arr[], const int n);
void Switch_ADC_Pin(int ADC_Pin);
int Read_ADC_MultiSampling(int Pin);
float map_value(float x, float in_min, float in_max, float out_min, float out_max);
void ADC_Process(void * parameter)
{
  vTaskDelay(1000 / portTICK_PERIOD_MS); /*Delay 1000ms*/
  UART_Debug.printf("ADC_Process is running on CPU %d\n", xPortGetCoreID());
  for(;;)
  {
    NTC1.ADC_Raw = ADCi.read(SINGLE_7);
    NTC1.ADC_Value = Comfilter*NTC1.ADC_Value + (1-Comfilter)*NTC1.ADC_Raw;
    NTC1.Resistance =((float)(NTC1.ADC_Value)*_ADC_MAX*10000)/(_ADC_MAX*(_ADC_MAX-(float)NTC1.ADC_Value));
    NTC1.Temperature_C = R_To_Temperature(NTC1.Resistance);

    NTC2.ADC_Raw = ADCi.read(SINGLE_6);
    NTC2.ADC_Value = Comfilter*NTC2.ADC_Value + (1-Comfilter)*NTC2.ADC_Raw;
    NTC2.Resistance =((float)(NTC2.ADC_Value)*_ADC_MAX*10000)/(_ADC_MAX*(_ADC_MAX-(float)NTC2.ADC_Value));
    NTC2.Temperature_C = R_To_Temperature(NTC2.Resistance);

    NTC3.ADC_Raw = ADCi.read(SINGLE_5);
    NTC3.ADC_Value = Comfilter*NTC3.ADC_Value + (1-Comfilter)*NTC3.ADC_Raw;
    NTC3.Resistance =((float)(NTC3.ADC_Value)*_ADC_MAX*10000)/(_ADC_MAX*(_ADC_MAX-(float)NTC3.ADC_Value));
    NTC3.Temperature_C = R_To_Temperature(NTC3.Resistance);

    NTC4.ADC_Raw = ADCi.read(SINGLE_4);
    NTC4.ADC_Value = Comfilter*NTC4.ADC_Value + (1-Comfilter)*NTC4.ADC_Raw;
    NTC4.Resistance =((float)(NTC4.ADC_Value)*_ADC_MAX*10000)/(_ADC_MAX*(_ADC_MAX-(float)NTC4.ADC_Value));
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
void setup() {
  // put your setup code here, to run once:
  UART_Debug.begin(115200);
  delay(500);
  
  ADCi.begin();
  UART_Debug.println("Hello World");
  UART_Debug.printf("Setup is running on CPU %d\n", xPortGetCoreID());
  
  PS1.Resistance = 150.0;
  PS2.Resistance = 150.0;
  
  xTaskCreatePinnedToCore(
    ADC_Process,
    "ADC_Process",  // Task name
    5000,             // Stack size (bytes)
    NULL,             // Parameter
    1,                // Task priority
    NULL,             // Task handle
    1    // CPU ID
  );
}

void loop() {
  // put your main code here, to run repeatedly:
//  UART_Debug.printf("ADC  = %d %d\n",NTC1.ADC_Raw,(int)NTC1.ADC_Value);
  UART_Debug.printf("T = %f   %f   %f   %f    %f\n", NTC1.Temperature_C, NTC2.Temperature_C, NTC3.Temperature_C, NTC4.Temperature_C, TC_K_Temperature);
//  UART_Debug.printf("ADC  = %f %f   %f %f\n",PS1.Voltage,PS1.Current,PS2.Voltage,PS2.Current );
  vTaskDelay(100 / portTICK_PERIOD_MS); /*Delay 1000ms*/
}

int getMostPopularElement(int arr[], const int n)
{
    int count = 1, tempCount;
    int temp = 0,i = 0,j = 0;
    //Get first element
    int popular = arr[0];
    for (i = 0; i < (n- 1); i++)
    {
        temp = arr[i];
        tempCount = 0;
        for (j = 1; j < n; j++)
        {
            if (temp == arr[j])
                tempCount++;
        }
        if (tempCount > count)
        {
            popular = temp;
            count = tempCount;
        }
    }
    return popular;
}
void Switch_ADC_Pin(int ADC_Pin)
{
  if(ADC_Pin == NTC1_Pin)
  {
    digitalWrite(SA0_Pin, LOW);
    digitalWrite(SA1_Pin, LOW);
    digitalWrite(SA2_Pin, LOW);
  }
  else if(ADC_Pin == NTC2_Pin)
  {
    digitalWrite(SA0_Pin, HIGH);
    digitalWrite(SA1_Pin, LOW);
    digitalWrite(SA2_Pin, LOW);
  }
  else if(ADC_Pin == NTC3_Pin)
  {
    digitalWrite(SA0_Pin, LOW);
    digitalWrite(SA1_Pin, HIGH);
    digitalWrite(SA2_Pin, LOW);
  }
  else if(ADC_Pin == NTC4_Pin)
  {
    digitalWrite(SA0_Pin, HIGH);
    digitalWrite(SA1_Pin, HIGH);
    digitalWrite(SA2_Pin, LOW);
  }
  else if(ADC_Pin == PS1_Pin)
  {
    digitalWrite(SA0_Pin, LOW);
    digitalWrite(SA1_Pin, LOW);
    digitalWrite(SA2_Pin, HIGH);
  }
  else if(ADC_Pin == PS2_Pin)
  {
    digitalWrite(SA0_Pin, HIGH);
    digitalWrite(SA1_Pin, LOW);
    digitalWrite(SA2_Pin, HIGH);
  }
  else if(ADC_Pin == PWR_Pin)
  {
    digitalWrite(SA0_Pin, LOW);
    digitalWrite(SA1_Pin, HIGH);
    digitalWrite(SA2_Pin, HIGH);
  }
  else if(ADC_Pin == VREF_Pin)
  {
    digitalWrite(SA0_Pin, HIGH);
    digitalWrite(SA1_Pin, HIGH);
    digitalWrite(SA2_Pin, HIGH);
  }
}
int Read_ADC_MultiSampling(int Pin)
{
  int ADC_Buff[MultiSample_num];
  int retVal;
  Switch_ADC_Pin(Pin);
  vTaskDelay(1 / portTICK_PERIOD_MS); /*Delay 1ms*/
  for(int i = 0; i< MultiSample_num; i++)
  {
    ADC_Buff[i] = analogRead(ADC_pin);
  }
  retVal = getMostPopularElement(ADC_Buff, MultiSample_num);
  return retVal;
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
