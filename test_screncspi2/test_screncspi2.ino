#define LED 4
#define DLYBON  100
#define DLYBOFF 950

unsigned long lastBlink=millis(),dlyBlink=DLYBOFF;

#define NB 5
uint8_t pin[NB];
char    pn[NB*5];
uint8_t nb=0,p;


void setup() {

pinMode(LED,OUTPUT);
  
Serial.begin(115200);
Serial.println("start (one key to change)");

/*pin[0]=11;
pin[1]=13;
pin[2]=8;
pin[3]=9;
pin[4]=10;*/
pin[0]=3;
pin[1]=5;
pin[2]=0;
pin[3]=1;
pin[4]=2;

memcpy(pn,"MOSI\0CLCK\0CSVA\0CSU5\0CSU4\0",NB*5);
}

void loop() {
  
  while(!Serial.available()){
    bitSet(PORTB,pin[nb]);bitClear(PORTB,pin[nb]);
    if(millis()>(lastBlink+dlyBlink)){
      if(dlyBlink==DLYBON){dlyBlink=DLYBOFF;digitalWrite(LED,LOW);}else{dlyBlink=DLYBON;digitalWrite(LED,HIGH);}
      lastBlink=millis();
    }
//  digitalWrite(pin[nb],1);delay(10);digitalWrite(pin[nb],0);delay(10);}
//  p=digitalRead(pin[nb]);
//  digitalWrite(pin[nb],!p);delay(10);}
  }

char a=Serial.read();

nb++;if(nb>=NB){nb=0;}
//pinMode(pin[nb],OUTPUT);
bitSet(DDRB,pin[nb]);

Serial.print(a);Serial.print(" ");Serial.print(nb);Serial.print(" ");Serial.print(pin[nb]);Serial.print(" ");Serial.println((char*)pn+nb*5);
}
