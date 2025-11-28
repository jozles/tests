#include "interruptTimerServer.h"
#include <Arduino.h>    

#include "SAMDueTimerInterrupt.hpp"


uint16_t attachDueInterrupt(double microseconds, timerCallback callback, const char* TimerName)
{
  DueTimerInterrupt dueTimerInterrupt = DueTimer.getAvailable(); 
  dueTimerInterrupt.attachInterruptInterval(microseconds, callback);
  uint16_t timerNumber = dueTimerInterrupt.getTimerNumber();
  
  Serial.print(TimerName); Serial.print(F(" attached to Timer(")); Serial.print(timerNumber); Serial.println(F(")"));

  return timerNumber;
}