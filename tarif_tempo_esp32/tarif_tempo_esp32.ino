#include <WiFi.h>          // Gestion du WiFi sur ESP32
#include <HTTPClient.h>    // Pour faire des requêtes HTTP GET
#include <ArduinoJson.h>   // Pour parser les réponses JSON
#include <TFT_eSPI.h> 
#include <SPI.h>
#include "font.h"
#include <TFT_Touch.h>

#define RTP_DOUT 39
#define RTP_DIN  32
#define RTP_SCK  25
#define RTP_CS   33
#define RTP_IRQ  36

TFT_eSPI my_lcd = TFT_eSPI(); 
TFT_Touch my_touch = TFT_Touch(RTP_CS, RTP_SCK, RTP_DIN, RTP_DOUT);

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

// ***********************************
// URL DE L’API TEMPO
// https://www.api-couleur-tempo.fr
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

// dimensions
uint16_t rectx[]={0,0},recth[]={200,100},recty[]={30,30+recth[0]},rectw[]={TFT_WIDTH,TFT_WIDTH},txtx[]={rectx[0]+2,rectx[1]+2},txty[]={recty[0]+10,recty[1]+10};
uint8_t txts=2;

char* sdow={"dimanche\0lundi\0  \0mardi\0  \0mercredi\0jeudi\0  \0vendredi\0samedi\0 \0"};

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
  Serial.println((char*)url[tt]);
  
  const char* color="Inconnu";
  int numcolor=-1;
  
  HTTPClient http;
  http.begin((String)(char*)url[tt]);
  int httpCode = http.GET();  // Exécute la requête GET

  if (httpCode > 0) {         // Si le serveur a répondu
    String payload = http.getString();   // Récupère la réponse au format texte
    printf("%s\n",payload.c_str());

    StaticJsonDocument<512> doc;
    if (deserializeJson(doc, payload) == DeserializationError::Ok) {

      color = doc["libCouleur"];  // Lit la valeur "couleur"
      strcpy(d,doc["dateJour"] | "INCONNU");
      printf("%s : %s ; date : %s\n",(char*)ttt[tt],color,d);
      numcolor=(strstr(validColors,color)-validColors)/LENVCOLOR;
    } 
    else {
      printf("Erreur parsing JSON (%c)",(char*)ttt[tt]);
      numcolor=-100;
    }
  } 
  else {
    printf("Erreur HTTP (%c): %d\n", (char*)ttt[tt],httpCode);
    numcolor=httpCode;
  }
  http.end();  // Ferme la connexion HTTP
  return numcolor;
}

int8_t dow(char* date){
  uint8_t monthl[]={31,28,31,30,31,30,31,31,30,31,30}; 
  uint8_t yy=(date[2]-48)*10+date[3]-48;if(yy>50){return-1;} // yy 
  uint8_t my=(date[5]-48)*10+date[6]-48;if(my<0 || my>11){return-1;} // mm
  uint16_t dy=(date[8]-48)*10+date[9]-48;if(dy>monthl[my]){return-1;} // dd
  for(uint8_t i=0;i<(my-1);i++){dy+=monthl[i];}
  uint16_t ndow=6; // samedi 2000/01/01
  ndow=6+yy*365+yy/4+dy;
  if(my<3 && ((yy%4)==0)){ndow--;};ndow%=7;
  return ndow;
}

void tempo(uint8_t when){
  #define LD 12
  char date[LD];
  memset(date,0x00,LD);
  uint16_t dayC=3;
  dayC=getTempo(when,date);
  char* dl=(sdow+9*dow(date));
  printf("dayC : %d ; %s %s\n",dayC,dl,date);
  if(dayC>=0){
    my_lcd.fillRect(rectx[when],recty[when],rectw[when],recth[when],dayColor[dayC]);
    my_lcd.setTextColor(txtColor[dayC]);
    my_lcd.drawString(dl,txtx[when],txty[when],txts);
    my_lcd.drawString(date,txtx[when]+8*strlen(dl),txty[when],txts);
  }
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
  
  my_touch.setCal(495, 3398, 721, 3448, 320, 240, 1);
  my_touch.setRotation(0);

  wifiConnect();
  
  tempo(TODAY);
  tempo(TOMORROW);
  
}

void loop(){}


