#include <ESP8266WiFi.h>

#define LED 0

uint8_t cnt=0;

void setup() {

Serial.begin(115200);
Serial.println("ready");
  
pinMode(LED,OUTPUT);
pinMode(2,OUTPUT);
pinMode(5,OUTPUT);

}

void loop() {
  
digitalWrite(LED,HIGH);digitalWrite(2,HIGH);digitalWrite(5,LOW);delay(500);
digitalWrite(LED,LOW);digitalWrite(2,LOW);digitalWrite(5,HIGH);delay(500);

}
