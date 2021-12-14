#include <ESP8266WiFi.h>
#include "Gsender.h"

#pragma region Globals

/* paramètres du routeur WIFI */
const char* ssid      = "pinks"; //le nom de ton wifi";                 // WIFI network name
const char* password  = "cain ne dormant pas songeait au pied des monts"; //le mot de passe de ton reseau wifi"; // WIFI network password

uint8_t connection_state = 0;                    // Connected to WIFI or not
uint16_t reconnect_interval = 10000;             // If not connected wait time to try again
#pragma endregion Globals

Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance


uint8_t WiFiConnect(const char* nSSID = nullptr, const char* nPassword = nullptr)
{
    static uint16_t attempt = 0;
    Serial.print("Connecting to ");
    if(nSSID) {
        WiFi.begin(nSSID, nPassword);  
        Serial.println(nSSID);
    } else {
        WiFi.begin(ssid, password);
        Serial.println(ssid);
    }

    uint8_t i = 0;
    while(WiFi.status()!= WL_CONNECTED && i++ < 50)
    {
        delay(200);
        Serial.print(".");
    }
    ++attempt;
    Serial.println("");
    if(i == 51) {
        Serial.print("Connection: TIMEOUT on attempt: ");
        Serial.println(attempt);
        if(attempt % 2 == 0)
            Serial.println("Check if access point available or SSID and Password\r\n");
        return false;
    }
    Serial.println("Connection: ESTABLISHED");
    Serial.print("Got IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void Awaits()
{
    uint32_t ts = millis();
    while(!connection_state)
    {
        delay(50);
        if(millis() > (ts + reconnect_interval) && !connection_state){
            connection_state = WiFiConnect();
            ts = millis();
        }
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
    
    connection_state = WiFiConnect();
    if(!connection_state)  // if not connected to WIFI
        Awaits();          // constantly trying to connect

    String sujet="test mail 8266";
    String dest="lucieliu66@gmail.com";
    String mess="message de test";

    if(sendMail(sujet,dest,mess)){             //"test mail 8266","lucieliu66@gmail.com","message de test")){
        Serial.println("Message send.");
    } else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError());
    }
}

void loop(){}
