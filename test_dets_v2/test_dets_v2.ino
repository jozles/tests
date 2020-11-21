#include "powerSleep.h"
#include "hard.h"
#include "eepr.h"
#include <shutil2.h>
#include <shconst2.h>

#define VERSION "01"
#define MAC "peri_"
#define CONC "ctest"

char c='\0';
char c1='\0';
char c2='\0';
float volts=0;                           // tension alim (VCC)
float temp;
float refMiniT=747;
float refMaxiT=753;                      // référence tension étalonage th
float refMiniV=350;
float refMaxiV=410;                      // référence tension étalonage volts

uint8_t k;

Eepr eeprom;

#define CONFIGLEN 36
byte    configData[CONFIGLEN];

byte*  configVers;
float* thFactor;
float* thOffset;
float* vFactor;
float* vOffset;
byte*  macAddr;
byte*  concAddr;

char*  chexa="0123456789ABCDEFabcdef\0";

void configPrint();
char getch();
void testSleep(char mode);
float adcRead(uint8_t admuxval,float factor, uint16_t offset, uint8_t ref,uint8_t dly);      // dly=1 if ADC halted
void getVT();
void initConf();
void testCrc();

void setup() {  
  
  bitClear(PORT_VCHK,BIT_VCHK);               //digitalWrite(VCHECK,VCHECKHL);
  bitSet(DDR_VCHK,BIT_VCHK);                //pinMode(VCHECK,OUTPUT);  

  Serial.begin(115200);delay(100);
  Serial.println("start");delay(10);

  //testCrc();

  initConf();
}

void loop(){  

  Serial.println("\nblink avec delay       O k  S kip  T calibration thermo  V calibration volts  E eprom");
  pinMode(LED,OUTPUT);
  while(!Serial.available()){digitalWrite(LED,HIGH);delay(500);digitalWrite(LED,LOW);delay(500);}
  c=Serial.read();Serial.println(c);

  switch(c){
    case 'O':
      Serial.print("blink avec sleep interne ");delay(10);testSleep('I');
      Serial.print("blink avec sleep externe ");delay(10);testSleep('E');
      break;

    case 'S':
      getVT();
      Serial.print(" voltage ");Serial.print(volts);getVT();Serial.print(" voltage ");Serial.print(volts);
      Serial.print(" thFactor=");Serial.print((float)*thFactor*1000);
      Serial.print(" temp ");Serial.println(temp);
      break;

    case 'T':
      bitSet(PORT_VCHK,BIT_VCHK);bitSet(DDR_VCHK,BIT_VCHK);
      delay(1);

      temp=adcRead(TADMUXVAL,1,0,0,20);
      Serial.print(" adcRead() Th ");Serial.println(temp);
      
      Serial.print(" valeur référence (");
      for(int k=0;k<(refMaxiT-refMiniT);k++){
        Serial.print(k);Serial.print("=.");Serial.print((int)(refMiniT+k));if(k<(refMaxiT-refMiniT-1)){Serial.print(" ");}
      }

      Serial.print(")? ");
      c=getch();while(c>'5' || c<'0'){c=getch();};
      k=(uint8_t)c-(uint8_t)'0';
      
      temp=adcRead(TADMUXVAL,1,0,0,20);
      *thFactor=(float)((float)(refMiniT+k)/(float)temp)/10;
      Serial.print(" adcRead() Th ");Serial.print(temp);Serial.print("  thFactor=");Serial.print(*thFactor*1000);
      getVT();Serial.print(" temp=");Serial.println(temp);
      bitClear(PORT_VCHK,BIT_VCHK);

      break;

    case 'V':
      bitSet(PORT_VCHK,BIT_VCHK);bitSet(DDR_VCHK,BIT_VCHK);
      delay(1);

      volts=adcRead(VADMUXVAL,1,0,0,20);
      Serial.print(" adcRead() V ");Serial.println(volts);
      
      Serial.print(" valeur référence (");
      for(int k=0;k<(refMaxiV/10-refMiniV/10+1);k++){
        Serial.print(k);Serial.print("=");Serial.print((refMiniV/10+k)/10);if(k<(refMaxiV/10-refMiniV/10)){Serial.print(" ");}
      }

      Serial.print(")? ");
      c=getch();while(c>'6' || c<'0'){c=getch();};
      k=(uint8_t)c-(uint8_t)'0';
      
      volts=adcRead(VADMUXVAL,1,0,0,20);
      *vFactor=(float)((float)(((refMiniV/10)+k)*10)/(float)volts)/100;
      Serial.print(" adcRead() V ");Serial.print(volts);Serial.print("   vFactor=");Serial.print(*vFactor*10000);
      getVT();Serial.print("  volt=");Serial.println(volts);
      bitClear(PORT_VCHK,BIT_VCHK);

      break;
      
    case 'E':
      Serial.print("Eeprom ");configPrint();
      Serial.print("L oad  I nit  P eri nb  S kip ");
      
        c1=getch();while(c1=='\0'){c1=getch();}
        switch(c1){
          case 'L':
            if(!eeprom.load(configData,CONFIGLEN)){Serial.println("****KO******");}
            configPrint();
            break;
          case 'I':
            configPrint();
            eeprom.store(configData,CONFIGLEN);
            configPrint();
            break;
          case 'P':
            Serial.print(" numéro ? ");
            c2=getch();while(c2=='\0'){c2=getch();}
            macAddr[4]=c2;macAddr[5]='\0';
            Serial.println((char*)macAddr);
            break;
          case 'S':
            break;
          default:break;
        }
      break;
    
    default:break;
  }
}

void testSleep(char mode)
{
  c='\0';
  while(c!='O'){
    switch (mode){
      case 'I':sleepPwrDown(T2000);break;
      case 'E':sleepPwrDown(0);break;
      default: break;
    }      
    hardwarePowerUp();
    uint8_t nb=2;if(mode=='E'){nb=3;}
    for(uint8_t i=0;i<nb;i++){
      digitalWrite(LED,HIGH);delay(200); // sleepPowerDown met le pin Led en entrée
      digitalWrite(LED,LOW);
      sleepPwrDown(T500);
      hardwarePowerUp();
    }

    Serial.begin(115200);delay(100);
    Serial.print(" 2 sec pour O k ");
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
  
  volts=adcRead(VADMUXVAL,*vFactor,0,0,1);
  delayMicroseconds(1000);                  // MCP9700 stabilize
  temp=adcRead(TADMUXVAL,*thFactor,*thOffset,0,1);

/* step 0.25 ***  
  uint16_t temp0=((int)temp)*100,temp1=(int)(temp*100); 
  if((temp1-temp0)>=12 && (temp1-temp0)<38){temp0+=25;}
  else if((temp1-temp0)>=38 && (temp1-temp0)<63){temp0+=50;}
  else if((temp1-temp0)>=63 && (temp1-temp0)<88){temp0+=75;}
  else if((temp1-temp0)>=88){temp0+=100;}
  temp=(float)temp0/100;
*/
/* step 0.1 ***/

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
  vFactor=(float*)temp;                           
  temp +=sizeof(float);
  vOffset=(float*)temp;
  temp +=sizeof(float);
  macAddr=(byte*)temp;
  temp +=6;
  concAddr=(byte*)temp;
  temp +=5;

  byte* configEndOfRecord=(byte*)temp;      // doit être le dernier !!!

  long configLength=(long)configEndOfRecord-(long)configBegOfRecord+1;  
  Serial.print("CONFIGLEN=");Serial.print(CONFIGLEN);Serial.print("/");Serial.println(configLength);
  delay(10);if(configLength!=CONFIGLEN) {ledblink(BCODECONFIGRECLEN);}

  memcpy(configVers,VERSION,2);
  memcpy(macAddr,MAC,6);
  strncpy((char*)concAddr,CONC,5);
  *thFactor=0.1071;
  *thOffset=50;
  *vFactor=0.0057;
  *vOffset=0;
}

void configPrint()
{
    uint16_t configLen;memcpy(&configLen,configData+EEPRLENGTH,2);
    char configVers[3];memcpy(configVers,configData+EEPRVERS,2);configVers[3]='\0';
    Serial.print("crc ");dumpfield((char*)configData,4);Serial.print(" len ");Serial.print(configLen);Serial.print(" V ");Serial.println(configVers);
    char buf[7];memcpy(buf,concAddr,5);buf[5]='\0';
    Serial.print("MAC  ");dumpstr((char*)macAddr,6);Serial.print("CONC ");dumpstr((char*)concAddr,5);

    Serial.print("  thFactor=");Serial.print(*thFactor*10000);Serial.print("  thOffset=");Serial.print(*thOffset);   
    Serial.print("   vFactor=");Serial.print(*vFactor*10000);Serial.print("   vOffset=");Serial.println(*vOffset);   
}



uint32_t calcCrc32b(byte* data,uint16_t len)
{
  uint32_t crc32=0;
  uint32_t reg;
  uint16_t i;
  uint8_t j,k;

  for(i=0;i<len;i++){
    reg=(uint32_t)data[i];

    for(j=0;j<8;j++){
        k=(crc32^reg)&0x00000001;
        crc32=crc32>>1;
        if(k==1){crc32=crc32^0x82608EDB;} //0x04C11DB7 rigth rotated polynom
        reg=reg>>1;
    }
  }
  return crc32;
}

void testCrc()
{
unsigned long t1;
unsigned long t2;
char chaine[]="bonjour les zozos de l'informatique 1234567890DFGTRE";
Serial.print("chaine =");Serial.print(chaine);Serial.println(" longueur 50");
t1=micros();
uint32_t crc1=eeprom.calcCrc32((byte*)chaine,48);
Serial.print("crc1=");Serial.print(crc1);Serial.print("  t1=");Serial.println(micros()-t1);
t2=micros();
uint32_t crc2=calcCrc32b((byte*)chaine,48);
Serial.print("crc2=");Serial.print(crc2);Serial.print("  t2=");Serial.println(micros()-t2);
while(1){};
}
