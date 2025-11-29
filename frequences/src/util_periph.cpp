
#include <Arduino.h>

#include "const.h"

void blink_wait(){
  pinMode(LED,OUTPUT);
  while(1){
    digitalWrite(LED,HIGH);delay(50);
    digitalWrite(LED,LOW);delay(500);
    if (Serial.available()) break;} // wait for Serial to be ready             
}