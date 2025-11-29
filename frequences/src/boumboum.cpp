#include <Arduino.h> 
#include <math.h> 
#include "const.h"  
#include "frequences.h"
#include "coder.h"
#include "util_periph.h"

#define LED_BLUE          5
#define LED_RED           6
#define LED_GREEN         7

uint32_t portD=  PIO_PDSR_D ;

int32_t coder1Counter=0;


void initLeds(){
    pinMode(LED_BLUE,OUTPUT); digitalWrite(LED_BLUE,LOW);
    pinMode(LED_RED,OUTPUT);  digitalWrite(LED_RED,LOW);
    pinMode(LED_GREEN,OUTPUT);digitalWrite(LED_GREEN,LOW);
}

void setup(){
    Serial.begin(115200);Serial.println("+boumboum");

    coderInit(PIO_PDSR_D,PIO_CLOCK,PIO_DATA,PIO_SW,PIN_CODER_A,PIN_CODER_B,PIN_CODER_C,PIN_CODER_GND,PIN_CODER_VCC,CODER_TIMER_POOLING_INTERVAL_MS,CODER_STROBE_NUMBER);
    coderSetup(&coder1Counter);

    initLeds();
}

void loop(){

      if(coder1Counter/2000==0){
      digitalWrite(LED_RED,!digitalRead(LED_RED));
    }
}
