#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile

RH_ASK driver;

void setup()
{

    digitalWrite(4,LOW);
    pinMode(4,OUTPUT);
    digitalWrite(7,HIGH);
    pinMode(7,OUTPUT);
    
    Serial.begin(115200); // Debugging only
    if (!driver.init()){Serial.println("init failed");}
    else {Serial.println("RX 433-RH ready");}
}

void loop()
{
    uint8_t buf[32];
    uint8_t buflen = sizeof(buf);
    memset(buf,'\0',sizeof(buf));
    if (driver.recv(buf, &buflen)) // Non-blocking
    {
      // Message with a good checksum received, dump it.
      Serial.print("Message: ");
      Serial.println((char*)buf);         
    }
}
