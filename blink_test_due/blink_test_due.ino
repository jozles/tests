

// the setup function runs once when you press reset or power the board

#define WD 3
#define LED LED_BUILTIN

void setup() {
  Serial.begin(115200);
  Serial.print("\n+");Serial.println(LED);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED, OUTPUT);
  pinMode(WD, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(WD,HIGH);
  delay(1000);                       // wait for a second
  digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(WD,LOW);
  delay(1000);                       // wait for a second
}
