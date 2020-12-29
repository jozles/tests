#include <SPI.h>
#include <TFT.h>


#define LED 4

/* ----------------------- TFT ----------------------- */

/* afficheur SPI TFT 128*160 */

#define TFTH 128
#define TFTW 160
#define TFTCH 10 // ?
#define TFTCW 6

#define SSTFT 2
#define RSTTFT -1
#define DCTFT 3
#define BLTFT A2

TFT tft = TFT(SSTFT, DCTFT, RSTTFT);


char car[]=" \0";
uint32_t t=25,t0=1000;

void setup() {

Serial.begin(115200);

pinMode(LED,OUTPUT);
blink();

Serial.print("beg ");



pinMode(BLTFT,OUTPUT);digitalWrite(BLTFT,HIGH);//delay(1000);digitalWrite(BLTFT,LOW);

  SPI.begin();
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

  tft.begin();
  tft.background(0,0,0);

  char text0[32];memset(text0,0x00,32);
  strcpy(text0,"alim \0");
  tft.setRotation(3);
  tft.setTextColor(0xFFFF,0x0000);
  tft.setTextSize(2);  
  //printAtTft(110,0,text0);
  tft.setCursor(10, 10);
  tft.print(text0);

Serial.print(" 1");






}

void loop() {

  
  //t0+=t;
  Serial.println(t0);
  digitalWrite(LED,HIGH);delay(t);
  digitalWrite(LED,LOW);delay(3*t);
  digitalWrite(LED,HIGH);delay(t);
  digitalWrite(LED,LOW);delay(t0-5*t);
  if(car[0]>=0x7f){tft.background(0,0,0);car[0]=0x20;tft.setCursor(0,0);}
  tft.print(car);
  (uint8_t)car[0]++;
}

void blink()
{
  digitalWrite(LED,HIGH);delay(t);
  digitalWrite(LED,LOW);delay(3*t);
  digitalWrite(LED,HIGH);delay(t);
  digitalWrite(LED,LOW);delay(t0-5*t);
}
