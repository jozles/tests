void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  delay(1000);

  Serial.println("ready");

  while(1){
    while(Serial1.available()){
      char a=Serial1.read();Serial.print(":");Serial.print(a);}
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
