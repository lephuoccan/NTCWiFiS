extern "C"{
  #include "circular_buffer.h"
}
#include "BoardCFG.h"
#include "MCP3208.h"
#include <driver/adc.h>

#define NSS 26
static const int spiClk = 1000000; // 1 MHz
iMCP3208 ADCi(14,12,13,NSS,spiClk);

const float _ADC_MAX = 4095;         /* ADC 12bit */
const float _T0 = 25.0;              /*  */
const float _NTC_BETA = 3950.0;
const float _R0 = 100000.0;
const float _R1 = 10000.0;

NTC_Typedef NTC1;
NTC_Typedef NTC2;
NTC_Typedef NTC3;
NTC_Typedef NTC4;

/*ADC Variables*/
uint16_t ADC_Value; /*ADC value Read from ADC_pin ( analog mltiplexer)*/
uint16_t NTC1_ADC_Val_Raw;  /*NTC1 ADC value Read from NTC1_Pin or analog mltiplexer (ouput SB2 = 0, SB1 = 0, SB0 = 0) */
float NTC1_ADC_Val;
uint16_t NTC2_ADC_Val;  /*NTC2 ADC value Read from NTC2_Pin or analog mltiplexer (ouput SB2 = 0, SB1 = 0, SB0 = 1) */
uint16_t NTC3_ADC_Val;  /*NTC3 ADC value Read from NTC3_Pin or analog mltiplexer (ouput SB2 = 0, SB1 = 1, SB0 = 0) */
uint16_t NTC4_ADC_Val;  /*NTC4 ADC value Read from NTC4_Pin or analog mltiplexer (ouput SB2 = 0, SB1 = 1, SB0 = 1) */
uint16_t PS1_ADC_Val;   /*PS1 ADC value Read from PS1_Pin or analog mltiplexer (ouput SB2 = 1, SB1 = 0, SB0 = 0) */
uint16_t PS2_ADC_Val;   /*PS2 ADC value Read from PS2_Pin or analog mltiplexer (ouput SB2 = 1, SB1 = 0, SB0 = 1) */
uint16_t VREF_ADC_Val;  /*VREF ADC value Read from analog mltiplexer (ouput SB2 = 1, SB1 = 1, SB0 = 1) */
float ADC_2500_Val;
int ADC_PWR_Val;
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
//    ADC_PWR_Val = Read_ADC_MultiSampling(PWR_Pin);
//    vTaskDelay(1 / portTICK_PERIOD_MS);
//    ADC_2500_Val = Read_ADC_MultiSampling(VREF_Pin);
//    vTaskDelay(1 / portTICK_PERIOD_MS);
//    NTC1.ADC_Raw = Read_ADC_MultiSampling(NTC1_Pin);
//    NTC1.ADC_Value = 0.99*(NTC1.ADC_Value) + 0.01*((float)NTC1.ADC_Raw);
//    NTC1.Voltage_G = map_value(NTC1.ADC_Value,ADC_2500_Val,4095,2.5,3.2);
//    NTC1.VR1 = 3.329 - NTC1.Voltage_G;
//    NTC1.I = NTC1.VR1 / R1;
//    NTC1.Resistance = (NTC1.Voltage_G / NTC1.I) - R2;
//    NTC1.VNTC = NTC1.Resistance*NTC1.I;
//    NTC1.Temperature_K = (NTC_BETA*(273.15+T0))/(NTC_BETA+((273.15+T0)*log(NTC1.Resistance/R0)));
//    NTC1.Temperature_C = NTC1.Temperature_K - 273.15;
    NTC1.ADC_Raw = ADCi.read(SINGLE_7);
    NTC1.ADC_Value = 0.995*NTC1.ADC_Value + 0.005*NTC1.ADC_Raw;
    NTC1.Resistance =((float)(NTC1.ADC_Value)*_ADC_MAX*10000)/(_ADC_MAX*(_ADC_MAX-(float)NTC1.ADC_Value));
    NTC1.Temperature_C = (_NTC_BETA*(273.15+_T0))/(_NTC_BETA+((273.15+_T0)*log(NTC1.Resistance/_R0)))-273.15;
    
    
    vTaskDelay(1 / portTICK_PERIOD_MS); /*Delay 1000ms*/
  }
}
void setup() {
  // put your setup code here, to run once:
  UART_Debug.begin(115200);
//  pinMode(SA0_Pin,OUTPUT);
//  pinMode(SA1_Pin,OUTPUT);
//  pinMode(SA2_Pin,OUTPUT);
//  digitalWrite(SA0_Pin, LOW);
//  digitalWrite(SA1_Pin, LOW);
//  digitalWrite(SA2_Pin, LOW);
//  Switch_ADC_Pin(VREF_Pin);
//  Switch_ADC_Pin(PWR_Pin);
  delay(500);
  ADCi.begin();
  UART_Debug.println("Hello World");
  UART_Debug.printf("Setup is running on CPU %d\n", xPortGetCoreID());
  
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
//  UART_Debug.printf("Resistance = %f\n",NTC1.Resistance);
  UART_Debug.printf("R T = %f %f\n",NTC1.Resistance,NTC1.Temperature_C);
  vTaskDelay(1000 / portTICK_PERIOD_MS); /*Delay 1000ms*/
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
