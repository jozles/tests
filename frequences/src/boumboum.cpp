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

//tests oscillateur    
    uint32_t a=72964537;
    double b,c,d,e,jd,cx,dx,ex;
    double f=1105;
    uint16_t ey;

    uint32_t t0,t1;
    double fsample=44100;
    double p=1/fsample;Serial.print(p,8);Serial.print(' ');

    uint64_t pi=(uint64_t) (226757);Serial.println((uint32_t)pi);
    uint64_t ne=fsample/f;
    uint64_t di,bi,ci,ei,fi=(uint64_t)f; 
    for(uint32_t j=a;j<(a+ne);j++){
        jd=(double)j;
        t0=micros();
        //b=((float)j)/(float)fsample;   // timeStamp
        
        b=jd*p;
        c=b*f;
        d=c-(double)((uint32_t)c);
        //e=d*2048;
        ey=(uint16_t)(d*2048);

        /*
        bi=j*pi;
        //ci=bi/1000*fi;
        cx=(double)bi/10000000000*f;
        //di=ci/10000000*10000000;di=ci-di;
        dx=cx-(double)((uint64_t)cx);
        //ei=di*2048/10000000;
        ex=dx*2048;
        */
        t1=micros()-t0;
        Serial.print(t1);Serial.println(' ');
        Serial.print(j);Serial.print(' ');Serial.print(b,6);Serial.print(' ');Serial.print(c,6);Serial.print(' ');Serial.print(d,6);Serial.print(' ');Serial.print(e);Serial.print(' ');Serial.println(ey);
        //Serial.print(j);Serial.print(' ');Serial.print((uint32_t)(bi/1000000));Serial.print(' ');Serial.print(cx,6);Serial.print(' ');Serial.print(dx,6);Serial.print(' ');Serial.println(ex);
    }
//    

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
