#ifndef _ITS_H_
#define _ITS_H_ 

#include "SAMDueTimerInterrupt.hpp"

uint16_t attachDueInterrupt(double microseconds, timerCallback callback, const char* TimerName);

#endif  //_ITS_H_