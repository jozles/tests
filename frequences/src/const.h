#ifndef _CONST_H
#define _CONST_H_

#define CODER_PORT PORTD

#define PIN_CODER_GND 23
#define PIN_CODER_VCC 24
#define PIN_CODER_A   25        // data
#define PIN_CODER_B   26        // switch D1
#define PIN_CODER_C   27        // clock

// every pins of a specific coder should be on the same port
#define PIO_PDSR_D uint32_t(*(volatile uint32_t *)0x400E143CU)   // PORTD 32 bits
#define PIO_CLOCK         2                              // D2 sur PORTD
#define PIO_DATA          0                              // D0 sur PORTD
#define PIO_SW            1                              // D1 sur PORTD

#define LED LED_BUILTIN

#endif  //_CONST_H_

