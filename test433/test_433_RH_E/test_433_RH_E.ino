#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

RH_ASK driver;

int i=0;
char text[32];

void setup()
{
    Serial.begin(115200);   // Debugging only
    if (!driver.init()) {Serial.println("init failed");}
    else  {Serial.println("TX 433-RH ready");}
}

void loop()
{
  i++;
    

    sprintf(text,"Hello World! %u",i);
    strcat(text,"\0");
   // Serial.println(text);

   // const char *msg = "Hello World !";
   // driver.send((uint8_t *)msg, strlen(msg));
    driver.send((uint8_t *)text, strlen(text));
    driver.waitPacketSent();
    delay(1000);
}

