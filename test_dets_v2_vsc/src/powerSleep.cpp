#include <Arduino.h>

#include "powerSleep.h"
#include "hard.h"

/* mécanisme
 *  
 *  la fonction uint16_t sleepPwrDown(durat) gère et effectue la mise en sommeil profond
 *  SLEEP_MODE_PWR_DOWN du MCU, et débranche tout ce qui consomme pour assurer la veille 
 *  la plus sobre possible.
 *  Selon l'argument durat, le réveil provient du watchdog timer ou du pin d'interruption 1 si durat = 0.
 *    WDT :
 *    Les durées valides effectuées par le WDT sont définies dans la fonction wdtSetup()
 *    qui initialise le registre WDTCSR pour déclencher le timer.
 *    Au réveil, la fonction wdtDisable() inhibe le WDT. 
 *    La fonction ISR(WDT_vect) est exécutée lors de l'interruption issue du WDT
 *    INT1 :
 *    La fonction int1_ISR() est exécutée lors de l'interruption,
 *    issue du pin INT1
 *    Comme le WDT est stoppé, la consommation est moindre
 *    
 *  la fonction letalSleep() n'effectue pas wdtSetup ni ne branche d'interruption du pin INT1
 *  rien ne sortira le CPU du sommeil sauf un reset physique. (à utiliser si les batteries 
 *  sont épuisées par exemple).
*/


#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

uint16_t wdtTime[]={16,32,64,125,250,500,1000,2000,4000,8000};   // durées WDT millis

uint8_t cntTest=0;              // test watchdog


ISR(WDT_vect)                   // ISR interrupt service for MPU INT WDT vector
{
}


void int1_ISR()                 // reed ISR
{
  sleep_disable();
  detachInterrupt(0);
  detachInterrupt(1);
}

void int0_ISR()                   // external timer ISR
{
  sleep_disable();
  detachInterrupt(0);
  detachInterrupt(1);
}

 
void hardwarePowerUp()
{
  bitSet(DDR_LED,BIT_LED);        //pinMode(LED,OUTPUT);
}


void hardwarePowerDown()          // every loaded port pin -> in
{
  bitClear(DDR_LED,BIT_LED);      //pinMode(LED,INPUT);
  bitClear(DDR_DONE,BIT_DONE);
  bitClear(DDR_SPICK,BIT_SPICK);
  bitClear(DDR_VCHK,BIT_VCHK);  
  bitClear(DDR_MOSI,BIT_MOSI);
  bitClear(DDR_NRFCE,BIT_NRFCE);
  bitClear(DDR_NRFPWR,BIT_NRFPWR);    
}


void wdtSetup(uint8_t durat)  // (0-9) durat>9 for external wdt on INT0 (à traiter)
{
// datasheet page 51 and 54, Watchdog Timer.


/*  MCUSR MCU status register (reset sources)(every bit cleared by writing 0 in it)
 *   WDRF reset effectué par WDT
 *   BORF ------------------ brown out detector
 *   EXTRF ----------------- pin reset
 *   PORF ------------------ power ON
 *
 *  WDTCSR watchdog timer control
 *   WDIF watchdog interrupt flag (set when int occurs with wdt configured for) (reset byu writing 1 or executing ISR(WDT_vect))
 *   WDIE watchdog interrupt enable  -> counter ovf make interrupt
 *   WDE  watchdog reset enable      -> counter ovf make reset
 *        WDE  WDIE   Mode
 *         0    0     stop (no watchdog)
 *         0    1     interrupt
 *         1    0     reset
 *         1    1     interrupt then reset (WDIE->0 lors de l'interruption, retour au mode reset)
 *       !!!! fuse WDTON forces reset mode if 0 !!!!
 *   WDCE watchdog change enable (0 write enable to WDE and prescaler update ; auto cleared after 4 cycles)
 *   WDP[3:0] prescaler 2^(0-9)*2048 divide WDT oscillator (f=128KHz p*2048=16mS)
 *
 *   wdr instruction resets timer (wdt_reset();)
 *   
 *   power down supply current for ATMEGA 328P page 594 : typically 4,5 uA at 3,3V 25°C with watchdog enabled (about 6,5 at 5V)
 *   
 *   idle supply current for ATMEGA 328P page 591 : typically labout 0,7mA at 3,3V 8MHz 
 *   
 *   
 */

// WDT prescaler - WDP3-0 bits (msec)

#define T16   0b00000000
#define T32   0b00000001
#define T64   0b00000010
#define T125  0b00000011
#define T250  0b00000100
#define T500  0b00000101
#define T1000 0b00000110
#define T2000 0b00000111
#define T4000 0b00100000
#define T8000 0b00100001

    noInterrupts();
    wdt_reset();

    MCUSR &= ~(1<<WDRF);  // pour autoriser WDE=0

#ifdef ATMEGA328  
    WDTCSR = (1<<WDCE) | (1<<WDE);    // WDCE ET WDE must be 1
                                      // to write WDP[0-3] and WDE in the following 4 cycles
    WDTCSR = (1<<WDIE) | durat;       // WDCE must be 0 ; WDE=0, WDIE=1 interrupt mode, TXXX 
#endif     

    interrupts();
}

void wdtDisable()
{
    noInterrupts();
    wdt_reset();
    MCUSR &= ~(1<<WDRF);              // to allow WDE=0
    
    WDTCSR = (1<<WDCE) | (1<<WDE);    // WDCE ET WDE must be 1 to write WDE in the following 4 cycles
    WDTCSR = 0;                       // WDE and WDIE disabled

    interrupts();                                                    
}

void lethalSleep()
{
    hardwarePowerDown();
    ADCSRA &= ~(1<<ADEN);                   // ADC shutdown    
    power_all_disable();                    // all bits set in PRR register (I/O modules clock halted)
    set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
    noInterrupts();                         // cli();
    sleep_enable();                       
#ifdef ATMEGA328
    sleep_bod_disable();                    // BOD halted if followed by sleep_cpu 
#endif
    interrupts();                           // sei();
    sleep_cpu();
}

void clearWd()
{
  
  delayMicroseconds(4);   // some time to saturate reset strobe mosfet (only 2.5V Vgs)
   // external timer : high on done pin ends high pulse -> falling edge generate low pulse on reset
   // VCHECK high shorten reset at VCC -> low pulse masked during volts reading
      bitSet(PORT_DONE,BIT_DONE);           //digitalWrite(DONE,HIGH);
      bitSet(DDR_DONE,BIT_DONE);            //pinMode(DONE,OUTPUT);                 // 100nS minimum pulse
      bitClear(PORT_DONE,BIT_DONE);         ////digitalWrite(DONE,LOW);
      bitClear(DDR_DONE,BIT_DONE);          //pinMode(DONE,INPUT);
  delayMicroseconds(8);   // some time to fill up the reset pulse capacitor low drived by 5111 

}


uint16_t sleepPwrDown(uint8_t durat)  /* *** WARNING *** hardwarePowerUp() not included to avoid multiple unusefull power on */
{                                     /* durat=0 to enable external timer (INT0) */

    hardwarePowerDown();

    ADCSRA &= ~(1<<ADEN);                   // ADC shutdown
    
    power_all_disable();                    // all bits set in PRR register (I/O modules clock halted)
    
    set_sleep_mode(SLEEP_MODE_PWR_DOWN); 

    if(durat!=0){wdtSetup(durat);}          // WDTCSR register setup for sleep with WDT int awake
    
    noInterrupts();                         // cli();

    if(durat==0){                           // external timer interrupt awaking
  /* it would have to wait here for low state on INT0 to avoid       
     possibility of falling transition between interrupts() and sleep_cpu()
     (in that case BOD is not disable ; that cause little more power wasting)
     it should not happen because no operation should take more than 1 sec 
     same issue for reed on INT1 which is a rare event (reed to greedy, no more detected by int)
     */
     
      attachInterrupt(0,int0_ISR,ISREDGE);  // external timer interrupt enable
      EIFR=bit(INTF0);                      // clr flag
    }
    //attachInterrupt(1,int1_ISR,CHANGE);   // reed interrupt enable
    //EIFR=bit(INTF1);                      // clr flag
        
    sleep_enable();                       
#ifdef ATMEGA328
    sleep_bod_disable();                    // BOD halted if followed by sleep_cpu 
#endif
    interrupts();                           // sei();
    sleep_cpu();
    sleep_disable();
    if(durat!=0){wdtDisable();}                         
    power_all_enable();                     // all bits clr in PRR register (I/O modules clock running)

    hardwarePowerUp();

    return wdtTime[durat]/10;               // not valid if durat=0...
}
