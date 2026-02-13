#include <Arduino.h>
#include <TFT_eSPI.h>
#include <TFT_Touch.h>

#define TOUCH_IRQ 36   // IO36 = EXT0 wakeup
#define TOUCH_CS  33
#define TOUCH_SCK 25
#define TOUCH_DIN 32
#define TOUCH_DOUT 39

TFT_eSPI lcd = TFT_eSPI();
TFT_Touch touch(TOUCH_CS, TOUCH_SCK, TOUCH_DIN, TOUCH_DOUT);
#define TFT_BL 21  // on high

void goToSleep() {
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  lcd.drawString("Going to sleep...", 10, 10, 2);
  delay(2500);
  lcd.drawString("sleeping...      ", 10, 10, 2);
  delay(1000);

  // EXT0 wakeup on LOW level
  esp_sleep_enable_ext0_wakeup((gpio_num_t)TOUCH_IRQ, 0);
  //esp_sleep_enable_ext1_wakeup(1ULL << TOUCH_IRQ, ESP_EXT1_WAKEUP_ALL_LOW);

  pinMode(TOUCH_IRQ, INPUT);

  esp_deep_sleep_start();
  //pinMode(TFT_BL, OUTPUT);
  //digitalWrite(TFT_BL,1);
  //esp_light_sleep_start();
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("+test sleep");
  delay(1000);

  // Detect if we woke from deep sleep
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
  //if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1) {
    lcd.init();
    lcd.setRotation(0);
    lcd.fillScreen(TFT_BLACK);
    lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
    lcd.drawString("Wakeup by TOUCH!", 10, 10, 2);
  } else {
    lcd.init();
    lcd.setRotation(0);
    lcd.fillScreen(TFT_BLUE);
    lcd.setTextColor(TFT_WHITE, TFT_BLUE);
    lcd.drawString("Boot normal", 10, 10, 2);
  }

  delay(3500);
  goToSleep();
}

void loop() {
  // Never reached
}