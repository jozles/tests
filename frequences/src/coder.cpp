#include <Arduino.h>
#include "coder.h"  
#include "SAMDUETimerInterrupt.h"
#include "interruptTimerServer.h"
#include "const.h"

uint16_t coderTimerPoolingInterval=1;       // delay betxeen Its (mS) changed by init
uint8_t coderStrobeNumber=3;                // 1st strobe delay (2nd strobe delay is 1)
volatile int32_t* coderTimerCount=nullptr;  // ptr to current value to be inc or dec
uint32_t coderTimer=0;                      // It counter
uint8_t coderItStatus=0;                    // coder decoding status
bool coderClock=0;                          // current physical coder clock value
bool coderClock0=0;                         // previous physical coder clock value
bool coderData=0;                           // current physical coder data value
bool coderData0=0;                          // previous physical coder data value
// due_pins
volatile uint32_t pio_port;
uint8_t pio_clock_bit;
uint8_t pio_data_bit;
uint8_t pio_switch_bit;
// arduino_pins
uint8_t arduino_clock_pin;
uint8_t arduino_data_pin;
uint8_t arduino_switch_pin;

uint8_t ledptr=0;
extern uint8_t leds[];

//uint32_t t0;
//extern uint32_t intDur;
//extern volatile int32_t coder1Counter;


void coderTimerHandler(){

/*if(coderTimer/256*256==coderTimer){                             // check Interrupt
    digitalWrite(LED,!digitalRead(LED));
}*/

    coderTimer++;
    //coderClock= (pio_port & (1<<pio_clock_bit)) !=0 ;
    coderClock=digitalRead(arduino_clock_pin);

    if(coderClock == coderClock0){                                // no change 
        if(coderItStatus<coderStrobeNumber){                      // wait for change after strobe delay
            coderItStatus++;return;}
        if(coderItStatus>coderStrobeNumber){                      // 2nd strobe fail
            coderItStatus=0;return;}
        return;
    }
    else{                                                         // clock change detected 
        coderData=digitalRead(arduino_data_pin);                  // latch data

        if(coderItStatus<coderStrobeNumber){                      // change to close to previous valid one : ignore it
            coderItStatus=0;return;}
                                                            
        if(coderItStatus==coderStrobeNumber){     
            coderItStatus++;return;}                              // 1st strobe passed wait next It
    
    }
 
    coderClock0=coderClock;                                       // valid clock change detected after 2 strobes delay
    coderItStatus=0;

    if(coderTimerCount!=nullptr){

        if((!coderClock)^coderData){
            (*coderTimerCount)--;
        } 
        else {
            (*coderTimerCount)++;
        }
    }

    // here accelerator management could be added
    
    return;
}

void coderInit(volatile uint32_t port,uint8_t pio_ck,uint8_t pio_d,uint8_t pio_sw,uint8_t clock_pin,uint8_t data_pin,uint8_t switch_pin,uint8_t ground_pin,uint8_t vcc_pin,uint16_t ctpi,uint8_t cstn){
    
    pio_port=port;
    pio_clock_bit=pio_ck;
    pio_data_bit=pio_d;
    pio_switch_bit=pio_sw;
    arduino_clock_pin=clock_pin;
    arduino_data_pin=data_pin;
    arduino_switch_pin=switch_pin;

    coderTimerPoolingInterval=ctpi;
    coderStrobeNumber=cstn;

    coderTimer=0;
    coderItStatus=0;

    pinMode(data_pin,INPUT_PULLUP);   // 25 coder A  data
    pinMode(clock_pin,INPUT_PULLUP);   // 27 coder C  clock
    pinMode(switch_pin,INPUT_PULLUP);   // 26 coder B  switch

    pinMode(ground_pin,OUTPUT);digitalWrite(23,LOW);       // coder ground
    pinMode(vcc_pin,OUTPUT);digitalWrite(24,HIGH);      // coder Vcc 

    //coderClock0= (pio_port & (1<<pio_clock_bit)) !=0 ;
    coderClock0=digitalRead(arduino_clock_pin);
    //coderData0= (pio_port & (1<<pio_data_bit)) !=0 ;
    coderData0=digitalRead(arduino_data_pin);

    attachDueInterrupt(coderTimerPoolingInterval*1000, coderTimerHandler, "coderTimer");
}

void coderSetup(volatile int32_t* cTC){
    coderTimerCount=cTC;}




