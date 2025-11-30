#include <Arduino.h>
#include "const.h"

uint16_t sineWaveform[WFSTEPNB];

#define PI 3.1415926536

void fillSineWaveForms(){
    for(uint16_t i=0;i<WFSTEPNB/4;i++){
        sineWaveform[i]=sin(i/2048*2*PI)*32765;
        sineWaveform[1023-i]=sineWaveform[i];
        sineWaveform[i+1023]=-sineWaveform[i];
        sineWaveform[2047-i]=-sineWaveform[i];
    }
}

float freqStepCalc(float freq){
    return freq*WFSTEPNB/FSAMPLE;
}

uint16_t freqPlay(uint16_t step,uint16_t timeStamp){
    
}