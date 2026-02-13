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

const uint16_t dayColor[]={BLUE,WHITE,RED,BLACK};
const uint16_t txtColor[]={BLACK,BLUE,BLACK,WHITE};
const char* validColors="Bleu BlancRougeInconnu";
#define LENVCOLOR 5

#define LD 12
char date[LD];

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

int getTempo(uint8_t tt,char* d) {

  //printf("%s\n",url[tt]);
  Serial.println((char*)url[tt]);
  
  const char* color="Inconnu";
  
  HTTPClient http;
  http.begin((String)(char*)url[tt]);
  int httpCode = http.GET();  // Exécute la requête GET

  if (httpCode > 0) {         // Si le serveur a répondu
    String payload = http.getString();   // Récupère la réponse au format texte
    printf("Réponse brute %s:\n",(char*)ttt[tt]);
    printf("%s\n",payload.c_str());

    // Document JSON pour ArduinoJson (512 octets suffisent après test)
    StaticJsonDocument<512> doc;

    // Décode le texte JSO
    if (deserializeJson(doc, payload) == DeserializationError::Ok) {

      color = doc["libCouleur"];  // Lit la valeur "couleur"
      strcpy(d,doc["dateJour"] | "INCONNU");
      printf("%s : %s ; date : %s\n",(char*)ttt[tt],color,d);
    } 
    else {
      printf("Erreur parsing JSON (%c)",(char*)ttt[tt]);
    }
  } 
  else {printf("Erreur HTTP (%c): %d\n", (char*)ttt[tt],httpCode);}
  http.end();  // Ferme la connexion HTTP
  return (strstr(validColors,color)-validColors)/LENVCOLOR;
}

void setup() {

  Serial.begin(115200); // Démarre la console série
  delay(2000);
  Serial.println("+test tempo ");

  my_lcd.init();
  my_lcd.fillScreen(BLACK);
  my_lcd.setRotation(0);
  
  my_lcd.setTextColor(BLUE);
  my_lcd.drawString("ST7789", 0,10,2); 

  wifiConnect();

  // Récupère les couleurs
  uint16_t dayC=3;
  
  memset(date,0x00,LD);
  dayC=getTempo(TODAY,date);
  printf("dayC : %d ; date : %s\n",dayC,date);
  my_lcd.fillRect(0,30,240,200,dayColor[dayC]);
  my_lcd.setTextColor(txtColor[dayC]);
  my_lcd.drawString(date,5,40,2);  
  
  memset(date,0x00,LD);
  dayC=getTempo(TOMORROW,date);
  printf("dayC : %d ; date : %s\n",dayC,date);
  my_lcd.fillRect(0,230,240,100,dayColor[dayC]);
  my_lcd.setTextColor(txtColor[dayC]);
  my_lcd.drawString(date,5,240,2);
  
  
   //my_lcd.drawString(color_name[i], 0, ((my_lcd.height()/cnum)-16)/2+(my_lcd.height()/cnum)*i,2);
   // my_lcd.setTextColor(RED);
   //my_lcd.drawString("Hello World!", 0, 0,1);
   //my_lcd.setTextColor(YELLOW);
   //my_lcd.drawFloat(01234.56789, 5, 0, 8,2);
  //fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);
  
  
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


