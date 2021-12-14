#include <ESP8266WiFi.h>
#include "Gsender.h"

#pragma region Globals

/* paramètres du routeur WIFI */
const char* ssid      = "le nom de ton wifi";                 // WIFI network name
const char* password  = "le mot de passe de ton reseau wifi"; // WIFI network password

uint8_t connection_state = 0;                    // Connected to WIFI or not
uint16_t reconnect_interval = 10000;             // If not connected wait time to try again
#pragma endregion Globals

Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance

String sujet,dest,mess;

uint8_t WiFiConnect(const char* ssid, const char* password)
{
    static uint16_t attempt = 0;
    Serial.print("Connecting to ");Serial.print(ssid);

    WiFi.begin(ssid, password);
    
    uint8_t i = 0;
    while(WiFi.status()!= WL_CONNECTED && i++ < 50){
        delay(200);
        Serial.print(".");
    }
    
    if(i == 51) {Serial.print("failed ");return false;}       
    else {    
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    }
}

bool sendMail(const String sujet, const String destinataire, const String message)
{
  return gsender->Subject(sujet)->Send(destinataire,message);
}


void setup()
{
    Serial.begin(115200);
    delay(1000);
    
    if(WiFiConnect(ssid,password)){

      sujet="test mail 8266";
      dest="lucieliu66@gmail.com";
      mess="message de test";

      if(sendMail(sujet,dest,mess)){             
        Serial.println("Message send.");
      } 
      else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError());
      }
    }
}

void loop(){}
