#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"

// D finition des entr es/sorties
#define PIN_LED_ROUGE 2
#define PIN_LED_VERTE 1
#define PIN_LED_BLANC 3

// Page html
const char index_html[] PROGMEM = R"=====(
<!doctype html>
<html lang="fr">
    <head>
        <meta charset="utf-8">
        <title>Commande RELAIS</title>
    </head>
    <body>
        <h1>Etat du RELAIS</h1>
        <h2 id="etatRELAIS">%RELAIS%</h2>
        <h1>Commande du relais</h1>
        <button onclick="appelServeur('/switchLedOn', traiteReponse)">RELAIS ON</button>
        <button onclick="appelServeur('/switchLedOff', traiteReponse)">RELAIS OFF</button>
        
         <h1>Etat de la LED</h1>
        <h2 id="etatLED">%LED%</h2>
         <h1>Commande de la led</h1>
        <button onclick="appelServeur('/switchLedOne', traiteReponse)">LED ON</button>
        <button onclick="appelServeur('/switchLedOffe', traiteReponse)">LED OFF</button>
        
        <script>
            function appelServeur(url, cFonction) {
                var xhttp = new XMLHttpRequest();
                xhttp.onreadystatechange = function() {
                    if (this.readyState == 4 && this.status == 200) {
                        cFonction(this);
                    }
                };
                xhttp.open("GET", url, true);
                xhttp.send();
            }
            function traiteReponse(xhttp) {
                document.getElementById("etatRELAIS").innerHTML = "RELAIS " + xhttp.responseText;
                document.getElementById("etatLED").innerHTML = "LED " + xhttp.responseText;
            }
        </script>
    </body>
</html>
)=====";

// Informations de connexion : cach es dans fichier InfoConnexion.h
// Vous pouvez d commenter ici ou cr er comme moi un fichier InfoConnexion.h
const char * SSID = "ssid";
const char * PASSWORD = "password";

// Gestion des  v nements du WiFi
// Lorsque la connexion vient d'aboutir
void onConnected(const WiFiEventStationModeConnected& event);
// Lorsque l'adresse IP est attribu e
void onGotIP(const WiFiEventStationModeGotIP& event);

// Objet WebServer
ESP8266WebServer serverWeb(80);

// Fonctions du serveur Web
void handleRoot() {
  String temp(reinterpret_cast<const __FlashStringHelper *> (index_html));
  if (digitalRead(PIN_LED_VERTE) == HIGH) temp.replace("%RELAIS%", "ON"); else temp.replace("%RELAIS%", "OFF"); 
  serverWeb.send(200, "text/html", temp);

  if (digitalRead(PIN_LED_BLANC) == HIGH) temp.replace("%LED%", "ON"); else temp.replace("%LED%", "OFF"); 
  serverWeb.send(200, "text/html", temp);
}

void switchLedOn() {
  digitalWrite(PIN_LED_VERTE, HIGH);
  serverWeb.send(200, "text/plain", "ON");
}

void switchLedOff() {
  digitalWrite(PIN_LED_VERTE, LOW);
  serverWeb.send(200, "text/plain", "OFF");
}


void switchLedOne() {
  digitalWrite(PIN_LED_BLANC, HIGH);
  serverWeb.send(200, "text/plain", "ON");
}

void switchLedOffe() {
  digitalWrite(PIN_LED_BLANC, LOW);
  serverWeb.send(200, "text/plain", "OFF");
}
void setup() {
  // Mise en place d'une liaison s rie
  Serial.begin(9600L);
  delay(100);

  // Configuration des entr es/sorties
  pinMode(PIN_LED_ROUGE, OUTPUT);
 pinMode(PIN_LED_VERTE, OUTPUT);
 pinMode(PIN_LED_BLANC, OUTPUT);
  // Mode de connexion
  WiFi.mode(WIFI_STA);
 
  // D marrage de la connexion
  WiFi.begin(SSID, PASSWORD);

  static WiFiEventHandler onConnectedHandler = WiFi.onStationModeConnected(onConnected);
  static WiFiEventHandler onGotIPHandler = WiFi.onStationModeGotIP(onGotIP);

  // Mise en place du serveur WebServer
  serverWeb.on("/switchLedOn", switchLedOn);
  serverWeb.on("/switchLedOff", switchLedOff);
  serverWeb.on("/switchLedOne", switchLedOne);
  serverWeb.on("/switchLedOffe", switchLedOffe);
  
  serverWeb.on("/", handleRoot);
  serverWeb.on("/index.html", handleRoot);
 

  
  serverWeb.begin();
}

void loop() {
  // Si l'objet est connect  au r seau, on effectue les t ches qui doivent l' tre dans ce cas
  if (WiFi.isConnected()) {
    digitalWrite(PIN_LED_ROUGE, HIGH);
    serverWeb.handleClient();
  }
  else {
    digitalWrite(PIN_LED_ROUGE, LOW);
  }

}

void onConnected(const WiFiEventStationModeConnected& event) {
  Serial.println("WiFi connect ");
  Serial.println("Adresse IP : " + WiFi.localIP().toString());
}

void onGotIP(const WiFiEventStationModeGotIP& event) {
  Serial.println("Adresse IP : " + WiFi.localIP().toString());
  Serial.println("Passerelle IP : " + WiFi.gatewayIP().toString());
  Serial.println("DNS IP : " + WiFi.dnsIP().toString());
  Serial.print("Puissance de r ception : ");
  Serial.println(WiFi.RSSI());
}