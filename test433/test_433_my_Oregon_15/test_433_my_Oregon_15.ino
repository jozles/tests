/*Status:  Yet another Oregon Scientific Weather Station 433Mhz Arduino Signal Intercepter
 *Open Source
 *Author: Rob Ward December 2013
 *Finds the header bits and synch's to the waveform's 1->0 going edges, which are '1's
 *Soaks up rest of header bits and waits for first 0
 *Accumulates incoming data stream into bytes, arranged from MSB to LSB by rotating a mask in a Left Rotate
 *Checks for Sensor ID's, the ID's used here are relative to the previous way the data is collected, so not the same as others
 *Using the rotate bit on the fly the two ID bytes are as follows, this program just use first byte to ID the sensor
 *        Oregon-THGN800   0xAF  	Outside Temp-Hygro    AF 82 ?? 4 ?? 1 == (Sensor switch nybble 1,2or3)
 *        Oregon-PCR800    0xA2 	Rain Gauge            A2 91 ?? 4 ?? 0
 *        Oregon-WGR800    0xA1 	Anemometer            A1 98 ?? 4 ?? 0
 *Rolling code is present to differentiate between alternative sensors
 *Calculate the check sum for the sensor and if data passes the test...continue
 *Decode the nybbles (if bites are bytes, nybbles are nybbles :-) and calculate the parameters for each sensor
 *Dump the calculations to the screen, round off to decimal places
 
 Why not use interrrupts and count durations?  Manchester encoding came from the time of slow chips, noisey environments and saggy waveforms.
 The beauty of Manchester encoding is that is it can be sampled so the logic transitions are the most important and at least the logic 
 state in the middle of the timing periods is most likely to be valid.  It also self synchronises throughout the packet and 
 automatically detects timeouts.  This is an old, classic Manchester decoding decoding strategy, but quite robust never the less.
 
 To do:
 *find the battery indicators if they exist. (7) suggests it is (my) upper 4 bits of byte 2.  How to fake old batteries??
 *Add a 1 minute dump of current values in CSF format for my WWW Weather station (Remove current debug sensor dump format).
 
 Reference Material:
 Thanks to these authors, they all helped in some way, especially, the last one Brian!!!!
 http://wmrx00.sourceforge.net/Arduino/OregonScientific-RF-Protocols.pdf (1)
 http://jeelabs.net/projects/cafe/wiki/Decoding_the_Oregon_Scientific_V2_protocol (2)
 https://github.com/phardy/WeatherStation (3)
 http://lucsmall.com/2012/04/27/weather-station-hacking-part-1/ (4)
 http://lucsmall.com/2012/04/29/weather-station-hacking-part-2/ (5)
 http://lucsmall.com/2012/04/29/weather-station-hacking-part-2/ (6)
 http://www.mattlary.com/2012/06/23/weather-station-project/(7)
 https://github.com/lucsmall/WH2-Weather-Sensor-Library-for-Arduino (8)
 http://www.lostbyte.com/Arduino-OSV3/ (9) brian@lostbyte.com
 
 Most of all thanks to Oregon Scientific, who have produced an affordable, high quality product.  I can now have my LCD Home Base in the
 kitchen to enjoy, with the Arduino in the garage also capturing data for WWW Weather pages.  Lovely!!!!  http://www.oregonscientific.com
 Very Highly recommended equipment. Rob Ward
 */

// Read data from 433MHz receiver on digital pin 8
#define RxPin 3    //Just an input
#define ledPin 13  //Human feedback
#define sDelay 230  //One Quarter Manchester Bit duration, is this OK?
#define lDelay 460  //One Half Manchester Bit duration
#define HEADERLEN 15
byte    headerHits = 0; //How many ones detected as a header bit (minimum HEADERLEN)
boolean header = false; //State of header detection
boolean logic = false; //State of the Manchester decoding
byte    signal = 0; //state of RF
boolean test230 = false;
boolean test460 = false;
int     maxBytes = 10; //sets the limits of how many data bytes will be required
int     nosBytes = 0; //counter for the data bytes required
boolean firstZero = false; //flags when the first '0' is found.
byte    dataByte = 0; //accumulates the bits of the signal
byte    dataMask = 16; //rotates, so allows nybbles to be reversed
byte    nosBits = 0; //counts the shifted bits in the dataByte
byte    manchester[11]; //storage array for the data accumulated via manchester format
const char windDir[16][4] = {  
  "N  ", "NNE", "NE ", "ENE",  "E  ", "ESE", "SE ", "SSE",  "S  ", "SSW", "SW ", "WSW",  "W  ", "WNW", "NW ", "NNW"};
double  avWindspeed = 0.0;
double  gustWindspeed = 0.0;
byte    quadrant = 0;
double  rainTotal = 0.0;
double  rainRate = 0.0;
double  temperature = 0.0;
int     humidity = 0;
long    lastmillis = millis();
int     cnt=0;

void setup(){
  digitalWrite(4,LOW);pinMode(4,OUTPUT);
  digitalWrite(7,HIGH);pinMode(7,OUTPUT);
  
  Serial.begin(115200);
  Serial.println("Robs V3.0 WMR86 Oregon Decoder                            x x x * C * h - H H S T T T * b * x x * c * x - Id.    ");
  pinMode(ledPin, OUTPUT);
  pinMode(RxPin, INPUT);
  digitalWrite(ledPin, HIGH);
  delay(100);//heart beat
  digitalWrite(ledPin, LOW);
  headerHits=0;
}

void loop(){
  //wait here for a header!!!
  // So far this appears as 'inverted' Manchester 1>0==1, 0>1==0 ??? (G. E. Thomas in 1949) http://en.wikipedia.org/wiki/Manchester_code
  while (header == false){
    while (digitalRead(RxPin)==1) { //Stay in loop while logic =1
      //loop while the RxPin==1, first half of bit pattern, just before data transition, 1 to 0
    }//exits when signal == 0, (1->0 falling edge found, transition to value 0) middle of bit pattern, ie the data edge
    delayMicroseconds(sDelay); //Short wait for a 1/4 of the "1" pattern
    if (digitalRead(RxPin) == 0){ //Check signal is still steady at 0 ( !0 = error detection)
      delayMicroseconds(lDelay); // long wait for next 1/2 of bit pattern, 
      // ie now at 3/4 way through, looks like an ok bit "1" now keep track of how many in row we have detected
      if (digitalRead(RxPin) == 1){ // Check Rx polarity has now swapped, good!
        headerHits ++; // Highly likely a "1" waveform, so count it in
        if (headerHits == HEADERLEN){ //if we can find 20 in a row we will assume it is a header
          header = true; //so we can exit this part, collect rest of header and begin data processing below
          headerHits=0; //reset, so ready to search for another header when next required, or should an error occur in this packet
          //Serial.println("");
          //Serial.print("H"); //uncomment to debug header detection
        }
      }
      else {
        headerHits =0;  // Has not followed the "1" wave pattern, probably badly formed, noisy waveform, so start again
        header = false; // make sure we look for another header
      }
    }
    else {
      headerHits =0;  // Has not followed wave pattern, probably just noise, so start again
      header = false; // make sure we look for another header
    }
  }

  //The program is now synched to the '1' waveform and detecting the 1->0  "data" transitions in the bit waveform
  //The data byte boundaries indicate the Synch '0' is considered a part of the data, so byte boundary begins at that '0'
  logic=1; // look for rest of header 1's, these must be soaked up intil first 0 arrives to denote start of data
  signal = 0; //RF Signal is at 0 after 1's 1->0 transition, inverted Manchester (see Wiki, it is a matter of opinion)
  firstZero = false; //The first zero is not immediately found, but is flagged when found

  while (header == true){
    //now get last of the header, and then store the data after trigger bit 0 arrives, and data train timing remains valid 
    while (digitalRead(RxPin)!=signal){ //halt here while signal matches inverse of logic, if prev=1 wait for sig=0
    }//exits when signal==logic
    delayMicroseconds(sDelay); //wait for first 1/4 of a bit pulse
    test230 = digitalRead(RxPin);//snapshot of the input
    if ((test230 == signal)&&(nosBytes < maxBytes)){  //after a wait the signal level is the same, so all good, continue!
      delayMicroseconds(lDelay); //wait for second 1/2 of a bit pulse
      test460=digitalRead(RxPin);//snapshot of the input
      if (test230==test460){  // finds a long pulse, so the logic to look for will change, so flip the logic value 
        //Assuming the manchester encoding, a long pulse means data flips, otherwise data stays the same
        logic = logic^1;
        signal = signal^1;
        //Serial.print(logic,BIN);  //debug data stream in binary
        if (!firstZero){ //if this is the first 0-1 data transition then is the sync 0
          digitalWrite(ledPin,1); //data processing begins, first though chew up remaining header
          firstZero = true; //flag that legit data has begun
          //VIP OS Seems to put the Synch '0' bit into the data, as it causes the rest to align onto byte boundaries
          dataByte = B00000000; // set the byte as 1's (just reflects the header bit that have preceded the trigger bit=0)
          dataMask = B00010000; // set the byte as 1's (just reflects the header bit that have preceded the trigger bit=0)
          nosBits = 0;  // preset bit counter so we have 7 bits counted already
          //Serial.print("!");  //debug detection of first zero
        }

      }
      //data stream has been detected begin packing bits into bytes
      if (firstZero){
        if (logic){
          dataByte = dataByte | dataMask; //OR the data bit into the dataByte
        }
        dataMask = dataMask << 1;//rotate the data bit
        if (dataMask==0){
          dataMask=1;//make it roll around, is there a cleaner way than this? eg dataMask *=2?
        }
        nosBits++;
        if (nosBits == 8){ //one byte created, so move onto the next one
          manchester[nosBytes] = dataByte; //store this byte
          nosBits = 0;     //found 8, rezero and get another 8
          dataByte = 0;    //hold the bits in this one
          dataMask = 16;   //mask to do reversed nybbles on the fly
          nosBytes++;      //keep a track of how many bytes we have made
        }
      }
    }  
    else {
      //non valid data found, or maxBytes equalled by nosBytes, reset all pointers and exit the while loop
      headerHits = 0;    // make sure header search begins again
      header = false;    // make sure we look for another header
      firstZero = false; // make sure we look for another 1->0 transition before processing incoming stream
      nosBytes = 0;      // either way, start again at beginning of the bank
    }
 
//if (nosBytes == 2){ Serial.print(manchester[0],HEX);Serial.println(manchester[1],HEX);}
      
    if (nosBytes == maxBytes){ 
      char paire[3];paire[2]='\0';
      long temps=(millis()/1000-lastmillis);lastmillis=millis()/1000;
      sprintf(paire,"%.4u",(lastmillis)/60);Serial.print(paire);Serial.print(":");
      sprintf(paire,"%.2u",(lastmillis)%60);Serial.print(paire);Serial.print(" ");
      Serial.print(cnt++);Serial.print(" ");
      for(int i=0;i<=9;i++){sprintf(paire,"%.2x",manchester[i]);Serial.print(paire);Serial.print(" ");}Serial.println("-");
      
      if (manchester[0]==0xaf){ //detected the Thermometer and Hygrometer
        if(ValidCS(16)){
          thermom();
        }
      }
      if (manchester[0]==0xa1){  //detected the Anemometer and Wind Direction
        if(ValidCS(18)){
          //hexBank();
          //binBank();
          anemom();
        }
      }
      if (manchester[0]==0xa2){  //detected the Rain Gauge
        if(ValidCS(19)){
          rain();
        }
      }
 
      headerHits = 0;
      header = false;
      nosBytes =0;       //reset byte pointer into bank
    }
  }
  digitalWrite(ledPin,0); //data processing ends, look for another header
} //end of main loop

//Support Routines for Nybbles and CheckSum

// http://www.lostbyte.com/Arduino-OSV3/ (9) brian@lostbyte.com
// Directly lifted, then modified from Brian's work, due to nybbles bits now in correct order MSB->LSB
// CS = the sum of nybbles, 1 to (CSpos-1) & 0xf, compared to CSpos nybble;
bool ValidCS(int CSPos){
  bool ok = false;
  byte check = nyb(CSPos);
  byte cs = 0;
  for (int x=1; x<CSPos; x++){
    byte test=nyb(x);
    cs +=test;
  }
  byte tmp = cs & 0xf;
  if (tmp == check){
    ok = true;
  }
  return ok;
}
// Get a nybble from manchester bytes, short name so equations elsewhere are neater :-)
byte nyb(int nybble){
  int bite = nybble / 2;  //DIV 2, find the byte
  int nybb  = nybble % 2;  //MOD 2  0=MSB 1=LSB
  byte b = manchester[bite];
  if (nybb == 0){
    b = (byte)((byte)(b) >> 4);
  }
  else{
    b = (byte)((byte)(b) & (byte)(0xf));
  }       
  return b;
}

//Calculation Routines

/*   PCR800 Rain Gauge  Sample Data:
 //  0        1        2        3        4        5        6        7        8        9
 //  A2       91       40       50       93       39       33       31       10       08 
 //  0   1    2   3    4   5    6   7    8   9    A   B    C   D    E   F    0   1    2   3    
 //  10100010 10010001 01000000 01010000 10010011 00111001 00110011 00110001 00010000 00001000 
 //  -------- -------  bbbb---  RRRRRRRR 88889999 AAAABBBB CCCCDDDD EEEEFFFF 00001111 2222CCCC
 
 // byte(0)_byte(1) = Sensor ID?????
 // bbbb = Battery indicator??? (7)
 // RRRRRRRR = Rolling Code ID
 // 222211110000.FFFFEEEEDDDD = Total Rain Fall (inches)
 // CCCCBBBB.AAAA99998888 = Current Rain Rate (inches per hour)
 // CCCC = CRC
 // Message length is 20 nybbles so working in inches
 Three tips caused the following
 Rain total: 11.72   rate: 39.33   tips: 300.41
 Rain total: 11.76   rate: 0.31   tips: 301.51
 Rain total: 11.80   rate: 0.31   tips: 302.54
 1 tip=0.04 inches or mm?
 My experiment
 24.2 converts reading below to mm (Best calibration so far)
 0.127mm per tip
 */
void rain(){
  rainTotal = ((nyb(18)*100000)+(nyb(17)*10000)+(nyb(16)*1000)+(nyb(15)*100)+(nyb(14)*10)+nyb(13))*24.2/1000;
  Serial.print("  Total Rain ");
  Serial.print(rainTotal);
  Serial.print(" mm, ");
  float rainRate = ((nyb(7)*100000)+(nyb(8)*10000)+(nyb(9)*1000)+(nyb(10)*100)+(nyb(11)*10)+nyb(12))*24.2/10000;
  Serial.print("Rain Rate ");
  Serial.print(rainRate);
  Serial.println(" mm/hr ");
}

// WGR800 Wind speed sensor
// Sample Data:
// 0        1        2        3        4        5        6        7        8        9
// A1       98       40       8E       00       0C       70       04       00       34
// 0   1    2   3    4   5    6   7    8   9    A   B    C   D    E   F    0   1    2   3
// 10100001 10011000 01000000 10001110 00000000 00001100 01110000 00000100 00000000 00110100
// -------- -------- bbbb---- RRRRRRRR xxx8999- xxxxxxxx CCCCDDDD xxxxFFFF 0000---- CCCC----
// Av Speed 0.4000000000m/s Gusts 0.7000000000m/s  Direction: N  

// byte(0)_byte(1) = Sensor ID?????
// bbbb = Battery indicator??? (7)
// RRRRRRRR = Rolling Code
// 8999 = Direction
// DDDD.CCCC = Gust Speed (m per sec)
// 0000.FFFF = Avg Speed(m per sec)
// multiply by 3600/1000 for km/hr

void anemom(){
  //D A1 98 40 8E 08 0C 60 04 00 A4
  avWindspeed = ((nyb(16)*10)+ nyb(15))*3.6/10;
  Serial.print("Av Speed ");
  Serial.print(avWindspeed);
  float gustWindspeed =((nyb(13)*10)+nyb(12))*3.6/10;
  Serial.print(" km/hr, Gusts ");
  Serial.print(gustWindspeed);
  quadrant = nyb(9)&0xf;
  Serial.print(" km/hr, Direction: ");
  Serial.println(windDir[quadrant]);
}

// THGN800 Temperature and Humidity Sensor
// 0        1        2        3        4        5        6        7        8        9          Bytes
// 0   1    2   3    4   5    6   7    8   9    A   B    C   D    E   F    0   1    2   3      nybbles
// 01011111 00010100 01000001 01000000 10001100 10000000 00001100 10100000 10110100 01111001   Bits
// -------- -------- bbbbcccc RRRRRRRR 88889999 AAAABBBB SSSSDDDD EEEEWWWW CCCC---- --------   Explanation
// byte(0)_byte(1) = Sensor ID?????
// bbbb = Battery indicator??? (7)
// byte(3) is rolling code R
// nybble(5) is channel selector c
// BBBBAAAA.99998888 Temperature in BCD
// SSSS sign for negative (- is !=0)
// EEEEDDDD Humidity in BCD
// WWWW 8=dry C=wet 0=middle
// nybble(16) is CRC C
// H 00 01 02 03 04 05 06 07 08 09    Byte Sequence
// D AF 82 41 CB 89 42 00 48 85 55    Real example
// Temperature 24.9799995422 degC Humidity 40.0000000000 % rel

void thermom(){
//  Serial.println("                                                          x x x * C * x - H H S T T T * b * x x * c * x - Id.    ");  

  if(nyb(5)!=99){Serial.print(" channel ");Serial.print(nyb(5));Serial.print(" ");
  Serial.print("Temperature ");
  temperature = (float)((nyb(11)*100)+(nyb(10)*10)+nyb(9))/10; //accuracy to 0.01 degree seems unlikely
  if(nyb(12)==1){//  Trigger a negative temp sign
    Serial.print("-");
  }
  Serial.print(temperature);
  Serial.print(" C, Humidity ");
  humidity = (nyb(14)*10)+nyb(13);
  Serial.print(humidity);
  Serial.print("% Rel ");
  switch(nyb(15)){
    case 4:Serial.print("com");break;// 70>h>40 & 25>t>20
    case 8:Serial.print("dry");break;// h<=40%
    case 12:Serial.print("wet");break;// h>=70%    
    default:Serial.print("   ");break;break;
  }
  if((nyb(8)&0x4)!=0){Serial.print(" batt ");}
  else{Serial.print("      ");}

for(int i=19;i>=17;i--){Serial.print(nyb(i),HEX);Serial.print(" ");}Serial.print("* ");Serial.print(nyb(16),HEX);Serial.print(" * ");Serial.print(nyb(15),HEX);Serial.print(" - ");for(int i=14;i>=9;i--){Serial.print(nyb(i),HEX);Serial.print(" ");}Serial.print("* ");Serial.print(nyb(8),HEX);Serial.print(" * ");for(int i=7;i>=6;i--){Serial.print(nyb(i),HEX);Serial.print(" ");}Serial.print("* ");Serial.print(nyb(5),HEX);Serial.print(" * ");Serial.print(nyb(4),HEX);Serial.print(" - ");for(int i=3;i>=0;i--){Serial.print(nyb(i),HEX);Serial.print(" ");}Serial.println();
}}

// Handy Debugging Routines

void binBank(){
  //Print the fully aligned binary data in manchester[] array
  Serial.print("D ");
  for( int i=0; i < maxBytes; i++){ 
    byte mask = B10000000;
    if (manchester[i]<16){
      Serial.print("0"); //pad single digit hex
    }
    Serial.print(manchester[i],HEX);
    Serial.print(" ");
    for (int k=0; k<8; k++){
      if (manchester[i] & mask){
        Serial.print("1");
      }
      else{
        Serial.print("0");
      }
      mask = mask >> 1;
    }
    Serial.print(" ");
  }
  Serial.println();
}

void hexBank(){
  //Print the fully aligned binary data, enable the headers if desired
  //Serial.println("H 00 01 02 03 04 05 06 07 08 09");
  //Serial.println("  00 00 00 00 00 00 00 00 11 11");
  //Serial.println("B 10 32 54 76 98 BA DC FE 10 32");
  Serial.print("D ");
  for( int i=0; i < maxBytes; i++){ 
    if (manchester[i]<16){
      Serial.print("0"); //pad single digit hex
    }
    Serial.print(manchester[i],HEX);
    Serial.print(" ");
  }
  Serial.println();
}




