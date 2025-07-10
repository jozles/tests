  #define OUT1 8
  #define OUT2 9
  #define LED 4
  #define ON HIGH
  #define OFF LOW
  #define SLOW 9
  #define FAST SLOW+1

  uint8_t nbre=10;
  unsigned long tempo = 200;


void setOut(uint8_t out1,uint8_t out2)
{
    digitalWrite(OUT1,out1);
    digitalWrite(OUT2,out2);
    digitalWrite(LED_BUILTIN,ON);
    delay(10);
    digitalWrite(LED_BUILTIN,OFF);
    delay(tempo);
}

void goUp(uint8_t n){
  for(uint8_t k=0;k<n;k++){
    setOut(LOW,HIGH);
    setOut(HIGH,HIGH);
    setOut(HIGH,LOW);    
    setOut(LOW,LOW);
  }
}
void goDown(uint8_t n){
  for(uint8_t k=0;k<n;k++){
    setOut(HIGH,LOW);    
    setOut(HIGH,HIGH);
    setOut(LOW,HIGH);
    setOut(LOW,LOW);
  }
}

void ledblink(uint8_t speed)
{
    if(speed>SLOW){digitalWrite(LED_BUILTIN,ON);delay(10);digitalWrite(LED_BUILTIN,OFF);delay(100);}
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
    
    setOut(LOW,LOW);
    pinMode(OUT1,OUTPUT);
    pinMode(OUT2,OUTPUT);
    Serial.print(LED_BUILTIN);Serial.println(" fin setup");
    
}

void loop()
{ 
  goUp(2);delay(2000);goDown(2);

  Serial.println("terminé");
  while(1){digitalWrite(LED_BUILTIN,ON);delay(20);digitalWrite(LED_BUILTIN,OFF);delay(1000);}
}
