#include <Arduino.h> 
#include <math.h>   
#include "frequences.h"
#include "util_periph.h"


uint32_t port_D;
bool clock_;
bool clock0=0;
bool data;
bool data0=0;
bool switch_;
uint8_t valid;
unsigned long valid_dly=0;

// PIO_PDSR 32 bits input/output (ex  0x400E143C (PIOD))
#define PIO_PDSR_D (*(volatile uint32_t *)0x400E143CU)

void init_un_codeur(){
    //bitSet(PORT_D,14); // A14 output
    pinMode(23,OUTPUT);digitalWrite(7,LOW);  // coder ground A14
    pinMode(24,OUTPUT);digitalWrite(8,HIGH); // coder Vcc A15
    pinMode(25,INPUT_PULLUP);   // coder A  D0
    pinMode(26,INPUT_PULLUP);   // coder B  D1
    pinMode(27,INPUT_PULLUP);   // coder C  D2 
    digitalWrite(23,LOW);       // coder ground
    digitalWrite(24,HIGH);      // coder Vcc   
}

void setup(){
    Serial.begin(115200);Serial.println("+boumboum");
    init_un_codeur();
    while(1){
        data=digitalRead(25);
        clock_=digitalRead(27);
        switch_=digitalRead(26);

        if(clock0==0 && clock_==1){ // front montant
          // la transition est validée si toujours pésente après 1mS
          switch (valid){                           // 0 rien ; 1 valid en cours ; 2 validé
            case 0:
              valid=1;valid_dly=millis();break;    
            case 1:
              if(millis()-valid_dly>1){
                valid=2;break;
              }
            case 2:
            default:
              break;            
            }
            
          if(valid==2){
            valid=0;
            data0=data;
            if(data0==0){Serial.print("+");}else{Serial.print("-");}
            
            /*Serial.print("\n");
            Serial.print(digitalRead(25));Serial.print(' ');
            Serial.print(digitalRead(26));Serial.print(' ');
            Serial.print(digitalRead(27));Serial.print(' ');
            port_D = PIO_PDSR_D;
            Serial.print(port_D, BIN);
            Serial.print(' ');*/
          }
        }
        clock0=clock_;
        delay(1);
    }
}

void loop(){}
