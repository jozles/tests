#include <ESP8266WiFi.h>
#include "Gsender.h"

#pragma region Globals
/* paramètres du routeur WIFI */
const char* ssid      = "le nom de ton wifi";                 // WIFI network name
const char* password  = "le mot de passe de ton reseau wifi"; // WIFI network password
#pragma endregion Globals

Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance

String sujet,dest,mess;

bool sendMail(const String sujet, const String destinataire, const String message)
{
  if(gsender->Subject(sujet)->Send(destinataire,message)){
        Serial.println("Message send.");
    } else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError());
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    
/* connexion au wifi à ne faire "normalement" qu'une fois */    
    if(!WiFiConnect(ssid,password))       // if not connected to WIFI
        Awaits(ssid,password);                         // constantly trying to connect

/* exemple d'utilisation de la fonction d'envoi de mails */
    sujet="test mail 8266";
    dest="toto@gmail.com";
    mess="message de test";

    sendMail(sujet,dest,mess);
}

void loop(){}
