#include <Arduino.h>

#define MACHINE_328

//#include "hard.h"
#include "radio_const.h"
#include "lpavr_powerSleep.h"
#include "lpavr_util.h"
#include "eepr.h"
#include <shutil2.h>
#include <shconst2.h>

#define TVERSION "03"
#define MAC "peri_"
#define CONC "SHCO3"  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<< configurer (ctest intérieur, dtest extérieur)
#define CONCNAMELEN 5

#define ADDR_LENGTH 5

char c='\0';
char c1='\0';
char c2='\0';
float voltage=0;                           // tension alim (VCC)
float temp;
float refMiniT=747;
float refMaxiT=753;                      // référence tension étalonnage th
float refMiniV=350;
float refMaxiV=410;                      // référence tension étalonage voltage

uint8_t k;

Eepr eeprom;

#define CHANNEL0    120   // radio channel conc0
#define CHANNEL1    110   // radio channel conc1
#define CHANNEL2    100   // radio channel conc2
#define CHANNEL3    90    // radio channel conc3

uint8_t channelTable[]={CHANNEL0,CHANNEL1,CHANNEL2,CHANNEL3};   // canal vs N° conc

#define CONFIGLEN 75 // 37 version 01 // 38 version 02 // 75 version 03
byte    configData[CONFIGLEN];

byte*  configVers;
float* thFactor;
float* thOffset;
float* vFactor;
float* vOffset;
byte*  macAddr;
byte*  concAddr;
uint8_t* numConc;
uint8_t* concChannel;
uint8_t* concSpeed;
uint8_t* concPeriParams;

bool   pgAuto=false;    // si true pas de saisie, paramétrage automatique ; valoriser kt=3 c='T', kv=4 c='V', c='E' c1='I' c2=n° perif, c='E' c1='L' ;
int    pg=-1;               // pointeur fonction en cours en mode auto
#define NBPG 5
char   cpg[]= {"TVPEE"};
char   c1pg[]={"   IL"};
char   c2pg='g';            // ******************** numéro périf *********************
uint8_t kv=5,kt=3;

//const char*  chexa="0123456789abcdef\0";

bool   extTimer;
unsigned long t_on=0;

void configPrint();
char getch();
char getch(bool cr);
void testSleep(char mode);
float adcRead(uint8_t admuxval,float factor, uint16_t offset, uint8_t ref,uint8_t dly);      // dly=1 if ADC halted
void getVT();
void initConf();
void testCrc();
void testSleepPwrDown();
void int_ISR()
{
  extTimer=true;
  //Serial.println("int_ISR");
}

void blk(uint16_t durat)
{
  bitSet(DDR_LED,BIT_LED);bitSet(PORT_LED,BIT_LED);
  delay(durat);
  bitClear(PORT_LED,BIT_LED);bitClear(DDR_LED,BIT_LED);
}

 
void hardwarePowerUp()
{
  bitSet(DDR_LED,BIT_LED);        //pinMode(LED,OUTPUT);
}

void hardwarePowerDown_()          // every loaded port pin -> in
{
  //bitClear(DDR_LED,BIT_LED);      //pinMode(LED,INPUT);
  bitClear(DDR_DONE,BIT_DONE);
  bitClear(DDR_CLK,BIT_CLK);
  //bitClear(DDR_VCHK,BIT_VCHK);  
  bitClear(DDR_MOSI,BIT_MOSI);
  bitClear(DDR_CE,BIT_CE);
  bitClear(DDR_RPOW,BIT_RPOW);
  bitClear(DDR_REED,BIT_REED);            //pinMode(REED,INPUT);    
}

void setup() {  
  
  bitClear(PORT_VCHK,BIT_VCHK);               //digitalWrite(VCHECK,VCHECKHL);
  bitSet(DDR_VCHK,BIT_VCHK);                  //pinMode(VCHECK,OUTPUT);  

  Serial.begin(115200);delay(100);
  if(pgAuto){
    Serial.print("mode automatique ");
    Serial.print(NBPG);Serial.print(" ");Serial.print(cpg);Serial.print(" kv=");Serial.print(kv);Serial.print(" kt=");Serial.print(kt);Serial.print(" c1=");Serial.print(c1pg);Serial.print(" c2=");Serial.print(c2pg);Serial.print(" ");
    Serial.println();
  }
  for(uint8_t i=0;i<5;i++){blk(20);delay(300);}
  Serial.print("_\n>>> start (patienter, sleepPowerDown en test)");delay(10);

//  testSleepPwrDown();
//  Serial.println(" sleepPwrDown ok");delay(10);

  sleepPwrDown(0);                        // wait for interrupt from external timer to reach beginning of period

  t_on=millis();
  blk(500);                             // 1 blk 500mS - external timer calibration begin (100mS blk)
  
  attachInterrupt(0,int_ISR,ISREDGE);     // external timer interrupt
  EIFR=bit(INTF0);                        // clr flag
  while(!extTimer){delay(1);}             // évite le blocage à la fin ... ???  
  detachInterrupt(0);

  float period=(float)(millis()-t_on)/1000;
  Serial.print(" sleepPwrDown ok ");blk(500);                       // 1 blk 500mS - external timer calibration end
  Serial.print(" period ");Serial.print((int)(period*1000));Serial.println("mSec "); // external timer period


  //testCrc();

  adcRead(INADMUXVAL,1,0,0,1); // réveil ADC

  initConf();
}

void loop(){  
  Serial.println("\nrégler le terminal sur \"pas de fin de ligne - et patienter à chaque saisie\" ");
  Serial.println("blink avec delay       O k  S kip  T calibration thermo  V calibration volts  P perif nb  C consos  X tempo  E eprom");

  if(!pgAuto){
    while(!Serial.available()){bitSet(DDR_LED,BIT_LED);bitSet(PORT_LED,BIT_LED);
      delay(2000);
      //sleepPwrDown(T2000);
      bitClear(PORT_LED,BIT_LED);bitClear(DDR_LED,BIT_LED);
      delay(2000);
      //sleepPwrDown(T2000);  
    }
    c=Serial.read();}
  else {pg++;if(pg>=NBPG){Serial.print("\nterminé");while(1){};}c=cpg[pg];c1=c1pg[pg];c2=c2pg;}

  Serial.println(c);
  
  switch(c){
    case 'O':
      Serial.print("blink avec sleep interne ");delay(10);testSleep('I');
      Serial.print("blink avec sleep externe ");delay(10);testSleep('E');
      break;

    case 'S':
      getVT();
      Serial.print(" voltage ");Serial.print(voltage);getVT();Serial.print(" voltage ");Serial.print(voltage);
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
      if(!pgAuto){
        c=getch();while(c>'5' || c<'0'){c=getch();};
        kt=(uint8_t)c-(uint8_t)'0';
      }

      Serial.println(kt);
      temp=adcRead(TADMUXVAL,1,0,0,20);
      *thFactor=(float)((float)(refMiniT+kt)/(float)temp)/10;
      Serial.print(" adcRead() Th ");Serial.print(temp);Serial.print("  thFactor=");Serial.print(*thFactor*10000);
      getVT();Serial.print(" temp=");Serial.println(temp);
      bitClear(PORT_VCHK,BIT_VCHK);

      break;

    case 'V':
    {
      bitSet(DDR_VCHK,BIT_VCHK);bitSet(PORT_VCHK,BIT_VCHK);
      delay(1);

      voltage=adcRead(VADMUXVAL,1,0,0,20);
      Serial.print(" adcRead(7) V ");Serial.println(voltage);

      float intThSensor=adcRead(INADMUXVAL,1,0,0,20);     
      float intTh=intThSensor*1100/1024-242-45;                               // see datasheet page 247 
      Serial.print(" adcRead(int th sensor) ");Serial.print(intThSensor);Serial.print(" internal temp =");Serial.println(intTh);
      
      Serial.print(" valeur référence (");
      for(int k=0;k<(refMaxiV/10-refMiniV/10+1);k++){
        Serial.print(k);Serial.print("=");Serial.print((refMiniV/10+k)/10);if(k<(refMaxiV/10-refMiniV/10)){Serial.print(" ");}
      }

      Serial.print(") ? ");
      if(!pgAuto){
        c=getch();while(c>'6' || c<'0'){c=getch();};
        kv=(uint8_t)c-(uint8_t)'0';
      }

      Serial.println(kv);
      voltage=adcRead(VADMUXVAL,1,0,0,20);
      *vFactor=(float)((float)(((refMiniV/10)+kv)*10)/(float)voltage)/100;
      Serial.print(" adcRead() V ");Serial.print(voltage);Serial.print("   vFactor=");Serial.print(*vFactor*10000);
      getVT();Serial.print("  volt=");Serial.println(voltage);
      bitClear(PORT_VCHK,BIT_VCHK);
    }
      break;

    case 'P':
    {
      Serial.print(" numéro perif ? ");
      if(!pgAuto){c2=getch();while(c2=='\0'){c2=getch();}}
      Serial.print(c2);Serial.print(" ");
      macAddr[4]=c2;macAddr[5]='\0';
      Serial.println((char*)macAddr);

      Serial.print(" numéro concentrateur (0-3)? ");
      if(!pgAuto){c2=getch();while(c2=='\0' || c2<'0' || c2>'3' ){c2=getch();}}
      Serial.print(c2);Serial.print(" ");
      *numConc=c2-48;
      Serial.println(*numConc);

      Serial.print(" nom concentrateur ? (ctest,SHCO2 etc) ");
      char concName[ADDR_LENGTH+1];
      memset(concName,0x00,ADDR_LENGTH+1);
      uint8_t k=0;
      while(k<ADDR_LENGTH-1){
        concName[k]=getch(false);
        if(concName[k]!='\0'){k++;}
      }
      //for(uint8_t c=0;ADDR_LENGTH;c++){concName[c]=getch(false);}
      strncpy((char*)concAddr,concName,ADDR_LENGTH);
      concAddr[ADDR_LENGTH-1]=*numConc+48;
      Serial.println(); Serial.print("concAddr:");Serial.println((char*)concAddr);
    }
      break;

    case 'C':
      Serial.println("Conso (brancher PPK / débrancher Serial après Start)");
      Serial.println("wait_tpl,sleepPwrDown(0),sleepPwrDown(500),sleepPowerDown(500)+led,sleepPowerDown(500)+pwr_th,Powon_Nrf(100)");
      Serial.println("S start (rebrancher Serial et reset pour sortir)");
      c='\0';
      while(c!='S' && c!='s'){while(!Serial.available()){};c=Serial.read();}Serial.println();delay(1);

      while(1){
        sleepPwrDown(0);sleepPwrDown(T500);digitalWrite(LED,HIGH);sleepPwrDown(T500);digitalWrite(LED,LOW);
        bitSet(DDR_VCHK,BIT_VCHK);bitSet(PORT_VCHK,BIT_VCHK);sleepPwrDown(T500);bitClear(PORT_VCHK,BIT_VCHK);
        bitSet(DDR_RPOW,BIT_RPOW);bitClear(PORT_RPOW,BIT_RPOW);delay(100);bitSet(PORT_RPOW,BIT_RPOW);
      } 
      break;
      
    case 'X':{
      k=8;
      int32_t timings[]={8000,4000,2000,1000,500,250,125,64,32};
      Serial.println("check TXXX (reset pour sortir ; pulse sur 5)");delay(5);
      while(1){
        k--;
        Serial.println(timings[k]+1);
        blk(4);marker(MARKER);
        sleepDly(timings[k]+1);
        //sleepDly(2001);
        if(k==0){k=8;markerL(MARKER);}
        else {marker(MARKER);}
      }
    }break;

    case 'E':
      Serial.print("Eeprom ");configPrint();
      Serial.print("L oad  R ecord  S kip ");
      
        if(!pgAuto){c1=getch();while(c1=='\0'){c1=getch();}}
        //Serial.println(c1);
        switch(c1){
          case 'L':
            if(!eeprom.load(configData,CONFIGLEN)){Serial.println("****KO******");}
            configPrint();
            break;
          case 'R':
            *concChannel=channelTable[*numConc];
            *concSpeed=0;
            *concPeriParams=1;
            memcpy(configVers,TVERSION,2);
            configPrint();
            eeprom.store(configData,CONFIGLEN);
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
      case 'I':sleepPwrDown(T4000);break;
      case 'E':sleepPwrDown(0);break;
      default: break;
    }      
    hardwarePowerUp();
    uint8_t nb=2;if(mode=='E'){nb=3;}
    for(uint8_t i=0;i<nb;i++){
      bitSet(DDR_LED,BIT_LED);bitSet(PORT_LED,BIT_LED);delay(200); // sleepPowerDown met le pin Led en entrée
      bitClear(PORT_LED,BIT_LED);bitClear(DDR_LED,BIT_LED);
      sleepPwrDown(T250);
      hardwarePowerUp();
    }

    Serial.begin(115200);delay(100);
    Serial.print(" 2 sec pour O k ");
    unsigned long tbeg=millis();
    c='\0';while((c=='\0') && (millis()-tbeg)<2000){c=getch();};
  }
}

char getch(bool cr)
{
    char c='\0';
    if(Serial.available()){
      c=Serial.read();
      Serial.print((char)c);delay(1);
      if(cr){Serial.println();delay(1);}
    }
    return c;
}

char getch()
{return getch(true);}

void getVT()                     // get unregulated voltage and reset watchdog for external timer period 
{

  bitSet(PORT_VCHK,BIT_VCHK);               //digitalWrite(VCHECK,VCHECKHL);
  bitSet(DDR_VCHK,BIT_VCHK);                //pinMode(VCHECK,OUTPUT);  
  
  voltage=adcRead(VADMUXVAL,*vFactor,0,0,1);
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

/*float adcRead(uint8_t admuxval,float factor, uint16_t offset, uint8_t ref,uint8_t dly)      // dly=1 if ADC halted
{
    uint16_t a=0;
    
    ADCSRA |= (1<<ADEN);                    // ADC enable to write ADMUX
    ADMUX   = admuxval;
    ADCSRA  = 0 | (1<<ADEN) | (1<<ADSC) | (1<<ADIF) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);   // ADC enable + start conversion + prescaler /16

    delayMicroseconds(40+dly*48);           // ok with /16 prescaler @8MHz
   
    a=ADCL;
    a+=ADCH*256;

    return (float)(a*factor-(offset))+ref;
}*/

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
  temp +=ADDR_LENGTH+1;
  numConc=(uint8_t*)temp;
  temp +=sizeof(uint8_t);
  concChannel=(uint8_t*)temp;
  temp +=sizeof(uint8_t);
  concSpeed=(uint8_t*)temp;
  temp +=sizeof(uint8_t);
  concPeriParams=(uint8_t*)temp;
  temp +=sizeof(uint8_t);

  temp+=31;                   // dispo

  byte* configEndOfRecord=(byte*)temp;      // doit être le dernier !!!

  long configLength=(long)configEndOfRecord-(long)configBegOfRecord+1;  
  Serial.print("CONFIGLEN=");Serial.print(CONFIGLEN);Serial.print("/");Serial.println(configLength);
  delay(10);if(configLength>CONFIGLEN) {lethalSleep();}

  memcpy(configVers,TVERSION,2);
  memcpy(macAddr,MAC,6);  
  strncpy((char*)concAddr,CONC,ADDR_LENGTH);
  *numConc=0;
  *thFactor=0.1071;
  *thOffset=50;
  *vFactor=0.0057;
  *vOffset=0;
  *concChannel=0;
  *concSpeed=0;
  *concPeriParams=0;
}

void configPrint()
{
    uint16_t configLen;memcpy(&configLen,configData+EEPRLENGTH,2);
    char configVers[3];memcpy(configVers,configData+EEPRVERS,2);configVers[3]='\0';
    Serial.print("crc ");dumpfield((char*)configData,4);Serial.print(" len ");Serial.print(configLen);Serial.print(" V ");Serial.print(configVers[0]);Serial.println(configVers[1]);
    char buf[7];memcpy(buf,concAddr,ADDR_LENGTH);buf[ADDR_LENGTH]='\0';
    Serial.print("MAC  ");dumpstr((char*)macAddr,6);Serial.print("CONC ");dumpstr((char*)concAddr,5);
    if(memcmp(configVers,"01",2)!=0){Serial.print("numConc ");dumpstr((char*)numConc,1);}

    Serial.print("  thFactor=");Serial.print(*thFactor*10000);Serial.print("  thOffset=");Serial.print(*thOffset);   
    Serial.print("   vFactor=");Serial.print(*vFactor*10000);Serial.print("   vOffset=");Serial.println(*vOffset);   
    Serial.print("(table channels : ");
    for(uint8_t i=0;i<MAXCONC;i++){Serial.print(i);Serial.print(" ");Serial.print(channelTable[i]);Serial.print("  ");}Serial.println(")");
    Serial.print("  channel=");Serial.print(*concChannel);Serial.print("  speed=");Serial.print(*concSpeed);
    Serial.print("  concPeriParams=");Serial.println(*concPeriParams);

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

void testSleepPwrDown()
{
    for(int i=0;i<10;i++){
      bitSet(DDR_LED,BIT_LED);
      bitSet(PORT_LED,BIT_LED);
      delay(5);
      bitClear(PORT_LED,BIT_LED);
      bitClear(DDR_LED,BIT_LED);
      delay(1000);
    }
    sleepPwrDown(0);  
}
