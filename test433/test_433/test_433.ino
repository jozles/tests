
#define GND 2
#define INT433 3 // pin interruption data 433MHz
#define VCC 5
#define OUT433 12
#define OUTPER 100

bool bitout=1;
long timeOutput=micros();

char* message="ABCDEFGHIJKLMNOPQRSTUVWXYZ\0";
uint8_t bytecnt=-1,bitcnt=8;
byte byteshift;

byte etat=0,etat0=0;

void isr433();

void setup() {
  Serial.begin(115200);
  Serial.println("\ntest 433 ready");

// module émission
  pinMode(OUT433,OUTPUT);
  digitalWrite(OUT433,bitout);

// alimentation module réception  
  digitalWrite (GND,0);pinMode(GND,OUTPUT);digitalWrite (GND,0);
  digitalWrite (VCC,1);pinMode(VCC,OUTPUT);digitalWrite (VCC,1);
  
  attachInterrupt(digitalPinToInterrupt(INT433), isr433, CHANGE);

}

void loop() 
{

  if(micros()>timeOutput+OUTPER){
    
    bitcnt++; if(bitcnt>=8){
      bitcnt=0;bytecnt++;if(bytecnt>26){while(1){};}
      byteshift=message[bytecnt];
      Serial.println();Serial.print(etat,HEX);Serial.print(" ");Serial.print(bytecnt);Serial.print(" ");Serial.print(byteshift,HEX);Serial.print(" ");
    }

    bitout=0;
    if((byteshift & 0x80) !=0){bitout=1;};
    byteshift=byteshift << 1;
    digitalWrite(OUT433,bitout);
    Serial.print(bitout);
  }
  
  //if(etat!=etat0){etat0=etat;Serial.println();Serial.println(etat0,HEX);}

}

void isr433()
{
  etat = (etat << 1) | digitalRead(INT433);
}

