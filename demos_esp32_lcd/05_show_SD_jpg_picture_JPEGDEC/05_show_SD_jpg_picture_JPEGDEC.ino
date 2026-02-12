#include <TFT_eSPI.h>
#include <SD.h>
#include <JPEGDEC.h>

TFT_eSPI tft = TFT_eSPI();
JPEGDEC jpeg;

#define FILE_NUMBER 4
#define FILE_NAME_SIZE_MAX 20
char file_name[FILE_NUMBER][FILE_NAME_SIZE_MAX];

int jpegDraw(JPEGDRAW *pDraw) {
    uint16_t *src = (uint16_t*)pDraw->pPixels;
    uint16_t *dst = src;

    // swap des octets pour chaque pixel
    for (int i = 0; i < pDraw->iWidth * pDraw->iHeight; i++) {
        uint16_t c = src[i];
        dst[i] = (c >> 8) | (c << 8);   // inversion MSB/LSB
    }

    tft.pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, dst);
  
    //tft.pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight,
    //              (uint16_t*)pDraw->pPixels);
    return 1;
}

void showJpgFromSD(const char *filename) {
    File jpgFile = SD.open(filename);
    if (!jpgFile) {
        Serial.println("Erreur ouverture JPG");
        return;
    }

    jpeg.open(jpgFile, jpegDraw);   // ouverture via File
    jpeg.decode(0, 0, 0);           // x, y, scale
    jpeg.close();

    jpgFile.close();
}

void setup() {
  
  Serial.begin(115200);
  Serial.println("+test_jpg_esp32_lcd");

  strcpy(file_name[0],"/tulip.jpg");
  strcpy(file_name[1],"/game.jpg");
  strcpy(file_name[2],"/tree.jpg");
  strcpy(file_name[3],"/flower.jpg");
  
    tft.init();
    tft.setRotation(1);

    tft.fillScreen(TFT_RED);delay(2000);
    tft.fillScreen(TFT_GREEN);delay(2000);
    tft.fillScreen(TFT_BLUE);delay(2000);

    SD.begin();

    while(1){
      for(uint8_t i=0;i<FILE_NUMBER;i++){
      showJpgFromSD(file_name[i]);
      delay(2000);
      }
    }
}

void loop() {}
