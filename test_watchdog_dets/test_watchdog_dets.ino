#define LED 13
#define OSC 7
#define INR 6
#define DONE 5

#define FREQ 50    //Hz

#define PERLED 100000   // 1/2 per uS

unsigned long ledTime=millis();

unsigned long period;

void setup() {

  period=1000000/FREQ;
  
  Serial.begin(115200);
  Serial.print("running at ");
  Serial.print(FREQ);Serial.print("Hz / ");
  Serial.print(period);Serial.println("uS");
  
  Serial.print("\nOSC=");Serial.println(OSC);
  Serial.print("INR=");Serial.println(INR);
  Serial.print("DONE=");Serial.println(DONE);

  pinMode(LED,OUTPUT);
  pinMode(OSC,OUTPUT);
  pinMode(INR,INPUT);
  pinMode(DONE,INPUT);
}

void loop() {
  unsigned long period_begin=micros();
  while(micros()-period_begin<(period/2)){

    if((micros()-ledTime)>PERLED){
      ledTime=micros();
      digitalWrite(LED,!digitalRead(LED));}
  }
  if(digitalRead(LED)&&digitalRead(OSC)){
    digitalWrite(DONE,HIGH);
    pinMode(DONE,OUTPUT);
  }
  if(!digitalRead(LED)&&digitalRead(OSC)){
    pinMode(DONE,INPUT);
  }
    
      digitalWrite(OSC,!digitalRead(OSC));
}
