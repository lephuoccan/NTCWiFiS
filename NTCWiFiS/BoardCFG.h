

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
typedef struct
{
  int ADC_Raw;
  float ADC_Value;
  float Voltage_G;
  float Voltage_NTC;
  float Resistance;
  float Temperature_K;
  float Temperature_C;
  float VR1;
  float VNTC;
  float I;
} NTC_Typedef;
