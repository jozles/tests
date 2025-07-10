  #define OUT1 8
  #define OUT2 9
  #define LED 4
  #define ON HIGH
  #define OFF LOW
  #define SLOW 9
  #define FAST SLOW+1

  uint8_t nbre=10;
  unsigned long tempo = 200;


void setInput(uint8_t out1,uint8_t out2)
{
    pinMode(out1,INPUT);
    pinMode(out2,INPUT);
}

void setOut(uint8_t out1,uint8_t out2)
{
    if(out1==LOW){pinMode(OUT1,OUTPUT);digitalWrite(OUT1,LOW);}
    else{pinMode(OUT1,INPUT);}
    if(out2==LOW){pinMode(OUT2,OUTPUT);digitalWrite(OUT2,LOW);}
    else{pinMode(OUT2,INPUT);}

    digitalWrite(LED_BUILTIN,ON);
    delay(100);
    digitalWrite(LED_BUILTIN,OFF);
    delay(tempo);

    pinMode(OUT1,INPUT);
    pinMode(OUT2,INPUT);
    
    digitalWrite(OUT1,HIGH);
    digitalWrite(OUT2,HIGH);
}

void goDown(uint8_t n){
  for(uint8_t k=0;k<n;k++){
    setOut(HIGH,LOW);
    setOut(LOW,LOW);
    setOut(LOW,HIGH);
    setOut(HIGH,HIGH);    
  }
}
void goUp(uint8_t n){
  for(uint8_t k=0;k<n;k++){
    setOut(LOW,HIGH);    
    setOut(LOW,LOW);
    setOut(HIGH,LOW);
    setOut(HIGH,HIGH);
  }
}

void ledblink(uint8_t speed)
{
    if(speed>SLOW){digitalWrite(LED_BUILTIN,ON);delay(10);digitalWrite(LED_BUILTIN,OFF);delay(40);}
    if(speed<=SLOW){digitalWrite(LED_BUILTIN,ON);delay(500);digitalWrite(LED_BUILTIN,OFF);delay(500);}
}


void setup() 
{
    Serial.begin(115200);
    pinMode(LED_BUILTIN,OUTPUT);digitalWrite(LED_BUILTIN,OFF);

    /*
    Serial.print("synchro...");
    pinMode(OUT1,INPUT);pinMode(OUT2,INPUT);
    while(OUT1!=HIGH || OUT2!=HIGH){ledblink(FAST);}
    while(char a=Serial.read()!='g'){ledblink(SLOW);}
    Serial.println();
*/

    Serial.println("synchro");
    setInput(OUT1,OUT2);
    /*
    while(digitalRead(OUT1)!=HIGH || digitalRead(OUT2)!=HIGH){
      ledblink(SLOW+1);
    }
    for(uint8_t i=0;i<3;i++){ledblink(SLOW);}
*/
    
    Serial.print(LED_BUILTIN);Serial.println(" fin setup");
    
}

void loop()
{ 
  //pinMode(OUT1,OUTPUT);
  //pinMode(OUT2,OUTPUT);
  
  
  goUp(15);delay(2000);goDown(12);
  setInput(OUT1,OUT2);

  Serial.println("terminé");
  while(1){digitalWrite(LED_BUILTIN,ON);delay(20);digitalWrite(LED_BUILTIN,OFF);delay(1000);}
}
