#include <Arduino.h>

#define ONBLINK 5;
#define OFFBLINK 200;
#define FASTBLINK 1000;

long blinkTime=millis();
int blinkPerOn=ONBLINK;
int blinkPerOff=OFFBLINK;
int blinkPerFast=FASTBLINK;
int nbBlink=4;

char test[1600];

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(112500);
  delay(1000);
  Serial.println("Due/Nucleo ready vcspio");
  Serial.println("xxx.. valeur numérique \n vxx.. perfast ; fxx.. peroff ; nxx.. peron ; bxx.. nbblink");

  long t0=micros();
  for(int n=0;n<10000;n++){
    test[n%1000]=(char)n%256;
    if(n%1000==0){
      Serial.print(n);Serial.print(" ");Serial.println(micros()-t0);}
  }
  long t1=micros();
  Serial.print("test boucle=");Serial.println((t1-t0));


}

void ledblink(int nbblink)
{
  for(int b=0;b<nbblink;b++){
    digitalWrite(LED_BUILTIN, HIGH);   
    delay(blinkPerOn);
    digitalWrite(LED_BUILTIN, LOW);
    delay(blinkPerOff);
  }
}

int numinp()
{
  int result=0;
  char c='0';
  
    while(c!='\n'){
      if(Serial.available()){
        c=Serial.read();Serial.print(c);
        if(c<='9'&&c>='0'){
          result*=10;result+=(c-48);
        }
      }
    }
    
    return result; 
}

void loop() {

char c;

    ledblink(nbBlink);
    delay(blinkPerFast);

    if(Serial.available()){
      c=Serial.read();
      Serial.print(c);
      switch(c){
        case 'v':blinkPerFast=numinp();break;
        case 'n':blinkPerOn=numinp();break;
        case 'f':blinkPerOff=numinp();break;
        case 'b':nbBlink=numinp();break;
        default:break;
      }
      Serial.print("v=");Serial.println(blinkPerFast);
      Serial.print("n=");Serial.println(blinkPerOn);
      Serial.print("f=");Serial.println(blinkPerOff);
      Serial.print("b=");Serial.println(nbBlink);
      Serial.println();
    }
}
