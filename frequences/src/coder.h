#ifndef CODER_H_
#define CODER_H_

#define PIN_CODER_GND 23
#define PIN_CODER_VCC 24
#define PIN_CODER_A   25        // data
#define PIN_CODER_B   26        // switch D1
#define PIN_CODER_C   27        // clock

#define PIO_PDSR_D (*(volatile uint32_t *)0x400E143CU)   // PORTD 32 bits
#define BIT_CODER_CLOCK   2                              // D2 sur PORTD
#define BIT_CODER_DATA    0                              // D0 sur PORTD
#define BIT_CODER_SW      1                              // D1 sur PORTD

// these 2 parameters are working together ; their product is the minimum time where no change should occur 
// on the clock line after a change
// the pooling interval duration is the time between 2 calls to the coder timer handler
// its also half the time where no change should occur on the clock line after a change 
// to validate it
// shortly there's 2 strobes : no change after change and no change before next change

#define CODER_TIMER_POOLING_INTERVAL_MS 5  // timer pooling interval in milliseconds
#define CODER_STROBE_NUMBER 3              // number of timer intervals for a valid strobe

void init_un_codeur();

#endif /* CODER_H_ */
