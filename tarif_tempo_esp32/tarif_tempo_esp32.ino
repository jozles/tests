#include <WiFi.h>          // Gestion du WiFi sur ESP32
#include <HTTPClient.h>    // Pour faire des requêtes HTTP GET
#include <ArduinoJson.h>   // Pour parser les réponses JSON
#include <TFT_eSPI.h> 
#include <SPI.h>
#include "font.h"
#include <TFT_Touch.h>
#include <esp_adc_cal.h>
#include "udp_ntp.h"

#define VERSION "1.1\0"

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

char* chexa={"0123456789ABCDEF"};

// ***********************************
// URL DE L’API TEMPO
// https://www.api-couleur-tempo.fr
// ***********************************

// ****** urls

const char* urlToday = "https://www.api-couleur-tempo.fr/api/jourTempo/today";
const char* urlTomorrow = "https://www.api-couleur-tempo.fr/api/jourTempo/tomorrow";

const char* url[]={urlToday,urlTomorrow};

// ****** pour debug

const char* tToday = "today";
const char* tTomorrow = "tomorrow";
const char* ttt[]={tToday,tTomorrow};

// ****** positions/dimensions pavés jours

#define RECT_TODAY_X 0
#define RECT_TODAY_Y 60
#define RECT_TODAY_W TFT_WIDTH
#define RECT_TODAY_H 150
#define SEP_LINE_H 5
#define RECT_TOMORROW_X RECT_TODAY_X
#define RECT_TOMORROW_Y RECT_TODAY_Y+RECT_TODAY_H+SEP_LINE_H
#define RECT_TOMORROW_H 110

uint16_t rectx[]={RECT_TODAY_X,RECT_TOMORROW_X},recth[]={RECT_TODAY_H,RECT_TOMORROW_H},recty[]={RECT_TODAY_Y,RECT_TOMORROW_Y},rectw[]={TFT_WIDTH,TFT_WIDTH},txtx[]={rectx[0]+2,rectx[1]+2},txty[]={recty[0]+10,recty[1]+10};
uint8_t txts=2;

char* sdow={"dimanche\0lundi\0  \0mardi\0  \0mercredi\0jeudi\0  \0vendredi\0samedi\0 \0"};

// ***** batterie

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

#define VOLTAGE_PIN 34

uint16_t wifiXpos=135;  // position x message wifi

// ****** udp/ntp



byte js=0;
uint32_t amj=0, hms=0;

void sleep_ms(uint32_t ms){
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();
  uint64_t us=(uint64_t)ms*1000ULL;
  esp_sleep_enable_timer_wakeup(us);
  esp_light_sleep_start();
}

void goToSleep() {
  my_lcd.fillRect(0,0,BATX-1,25,BLACK);
  my_lcd.setTextColor(GREEN, BLACK);
  my_lcd.drawString("Going to sleep...", 0, 10, 2);
  sleep_ms(2500);
  my_lcd.drawString("sleeping...       ", 0, 10, 2);
  sleep_ms(1000);

  esp_sleep_enable_ext0_wakeup((gpio_num_t)TOUCH_IRQ, 0);   // EXT0 wakeup on LOW level
  
  const uint64_t uS = 3600ULL * 1000000ULL;                 // microsec delay
  esp_sleep_enable_timer_wakeup(uS);                        // wakeup on timer

  pinMode(TOUCH_IRQ, INPUT);
  esp_deep_sleep_start();
}

bool wifiConnect(){
  char wifiWait[]={"+\0x\0"}; //{"-\0\\\0|\0/\0"};
  printf("Connexion au WiFi\n");
  my_lcd.drawString("WiFi",wifiXpos, 10, 2);
  uint32_t cnt=0,wait=500,a=0;
  WiFi.begin("pinks", "cain ne dormant pas songeait au pied des monts");
  while (WiFi.status() != WL_CONNECTED) { 
    //sleep_ms(wait);
    delay(wait);
    printf(".");
    my_lcd.drawString(wifiWait+((cnt/wait)%2)*2,wifiXpos+30, 10, 2);
    cnt+=wait;
    if(cnt>30000){
      printf("failed\r");
      //cnt=0;a++;
      //printf("wait 10 min ; attempt#%d ",a);
      //sleep_ms(600000);
      char wifiMess[]={"WiFi KO"};
      my_lcd.setTextColor(RED, BLACK);  
      my_lcd.drawString(wifiMess,wifiXpos, 10, 2);
      sleep_ms(5000);return false;
    }
  }
  printf("\nWiFi connecté !\n");
  my_lcd.drawString("OK",wifiXpos+30, 10, 2);
  return true;  
}

// ****** acquisition n° couleur du jour

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
    my_lcd.drawString("Http",wifiXpos+50, 10, 2);
  }
  http.end();  // Ferme la connexion HTTP
  return numcolor;
}

int8_t dow(char* date){   // n° jour de la semaine
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

#define TODAY 0
#define TOMORROW 1

void tempo(uint8_t when){     // when = TODAY ou TOMORROW
  #define LD 12
  char date[LD];
  memset(date,0x00,LD);
  uint16_t dayC=3;
  dayC=getTempo(when,date);
  char* dl=(sdow+9*dow(date));    // texte jour de la semaine
  printf("dayC :%d ; %s %s\n",dayC,dl,date);
  if(dayC>=0){
    my_lcd.fillRect(rectx[when],recty[when],rectw[when],recth[when],dayColor[dayC]);
    my_lcd.setTextColor(txtColor[dayC]);
    my_lcd.drawString(dl,txtx[when],txty[when],txts);
    char datefr[]={date[8],date[9],'-',date[5],date[6],'-',date[0],date[1],date[2],date[3],'\0'};
    my_lcd.drawString(datefr,txtx[when]+8*strlen(dl),txty[when],txts);
  }
  else {
    sleep_ms(5000);
  }
}

float voltage(uint8_t vp){
    // ****** mesure tension
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type=esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    uint32_t raw=analogRead(VOLTAGE_PIN);
    float vbat=(float)(esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2)/1000;
  
    // ****** affichage tension
    my_lcd.setRotation(3);
    my_lcd.drawFloat(vbat,2,TFT_HEIGHT-BATH-2,BATX-10,1);    // tension
    my_lcd.setRotation(0);
    
    // ****** affichage picto
    uint16_t powcol=POWCOL,batcol=BATCOL;
    if(vbat<=VPMIN+.1){powcol=RED;batcol=RED;}
    my_lcd.fillRect(BATX+BATPINW,BATY-BATPINH,BATPINW,BATPINH,batcol);        
    my_lcd.fillRect(BATX,BATY,BATW,BATH,batcol);      //drawRoundRect(BATX,BATY,BATW,BATH,2,batcol);
    my_lcd.fillRect(BATX+BATLINE,BATY+BATLINE,BATW-2*BATLINE,BATH-2*BATLINE,BLACK);
    uint8_t powh=BATH-2*BATLINE-(int)(((vbat-VPMIN)/(VPMAX-VPMIN))*(BATH-2*BATLINE));
    uint8_t powx=BATX+BATLINE;
    uint8_t powy=BATY+BATLINE+powh;
    my_lcd.fillRect(powx,powy,BATW-2*BATLINE,BATH-2*BATLINE-powh,powcol);
    
    printf("vbat:%fV powx:%d powh:%d\n",vbat,powx,powh);sleep_ms(1000);
    return vbat;
}

void bootReason()
{
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  #define REASL 12
  char reasont[]={"Touch boot \0Timed boot \0PowerOn boot"};
  uint8_t reason=2;
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:  reason=0;break;
    case ESP_SLEEP_WAKEUP_TIMER: reason=1;break;
    default: reason=2;break;
  }
  char* rt=reasont+REASL*reason;
  my_lcd.drawString(rt,32, 10, 2);
  printf("%s",rt);
}

void setup() {

  Serial.begin(115200);
  sleep_ms(200);
  Serial.println("+test tempo with deepSleep v1.1");
  sleep_ms(200);
  
  my_lcd.init();
  my_lcd.fillScreen(BLACK);
  my_lcd.setRotation(0);  
  my_lcd.setTextColor(BLUE);
  my_lcd.setTextColor(YELLOW, BLACK);
  
  char vers[5];vers[0]='v';memcpy(&vers[1],VERSION,4);
  my_lcd.drawString(vers, 1, 10, 2); 
  
  bootReason();
  
  voltage(VOLTAGE_PIN);

  if(!wifiConnect()){goToSleep();};
  
  if(!getUDPdate(&hms,&amj,&js)){
    printf("udp_ntp ko\n");
    my_lcd.drawString("ntp KO",0,RECT_TODAY_Y-30,txts);
  }
  else {
    char dd[64];
    memcpy(dd,(char*)(sdow+9*js),8);dd[8]=' ';convIntToString(&dd[9],amj);dd[17]=' ';convIntToString(&dd[18],hms);memcpy(&dd[24]," GMT\0",5);
    my_lcd.drawString(dd,0,RECT_TODAY_Y-30,txts);
    Serial.print(js);Serial.print(" ");Serial.print(amj);Serial.print(" ");Serial.print(hms);Serial.println(" GMT");
  }
  
  tempo(TODAY);
  tempo(TOMORROW);
  
  sleep_ms(20000);
  goToSleep();
  
}

void loop(){}

int convIntToString(char* str,int32_t num,uint8_t len)
{
  int i=0,t=0,num0=num;
  if(num<0){i=1;str[0]='-';}
  while(num0!=0){num0/=10;i++;}             // comptage nbre chiffres partie entière
  if(len!=0){i=len;}                        // len!=0 complète avec des 0 ou troncate
  t=i;
  for (i=i;i>0;i--){num0=num%10;num/=10;str[i-1]=chexa[num0];}
  str[t]='\0';
  if(str[0]=='\0'){str[0]='0';}
  return t;
}

int convIntToString(char* str,int32_t num)
{
  return convIntToString(str,num,0);
}
