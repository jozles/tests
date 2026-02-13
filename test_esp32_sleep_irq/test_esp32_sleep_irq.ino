
extern "C" {
#include "esp_core_dump.h"
}

void disableCoreDump() __attribute__((constructor));
void disableCoreDump() {
    esp_core_dump_to_flash_disable();
}




#include "esp_sleep.h"
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

#define TOUCH_IRQ 36

uint32_t timesleep=millis();

void setup() {
  Serial.begin(115200); // Démarre la console série
  delay(2000);
  Serial.println("+test sleep/réveil ");

  my_lcd.init();
  my_lcd.fillScreen(BLACK);
  my_lcd.setRotation(0);
  
  my_lcd.setTextColor(BLUE);
  my_lcd.drawString("ST7789", 0,10,2);
  
  pinMode(TOUCH_IRQ, INPUT);  // pas de pull-up interne !
  esp_sleep_enable_ext0_wakeup((gpio_num_t)TOUCH_IRQ, 0); // 0 = réveil sur LOW

}

void loop() {
  if(millis()<(timesleep+10000)){
    Serial.println("dodo");
    
    my_lcd.drawString("Dodo  ", 0,30,2);
    delay(1000);
    
    esp_deep_sleep_start();
    Serial.println("debout");
    my_lcd.drawString("Debout", 0,30,2);
    timesleep=millis();
    delay(1000);
  }
}

