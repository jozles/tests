
#include <DueFlashStorage.h>

DueFlashStorage dFS;

#define V1_1 1
#define V1_2 2
#define REDVERS V1_2

#define LED 13
#define LEDTRIG 3    // wd trig
#define ONTRIG HIGH
#define CS_SD 4
#define ONSD LOW
#define CS_NRF 8
#define ONNRF LOW
#define POWER 5
#define PUSHB 2
#define TEST  7

#if REDVERS==V1_1
  #define POWON LOW
  #define POWOFF HIGH
#endif

#if REDVERS==V1_2
  #define POWON HIGH
  #define POWOFF LOW
#endif

unsigned long dly=0;
uint8_t pushb=0;
char t[]="LH";

void blk(uint8_t led,uint8_t level);

void setup() {
  Serial.begin(115200);
  Serial.print("\nstart...");

  uint8_t eread=0,ewrite;

  eread=dFS.read(0);
  Serial.print("\neread=");Serial.println(eread,HEX);
  
  ewrite=0xC4;
  dFS.write(0,ewrite);
  eread=dFS.read(0);
  dFS.write(0,0x56);

  Serial.print("ewrite=");Serial.println(ewrite,HEX);
  Serial.print("eread=");Serial.println(eread,HEX);
  
// tout OFF
  pinMode(LED,OUTPUT);digitalWrite(LED,LOW);
  pinMode(POWER,OUTPUT);digitalWrite(POWER,POWOFF);
  pinMode(CS_SD,OUTPUT);digitalWrite(CS_SD,!ONSD);
  pinMode(CS_NRF,OUTPUT);digitalWrite(CS_NRF,!ONNRF);  
  pinMode(LEDTRIG,OUTPUT);digitalWrite(LEDTRIG,!ONTRIG);
  delay(2000);
  
// tout ON  
  digitalWrite(LED,HIGH);
  digitalWrite(POWER,POWON);
  digitalWrite(CS_SD,ONSD);
  digitalWrite(CS_NRF,ONNRF);
  digitalWrite(LEDTRIG,ONTRIG);
  delay(2000);
// tout OFF
  digitalWrite(LED,LOW);
  digitalWrite(POWER,POWOFF);
  digitalWrite(CS_SD,!ONSD);
  digitalWrite(CS_NRF,!ONNRF);
  digitalWrite(LEDTRIG,!ONTRIG);
  delay(2000);

Serial.println("rdy...");
  
/*pinMode(LED,OUTPUT);digitalWrite(LED,LOW);
pinMode(LEDTRIG,OUTPUT);digitalWrite(LEDTRIG,LOW);
pinMode(CS_SD,OUTPUT);digitalWrite(CS_SD,HIGH);
pinMode(POWON,OUTPUT);digitalWrite(POWON,HIGH);
*/
pinMode(PUSHB,INPUT_PULLUP);

digitalWrite(LED,HIGH);Serial.println("U");delay(500);
digitalWrite(LEDTRIG,ONTRIG);Serial.println("J");delay(500);
digitalWrite(POWER,POWON);Serial.println("V");delay(500);
digitalWrite(CS_SD,ONSD);Serial.println("R");
digitalWrite(CS_NRF,ONNRF);Serial.println("R");delay(500);

digitalWrite(LED,LOW);delay(500);
digitalWrite(LEDTRIG,!ONTRIG);delay(500);
digitalWrite(CS_SD,!ONSD);
digitalWrite(CS_NRF,!ONNRF);delay(500);
digitalWrite(POWER,POWOFF);delay(500);

Serial.println(" init done");delay(100);

blk(LEDTRIG,HIGH);blk(LEDTRIG,HIGH);blk(LEDTRIG,HIGH);
digitalWrite(POWER,POWON);
blk(CS_SD,ONSD);blk(CS_SD,ONSD);blk(CS_SD,ONSD);
blk(CS_SD,ONNRF);blk(CS_SD,ONNRF);blk(CS_SD,ONNRF);

}

void loop() 
{
  dly++;
  Serial.print("6 sec mini ");Serial.println(((float)dly)/10);

  blk(LED,HIGH);
  blk(LEDTRIG,ONTRIG);
  blk(CS_SD,ONSD);
  blk(CS_NRF,ONNRF);

  for(uint8_t i=0;i<dly;i++){
    
    if(digitalRead(PUSHB)==LOW && pushb==1){
      pushb=0;
      Serial.println("PUSH ");}
      
    if(digitalRead(PUSHB)==HIGH){pushb=1;}
    
    delay(100);
  }
}

void blk(uint8_t led,uint8_t level)
{
  for(uint8_t i=0;i<2;i++){
    digitalWrite(led,level);delay(10);digitalWrite(led,!level);delay(100);}
}
