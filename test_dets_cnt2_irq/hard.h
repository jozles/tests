#ifndef HARD_H_INCLUDED
#define HARD_H_INCLUDED

  #define PORT_NRFPWR PORTD
  #define DDR_NRFPWR  DDRD
  #define BIT_NRFPWR  7
  #define PORT_NRFCE  PORTB
  #define DDR_NRFCE   DDRB
  #define BIT_NRFCE   1
  #define PORT_MOSI   PORTB
  #define DDR_MOSI    DDRB  
  #define BIT_MOSI    3
  #define PORT_SPICK  PORTB
  #define DDR_SPICK   DDRB  
  #define BIT_SPICK   5
  #define PORT_VCHK   PORTC
  #define DDR_VCHK    DDRC
  #define BIT_VCHK    3
  #define PORT_DONE   PORTB
  #define DDR_DONE    DDRB
  #define BIT_DONE    0
  #define DONE        8  
  #define PORT_LED    PORTD
  #define DDR_LED     DDRD
  #define BIT_LED     4
  #define LED 4

  #define INADMUXVAL 0 | (1<<REFS1) | (1<<REFS0) | 0x08         // internal 1,1V ref + ADC input for internal temp
  #define VCHECKADC  7             // VOLTS ADC pin Nb
  #define VADMUXVAL  0 | (1<<REFS1) | (1<<REFS0) | VCHECKADC    // internal 1,1V ref + ADC input for volts
  //#define VFACTOR 0.00845         // volts conversion 1K+6,8K (MOSFET)
  #define VFACTOR 0.00594         // volts conversion 1,5K+6,8K (MOSFET)

  #define TCHECKADC 1             // TEMP  ADC pin Nb (6 DETS1.0 ; 1 DETS2.0)
  #define TREF      25            // TEMP ref for TOFFSET 
  #define LTH       6             // len thermo name                                 
                                  // temp=(ADCreading/1024*ADCREF(mV)-TOFFSET(mV))/10+TREF                                
                                  // equivalent to // temp=(ADC*TFACTOR-(TOFFSET))+TREF (no dividing)
                                  // with
                                  // TFACTOR=1.1/10.24 or VCC/10.24 or AREF/10.24
                                  // TOFFSET voltage(mV)/10 @ TREF @ 10mV/°C

  #define MCP9700                 //#define TMP36 //#define LM335 //#define DS18X20 // modèle thermomètre

  #ifdef LM335
  #define TADMUXVAL  0 | (0<<REFS1) | (1<<REFS0) | TCHECKADC     // ADVCC ref + ADC input for temp
  #define THERMO "LM335 "
  #define THN    'L'
  #define TFACTOR 0.806           // temp conversion pour LM335
  #define TOFFSET 750             // @25°
  #endif LM335
  #ifdef TMP36
  #define THERMO "TMP36 "
  #define THN    'T'
  #define TADMUXVAL  0 | (1<<REFS1) | (1<<REFS0) | TCHECKADC     // internal 1,1V ref + ADC input for temp
  #define TFACTOR 1               // temp conversion pour TMP36
  #define TOFFSET 698             // @25°
  #endif TMP36
  #ifdef MCP9700
  #define TADMUXVAL  0 | (1<<REFS1) | (1<<REFS0) | TCHECKADC     // internal 1,1V ref + ADC input for temp
  #define THERMO "MCP97 "
  #define THN    'M'
  #define TFACTOR 0.1074          // temp conversion pour MCP9700
  #define TOFFSET 75              // @25°
  #endif MCP9700


#endif // HARD_H_INCLUDED
