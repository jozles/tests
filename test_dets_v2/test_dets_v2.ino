#include "powerSleep.h"
#include "hard.h"
#include "eepr.h"
#include <shutil2.h>
#include <shconst2.h>

char c='\0';
float volts=0;                           // tension alim (VCC)
float temp;

Eepr eeprom;

#define CONFIGLEN 64
byte    configData[CONFIGLEN];

byte*  configVers;
float* thFactor;
float* thOffset;
byte*  macAddr;
byte*  concAddr;

void getVT();
void initConf();

void setup() {
  bitClear(PORT_VCHK,BIT_VCHK);               //digitalWrite(VCHECK,VCHECKHL);
  bitSet(DDR_VCHK,BIT_VCHK);                //pinMode(VCHECK,OUTPUT);  

  Serial.begin(115200);delay(100);
  Serial.println("start");delay(10);

  initConf();
}

void loop(){  

  Serial.println("blink avec delay       V alider  S kip  T power on thermo  E eprom");
  pinMode(LED,OUTPUT);
  while(!Serial.available()){digitalWrite(LED,HIGH);delay(500);digitalWrite(LED,LOW);delay(500);}
  c=Serial.read();Serial.println(c);

  switch(c){
    case 'V':
      Serial.print("blink avec sleep interne ");delay(10);testSleep('I');
      Serial.print("blink avec sleep externe ");delay(10);testSleep('E');
      break;

    case 'S':
      getVT();
      Serial.print(" voltage ");Serial.print(volts);getVT();Serial.print(" voltage ");Serial.println(volts);
      Serial.print(" th ADC ");Serial.println(temp);
      break;

    case 'T':
      bitSet(PORT_VCHK,BIT_VCHK);bitSet(DDR_VCHK,BIT_VCHK);delay(10);
      Serial.println("une touche sort");
      
      temp=analogRead(A3);Serial.print(" analogRead() Th ");Serial.println(temp);
      delay(10);
      temp=adcRead(TADMUXVAL,1,0,0,20);Serial.print(" adcRead() Th ");Serial.println(temp);
      while(getch()=='\0'){};
      temp=adcRead(TADMUXVAL,1,0,0,20);Serial.print(" adcRead() Th ");Serial.println(temp);
      bitClear(PORT_VCHK,BIT_VCHK);
      break;
      
    case 'E':
      Serial.print("Eeprom ");
      if(!eeprom.load(configData,CONFIGLEN)){
        Serial.print("ko -- I nit  S kip");
        c=getch();
        switch(c){
          case 'I':break;
          case 'S':break;
        }
      }
      else {
        configPrint();
      }
      break;  
    default:break;
  }
}

void testSleep(char mode)
{
  c='\0';
  while(c!='V'){
    switch (mode){
      case 'I':sleepPwrDown(T2000);break;
      case 'E':sleepPwrDown(0);break;
      default: break;
    }      
    hardwarePowerUp();
    uint8_t nb=2;if(mode=='E'){nb=3;}
    for(uint8_t i=0;i<nb;i++){
      digitalWrite(LED,HIGH);delay(100);
      digitalWrite(LED,LOW);
      sleepPwrDown(T500);
      hardwarePowerUp();
    }

    Serial.begin(115200);delay(100);
    Serial.print(" 2 sec pour V alider ");
    unsigned long tbeg=millis();
    c='\0';while((c=='\0') && (millis()-tbeg)<2000){c=getch();};
  }
}

char getch()
{
    char c='\0';
    if(Serial.available()){
      c=Serial.read();
      Serial.println((char)c);delay(1);}
    return c;
}

void getVT()                     // get unregulated voltage and reset watchdog for external timer period 
{
  unsigned long t=micros();

  bitSet(PORT_VCHK,BIT_VCHK);               //digitalWrite(VCHECK,VCHECKHL);
  bitSet(DDR_VCHK,BIT_VCHK);                //pinMode(VCHECK,OUTPUT);  
  
  volts=adcRead(VADMUXVAL,VFACTOR,0,0,1);
  delayMicroseconds(1000);                  // MCP9700 stabilize
  temp=adcRead(TADMUXVAL,1,0,0,20);
//  Serial.print("temp 1mS=");Serial.print(temp);
//  delayMicroseconds(10000);                 // MCP9700 stabilize
//  temp=adcRead(TADMUXVAL,1,0,0,0);
//  Serial.print(" temp 10mS=");Serial.println(temp);

/* step 0.25 ***  
  uint16_t temp0=((int)temp)*100,temp1=(int)(temp*100); 
  if((temp1-temp0)>=12 && (temp1-temp0)<38){temp0+=25;}
  else if((temp1-temp0)>=38 && (temp1-temp0)<63){temp0+=50;}
  else if((temp1-temp0)>=63 && (temp1-temp0)<88){temp0+=75;}
  else if((temp1-temp0)>=88){temp0+=100;}
  temp=(float)temp0/100;
*/
/* step 0.1 ***/
//  temp=(float)((int)(temp*10))/10;

  bitClear(PORT_VCHK,BIT_VCHK);  
  bitClear(DDR_VCHK,BIT_VCHK);      //pinMode(VCHECK,INPUT);              // reset pulse strobe released 

  ADCSRA &= ~(1<<ADEN);                   // ADC shutdown for clean next voltage measurement
}

float adcRead(uint8_t admuxval,float factor, uint16_t offset, uint8_t ref,uint8_t dly)      // dly=1 if ADC halted
{
    uint16_t a=0;
    
    ADCSRA |= (1<<ADEN);                    // ADC enable to write ADMUX
    ADMUX   = admuxval;
    ADCSRA  = 0 | (1<<ADEN) | (1<<ADSC) | (1<<ADIF) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);   // ADC enable + start conversion + prescaler /16

    delayMicroseconds(40+dly*48);           // ok with /16 prescaler @8MHz
   
    a=ADCL;
    a+=ADCH*256;
//  Serial.println();Serial.print(" a=");Serial.print(a);Serial.print("  factor=");Serial.println(factor*100);
    return (float)(a*factor-(offset))+ref;
}

void initConf()
{
  byte* temp=(byte*)configData;

  byte* configBegOfRecord=(byte*)temp;         // doit être le premier !!!

  configVers=temp+EEPRVERS;
  temp += EEPRHEADERLENGTH;
  thFactor=(float*)temp;                           
  temp +=sizeof(float);
  thOffset=(float*)temp;
  temp +=sizeof(float);
  macAddr=(byte*)temp;
  temp +=6;
  concAddr=(byte*)temp;
  temp +=5;

  byte* filler;
  filler=(byte*)temp;
  temp +=33;
  
  byte* configEndOfRecord=(byte*)temp;      // doit être le dernier !!!

  long configLength=(long)configEndOfRecord-(long)configBegOfRecord+1;  
  Serial.print("CONFIGLEN=");Serial.print(CONFIGLEN);Serial.print("/");Serial.println(configLength);
  delay(10);if(configLength!=CONFIGLEN) {ledblink(BCODECONFIGRECLEN);}

  *thFactor=0.0976;
  *thOffset=-50;
}

void configPrint()
{
    uint16_t configLen;memcpy(&configLen,configData+EEPRLENGTH,2);
    char configVers[3];memcpy(configVers,configData+EEPRVERS,2);configVers[3]='\0';
    Serial.print("crc ");dumpfield((char*)configData,4);Serial.print(" len ");Serial.print(configLen);Serial.print(" V ");Serial.println(configVers);
}
