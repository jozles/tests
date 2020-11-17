#include <SPI.h>
#include <SD.h>
#include <Wire.h>     //biblio I2C pour RTC 3231
#include "ds3231.h"

#define SDC 3
#define LED 2
#define PER 2000
#define DUR 10

char test=0;
char testFile[]="oregon.txt";
char in=0;
uint8_t sdc=SDC;
uint32_t dur=DUR;
uint32_t per=PER;

Ds3231 pendule;

#define DS3231_I2C_ADDRESS 0x68;
byte ss,mm,hh,wd,dd,dm,yy;

#define PINVCCDS 19   // alim + DS3231
#define PINGNDDS 18   // alim - DS3231

void sdInit();
void sdFileRead(char* fileName);
void printdh();

void setup() {
  digitalWrite(LED,LOW);
  pinMode(LED,OUTPUT);
  delay(100);
  Serial.begin(115200);
  Serial.println("\nTest red shield\nW watchdog ; S SDcard ; D DS3231 clock ; O mise à l'heure");
  test=getch();Serial.println(test);

    switch (test){
      
      case '\W':
        Serial.println("- double la période ; + la divise par 2 ; 1,2,5,0 ajoute 100,200,500,1000 ");
        Serial.println(" > add 1mS à la durée < sub 1 mS ; $ menu");
        break;
        
      case '\S':
        Serial.print("CS pin (");Serial.print(sdc);Serial.println(")");
//        sdc=(uint8_t)getch()-48;Serial.println(sdc);
        dur=1;
        sdInit();
        sdFileRead(testFile);
        break;
        
      case '\D':
        Wire.begin();delay(100);
        digitalWrite(PINGNDDS,LOW);pinMode(PINGNDDS,OUTPUT);  
        digitalWrite(PINVCCDS,HIGH);pinMode(PINVCCDS,OUTPUT);
        pendule.i2cAddr=DS3231_I2C_ADDRESS;
        
        pendule.readTime(&ss,&mm,&hh,&wd,&dd,&dm,&yy);
        printdh();
        break;

      case '\O':
Serial.println("*** modifier date/heure puis relancer ***");

        Wire.begin();delay(100);
        digitalWrite(PINGNDDS,LOW);pinMode(PINGNDDS,OUTPUT);  
        digitalWrite(PINVCCDS,HIGH);pinMode(PINVCCDS,OUTPUT);
        pendule.i2cAddr=DS3231_I2C_ADDRESS;
/*
        ss=0;mm=45;hh=14;wd=4;dd=28;dm=10;yy=20;
        pendule.setTime(ss,mm,hh,wd,dd,dm,yy);
*/
        pendule.readTime(&ss,&mm,&hh,&wd,&dd,&dm,&yy);
        printdh();
        break;
                        
      default:
        break;
    }
  Serial.println("end setup");
}

void loop() {

  led();delay(per-dur);

  if(test==87){             /* test watchdog */    
    if(Serial.available()!=0){
      in=Serial.read();
      switch(in){
        case '\+': per/=2;break;
        case '\-': per*=2;break;
        case 49: per+=100;break;      
        case 50: per+=200;break;
        case 53: per+=500;break;
        case 48: per+=1000;break;            
        case '\>': dur++;break;
        case '\<': dur--;break;
        default: break;
      }
      Serial.print(in);Serial.print(" per=");Serial.print(per);Serial.print(" dur=");Serial.println(dur);
    }
  }
  if(test==83 || test=='\O' || test=='\D'){             /* test SDcard ou DS3231 retour menu */    
    while(1){};
  }
  if(in=='\$'){           /* retour menu */    
    while(1){};
  }
  
}

void sdInit()
{ 
  Serial.print("Initializing SD card...");
  led();
  // see if the card is present and can be initialized:
  if (!SD.begin(sdc)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  led();
  Serial.println("card initialized.");
}

void sdFileRead(char* fileName)
{
  led();
  File dataFile = SD.open(fileName);      
  led();
  if (dataFile) {
    while (dataFile.available()) {Serial.write(dataFile.read());}
    led();
    dataFile.close();
  }
  
  else {
    Serial.print("error opening ");Serial.println(fileName);
  }
}

void led()
{
    digitalWrite(LED,HIGH);
  delay(dur);
  digitalWrite(LED,LOW);
}

char getch()
{
  while(Serial.available()==0){led();delay(1000);}
  return Serial.read();
}

void printdh0(byte v)
{
  if(v<10){Serial.print("0");}Serial.print(v);
}

void printdh()
{
  printdh0(dd);Serial.print("/");printdh0(dm);Serial.print("/");printdh0(yy);Serial.print(" ");
  printdh0(hh);Serial.print(":");printdh0(mm);Serial.print(":");printdh0(ss);
  Serial.println();
}
