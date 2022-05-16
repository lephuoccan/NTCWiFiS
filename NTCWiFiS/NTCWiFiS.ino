extern "C"{
  #include "circular_buffer.h"
}
#include "BoardCFG.h"
/*ADC Variables*/
uint16_t ADC_Value; /*ADC value Read from ADC_pin ( analog mltiplexer)*/
uint16_t NTC1_ADC_Val;  /*NTC1 ADC value Read from NTC1_Pin or analog mltiplexer (ouput SB2 = 0, SB1 = 0, SB0 = 0) */
uint16_t NTC2_ADC_Val;  /*NTC2 ADC value Read from NTC2_Pin or analog mltiplexer (ouput SB2 = 0, SB1 = 0, SB0 = 1) */
uint16_t NTC3_ADC_Val;  /*NTC3 ADC value Read from NTC3_Pin or analog mltiplexer (ouput SB2 = 0, SB1 = 1, SB0 = 0) */
uint16_t NTC4_ADC_Val;  /*NTC4 ADC value Read from NTC4_Pin or analog mltiplexer (ouput SB2 = 0, SB1 = 1, SB0 = 1) */
uint16_t PS1_ADC_Val;   /*PS1 ADC value Read from PS1_Pin or analog mltiplexer (ouput SB2 = 1, SB1 = 0, SB0 = 0) */
uint16_t PS2_ADC_Val;   /*PS2 ADC value Read from PS2_Pin or analog mltiplexer (ouput SB2 = 1, SB1 = 0, SB0 = 1) */
uint16_t VREF_ADC_Val;  /*VREF ADC value Read from analog mltiplexer (ouput SB2 = 1, SB1 = 1, SB0 = 1) */
float NTC1, NTC2, NTC3, NTC4, PS1, PS2, I1, I2;
/***************/
void ADC_Process(void * parameter)
{
  UART_Debug.printf("ADC_Process is running on CPU %d", xPortGetCoreID());
  for(;;)
  {
    
//    vTaskDelay(1000 / portTICK_PERIOD_MS); /*Delay 1000ms*/
  }
}
void setup() {
  // put your setup code here, to run once:
  UART_Debug.begin(115200);
  UART_Debug.println("Hello World");
  
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
  UART_Debug.printf("NTC %f\n",NTC1,NTC2,NTC3,NTC4);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  
}
