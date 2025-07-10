/*
 * EMailSender library for Arduino, esp8266 and esp32
 * Simple esp8266 Gmail send example
 *
 * https://www.mischianti.org/category/my-libraries/emailsender-send-email-with-attachments/
 *
 * modif 28/08/2023 @AP la led s' allume au démarrage pendant plusieurs secondes 
 * jusqu'à ce que la connexion wifi soit réalisée
 * puis s'éteint 1 sezconde et clignote 3 fois 
 * un mail est alors envoyé : 
 * clignotement régulier 0,5sec ON/OFF KO ; 2 flashs / 0,5 sec OK
 * modifier LED/LEDON/LEDOFF selon la carte utilisée
 * modifier les params : 
 * ssid,password,msender,msenderpwd,mreceiver,msujet,mmessage et rien d'autre
 * 
 */

#include "Arduino.h"
#include <EMailSender.h>
#include <ESP8266WiFi.h>

// pour sonoff_RF_R2 modifier les numéros de pins et LEDON/LEDOFF pour une autre carte
#define LED 13
#define LEDON LOW
#define LEDOFF HIGH
#define RELAY 12

//------------ params ---------------
const char* ssid = "mon_wifi";
const char* password = "mon_mdp_du_wifi";

const char* msender    = "mon adresse mail expéditeur";
const char* msenderpwd = "mon mdp google mail expéditeur";
const char* mreceiver  = "adresse mail destinataire"; // peut être la même que l'expéditeur
const char* msujet     = "test emailsender 2.4.2"; //"sujet";
const char* mmessage   = "great !<br>that works<br>merci + bises"; //"mon message";
// -----------------------------------


uint8_t connection_state = 0;
uint16_t reconnect_interval = 1000;

EMailSender emailSend(msender, msenderpwd);

uint8_t WiFiConnect(const char* nSSID = nullptr, const char* nPassword = nullptr)
{
    static uint16_t attempt = 0;
    Serial.print("Connecting to ");Serial.print(nSSID);Serial.print("/");Serial.println(nPassword);
    if(nSSID) {
        WiFi.begin(nSSID, nPassword);
        Serial.print(nSSID);
    }

    uint8_t i = 0;
    while(WiFi.status()!= WL_CONNECTED && i++ < 50)
    {
        delay(200);
        Serial.print(".");
    }
    ++attempt;

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
        digitalWrite(LED,!LED);
        if(millis() > (ts + reconnect_interval) && !connection_state){
            connection_state = WiFiConnect();
            ts = millis();
        }
    }
}

void setup()
{
    pinMode(LED,OUTPUT);

    Serial.begin(115200);
    Serial.println("start ");
    delay(1000);

    connection_state = WiFiConnect(ssid, password);
    if(!connection_state)  // if not connected to WIFI
        Awaits();          // constantly trying to connect

    digitalWrite(LED,LEDOFF);delay(1000);
    // 3 blinks ok
    for(uint8_t i=0;i<3;i++){
      digitalWrite(LED,LEDON);delay(5);digitalWrite(LED,LEDOFF);delay(200);
    }

    EMailSender::EMailMessage message;
    message.subject = msujet; 
    message.message = mmessage;

    EMailSender::Response resp = emailSend.send(mreceiver, message);

    Serial.println("Sending status: ");

    Serial.println(resp.status);
    Serial.println(resp.code);
    Serial.println(resp.desc);

    delay(1000);
    
#define SLOW 500
      while(1){
        if(resp.status==true){    
          for(uint8_t i=0;i<2;i++){digitalWrite(LED,LEDON);delay(5);digitalWrite(LED,LEDOFF);delay(200);}
        }
        else{
          digitalWrite(LED,LEDON);delay(SLOW);digitalWrite(LED,LEDOFF);
        }         
        delay(SLOW);
      }


}

void loop()
{

}
