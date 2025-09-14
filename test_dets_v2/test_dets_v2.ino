#include "powerSleep.h"
#include "hard.h"
#include "eepr.h"
//#include "d:\data\arduino\libraries\nrf\src\nrf24l01s.h"
#include "d:\data\arduino\libraries\shlib2\src\shconst2.h"


#define VERSION "03"
#define MAC "peri_"
#define CONC "SHCO3"  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<< configurer 

#define ADDR_LENGTH 5

#define N_PWR_LEVEL 4
#define RF_POWER_0_VALUE  0X03        // 0db
#define RF_POWER_6_VALUE  0X02        // -6db
#define RF_POWER_12_VALUE 0X01        // -12db
#define RF_POWER_18_VALUE 0X00        // -18db
#define RF_POWER_L0 -18
#define RF_POWER_L1 -12
#define RF_POWER_L2 -6
#define RF_POWER_L3 0
uint8_t rf_power_v[N_PWR_LEVEL]={RF_POWER_0_VALUE,RF_POWER_6_VALUE,RF_POWER_12_VALUE,RF_POWER_18_VALUE};
int8_t rf_power[N_PWR_LEVEL]={RF_POWER_L3,RF_POWER_L2,RF_POWER_L1,RF_POWER_L0};

char c='\0';
char c1='\0';
char c2='\0';
float volts=0;                           // tension alim (VCC)
float temp;
float refMiniT=735; //747;
float refMaxiT=785; //753;                      // référence tension étalonnage th
float refMiniV=320;
float refMaxiV=410;                      // référence tension étalonage volts

uint8_t k;

Eepr eeprom;

//uint8_t channelTable[]={CHANNEL0,CHANNEL1,CHANNEL2,CHANNEL3};   // canal vs N° conc
uint8_t channelTable[]={120,110,100,90};   // canal vs N° conc

#define CONFIGLEN 75 // 37 version 01 // 38 version 02 // 75 version 03
byte    configData[CONFIGLEN];

byte*  configVers;
float* thFactor;
float* thOffset;
float* vFactor;
float* vOffset;
byte*  macAddr;
byte*  concAddr;
uint8_t* concNb;
uint8_t* concChannel;
uint8_t* concSpeed;
uint8_t* concPeriParams;
uint8_t* powerLevel;
int8_t*  perAdjust;

bool   pgAuto=false;    // si true pas de saisie, paramétrage automatique ; valoriser kt=3 c='T', kv=4 c='V', c='E' c1='I' c2=n° perif, c='E' c1='L' ;
int    pg=-1;               // pointeur fonction en cours en mode auto
#define NBPG 5
char   cpg[]= {"TVPEE"};
char   c1pg[]={"   IL"};
char   c2pg='g';            // ******************** numéro périf *********************
uint8_t kv=5,kt=3;

char*  chexa="0123456789ABCDEFabcdef\0";

bool   extTimer;
unsigned long t_on=0;

void configPrint();
char getch();
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

void conv_htoa(char* ascii,byte* h)
{
    uint8_t c=*h,d=c>>4,e=c&0x0f;
        ascii[0]=chexa[d];ascii[1]=chexa[e];
}

void conv_htoa(char* ascii,byte* h,uint8_t len)
{
    for(uint8_t i=0;i<len;i++){
      conv_htoa(ascii+2*(len-i-1),(byte*)(h+i));
    }
}

void dumpstr0(char* data,uint8_t len,bool cr)
{
    char a[]={0x00,0x00,0x00};
    uint8_t c;
    Serial.print("   ");Serial.print((long)data,HEX);Serial.print("   ");

    for(int k=0;k<len;k++){conv_htoa(a,(byte*)&data[k]);Serial.print(a);Serial.print(" ");}
    Serial.print("    ");
    for(int k=0;k<len;k++){
            c=data[k];
            if(c<32 || c>127){c='.';}
            Serial.print((char)c);
    }
    if(cr){Serial.println();}
}

void dumpstr(char* data,uint16_t len,bool cr)
{
    while(len>=16){len-=16;dumpstr0(data,16,len>0);data+=16;}
    if(len!=0){dumpstr0(data,len,false);}
    if(cr){Serial.println();}
}

void dumpstr(char* data,uint16_t len)
{
  return dumpstr(data,len,true);
}

void dumpfield(char* fd,uint8_t ll)
{
    byte a;
    for(int ff=ll-1;ff>=0;ff--){
            a=((fd[ff]&0xF0)>>4)+'0';if(a>'9'){a+=7;}Serial.print((char)a);
            a=(fd[ff]&0x0F)+'0';if(a>'9'){a+=7;}Serial.print((char)a);
            Serial.print(" ");
    }
    Serial.print(" ");
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
  Serial.print(" period ");Serial.print(period);Serial.println("sec "); // external timer period


  //testCrc();

  adcRead(INADMUXVAL,1,0,0,1); // réveil ADC

  initConf();
}

void loop(){  
  Serial.println("\nrégler le terminal sur \"pas de fin de ligne - et patienter à chaque saisie\" ");
  Serial.println("blink avec delay       O k  S kip  T calibration thermo  V calibration volts  P perif nb etc   E eprom");

  if(!pgAuto){
    while(!Serial.available()){bitSet(DDR_LED,BIT_LED);bitSet(PORT_LED,BIT_LED);delay(2000);bitClear(PORT_LED,BIT_LED);bitClear(DDR_LED,BIT_LED);delay(2000);}
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
      Serial.print(" voltage ");Serial.print(volts);getVT();Serial.print(" voltage ");Serial.print(volts);
      Serial.print(" thFactor=");Serial.print((float)*thFactor*1000);
      Serial.print(" temp ");Serial.println(temp);
      break;

    case 'T':
    {
      bitSet(PORT_VCHK,BIT_VCHK);bitSet(DDR_VCHK,BIT_VCHK);
      delay(1000);

      temp=adcRead(TADMUXVAL,1,0,0,20);
      Serial.print(" adcRead() Th ");Serial.println(temp);
      
      Serial.print(" valeur référence (");
      uint8_t v=5;
      for(int k=0;k<(refMaxiT-refMiniT)/v;k++){
        Serial.print(k);Serial.print("=.");Serial.print((int)(refMiniT+k*v));if(k*v<(refMaxiT-refMiniT-1)){Serial.print(" ");}
      }

      Serial.print(")? ");
      if(!pgAuto){
        c=getch();while(c>'9' || c<'0'){c=getch();};
        kt=(uint8_t)c-(uint8_t)'0';
      }

      Serial.println(kt);
      temp=adcRead(TADMUXVAL,1,0,0,20);
      *thFactor=(float)((float)(refMiniT+kt*v)/(float)temp)/10;
      Serial.print(" adcRead() Th ");Serial.print(temp);Serial.print("  thFactor=");Serial.print(*thFactor*10000);
      getVT();Serial.print(" temp=");Serial.println(temp);
      bitClear(PORT_VCHK,BIT_VCHK);
    }
      break;

    case 'V':
    {
      bitSet(PORT_VCHK,BIT_VCHK);bitSet(DDR_VCHK,BIT_VCHK);
      delay(1);

      volts=adcRead(VADMUXVAL,1,0,0,20);
      Serial.print(" adcRead(7) V ");Serial.println(volts);

      float intThSensor=adcRead(INADMUXVAL,1,0,0,20);     
      float intTh=intThSensor*1100/1024-242-45;                               // see datasheet page 247 
      Serial.print(" adcRead(int th sensor) ");Serial.print(intThSensor);Serial.print(" internal temp =");Serial.println(intTh);
      
      Serial.print(" valeur référence (");
      for(int k=0;k<(refMaxiV/10-refMiniV/10+1);k++){
        Serial.print(k);Serial.print("=");Serial.print((refMiniV/10+k)/10);if(k<(refMaxiV/10-refMiniV/10)){Serial.print(" ");}
      }

      Serial.print(") ? ");
      if(!pgAuto){
        c=getch();while(c>'9' || c<'0'){c=getch();};
        kv=(uint8_t)c-(uint8_t)'0';
      }

      Serial.println(kv);
      volts=adcRead(VADMUXVAL,1,0,0,20);
      *vFactor=(float)((float)(((refMiniV/10)+kv)*10)/(float)volts)/100;
      Serial.print(" adcRead() V ");Serial.print(volts);Serial.print("   vFactor=");Serial.print(*vFactor*10000);
      getVT();Serial.print("  volt=");Serial.println(volts);
      bitClear(PORT_VCHK,BIT_VCHK);
    }
      break;

    case 'P':
      c2=getch();
      Serial.print(" numéro perif ? ");
      if(!pgAuto){c2='\0';while(c2=='\0'){c2=getch();}}
      Serial.print(' ');
      macAddr[4]=c2;macAddr[5]='\0';
      Serial.println((char*)macAddr);

      c2=getch();
      Serial.print(" numéro concentrateur (0-3)? ");
      if(!pgAuto){c2='\0';while(c2=='\0' || c2<'0' || c2>'3' ){c2=getch();}}
      Serial.print(' ');
      *concNb=c2-48;
      Serial.println(*concNb);
      
      if(memcmp(configVers,"2d",2)==0){
        c2=getch();
        Serial.print(" powerLevel (");
        for(uint8_t i=0;i<N_PWR_LEVEL;i++){
          Serial.print(i);Serial.print("=");Serial.print(rf_power[i]);if(i<N_PWR_LEVEL-1){Serial.print(' ');}}
        Serial.print("db)? ");
        if(!pgAuto){c2='\0';while(c2=='\0' || c2<'0' || c2>(N_PWR_LEVEL+48) ){c2=getch();}}
        Serial.print(' ');
        *powerLevel=rf_power_v[c2-48];
        uint8_t p=0;
        for(p=0;p<N_PWR_LEVEL;p++){
          if(*powerLevel==rf_power_v[p]){break;}
        }
        Serial.print(rf_power[p]);Serial.print("db (");Serial.print(p);Serial.println(')');
      
        c2=getch();
        Serial.print(" perAdjust (0=-1 1=0 2=+1)? ");
        if(!pgAuto){c2='\0';while(c2=='\0' || c2<'0' || c2>'2' ){c2=getch();}}
        Serial.print(' ');
        *perAdjust=c2-48-1;
        Serial.print(*perAdjust);Serial.println("ms");
      }
      break;
      
    case 'E':
      Serial.print("Eeprom ");configPrint();
      Serial.print("L oad  R ecord  M ove to v2d   S kip ");
      
        if(!pgAuto){c1=getch();while(c1=='\0'){c1=getch();}}
        Serial.println(c1);
        switch(c1){
          case 'L':
            if(!eeprom.load(configData,CONFIGLEN)){Serial.println("****KO******");}
            configPrint();
            break;
          case 'R':
            *concChannel=channelTable[*concNb];
            *concSpeed=0;
            *concPeriParams=1;
            //memcpy(configVers,VERSION,2);
            configPrint();
            eeprom.store(configData,CONFIGLEN);
            break;
          case 'S':
            break;
          case 'M':
            if(!eeprom.load(configData,CONFIGLEN)){Serial.println("****KO******");}
            configPrint();          
            memcpy(configVers,"2d",2);
            initConf();
            *powerLevel=1;
            *perAdjust=0;
            configPrint();
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
    delay(1000);
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
  temp +=6;
  concNb=(uint8_t*)temp;
  temp +=sizeof(uint8_t);
  concChannel=(uint8_t*)temp;
  temp+=sizeof(uint8_t);
  concSpeed=(uint8_t*)temp;
  temp+=sizeof(uint8_t);
  concPeriParams=(uint8_t*)temp;
  temp+=sizeof(uint8_t);
  powerLevel=(uint8_t*)temp;
  temp+=sizeof(uint8_t);
  perAdjust=(uint8_t*)temp;
  temp+=sizeof(uint8_t);
  powerLevel=(uint8_t*)temp;
  temp+=sizeof(uint8_t);
  perAdjust=(uint8_t*)temp;
  temp+=sizeof(uint8_t);
  temp+=29;                   // dispo
  
  byte* configEndOfRecord=(byte*)temp;      // doit être le dernier !!!

  long configLength=(long)configEndOfRecord-(long)configBegOfRecord+1;  
  Serial.print("CONFIGLEN=");Serial.print(CONFIGLEN);Serial.print("/");Serial.println(configLength);
  delay(10);if(configLength>CONFIGLEN) {lethalSleep();}

  //memcpy(configVers,VERSION,2);
  memcpy(macAddr,MAC,6);  
  strncpy((char*)concAddr,CONC,5);
  *concNb=0;
  *thFactor=0.1071;
  *thOffset=50;
  *vFactor=0.0057;
  *vOffset=0;
  *concChannel=0;
  *concSpeed=0;
  *concPeriParams=0;
  *powerLevel=1;
  *perAdjust=0;
}

void configPrint()
{
    uint16_t configLen;memcpy(&configLen,configData+EEPRLENGTH,2);
    char configVers[3];memcpy(configVers,configData+EEPRVERS,2);configVers[2]='\0';
    Serial.print(" V ");Serial.print(configVers);Serial.print("  ");
    Serial.print("crc ");dumpfield((char*)configData,4);Serial.print(" len ");Serial.println(configLen);
    char buf[7];memcpy(buf,concAddr,5);buf[5]='\0';
    Serial.print("MAC   ");dumpstr((char*)macAddr,6);Serial.print("CONC  ");dumpstr((char*)concAddr,5);
    if(memcmp(configVers,"01",2)!=0);Serial.print("concNb");dumpstr((char*)concNb,1);

    Serial.print("  thFactor=");Serial.print(*thFactor*10000);Serial.print("  thOffset=");Serial.print(*thOffset);   
    Serial.print("   vFactor=");Serial.print(*vFactor*10000);Serial.print("   vOffset=");Serial.println(*vOffset);   
    Serial.print("(table channels : ");
    for(uint8_t i=0;i<MAXCONC;i++){Serial.print(i);Serial.print(" ");Serial.print(channelTable[i]);Serial.print("  ");}Serial.println(")");
    Serial.print("  channel=");Serial.print(*concChannel);Serial.print("  speed=");Serial.print(*concSpeed);
    Serial.print("  concPeriParams=");Serial.println(*concPeriParams);
  if(memcmp(configVers,"2d",2)==0){
    Serial.print("  powerLevel=");Serial.print(*powerLevel);Serial.print(" ! ");
    uint8_t p=0;
    for(p=0;p<N_PWR_LEVEL;p++){//Serial.print(p);Serial.print(' ');Serial.print(rf_power_v[p]);Serial.print(' ');Serial.print(rf_power[p]);Serial.print(' ');
        if(*powerLevel==rf_power_v[p]){break;}}
    Serial.print(rf_power[p]);Serial.println("db");
      
    Serial.print("  perAdjust=");Serial.print(*perAdjust);Serial.println("ms");
  }
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
