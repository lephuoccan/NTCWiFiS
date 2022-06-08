

#define DEBUG_DISABLED  0
#define DEBUG_ENABLED  1
#define DEBUG_MODE DEBUG_ENABLED

#define ADC1_CH0  36
#define ADC1_CH3  39
#define ADC1_CH4  32
#define ADC1_CH5  33
#define ADC1_CH6  34
#define ADC1_CH7  35

#define NTC1_Pin  ADC1_CH7
#define NTC2_Pin  ADC1_CH6
#define NTC3_Pin  ADC1_CH3
#define NTC4_Pin  ADC1_CH0
#define PS1_Pin   ADC1_CH5
#define PS2_Pin   ADC1_CH4
#define VREF_Pin  0
#define PWR_Pin   1
#define ADC_pin   ADC1_CH5

#define SA0_Pin   27  /* OUTPUT */
#define SA1_Pin   26  /* OUTPUT */
#define SA2_Pin   22  /* OUTPUT */

#define UART_Debug  Serial
#define UART1       Serial1

#define NTC_SR_OC       /*NTC open circuit*/
#define NTC_SR_DRDY      /*NTC Data ready*/

typedef struct
{
  float RS;             /*Potential divider resistor*/
  int ADC_Raw;          /*ADC raw value*/
  float ADC_Value;      /*ADC Value after 50Hz notch filter*/
  float Resistance;     /*Resistance (Ohm unit)*/
  float Temperature_C;  /*Temperature value in C*/
  uint8_t SR;           /*Status register*/
  
} NTC_Typedef;

typedef struct
{
  const float RS;             /*Potential divider resistor*/
  const float R25;            /*Resistance of Thermistor at 25°C, Must have value*/
  const float R50;            /*Resistance of Thermistor at 50°C, Must have value*/
  const float R100;           /*Resistance of Thermistor at 100°C, Must have value*/
  const float R150;           /*Resistance of Thermistor at 150°C, Optional value, 0 = don't care*/
  const float R300;           /*Resistance of Thermistor at 300°C, Optional value, 0 = don't care*/
  const float BETA25_50;      
  const float BETA50_100;
  const float BETA100_150;
  const float BETA150_300;
} NTCType_Typedef;

typedef struct
{
  int ADC_Raw;          /*ADC raw value*/
  float ADC_Value;      /*ADC Value after 50Hz notch filter*/
  float Resistance;
  float Voltage;        /*Voltage (mV)*/
  float Current;        /*Current (mA)*/
  float Pressure;       /*Pressure in Bar*/
  uint8_t SR;           /*Status register*/
} PS_Typedef;

NTCType_Typedef NTC100k_3950 = {
  .RS = 10000.0,
  .R25 = 100000.0,
  .R50 = 35899.9,
  .R100 = 6710.0,
  .R150 = 1770.0,
  .R300 = 105.6,
  .BETA25_50 = 3948.06,
  .BETA50_100 = 4044.69,
  .BETA100_150 = 4208.37,
  .BETA150_300 = 4504.0
};
NTCType_Typedef NTC10k_3950 = {
  .RS = 10000.0,
  .R25 = 10000.0,
  .R50 = 3588.0,
  .R100 = 674.4,
  .R150 = 0,
  .R300 = 0,
  .BETA25_50 = 3950.20,
  .BETA50_100 = 4031.17,
  .BETA100_150 = 0,
  .BETA150_300 = 0
};
