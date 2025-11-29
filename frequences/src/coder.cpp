#include <Arduino.h>
#include "coder.h"  
#include "SAMDUETimerInterrupt.h"
#include "interruptTimerServer.h"

uint16_t coderTimerPoolingInterval=1;
uint8_t coderStrobeNumber=3;
volatile int32_t* coderTimerCount=nullptr;
uint32_t coderTimer=0;
uint8_t coderItStatus=0;
bool coderClock=0;
bool coderClock0=0;
bool coderData=0;
bool coderData0=0;
volatile uint32_t pio_port;
uint8_t pio_clock_bit;
uint8_t pio_data_bit;
uint8_t pio_switch_bit;


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
    coderTimer++;
    coderClock= (pio_port & (1<<pio_clock_bit)) !=0 ;
    
    if(coderClock == coderClock0){                                  // no change 
        if(coderItStatus<coderStrobeNumber){                      // wait for change after strobe delay
            coderItStatus++;
            return;}
        if(coderItStatus>coderStrobeNumber){                      // coderClock/clock0 should be different : reset status
            coderItStatus=0;
            return;}
    }
    
    // clock change detected or valid 2nd strobe reached (coderItStatus > CODER_STROBE_NUMBER)

    if(coderItStatus<coderStrobeNumber){                          // change to close to previous valid one : ignore it
        coderItStatus=0;
        return;}
                                                            
    if(coderItStatus==coderStrobeNumber){     
        coderItStatus++;return;                                     // 1st strobe passed wait next It
    }

    // valid clock change detected after 2 strobes delay

    coderClock0=coderClock;
    coderItStatus=0;
    if(coderClock==0){return;}                                      // falling edge : nothing to do

    coderData= (pio_port & (1<<pio_data_bit)) !=0 ;

    if(coderData==coderData0){return;}                              // no change on data : invalid clock or data : exit
    
    coderData0=coderData;                                       // valid rising change : update state
    if(coderTimerCount!=nullptr){
        if(coderData){*coderTimerCount++;} else {*coderTimerCount--;}
    }
    // here accelerator management could be added
    
    return;
}

void coderInit(volatile uint32_t port,uint8_t pio_ck,uint8_t pio_d,uint8_t pio_sw,uint8_t clock_pin,uint8_t data_pin,uint8_t switch_pin,uint8_t ground_pin,uint8_t vcc_pin,uint16_t ctpi,uint8_t cstn){
    
    pio_port=port;
    pio_clock_bit=pio_ck;
    pio_data_bit=pio_d;
    pio_switch_bit=pio_sw;
    coderTimerPoolingInterval=ctpi;
    coderStrobeNumber=cstn;

    coderTimer=0;
    coderItStatus=0;
    coderClock0= (pio_port & (1<<pio_clock_bit)) !=0 ;
    coderData0= (pio_port & (1<<pio_data_bit)) !=0 ;

    pinMode(data_pin,INPUT_PULLUP);   // 25 coder A  data
    pinMode(clock_pin,INPUT_PULLUP);   // 27 coder C  clock
    pinMode(switch_pin,INPUT_PULLUP);   // 26 coder B  switch

    pinMode(ground_pin,OUTPUT);digitalWrite(23,LOW);       // coder ground
    pinMode(vcc_pin,OUTPUT);digitalWrite(24,HIGH);      // coder Vcc 

    attachDueInterrupt(coderTimerPoolingInterval*1000, coderTimerHandler, "coderTimer");
}

void coderSetup(volatile int32_t* cTC){
    coderTimerCount=cTC;}




