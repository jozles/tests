
#include <Arduino.h> 

#define LED LED_BUILTIN

/* générateur de fréquences discrètes à partir de valeurs linéaires

  Le dac fournit des valeurs v comprises entre 0 et 2^n  (n=nombre de bits du dac)
  On décide du nombre d'octaves couvert OCTNB
  Il y aura (2^n)%OCTNB incréments par octave.
  La fréquence à chaque incrément est calculée par la formule :
    freq = 2^v soit 2^(int(v/OCTNB))+2^((v%OCTNB)/(n%OCTNB)) 
  on a donc 2 tables : les fréquences d'octaves et les ratios d'incréments 
  ainsi le calcul est minimisé 
  Quand on dispose d'une grande mémoire et que l'on est très pressé on peut faire une table avec les 2^n valeurs 

*/


#define OCTNB 11
#define INCRNB 409    // int(4096/10)=409
#define FREQ0 16.345  // pour avoir un LA à 440Hz avecc 409 incréments par octave


const uint8_t octNb = OCTNB;
float baseFreq = FREQ0;
float octFreq[octNb+1];

void fillOctFreq() { 
  for (uint8_t i = 0; i <= octNb; i++) {
    octFreq[i] = baseFreq * (1<<i); 
  }
}

void showOctFreq() 
{ 
  Serial.println("fréquences des octaves");
  uint8_t dec=4;
  for (uint16_t i = 0; i < octNb; i++ ){
    Serial.println(octFreq[i], dec);
  }
}

const uint16_t octIncrNb = 409;
float octIncr[octIncrNb];

void fillOctIncr() 
{ 
  for (uint16_t i = 0; i < octIncrNb; i++) {
    octIncr[i] = (float)(pow((float)2,(float)i/(float)octIncrNb))-1; 
  }
}

void fout(float octF,float octF_1,uint16_t incr,uint8_t dec) 
{ 
  Serial.print(octIncr[incr],dec);Serial.print("/");Serial.print(octF+(octF_1-octF)*octIncr[incr],dec);Serial.print("  ");
}                         

void showOctIncr(float octF,float octF1)
{ 
  Serial.println("ratios d'incréments d'octave");
  uint8_t dec=4;
  uint8_t step=4;
  uint16_t max=octIncrNb/step*step;
  for (uint16_t i = 0; i < max; i+=step ){
    for(uint8_t j=0;j<step;j++){fout(octF,octF1,i+j,dec);} 
    Serial.println();
  }
  if(max<octIncrNb){
    for (uint16_t i = max+1; i < octIncrNb; i++ ){
      fout(octF,octF1,i,dec);
    }
    Serial.println("\n---");
  }
}

float calcFreq(uint16_t val) 
{ 
  uint8_t oct = val / octIncrNb;
  uint16_t incr = val % octIncrNb;
  float freq = octFreq[oct] * octIncr[incr];
 return freq;
}

void setup() {
  Serial.begin(115200);Serial.println("+calcul fréquences");
  
  while(1){pinMode(LED,OUTPUT);digitalWrite(LED,HIGH);delay(50);digitalWrite(LED,LOW);delay(500);if (Serial.available()) break;} // wait for Serial to be ready
  
  fillOctFreq();
  showOctFreq();
  fillOctIncr();
  showOctIncr(0,0);
  
  uint8_t oct = 4;
  Serial.print("Exemples de fréquences (octave ");
  Serial.print(oct);Serial.println(")");
  showOctIncr(octFreq[oct],octFreq[oct+1]);
}

void loop(){}