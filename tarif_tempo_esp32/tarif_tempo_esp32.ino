#include <WiFi.h>          // Gestion du WiFi sur ESP32
#include <HTTPClient.h>    // Pour faire des requêtes HTTP GET
#include <ArduinoJson.h>   // Pour parser les réponses JSON
#include <TFT_eSPI.h> 
#include <SPI.h>
#include "font.h"
#include <TFT_Touch.h>
#include <esp_adc_cal.h>

#define VERSION "1.1\0"

#define VOLTAGE_PIN 34

#define TOUCH_IRQ 36   // IO36 = EXT0 wakeup
#define TOUCH_CS  33
#define TOUCH_SCK 25
#define TOUCH_DIN 32
#define TOUCH_DOUT 39

TFT_eSPI my_lcd = TFT_eSPI(); 
TFT_Touch my_touch = TFT_Touch(TOUCH_CS, TOUCH_SCK, TOUCH_DIN, TOUCH_DOUT);

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
uint16_t rectx[]={0,0},recth[]={170,125},recty[]={30,5+30+recth[0]},rectw[]={TFT_WIDTH,TFT_WIDTH},txtx[]={rectx[0]+2,rectx[1]+2},txty[]={recty[0]+10,recty[1]+10};
uint8_t txts=2;

char* sdow={"dimanche\0lundi\0  \0mardi\0  \0mercredi\0jeudi\0  \0vendredi\0samedi\0 \0"};

#define BATX 220
#define BATY 4
#define BATH 25
#define BATW 12
#define BATPINH 3
#define BATPINW BATW/3
#define BATCOL WHITE
#define POWCOL GREEN
#define BATLINE 2
#define VPMAX 4.20
#define VPMIN 3.20

uint16_t xpos=160;

void goToSleep() {
  my_lcd.fillRect(0,0,BATX-1,25,BLACK);
  my_lcd.setTextColor(GREEN, BLACK);
  my_lcd.drawString("Going to sleep...", 0, 10, 2);
  delay(2500);
  my_lcd.drawString("sleeping...       ", 0, 10, 2);
  delay(1000);

  esp_sleep_enable_ext0_wakeup((gpio_num_t)TOUCH_IRQ, 0);   // EXT0 wakeup on LOW level
  pinMode(TOUCH_IRQ, INPUT);
  esp_deep_sleep_start();
}

bool wifiConnect(){
  char wifiWait[]={"+\0x\0"}; //{"-\0\\\0|\0/\0"};
  printf("Connexion au WiFi\n");
  my_lcd.drawString("WiFi",xpos, 10, 2);
  uint32_t cnt=0,wait=500,a=0;
  WiFi.begin("pinks", "cain ne dormant pas songeait au pied des monts");
  while (WiFi.status() != WL_CONNECTED) { 
    delay(wait);printf(".");
    my_lcd.drawString(wifiWait+((cnt/wait)%2)*2,xpos+30, 10, 2);
    cnt+=wait;
    if(cnt>30000){
      printf("failed\r");
      //cnt=0;a++;
      //printf("wait 10 min ; attempt#%d ",a);
      //delay(600000);
      char wifiMess[]={"WiFi KO"};
      my_lcd.setTextColor(RED, BLACK);  
      my_lcd.drawString(wifiMess,xpos, 10, 2);
      delay(5000);return false;
    }
  }
  printf("\nWiFi connecté !\n");
  my_lcd.drawString("OK",xpos+30, 10, 2);
  return true;  
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
    my_lcd.setTextColor(RED);
    my_lcd.drawString("Http",xpos+50, 10, 2);
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
    char datefr[]={date[8],date[9],'-',date[5],date[6],'-',date[0],date[1],date[2],date[3],'\0'};
    my_lcd.drawString(datefr,txtx[when]+8*strlen(dl),txty[when],txts);
  }
  else {
    delay(5000);
  }
}

float voltage(uint8_t vp){
    float vbat=3.8;
    my_lcd.fillRect(BATX+BATPINW,BATY-BATPINH,BATPINW,BATPINH,BATCOL);        
    my_lcd.fillRect(BATX,BATY,BATW,BATH,BATCOL);      //drawRoundRect(BATX,BATY,BATW,BATH,2,BATCOL);
    my_lcd.fillRect(BATX+BATLINE,BATY+BATLINE,BATW-2*BATLINE,BATH-2*BATLINE,BLACK);
    uint8_t powh=BATH-2*BATLINE-8; //(int)((vbat-VPMIN)/(VPMAX-VPMIN))*(BATH-2*BATLINE);
    uint8_t powx=BATX+BATLINE;
    uint8_t powy=BATY+BATLINE+powh;
    my_lcd.fillRect(powx,powy,BATW-2*BATLINE,BATH-2*BATLINE-powh,POWCOL);
    
    printf("powx:%d powh:%d\n",powx,powh);delay(1000);
    return vbat;
}

void setup() {

  Serial.begin(115200);
  delay(200);
  Serial.println("+test tempo with deepSleep v1.1");
  delay(200);
  
  my_lcd.init();
  my_lcd.fillScreen(BLACK);
  my_lcd.setRotation(0);  
  my_lcd.setTextColor(BLUE);
  my_lcd.setTextColor(YELLOW, BLACK);
  
  char vers[5];vers[0]='v';memcpy(&vers[1],VERSION,4);
  my_lcd.drawString(vers, 1, 10, 2); 
  
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
    my_lcd.drawString("Touch wakeup", 40, 10, 2); 
  } else
  {
    my_lcd.drawString("Boot normal", 40, 10, 2);
  }
  
  voltage(VOLTAGE_PIN);

  if(!wifiConnect()){goToSleep();};
  
  tempo(TODAY);
  tempo(TOMORROW);
  
  delay(10000);
  goToSleep();
  
}

void loop(){}


