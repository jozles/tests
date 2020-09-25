/*
  Blink
*/

#define LED 4    // 4 on det328

#define REED 3

bool reed;

void setup() {
  pinMode(LED, OUTPUT);
}


void loop() {
  digitalWrite(LED, HIGH); 
  delay(1);                
  digitalWrite(LED, LOW);  

/*    delay(100);             
    digitalWrite(LED, HIGH); 
    delay(1);               
    digitalWrite(LED, LOW);  
    delay(250);   
*/

  if(reed){
    delay(100);             
    digitalWrite(LED, HIGH); 
    delay(1);               
    digitalWrite(LED, LOW);  
    delay(100);               
    digitalWrite(LED, HIGH); 
    delay(1);               
    digitalWrite(LED, LOW);  
  }

delay(1000);  

//if(digitalRead(REED)!=reed){reed=!reed;}
reed=digitalRead(REED);
}
