#include <Arduino.h> 
#include <math.h> 
#include "const.h"  
#include "frequences.h"
#include "coder.h"
#include "util_periph.h"

uint8_t leds[NBLEDS]={LED_RED,LED_GREEN,LED_BLUE};

uint32_t portD=  PIO_PDSR_D ;

volatile int32_t coder1Counter=0;
int32_t coder1Counter0=0;

uint32_t intDur;

void initLeds(){
    pinMode(LED,OUTPUT); digitalWrite(LED,LOW);
    pinMode(LED_BLUE,OUTPUT);digitalWrite(LED_BLUE,LOW);
    pinMode(LED_RED,OUTPUT);digitalWrite(LED_RED,LOW);
    pinMode(LED_GREEN,OUTPUT);digitalWrite(LED_GREEN,LOW);
    pinMode(LED_GND,OUTPUT);digitalWrite(LED_GND,LOW);
}

void setup(){
    Serial.begin(115200);Serial.println("+boumboum");

    coderInit(PIO_PDSR_D,PIO_CLOCK,PIO_DATA,PIO_SW,PIN_CODER_A,PIN_CODER_B,PIN_CODER_C,PIN_CODER_GND,PIN_CODER_VCC,CODER_TIMER_POOLING_INTERVAL_MS,CODER_STROBE_NUMBER);
    coderSetup(&coder1Counter);

    initLeds();
}

void loop(){

    if(coder1Counter!=coder1Counter0){
        Serial.print("\r     \r");Serial.print(coder1Counter);            // Serial.print(intDur);Serial.print(' ');
        coder1Counter0=coder1Counter;    
    }
    
}
