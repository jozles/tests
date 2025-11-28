#include <Arduino.h>
#include "coder.h"  
#include "SAMDUETimerInterrupt.h"
#include "interruptTimerServer.h"


volatile int32_t* coderTimerCount=0;
uint8_t coderItStatus=0;
bool coderClock=0;
bool coderClock0=0;
bool coderData=0;
bool coderData0=0;

void coderTimerHandler(){
/*
    coderClock= (PIO_PDSR_D & (1<<BIT_CODER_CLOCK)) !=0 ;

    if(coderClock == coderClock0){                                  // no change : exit
        if(coderItStatus<CODER_STROBE_NUMBER){coderItStatus++;}
        return;
    }
    
    if(coderItStatus<CODER_STROBE_NUMBER){coderItStatus=0;return;}  // change too close to previous valid one : ignore it

    coderItStatus=0;

    if(coderClock==0){
        coderClock0=coderClock;                                     // valid falling change : update state
        return;}                                      

    coderData= (PIO_PDSR_D & (1<<BIT_CODER_DATA)) !=0 ;
    
    if(coderData==coderData0){return;}                              // no change on data : invalid clock or data : exit
           
    coderData0=coderData;                                           // valid rising change : update state
    if(coderData){*coderTimerCount++;} else {*coderTimerCount--;}
    
    return;
*/

    coderClock= (PIO_PDSR_D & (1<<BIT_CODER_CLOCK)) !=0 ;
    if(coderClock == coderClock0){                                 // no change 
        if(coderItStatus<CODER_STROBE_NUMBER){coderItStatus++;}
        return;                                                     // wait for change after strobe delay 
    }
    if(coderItStatus==CODER_STROBE_NUMBER){
        if(coderClock == coderClock0){return;}
        coderItStatus++;return;
    }
    if(coderClock == coderClock0){return;}

    coderClock0=coderClock;
    coderItStatus=0;
    if(coderClock==0){return;}                                      // falling edge : exit

    coderData= (PIO_PDSR_D & (1<<BIT_CODER_DATA)) !=0 ;
    if(coderData==coderData0){
        coderItStatus=0;
        return;                                                     // no change on data : invalid clock or data : exit
    }
        coderData0=coderData;                                       // valid rising change : update state
    if(coderData){*coderTimerCount++;} else {*coderTimerCount--;}
    return;
}



void init_un_codeur(){
    
    pinMode(25,INPUT_PULLUP);   // coder A  data
    pinMode(27,INPUT_PULLUP);   // coder C  clock
    pinMode(26,INPUT_PULLUP);   // coder B  switch

    pinMode(PIN_CODER_GND,OUTPUT);digitalWrite(23,LOW);       // coder ground
    pinMode(PIN_CODER_VCC,OUTPUT);digitalWrite(24,HIGH);      // coder Vcc 
    
    attachDueInterrupt(CODER_TIMER_POOLING_INTERVAL_MS*1000, coderTimerHandler, "coderTimer");
}



