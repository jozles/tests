


#define  TIM_DIV1   0  //80MHz (80 ticks/us - 104857.588 us max)
#define  TIM_DIV16  1  //5MHz (5 ticks/us - 1677721.4 us max)
#define  TIM_DIV256 3  //312.5Khz (1 tick = 3.2us - 26843542.4 us max)


long cnt=0;
bool chg;

void testisr()
{
  cnt++;
  chg=!chg;
}


void setup() {

Serial.begin(115200);

      timer1_isr_init();
      timer1_attachInterrupt(testisr);
      timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
      timer1_write(31200);

}

void loop() {

 if(millis()%10000==0){Serial.println(cnt);delay(1);}

}
