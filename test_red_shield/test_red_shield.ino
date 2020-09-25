#include <SPI.h>
#include <SD.h>

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

void sdInit();
void sdFileRead(char* fileName);

void setup() {
  digitalWrite(LED,LOW);
  pinMode(LED,OUTPUT);
  delay(100);
  Serial.begin(115200);
  Serial.println("\nW watchdog ; S SDcard");
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
    }
 
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
  if(test==83){             /* test SDcard retour menu */    
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
