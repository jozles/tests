#include <WiFi.h>          // Gestion du WiFi sur ESP32
#include <HTTPClient.h>    // Pour faire des requêtes HTTP GET
#include <ArduinoJson.h>   // Pour parser les réponses JSON
#include <TFT_eSPI.h> 
#include <SPI.h>
#include "font.h"

TFT_eSPI my_lcd = TFT_eSPI(); 

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


// ***********************************

// URL DE L’API TEMPO

// https://www.api-couleur-tempo.fr

// Test du 20/06/2025

// ***********************************

// Renvoie la couleur du jour
const char* urlToday = "https://www.api-couleur-tempo.fr/api/jourTempo/today";
const char* tToday = "today";
#define TODAY 0
// Renvoie la couleur de demain
const char* urlTomorrow = "https://www.api-couleur-tempo.fr/api/jourTempo/tomorrow";
const char* tTomorrow = "tomorrow";
#define TOMORROW 1

const char* url[]={urlToday,urlTomorrow};
const char* ttt[]={tToday,tTomorrow};

void wifiConnect(){
  printf("Connexion au WiFi\n");
  uint32_t cnt=0,wait=500,a=0;
  WiFi.begin("pinks", "cain ne dormant pas songeait au pied des monts");
  while (WiFi.status() != WL_CONNECTED) { 
    delay(wait);printf(".");
    cnt+=wait;
    if(cnt>60000){
      printf("failed\r");
      cnt=0;a++;delay(600000);
      printf("attempt#%d ",a);
    }
  }
  printf("\nWiFi connecté !      \n");  
}

void getTempo(uint8_t tt) {

  printf("%c\n",*url[tt]);
  
  HTTPClient http;
  http.begin((String)*url[tt]);
  int httpCode = http.GET();  // Exécute la requête GET

  if (httpCode > 0) {         // Si le serveur a répondu
    String payload = http.getString();   // Récupère la réponse au format texte
    printf("Réponse brute %c:\n",ttt[tt]);
    printf("%c\n",payload);

    // Document JSON pour ArduinoJson (512 octets suffisent après test)
    StaticJsonDocument<512> doc;

    // Décode le texte JSO
    if (deserializeJson(doc, payload) == DeserializationError::Ok) {

      const char* color = doc["libCouleur"] | "INCONNU";  // Lit la valeur "couleur"
      printf("%c : %c\n",ttt[tt],color);
    } 
    else {printf("Erreur parsing JSON (%c)",ttt[tt]);
    }

  } 
  else {printf("Erreur HTTP (%c): %d\n", ttt[tt],httpCode);}
  http.end();  // Ferme la connexion HTTP
}

void setup() {

  Serial.begin(115200); // Démarre la console série
  Serial.println("+test tempo ");

  my_lcd.init();
  my_lcd.fillScreen(BLACK);
  my_lcd.setRotation(0);
  
  my_lcd.setTextColor(BLUE);
  my_lcd.drawString("ST7789", 0,10,2); 

  wifiConnect();

  // Récupère les couleurs
  getTempo(TODAY);
  getTempo(TOMORROW);
  
   //my_lcd.drawString(color_name[i], 0, ((my_lcd.height()/cnum)-16)/2+(my_lcd.height()/cnum)*i,2);
   // my_lcd.setTextColor(RED);
   //my_lcd.drawString("Hello World!", 0, 0,1);
   //my_lcd.setTextColor(YELLOW);
   //my_lcd.drawFloat(01234.56789, 5, 0, 8,2);
   //my_lcd.fillRect(n*my_lcd.width()/32,48,(n+1)*my_lcd.width()/32,64,my_lcd.color565(n*8, n*8, n*8)&color_mask[rotation])
  

}



void loop() {}



// ------------------------------------------------------
// FONCTION : RÉCUPÉRER LA COULEUR TEMPO DU JOUR
// ------------------------------------------------------

void getTempoToday() {

  HTTPClient http;
  http.begin(urlToday);
  int httpCode = http.GET();  // Exécute la requête GET

  if (httpCode > 0) {         // Si le serveur a répondu
    String payload = http.getString();   // Récupère la réponse au format texte
    printf("Réponse brute today:\n");
    printf("%c\n",payload);

    // Document JSON pour ArduinoJson (512 octets suffisent après test)
    StaticJsonDocument<512> doc;

    // Décode le texte JSO
    if (deserializeJson(doc, payload) == DeserializationError::Ok) {

      const char* color = doc["libCouleur"] | "INCONNU";  // Lit la valeur "couleur"
      printf("Aujourd'hui : %c\n",color);
    } 
    else {printf("Erreur parsing JSON (today)");
    }

  } 
  else {printf("Erreur HTTP (today): %d\n", httpCode);}
  http.end();  // Ferme la connexion HTTP
}



// ------------------------------------------------------

// FONCTION : RÉCUPÉRER LA COULEUR TEMPO DE DEMAIN

// ------------------------------------------------------

void getTempoTomorrow() {

  HTTPClient http;

  http.begin(urlTomorrow);



  int httpCode = http.GET();

  if (httpCode > 0) {

    String payload = http.getString();

    Serial.println("Réponse brute tomorrow:");

    Serial.println(payload);



    StaticJsonDocument<512> doc;

    if (deserializeJson(doc, payload) == DeserializationError::Ok) {



      const char* color = doc["libCouleur"] | "INCONNU";



      Serial.print("Demain : ");

      Serial.println(color);



    } else {

      Serial.println("Erreur parsing JSON (tomorrow)");

    }

  } else {

    Serial.printf("Erreur HTTP (tomorrow): %d\n", httpCode);

  }



  http.end();

}


