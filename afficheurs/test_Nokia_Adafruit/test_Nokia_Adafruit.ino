#include <avr/dtostrf.h>         // pour DUE seulement
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// Declare LCD object for software SPI
// Adafruit_PCD8544(CLK,DIN,D/C,CE,RST);
#define DISPRST 41
#define SPICS   43
#define DATACD  45
#define MOSI    47
#define SPICLK  49
#define DISPVCC 51
#define DISPBL  53

Adafruit_PCD8544 display = Adafruit_PCD8544(SPICLK, MOSI, DATACD, SPICS, DISPRST);

int rotatetext = 1;

/*  INA219  */

#define SLAVE 64 // (A0,A1=LOW)
#define CONFREG 0
#define CALIBREG 5
//#define DEFCONF 0x3DDF      /* 32V, /8, moyenne sur 8 conv, mode continu */
#define DEFCONF 0x25FF      /* 32V, /1, moyenne sur 128 conv, mode continu */
#define RSHUNT 100          /* valeur resistance shunt */
#define POSDIV 11           /* nbre shifts pour positionner les bits du diviseur (00=1 01=2 10=4 11=8) */
#define MASKDIV 0xE7FF      /* mask pour POSDIV */
#define CONVTIME 68         /* délais 128 conversions */

uint8_t slave = SLAVE;
int oldInaV = 0;
float oldInaA = 0, f = 0;
uint16_t defconf=DEFCONF;
uint16_t g;                  // facteur de gain pour atténuateur INA (0 à 3)

  uint16_t cnt=0;

void inaAmpVolt(float* amp, int* volt);
void ina219_write(uint8_t slave, uint8_t reg, uint16_t data);
int16_t ina219_read(uint8_t slave, uint8_t reg);

void setup()   {
  Serial.begin(115200);Serial.println("5110/INA219 ready");

    /* Wire */
  Wire.begin();
  ina219_write(slave, CALIBREG, 4096);         // init cal reg
  ina219_write(slave, CONFREG, defconf);       // init config 
  delay(9);                                   
#define NBWIREG 6  
  for(int i=0;i<NBWIREG;i++){
    Serial.print(ina219_read(slave,i),HEX);Serial.print(" ");
  }
  Serial.println();
  


  //Initialize Display
  pinMode(DISPVCC,OUTPUT);digitalWrite(DISPVCC,HIGH);
  pinMode(DISPBL,OUTPUT);digitalWrite(DISPBL,HIGH);delay(1000);digitalWrite(DISPBL,LOW);
  display.begin();
  display.setContrast(57);
  display.clearDisplay();

  textDisp("Hi man !",0,0,10,BLACK,1);delay(2000);  
  //testDisplay();
}

void textDisp(char* text,bool clDisp,uint8_t xpos,uint8_t ypos,uint8_t tColor,uint8_t tSize)
{ 
  display.setRotation(2);
  display.setTextSize(tSize);
  display.setTextColor(tColor);
  display.setCursor(xpos,ypos);
  display.println(text);
  display.display();
  if(clDisp){display.clearDisplay();}
}


void ina219_write(uint8_t slave, uint8_t reg, uint16_t data)
{
  Wire.beginTransmission(slave);
  Wire.write(reg);
  Wire.write(data / 256);
  Wire.write(data % 256);
  Wire.endTransmission();
}

int16_t ina219_read(uint8_t slave, uint8_t reg)
{
  Wire.beginTransmission(slave);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom(slave, 2);
  int16_t data = 0;
  uint8_t cnt=0;
  while (Wire.available())     // slave may send less (or more) than requested
  {
    cnt++;
    data *= 256;
    data += Wire.read();
  }
  if(cnt==0){data=-9999;}
  Serial.print(data);Serial.print(" ");
  return data;
}

void inaAmpVolt(float* amp, int* volt)
{
  int v;
  v = ina219_read(slave, 2); *volt = (v >> 3) * 4;
  v = ina219_read(slave, 4); *amp = (float)v*10 / RSHUNT;    // RSHUNT in Ohms *amp in uA
  
  if((v)<4000){g=0;}
  else if(v<8000){g=1;}
  else if(v<16000){g=2;}
  else g=3;
}

void ajustInaAmpVolt(float* amp, int* volt)
{
  uint16_t gain=(defconf & ~MASKDIV)>>POSDIV;

    delay(CONVTIME);
    inaAmpVolt(amp,volt);
Serial.print(*amp);Serial.print(" ");Serial.print(*volt);Serial.print(" ");Serial.print(defconf,HEX);Serial.print(" ");Serial.println(gain,HEX);
  
    if(gain!=g){ 
      defconf &= MASKDIV ;defconf |= g<<POSDIV;
      ina219_write(slave, CONFREG, defconf);       // init config diviseur plus grand
      delay(CONVTIME);
      inaAmpVolt(amp,volt);
    }
  
}

void loop() 
{

  float amp;
  int volt;
  #define LVALUE 8
  char value[LVALUE];memset(value,0x00,LVALUE);

  cnt++;
  
  ajustInaAmpVolt(&amp,&volt);           // volt in V amp in uA 
  display.clearDisplay();

  dtostrf(cnt, 5, 0, value);
  textDisp(value,0,15,0,BLACK,1);
  dtostrf((float)volt/1000, 5, 3, value);
  textDisp("Volts ",0,0,10,BLACK,1);
  Serial.print(cnt);
  Serial.print("/volts=");Serial.print(value);
  textDisp(value,0,42,10,BLACK,1);
  textDisp("mAmps ",0,0,30,BLACK,1);  

#if RSHUNT==100
#define OFFSET 29.2
#define DECIM  4
#endif
#if RSHUNT==10
#define OFFSET 28
#define DECIM  3
#endif
#if RSHUNT==1
#define OFFSET 9
#define DECIM  2
#endif

  dtostrf((amp-OFFSET)/(1000), 6, DECIM, value);    // mA
  Serial.print(" amps=");Serial.println(amp);
  textDisp(value,0,42,30,BLACK,1);
  memcpy(value,"offset ",7);
  dtostrf(OFFSET, 6, 2, value+7);              // mA
  textDisp(value,0,0,40,BLACK,1);
delay(1000);
}


void testDisplay()
{
  // Display Inverted Text
  display.setTextColor(WHITE, BLACK); // 'inverted' text
  display.setCursor(0,0);
  display.println("Hello world!");
  display.display();
  delay(2000);
  display.clearDisplay();

  // Scaling Font Size
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.setTextSize(2);
  display.println("Hello!");
  display.display();
  delay(2000);
  display.clearDisplay();

  // Display Numbers
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println(123456789);
  display.display();
  delay(2000);
  display.clearDisplay();

  // Specifying Base For Numbers
  display.setCursor(0,0);
  display.print("0x"); display.print(0xFF, HEX); 
  display.print("(HEX) = ");
  display.print(0xFF, DEC);
  display.println("(DEC)"); 
  display.display();
  delay(2000);
  display.clearDisplay();

  // Display ASCII Characters
  display.setCursor(0,0);
  display.setTextSize(2);
  display.write(3);
  display.display();
  delay(2000);
  display.clearDisplay();

  // Text Rotation
  uint8_t cnt=0;
  while(cnt<8)
  {
  display.clearDisplay();
  display.setRotation(rotatetext);  // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("Text Rotation");
  display.display();
  delay(1000);
  display.clearDisplay();
  rotatetext++;
  cnt++;
  }
}
