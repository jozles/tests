// All the mcufriend.com UNO shields have the same pinout.
// i.e. control pins A0-A4.  Data D2-D9.  microSD D10-D13.
// Touchscreens are normally A1, A2, D7, D6 but the order varies
//
// This demo should work with most Adafruit TFT libraries
// If you are not using a shield,  use a full Adafruit constructor()
// e.g. Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define LCD_CS A3   // Chip Select goes to Analog 3
#define LCD_CD A2   // Command/Data goes to Analog 2
#define LCD_WR A1   // LCD Write goes to Analog 1
#define LCD_RD A0   // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#include <SPI.h>   
#include "Adafruit_GFX.h" // Hardware-specific library
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
//#include <Adafruit_TFTLCD.h>
//Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

void setup() {
  
    Serial.begin(115200);

    uint16_t ID = tft.readID(); 
    Serial.print("ID = 0x");
    Serial.println(ID, HEX);
    if (ID == 0xD3D3) ID = 0x9481; // write-only shield
//    ID = 0x9329;                             // force ID
    tft.begin(ID);
}

void loop(void) {

    uint16_t wid = tft.width();
    uint16_t ht  = tft.height();

    int i=0,j=0,k=0,c=0;
    uint8_t aspect;
    uint16_t pixel;
    uint8_t color1=0,color2=1;
    
    const char *aspectname[] = {"PORTRAIT", "LANDSCAPE", "PORTRAIT_REV", "LANDSCAPE_REV"};
    const char *colorname[]  = {"BLACK", "WHITE", "BLUE", "GREEN", "RED", "CYAN", "YELLOW", "MAGENTA"};
    uint16_t color[] = {BLACK, WHITE, BLUE, GREEN, RED, CYAN, YELLOW, MAGENTA};

    uint16_t coordx[] = {20,wid/2,wid-20};
    uint16_t coordy[] = {20,ht/2,ht-20};


        tft.fillScreen(BLACK);
        tft.setRotation(0);
        tft.setCursor(0,0);
        tft.setTextSize(2);
        for(i=0;i<19;i++){
          tft.println(i);
        }
        delay(2000);

        for (uint16_t i = 1; i <= ht; i++) {
                tft.vertScroll(0, ht, i);
                delay(40);
            }
        delay(2000);
    for(i=0;i<sizeof(color)/2;i++){
      
      tft.fillScreen(color[i]);
      delay(1000);
      c=i+1;if(j>sizeof(color)/2){c=0;}
      
      for(j=0;j<2;j++){
        for(k=0;k<3;k++){
          tft.drawLine(coordx[j],coordy[k],coordx[2-j],coordy[2-k], color[c]);
          delay(500);
          if(j==1)break;  
        }
      }
      
      tft.setTextSize(1);tft.setTextColor(color[c]);tft.setCursor(50,100);tft.print("ID: 0x");tft.print(tft.readID(), HEX);
      tft.print(" ");tft.print(wid);tft.print("x");tft.print(ht);
      delay(2000);}
    
      for(aspect=0;aspect<4;aspect++){

        tft.fillScreen(BLACK);
        tft.setRotation(aspect);
        tft.setCursor(0,100);

        for(i=0;i<4;i++){
          color1=2*i;color2=color1+1;
          tft.setTextColor(color[color1], color[color2]);         
          tft.setTextSize(2);
          tft.print(aspectname[aspect]);tft.print("  dir ");tft.println(aspect);
          delay(1000);
        }
      }


    //tft.drawCircle(x, y, radius, color);
    //tft.fillCircle(x, y, radius, color);
    //tft.drawRoundRect(cx - i2, cy - i2, i, i, i / 8, tft.color565(red, 0, 0));
    //tft.fillRoundRect(cx - i2, cy - i2, i, i, i / 8, tft.color565(0, green, 0));
    //tft.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i, tft.color565(i, i, 0));
    //tft.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i, tft.color565(0, i, i));
    //tft.drawRect(cx - i2, cy - i2, i, i, color);        
    //tft.fillRect(cx - i2, cy - i2, i, i, color1);
    //tft.drawFastHLine(0, y, w, color1);
    //tft.drawFastVLine(x, 0, h, color2);
    
}




#if defined(MCUFRIEND_KBV_H_)
uint16_t scrollbuf[320];    // my biggest screen is 320x480
#define READGRAM(x, y, buf, w, h)  tft.readGRAM(x, y, buf, w, h)
#else
uint16_t scrollbuf[320];    // Adafruit only does 240x320
// Adafruit can read a block by one pixel at a time
int16_t  READGRAM(int16_t x, int16_t y, uint16_t *block, int16_t w, int16_t h)
{
    uint16_t *p;
    for (int row = 0; row < h; row++) {
        p = block + row * w;
        for (int col = 0; col < w; col++) {
            *p++ = tft.readPixel(x + col, y + row);
        }
    }
}
#endif

void windowScroll(int16_t x, int16_t y, int16_t wid, int16_t ht, int16_t dx, int16_t dy, uint16_t *buf)
{
    if (dx) for (int16_t row = 0; row < ht; row++) {
            READGRAM(x, y + row, buf, wid, 1);
            tft.setAddrWindow(x, y + row, x + wid - 1, y + row);
            tft.pushColors(buf + dx, wid - dx, 1);
            tft.pushColors(buf + 0, dx, 0);
        }
    if (dy) for (int16_t col = 0; col < wid; col++) {
            READGRAM(x + col, y, buf, 1, ht);
            tft.setAddrWindow(x + col, y, x + col, y + ht - 1);
            tft.pushColors(buf + dy, ht - dy, 1);
            tft.pushColors(buf + 0, dy, 0);
        }
}

