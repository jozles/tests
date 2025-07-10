#define LED 12
#define LOAD 7

#define PER 100

void setup() {
  Serial.begin(115200);Serial.print("per=");Serial.print(PER);Serial.println(" ready");
  pinMode(LED,OUTPUT);
  //pinMode(LOAD,OUTPUT);
}

void loop() {
  delay(PER/2);
  digitalWrite(LED,HIGH);//digitalWrite(LOAD,HIGH);
  delay(PER/2);
  digitalWrite(LED,LOW);//digitalWrite(LOAD,LOW);

}
