#include <Arduino.h>
#include <SPI.h>     
#include <Ethernet.h>   // Copyright 2018 Paul Stoffregen

#define PINLED 3

byte mac[6];
byte localIp[]={192,168,0,47};

void blink(uint8_t nb)
{
  for(nb=nb;nb>0;nb--){
    digitalWrite(PINLED,HIGH);delay(5);
    digitalWrite(PINLED,LOW);delay(100);
  }
}

void serialPrintLocalIp()
{
for(uint8_t i=0;i<4;i++){
    Serial.print(Ethernet.localIP()[i]);if(i<3){Serial.print(".");}}
  Serial.println();
}  

void serialPrintIp(uint8_t* ip)
{
  for(int i=0;i<4;i++){Serial.print(ip[i]);if(i<3){Serial.print(".");}}
  Serial.println();
}

void serialPrintMac(uint8_t* mac)
{
  for(uint8_t i=0;i<6;i++){Serial.print(mac[i],HEX);if(i<5){Serial.print(".");}}
  Serial.println();
}

void setup() {

  Serial.begin(115200);
  delay(1000);
  Serial.println("\nready");
   
  memcpy(mac,"\x55\x55\x55\x55\x55\x55",6); 
  
  Serial.print(" mac=");serialPrintMac(mac);

  if(Ethernet.begin(mac) == 0)
    {
    Serial.print("\nFailed with DHCP... forcing Ip ");serialPrintIp(localIp);
    Ethernet.begin (mac, localIp); 
    }
  Serial.print(" localIP=");Serial.print((IPAddress)Ethernet.localIP());
  Serial.println();

  

  while(1){blink(1);delay(1000);}
}

void loop() {
  // put your main code here, to run repeatedly:

}
