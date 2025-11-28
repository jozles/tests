#include <Arduino.h> 
#include <math.h>   
#include "frequences.h"
#include "coder.h"
#include "util_periph.h"

#define LED_BLUE          5
#define LED_RED           6
#define LED_GREEN         7

uint32_t portD=  PIO_PDSR_D ;
extern volatile int32_t* coderTimerCount;
int32_t coder1Counter=0;


void initLeds(){
    pinMode(LED_BLUE,OUTPUT); digitalWrite(LED_BLUE,LOW);
    pinMode(LED_RED,OUTPUT);  digitalWrite(LED_RED,LOW);
    pinMode(LED_GREEN,OUTPUT);digitalWrite(LED_GREEN,LOW);
}

void setup(){
    Serial.begin(115200);Serial.println("+boumboum");

    int32_t* coderTimerCount=&coder1Counter;
    init_un_codeur();

    initLeds();


}

void loop(){

      if(coder1Counter/2000==0){
      digitalWrite(LED_RED,!digitalRead(LED_RED));
    }
}
