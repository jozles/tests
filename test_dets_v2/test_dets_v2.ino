#include "powerSleep.h"
#include "hard.h"

int k=0;
char c='\0';
float volts=0;                           // tension alim (VCC)
float temp;

void setup() {}

void loop(){  
  Serial.begin(115200);delay(100);
  Serial.println("start");delay(10);
  Serial.println("blink avec delay V alider");
  pinMode(LED,OUTPUT);
  while(!Serial.available()){digitalWrite(LED,HIGH);delay(500);digitalWrite(LED,LOW);delay(500);}
  c=Serial.read();if(c!='V'){Serial.println(c);while(1){};}
  
  Serial.print("blink avec sleep interne ");delay(10);
  testSleep('I');
  Serial.print("blink avec sleep externe ");delay(10);
  testSleep('E');
    
  if(c='V'){
    getVT();
    Serial.print(" voltage ");Serial.print(volts);getVT();Serial.print(" voltage ");Serial.println(volts);
    Serial.print(" température ");Serial.println(temp);
    delay(10);
  }
}

void testSleep(char mode)
{
  c='\0';
  while(c!='V'){
    k++;
    switch (mode){
      case 'I':sleepPwrDown(T2000);break;
      case 'E':sleepPwrDown(0);break;
      default: break;
    }      
    hardwarePowerUp();
    for(int i=0;i<4;i++){
      digitalWrite(LED,HIGH);delay(1);
      digitalWrite(LED,LOW);
    sleepPwrDown(T250);
    hardwarePowerUp();
    }

   Serial.begin(115200);delay(100);
    Serial.print(" 2 sec pour V alider ");
    delay(2000);
    c='\0';
    if(Serial.available()){
      c=Serial.read();
      Serial.println((char)c);delay(1);}
  }
}

void getVT()                     // get unregulated voltage and reset watchdog for external timer period 
{
  unsigned long t=micros();

  bitSet(PORT_VCHK,BIT_VCHK);               //digitalWrite(VCHECK,VCHECKHL);
  bitSet(DDR_VCHK,BIT_VCHK);                //pinMode(VCHECK,OUTPUT);  
  
  volts=adcRead(VADMUXVAL,VFACTOR,0,0,1);
  delayMicroseconds(1000);                  // MCP9700 stabilize
  temp=adcRead(TADMUXVAL,TFACTOR,TOFFSET,TREF,0);
/* step 0.25 ***  
  uint16_t temp0=((int)temp)*100,temp1=(int)(temp*100); 
  if((temp1-temp0)>=12 && (temp1-temp0)<38){temp0+=25;}
  else if((temp1-temp0)>=38 && (temp1-temp0)<63){temp0+=50;}
  else if((temp1-temp0)>=63 && (temp1-temp0)<88){temp0+=75;}
  else if((temp1-temp0)>=88){temp0+=100;}
  temp=(float)temp0/100;
*/
/* step 0.1 ***/
  temp=(float)((int)(temp*10))/10;

  bitClear(PORT_VCHK,BIT_VCHK);  
  bitClear(DDR_VCHK,BIT_VCHK);      //pinMode(VCHECK,INPUT);              // reset pulse strobe released 

  ADCSRA &= ~(1<<ADEN);                   // ADC shutdown for clean next voltage measurement
}

float adcRead(uint8_t admuxval,float factor, uint16_t offset, uint8_t ref,uint8_t dly)      // dly=1 if ADC halted
{
    uint16_t a=0;
    
    ADCSRA |= (1<<ADEN);                    // ADC enable to write ADMUX
    ADMUX   = admuxval;
    ADCSRA  = 0 | (1<<ADEN) | (1<<ADSC) | (1<<ADIF) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);   // ADC enable + start conversion + prescaler /16

    delayMicroseconds(40+dly*48);           // ok with /16 prescaler @8MHz

    a=ADCL;
    a+=ADCH*256;
//  Serial.println();Serial.print(" a=");Serial.print(a);Serial.print("  factor=");Serial.println(factor*100);
    return (float)(a*factor-(offset))+ref;
}
